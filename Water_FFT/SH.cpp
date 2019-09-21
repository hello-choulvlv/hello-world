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

// ����Ҫ��FP32��Cubemap����MRT��ps2.a����ʵ����sm3.0��
// Resolution��ʾ��Ҫ̫��64*64*6=24576���Ѿ�������24576���������ߣ��㹻���ˣ��ֱ��ʹ���Ļ��ᵼ�����ʱ���ر�
HRESULT KSHCubeMap::Init(UINT iBandNum, UINT iSampleNum, char *szPS, UINT iResolution/* = 64*/)
{
	// ���ֻ֧��4Band����������㲻��
	if(iBandNum < MYSH_MINORDER || iBandNum > MYSH_MAXORDER || !iSampleNum || !iResolution || !szPS)
		return D3DERR_INVALIDCALL;
	if(m_bInit)
		return D3DERR_NOTAVAILABLE;

	// ���ͷ�
	Release();

	m_iBandNum = iBandNum;
	m_iTextureNum = (iBandNum*iBandNum) / 4;

	// ����4��ͨ���ģ�Ҫ��һ��
	if((iBandNum*iBandNum) % 4)
		m_iTextureNum++;
	
	if(!m_iTextureNum)
		return D3DERR_INVALIDCALL;


	// ����Shader
	D3DVERTEXELEMENT9 dclr[] = {
		{0,0,D3DDECLTYPE_FLOAT4 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0},
		D3DDECL_END()
	};
	if(FAILED(d3ddevice->CreateVertexDeclaration(dclr, &m_pVertexDeclaration)))
		return E_FAIL;

	if(FAILED(m_PS.InitPixelShader(szPS)))
	{
		OutputDebugString("����ָ�����ļ��Ƿ���GPU Project CubeMap�õ�Pixel Shader��\n");
		return E_FAIL;
	}


	// ����VB�����ǻ���TextureNum������ѣ���XYZRHW�͹��ˣ���Ҫ����PS��VS���Ժ��Ե�
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


	// ����CubeMap
	D3DFORMAT Format = D3DFMT_A32B32G32R32F;
	m_ppCubeMapSH = new LPDIRECT3DCUBETEXTURE9[m_iTextureNum];

	for(UINT i = 0; i < m_iTextureNum; i++)
		if(FAILED(d3ddevice->CreateCubeTexture(iResolution, 1, D3DUSAGE_0, Format, D3DPOOL_MANAGED, &m_ppCubeMapSH[i], NULL)))
		{
			OutputDebugString("����CubeMap������������ֱ��ʣ������2���ݣ�����ܶ�Ӳ����֧�֣�\n");
			return E_FAIL;
		}

	if(FAILED(GenerateSHCubeMap()))
		return E_FAIL;


	// ���������㼰��������
	if(FAILED(m_Samples.Init(iSampleNum, iBandNum)))
		return E_FAIL;
	m_iSampleNum = iSampleNum;
	
	if(FAILED(d3ddevice->CreateTexture(iSampleNum, 1, 1, D3DUSAGE_0, Format, D3DPOOL_MANAGED, &m_pTexSamples, NULL)))
	{
		OutputDebugString("�������������������������������������2���ݣ�����ܶ�Ӳ����֧�֣�\n");
		return E_FAIL;
	}

	if(FAILED(GenerateSampleMap()))
		return E_FAIL;

	// ����RGB����
	if(FAILED(d3ddevice->CreateTexture(m_iTextureNum, 1, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pTexProjectR, NULL)))
		return E_FAIL;
	if(FAILED(d3ddevice->CreateTexture(m_iTextureNum, 1, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pTexProjectG, NULL)))
		return E_FAIL;
	if(FAILED(d3ddevice->CreateTexture(m_iTextureNum, 1, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pTexProjectB, NULL)))
		return E_FAIL;

	// ������ʱ����
	if(FAILED(d3ddevice->CreateTexture(m_iTextureNum, 1, 1, D3DUSAGE_0, Format, D3DPOOL_SYSTEMMEM, &m_pTexSystemMem, NULL)))
		return E_FAIL;

	m_bInit = TRUE;
	return S_OK;
}


// ���Cubemap�õĻص�����
void WINAPI SHCubeFill(D3DXVECTOR4* pOut, CONST D3DXVECTOR3* pTexCoord, CONST D3DXVECTOR3* pTexelSize, LPVOID pData)
{
	UINT *pMonoNum = (UINT *)pData;
	
	// �õ����е�Y
	float *pY = new float[pMonoNum[1]];
	if(!pY)
		return;
	
	static D3DXVECTOR3 VecCoord(0, 0, 0);
	D3DXVec3Normalize(&VecCoord, pTexCoord);
	if(FAILED(D3DXSHEvalDirection(pY, pMonoNum[2], &VecCoord)))
		return;

	// ���ݵ�ǰ����ʽ��д���Ӧ��Y
	float fColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	// ��������Ԥ�������в���4����������������Ļ��ͻ��ó�ʼֵ0
	for(UINT i = 0; i < 4; i++)
	{
		if((i + pMonoNum[0]) >= pMonoNum[1])
			break;
		fColor[i] = pY[i + pMonoNum[0]];
	}
	// R/G/B/A�ֱ��Ӧ0/1/2/3
	*pOut = D3DXVECTOR4(fColor[0], fColor[1], fColor[2], fColor[3]);
	SAFE_DELETE_ARRAY(pY);
}


HRESULT KSHCubeMap::GenerateSHCubeMap()
{
	// ��������һ��������ʾ��ǰ�����ĵ���ʽ�����ڶ�����ʾ�����ʽ������������ʾ��Band��
	UINT iMonoNum[3] = {0, m_iBandNum*m_iBandNum, m_iBandNum};

	for(UINT i = 0; i < m_iTextureNum; i++)
	{
		if(FAILED(D3DXFillCubeTexture(m_ppCubeMapSH[i], SHCubeFill, iMonoNum)))
			return E_FAIL;
		// ÿ���һ��Cube����ǰ�����ĵ���ʽ���ͼ�4
		iMonoNum[0] += 4;
	}

	return S_OK;
}



// ���SampleTexture�õĻص�����
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
	
	// ��������һ��������ʾ�ܲ����������������������������ֵ����
	// ��д���ܲ�������
	*pSampleNum = m_iSampleNum;
	pData++;

	LPD3DXVECTOR3 pDataVector3 = (LPD3DXVECTOR3)pData;

	// ��д������ֵ
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

	// ��Get����Set
	LPDIRECT3DSURFACE9 pOldRT[3] = {NULL, NULL, NULL};
	HRESULT hr = S_OK;
	for(UINT i = 0; i < 3; i++)
	{
		if(FAILED(hr = d3ddevice->GetRenderTarget(i, &pOldRT[i])))
			if(hr != D3DERR_NOTFOUND)
			{
				OutputDebugString("GetMRTʧ�ܣ��������Կ���֧��MRT��\n");
				return E_FAIL;
			}
	}

	// ������Ⱦ���
	if(FAILED(d3ddevice->SetFVF(D3DFVF_XYZRHW)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexDeclaration(m_pVertexDeclaration)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexShader(NULL)))
		return E_FAIL;
	if(FAILED(m_PS.SetPixelShader()))
		return E_FAIL;

	// ����MRT
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

	// ��������
	if(FAILED(d3ddevice->SetTexture(0, pCubeMap)))
	{
		OutputDebugString("SetTextureʧ�ܣ�������ָ����CubeMap������ϵͳ�ڴ����ˣ������Managed��Default��\n");
		return E_FAIL;
	}	// Tex0: ��ͶӰ��Cubemap

	if(FAILED(d3ddevice->SetTexture(1, m_pTexSamples)))
		return E_FAIL;	// Tex1: ������������

	// ������Ⱦ����
	if(FAILED(d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE)))
		return E_FAIL;	// ƽ������Ҫ�ȹص�Z

	// ���ó����Ĵ���
	// c0 0��	���ؿ���ϵ��4Pi/N	1/��������
	// i0 ��������
	m_PS.SetConstant(0, &D3DXVECTOR4(	0.0f,	4.0f*D3DX_PI/(float)m_iSampleNum,	1.0f/((float)m_iSampleNum-1.0f),	0.0f	), 1);

	// ��Ϊһ��ѭ�����ֻ֧��255�Σ�������128��ѭ��Ϊ׼��������ѭ��
	int iConstReg1[4] = {(int)(m_iSampleNum>128 ? 128 : m_iSampleNum), 0, 1, 0};
	int iConstReg2[4] = {(int)m_iSampleNum / 128, 0, 1, 0};

	m_PS.SetConstantI(0, iConstReg1, 1);
	m_PS.SetConstantI(1, iConstReg2, 1);


	// ѭ����Ⱦ
	if(FAILED(d3ddevice->BeginScene()))
		return E_FAIL;

	for(int i = 0; i < m_iTextureNum; i++)
	{
		if(FAILED(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iVBStride)))
			return E_FAIL;

		d3ddevice->SetTexture(2, m_ppCubeMapSH[i]);	// ��2�㣺��ǰ��SH����

		// ��Ⱦ
		if(FAILED(d3ddevice->DrawPrimitive(D3DPT_POINTLIST, i, 1)))
			return E_FAIL;
	}

	if(FAILED(d3ddevice->EndScene()))
		return E_FAIL;


	// ��ȡ��Ⱦ�õ�SHϵ��
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


	// �ָ���ǰ�������Ⱦ����
	m_PS.RestorePixelShader();

	if(FAILED(d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE)))
		return E_FAIL;

	// ע������������½�����������MRT̫�ֲ��ˣ���ֵ���ǰ����ôû�½����ٰ�
	for(UINT i = 0; i < 3; i++)
	{
		if(FAILED(d3ddevice->SetRenderTarget(i, pOldRT[i])))
			return E_FAIL;
		SAFE_RELEASE(pOldRT[i]);
	}



	// ����
	SAFE_RELEASE(pSurfR);
	SAFE_RELEASE(pSurfG);
	SAFE_RELEASE(pSurfB);
	SAFE_RELEASE(pSurfSysMem);

	return S_OK;
}









// ���������ԺͶ�Ӧ��2D�������õ���Ӧ��Cubemap���꣨�������ߣ�
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


// ���ݲ��������ҵ�CubeMap��Ӧ�Ľ��㴦��������Ӧ����������Get/Set��BYTEָ������ȷ����һ��������ռ�Ŀռ䣩�����ұ���Ԥ�ȸ���Formatָ�����ش�С���ֽڣ���Ĭ��ΪA8R8G8B8
// ��һ��������ʾGet/Set�������ڲ�������ͨ��Get/SetCubeMapPixelData������
HRESULT KSHCubeMap::OperateCubeMapPixelData(BOOL bGet, LPDIRECT3DCUBETEXTURE9 pCubeMap, D3DXVECTOR3 VecRay, void *pPixelData, UINT iPixelSize /* = 4 */, UINT iMip /* = 0 */)
{
	if( !pCubeMap || !pPixelData || D3DXVec3Length(&VecRay) == 0.0f || iMip > 7 || !iPixelSize)
		return D3DERR_INVALIDCALL;

	D3DXVec3Normalize(&VecRay, &VecRay);

	// �Ȱ������߷���õ��ĸ��漰��Ӧ��2D��������
	D3DCUBEMAP_FACES iFace = D3DCUBEMAP_FACE_FORCE_DWORD;
	// ����6���棬��һ����λ���ӣ����궼��-0.5��0.5
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

	float fDelta = 0.00001f;	// ��һ��С��������ȷ������Ƚϲ�������ȷ��

	// ����ǰ���ж������ߵĺϷ��ԣ������������ض�������ƽ���ཻ����ֻ��������һ�����ཻ�������з�Χ���Ƶģ�
	int i;
	for( i = 0; i < 6 ; i++)
	{
		if(GetIntersectPlane(D3DXVECTOR3(0, 0, 0), VecRay, pPlanes[i], &PtIntersect))
		{
			// ������Ҫ�жϣ������Ƿ����������ڶ�Ӧ��ķ�Χ֮�ڣ���Ϊƽ����û�з�Χ����ģ��������ﻹҪ����Χ�ж�
			// �����������������ʱ��Ҫ��-0.5��0.5ת��Ϊ0��1�����⿼�ǵ���Щ���ϵ�V���ʵ�ʵ�XYZ�����᷽���෴�����Ի���Ҫȡ��
			// ������ʮ��������ʱ����Ч�Էֱ�ƽ����ÿ�����ϣ�ÿ����ֻ�ж��ڸ�2D����Һ��±���Ч�����������+/-0.5��ʱ��
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

	// û�к��κ�һ�����ཻ�����������ܵģ�һ�����ڲ�����
	if(i > 5)
		return E_FAIL;

	iFace = D3DCUBEMAP_FACES(i);

	// �õ�����Ϣ����λ��2D��������
	D3DSURFACE_DESC Desc;
	if(FAILED(pCubeMap->GetLevelDesc(iMip, &Desc)))
		return E_FAIL;
	
	float fX = TexCoord.x * (float)(Desc.Width-1);
	float fY = TexCoord.y * (float)(Desc.Height-1);
	// ����ǧ����ֱ��ȡ������Ȼ�ᶪʧС��β��
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

	// ������CubeMap���Ǹɾ�����Surface���ò��ţ�������ҪRelease
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

		// color, clamp to 0��1
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
// Optimization��Pre-Compute SH(GenerateSH) and save to file
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

				// color, clamp to 0��1
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

				// color, clamp to 0��1
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