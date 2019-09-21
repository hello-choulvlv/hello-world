#include "Myd3d.h"
#include "Shader.h"
#include "ProcXFile.h"
#include "MeshAnim.h"




HRESULT g_SplitHDRCrossTexture(LPSTR szCubeFileName, DWORD dwDimension)
{
	if(!szCubeFileName || !dwDimension)
		return D3DERR_INVALIDCALL;
	
#ifdef USE_FP16
	D3DFORMAT Format = D3DFMT_A16B16G16R16F;
#elif defined USE_FP32
	D3DFORMAT Format = D3DFMT_A32B32G32R32F;
#else
	D3DFORMAT Format;
	OutputDebugString("使用HDR函数必须预先定义FP16/FP32！");
	return E_FAIL;
#endif
	
	OutputDebugString("分割整合的HDR CubeMap，记得posz.hdr要用HDRShop旋转180度，就是把Y轴倒过来");
	
	LPDIRECT3DTEXTURE9 pCubeTex = NULL, pTexture[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
	LPDIRECT3DSURFACE9 pSrcSurf = NULL, pDstSurf = NULL;
	int i = 0;
	char *p = NULL;
	
	// 创建CubeMap临时纹理
	V_RETURN(D3DXCreateTextureFromFileEx(d3ddevice, szCubeFileName, D3DX_DEFAULT, D3DX_DEFAULT, 1, D3DUSAGE_0, Format, D3DPOOL_DEFAULT, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, NULL, NULL, &pCubeTex))
		
		// 开始查找文件名，根据文件名（neg pos xyz）来分割不同的区域
		char *pKeyName[] = {"posx", "negx", "posy", "negy", "posz", "negz"};
	RECT Rect[6] = 
	{
		// X
		{dwDimension*2, dwDimension, dwDimension*3, dwDimension*2},	// 第二排第三个
		{0, dwDimension, dwDimension, dwDimension*2},		// 第二排第一个
		
		// Y
		{dwDimension, 0, dwDimension*2, dwDimension},		// 第一排第二个
		{dwDimension, dwDimension*2, dwDimension*2, dwDimension*3},	// 第三排第二个
		
		// Z
		{dwDimension, dwDimension*3, dwDimension*2, dwDimension*4},	// 第四排第二个
		{dwDimension, dwDimension, dwDimension*2, dwDimension*2},	// 第二排第二个
	};
	
	
	char szFileName[100];

	for(i=0; i<6; i++)
	{
		// 创建纹理
		if(FAILED(d3ddevice->CreateTexture(dwDimension, dwDimension, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &pTexture[i], NULL)))
			goto fail;
		
		// 复制区域
		if(FAILED(pTexture[i]->GetSurfaceLevel(0, &pDstSurf)))
			goto fail;
		if(FAILED(pCubeTex->GetSurfaceLevel(0, &pSrcSurf)))
			goto fail;
		
		if(FAILED(d3ddevice->StretchRect(pSrcSurf, &Rect[i], pDstSurf, NULL, D3DTEXF_POINT)))
			goto fail;
		
		SAFE_RELEASE(pSrcSurf);
		SAFE_RELEASE(pDstSurf);
		
		sprintf(szFileName, "%s.hdr", pKeyName[i]);
		D3DXSaveTextureToFile(szFileName, D3DXIFF_HDR, pTexture[i], NULL);
		SAFE_RELEASE(pTexture[i]);
	}
	
	SAFE_RELEASE(pCubeTex);
	return S_OK;
	
fail:
	SAFE_RELEASE(pCubeTex);
	for(i=0; i<6; i++)
		SAFE_RELEASE(pTexture[i]);
	SAFE_RELEASE(pSrcSurf);
	SAFE_RELEASE(pDstSurf);
	return E_FAIL;
}








/********************************Normal Mesh*****************************/
NORMALMESH::NORMALMESH()
{
	Material=NULL; Texture=NULL; Mesh=NULL;
	SubsetNum=0;	m_dwAttribTableNum = 0;
	m_pDeclaration = NULL;
	m_pAttribTable = NULL;
}

NORMALMESH::~NORMALMESH()
{
	Release();
}


void NORMALMESH::Release()
{
	DWORD i = 0;
	//释放每个子集的纹理
	for(i=0; i<SubsetNum; i++) if(Texture[i]) SAFE_RELEASE(Texture[i]);
	//释放纹理指针数组
	if(Texture) delete[] Texture;
	//释放材质数组
	if(Material) delete[] Material;
	//释放顶点缓存
	if(Mesh) Mesh->Release();
	Mesh = NULL; Texture = NULL; Material = NULL;

	SAFE_RELEASE(m_pDeclaration);
	SAFE_DELETE_ARRAY(m_pAttribTable);
	SubsetNum = 0;	m_dwAttribTableNum = 0;
}

HRESULT NORMALMESH::GetBindVertexInfo(LPVERTEXBINDINFO pVertexBindInfo, UINT iVertexNo)
{
	if(!Mesh || !SubsetNum || !d3ddevice)
		return E_FAIL;
	if(iVertexNo >= Mesh->GetNumVertices() || !pVertexBindInfo)
		return D3DERR_INVALIDCALL;

	// 得到该顶点的坐标
	char *pSrcData = NULL;
	float *pData = NULL;
	DWORD dwStride = Mesh->GetNumBytesPerVertex();
	if(FAILED(Mesh->LockVertexBuffer(D3DLOCK_READONLY, (void **)&pSrcData)))
		return E_FAIL;

	pSrcData += iVertexNo * dwStride;
	pData = (float *)pSrcData;

	pVertexBindInfo->dwVertexNo = iVertexNo;
	pVertexBindInfo->PtPosition = D3DXVECTOR3(*pData, *(pData+1), *(pData+2));
	Mesh->UnlockVertexBuffer();

	return S_OK;
}



HRESULT NORMALMESH::LoadFromFile(LPSTR pFileName, LPSTR pPathName, D3DFORMAT Format)
{	
	LPD3DXBUFFER materialbuffer;
	LPD3DXMATERIAL materialstruct;  //其实这个就指向上面的MATERIALBUFFER里的数据
	//LPD3DXBUFFER adjacency;   //表示平面邻接关系
	
	DWORD i;
	LPD3DXBUFFER pAdjancy = NULL;
	//从磁盘中读取X模型文件
	if(!Mesh && !Material && !Texture && !SubsetNum && pFileName && d3ddevice)
	{
		if(FAILED(D3DXLoadMeshFromX(pFileName, D3DXMESH_MANAGED, d3ddevice, &pAdjancy, &materialbuffer, NULL, &SubsetNum, &Mesh)))
			return E_FAIL;
		// 千万小心VertexCache和StripeOrder优化，在不同的显卡上优化出来的Mesh都不同
		if(FAILED(Mesh->OptimizeInplace(D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT/* | D3DXMESHOPT_VERTEXCACHE*/, (DWORD *)pAdjancy->GetBufferPointer(), NULL, NULL, NULL)))
			return E_FAIL;

		if(FAILED(Mesh->GetAttributeTable(NULL, &m_dwAttribTableNum)))
			return E_FAIL;
		m_pAttribTable = new D3DXATTRIBUTERANGE[m_dwAttribTableNum];
		if(FAILED(Mesh->GetAttributeTable(m_pAttribTable, NULL)))
			return E_FAIL;
	}
	else
		return D3DERR_INVALIDCALL;

		Material=new D3DMATERIAL9[SubsetNum];
		Texture=new LPDIRECT3DTEXTURE9[SubsetNum];
		ZeroMemory(Texture, sizeof(LPDIRECT3DTEXTURE9) * SubsetNum);
		materialstruct=(D3DXMATERIAL*)materialbuffer->GetBufferPointer();
		
		// 转换为Bin类型的X文件，可以做简单加密用，也可以减小文件容量（基本是TEXT类型的1/4），提高读取速度
		//D3DXSaveMeshToX("SaveMeshFile_Binary.x", Mesh, NULL, materialstruct, NULL, SubsetNum, D3DXF_FILEFORMAT_BINARY);

		char pPureFileName[100];
		
		for(i=0;i<SubsetNum;i++)
		{	//保存材质信息
			Material[i]=materialstruct[i].MatD3D;
			Material[i].Ambient=Material[i].Diffuse;  //D3D奇怪的设计，不自动将漫射光赋值，还得自己动手
			
			//根据BUFFER中的纹理文件信息创建纹理，如果纹理文件不存在就把指针置空
			if(pPathName)
			{
				// 如果自定义纹理路径，就把材质纹理文件名中的路径全去掉，只保留纯文件名，再和自定义路径混合
				if(!(materialstruct[i].pTextureFilename))
				{
					pPureFileName[0] = '\0';
				}
				else
				{
					char *p = strrchr(materialstruct[i].pTextureFilename, '\\');
					if(p)
						sprintf(pPureFileName, "%s", p+1);
					else
						sprintf(pPureFileName, "%s", materialstruct[i].pTextureFilename);
				}

				sprintf(m_pTextureFileName[i], "%s\\%s", pPathName, pPureFileName);
			}
			else
				sprintf(m_pTextureFileName[i], "%s", materialstruct[i].pTextureFilename);
			
			// 自定义纹理格式生效
			if(Format != D3DFMT_A8R8G8B8)
			{
				if(FAILED(D3DXCreateTextureFromFileEx(d3ddevice, m_pTextureFileName[i], D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, D3DUSAGE_0, Format, D3DPOOL_DEFAULT, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, NULL, NULL, &Texture[i])))
					Texture[i]=NULL;
			}
			else
			{
				if(FAILED(D3DXCreateTextureFromFile(d3ddevice, m_pTextureFileName[i], &Texture[i])))
					Texture[i]=NULL;
			}
		}
		
		SAFE_RELEASE(materialbuffer);     //已将材质信息转存到material中，释放临时材质缓存
		
		D3DVERTEXELEMENT9 dclr[MAX_FVF_DECL_SIZE];
		if(FAILED(D3DXDeclaratorFromFVF(Mesh->GetFVF(), dclr)))
			return E_FAIL;
		if(FAILED(d3ddevice->CreateVertexDeclaration(dclr, &m_pDeclaration)))
			return E_FAIL;
		return S_OK;
}




HRESULT NORMALMESH::SetUserTexture(DWORD SubsetNo, LPSTR UserTextureFilename)
{
	if(SubsetNo>=SubsetNum) return E_FAIL;
	//要已分配，就释放
	if(Texture[SubsetNo]) {Texture[SubsetNo]->Release(); Texture[SubsetNo]=NULL; }
	if(FAILED(D3DXCreateTextureFromFile(d3ddevice,UserTextureFilename,&Texture[SubsetNo])))
		return E_FAIL;
	return S_OK;
}



HRESULT NORMALMESH::DrawAll(bool bAutoDisable /* = true */, LPDIRECT3DVERTEXDECLARATION9 pDeclaration /* = NULL */, LPDIRECT3DVERTEXSHADER9 pVS /* = NULL */)
{
	HRESULT hr = S_OK;
	if(Mesh && m_dwAttribTableNum)
	{
		LPDIRECT3DVERTEXBUFFER9 pVB = NULL;
		LPDIRECT3DINDEXBUFFER9 pIB = NULL;
		if( FAILED(Mesh->GetVertexBuffer(&pVB)) || FAILED(Mesh->GetIndexBuffer(&pIB)) )
		{
			SAFE_RELEASE(pVB);
			SAFE_RELEASE(pIB);
			return E_FAIL;
		}

		DWORD dwSubsetID = 0;

		hr = d3ddevice->BeginScene();

		for(DWORD i=0;i<m_dwAttribTableNum;i++)
		{
			dwSubsetID = m_pAttribTable[i].AttribId;

			//如果纹理读取失败或未创建，参数为真表示强制设置，即禁用纹理，为假则表示忽略，用于自定义贴图，如CUBEMAP）
			if(Texture)
			{
				if(bAutoDisable)
					d3ddevice->SetTexture(0,Texture[dwSubsetID]);
				if(Texture[dwSubsetID] && !bAutoDisable)
					d3ddevice->SetTexture(0,Texture[dwSubsetID]);
			}
			
			if(Material)
				d3ddevice->SetMaterial(&Material[dwSubsetID]);
/*
			//开始渲染
			d3ddevice->BeginScene();
			if(FAILED(Mesh->DrawSubset(i))) return E_FAIL;
			d3ddevice->EndScene();
*/
			// 如果是FVF
			if(!pDeclaration)
			{
				SetTextureColorMix(0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
				hr = d3ddevice->SetFVF(Mesh->GetFVF());
				hr = d3ddevice->SetVertexDeclaration(m_pDeclaration);
				hr = d3ddevice->SetVertexShader(NULL);
				hr = d3ddevice->SetPixelShader(NULL);

			}
			else
			{
				hr = d3ddevice->SetFVF(0);
				hr = d3ddevice->SetVertexDeclaration(pDeclaration);
				hr = d3ddevice->SetVertexShader(pVS);
			}

			// 开始绘制
			if( FAILED( d3ddevice->SetStreamSource(0, pVB, 0, Mesh->GetNumBytesPerVertex()) ))
				return E_FAIL;
			hr = d3ddevice->SetIndices(pIB);
		
			if( FAILED( d3ddevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, Mesh->GetNumVertices(), m_pAttribTable[i].FaceStart*3, m_pAttribTable[i].FaceCount) ) )
				return E_FAIL;

		}// end for draw attribute subset
		
		hr = d3ddevice->EndScene();


			SAFE_RELEASE(pVB);
			SAFE_RELEASE(pIB);
	}// end if meshdata is valid
		return hr;
}











/*************************骨骼动画*************************/
SKINMESH::SKINMESH()
{
	m_CreateAttrib = 0;
	m_pMesh = NULL;
	m_pFrame = NULL;
	m_pRootMesh = NULL;
	m_pRootFrame = NULL;
	m_fFPS = 1/(float)20;
	StartTime.QuadPart = 0;
	CurrentTime.QuadPart = 0;
	LastTime.QuadPart = 0;
	m_bLoop = TRUE;
	m_bInverse = FALSE;
	m_fSpeed = 1;
	m_pVertexShader = NULL;
	m_iStartConstantIndex = 0;
}

void SKINMESH::Release()
{
	if(!m_CreateAttrib)
		return;
	m_CreateAttrib = 0;
	m_pMesh = NULL;
	m_pFrame = NULL;
	m_fFPS = 1/(float)20;
	StartTime.QuadPart = 0;
	CurrentTime.QuadPart = 0;
	LastTime.QuadPart = 0;
	m_bLoop = TRUE;
	m_bInverse = FALSE;
	m_fSpeed = 1;
	m_pVertexShader = NULL;
	m_iStartConstantIndex = 0;
	SkinAnimation.Release();
	ParseFrame.Release();


	//ParseFrame.ReleaseMeshContainer(m_pRootMesh);
	delete m_pRootMesh;
	m_pRootMesh = NULL;
	//ParseFrame.ReleaseFrame(m_pRootFrame);
	delete m_pRootFrame;
	m_pRootFrame = NULL;
}

void SKINMESH::Reset()
{
	StartTime.QuadPart = 0;
	LastTime.QuadPart = 0;
	CurrentTime.QuadPart = 0;
}

void SKINMESH::Set(DWORD dwFPS, float fSpeed, BOOL bLoop, BOOL bInverse)
{
	m_fFPS = 1 / (float)dwFPS;
	if(fSpeed >= 0)
		m_fSpeed = fSpeed;
	m_bLoop = bLoop;
	m_bInverse = bInverse;
}

DWORD SKINMESH::GetElapseTime()
{
	return DWORD((float)(CurrentTime.QuadPart - StartTime.QuadPart) / (float)PerformanceFrequency.QuadPart * m_fSpeed * 1000);
}

HRESULT SKINMESH::LoadFromX(LPSTR pFileName, char *pTexturePath, DWORD NewFVF, DWORD LoadFlags)
{
	if(m_CreateAttrib)
		return S_OK;

	if(FAILED(ParseFrame.LoadFromX(&m_pRootMesh, &m_pRootFrame, pFileName, pTexturePath, NewFVF, LoadFlags)))
		return E_FAIL;
	m_pFrame = m_pRootFrame;
	m_pMesh = m_pRootMesh;

	if(FAILED(SkinAnimation.LoadFromX(pFileName)))
		return E_FAIL;

	m_CreateAttrib = 1;
	return S_OK;
}


HRESULT SKINMESH::GetBindVertexInfo(LPVERTEXBINDINFO pVertexBindInfo, UINT iVertexNo)
{
	if(!m_CreateAttrib || !m_pMesh)
		return E_FAIL;
	if(!m_pMesh->pSkinInfo || !m_pMesh->MeshData.pMesh)
		return E_FAIL;
	if(!pVertexBindInfo || iVertexNo >= m_pMesh->MeshData.pMesh->GetNumVertices())
		return D3DERR_INVALIDCALL;

	extern D3DXMATRIX g_Matrix1, g_Matrix2, g_MatrixTrans;
	extern float g_fWeight1, g_fWeight2;

	DWORD dwBoneNum = m_pMesh->pSkinInfo->GetNumBones();

	UINT iVertexMatrixNum = 0;
	DWORD pMatIndex[100];
	float pWeight[100];
	DWORD dwTemp = 0, dwTemp1 = 0;
	UINT k = 0;
	m_pMesh->pSkinInfo->GetMaxVertexInfluences(&dwTemp);
	for(UINT j=0; j<dwBoneNum; j++)
	{
		if(S_OK == m_pMesh->pSkinInfo->FindBoneVertexInfluenceIndex(j, iVertexNo, &dwTemp))
		{
			pMatIndex[iVertexMatrixNum] = j;
			if(FAILED( m_pMesh->pSkinInfo->GetBoneVertexInfluence(j, dwTemp, &pWeight[iVertexMatrixNum], &dwTemp1)))
				OutputDebugString("警告，指定顶点的骨骼影响数据有误！");
			iVertexMatrixNum++;
		}
	}

	// 先将物体变换到人物顶点所在位置
	char *SrcPtr = NULL;
	if(FAILED(m_pMesh->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&SrcPtr)))
		return E_FAIL;
	SrcPtr += iVertexNo * m_pMesh->MeshData.pMesh->GetNumBytesPerVertex();

	float *pData = (float *)SrcPtr;
	D3DXVECTOR3 VecPosition = D3DXVECTOR3( *pData, *(pData+1), *(pData+2) );

	m_pMesh->MeshData.pMesh->UnlockVertexBuffer();

	// 查找结束，得到该点对应的总矩阵数、矩阵序号和混合系数，写入结构体
	pVertexBindInfo->dwBoneNum = iVertexMatrixNum;
	pVertexBindInfo->dwVertexNo = iVertexNo;
	pVertexBindInfo->PtPosition = VecPosition;
	for(int j=0; j<iVertexMatrixNum; j++)
	{
		pVertexBindInfo->fWeight[j] = pWeight[j];
		pVertexBindInfo->pMatIndex[j] = pMatIndex[j];
	}
	return S_OK;
}


HRESULT SKINMESH::GetBindVertexBoneMatrix(LPVERTEXBINDINFO pVertexBindInfo, LPD3DXMATRIX pMatBoneCombine)
{
	if(!pVertexBindInfo || !pMatBoneCombine)
		return D3DERR_INVALIDCALL;
	if(!m_pMesh)
		return E_FAIL;
	if(!m_pMesh->pSkinInfo)
		return E_FAIL;

	D3DXMATRIX MatTemp1, MatTemp2;

	ZeroMemory(pMatBoneCombine, sizeof(D3DXMATRIX));
	for(UINT j=0; j<pVertexBindInfo->dwBoneNum; j++)
	{
		MatTemp1 = *m_pMesh->pSkinInfo->GetBoneOffsetMatrix(pVertexBindInfo->pMatIndex[j]);
		MatTemp2 = *m_pMesh->ppFrameMatrices[pVertexBindInfo->pMatIndex[j]];
		*pMatBoneCombine += MatTemp1 * MatTemp2 * pVertexBindInfo->fWeight[j];
	}
	return S_OK;
}


HRESULT SKINMESH::Draw(LPSTR pAnimationSetName, UINT iAnimationSetIndex)
{
	if(!m_CreateAttrib)
		return E_FAIL;
	
		// 得到起始时间和当前时间之差（ms），并且判断是否满足FPS条件，不满足就直接画（不更新矩阵）
	if(LastTime.QuadPart == 0)
		QueryPerformanceCounter(&LastTime);
	if(StartTime.QuadPart == 0)
		QueryPerformanceCounter(&StartTime);
	
	QueryPerformanceCounter(&CurrentTime);

	float fTime = (float)(CurrentTime.QuadPart - LastTime.QuadPart) / (float)PerformanceFrequency.QuadPart;
	if(fTime >= (float)m_fFPS)
	{
		// 设置
		DWORD dwTime;
		if(m_bInverse) dwTime= (DWORD)((float)(StartTime.QuadPart - CurrentTime.QuadPart) / (float)PerformanceFrequency.QuadPart * m_fSpeed * 1000);
		else dwTime= (DWORD)((float)(CurrentTime.QuadPart - StartTime.QuadPart) / (float)PerformanceFrequency.QuadPart * m_fSpeed * 1000);
		QueryPerformanceCounter(&LastTime);
		SkinAnimation.Map(pAnimationSetName, iAnimationSetIndex, m_pFrame);
		SkinAnimation.SetAnimation(pAnimationSetName, iAnimationSetIndex, dwTime, m_bLoop);
		ParseFrame.UpdateHierarchy(m_pFrame, NULL);
		ParseFrame.UpdateMesh(m_pMesh);
	}

	// 开始绘制
	d3ddevice->BeginScene();
	for(UINT i=0; i<m_pMesh->NumMaterials; i++)
	{
		d3ddevice->SetMaterial(&(m_pMesh->pMaterials)[i].MatD3D);
		d3ddevice->SetTexture(0, m_pMesh->pTextures[i]);
		//d3ddevice->SetVertexDeclaration(NULL);
		d3ddevice->SetVertexShader(NULL);
		d3ddevice->SetPixelShader(NULL);

		SetTextureColorMix(0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
		SetTextureColorMix(1, D3DTOP_DISABLE, D3DTA_TEXTURE, D3DTA_DIFFUSE);

		// SkinMesh就画pSKinMesh，反之就画MeshData
		if(m_pMesh->pSkinInfo)
			m_pMesh->pSkinMesh->DrawSubset(i);
		else
			m_pMesh->MeshData.pMesh->DrawSubset(i);
	}
	d3ddevice->EndScene();
	
	return S_OK;
}



HRESULT SKINMESH::SetVS(UINT iStartConstantIndex, VERTEXSHADER *pVS, D3DXMATRIX *pMatWorld)
{
	if(iStartConstantIndex>256 || !pVS || !m_CreateAttrib)
		return E_FAIL;
	m_iStartConstantIndex = iStartConstantIndex;
	m_pVertexShader = pVS;

	if(pMatWorld)
		m_matWorld = *pMatWorld;
	else
		D3DXMatrixIdentity(&m_matWorld);

	return S_OK;
}

HRESULT SKINMESH::DrawVS(LPSTR pAnimationSetName, UINT iAnimationSetIndex)
{
	// 有效性判断
	if(m_iStartConstantIndex>256 || !m_pFrame || !m_pMesh || !m_pMesh->pSkinInfo || !m_pMesh->pSplitMesh || !m_pMesh->dwMaxMatrixNum || m_CreateAttrib!=2 )
		return E_FAIL;
	
	// 得到起始时间和当前时间之差（ms），并且判断是否满足FPS条件，不满足就直接画（不更新矩阵）
	if(LastTime.QuadPart == 0)
		QueryPerformanceCounter(&LastTime);
	if(StartTime.QuadPart == 0)
		QueryPerformanceCounter(&StartTime);
	
	QueryPerformanceCounter(&CurrentTime);

	float fTime = (float)(CurrentTime.QuadPart - LastTime.QuadPart) / (float)PerformanceFrequency.QuadPart;
	if(fTime >= (float)m_fFPS)
	{
		// 设置
		DWORD dwTime;
		if(m_bInverse) dwTime= (DWORD)((float)(StartTime.QuadPart - CurrentTime.QuadPart) / (float)PerformanceFrequency.QuadPart * m_fSpeed * 1000);
		else dwTime= (DWORD)((float)(CurrentTime.QuadPart - StartTime.QuadPart) / (float)PerformanceFrequency.QuadPart * m_fSpeed * 1000);
		QueryPerformanceCounter(&LastTime);
		SkinAnimation.Map(pAnimationSetName, iAnimationSetIndex, m_pFrame);
		SkinAnimation.SetAnimation(pAnimationSetName, iAnimationSetIndex, dwTime, m_bLoop);
		ParseFrame.UpdateHierarchy(m_pFrame, NULL);
	}


	// 开始绘制
	SplitMesh *pSplitMesh = m_pMesh->pSplitMesh;
	D3DXVECTOR4 IndexConstant = D3DXVECTOR4(4, 4, 4, (float)m_iStartConstantIndex);
	UINT x=0;
	while(pSplitMesh)
	{
		d3ddevice->SetMaterial(&((m_pMesh->pMaterials)[pSplitMesh->dwSubset].MatD3D));
		d3ddevice->SetTexture(0, m_pMesh->pTextures[pSplitMesh->dwSubset]);

		d3ddevice->SetStreamSource(0, pSplitMesh->pVB, 0, pSplitMesh->dwVertexSize);
		d3ddevice->SetStreamSource(1, pSplitMesh->pDB, 0, pSplitMesh->dwDataVertexSize);
		d3ddevice->SetIndices(pSplitMesh->pIB);
		d3ddevice->SetFVF(NULL);

		d3ddevice->SetRenderState(D3DRS_LIGHTING, TRUE);
		SetTextureColorMix(0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
		SetTextureColorMix(1, D3DTOP_DISABLE, D3DTA_TEXTURE, D3DTA_CURRENT);
				
		m_pVertexShader->SetTransform(&m_matWorld);
		m_pVertexShader->SetConstant(73, &IndexConstant, 1);
		
		SetMatrixData(m_iStartConstantIndex, pSplitMesh, m_pMesh);
		
		m_pVertexShader->DrawIndexPrimitive(0, pSplitMesh->dwFaceNum, pSplitMesh->dwVertexNum);
		//m_pVertexShader->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, pSplitMesh->dwFaceNum/2);

		pSplitMesh = pSplitMesh->pNextMesh;
	}
	
	return S_OK;
}

















BOOL CheckAllFacesHaveBeenSaved(char *pFaces, DWORD dwFaceNum)
{
	for(DWORD i=0; i<dwFaceNum; i++)
		if(pFaces[i] == 0)
			return FALSE;
	return TRUE;
}
//输入一个原MESH顶点索引的值，在数组中搜索，返回在SPLITMESH中对应顶点的索引号
unsigned short int GetVertexMapping(DWORD *pSplitMeshVertex, DWORD dwVertexNum, DWORD dwVertexValue)
{
	for(unsigned short int i=0; i<dwVertexNum; i++)
		if(pSplitMeshVertex[i] == dwVertexValue)
			return i;
	return 65535;
}


HRESULT SKINMESH::SplitMeshVS(D3DXMESHCONTAINER_EX *pMesh, DWORD dwMaxMatrixNum)
{
	if(pMesh == NULL)
		pMesh = m_pMesh;
	if(m_CreateAttrib == 2)
		return S_OK;
	if(!dwMaxMatrixNum || dwMaxMatrixNum>30)
		return D3DERR_INVALIDCALL;

	if(!pMesh || !pMesh->pSkinInfo || !pMesh->pSkinMesh || !m_CreateAttrib )
		return E_FAIL;
	
	DWORD i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, x = 0, y = 0, z = 0, temp = 0, temp1 = 0;
	// 将dwMaxMatrixNum取能被4整除的最小值，用于填充索引和权重（Data Stream）
	DWORD dwMaxMatrixModNum = dwMaxMatrixNum + ((dwMaxMatrixNum%4) ? (4-dwMaxMatrixNum%4) : 0);
	char *p = NULL, *p1 = NULL; //临时使用，用于填充缓冲区
	
	// 如果已分割过，就释放掉，重新分割
	if(pMesh->dwSplitMeshNum || pMesh->pSplitMesh)
	{
		delete pMesh->pSplitMesh;
		pMesh->pSplitMesh = NULL;
		pMesh->dwSplitMeshNum = 0;
		pMesh->dwMaxMatrixNum = 0;
	}
	
	// 先得到索引缓冲中的数据，在pSource中待用
	LPDIRECT3DINDEXBUFFER9 pIB;
	D3DINDEXBUFFER_DESC Desc;
	char *pHeadSource;
	pMesh->pSkinMesh->GetIndexBuffer(&pIB);
	pIB->GetDesc(&Desc);
	pIB->Lock(0, Desc.Size, (void **)&pHeadSource, D3DLOCK_READONLY);


	// 得到VB中的顶点数
	DWORD dwVertexNum = pMesh->pSkinMesh->GetNumVertices();
	
	// 得到每个骨头所影响的顶点数据
	DWORD dwBoneNum = pMesh->pSkinInfo->GetNumBones();
	DWORD **pBoneVertics = new DWORD* [dwBoneNum];
	ZeroMemory(pBoneVertics, sizeof(DWORD *) * dwBoneNum);
	float **pWeights = new float* [dwBoneNum];
	ZeroMemory(pWeights, sizeof(float *) * dwBoneNum);
	DWORD *CurrentVertexNum = new DWORD[dwBoneNum];        // 每个骨头所影响的顶点数

	for(i=0; i<dwBoneNum; i++)
	{
		CurrentVertexNum[i] = pMesh->pSkinInfo->GetNumBoneInfluences(i);
		pBoneVertics[i] = new DWORD[CurrentVertexNum[i]];
		pWeights[i] = new float[CurrentVertexNum[i]];
		pMesh->pSkinInfo->GetBoneInfluence(i, pBoneVertics[i], pWeights[i]);
	}
	
	// 排序骨头影响的顶点数，把顶点数最多的骨头排到前面（从大到小排列），只用于第一种分割，这步的优化很重要
	// pBoneOptimizedMapping[i]等于排第i位（从大到小排列）的骨头所对应的实际位置
	DWORD *pBoneOptimizedMapping = new DWORD[dwBoneNum];
	for(i=0; i<dwBoneNum; i++)
		pBoneOptimizedMapping[i] = i;
	
	DWORD *pOptimizedCurrentVertexNum = new DWORD[dwBoneNum];
	memcpy(pOptimizedCurrentVertexNum, CurrentVertexNum, dwBoneNum*sizeof(DWORD));

	for(j=0; j<dwBoneNum-1; j++)
		for(i=1; i<dwBoneNum-j; i++)
			if(pOptimizedCurrentVertexNum[i] > pOptimizedCurrentVertexNum[i-1])
			{
				temp = pOptimizedCurrentVertexNum[i];
				pOptimizedCurrentVertexNum[i] = pOptimizedCurrentVertexNum[i-1];
				pOptimizedCurrentVertexNum[i-1] = temp;

				temp = pBoneOptimizedMapping[i];
				pBoneOptimizedMapping[i] = pBoneOptimizedMapping[i-1];
				pBoneOptimizedMapping[i-1] = temp;
			}
			
	// 得到每个面的属性数据
	DWORD dwFaceNum = pMesh->pSkinMesh->GetNumFaces();
	DWORD *pFaceID = new DWORD[dwFaceNum];
	
	    // 得到属性表
	DWORD dwAttributeTableNum = 0;
	pMesh->pSkinMesh->GetAttributeTable(NULL, &dwAttributeTableNum);
	D3DXATTRIBUTERANGE *pAttributeTable = new D3DXATTRIBUTERANGE[dwAttributeTableNum];
	ZeroMemory(pAttributeTable, sizeof(D3DXATTRIBUTERANGE) * dwAttributeTableNum);

	pMesh->pSkinMesh->GetAttributeTable(pAttributeTable, NULL);
		// 开始将属性表的内容填充到面属性中去
	for(i=0; i<dwAttributeTableNum; i++)
	{
		for(j=pAttributeTable[i].FaceStart; (j-pAttributeTable[i].FaceStart)<pAttributeTable[i].FaceCount; j++)
			pFaceID[j] = pAttributeTable[i].AttribId;
	}

	delete [] pAttributeTable;
	pAttributeTable = NULL;

	// 开始扫描每个面，符合要求的话就加入SplitMesh
	DWORD dwFaceMatrixNum;  // 当前处理的面需要的矩阵（骨骼）数量
	char *pFaceSign = new char[dwFaceNum];   // 如果某个序号的面已存，则对应序号的该数组元素记为1，用来快速选择本次使用哪些面
	memset(pFaceSign, 0, dwFaceNum);
	char *pEachFaceSign = new char[dwFaceNum];   // 如果某个序号的面已存，则对应序号的该数组元素记为1，和FaceSign不同，它的生存期只在一个MESH中，用于统计该SPLITMESH的总面数
	memset(pEachFaceSign, 0, dwFaceNum);
	char *pVertexSign = new char[dwVertexNum];   // 对应在顶点缓冲区的每个顶点，标记SPLITMESH中用到的所有顶点，用于过滤IB中重复的顶点，建立SPLITMESH自己的顶点结构
	memset(pVertexSign, 0, dwVertexNum);
	DWORD *pVertexMapping = new DWORD[dwVertexNum];   // 对应在SPLITMESH的每个顶点，存储在MESH顶点缓冲区的位置，比如说VertexMapping[0]就存放SPLITMESH第一个顶点在原MESH中的位置
	memset(pVertexMapping, 0, dwVertexNum*sizeof(DWORD));
	DWORD VertexIndics = 0;
	SplitMesh *pSplitMesh = new SplitMesh;


//存MESHCONTAINER的值
	pMesh->pSplitMesh = pSplitMesh;
	pMesh->dwMaxMatrixNum = dwMaxMatrixNum;
	
	//第一种类型：选择参数指定的矩阵数，排序后选择影响顶点最多的第0到15个，依次递增，循环下一次就是第16到31个……直到循环到最后一个为止
	//循环扫描所有的Subset，扫描一次就得切换一次SPLITMESH
	//注意这里为何要用dwAttributeTableNum，原因是MaterialNum可能有几个，但某些MATERAIL可能没一个顶点会用到，而属性表的数目恰恰等于顶点所用到的SUBSET总数目，跟多余的MATERIAL无关
	char pFaceVertexSign[3] = {0,0,0};
	DWORD dwBoneRangeMin = 0, dwBoneRangeMax = dwMaxMatrixNum;           //每次循环中，当前的骨骼索引范围

	for(n=0; n<dwAttributeTableNum; n++)
	{
		dwBoneRangeMin = 0; dwBoneRangeMax = dwMaxMatrixNum;
		dwBoneRangeMin = dwBoneRangeMin>dwBoneNum ? dwBoneNum : dwBoneRangeMin;
		dwBoneRangeMax = dwBoneRangeMax>dwBoneNum ? dwBoneNum : dwBoneRangeMax;
		
		for(m=0;m<(dwBoneNum/dwMaxMatrixNum+1);m++)
		{
			for(i=0; i<dwFaceNum; i++)
			{
				//该面没有存，处理
				if(!pFaceSign[i] && pFaceID[i]==n)
				{	
					memset(pFaceVertexSign, 0, 3);
					//先判断三个顶点是否都受范围内的矩阵影响，仅判断而已
					for(j=0,x=0; j<3; j++,x++)
					{
						//先得到顶点索引值
						if(Desc.Format == D3DFMT_INDEX16)
							VertexIndics = *((unsigned short int *)pHeadSource+i*3+j);
						else if(Desc.Format == D3DFMT_INDEX32)
							VertexIndics = *((DWORD *)pHeadSource+i*3+j);

						//根据索引值去骨骼数据中查找，看该顶点的每个矩阵都是否在目前的骨骼范围内
						for(k=0; k<dwBoneNum; k++)
							for(l=0; l<CurrentVertexNum[pBoneOptimizedMapping[k]]; l++)
								// 找到
								if(*(*(pBoneVertics+pBoneOptimizedMapping[k])+l) == VertexIndics)
								{
									//判断是否在目前的骨骼范围内，如果该顶点有一个骨骼不在，就不能要
									if(!(k<dwBoneRangeMax && k>=dwBoneRangeMin))
									{
										goto ttt;
									}
								}
						// 注意这里不用判断：是否可能有多过dwMaxMatrixNum个矩阵来影响一个顶点，因为这是不可能发生的^_^
						// 循环完毕，能不去ttt而来到这里，说明所有骨骼都在范围内，然后根据该面找到顶点在VB中的序号并做标记
						pFaceVertexSign[j] = 1;
								
ttt:
;
					}//end for 计算三个顶点矩阵范围


					// 成功判断一个面，可以加入MESH
					if(pFaceVertexSign[0]==1 && pFaceVertexSign[1]==1 && pFaceVertexSign[2]==1)
					{
						if(Desc.Format == D3DFMT_INDEX16)
						{
							pVertexSign[*((unsigned short int *)pHeadSource+i*3)] = 1;
							pVertexSign[*((unsigned short int *)pHeadSource+i*3+1)] = 1;
							pVertexSign[*((unsigned short int *)pHeadSource+i*3+2)] = 1;
						}
						else if(Desc.Format == D3DFMT_INDEX32)
						{
							pVertexSign[*((DWORD *)pHeadSource+i*3)] = 1;
							pVertexSign[*((DWORD *)pHeadSource+i*3+1)] = 1;
							pVertexSign[*((DWORD *)pHeadSource+i*3+2)] = 1;
						}
		
						pFaceSign[i] = 1;
						pEachFaceSign[i] = 1;
					}//end if判断一个面成功后

				}// end if该面未存
			}//end for eachface

				//扫描所有面结束，写入一个完整的SPLITMESH，除了m和n之外，所有的循环控制变量都可以用了
					//首先统计顶点数量，以便分配顶点缓冲区，并同时建立SPLITMESH和MESH的顶点映射关系
				for(y=0,temp=0; y<dwVertexNum; y++)
					if(pVertexSign[y])
					{
						pVertexMapping[temp] = y;
						temp++;
					}
					//再统计面数量，以便分配索引缓冲区
				for(y=0,temp1=0; y<dwFaceNum; y++)
					if(pEachFaceSign[y])
						temp1++;

					//该区域没有分配到一个顶点，就跳过继续
				if(!temp1 || !temp)
					continue;
					//写入数值数据
				pSplitMesh->dwVertexNum = temp;
				pSplitMesh->dwFaceNum = temp1;
				pSplitMesh->Type = 1;
				pSplitMesh->dwSubset = n;
				pSplitMesh->dwMatrixNum = dwBoneRangeMax - dwBoneRangeMin;
				pSplitMesh->dwDataVertexSize = dwMaxMatrixModNum * 4 * 2;

					//建立索引缓冲（因为SPLITMESH比较小，所以就用16BIT，万一遇到变态的模型就报错吧，XIXI）
				if(temp1 > 65535)
				{
					mymessage("模型太大，分割模型顶点数大于INDEX_16！暂时不支持！");
					OutputDebugString("模型太大，分割模型顶点数大于INDEX_16！暂时不支持！");
					return E_INVALIDARG;
				}
				HRESULT hr = d3ddevice->CreateIndexBuffer(temp1*3*2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pSplitMesh->pIB, NULL);
				pSplitMesh->pIB->Lock(0, temp1*3*2, (void **)&p, 0);
				
				for(y=0; y<dwFaceNum; y++)
					if(pEachFaceSign[y])
						for(z=0; z<3; z++)
						{
							if(Desc.Format == D3DFMT_INDEX16)
							{
								//首先分别得到该面的三个顶点在原MESH中的索引信息
								VertexIndics = *((unsigned short int *)pHeadSource+y*3+z);
								//再查找到SPLITMESH中对应顶点的索引值并写入
								*((unsigned short int *)p) = GetVertexMapping(pVertexMapping, temp, VertexIndics);
							}
							else if(Desc.Format == D3DFMT_INDEX32)
							{
								//首先分别得到该面的三个顶点在原MESH中的索引信息
								VertexIndics = *((DWORD *)pHeadSource+y*3+z);
								//再查找到SPLITMESH中对应顶点的索引值并写入
								*((unsigned short int *)p) = GetVertexMapping(pVertexMapping, temp, VertexIndics);
							}
							p+=2;
						}
				pSplitMesh->pIB->Unlock();

					//建立顶点缓冲
				z = D3DXGetFVFVertexSize(pMesh->pSkinMesh->GetFVF());
				pSplitMesh->dwVertexSize = z;
				pSplitMesh->FVF = pMesh->pSkinMesh->GetFVF();
				d3ddevice->CreateVertexBuffer(temp*z, D3DUSAGE_WRITEONLY, pMesh->pSkinMesh->GetFVF(), D3DPOOL_DEFAULT, &pSplitMesh->pVB, NULL);
				pSplitMesh->pVB->Lock(0, temp*z, (void **)&p, 0);
				pMesh->pSkinMesh->LockVertexBuffer(0, (void **)&p1);

				for(y=0; y<dwVertexNum; y++)
					if(pVertexSign[y])
					{
						memcpy(p, p1+y*z, z);
						p+=z;
					}

				pSplitMesh->pVB->Unlock();
				pMesh->pSkinMesh->UnlockVertexBuffer();

					//建立数据流缓冲，注意每个顶点的大小等于矩阵数*4*2（索引和权重都是float，索引也可以用float，不用怀疑，a0.x支持的），这是因为VS中无法循环、跳转，必须按定值计算
				d3ddevice->CreateVertexBuffer(temp*dwMaxMatrixModNum*4*2, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pSplitMesh->pDB, NULL);
					//FVF: D3DFVF_TEX8|D3DFVF_TEXCOORDSIZE4(0)|D3DFVF_TEXCOORDSIZE4(1)|D3DFVF_TEXCOORDSIZE4(2)|D3DFVF_TEXCOORDSIZE4(3)|D3DFVF_TEXCOORDSIZE4(4)|D3DFVF_TEXCOORDSIZE4(5)|D3DFVF_TEXCOORDSIZE4(6)|D3DFVF_TEXCOORDSIZE4(7)
				pSplitMesh->pDB->Lock(0, temp*dwMaxMatrixModNum*4*2, (void **)&p, 0);
				p1 = p + dwMaxMatrixModNum * 4;   //p1是写权重用的，如果有16个矩阵话，它跟索引总是差4个寄存器的位置（16个float）

						//1.得到SPLITMESH每个顶点在原MESH中的索引值
				for(y=0; y<temp; y++)
				{
					VertexIndics = pVertexMapping[y];
						//2.通过该索引和目前骨骼的范围在pBoneVertics和pWeight中查找对应值（肯定都能找到的）并写入
					for(k=0,z=0; k<dwBoneNum; k++)
						for(l=0; l<CurrentVertexNum[pBoneOptimizedMapping[k]]; l++)
							// 找到
							if(*(*(pBoneVertics+pBoneOptimizedMapping[k])+l) == VertexIndics)
							{
								//写入索引数据和权重，注意索引是本块范围的相对索引，所以要减去BoneRange最小值
								*((float *)p) = (float)(k - dwBoneRangeMin);    //注意这里，因为后面的矩阵存储也是按照排序来完成的，所以不用映射到原矩阵顺序，还有就是为何不判断是否在范围内，因为能根据SPLITMESH顶点映射到这里来的，已经必定是在范围内了
								p+=4;
								*((float *)p1) = *(*(pWeights+pBoneOptimizedMapping[k])+l);
								p1+=4;
								z++;  //z的作用是记录该顶点写入的数目，以便下面补足
							}
						//3.上面写入的数目不定（即影响每个顶点的矩阵数都是不同的），所以要记录下来，在这数目到dwMaxMatrixModNum中间的所有权重都为0（索引随便，反正权重为0怎么乘都无效的）
					for(k=0; k<(dwMaxMatrixModNum-z); k++)
					{
						*((float *)p) = 0;
						p+=4;
						*((float *)p1) = 0;
						p1+=4;
					}
					//跳过一段，免得索引接下来写，就把权重刚才写的覆盖了，造成数据混乱:-(
					p+=dwMaxMatrixModNum*4;
					p1+=dwMaxMatrixModNum*4;
				}						
				pSplitMesh->pDB->Unlock();

				//建立矩阵映射信息（注意是经过优化的而非原来的，而且是从RangeMin到RangeMax）
				//注意矩阵内容不在这里写入，因为这是预处理，矩阵内容要在每次Draw的时候UpdateHierarchy后再写入
				pSplitMesh->pMatrixMapping = new DWORD[dwBoneRangeMax-dwBoneRangeMin];
				for(y=0; y<dwBoneRangeMax-dwBoneRangeMin; y++)
				{
					pSplitMesh->pMatrixMapping[y] = pBoneOptimizedMapping[y+dwBoneRangeMin];
				}
				
				
				//数据写入全部结束，初始化一些数据，准备进入下一个SPLITMESH的循环
				memset(pVertexSign, 0, dwVertexNum);
				memset(pEachFaceSign, 0, dwFaceNum);
				memset(pVertexMapping, 0, dwVertexNum*sizeof(DWORD));

					//分配NEXT SPLITMESH
				pSplitMesh->pNextMesh = new SplitMesh;
				pSplitMesh = pSplitMesh->pNextMesh;

			dwBoneRangeMin += dwMaxMatrixNum;
			dwBoneRangeMax += dwMaxMatrixNum;
			dwBoneRangeMin = dwBoneRangeMin>dwBoneNum ? dwBoneNum : dwBoneRangeMin;
			dwBoneRangeMax = dwBoneRangeMax>dwBoneNum ? dwBoneNum : dwBoneRangeMax;

		}//end for bone range
	}//end for attribute table


	//第二种分割
	memset(pEachFaceSign, 0, dwFaceNum);
	memset(pVertexSign, 0, dwVertexNum);
	memset(pVertexMapping, 0, dwVertexNum*sizeof(DWORD));

	DWORD dwMatrixNum = 0;   // 当前分割的MESH已有的矩阵（骨骼）数量
	char *pFaceMatrixSign = new char[dwBoneNum]; // 如果某个序号的矩阵已存，则对应序号的该数组元素记为1，用来过滤同一个面中三个顶点重复使用的矩阵
	memset(pFaceMatrixSign, 0, dwBoneNum);
	char *pMatrixSign = new char[dwBoneNum]; // 如果某个序号的矩阵已存，则对应序号的该数组元素记为1，用来建立每个SPLITMESH最终的矩阵数据映射
	memset(pMatrixSign, 0, dwBoneNum);

	// 上一种类型的分割完后，进入第二种类型：将剩下比较分散的面重新合并为一个SPLITMESH，所谓分散的意思就是说影响三个顶点的矩阵可能不连续，甚至是离得很开
	// 这种类型的每个SPLITMESH中的面，都会小于等于最大矩阵数的
	// 直到所有的面都存了，就结束，每进行一次while循环就相当于已经存储完一个SPLITMESH
	while(!CheckAllFacesHaveBeenSaved(pFaceSign, dwFaceNum))
	{
		//循环扫描所有的Subset，扫描一次就得切换一次SPLITMESH
		//注意这里为何要用dwAttributeTableNum，原因是MaterialNum可能有几个，但某些MATERAIL可能没一个顶点会用到，而属性表的数目恰恰等于顶点所用到的SUBSET总数目，跟多余的MATERIAL无关
		for(n=0; n<dwAttributeTableNum; n++)
		{
			dwMatrixNum = 0;
			for(i=0,dwFaceMatrixNum=0; i<dwFaceNum; i++,dwFaceMatrixNum=0)
				//该面没有存，处理
				if(!pFaceSign[i] && pFaceID[i]==n)
				{	
					memset(pFaceMatrixSign, 0, dwBoneNum);
					//先计算三个顶点所需矩阵之和，存在该面矩阵数的临时变量中
					for(j=0; j<3; j++)
					{
						//先得到顶点索引值
						if(Desc.Format == D3DFMT_INDEX16)
							VertexIndics = *((unsigned short int *)pHeadSource+i*3+j);
						else if(Desc.Format == D3DFMT_INDEX32)
							VertexIndics = *((DWORD *)pHeadSource+i*3+j);

						//根据索引值去骨骼数据中查找，看有多少个不同的骨骼会影响该顶点
						for(k=0; k<dwBoneNum; k++)
							for(l=0; l<CurrentVertexNum[k]; l++)
								//找到一块骨骼，但要看是否是新的骨骼
								if(*(*(pBoneVertics+k)+l) == VertexIndics)
								{
									//新骨骼！计数器加1，并置相应的标记
									if(!pMatrixSign[k])
									{
										dwFaceMatrixNum++;
										pFaceMatrixSign[k] = 1;
										pMatrixSign[k] = 1;  //虽然在这里置1,但是若该面不能存，则还要还原
									}
								}

					}//end for 计算三个顶点矩阵之和

					//单个面的矩阵（骨头）总数就超过了指定的最大矩阵数，无法处理，只能增加同时处理的最大矩阵数（用于新版VS）
					if(dwFaceMatrixNum > dwMaxMatrixNum)
						return D3DERR_OUTOFVIDEOMEMORY;
			
					//若该面加上已存的矩阵数超过了最大矩阵数，则切换新建后面的SPLITMesh
					if((dwFaceMatrixNum+dwMatrixNum) > dwMaxMatrixNum)
					{
					
						//恢复MatrixSign
						for(y=0; y<dwBoneNum; y++)
							if(pFaceMatrixSign[y])
								pMatrixSign[y] = 0;

						//开始存储数据
							//1. 得到面数量和顶点数量以及顶点映射
						for(y=0,temp=0; y<dwVertexNum; y++)
							if(pVertexSign[y])
							{
								pVertexMapping[temp] = y;
								temp++;
							}
						for(y=0,temp1=0; y<dwFaceNum; y++)
							if(pEachFaceSign[y])
								temp1++;

							//该区域没有分配到一个顶点，就跳过继续
						if(!temp1 || !temp)
						{
							dwMatrixNum = 0;
							i--;
							continue;
						}
							//2. 写入数值数据
						pSplitMesh->dwVertexNum = temp;
						pSplitMesh->dwFaceNum = temp1;
						pSplitMesh->Type = 2;
						pSplitMesh->dwSubset = n;
						pSplitMesh->dwMatrixNum = dwMatrixNum;
						pSplitMesh->dwDataVertexSize = dwMaxMatrixModNum * 4 * 2;


							//3. 建立索引缓冲（因为SPLITMESH比较小，所以就用16BIT，万一遇到变态的模型就报错吧，XIXI）
						if(temp1 > 65535)
						{
							mymessage("模型太大，分割模型顶点数大于INDEX_16！暂时不支持！");
								OutputDebugString("模型太大，分割模型顶点数大于INDEX_16！暂时不支持！");
								return E_INVALIDARG;
						}

						HRESULT hr = d3ddevice->CreateIndexBuffer(temp1*3*2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pSplitMesh->pIB, NULL);
						pSplitMesh->pIB->Lock(0, temp1*3*2, (void **)&p, 0);
				
						for(y=0; y<dwFaceNum; y++)
							if(pEachFaceSign[y])
								for(z=0; z<3; z++)
								{
									if(Desc.Format == D3DFMT_INDEX16)
									{
										//首先分别得到该面的三个顶点在原MESH中的索引信息
										VertexIndics = *((unsigned short int *)pHeadSource+y*3+z);
										//再查找到SPLITMESH中对应顶点的索引值并写入
										*((unsigned short int *)p) = GetVertexMapping(pVertexMapping, temp, VertexIndics);
									}
									else if(Desc.Format == D3DFMT_INDEX32)
									{
										//首先分别得到该面的三个顶点在原MESH中的索引信息
										VertexIndics = *((DWORD *)pHeadSource+y*3+z);
										//再查找到SPLITMESH中对应顶点的索引值并写入
										*((unsigned short int *)p) = GetVertexMapping(pVertexMapping, temp, VertexIndics);
									}
									p+=2;
								}
						pSplitMesh->pIB->Unlock();

							//4. 建立顶点缓冲
						z = D3DXGetFVFVertexSize(pMesh->pSkinMesh->GetFVF());
						pSplitMesh->dwVertexSize = z;
						pSplitMesh->FVF = pMesh->pSkinMesh->GetFVF();
						d3ddevice->CreateVertexBuffer(temp*z, D3DUSAGE_WRITEONLY, pMesh->pSkinMesh->GetFVF(), D3DPOOL_DEFAULT, &pSplitMesh->pVB, NULL);
						pSplitMesh->pVB->Lock(0, temp*z, (void **)&p, 0);
						pMesh->pSkinMesh->LockVertexBuffer(0, (void **)&p1);

						for(y=0; y<dwVertexNum; y++)
							if(pVertexSign[y])
							{
								memcpy(p, p1+y*z, z);
								p+=z;
							}

						pSplitMesh->pVB->Unlock();
						pMesh->pSkinMesh->UnlockVertexBuffer();

							//5. 建立数据流缓冲，注意每个顶点的大小等于矩阵数*4*2（索引和权重都是float，索引也可以用float，不用怀疑，a0.x支持的），这是因为VS中无法循环、跳转，必须按定值计算
						d3ddevice->CreateVertexBuffer(temp*dwMaxMatrixModNum*4*2, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pSplitMesh->pDB, NULL);
						pSplitMesh->pDB->Lock(0, temp*dwMaxMatrixModNum*4*2, (void **)&p, 0);
						p1 = p + dwMaxMatrixModNum * 4;   //p1是写权重用的，如果有16个矩阵话，它跟索引总是差4个寄存器的位置（16个float）

								//1.得到SPLITMESH每个顶点在原MESH中的索引值
						for(y=0; y<temp; y++)
						{
							VertexIndics = pVertexMapping[y];
								//2.通过该索引和目前骨骼的集合在pBoneVertics和pWeight中查找对应值（肯定都能找到的）并写入
							for(k=0,z=0,x=0; k<dwBoneNum; k++)
								if(pMatrixSign[k])
								{
									for(l=0; l<CurrentVertexNum[k]; l++)
										// 找到
										if(*(*(pBoneVertics+k)+l) == VertexIndics)
										{
											//写入索引数据和权重，注意索引是本块范围的相对索引，所以要减去BoneRange最小值
											*((float *)p) = (float)x;    //注意这里，因为后面的矩阵存储也是按照排序来完成的，所以不用映射到原矩阵顺序，还有就是为何不判断是否在范围内，因为能根据SPLITMESH顶点映射到这里来的，已经必定是在范围内了
											p+=4;
											*((float *)p1) = *(*(pWeights+k)+l);
											p1+=4;
											z++;  //z的作用是记录该顶点写入的数目，以便下面补足
										}
									x++;  //x的作用是记录当前的矩阵索引号
								}
								//3.上面写入的数目不定（即影响每个顶点的矩阵数都是不同的），所以要记录下来，在这数目到dwMaxMatrixModNum中间的所有权重都为0（索引随便，反正权重为0怎么乘都无效的）
							for(k=0; k<(dwMaxMatrixModNum-z); k++)
							{
								*((float *)p) = 0;
								p+=4;
								*((float *)p1) = 0;
								p1+=4;
							}
							//跳过一段，免得索引接下来写，就把权重刚才写的覆盖了，造成数据混乱:-(
							p+=dwMaxMatrixModNum*4;
							p1+=dwMaxMatrixModNum*4;
						}						
						pSplitMesh->pDB->Unlock();

						//建立矩阵映射信息（注意是经过优化的而非原来的，而且是从RangeMin到RangeMax）
						//注意矩阵内容不在这里写入，因为这是预处理，矩阵内容要在每次Draw的时候UpdateHierarchy后再写入
						pSplitMesh->pMatrixMapping = new DWORD[dwMatrixNum];

						for(y=0,x=0; y<dwBoneNum; y++)
							if(pMatrixSign[y])
							{
								pSplitMesh->pMatrixMapping[x] = y;
								x++;
							}

						//数据写入全部结束，初始化一些数据，准备进入下一个SPLITMESH的循环
							//分配NEXT SPLITMESH
						pSplitMesh->pNextMesh = new SplitMesh;
						pSplitMesh = pSplitMesh->pNextMesh;

						dwMatrixNum = 0;

						memset(pVertexMapping, 0, dwVertexNum*sizeof(DWORD));
						memset(pVertexSign, 0, dwVertexNum);
						memset(pEachFaceSign, 0, dwFaceNum);
						memset(pFaceMatrixSign, 0, dwBoneNum);
						memset(pMatrixSign, 0, dwBoneNum);

						//重复该面，如果i不--，就跳过去了，要不得
						i--;
						continue;
					}//end save and toggle next splitmesh





					//该面满足要求，将该面标记为1，表示已存
					else
					{
						//将该面数据存入SplitMesh（仅数据）
						dwMatrixNum += dwFaceMatrixNum;

						for(y=0; y<dwBoneNum; y++)
							if(pFaceMatrixSign[y])
								pMatrixSign[y] = 1;

						if(Desc.Format == D3DFMT_INDEX16)
						{
							pVertexSign[*((unsigned short int *)pHeadSource+i*3)] = 1;
							pVertexSign[*((unsigned short int *)pHeadSource+i*3+1)] = 1;
							pVertexSign[*((unsigned short int *)pHeadSource+i*3+2)] = 1;
						}
						else if(Desc.Format == D3DFMT_INDEX32)
						{
							pVertexSign[*((DWORD *)pHeadSource+i*3)] = 1;
							pVertexSign[*((DWORD *)pHeadSource+i*3+1)] = 1;
							pVertexSign[*((DWORD *)pHeadSource+i*3+2)] = 1;
						}

						pEachFaceSign[i] = 1;
						pFaceSign[i] = 1;
					}
				}// end for each face and if该面未存




			//遍历一个SUBSET的所有面结束，进行扫尾，如果当前SPLITMESH中有东西，就说明存放了最后一个面，就要单独存成一个SPLITMESH
							//1. 得到面数量和顶点数量以及顶点映射
						for(y=0,temp=0; y<dwVertexNum; y++)
							if(pVertexSign[y])
							{
								pVertexMapping[temp] = y;
								temp++;
							}
						for(y=0,temp1=0; y<dwFaceNum; y++)
							if(pEachFaceSign[y])
								temp1++;
						
							//该区域没有分配到一个顶点，说明SPLITMESH没有东西，跳过继续
						if(!temp1 || !temp)
						{
							dwMatrixNum = 0;
							memset(pVertexMapping, 0, dwVertexNum*sizeof(DWORD));
							memset(pVertexSign, 0, dwVertexNum);
							memset(pEachFaceSign, 0, dwFaceNum);
							memset(pFaceMatrixSign, 0, dwBoneNum);
							memset(pMatrixSign, 0, dwBoneNum);
							continue;
						}
							//2. 写入数值数据
						pSplitMesh->dwVertexNum = temp;
						pSplitMesh->dwFaceNum = temp1;
						pSplitMesh->Type = 2;
						pSplitMesh->dwSubset = n;
						pSplitMesh->dwMatrixNum = dwMatrixNum;
						pSplitMesh->dwDataVertexSize = dwMaxMatrixModNum * 4 * 2;


							//3. 建立索引缓冲（因为SPLITMESH比较小，所以就用16BIT，万一遇到变态的模型就报错吧，XIXI）
						if(temp1 > 65535)
						{
							mymessage("模型太大，分割模型顶点数大于INDEX_16！暂时不支持！");
								OutputDebugString("模型太大，分割模型顶点数大于INDEX_16！暂时不支持！");
								return E_INVALIDARG;
						}

						HRESULT hr = d3ddevice->CreateIndexBuffer(temp1*3*2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pSplitMesh->pIB, NULL);
						pSplitMesh->pIB->Lock(0, temp1*3*2, (void **)&p, 0);
				
						for(y=0; y<dwFaceNum; y++)
							if(pEachFaceSign[y])
								for(z=0; z<3; z++)
								{
									if(Desc.Format == D3DFMT_INDEX16)
									{
										//首先分别得到该面的三个顶点在原MESH中的索引信息
										VertexIndics = *((unsigned short int *)pHeadSource+y*3+z);
										//再查找到SPLITMESH中对应顶点的索引值并写入
										*((unsigned short int *)p) = GetVertexMapping(pVertexMapping, temp, VertexIndics);
									}
									else if(Desc.Format == D3DFMT_INDEX32)
									{
										//首先分别得到该面的三个顶点在原MESH中的索引信息
										VertexIndics = *((DWORD *)pHeadSource+y*3+z);
										//再查找到SPLITMESH中对应顶点的索引值并写入
										*((unsigned short int *)p) = GetVertexMapping(pVertexMapping, temp, VertexIndics);
									}
									p+=2;
								}
						pSplitMesh->pIB->Unlock();

							//4. 建立顶点缓冲
						z = D3DXGetFVFVertexSize(pMesh->pSkinMesh->GetFVF());
						pSplitMesh->dwVertexSize = z;
						pSplitMesh->FVF = pMesh->pSkinMesh->GetFVF();
						d3ddevice->CreateVertexBuffer(temp*z, D3DUSAGE_WRITEONLY, pMesh->pSkinMesh->GetFVF(), D3DPOOL_DEFAULT, &pSplitMesh->pVB, NULL);
						pSplitMesh->pVB->Lock(0, temp*z, (void **)&p, 0);
						pMesh->pSkinMesh->LockVertexBuffer(0, (void **)&p1);

						for(y=0; y<dwVertexNum; y++)
							if(pVertexSign[y])
							{
								memcpy(p, p1+y*z, z);
								p+=z;
							}

						pSplitMesh->pVB->Unlock();
						pMesh->pSkinMesh->UnlockVertexBuffer();

							//5. 建立数据流缓冲，注意每个顶点的大小等于矩阵数*4*2（索引和权重都是float，索引也可以用float，不用怀疑，a0.x支持的），这是因为VS中无法循环、跳转，必须按定值计算
						d3ddevice->CreateVertexBuffer(temp*dwMaxMatrixModNum*4*2, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pSplitMesh->pDB, NULL);
						pSplitMesh->pDB->Lock(0, temp*dwMaxMatrixModNum*4*2, (void **)&p, 0);
						p1 = p + dwMaxMatrixModNum * 4;   //p1是写权重用的，如果有16个矩阵话，它跟索引总是差4个寄存器的位置（16个float）

								//1.得到SPLITMESH每个顶点在原MESH中的索引值
						for(y=0; y<temp; y++)
						{
							VertexIndics = pVertexMapping[y];
								//2.通过该索引和目前骨骼的集合在pBoneVertics和pWeight中查找对应值（肯定都能找到的）并写入
							for(k=0,z=0,x=0; k<dwBoneNum; k++)
								if(pMatrixSign[k])
								{
									for(l=0; l<CurrentVertexNum[k]; l++)
										// 找到
										if(*(*(pBoneVertics+k)+l) == VertexIndics)
										{
											//写入索引数据和权重，注意索引是本块范围的相对索引，所以要减去BoneRange最小值
											*((float *)p) = (float)x;    //注意这里，因为后面的矩阵存储也是按照排序来完成的，所以不用映射到原矩阵顺序，还有就是为何不判断是否在范围内，因为能根据SPLITMESH顶点映射到这里来的，已经必定是在范围内了
											p+=4;
											*((float *)p1) = *(*(pWeights+k)+l);
											p1+=4;
											z++;  //z的作用是记录该顶点写入的数目，以便下面补足
										}
									x++;  //x的作用是记录当前的矩阵索引号
								}
								//3.上面写入的数目不定（即影响每个顶点的矩阵数都是不同的），所以要记录下来，在这数目到dwMaxMatrixModNum中间的所有权重都为0（索引随便，反正权重为0怎么乘都无效的）
							for(k=0; k<(dwMaxMatrixModNum-z); k++)
							{
								*((float *)p) = 0;
								p+=4;
								*((float *)p1) = 0;
								p1+=4;
							}
							//跳过一段，免得索引接下来写，就把权重刚才写的覆盖了，造成数据混乱:-(
							p+=dwMaxMatrixModNum*4;
							p1+=dwMaxMatrixModNum*4;
						}						
						pSplitMesh->pDB->Unlock();

						//建立矩阵映射信息（注意是经过优化的而非原来的，而且是从RangeMin到RangeMax）
						//注意矩阵内容不在这里写入，因为这是预处理，矩阵内容要在每次Draw的时候UpdateHierarchy后再写入
						pSplitMesh->pMatrixMapping = new DWORD[dwMatrixNum];

						for(y=0,x=0; y<dwBoneNum; y++)
							if(pMatrixSign[y])
							{
								pSplitMesh->pMatrixMapping[x] = y;
								x++;
							}				
				
						//数据写入全部结束，初始化一些数据，准备进入下一个SPLITMESH的循环

			//统统新换新建的MESH，进入下一个SUBSET的扫描
			pSplitMesh->pNextMesh = new SplitMesh;
			pSplitMesh = pSplitMesh->pNextMesh;

			dwMatrixNum = 0;
			memset(pVertexMapping, 0, dwVertexNum*sizeof(DWORD));
			memset(pVertexSign, 0, dwVertexNum);
			memset(pEachFaceSign, 0, dwFaceNum);
			memset(pFaceMatrixSign, 0, dwBoneNum);
			memset(pMatrixSign, 0, dwBoneNum);
		}//end for所有Subset

	}//end while
	
	pIB->Unlock();
	pIB->Release();

	//每次最后新建SplitMesh的方法，会导致最后多出一个无用的SPLITMESH，要DELETE掉它，顺路算出SplitMesh的总数目
	SplitMesh *pSpMesh = pMesh->pSplitMesh;
	temp = 1;
	while(pSpMesh)
	{
		if(pSpMesh->pNextMesh == pSplitMesh)
		{
			delete pSpMesh->pNextMesh;
			pSpMesh->pNextMesh = NULL;
			break;
		}
		pSpMesh = pSpMesh->pNextMesh;
		temp++;
	}
	pMesh->dwSplitMeshNum = temp;
	
	//清除所有临时分配的东西
	for(y=0; y<dwBoneNum; y++)
	{
		delete [] pWeights[y];
		pWeights[y] = NULL;
		delete [] pBoneVertics[y];
		pBoneVertics[y] = NULL;
	}

	delete [] pAttributeTable;
	pAttributeTable = NULL;
	delete [] pBoneVertics;
	pBoneVertics = NULL;
	delete [] pWeights;
	pWeights = NULL;
	delete [] CurrentVertexNum;
	CurrentVertexNum = NULL;
	delete [] pBoneOptimizedMapping;
	pBoneOptimizedMapping = NULL;
	delete [] pFaceID;
	pFaceID = NULL;
	delete [] pFaceSign;
	pFaceSign = NULL;
	delete [] pEachFaceSign;
	pEachFaceSign = NULL;
	delete [] pVertexSign;
	pVertexSign = NULL;
	delete [] pVertexMapping;
	pVertexMapping = NULL;
	delete [] pFaceMatrixSign;
	pFaceMatrixSign = NULL;
	delete [] pMatrixSign;
	pMatrixSign = NULL;
	delete [] pOptimizedCurrentVertexNum;
	pOptimizedCurrentVertexNum = NULL;
	
	m_CreateAttrib = 2;
	return S_OK;
}














// 设置SplitMesh所有的矩阵数据，使用前确保常量寄存器的起始索引是没有错误的！！
HRESULT SKINMESH::SetMatrixData(UINT iStartConstantIndex, SplitMesh *pSplitMesh, D3DXMESHCONTAINER_EX *pMesh)
{
	if(!pSplitMesh || !pMesh || m_CreateAttrib!=2 )
		return E_FAIL;
	if(!pSplitMesh->Type || !pSplitMesh->pMatrixMapping || !pSplitMesh->dwMatrixNum || !pSplitMesh->pVB || !pSplitMesh->pIB || !pSplitMesh->pDB)
		return E_FAIL;
	if(iStartConstantIndex > 255)
		return D3DERR_INVALIDCALL;
	
	//pSplitMesh->pMatrix = new D3DXMATRIX[pSplitMesh->dwMatrixNum];
	DWORD i = 0, j = 0;
	D3DXMATRIX matTemp;
	
	for(i=0; i<pSplitMesh->dwMatrixNum; i++) 
	{
		j = pSplitMesh->pMatrixMapping[i];

		if(pMesh->ppFrameMatrices[j])
		{
			matTemp = (*(pMesh->pSkinInfo->GetBoneOffsetMatrix(j))) * (*((pMesh->ppFrameMatrices)[j]));
			// 存入所有的矩阵，不用补足，将不足的权值设为0就可以了（在SplitMesh函数中已经做了）
			D3DXMatrixTranspose(&matTemp, &matTemp);
			//(pSplitMesh->pMatrix)[i] = (*(pMesh->pSkinInfo->GetBoneOffsetMatrix(j))) * (*((pMesh->ppFrameMatrices)[j]));
			d3ddevice->SetVertexShaderConstantF(iStartConstantIndex+i*4, (const float *)matTemp, 4);
		}
	}
	return S_OK;
}//end function










HRESULT SKINMESH::LoadSplitMeshFromFile(char *szFileName)
{
	if(!szFileName)
		return D3DERR_INVALIDCALL;
	if(!szFileName[0])
		return D3DERR_INVALIDCALL;

	if(!m_pMesh)
	{
		OutputDebugString("请先读入一个SkinMesh!!");
		return E_FAIL;
	}
	if(m_pMesh->pSplitMesh || m_pMesh->dwSplitMeshNum)
	{
		OutputDebugString("已经分割过了SkinMesh，不用再读入数据了!!");
		return S_OK;
	}

	FILE *fp = fopen(szFileName, "rb");
	if(!fp)
	{
		OutputDebugString("请指定一个有效的路径和文件名！");
		return D3DERR_INVALIDCALL;
	}

	// 读取文件头信息
	char szVerifyName[20] = "SPLITMESHVS";
	fread(szVerifyName, 1, sizeof("SPLITMESHVS"), fp);
	if(strcmp(szVerifyName, "SPLITMESHVS"))
	{
		OutputDebugString("文件头校验失败，请确保文件是有效的SplitMesh数据文件！！");
		return E_FAIL;
	}

	DWORD iData = 0;
	fread(&iData, 1, sizeof(DWORD), fp);
	if(!iData)
	{
		OutputDebugString("文件头数据校验失败，请确保文件是有效的SplitMesh数据文件！！");
		return E_FAIL;
	}
	m_pMesh->dwSplitMeshNum = iData;

	fread(&iData, 1, sizeof(DWORD), fp);
	if(!iData)
	{
		OutputDebugString("文件头数据校验失败，请确保文件是有效的SplitMesh数据文件！！");
		return E_FAIL;
	}
	m_pMesh->dwMaxMatrixNum = iData;




	// 读取每个分块的信息，创建新的SplitMesh
	if(m_pMesh->pSplitMesh)
	{
		OutputDebugString("已经分割过了SkinMesh，不用再读入数据了!!");
		goto FAILED;
	}


	SplitMesh *pSplitMesh = new SplitMesh;
	m_pMesh->pSplitMesh = pSplitMesh;

	for(DWORD i = 0; i < m_pMesh->dwSplitMeshNum; i++)
	{
		// 读取字符串标识和序号
		fread(szVerifyName, 1, sizeof("SPLITMESH"), fp);
		fread(&iData, 1, sizeof(DWORD), fp);
		if(strcmp(szVerifyName, "SPLITMESH") || iData != i)
		{
			OutputDebugString("文件段头校验失败，请确保文件是有效的SplitMesh数据文件！！");
			return E_FAIL;
		}


		// 读取每个SplitMesh中的定长数据
		fread(&pSplitMesh->dwSubset, 1, sizeof(DWORD), fp);
		fread(&pSplitMesh->Type, 1, sizeof(char), fp);
		fread(&pSplitMesh->dwMatrixNum, 1, sizeof(DWORD), fp);
		fread(&pSplitMesh->dwFaceNum, 1, sizeof(DWORD), fp);
		fread(&pSplitMesh->dwVertexNum, 1, sizeof(DWORD), fp);
		fread(&pSplitMesh->dwVertexSize, 1, sizeof(DWORD), fp);
		fread(&pSplitMesh->dwDataVertexSize, 1, sizeof(DWORD), fp);
		fread(&pSplitMesh->FVF, 1, sizeof(DWORD), fp);
		if(!pSplitMesh->dwVertexNum || !pSplitMesh->dwFaceNum || !pSplitMesh->dwVertexSize || !pSplitMesh->dwDataVertexSize || !pSplitMesh->dwMatrixNum)
		{
			OutputDebugString("文件段数据校验失败，请确保文件是有效的SplitMesh数据文件！！");
			goto FAILED;
		}

		// 读取非定长数据，先是矩阵映射索引数据
		pSplitMesh->pMatrixMapping = new DWORD[pSplitMesh->dwMatrixNum];
		fread(pSplitMesh->pMatrixMapping, pSplitMesh->dwMatrixNum, sizeof(DWORD), fp);


		// 再是VB IB DB
		if(pSplitMesh->pVB)
		{
			OutputDebugString("已有顶点缓冲数据！");
			goto FAILED;
		}
		if(pSplitMesh->pIB)
		{
			OutputDebugString("已有索引缓冲数据！");
			goto FAILED;
		}
		if(pSplitMesh->pDB)
		{
			OutputDebugString("已有数据缓冲数据！");
			goto FAILED;
		}

		// 得到大小并创建顶点缓冲
		DWORD dwVBSize = 0, dwIBSize = 0, dwDBSize = 0;
		char *pVBData = NULL, *pIBData = NULL, *pDBData = NULL;

		// VB
		fread(&dwVBSize, 1, sizeof(DWORD), fp);
		if(!dwVBSize)
		{
			OutputDebugString("文件段数据校验失败，请确保文件是有效的SplitMesh数据文件！！");
			goto FAILED;
		}

		// 创建缓冲区，定格式的，所以不需要读取Desc数据
		HRESULT hr = d3ddevice->CreateVertexBuffer(dwVBSize, D3DUSAGE_WRITEONLY, m_pMesh->pSkinMesh->GetFVF(), D3DPOOL_DEFAULT, &pSplitMesh->pVB, NULL);
		if(FAILED(hr))
		{
			OutputDebugString("创建顶点缓冲失败！！请检查硬件或驱动！可能是显存不足！");
			goto FAILED;
		}
		// Lock，填充数据
		hr = pSplitMesh->pVB->Lock(0, dwVBSize, (void **)&pVBData, 0);
		if(FAILED(hr))
		{
			OutputDebugString("Lock顶点缓冲失败！！");
			goto FAILED;
		}

		fread(pVBData, 1, dwVBSize, fp);



		// IB
		fread(&dwIBSize, 1, sizeof(DWORD), fp);
		if(!dwIBSize)
		{
			OutputDebugString("文件段数据校验失败，请确保文件是有效的SplitMesh数据文件！！");
			goto FAILED;
		}

		hr = d3ddevice->CreateIndexBuffer(dwIBSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pSplitMesh->pIB, NULL);
		if(FAILED(hr))
		{
			OutputDebugString("创建索引缓冲失败！！请检查硬件或驱动！可能是显存不足！");
			goto FAILED;
		}

		hr = pSplitMesh->pIB->Lock(0, dwIBSize, (void **)&pIBData, 0);
		if(FAILED(hr))
		{
			OutputDebugString("Lock索引缓冲失败！！");
			goto FAILED;
		}

		fread(pIBData, 1, dwIBSize, fp);




		// DB
		fread(&dwDBSize, 1, sizeof(DWORD), fp);
		if(!dwDBSize)
		{
			OutputDebugString("文件段数据校验失败，请确保文件是有效的SplitMesh数据文件！！");
			goto FAILED;
		}

		hr = d3ddevice->CreateVertexBuffer(dwDBSize, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pSplitMesh->pDB, NULL);
		if(FAILED(hr))
		{
			OutputDebugString("创建数据缓冲失败！！请检查硬件或驱动！可能是显存不足！");
			goto FAILED;
		}

		hr = pSplitMesh->pDB->Lock(0, dwDBSize, (void **)&pDBData, 0);
		if(FAILED(hr))
		{
			OutputDebugString("Lock数据缓冲失败！！");
			goto FAILED;
		}

		fread(pDBData, 1, dwDBSize, fp);


		// 填充结束
		pSplitMesh->pVB->Unlock();
		pSplitMesh->pIB->Unlock();
		pSplitMesh->pDB->Unlock();


		// 读取完毕，创建新的SplitMesh
		pSplitMesh->pNextMesh = new SplitMesh;
		pSplitMesh = pSplitMesh->pNextMesh;
	}


	//每次最后新建SplitMesh的方法，会导致最后多出一个无用的SPLITMESH，要DELETE掉它，顺路算出SplitMesh的总数目
	SplitMesh *pSpMesh = m_pMesh->pSplitMesh;
	int i = 1;
	while(pSpMesh)
	{
		if(pSpMesh->pNextMesh == pSplitMesh)
		{
			delete pSpMesh->pNextMesh;
			pSpMesh->pNextMesh = NULL;
			break;
		}
		pSpMesh = pSpMesh->pNextMesh;
		i++;
	}

	if(m_pMesh->dwSplitMeshNum != i)
	{
		OutputDebugString("分割Mesh数量校验失败，请确保文件是有效的SplitMesh数据文件！！");
		goto FAILED;
	}



	m_CreateAttrib = 2;
	fclose(fp);
	return S_OK;


FAILED:
	fclose(fp);
	return E_FAIL;

}





HRESULT SKINMESH::SaveSplitMeshToFile(char *szFileName)
{
	if(!szFileName)
		return D3DERR_INVALIDCALL;
	if(!szFileName[0])
		return D3DERR_INVALIDCALL;

	if(!m_pMesh)
	{
		OutputDebugString("请先读入一个SkinMesh!!");
		return E_FAIL;
	}
	if(!m_pMesh->pSplitMesh || !m_pMesh->dwSplitMeshNum)
	{
		OutputDebugString("请先将SkinMesh分割好!!");
		return E_FAIL;
	}

	FILE *fp = fopen(szFileName, "wb");
	if(!fp)
	{
		OutputDebugString("请指定一个有效的路径和文件名！或保证磁盘不是只读而且有足够的剩余空间！");
		return D3DERR_INVALIDCALL;
	}

	// 写入文件头信息
	char szFileHeaderName[] = "SPLITMESHVS";
	fwrite(szFileHeaderName, 1, sizeof("SPLITMESHVS"), fp);
	fwrite(&m_pMesh->dwSplitMeshNum, 1, sizeof(DWORD), fp);
	fwrite(&m_pMesh->dwMaxMatrixNum, 1, sizeof(DWORD), fp);

	// 写入每个分块的信息
	char szFileSegmentName[] = "SPLITMESH";
	SplitMesh *pSplitMesh = m_pMesh->pSplitMesh;

	for(DWORD i = 0; i < m_pMesh->dwSplitMeshNum; i++)
	{
		if(!pSplitMesh)
			break;

		// 写入字符串标识和序号
		fwrite(szFileSegmentName, 1, sizeof("SPLITMESH"), fp);
		fwrite(&i, 1, sizeof(DWORD), fp);

		// 写入每个SplitMesh中的定长数据
		if(!pSplitMesh->dwVertexNum || !pSplitMesh->dwFaceNum || !pSplitMesh->dwVertexSize || !pSplitMesh->dwDataVertexSize)
		{
			OutputDebugString("分割出来的SkinMesh顶点、索引数据有误！！");
			goto FAILED;
		}
		fwrite(&pSplitMesh->dwSubset, 1, sizeof(DWORD), fp);
		fwrite(&pSplitMesh->Type, 1, sizeof(char), fp);
		fwrite(&pSplitMesh->dwMatrixNum, 1, sizeof(DWORD), fp);
		fwrite(&pSplitMesh->dwFaceNum, 1, sizeof(DWORD), fp);
		fwrite(&pSplitMesh->dwVertexNum, 1, sizeof(DWORD), fp);
		fwrite(&pSplitMesh->dwVertexSize, 1, sizeof(DWORD), fp);
		fwrite(&pSplitMesh->dwDataVertexSize, 1, sizeof(DWORD), fp);
		fwrite(&pSplitMesh->FVF, 1, sizeof(DWORD), fp);

		// 写入非定长数据，先是矩阵映射索引数据
		if(!pSplitMesh->pMatrixMapping)
		{
			OutputDebugString("分割出来的SkinMesh数据有误！！没有矩阵映射数据！");
			goto FAILED;
		}
		fwrite(pSplitMesh->pMatrixMapping, pSplitMesh->dwMatrixNum, sizeof(DWORD), fp);


		// 再是VB IB DB
		if(!pSplitMesh->pVB)
		{
			OutputDebugString("分割出来的SkinMesh数据有误！！没有顶点缓冲数据！");
			goto FAILED;
		}
		if(!pSplitMesh->pIB)
		{
			OutputDebugString("分割出来的SkinMesh数据有误！！没有索引缓冲数据！");
			goto FAILED;
		}
		if(!pSplitMesh->pDB)
		{
			OutputDebugString("分割出来的SkinMesh数据有误！！没有数据缓冲数据！");
			goto FAILED;
		}

		// 得到大小并Lock
		D3DVERTEXBUFFER_DESC VBDesc, DBDesc;
		D3DINDEXBUFFER_DESC IBDesc;
		pSplitMesh->pVB->GetDesc(&VBDesc);
		pSplitMesh->pIB->GetDesc(&IBDesc);
		pSplitMesh->pDB->GetDesc(&DBDesc);

		/*因为是定格式的，所以其他Desc数据不用保存
		d3ddevice->CreateVertexBuffer(temp*z, D3DUSAGE_WRITEONLY, pMesh->pSkinMesh->GetFVF(), D3DPOOL_DEFAULT, &pSplitMesh->pVB, NULL);
		HRESULT hr = d3ddevice->CreateIndexBuffer(temp1*3*2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pSplitMesh->pIB, NULL);
		d3ddevice->CreateVertexBuffer(temp*dwMaxMatrixModNum*4*2, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pSplitMesh->pDB, NULL);
		*/

		char *pVBData = NULL, *pIBData = NULL, *pDBData = NULL;
		HRESULT hr = pSplitMesh->pVB->Lock(0, VBDesc.Size, (void **)&pVBData, D3DLOCK_READONLY);
		if(FAILED(hr))
		{
			OutputDebugString("Lock顶点缓冲失败！！");
			goto FAILED;
		}
		hr = pSplitMesh->pIB->Lock(0, IBDesc.Size, (void **)&pIBData, D3DLOCK_READONLY);
		if(FAILED(hr))
		{
			OutputDebugString("Lock索引缓冲失败！！");
			goto FAILED;
		}
		hr = pSplitMesh->pDB->Lock(0, DBDesc.Size, (void **)&pDBData, D3DLOCK_READONLY);
		if(FAILED(hr))
		{
			OutputDebugString("Lock数据缓冲失败！！");
			goto FAILED;
		}

		fwrite(&VBDesc.Size, 1, sizeof(DWORD), fp);
		fwrite(pVBData, 1, VBDesc.Size, fp);

		fwrite(&IBDesc.Size, 1, sizeof(DWORD), fp);
		fwrite(pIBData, 1, IBDesc.Size, fp);

		fwrite(&DBDesc.Size, 1, sizeof(DWORD), fp);
		fwrite(pDBData, 1, DBDesc.Size, fp);

		pSplitMesh->pVB->Unlock();
		pSplitMesh->pIB->Unlock();
		pSplitMesh->pDB->Unlock();


		// 存储完毕，跳到下一个
		pSplitMesh = pSplitMesh->pNextMesh;
	}	


	fclose(fp);
	return S_OK;


FAILED:
	fclose(fp);
	return E_FAIL;

}

































// 遍历每个面得出的边信息
struct MeshEdge
{
	BYTE uCount;   // 该条边出现的总数量，正常情况下等于1，如果大于1，就说明有两个面共用该边，为0还可以表示未初始化的状态
	DWORD dwVertexIndex1, dwVertexIndex2;  // 该边两条顶点在索引缓冲中的索引号，要经过Point Representive，否则会有重复的顶点
	bool bAdded;   // 是否已经加入Degenerated Quad，初始为false，必须在count大于1的情况下才可以为true
	D3DXVECTOR3 ShareFaceNormal[2];        // 共享该边的面法线，因为一条边最多只能被两个面共享，故只有两个元素，具体只用一个还是两个都用，取决于Count
	D3DXVECTOR3 posVertex[2];           // 存放两个顶点的坐标，0和1元素分别对应Index1和Index2，方便以后的使用

	const static D3DVERTEXELEMENT9 dclr[6];   //轮廓边声明

	MeshEdge()
	{
		uCount = 0;
		dwVertexIndex1 = dwVertexIndex2 = 0;
		bAdded = false;
		
		ShareFaceNormal[0] = D3DXVECTOR3(0, 0, 0);
		ShareFaceNormal[1] = D3DXVECTOR3(0, 0, 0);
		posVertex[0] = D3DXVECTOR3(0, 0, 0);
		posVertex[1] = D3DXVECTOR3(0, 0, 0);
	}

};
const D3DVERTEXELEMENT9 MeshEdge::dclr[6] = 
{
	{0,0,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0,12,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0,12+12,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},  // 这是是否能扩展的标志，只有QUAD的两个顶点该标志置1,其他全为0,表示不能扩展
	{0,12+12+12,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 1}, // 下面两个就是两个面的法线
	{0,12+12+12+12,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 2},
	D3DDECL_END()
};


// 该函数会遍历边数组，如果找到共享边，会存入，如果没找到，会新增加，返回的是当前共有的总边数
DWORD SetEdgeData(MeshEdge *p, DWORD Vertex1, DWORD Vertex2, D3DXVECTOR3 posVertex1, D3DXVECTOR3 posVertex2, D3DXVECTOR3 vecFaceNormal)
{
	UINT i = 0, FindSign = 0;
	while(p[i].uCount && !FindSign)
	{
		// 在已有的选项中找到，在这里不返回是为了执行到函数结尾，可以得到最大值
		if(!FindSign)
		{
			if(p[i].posVertex[0] == posVertex1 && p[i].posVertex[1] == posVertex2 || p[i].posVertex[0] == posVertex2 && p[i].posVertex[1] == posVertex1)
			{
				p[i].ShareFaceNormal[p[i].uCount] = vecFaceNormal;
				p[i].uCount++;
				FindSign = 1;
			}
		}
		i++;
	}

	// 如果没有找到，就开一个新的
	if(!FindSign)
	{
		p[i].ShareFaceNormal[p[i].uCount] = vecFaceNormal;
		p[i].dwVertexIndex1 = Vertex1;
		p[i].dwVertexIndex2 = Vertex2;
		p[i].posVertex[0] = posVertex1;
		p[i].posVertex[1] = posVertex2;
		p[i].uCount++;
		i++;
	}

	// 计算当前有效的总边数
	for(i=0; p[i].uCount; i++);
	return i;
}


//FILE *fp=fopen("c:\\log.txt", "w");
//#define LOG(p) fwrite(p, strlen(p), 1, fp);
// 整个MESH必须为封闭的体，如果有不封闭的部分存在，该部分很可能会出现错误的效果
// 还有就是如果在一个文件中有多个MESH存在，用D3DXLOAD时会自动将所有MESH数据读入同一个MESH中，把这样的MESH传入该函数时在GenerateAdjancy时就会非法操作
HRESULT GenerateSilhouetteEdgeVS(LPD3DXMESH pMesh, LPD3DXMESH *ppVolumn)
{
	if(!pMesh || !ppVolumn)
		return D3DERR_INVALIDCALL;
	
	LPDIRECT3DVERTEXBUFFER9 pSourceVB=NULL, pDestVB=NULL;
	LPDIRECT3DINDEXBUFFER9 pSourceIB=NULL, pDestIB=NULL;
	D3DINDEXBUFFER_DESC Desc, VolumnDesc;
	char *pHeadSourceIB=NULL, *pHeadSourceVB=NULL;
	char *pHeadDestIB=NULL, *pHeadDestVB=NULL;
	DWORD i=0, j=0, x=0, z=0;
	DWORD dwFaceNum = pMesh->GetNumFaces(), dwVolumnFaceNum = 0;
	DWORD dwVertexNum = pMesh->GetNumVertices(), dwVolumnVertexNum = 0;
	DWORD dwVertexSize = D3DXGetFVFVertexSize(pMesh->GetFVF()), dwVolumnVertexSize = 0;
	DWORD dwQuadNum = 0;
	DWORD dwEdgeNum = 0;    // 不算重复的边的总数量
	DWORD dwVertex1=0, dwVertex2=0, dwVertex3=0;

	DWORD *pAdjancy = new DWORD[3*sizeof(DWORD)*dwFaceNum], *pVertexRep = new DWORD[dwVertexNum*sizeof(DWORD)];      // 忽略重复顶点的缓冲区

	if(!pAdjancy || !pVertexRep)
		return E_OUTOFMEMORY;
	
	pMesh->GenerateAdjacency(0.00001f, pAdjancy);
	pMesh->ConvertAdjacencyToPointReps(pAdjancy, pVertexRep);
	delete [] pAdjancy;
	
	// 得到两个BUFFER
	if(FAILED(pMesh->GetIndexBuffer(&pSourceIB)))
		return E_FAIL;
	if(FAILED(pMesh->GetVertexBuffer(&pSourceVB)))
		return E_FAIL;

	if(!pSourceIB || !pSourceVB || !dwFaceNum || !dwVertexNum)
		return E_FAIL;


	// 得到IB指针
	pSourceIB->GetDesc(&Desc);
	
	if(Desc.Format == D3DFMT_INDEX16)
		if(FAILED(pSourceIB->Lock(0, 2*dwFaceNum*3, (void **)&pHeadSourceIB, 0)))
			return E_FAIL;
	if(Desc.Format == D3DFMT_INDEX32)
		if(FAILED(pSourceIB->Lock(0, 4*dwFaceNum*3, (void **)&pHeadSourceIB, 0)))
			return E_FAIL;

	if(!pHeadSourceIB) return E_FAIL;

	// 得到VB指针
	if(FAILED(pSourceVB->Lock(0, dwVertexSize*dwVertexNum, (void **)&pHeadSourceVB, 0)))
		return E_FAIL;
	if(!pHeadSourceVB) return E_FAIL;


	// 分配MeshEdge（最大值，因为有重复的边，所以说有些是用不到的）
	MeshEdge *pEdge = new MeshEdge[dwFaceNum*3];

	// 遍历索引缓冲，置MeshEdge的值
	for(i=0; i<dwFaceNum; i++)
	{
		if(Desc.Format == D3DFMT_INDEX16)
		{
			dwVertex1 = pVertexRep[*((unsigned short int *)pHeadSourceIB+i*3)];
			dwVertex2 = pVertexRep[*((unsigned short int *)pHeadSourceIB+i*3+1)];
			dwVertex3 = pVertexRep[*((unsigned short int *)pHeadSourceIB+i*3+2)];
		}
		else if(Desc.Format == D3DFMT_INDEX32)
		{
			dwVertex1 = pVertexRep[*((DWORD *)pHeadSourceIB+i*3)];
			dwVertex2 = pVertexRep[*((DWORD *)pHeadSourceIB+i*3+1)];
			dwVertex3 = pVertexRep[*((DWORD *)pHeadSourceIB+i*3+2)];
		}
		// 得到顶点坐标，算面法线
		D3DXVECTOR3 posVertex1 = *((D3DXVECTOR3 *)(pHeadSourceVB+dwVertex1*dwVertexSize));
		D3DXVECTOR3 posVertex2 = *((D3DXVECTOR3 *)(pHeadSourceVB+dwVertex2*dwVertexSize));
		D3DXVECTOR3 posVertex3 = *((D3DXVECTOR3 *)(pHeadSourceVB+dwVertex3*dwVertexSize));

		D3DXVECTOR3 vecFaceNormal;
		D3DXVec3Cross(&vecFaceNormal, &(posVertex1-posVertex3), &(posVertex2-posVertex3));
		D3DXVec3Normalize(&vecFaceNormal, &vecFaceNormal);

		SetEdgeData(pEdge, dwVertex1, dwVertex2, posVertex1, posVertex2, vecFaceNormal);
		SetEdgeData(pEdge, dwVertex1, dwVertex3, posVertex1, posVertex3, vecFaceNormal);
		dwEdgeNum = SetEdgeData(pEdge, dwVertex2, dwVertex3, posVertex2, posVertex3, vecFaceNormal);     // 总是得到最后一个（即最终）的边总数
	}


	// 得到共享边的总数，用于构建Degenerate Quad，如果dwQuadNum为0，就奇了怪了，难道只是一个三角形？？
	for(i=0,dwQuadNum=0; i<dwEdgeNum; i++)
		if(pEdge[i].uCount > 1)
			dwQuadNum++;

	if(!dwQuadNum)
		return E_FAIL;

	// 新建顶点缓冲和索引缓冲并得到该缓冲
	dwVolumnFaceNum = dwFaceNum + dwQuadNum * 2;
	dwVolumnVertexNum = dwVertexNum + dwQuadNum * 4;

	D3DXCreateMesh(dwVolumnFaceNum, dwVolumnVertexNum, D3DXMESH_WRITEONLY | D3DXMESH_MANAGED, MeshEdge::dclr, d3ddevice, ppVolumn);
	dwVolumnVertexSize = D3DXGetFVFVertexSize((*ppVolumn)->GetFVF());
	
	
	if(!ppVolumn)
		return E_FAIL;

	(*ppVolumn)->GetVertexBuffer(&pDestVB);
	(*ppVolumn)->GetIndexBuffer(&pDestIB);

	// 得到IB指针
	pDestIB->GetDesc(&VolumnDesc);
	
	if(VolumnDesc.Format == D3DFMT_INDEX16)
		if(FAILED(pDestIB->Lock(0, 2*dwVolumnFaceNum*3, (void **)&pHeadDestIB, 0)))
			return E_FAIL;
	if(VolumnDesc.Format == D3DFMT_INDEX32)
		if(FAILED(pDestIB->Lock(0, 4*dwVolumnFaceNum*3, (void **)&pHeadDestIB, 0)))
			return E_FAIL;
			
	if(!pHeadDestIB) return E_FAIL;

z = dwFaceNum;
x = dwVertexNum;
dwVertexNum = 0;
dwFaceNum = 0;
		// 先复制源MESH的部分，这部分可要可不要，只是为了能在不重绘原MESH的情况下绘制完整的模型＋边缘
	if(VolumnDesc.Format == D3DFMT_INDEX16)
	{
		memcpy(pHeadDestIB, pHeadSourceIB, 2*dwFaceNum*3);
	}
	if(VolumnDesc.Format == D3DFMT_INDEX32)
	{
		memcpy(pHeadDestIB, pHeadSourceIB, 4*dwFaceNum*3);
	}
	
		// 再置QUAD的索引
	for(i=0; i<dwQuadNum; i++)
	{
		if(VolumnDesc.Format == D3DFMT_INDEX16)
		{
			*((unsigned short int *)pHeadDestIB + dwFaceNum*3 + i*6 + 0) = (unsigned short int)(dwVertexNum + i*4 + 0);
			*((unsigned short int *)pHeadDestIB + dwFaceNum*3 + i*6 + 1) = (unsigned short int)(dwVertexNum + i*4 + 1);
			*((unsigned short int *)pHeadDestIB + dwFaceNum*3 + i*6 + 2) = (unsigned short int)(dwVertexNum + i*4 + 2);
			*((unsigned short int *)pHeadDestIB + dwFaceNum*3 + i*6 + 3) = (unsigned short int)(dwVertexNum + i*4 + 1);
			*((unsigned short int *)pHeadDestIB + dwFaceNum*3 + i*6 + 4) = (unsigned short int)(dwVertexNum + i*4 + 3);
			*((unsigned short int *)pHeadDestIB + dwFaceNum*3 + i*6 + 5) = (unsigned short int)(dwVertexNum + i*4 + 2);
		}
		if(VolumnDesc.Format == D3DFMT_INDEX32)
		{
			*((DWORD *)pHeadDestIB + dwFaceNum*3 + i*6 + 0) = dwVertexNum + i*4 + 0;
			*((DWORD *)pHeadDestIB + dwFaceNum*3 + i*6 + 1) = dwVertexNum + i*4 + 1;
			*((DWORD *)pHeadDestIB + dwFaceNum*3 + i*6 + 2) = dwVertexNum + i*4 + 2;
			*((DWORD *)pHeadDestIB + dwFaceNum*3 + i*6 + 3) = dwVertexNum + i*4 + 1;
			*((DWORD *)pHeadDestIB + dwFaceNum*3 + i*6 + 4) = dwVertexNum + i*4 + 3;
			*((DWORD *)pHeadDestIB + dwFaceNum*3 + i*6 + 5) = dwVertexNum + i*4 + 2;
		}
	}
dwFaceNum = z;


	// 得到VB指针
	if(FAILED(pDestVB->Lock(0, dwVolumnVertexSize*dwVolumnVertexNum, (void **)&pHeadDestVB, 0)))
		return E_FAIL;
	if(!pHeadDestVB) return E_FAIL;

		// 拷贝原来的MESH顶点数据
	for(i=0; i<dwVertexNum; i++)
	{
		*((D3DXVECTOR3 *)pHeadDestVB + i*5 + 0) = *((D3DXVECTOR3 *)(pHeadSourceVB + i*dwVertexSize));
		*((D3DXVECTOR3 *)pHeadDestVB + i*5 + 1) = D3DXVECTOR3(0, 0, 0);
		*((D3DXVECTOR3 *)pHeadDestVB + i*5 + 2) = D3DXVECTOR3(0, 0, 0);
		*((D3DXVECTOR3 *)pHeadDestVB + i*5 + 3) = D3DXVECTOR3(0, 0, 0);
		*((D3DXVECTOR3 *)pHeadDestVB + i*5 + 4) = D3DXVECTOR3(0, 0, 0);
	}


		// 拷贝QUAD
	for(i=0,j=0; i<dwEdgeNum; i++)
		if(pEdge[i].uCount > 1)
		{
			// 拷贝QUAD的四个顶点数据，顺序：不扩展的顶点1,扩展的顶点1,不扩展的顶点2,扩展的顶点2
				// 不能扩展的顶点1
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 0) = pEdge[i].posVertex[0];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 1) = *((D3DXVECTOR3 *)(pHeadSourceVB + pEdge[i].dwVertexIndex1*dwVertexSize + sizeof(D3DXVECTOR3)));
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 2) = D3DXVECTOR3(0, 0, 0);
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 3) = pEdge[i].ShareFaceNormal[1];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 4) = pEdge[i].ShareFaceNormal[0];
			

				// 不能扩展的顶点2
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 5) = pEdge[i].posVertex[0];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 6) = *((D3DXVECTOR3 *)(pHeadSourceVB + pEdge[i].dwVertexIndex1*dwVertexSize + sizeof(D3DXVECTOR3)));
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 7) = D3DXVECTOR3(1, 1, 1);
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 8) = pEdge[i].ShareFaceNormal[1];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 9) = pEdge[i].ShareFaceNormal[0];
			
			
				// 不能扩展的顶点3
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 10) = pEdge[i].posVertex[1];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 11) = *((D3DXVECTOR3 *)(pHeadSourceVB + pEdge[i].dwVertexIndex2*dwVertexSize + sizeof(D3DXVECTOR3)));
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 12) = D3DXVECTOR3(0, 0, 0);
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 13) = pEdge[i].ShareFaceNormal[1];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 14) = pEdge[i].ShareFaceNormal[0];
			
				// 不能扩展的顶点4
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 15) = pEdge[i].posVertex[1];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 16) = *((D3DXVECTOR3 *)(pHeadSourceVB + pEdge[i].dwVertexIndex2*dwVertexSize + sizeof(D3DXVECTOR3)));
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 17) = D3DXVECTOR3(1, 1, 1);
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 18) = pEdge[i].ShareFaceNormal[1];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 19) = pEdge[i].ShareFaceNormal[0];
			j++;
		}
dwVertexNum=z;
	
	
	// 全部结束
	pDestIB->Unlock();
	pDestVB->Unlock();
	pSourceIB->Unlock();
	pSourceVB->Unlock();
	


	// 得到IB指针
	pDestIB->GetDesc(&VolumnDesc);
	
	if(VolumnDesc.Format == D3DFMT_INDEX16)
		if(FAILED(pDestIB->Lock(0, 2*dwVolumnFaceNum*3, (void **)&pHeadDestIB, 0)))
			return E_FAIL;
	if(VolumnDesc.Format == D3DFMT_INDEX32)
		if(FAILED(pDestIB->Lock(0, 4*dwVolumnFaceNum*3, (void **)&pHeadDestIB, 0)))
			return E_FAIL;
			
	// 得到VB指针
	if(FAILED(pDestVB->Lock(0, dwVolumnVertexSize*dwVolumnVertexNum, (void **)&pHeadDestVB, 0)))
		return E_FAIL;
	if(!pHeadDestVB) return E_FAIL;


/*
	sprintf(a, "原MESH有%d个顶点，有%d条不重复边，新VOLUMN有%d个顶点，顶点缓冲具体如下：\n\n", dwVertexNum, dwQuadNum, dwVolumnVertexNum);
	LOG(a);
	for(i=0; i<dwVolumnVertexNum; i++)
	{
		sprintf(a, "第%d个顶点：\n", i);
		LOG(a);
		aa = *((D3DXVECTOR3 *)pHeadDestVB + i*4);
		sprintf(a, "顶点坐标为： %f,%f,%f\n", aa.x, aa.y, aa.z);
		LOG(a);
		aa = *((D3DXVECTOR3 *)pHeadDestVB + i*4 + 1);
		sprintf(a, "标记为： %f,%f,%f\n", aa.x, aa.y, aa.z);
		LOG(a);
		aa = *((D3DXVECTOR3 *)pHeadDestVB + i*4 + 2);
		sprintf(a, "面法线1为： %f,%f,%f\n", aa.x, aa.y, aa.z);
		LOG(a);
		aa = *((D3DXVECTOR3 *)pHeadDestVB + i*4 + 3);
		sprintf(a, "面法线2为： %f,%f,%f\n", aa.x, aa.y, aa.z);
		LOG(a);
		LOG("\n");
	}
	
	sprintf(a, "原MESH有%d个面，有%d条不重复边，新VOLUMN有%d个面，索引缓冲具体如下：\n\n", dwFaceNum, dwQuadNum, dwVolumnFaceNum);
	LOG(a);
	unsigned short int aaa;
	for(i=0; i<dwVolumnFaceNum; i++)
	{
		sprintf(a, "第%d个面：\n", i);
		LOG(a);
		aaa = *((unsigned short int *)pHeadDestIB + i*3 + 0);
		sprintf(a, "索引为： %d, ", aaa);
		LOG(a);
		aaa = *((unsigned short int *)pHeadDestIB + i*3 + 1);
		sprintf(a, "%d, ", aaa);
		LOG(a);
		aaa = *((unsigned short int *)pHeadDestIB + i*3 + 2);
		sprintf(a, "%d\n", aaa);
		LOG(a);
		LOG("\n");
	}


	fclose(fp);
/*	sprintf(a, "一共有%d个顶点，%d个面，%d条不重复边\n\n", dwVertexNum, dwFaceNum, dwEdgeNum);
	LOG(a);
	for(i=0; i<dwEdgeNum; i++)
	{
		sprintf(a, "第%d条边：出现%d次\n", i, pEdge[i].uCount);
		LOG(a);
		sprintf(a, "顶点1为%d： %f,%f,%f\n", pEdge[i].dwVertexIndex1, pEdge[i].posVertex[0].x, pEdge[i].posVertex[0].y, pEdge[i].posVertex[0].z);
		LOG(a);
		sprintf(a, "顶点2为%d： %f,%f,%f\n", pEdge[i].dwVertexIndex2, pEdge[i].posVertex[1].x, pEdge[i].posVertex[1].y, pEdge[i].posVertex[1].z);
		LOG(a);
		sprintf(a, "面法线1为： %f,%f,%f\n", pEdge[i].ShareFaceNormal[0].x, pEdge[i].ShareFaceNormal[0].y, pEdge[i].ShareFaceNormal[0].z);
		LOG(a);
		sprintf(a, "面法线2为： %f,%f,%f\n", pEdge[i].ShareFaceNormal[1].x, pEdge[i].ShareFaceNormal[1].y, pEdge[i].ShareFaceNormal[1].z);
		LOG(a);
		LOG("\n");
	}
	fclose(fp);
*/
	delete [] pEdge;
	delete [] pVertexRep;
	pDestIB->Unlock();
	pDestVB->Unlock();
	pDestIB->Release();
	pDestVB->Release();
	pSourceIB->Release();
	pSourceVB->Release();
	
	return S_OK;
}



































// 方便写入VB，但它并不直接作为数据使用
struct SHADOWVERT
{
    D3DXVECTOR3 Position;
    D3DXVECTOR3 Normal;
	
    const static D3DVERTEXELEMENT9 Decl[3];
};

const D3DVERTEXELEMENT9 SHADOWVERT::Decl[3] =
{
    { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
    D3DDECL_END()
};

#define ADJACENCY_EPSILON 0.0001f
#define EXTRUDE_EPSILON 0.1f


struct CEdgeMapping
{
    int m_anOldEdge[2];  // 该边对应在原MESH中的两顶点索引号（PtRep过）
    int m_aanNewEdge[2][2]; // vertex indexes of the new edge
	// newEdge[0][0/1]：第一个共享边在原MESH中的索引缓冲号，就是说它分别存放该边两顶点在原MESH中的面顶点序号值（晕菜了我也不知道怎么说），就是说它存放Indices[i*3+j]中的i*3+j这个值，不知道为啥不用Indices[i*3+j]，怪异
	// newEdge[1][0/1]：第二个共享边……，对这两条共享边构造一个QUAD（平行的两条边嘛）
	// 新MESH的顶点缓冲，依次由原MESH每个面的三个顶点数据来填充，索引缓冲，以面为单位，先是渲染一个原MESH的面，后面就由QUAD组成，在遍历原MESH的每个面时，发现有一条共享边（即同一条边出现了两次）的，就写入新MESH，当然是通过PtRep的索引值来判断的（经典啊）
	// First subscript = index of the new edge
	// Second subscript = index of the vertex for the edge
	
public:
    CEdgeMapping()
    {
        FillMemory( m_anOldEdge, sizeof(m_anOldEdge), -1 );
        FillMemory( m_aanNewEdge, sizeof(m_aanNewEdge), -1 );
    }
};



// 只是返回有效的索引值罢了，还未分配的也算，或已有的匹配的也算
int FindEdgeInMappingTable( int nV1, int nV2, CEdgeMapping *pMapping, int nCount )
{
    for( int i = 0; i < nCount; ++i )
    {
        // If both vertex indexes of the old edge in mapping entry are -1, then
        // we have searched every valid entry without finding a match.  Return
        // this index as a newly created entry.
        if( ( pMapping[i].m_anOldEdge[0] == -1 && pMapping[i].m_anOldEdge[1] == -1 ) ||
			
            // Or if we find a match, return the index.
            ( pMapping[i].m_anOldEdge[1] == nV1 && pMapping[i].m_anOldEdge[0] == nV2 ) )
        {
            return i;
        }
    }
	
    return -1;  // We should never reach this line
}

HRESULT GenerateShadowMesh( ID3DXMesh *pMesh, ID3DXMesh **ppOutMesh )
{
    HRESULT hr = S_OK;
    ID3DXMesh *pInputMesh;

    if( !ppOutMesh )
        return E_INVALIDARG;
    *ppOutMesh = NULL;

    // Convert the input mesh to a format same as the output mesh using 32-bit index.
    hr = pMesh->CloneMesh( D3DXMESH_32BIT | D3DXMESH_MANAGED, SHADOWVERT::Decl, d3ddevice, &pInputMesh );
    if( FAILED( hr ) )
        return hr;

    //DXUTTRACE( L"Input mesh has %u vertices, %u faces\n", pInputMesh->GetNumVertices(), pInputMesh->GetNumFaces() );

    // Generate adjacency information
    DWORD *pdwAdj = new DWORD[3 * pInputMesh->GetNumFaces()];
    DWORD *pdwPtRep = new DWORD[pInputMesh->GetNumVertices()];
    if( !pdwAdj || !pdwPtRep )
    {
        delete[] pdwAdj; delete[] pdwPtRep;
        pInputMesh->Release();
        return E_OUTOFMEMORY;
    }

    hr = pInputMesh->GenerateAdjacency( ADJACENCY_EPSILON, pdwAdj );
    if( FAILED( hr ) )
    {
        delete[] pdwAdj; delete[] pdwPtRep;
        pInputMesh->Release();
        return hr;
    }

    pInputMesh->ConvertAdjacencyToPointReps( pdwAdj, pdwPtRep );
    delete[] pdwAdj;

    SHADOWVERT *pVBData = NULL;
    DWORD *pdwIBData = NULL;

    pInputMesh->LockVertexBuffer( 0, (LPVOID*)&pVBData );
    pInputMesh->LockIndexBuffer( 0, (LPVOID*)&pdwIBData );

    if( pVBData && pdwIBData )
    {
        // Maximum number of unique edges = Number of faces * 3
        DWORD dwNumEdges = pInputMesh->GetNumFaces() * 3;
        CEdgeMapping *pMapping = new CEdgeMapping[dwNumEdges];
        if( pMapping )
        {
            int nNumMaps = 0;  // Number of entries that exist in pMapping

            // Create a new mesh
            ID3DXMesh *pNewMesh;
            hr = D3DXCreateMesh( pInputMesh->GetNumFaces() + dwNumEdges * 2,      //因为索引缓冲里面还要画一次原MESH的面，所以要加NumFaces，dwNumEdges*2，每个面都有三条边，每条边都要扩展为两个面（QUAD），所以就是Faces * 3 * 2
                                 pInputMesh->GetNumFaces() * 3,
                                 D3DXMESH_WRITEONLY | D3DXMESH_32BIT,
                                 SHADOWVERT::Decl,
                                 d3ddevice,
                                 &pNewMesh );
            if( SUCCEEDED( hr ) )
            {
                SHADOWVERT *pNewVBData = NULL;
                DWORD *pdwNewIBData = NULL;

                pNewMesh->LockVertexBuffer( 0, (LPVOID*)&pNewVBData );
                pNewMesh->LockIndexBuffer( 0, (LPVOID*)&pdwNewIBData );

                // 用来写新IB的索引号，递增
                int nNextIndex = 0;

                if( pNewVBData && pdwNewIBData )
                {
                    ZeroMemory( pNewVBData, pNewMesh->GetNumVertices() * pNewMesh->GetNumBytesPerVertex() );
                    ZeroMemory( pdwNewIBData, sizeof(DWORD) * pNewMesh->GetNumFaces() * 3 );

                    // 用来写新VB的指针，递增
                    SHADOWVERT *pNextOutVertex = pNewVBData;

                    // Iterate through the faces.  For each face, output new
                    // vertices and face in the new mesh, and write its edges
                    // to the mapping table.

                    for( UINT f = 0; f < pInputMesh->GetNumFaces(); ++f )
                    {
                        // Copy the vertex data for all 3 vertices
                        CopyMemory( pNextOutVertex, pVBData + pdwIBData[f * 3], sizeof(SHADOWVERT) );
                        CopyMemory( pNextOutVertex + 1, pVBData + pdwIBData[f * 3 + 1], sizeof(SHADOWVERT) );
                        CopyMemory( pNextOutVertex + 2, pVBData + pdwIBData[f * 3 + 2], sizeof(SHADOWVERT) );

                        // Write out the face
                        pdwNewIBData[nNextIndex++] = f * 3;
                        pdwNewIBData[nNextIndex++] = f * 3 + 1;
                        pdwNewIBData[nNextIndex++] = f * 3 + 2;

                        // Compute the face normal and assign it to
                        // the normals of the vertices.
                        D3DXVECTOR3 v1, v2;  // v1 and v2 are the edge vectors of the face
                        D3DXVECTOR3 vNormal;
                        v1 = *(D3DXVECTOR3*)(pNextOutVertex + 1) - *(D3DXVECTOR3*)pNextOutVertex;
                        v2 = *(D3DXVECTOR3*)(pNextOutVertex + 2) - *(D3DXVECTOR3*)(pNextOutVertex + 1);
                        D3DXVec3Cross( &vNormal, &v1, &v2 );
                        D3DXVec3Normalize( &vNormal, &vNormal );

						// 在这将每个三角形原封不动存到pNextOutVertex中，只是将每个顶点的法线换成面法线（三个都相同）
                        pNextOutVertex->Normal = vNormal;
                        (pNextOutVertex + 1)->Normal = vNormal;
                        (pNextOutVertex + 2)->Normal = vNormal;

                        pNextOutVertex += 3;

                        // Add the face's edges to the edge mapping table

                        // Edge 1
                        int nIndex;
                        int nVertIndex[3] = { pdwPtRep[pdwIBData[f * 3]],
                                              pdwPtRep[pdwIBData[f * 3 + 1]],
                                              pdwPtRep[pdwIBData[f * 3 + 2]] };
                        nIndex = FindEdgeInMappingTable( nVertIndex[0], nVertIndex[1], pMapping, dwNumEdges );

                        // If error, we are not able to proceed, so abort.
                        if( -1 == nIndex )
                        {
                            hr = E_INVALIDARG;
                            goto cleanup;
                        }

                        if( pMapping[nIndex].m_anOldEdge[0] == -1 && pMapping[nIndex].m_anOldEdge[1] == -1 )
                        {
                            // No entry for this edge yet.  Initialize one.
                            pMapping[nIndex].m_anOldEdge[0] = nVertIndex[0];
                            pMapping[nIndex].m_anOldEdge[1] = nVertIndex[1];
                            pMapping[nIndex].m_aanNewEdge[0][0] = f * 3;
                            pMapping[nIndex].m_aanNewEdge[0][1] = f * 3 + 1;

                            ++nNumMaps;
                        } else
                        {
                            // An entry is found for this edge.  Create
                            // a quad and output it.
                            //assert( nNumMaps > 0 );

                            pMapping[nIndex].m_aanNewEdge[1][0] = f * 3;      // For clarity
                            pMapping[nIndex].m_aanNewEdge[1][1] = f * 3 + 1;

                            // First triangle
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][1];
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];

                            // Second triangle
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][1];
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];

                            // pMapping[nIndex] is no longer needed. Copy the last map entry
                            // over and decrement the map count.

                            pMapping[nIndex] = pMapping[nNumMaps-1];
                            FillMemory( &pMapping[nNumMaps-1], sizeof( pMapping[nNumMaps-1] ), 0xFF );
                            --nNumMaps;
                        }

                        // Edge 2
                        nIndex = FindEdgeInMappingTable( nVertIndex[1], nVertIndex[2], pMapping, dwNumEdges );

                        // If error, we are not able to proceed, so abort.
                        if( -1 == nIndex )
                        {
                            hr = E_INVALIDARG;
                            goto cleanup;
                        }

                        if( pMapping[nIndex].m_anOldEdge[0] == -1 && pMapping[nIndex].m_anOldEdge[1] == -1 )
                        {
                            pMapping[nIndex].m_anOldEdge[0] = nVertIndex[1];
                            pMapping[nIndex].m_anOldEdge[1] = nVertIndex[2];
                            pMapping[nIndex].m_aanNewEdge[0][0] = f * 3 + 1;
                            pMapping[nIndex].m_aanNewEdge[0][1] = f * 3 + 2;

                            ++nNumMaps;
                        } else
                        {
                            // An entry is found for this edge.  Create
                            // a quad and output it.
                            //assert( nNumMaps > 0 );

                            pMapping[nIndex].m_aanNewEdge[1][0] = f * 3 + 1;
                            pMapping[nIndex].m_aanNewEdge[1][1] = f * 3 + 2;

                            // First triangle
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][1];
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];

                            // Second triangle
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][1];
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];

                            // pMapping[nIndex] is no longer needed. Copy the last map entry
                            // over and decrement the map count.

                            pMapping[nIndex] = pMapping[nNumMaps-1];
                            FillMemory( &pMapping[nNumMaps-1], sizeof( pMapping[nNumMaps-1] ), 0xFF );
                            --nNumMaps;
                        }

                        // Edge 3
                        nIndex = FindEdgeInMappingTable( nVertIndex[2], nVertIndex[0], pMapping, dwNumEdges );

                        // If error, we are not able to proceed, so abort.
                        if( -1 == nIndex )
                        {
                            hr = E_INVALIDARG;
                            goto cleanup;
                        }

                        if( pMapping[nIndex].m_anOldEdge[0] == -1 && pMapping[nIndex].m_anOldEdge[1] == -1 )
                        {
                            pMapping[nIndex].m_anOldEdge[0] = nVertIndex[2];
                            pMapping[nIndex].m_anOldEdge[1] = nVertIndex[0];
                            pMapping[nIndex].m_aanNewEdge[0][0] = f * 3 + 2;
                            pMapping[nIndex].m_aanNewEdge[0][1] = f * 3;

                            ++nNumMaps;
                        } else
                        {
                            // An entry is found for this edge.  Create
                            // a quad and output it.
                            //assert( nNumMaps > 0 );

                            pMapping[nIndex].m_aanNewEdge[1][0] = f * 3 + 2;
                            pMapping[nIndex].m_aanNewEdge[1][1] = f * 3;

                            // First triangle
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][1];
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];

                            // Second triangle
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][1];
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[1][0];
                            pdwNewIBData[nNextIndex++] = pMapping[nIndex].m_aanNewEdge[0][0];

                            // pMapping[nIndex] is no longer needed. Copy the last map entry
                            // over and decrement the map count.

                            pMapping[nIndex] = pMapping[nNumMaps-1];
                            FillMemory( &pMapping[nNumMaps-1], sizeof( pMapping[nNumMaps-1] ), 0xFF );
                            --nNumMaps;
                        }
                    }

                    // Now the entries in the edge mapping table represent
                    // non-shared edges.  What they mean is that the original
                    // mesh has openings (holes), so we attempt to patch them.
                    // First we need to recreate our mesh with a larger vertex
                    // and index buffers so the patching geometry could fit.

                    //DXUTTRACE( L"Faces to patch: %d\n", nNumMaps );

                    // Create a mesh with large enough vertex and
                    // index buffers.

                    SHADOWVERT *pPatchVBData = NULL;
                    DWORD *pdwPatchIBData = NULL;

                    ID3DXMesh *pPatchMesh = NULL;
                    // Make enough room in IB for the face and up to 3 quads for each patching face
                    hr = D3DXCreateMesh( nNextIndex / 3 + nNumMaps * 7,
                                         ( pInputMesh->GetNumFaces() + nNumMaps ) * 3,
                                         D3DXMESH_WRITEONLY | D3DXMESH_32BIT,
                                         SHADOWVERT::Decl,
                                         d3ddevice,
                                         &pPatchMesh );

                    if( FAILED( hr ) )
                        goto cleanup;

                    hr = pPatchMesh->LockVertexBuffer( 0, (LPVOID*)&pPatchVBData );
                    if( SUCCEEDED( hr ) )
                        hr = pPatchMesh->LockIndexBuffer( 0, (LPVOID*)&pdwPatchIBData );

                    if( pPatchVBData && pdwPatchIBData )
                    {
                        ZeroMemory( pPatchVBData, sizeof(SHADOWVERT) * ( pInputMesh->GetNumFaces() + nNumMaps ) * 3 );
                        ZeroMemory( pdwPatchIBData, sizeof(DWORD) * ( nNextIndex + 3 * nNumMaps * 7 ) );

                        // Copy the data from one mesh to the other

                        CopyMemory( pPatchVBData, pNewVBData, sizeof(SHADOWVERT) * pInputMesh->GetNumFaces() * 3 );
                        CopyMemory( pdwPatchIBData, pdwNewIBData, sizeof(DWORD) * nNextIndex );
                    } else
                    {
                        // Some serious error is preventing us from locking.
                        // Abort and return error.

                        pPatchMesh->Release();
                        goto cleanup;
                    }

                    // Replace pNewMesh with the updated one.  Then the code
                    // can continue working with the pNewMesh pointer.

                    pNewMesh->UnlockVertexBuffer();
                    pNewMesh->UnlockIndexBuffer();
                    pNewVBData = pPatchVBData;
                    pdwNewIBData = pdwPatchIBData;
                    pNewMesh->Release();
                    pNewMesh = pPatchMesh;

                    // Now, we iterate through the edge mapping table and
                    // for each shared edge, we generate a quad.
                    // For each non-shared edge, we patch the opening
                    // with new faces.

                    // nNextVertex is the index of the next vertex.
                    int nNextVertex = pInputMesh->GetNumFaces() * 3;

                    for( int i = 0; i < nNumMaps; ++i )
                    {
                        if( pMapping[i].m_anOldEdge[0] != -1 &&
                            pMapping[i].m_anOldEdge[1] != -1 )
                        {
                            // 到这一步，因为未用的项都是填入-1的，所以只要第二条共享边为-1就说明该边未被共享，就要patch
                            if( pMapping[i].m_aanNewEdge[1][0] == -1 ||
                                pMapping[i].m_aanNewEdge[1][1] == -1 )
                            {
                                // Find another non-shared edge that
                                // shares a vertex with the current edge.
                                for( int i2 = i + 1; i2 < nNumMaps; ++i2 )
                                {
                                    if( pMapping[i2].m_anOldEdge[0] != -1 &&       // must have a valid old edge
                                        pMapping[i2].m_anOldEdge[1] != -1 &&
                                        ( pMapping[i2].m_aanNewEdge[1][0] == -1 || // must have only one new edge
                                        pMapping[i2].m_aanNewEdge[1][1] == -1 ) )
                                    {
                                        int nVertShared = 0;
                                        if( pMapping[i2].m_anOldEdge[0] == pMapping[i].m_anOldEdge[1] )
                                            ++nVertShared;
                                        if( pMapping[i2].m_anOldEdge[1] == pMapping[i].m_anOldEdge[0] )
                                            ++nVertShared;

                                        if( 2 == nVertShared )
                                        {
											// 这一步真的很奇怪，怎么可能找到nVertShared为2的边？2就代表i2跟i是同一条边，既然同一条那就是共享边，i又怎么可能在前面被标记成没共享边呢？
                                            // These are the last two edges of this particular
                                            // opening. Mark this edge as shared so that a degenerate
                                            // quad can be created for it.

                                            pMapping[i2].m_aanNewEdge[1][0] = pMapping[i].m_aanNewEdge[0][0];
                                            pMapping[i2].m_aanNewEdge[1][1] = pMapping[i].m_aanNewEdge[0][1];
                                            break;
                                        }
                                        else
                                        if( 1 == nVertShared )
                                        {
											// 这才对嘛，找到i2跟i只是共享一个顶点而已，下面是判断具体共享哪个顶点
                                            // nBefore and nAfter tell us which edge comes before the other.
                                            int nBefore, nAfter;
                                            if( pMapping[i2].m_anOldEdge[0] == pMapping[i].m_anOldEdge[1] )
                                            {
                                                nBefore = i;
                                                nAfter = i2;
                                            } else
                                            {
                                                nBefore = i2;
                                                nAfter = i;
                                            }

                                            // Found such an edge. Now create a face along with two
                                            // degenerate quads from these two edges.

                                            pNewVBData[nNextVertex] = pNewVBData[pMapping[nAfter].m_aanNewEdge[0][1]];
                                            pNewVBData[nNextVertex+1] = pNewVBData[pMapping[nBefore].m_aanNewEdge[0][1]];
                                            pNewVBData[nNextVertex+2] = pNewVBData[pMapping[nBefore].m_aanNewEdge[0][0]];
                                            // Recompute the normal
                                            D3DXVECTOR3 v1 = pNewVBData[nNextVertex+1].Position - pNewVBData[nNextVertex].Position;
                                            D3DXVECTOR3 v2 = pNewVBData[nNextVertex+2].Position - pNewVBData[nNextVertex+1].Position;
                                            D3DXVec3Normalize( &v1, &v1 );
                                            D3DXVec3Normalize( &v2, &v2 );
                                            D3DXVec3Cross( &pNewVBData[nNextVertex].Normal, &v1, &v2 );
                                            pNewVBData[nNextVertex+1].Normal = pNewVBData[nNextVertex+2].Normal = pNewVBData[nNextVertex].Normal;

                                            pdwNewIBData[nNextIndex] = nNextVertex;
                                            pdwNewIBData[nNextIndex+1] = nNextVertex + 1;
                                            pdwNewIBData[nNextIndex+2] = nNextVertex + 2;

                                            // 1st quad

                                            pdwNewIBData[nNextIndex+3] = pMapping[nBefore].m_aanNewEdge[0][1];
                                            pdwNewIBData[nNextIndex+4] = pMapping[nBefore].m_aanNewEdge[0][0];
                                            pdwNewIBData[nNextIndex+5] = nNextVertex + 1;

                                            pdwNewIBData[nNextIndex+6] = nNextVertex + 2;
                                            pdwNewIBData[nNextIndex+7] = nNextVertex + 1;
                                            pdwNewIBData[nNextIndex+8] = pMapping[nBefore].m_aanNewEdge[0][0];

                                            // 2nd quad

                                            pdwNewIBData[nNextIndex+9] = pMapping[nAfter].m_aanNewEdge[0][1];
                                            pdwNewIBData[nNextIndex+10] = pMapping[nAfter].m_aanNewEdge[0][0];
                                            pdwNewIBData[nNextIndex+11] = nNextVertex;

                                            pdwNewIBData[nNextIndex+12] = nNextVertex + 1;
                                            pdwNewIBData[nNextIndex+13] = nNextVertex;
                                            pdwNewIBData[nNextIndex+14] = pMapping[nAfter].m_aanNewEdge[0][0];

                                            // Modify mapping entry i2 to reflect the third edge
                                            // of the newly added face.

                                            if( pMapping[i2].m_anOldEdge[0] == pMapping[i].m_anOldEdge[1] )
                                            {
                                                pMapping[i2].m_anOldEdge[0] = pMapping[i].m_anOldEdge[0];
                                            } else
                                            {
                                                pMapping[i2].m_anOldEdge[1] = pMapping[i].m_anOldEdge[1];
                                            }
                                            pMapping[i2].m_aanNewEdge[0][0] = nNextVertex + 2;
                                            pMapping[i2].m_aanNewEdge[0][1] = nNextVertex;

                                            // Update next vertex/index positions

                                            nNextVertex += 3;
                                            nNextIndex += 15;

                                            break;
                                        }
                                    }
                                }
                            } else
                            {
                                // This is a shared edge.  Create the degenerate quad.

                                // First triangle
                                pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[0][1];
                                pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[0][0];
                                pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[1][0];

                                // Second triangle
                                pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[1][1];
                                pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[1][0];
                                pdwNewIBData[nNextIndex++] = pMapping[i].m_aanNewEdge[0][0];
                            }
                        }
                    }
                }

cleanup:;
                if( pNewVBData )
                {
                    pNewMesh->UnlockVertexBuffer();
                    pNewVBData = NULL;
                }
                if( pdwNewIBData )
                {
                    pNewMesh->UnlockIndexBuffer();
                    pdwNewIBData = NULL;
                }

                if( SUCCEEDED( hr ) )
                {
                    // At this time, the output mesh may have an index buffer
                    // bigger than what is actually needed, so we create yet
                    // another mesh with the exact IB size that we need and
                    // output it.  This mesh also uses 16-bit index if
                    // 32-bit is not necessary.

                    //DXUTTRACE( L"Shadow volume has %u vertices, %u faces.\n", ( pInputMesh->GetNumFaces() + nNumMaps ) * 3, nNextIndex / 3 );

                    bool bNeed32Bit = ( pInputMesh->GetNumFaces() + nNumMaps ) * 3 > 65535;
                    ID3DXMesh *pFinalMesh;
                    hr = D3DXCreateMesh( nNextIndex / 3,  // Exact number of faces
                                         ( pInputMesh->GetNumFaces() + nNumMaps ) * 3,
                                         D3DXMESH_WRITEONLY | ( bNeed32Bit ? D3DXMESH_32BIT : 0 ),
                                         SHADOWVERT::Decl,
                                         d3ddevice,
                                         &pFinalMesh );
                    if( SUCCEEDED( hr ) )
                    {
                        pNewMesh->LockVertexBuffer( 0, (LPVOID*)&pNewVBData );
                        pNewMesh->LockIndexBuffer( 0, (LPVOID*)&pdwNewIBData );

                        SHADOWVERT *pFinalVBData = NULL;
                        WORD *pwFinalIBData = NULL;

                        pFinalMesh->LockVertexBuffer( 0, (LPVOID*)&pFinalVBData );
                        pFinalMesh->LockIndexBuffer( 0, (LPVOID*)&pwFinalIBData );

                        if( pNewVBData && pdwNewIBData && pFinalVBData && pwFinalIBData )
                        {
                            CopyMemory( pFinalVBData, pNewVBData, sizeof(SHADOWVERT) * ( pInputMesh->GetNumFaces() + nNumMaps ) * 3 );

                            if( bNeed32Bit )
                                CopyMemory( pwFinalIBData, pdwNewIBData, sizeof(DWORD) * nNextIndex );
                            else
                            {
                                for( int i = 0; i < nNextIndex; ++i )
                                    pwFinalIBData[i] = (WORD)pdwNewIBData[i];
                            }
                        }

                        if( pNewVBData )
                            pNewMesh->UnlockVertexBuffer();
                        if( pdwNewIBData )
                            pNewMesh->UnlockIndexBuffer();
                        if( pFinalVBData )
                            pFinalMesh->UnlockVertexBuffer();
                        if( pwFinalIBData )
                            pFinalMesh->UnlockIndexBuffer();

                        // Release the old
                        pNewMesh->Release();
                        pNewMesh = pFinalMesh;
                    }

                    *ppOutMesh = pNewMesh;
                }
                else
                    pNewMesh->Release();
            }
            delete[] pMapping;
        } else
            hr = E_OUTOFMEMORY;
    } else
        hr = E_FAIL;

    if( pVBData )
        pInputMesh->UnlockVertexBuffer();

    if( pdwIBData )
        pInputMesh->UnlockIndexBuffer();

    delete[] pdwPtRep;
    pInputMesh->Release();

    return hr;
}















VOID AddEdge( WORD* pEdges, DWORD& dwNumEdges, WORD v0, WORD v1 )
{
    // Remove interior edges (which appear in the list twice)
    for( DWORD i=0; i < dwNumEdges; i++ )
    {
        if( ( pEdges[2*i+0] == v0 && pEdges[2*i+1] == v1 ) ||
            ( pEdges[2*i+0] == v1 && pEdges[2*i+1] == v0 ) )
        {
            if( dwNumEdges > 1 )
            {
                pEdges[2*i+0] = pEdges[2*(dwNumEdges-1)+0];
                pEdges[2*i+1] = pEdges[2*(dwNumEdges-1)+1];
            }
            dwNumEdges--;
            return;
        }
    }

    pEdges[2*dwNumEdges+0] = v0;
    pEdges[2*dwNumEdges+1] = v1;
    dwNumEdges++;
}
