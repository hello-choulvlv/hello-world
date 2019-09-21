#include "myd3d.h"
#include "sh.h"



// Output SH_Y or SH_Coefficients
void OutputSH(float *pData, UINT iBandNum)
{
	char a[1000] = "";
	char temp[1000] = "";
	for(UINT i = 0; i < iBandNum; i++)
	{
		for(UINT j = 0; j < 2*i+1; j++)
		{
			sprintf(temp, "%f, ", pData[i*i+j]);
			strcat(a, temp);
		}

		sprintf(temp, "\n");
		strcat(a, temp);

		// Higher Band has many numbers, so they take on more than one line
		if(i > 4)
		{
			sprintf(temp, "\n");
			strcat(a, temp);
		}
	}
	mymessage(a);
}









/************************************************************************/
/*					Spherical Harmonics CubeMaps                        */
/************************************************************************/

// 必须要用FP32（Cubemap）、MRT、ps2.a（其实就是sm3.0）
// Resolution表示不要太大，64*64*6=24576，已经包含了24576条采样射线，足够用了，分辨率过大的话会导致填充时间特别长
HRESULT KSHCubeMap::Init(UINT iBandNum, UINT iSampleNum, char *szPS, UINT iResolution/* = 64*/)
{
	// 最多只支持4Band！否则纹理层不够
	if(iBandNum < MYSH_MINORDER || iBandNum > MYSH_MAXORDER || !iSampleNum || !iResolution || !szPS)
		return D3DERR_INVALIDCALL;
	if(m_bInit)
		return D3DERR_NOTAVAILABLE;

	// 先释放
	Release();

	m_iBandNum = iBandNum;
	m_iTextureNum = (iBandNum*iBandNum) / 4;

	// 不满4个通道的，要加一个
	if((iBandNum*iBandNum) % 4)
		m_iTextureNum++;
	
	if(!m_iTextureNum)
		return D3DERR_INVALIDCALL;


	// 创建Shader
	D3DVERTEXELEMENT9 dclr[] = {
		{0,0,D3DDECLTYPE_FLOAT4 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0},
		D3DDECL_END()
	};
	if(FAILED(d3ddevice->CreateVertexDeclaration(dclr, &m_pVertexDeclaration)))
		return E_FAIL;

	if(FAILED(m_PS.InitPixelShader(szPS)))
	{
		OutputDebugString("请检查指定的文件是否是GPU Project CubeMap用的Pixel Shader！\n");
		return E_FAIL;
	}


	// 创建VB，就是绘制TextureNum个点而已，用XYZRHW就够了，主要是用PS，VS可以忽略掉
	m_iVBStride = sizeof(D3DXVECTOR4);
	if(FAILED(d3ddevice->CreateVertexBuffer(m_iTextureNum * m_iVBStride, D3DUSAGE_0, 0, D3DPOOL_DEFAULT, &m_pVB, NULL)))
		return E_FAIL;
	LPD3DXVECTOR4 pData = NULL;
	if(FAILED(m_pVB->Lock(0, m_iTextureNum * m_iVBStride, (void **)&pData, 0)))
		return E_FAIL;
	for(UINT i = 0; i < m_iTextureNum; i++)
	{
		*pData++ = D3DXVECTOR4((float)i, 0.0f, 0.0f, 1.0f);
	}
	if(FAILED(m_pVB->Unlock()))
		return E_FAIL;


	// 创建CubeMap
	D3DFORMAT Format = D3DFMT_A32B32G32R32F;
	m_ppCubeMapSH = new LPDIRECT3DCUBETEXTURE9[m_iTextureNum];

	for(UINT i = 0; i < m_iTextureNum; i++)
		if(FAILED(d3ddevice->CreateCubeTexture(iResolution, 1, D3DUSAGE_0, Format, D3DPOOL_MANAGED, &m_ppCubeMapSH[i], NULL)))
		{
			OutputDebugString("创建CubeMap出错，请检查纹理分辨率，最好是2的幂，否则很多硬件不支持！\n");
			return E_FAIL;
		}

	if(FAILED(GenerateSHCubeMap()))
		return E_FAIL;


	// 创建采样点及采样纹理
	if(FAILED(m_Samples.Init(iSampleNum, iBandNum)))
		return E_FAIL;
	m_iSampleNum = iSampleNum;
	
	if(FAILED(d3ddevice->CreateTexture(iSampleNum, 1, 1, D3DUSAGE_0, Format, D3DPOOL_MANAGED, &m_pTexSamples, NULL)))
	{
		OutputDebugString("创建采样点纹理出错，请检查采样点数量，最好是2的幂，否则很多硬件不支持！\n");
		return E_FAIL;
	}

	if(FAILED(GenerateSampleMap()))
		return E_FAIL;

	// 创建RGB纹理
	if(FAILED(d3ddevice->CreateTexture(m_iTextureNum, 1, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pTexProjectR, NULL)))
		return E_FAIL;
	if(FAILED(d3ddevice->CreateTexture(m_iTextureNum, 1, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pTexProjectG, NULL)))
		return E_FAIL;
	if(FAILED(d3ddevice->CreateTexture(m_iTextureNum, 1, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pTexProjectB, NULL)))
		return E_FAIL;

	// 创建临时纹理
	if(FAILED(d3ddevice->CreateTexture(m_iTextureNum, 1, 1, D3DUSAGE_0, Format, D3DPOOL_SYSTEMMEM, &m_pTexSystemMem, NULL)))
		return E_FAIL;

	m_bInit = TRUE;
	return S_OK;
}


// 填充Cubemap用的回调函数
void WINAPI SHCubeFill(D3DXVECTOR4* pOut, CONST D3DXVECTOR3* pTexCoord, CONST D3DXVECTOR3* pTexelSize, LPVOID pData)
{
	UINT *pMonoNum = (UINT *)pData;
	
	// 得到所有的Y
	float *pY = new float[pMonoNum[1]];
	if(!pY)
		return;
	
	static D3DXVECTOR3 VecCoord(0, 0, 0);
	D3DXVec3Normalize(&VecCoord, pTexCoord);
	if(FAILED(D3DXSHEvalDirection(pY, pMonoNum[2], &VecCoord)))
		return;

	// 根据当前单项式数写入对应的Y
	float fColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	// 这样做是预防最后会有不足4的情况发生，这样的话就会用初始值0
	for(UINT i = 0; i < 4; i++)
	{
		if((i + pMonoNum[0]) >= pMonoNum[1])
			break;
		fColor[i] = pY[i + pMonoNum[0]];
	}
	// R/G/B/A分别对应0/1/2/3
	*pOut = D3DXVECTOR4(fColor[0], fColor[1], fColor[2], fColor[3]);
	SAFE_DELETE_ARRAY(pY);
}


HRESULT KSHCubeMap::GenerateSHCubeMap()
{
	// 参数表，第一个参数表示当前已填充的单项式数，第二个表示最大单项式数，第三个表示总Band数
	UINT iMonoNum[3] = {0, m_iBandNum*m_iBandNum, m_iBandNum};

	for(UINT i = 0; i < m_iTextureNum; i++)
	{
		if(FAILED(D3DXFillCubeTexture(m_ppCubeMapSH[i], SHCubeFill, iMonoNum)))
			return E_FAIL;
		// 每填充一个Cube，当前已填充的单项式数就加4
		iMonoNum[0] += 4;
	}

	return S_OK;
}



// 填充SampleTexture用的回调函数
void WINAPI SamplesTexFill(D3DXVECTOR4* pOut, CONST D3DXVECTOR2* pTexCoord, CONST D3DXVECTOR2* pTexelSize, LPVOID pDataHead)
{
	float *pData = (float *)pDataHead;
	UINT iSampleNum = *(UINT *)pData++;
	UINT iCurrentSampleNo = (UINT)( (float)(iSampleNum-1) * pTexCoord->x );
	LPD3DXVECTOR3 pSamplesData = (LPD3DXVECTOR3)pData + iCurrentSampleNo;

	*pOut = D3DXVECTOR4(pSamplesData->x, pSamplesData->y, pSamplesData->z, 1.0f);
}

HRESULT KSHCubeMap::GenerateSampleMap()
{
	float *pDataHead = new float[3 * m_iSampleNum + 1];
	float *pData = pDataHead;
	UINT *pSampleNum = (UINT *)pData;
	
	// 参数表，第一个参数表示总采样点数，后面就是整个采样向量值数组
	// 先写入总采样点数
	*pSampleNum = m_iSampleNum;
	pData++;

	LPD3DXVECTOR3 pDataVector3 = (LPD3DXVECTOR3)pData;

	// 再写入向量值
	D3DXVECTOR3 VecSample(0, 0, 0);
	for(UINT i = 0; i < m_iSampleNum; i++)
	{
		if(FAILED(m_Samples.GetSample(i, &VecSample)))
			return E_FAIL;
		*pDataVector3++ = VecSample;		
	}
		

	if(FAILED(D3DXFillTexture(m_pTexSamples, SamplesTexFill, pDataHead)))
		return E_FAIL;

	SAFE_DELETE_ARRAY(pDataHead);

	return S_OK;
}









HRESULT KSHCubeMap::GPUProjectCubeMap(LPDIRECT3DCUBETEXTURE9 pCubeMap, float *pSHCoefficients[3])
{
	if(!m_bInit)
		return D3DERR_NOTAVAILABLE;
	if(!pCubeMap || !pSHCoefficients[0] || !pSHCoefficients[1] || !pSHCoefficients[2])
		return D3DERR_INVALIDCALL;

	// 先Get，再Set
	LPDIRECT3DSURFACE9 pOldRT[3] = {NULL, NULL, NULL};
	HRESULT hr = S_OK;
	for(UINT i = 0; i < 3; i++)
	{
		if(FAILED(hr = d3ddevice->GetRenderTarget(i, &pOldRT[i])))
			if(hr != D3DERR_NOTFOUND)
			{
				OutputDebugString("GetMRT失败，可能是显卡不支持MRT！\n");
				return E_FAIL;
			}
	}

	// 设置渲染相关
	if(FAILED(d3ddevice->SetFVF(D3DFVF_XYZRHW)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexDeclaration(m_pVertexDeclaration)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexShader(NULL)))
		return E_FAIL;
	if(FAILED(m_PS.SetPixelShader()))
		return E_FAIL;

	// 设置MRT
	LPDIRECT3DSURFACE9 pSurfR = NULL, pSurfG = NULL, pSurfB = NULL;
	LPDIRECT3DSURFACE9 pSurfSysMem = NULL;

	if(FAILED(m_pTexProjectR->GetSurfaceLevel(0, &pSurfR)))
		return E_FAIL;
	if(FAILED(m_pTexProjectG->GetSurfaceLevel(0, &pSurfG)))
		return E_FAIL;
	if(FAILED(m_pTexProjectB->GetSurfaceLevel(0, &pSurfB)))
		return E_FAIL;

	if(FAILED(d3ddevice->SetRenderTarget(0, pSurfR)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetRenderTarget(1, pSurfG)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetRenderTarget(2, pSurfB)))
		return E_FAIL;

	// 设置纹理
	if(FAILED(d3ddevice->SetTexture(0, pCubeMap)))
	{
		OutputDebugString("SetTexture失败！可能是指定的CubeMap创建在系统内存中了！请改用Managed或Default！\n");
		return E_FAIL;
	}	// Tex0: 被投影的Cubemap

	if(FAILED(d3ddevice->SetTexture(1, m_pTexSamples)))
		return E_FAIL;	// Tex1: 采样向量数据

	// 设置渲染参数
	if(FAILED(d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE)))
		return E_FAIL;	// 平面物体要先关掉Z

	// 设置常量寄存器
	// c0 0，	蒙特卡罗系数4Pi/N	1/采样总数
	// i0 采样总数
	m_PS.SetConstant(0, &D3DXVECTOR4(	0.0f,	4.0f*D3DX_PI/(float)m_iSampleNum,	1.0f/((float)m_iSampleNum-1.0f),	0.0f	), 1);

	// 因为一层循环最大只支持255次，这里以128次循环为准，给两层循环
	int iConstReg1[4] = {(int)(m_iSampleNum>128 ? 128 : m_iSampleNum), 0, 1, 0};
	int iConstReg2[4] = {(int)m_iSampleNum / 128, 0, 1, 0};

	m_PS.SetConstantI(0, iConstReg1, 1);
	m_PS.SetConstantI(1, iConstReg2, 1);


	// 循环渲染
	if(FAILED(d3ddevice->BeginScene()))
		return E_FAIL;

	for(int i = 0; i < m_iTextureNum; i++)
	{
		if(FAILED(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iVBStride)))
			return E_FAIL;

		d3ddevice->SetTexture(2, m_ppCubeMapSH[i]);	// 第2层：当前的SH数据

		// 渲染
		if(FAILED(d3ddevice->DrawPrimitive(D3DPT_POINTLIST, i, 1)))
			return E_FAIL;
	}

	if(FAILED(d3ddevice->EndScene()))
		return E_FAIL;


	// 读取渲染好的SH系数
	D3DLOCKED_RECT Rect;
	if(FAILED(m_pTexSystemMem->GetSurfaceLevel(0, &pSurfSysMem)))
		return E_FAIL;

	if(FAILED(d3ddevice->GetRenderTargetData(pSurfR, pSurfSysMem)))
		return E_FAIL;	
	if(FAILED(pSurfSysMem->LockRect(&Rect, NULL, 0)))
		return E_FAIL;
	memcpy(pSHCoefficients[0], Rect.pBits, m_iBandNum * m_iBandNum * sizeof(float));
	if(FAILED(pSurfSysMem->UnlockRect()))
		return E_FAIL;

	if(FAILED(d3ddevice->GetRenderTargetData(pSurfG, pSurfSysMem)))
		return E_FAIL;	
	if(FAILED(pSurfSysMem->LockRect(&Rect, NULL, 0)))
		return E_FAIL;
	memcpy(pSHCoefficients[1], Rect.pBits, m_iBandNum * m_iBandNum * sizeof(float));
	if(FAILED(pSurfSysMem->UnlockRect()))
		return E_FAIL;

	if(FAILED(d3ddevice->GetRenderTargetData(pSurfB, pSurfSysMem)))
		return E_FAIL;	
	if(FAILED(pSurfSysMem->LockRect(&Rect, NULL, 0)))
		return E_FAIL;
	memcpy(pSHCoefficients[2], Rect.pBits, m_iBandNum * m_iBandNum * sizeof(float));
	if(FAILED(pSurfSysMem->UnlockRect()))
		return E_FAIL;


	// 恢复以前保存的渲染参数
	m_PS.RestorePixelShader();

	if(FAILED(d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE)))
		return E_FAIL;

	// 注意这里的性能下降！！！设置MRT太恐怖了，奇怪的是前面怎么没下降多少啊
	for(UINT i = 0; i < 3; i++)
	{
		if(FAILED(d3ddevice->SetRenderTarget(i, pOldRT[i])))
			return E_FAIL;
		SAFE_RELEASE(pOldRT[i]);
	}



	// 结束
	SAFE_RELEASE(pSurfR);
	SAFE_RELEASE(pSurfG);
	SAFE_RELEASE(pSurfB);
	SAFE_RELEASE(pSurfSysMem);

	return S_OK;
}









// 根据面属性和对应的2D像素来得到对应的Cubemap坐标（采样射线）
HRESULT KSHCubeMap::GetCubeMapVector(D3DCUBEMAP_FACES iFace, UINT iResolutionX, UINT iResolutionY, UINT iX, UINT iY, LPD3DXVECTOR3 pVecCoord)
{
	if(iFace > D3DCUBEMAP_FACE_NEGATIVE_Z || !pVecCoord)
		return D3DERR_INVALIDCALL;
	if(!iResolutionX || !iResolutionY || iX >= iResolutionX || iY >= iResolutionY)
		return D3DERR_INVALIDCALL;

	float fX = (float)iX / (float)(iResolutionX-1);
	float fY = (float)iY / (float)(iResolutionY-1);

	// Convert to Unit Cube Coordinate
	fX -= 0.5f;
	fY -= 0.5f;

	switch(iFace)
	{
	case D3DCUBEMAP_FACE_POSITIVE_X:
		pVecCoord->x = 0.5f;
		pVecCoord->y = -fY;
		pVecCoord->z = -fX;
		break;
	case D3DCUBEMAP_FACE_NEGATIVE_X:
		pVecCoord->x = -0.5f;
		pVecCoord->y = -fY;
		pVecCoord->z = fX;
		break;
	case D3DCUBEMAP_FACE_POSITIVE_Y:
		pVecCoord->x = fX;
		pVecCoord->y = 0.5f;
		pVecCoord->z = fY;
		break;
	case D3DCUBEMAP_FACE_NEGATIVE_Y:
		pVecCoord->x = fX;
		pVecCoord->y = -0.5f;
		pVecCoord->z = -fY;
		break;
	case D3DCUBEMAP_FACE_POSITIVE_Z:
		pVecCoord->x = fX;
		pVecCoord->y = -fY;
		pVecCoord->z = 0.5f;
		break;
	case D3DCUBEMAP_FACE_NEGATIVE_Z:
		pVecCoord->x = -fX;
		pVecCoord->y = -fY;
		pVecCoord->z = -0.5f;
		break;
	}

	D3DXVec3Normalize(pVecCoord, pVecCoord);
	return S_OK;
}


// 根据采样射线找到CubeMap对应的交点处，操作对应的像素数据Get/Set（BYTE指针必须先分配好一个像素所占的空间），而且必须预先根据Format指定像素大小（字节），默认为A8R8G8B8
// 第一个参数表示Get/Set，这是内部函数，通过Get/SetCubeMapPixelData来访问
HRESULT KSHCubeMap::OperateCubeMapPixelData(BOOL bGet, LPDIRECT3DCUBETEXTURE9 pCubeMap, D3DXVECTOR3 VecRay, void *pPixelData, UINT iPixelSize /* = 4 */, UINT iMip /* = 0 */)
{
	if( !pCubeMap || !pPixelData || D3DXVec3Length(&VecRay) == 0.0f || iMip > 7 || !iPixelSize)
		return D3DERR_INVALIDCALL;

	D3DXVec3Normalize(&VecRay, &VecRay);

	// 先按照射线方向得到哪个面及对应的2D纹理坐标
	D3DCUBEMAP_FACES iFace = D3DCUBEMAP_FACE_FORCE_DWORD;
	// 定义6个面，是一个单位盒子，坐标都从-0.5～0.5
	D3DXPLANE pPlanes[6] = 
	{
		D3DXPLANE(-1, 0, 0, -0.5f),		// Positive X
		D3DXPLANE(1, 0, 0, -0.5f),		// Negative X
		D3DXPLANE(0, -1, 0, -0.5f),		// Positive Y
		D3DXPLANE(0, 1, 0, -0.5f),		// Negative Y
		D3DXPLANE(0, 0, -1, -0.5f),		// Positive Z
		D3DXPLANE(0, 0, 1, -0.5f),		// Negative Z
	};

	D3DXVECTOR3 PtIntersect(0, 0, 0);
	D3DXVECTOR2 TexCoord(0, 0);

	float fDelta = 0.00001f;	// 给一个小的增量，确保浮点比较操作的正确性

	// 由于前面判断了射线的合法性，所以这里它必定能与多个平面相交，但只能与其中一个面相交（面是有范围限制的）
	int i;
	for( i = 0; i < 6 ; i++)
	{
		if(GetIntersectPlane(D3DXVECTOR3(0, 0, 0), VecRay, pPlanes[i], &PtIntersect))
		{
			// 这里需要判断，交点是否正常，即在对应面的范围之内，因为平面是没有范围概念的，所以这里还要做范围判断
			// 另外生成纹理坐标的时候，要从-0.5～0.5转换为0～1，另外考虑到有些面上的V轴和实际的XYZ坐标轴方向相反，所以还需要取负
			// 交点在十二条边上时，有效性分别平均到每个面上，每个面只有对于该2D面的右和下边有效（即交点等于+/-0.5的时候）
			if(i == 0)
				if(PtIntersect.y>=-(0.5f+fDelta) && PtIntersect.y<(0.5f+fDelta) && PtIntersect.z>=-(0.5f+fDelta) && PtIntersect.z<(0.5f+fDelta))
				{
					TexCoord = D3DXVECTOR2(-PtIntersect.z+0.5f, -PtIntersect.y+0.5f);
					break;
				}
			if(i == 1)
				if(PtIntersect.y>-(0.5f+fDelta) && PtIntersect.y<=(0.5f+fDelta) && PtIntersect.z>-(0.5f+fDelta) && PtIntersect.z<=(0.5f+fDelta))
				{
					TexCoord = D3DXVECTOR2(PtIntersect.z+0.5f, -PtIntersect.y+0.5f);
					break;
				}
			if(i == 2)
				if(PtIntersect.x>-(0.5f+fDelta) && PtIntersect.x<=(0.5f+fDelta) && PtIntersect.z>=-(0.5f+fDelta) && PtIntersect.z<(0.5f+fDelta))
				{
					TexCoord = D3DXVECTOR2(PtIntersect.x+0.5f, PtIntersect.z+0.5f);
					break;
				}
			if(i == 3)
				if(PtIntersect.x>=-(0.5f+fDelta) && PtIntersect.x<(0.5f+fDelta) && PtIntersect.z>-(0.5f+fDelta) && PtIntersect.z<=(0.5f+fDelta))
				{
					TexCoord = D3DXVECTOR2(PtIntersect.x+0.5f, -PtIntersect.z+0.5f);
					break;
				}
			if(i == 4)
				if(PtIntersect.x>-(0.5f+fDelta) && PtIntersect.x<=(0.5f+fDelta) && PtIntersect.y>-(0.5f+fDelta) && PtIntersect.y<=(0.5f+fDelta))
				{
					TexCoord = D3DXVECTOR2(PtIntersect.x+0.5f, -PtIntersect.y+0.5f);
					break;
				}
			if(i == 5)
				if(PtIntersect.x>=-(0.5f+fDelta) && PtIntersect.x<(0.5f+fDelta) && PtIntersect.y>=-(0.5f+fDelta) && PtIntersect.y<(0.5f+fDelta))
				{
					TexCoord = D3DXVECTOR2(-PtIntersect.x+0.5f, -PtIntersect.y+0.5f);
					break;
				}
		}
	}

	// 没有和任何一个面相交？？？不可能的，一定是内部错误
	if(i > 5)
		return E_FAIL;

	iFace = D3DCUBEMAP_FACES(i);

	// 得到面信息，定位到2D像素坐标
	D3DSURFACE_DESC Desc;
	if(FAILED(pCubeMap->GetLevelDesc(iMip, &Desc)))
		return E_FAIL;
	
	float fX = TexCoord.x * (float)(Desc.Width-1);
	float fY = TexCoord.y * (float)(Desc.Height-1);
	// 这里千万不能直接取整，不然会丢失小数尾数
	UINT iX = (UINT)roundf(fX);
	UINT iY = (UINT)roundf(fY);

	// Lock & Locate & Copy Data
	D3DLOCKED_RECT LockRect;

	if(bGet)
	{
		if(FAILED(pCubeMap->LockRect(iFace, iMip, &LockRect, NULL, D3DLOCK_READONLY)))
			return E_FAIL;
	}
	else
	{
		if(FAILED(pCubeMap->LockRect(iFace, iMip, &LockRect, NULL, 0)))
			return E_FAIL;
	}

	BYTE *pData = (BYTE *)LockRect.pBits;
	pData += iY * LockRect.Pitch;
	pData += iX * iPixelSize;

	// Get Data
	if(bGet)
		memcpy(pPixelData, pData, iPixelSize);
	// Set Data
	else
		memcpy(pData, pPixelData, iPixelSize);

	// 结束，CubeMap就是干净，连Surface都用不着，更不需要Release
	if(FAILED(pCubeMap->UnlockRect(iFace, iMip)))
		return E_FAIL;

	return S_OK;
}





HRESULT KSHCubeMap::GetCubeMapPixelData(LPDIRECT3DCUBETEXTURE9 pCubeMap, D3DXVECTOR3 VecRay, void *pPixelData, UINT iPixelSize /* = 4 */, UINT iMip /* = 0 */)
{
	return OperateCubeMapPixelData(1, pCubeMap, VecRay, pPixelData, iPixelSize, iMip);
}
HRESULT KSHCubeMap::SetCubeMapPixelData(LPDIRECT3DCUBETEXTURE9 pCubeMap, D3DXVECTOR3 VecRay, void *pPixelData, UINT iPixelSize /* = 4 */, UINT iMip /* = 0 */)
{
	return OperateCubeMapPixelData(0, pCubeMap, VecRay, pPixelData, iPixelSize, iMip);
}


// Cubemap Projection Using Random Samples
HRESULT KSHCubeMap::ProjectCubeMap(LPDIRECT3DCUBETEXTURE9 pCubeMap, UINT iSampleNum, UINT iBandNum, float *pSHCoefficients[3])
{
	if(!pCubeMap || !iSampleNum || iBandNum < MYSH_MINORDER || iBandNum > MYSH_MAXORDER || !pSHCoefficients)
		return D3DERR_INVALIDCALL;

	if(!pSHCoefficients[0] || !pSHCoefficients[1] || !pSHCoefficients[2])
		return D3DERR_INVALIDCALL;

	// Initialize Random Samples
	KMCSample Samples;
	HRESULT hr = Samples.Init(iSampleNum, iBandNum);
	if(FAILED(hr))
		return hr;

	// Init RGB Coef Buffer
	float *pCoef[3] = {pSHCoefficients[0], pSHCoefficients[1], pSHCoefficients[2]};
	//	float *pBrightCoef = pSHCoefficients[0];

	UINT i = 0, j = 0;
	for(j = 0; j < iBandNum*iBandNum; j++)
	{
		pCoef[0][j] = 0;
		pCoef[1][j] = 0;
		pCoef[2][j] = 0;
//		pBrightCoef[j] = 0;
	}



	// SH Polymonia, Temp Use
	float *pY = NULL;

	// R/G/B Color Bright, Temp Use
	DWORD dwColor = 0;
	float fR = 0.0f, fG = 0.0f, fB = 0.0f, fBright = 0.0f;
	// Sample Coord, Temp Use
	D3DXVECTOR3 VecSample;

	for(i = 0; i < iSampleNum; i++)
	{
		// Get Sample
		if(FAILED(Samples.GetSample(i, &VecSample)))
			return E_FAIL;

		// Get SH
		pY = Samples.GetSHTable(i);
		if(!pY)
			return E_FAIL;

		// Get Color
		if(FAILED(GetCubeMapPixelData(pCubeMap, VecSample, &dwColor)))
			return E_FAIL;

		fR = (float)((dwColor>>16)&0xff) / 255.0f;
		fG = (float)((dwColor>>8)&0xff) / 255.0f; 
		fB = (float)((dwColor>>0)&0xff) / 255.0f;

		// color, clamp to 0～1
		fR = clampf(fR, 0.0f, 1.0f);
		fG = clampf(fG, 0.0f, 1.0f);
		fB = clampf(fB, 0.0f, 1.0f);

//		fBright = fR * 0.2125f + fG * 0.7154f + fB * 0.0721f;

		// Coef += Color*Y
		for(j = 0; j < iBandNum*iBandNum; j++)
		{
			pCoef[0][j] += fR*pY[j];
			pCoef[1][j] += fG*pY[j];
			pCoef[2][j] += fB*pY[j];
//			pBrightCoef[j] += fBright*pY[j];
		}
	}

	// Mean Value
	for(j = 0; j < iBandNum*iBandNum; j++)
	{
		pCoef[0][j] = pCoef[0][j] * 4.0f*D3DX_PI / (float)iSampleNum;
		pCoef[1][j] = pCoef[1][j] * 4.0f*D3DX_PI / (float)iSampleNum;
		pCoef[2][j] = pCoef[2][j] * 4.0f*D3DX_PI / (float)iSampleNum;
//		pBrightCoef[j] = pBrightCoef[j] * 4.0f*D3DX_PI / (float)iSampleNum;
	}

	pSHCoefficients[0] = pCoef[0];
	pSHCoefficients[1] = pCoef[1];
	pSHCoefficients[2] = pCoef[2];

	//pSHCoefficients = pBrightCoef;

	return S_OK;
}


// Cubemap Projection Using Pixel-to-Pixel, Too Slow, Decrease the resolution of your EnvCubemap first
// Optimization：Pre-Compute SH(GenerateSH) and save to file
// Band5 Index 20/24 Error!!!!! Using 4 Bands Recommanded!!!!
// Brightness, Using Each (Red,Green,Blue) Coefficients dot (0.2125f, 0.7154f, 0.0721f)
HRESULT KSHCubeMap::ProjectCubeMap(LPDIRECT3DCUBETEXTURE9 pCubeMap, UINT iBandNum, float *pSHCoefficients[3])
{
	if(!pCubeMap || iBandNum < MYSH_MINORDER || iBandNum > MYSH_MAXORDER || !pSHCoefficients)
		return D3DERR_INVALIDCALL;

	if(!pSHCoefficients[0] || !pSHCoefficients[1] || !pSHCoefficients[2])
		return D3DERR_INVALIDCALL;

	// Init RGB Coef Buffer
	float *pCoef[3] = {pSHCoefficients[0], pSHCoefficients[1], pSHCoefficients[2]};
//	float *pBrightCoef = pSHCoefficients[0];

	UINT i = 0, j = 0, iX = 0, iY = 0;
	UINT iFaceNo = 0;

	for(j = 0; j < iBandNum*iBandNum; j++)
	{
		pCoef[0][j] = 0;
		pCoef[1][j] = 0;
		pCoef[2][j] = 0;
//		pBrightCoef[j] = 0;
	}

	// SH Polymonia, Temp Use
	SHTable TableY;
	if(FAILED(TableY.Init(iBandNum)))
		return E_FAIL;
	float *pY = NULL;

	// R/G/B Color Bright, Temp Use
	DWORD dwColor = 0;
	float fR = 0.0f, fG = 0.0f, fB = 0.0f, fBright = 0.0f;
	// Sample Coord, Temp Use
	D3DXVECTOR3 VecSample;
	D3DXVECTOR2 VecSampleSC;

	// Get CubeMap Attribute
	D3DSURFACE_DESC Desc;
	if(FAILED(pCubeMap->GetLevelDesc(0, &Desc)))
		return E_FAIL;

	UINT iSampleNum = Desc.Width * Desc.Height * 6;


	for(iFaceNo = 0; iFaceNo <= 5; iFaceNo++)
		for(iY = 0; iY < Desc.Height; iY++)
			for(iX = 0; iX < Desc.Width; iX++)
			{
				// Get Sample
				if(FAILED(GetCubeMapVector((D3DCUBEMAP_FACES)iFaceNo, Desc.Width, Desc.Height, iX, iY, &VecSample)))
					return E_FAIL;
				CartesianToSpherical(VecSample, &VecSampleSC);

				// Get SH(Too Slow!!!!)
				if(FAILED(TableY.GenerateSH(VecSampleSC)))
					return E_FAIL;

				pY = TableY.GetSHTable();
				if(!pY)
					return E_FAIL;

				// Get Color
				if(FAILED(GetCubeMapPixelData(pCubeMap, VecSample, &dwColor)))
					return E_FAIL;

				fR = (float)((dwColor>>16)&0xff) / 255.0f;
				fG = (float)((dwColor>>8)&0xff) / 255.0f; 
				fB = (float)((dwColor>>0)&0xff) / 255.0f;

				// color, clamp to 0～1
				fR = clampf(fR, 0.0f, 1.0f);
				fG = clampf(fG, 0.0f, 1.0f);
				fB = clampf(fB, 0.0f, 1.0f);

//				fBright = fR * 0.2125f + fG * 0.7154f + fB * 0.0721f;

				// Coef += Color*Y
				for(j = 0; j < iBandNum*iBandNum; j++)
				{
					pCoef[0][j] += fR*pY[j];
					pCoef[1][j] += fG*pY[j];
					pCoef[2][j] += fB*pY[j];
//					pBrightCoef[j] += fBright*pY[j];
				}

			}

	// Mean Value
	for(j = 0; j < iBandNum*iBandNum; j++)
	{
		pCoef[0][j] = pCoef[0][j] * 4.0f*D3DX_PI / (float)iSampleNum;
		pCoef[1][j] = pCoef[1][j] * 4.0f*D3DX_PI / (float)iSampleNum;
		pCoef[2][j] = pCoef[2][j] * 4.0f*D3DX_PI / (float)iSampleNum;
//		pBrightCoef[j] = pBrightCoef[j] * 4.0f*D3DX_PI / (float)iSampleNum;
	}

//Modifier in Band 5, 20/24 too large
/*	pCoef[0][20] /= (float)iSampleNum;
	pCoef[1][20] /= (float)iSampleNum;
	pCoef[2][20] /= (float)iSampleNum;

	pCoef[0][24] /= (float)iSampleNum;
	pCoef[1][24] /= (float)iSampleNum;
	pCoef[2][24] /= (float)iSampleNum;
*/
	return S_OK;
}




// Cubemap Reconstruction
HRESULT KSHCubeMap::ReconstructCubeMap(LPDIRECT3DCUBETEXTURE9 pCubeMap, UINT iBandNum, float *pCoef[3])
{
	if(!pCubeMap || iBandNum < MYSH_MINORDER || iBandNum > MYSH_MAXORDER || !pCoef)
		return D3DERR_INVALIDCALL;

	// Init RGB Coef Buffer
	if(!pCoef[0] || !pCoef[1] || !pCoef[2])
		return E_OUTOFMEMORY;

	UINT i = 0, j = 0, iX = 0, iY = 0;
	UINT iFaceNo = 0;

	// SH Polymonia, Temp Use
	SHTable TableY;
	if(FAILED(TableY.Init(iBandNum)))
		return E_FAIL;
	float *pY = NULL;

	// R/G/B Color Bright, Temp Use
	D3DCOLOR dwColor = 0;
	float fR = 0.0f, fG = 0.0f, fB = 0.0f, fBright = 0.0f;
	UCHAR uR = 0, uG = 0, uB = 0;
	// Sample Coord, Temp Use
	D3DXVECTOR3 VecSample;
	D3DXVECTOR2 VecSampleSC;

	// Get CubeMap Attribute
	D3DSURFACE_DESC Desc;
	if(FAILED(pCubeMap->GetLevelDesc(0, &Desc)))
		return E_FAIL;

	int iSampleNum = Desc.Width * Desc.Height * 6;


	for(iFaceNo = 0; iFaceNo <= 5; iFaceNo++)
		for(iY = 0; iY < Desc.Height; iY++)
			for(iX = 0; iX < Desc.Width; iX++)
			{
				// Get Sample
				if(FAILED(GetCubeMapVector((D3DCUBEMAP_FACES)iFaceNo, Desc.Width, Desc.Height, iX, iY, &VecSample)))
					return E_FAIL;
				CartesianToSpherical(VecSample, &VecSampleSC);

				// Get SH
				if(FAILED(TableY.GenerateSH(VecSampleSC)))
					return E_FAIL;

				pY = TableY.GetSHTable();
				if(!pY)
					return E_FAIL;

				if(iFaceNo == D3DCUBEMAP_FACE_NEGATIVE_X && iY == 0 && iX == 97)
					fR = fR;

				// Coef += Color*Y
				fR = fG = fB = 0.0f;
				for(j = 0; j < iBandNum*iBandNum; j++)
				{
					fR += pCoef[0][j] * pY[j];
					fG += pCoef[1][j] * pY[j];
					fB += pCoef[2][j] * pY[j];
				}

				// color, clamp to 0～1
				fR = clampf(fR, 0.0f, 1.0f);
				fG = clampf(fG, 0.0f, 1.0f);
				fB = clampf(fB, 0.0f, 1.0f);

				// Get Color
				uR = (UCHAR)(fR*255.0f);
				uG = (UCHAR)(fG*255.0f);
				uB = (UCHAR)(fB*255.0f);

				dwColor = D3DCOLOR_XRGB(uR, uG, uB);

				if(FAILED(SetCubeMapPixelData(pCubeMap, VecSample, &dwColor)))
					return E_FAIL;

			}

	return S_OK;
}