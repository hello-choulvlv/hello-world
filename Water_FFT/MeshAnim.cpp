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
	OutputDebugString("ʹ��HDR��������Ԥ�ȶ���FP16/FP32��");
	return E_FAIL;
#endif
	
	OutputDebugString("�ָ����ϵ�HDR CubeMap���ǵ�posz.hdrҪ��HDRShop��ת180�ȣ����ǰ�Y�ᵹ����");
	
	LPDIRECT3DTEXTURE9 pCubeTex = NULL, pTexture[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
	LPDIRECT3DSURFACE9 pSrcSurf = NULL, pDstSurf = NULL;
	int i = 0;
	char *p = NULL;
	
	// ����CubeMap��ʱ����
	V_RETURN(D3DXCreateTextureFromFileEx(d3ddevice, szCubeFileName, D3DX_DEFAULT, D3DX_DEFAULT, 1, D3DUSAGE_0, Format, D3DPOOL_DEFAULT, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, NULL, NULL, &pCubeTex))
		
		// ��ʼ�����ļ����������ļ�����neg pos xyz�����ָͬ������
		char *pKeyName[] = {"posx", "negx", "posy", "negy", "posz", "negz"};
	RECT Rect[6] = 
	{
		// X
		{dwDimension*2, dwDimension, dwDimension*3, dwDimension*2},	// �ڶ��ŵ�����
		{0, dwDimension, dwDimension, dwDimension*2},		// �ڶ��ŵ�һ��
		
		// Y
		{dwDimension, 0, dwDimension*2, dwDimension},		// ��һ�ŵڶ���
		{dwDimension, dwDimension*2, dwDimension*2, dwDimension*3},	// �����ŵڶ���
		
		// Z
		{dwDimension, dwDimension*3, dwDimension*2, dwDimension*4},	// �����ŵڶ���
		{dwDimension, dwDimension, dwDimension*2, dwDimension*2},	// �ڶ��ŵڶ���
	};
	
	
	char szFileName[100];

	for(i=0; i<6; i++)
	{
		// ��������
		if(FAILED(d3ddevice->CreateTexture(dwDimension, dwDimension, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &pTexture[i], NULL)))
			goto fail;
		
		// ��������
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
	//�ͷ�ÿ���Ӽ�������
	for(i=0; i<SubsetNum; i++) if(Texture[i]) SAFE_RELEASE(Texture[i]);
	//�ͷ�����ָ������
	if(Texture) delete[] Texture;
	//�ͷŲ�������
	if(Material) delete[] Material;
	//�ͷŶ��㻺��
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

	// �õ��ö��������
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
	LPD3DXMATERIAL materialstruct;  //��ʵ�����ָ�������MATERIALBUFFER�������
	//LPD3DXBUFFER adjacency;   //��ʾƽ���ڽӹ�ϵ
	
	DWORD i;
	LPD3DXBUFFER pAdjancy = NULL;
	//�Ӵ����ж�ȡXģ���ļ�
	if(!Mesh && !Material && !Texture && !SubsetNum && pFileName && d3ddevice)
	{
		if(FAILED(D3DXLoadMeshFromX(pFileName, D3DXMESH_MANAGED, d3ddevice, &pAdjancy, &materialbuffer, NULL, &SubsetNum, &Mesh)))
			return E_FAIL;
		// ǧ��С��VertexCache��StripeOrder�Ż����ڲ�ͬ���Կ����Ż�������Mesh����ͬ
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
		
		// ת��ΪBin���͵�X�ļ����������򵥼����ã�Ҳ���Լ�С�ļ�������������TEXT���͵�1/4������߶�ȡ�ٶ�
		//D3DXSaveMeshToX("SaveMeshFile_Binary.x", Mesh, NULL, materialstruct, NULL, SubsetNum, D3DXF_FILEFORMAT_BINARY);

		char pPureFileName[100];
		
		for(i=0;i<SubsetNum;i++)
		{	//���������Ϣ
			Material[i]=materialstruct[i].MatD3D;
			Material[i].Ambient=Material[i].Diffuse;  //D3D��ֵ���ƣ����Զ�������⸳ֵ�������Լ�����
			
			//����BUFFER�е������ļ���Ϣ����������������ļ������ھͰ�ָ���ÿ�
			if(pPathName)
			{
				// ����Զ�������·�����ͰѲ��������ļ����е�·��ȫȥ����ֻ�������ļ������ٺ��Զ���·�����
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
			
			// �Զ��������ʽ��Ч
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
		
		SAFE_RELEASE(materialbuffer);     //�ѽ�������Ϣת�浽material�У��ͷ���ʱ���ʻ���
		
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
	//Ҫ�ѷ��䣬���ͷ�
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

			//��������ȡʧ�ܻ�δ����������Ϊ���ʾǿ�����ã�����������Ϊ�����ʾ���ԣ������Զ�����ͼ����CUBEMAP��
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
			//��ʼ��Ⱦ
			d3ddevice->BeginScene();
			if(FAILED(Mesh->DrawSubset(i))) return E_FAIL;
			d3ddevice->EndScene();
*/
			// �����FVF
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

			// ��ʼ����
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











/*************************��������*************************/
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
				OutputDebugString("���棬ָ������Ĺ���Ӱ����������");
			iVertexMatrixNum++;
		}
	}

	// �Ƚ�����任�����ﶥ������λ��
	char *SrcPtr = NULL;
	if(FAILED(m_pMesh->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&SrcPtr)))
		return E_FAIL;
	SrcPtr += iVertexNo * m_pMesh->MeshData.pMesh->GetNumBytesPerVertex();

	float *pData = (float *)SrcPtr;
	D3DXVECTOR3 VecPosition = D3DXVECTOR3( *pData, *(pData+1), *(pData+2) );

	m_pMesh->MeshData.pMesh->UnlockVertexBuffer();

	// ���ҽ������õ��õ��Ӧ���ܾ�������������źͻ��ϵ����д��ṹ��
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
	
		// �õ���ʼʱ��͵�ǰʱ��֮�ms���������ж��Ƿ�����FPS�������������ֱ�ӻ��������¾���
	if(LastTime.QuadPart == 0)
		QueryPerformanceCounter(&LastTime);
	if(StartTime.QuadPart == 0)
		QueryPerformanceCounter(&StartTime);
	
	QueryPerformanceCounter(&CurrentTime);

	float fTime = (float)(CurrentTime.QuadPart - LastTime.QuadPart) / (float)PerformanceFrequency.QuadPart;
	if(fTime >= (float)m_fFPS)
	{
		// ����
		DWORD dwTime;
		if(m_bInverse) dwTime= (DWORD)((float)(StartTime.QuadPart - CurrentTime.QuadPart) / (float)PerformanceFrequency.QuadPart * m_fSpeed * 1000);
		else dwTime= (DWORD)((float)(CurrentTime.QuadPart - StartTime.QuadPart) / (float)PerformanceFrequency.QuadPart * m_fSpeed * 1000);
		QueryPerformanceCounter(&LastTime);
		SkinAnimation.Map(pAnimationSetName, iAnimationSetIndex, m_pFrame);
		SkinAnimation.SetAnimation(pAnimationSetName, iAnimationSetIndex, dwTime, m_bLoop);
		ParseFrame.UpdateHierarchy(m_pFrame, NULL);
		ParseFrame.UpdateMesh(m_pMesh);
	}

	// ��ʼ����
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

		// SkinMesh�ͻ�pSKinMesh����֮�ͻ�MeshData
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
	// ��Ч���ж�
	if(m_iStartConstantIndex>256 || !m_pFrame || !m_pMesh || !m_pMesh->pSkinInfo || !m_pMesh->pSplitMesh || !m_pMesh->dwMaxMatrixNum || m_CreateAttrib!=2 )
		return E_FAIL;
	
	// �õ���ʼʱ��͵�ǰʱ��֮�ms���������ж��Ƿ�����FPS�������������ֱ�ӻ��������¾���
	if(LastTime.QuadPart == 0)
		QueryPerformanceCounter(&LastTime);
	if(StartTime.QuadPart == 0)
		QueryPerformanceCounter(&StartTime);
	
	QueryPerformanceCounter(&CurrentTime);

	float fTime = (float)(CurrentTime.QuadPart - LastTime.QuadPart) / (float)PerformanceFrequency.QuadPart;
	if(fTime >= (float)m_fFPS)
	{
		// ����
		DWORD dwTime;
		if(m_bInverse) dwTime= (DWORD)((float)(StartTime.QuadPart - CurrentTime.QuadPart) / (float)PerformanceFrequency.QuadPart * m_fSpeed * 1000);
		else dwTime= (DWORD)((float)(CurrentTime.QuadPart - StartTime.QuadPart) / (float)PerformanceFrequency.QuadPart * m_fSpeed * 1000);
		QueryPerformanceCounter(&LastTime);
		SkinAnimation.Map(pAnimationSetName, iAnimationSetIndex, m_pFrame);
		SkinAnimation.SetAnimation(pAnimationSetName, iAnimationSetIndex, dwTime, m_bLoop);
		ParseFrame.UpdateHierarchy(m_pFrame, NULL);
	}


	// ��ʼ����
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
//����һ��ԭMESH����������ֵ����������������������SPLITMESH�ж�Ӧ�����������
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
	// ��dwMaxMatrixNumȡ�ܱ�4��������Сֵ���������������Ȩ�أ�Data Stream��
	DWORD dwMaxMatrixModNum = dwMaxMatrixNum + ((dwMaxMatrixNum%4) ? (4-dwMaxMatrixNum%4) : 0);
	char *p = NULL, *p1 = NULL; //��ʱʹ�ã�������仺����
	
	// ����ѷָ�������ͷŵ������·ָ�
	if(pMesh->dwSplitMeshNum || pMesh->pSplitMesh)
	{
		delete pMesh->pSplitMesh;
		pMesh->pSplitMesh = NULL;
		pMesh->dwSplitMeshNum = 0;
		pMesh->dwMaxMatrixNum = 0;
	}
	
	// �ȵõ����������е����ݣ���pSource�д���
	LPDIRECT3DINDEXBUFFER9 pIB;
	D3DINDEXBUFFER_DESC Desc;
	char *pHeadSource;
	pMesh->pSkinMesh->GetIndexBuffer(&pIB);
	pIB->GetDesc(&Desc);
	pIB->Lock(0, Desc.Size, (void **)&pHeadSource, D3DLOCK_READONLY);


	// �õ�VB�еĶ�����
	DWORD dwVertexNum = pMesh->pSkinMesh->GetNumVertices();
	
	// �õ�ÿ����ͷ��Ӱ��Ķ�������
	DWORD dwBoneNum = pMesh->pSkinInfo->GetNumBones();
	DWORD **pBoneVertics = new DWORD* [dwBoneNum];
	ZeroMemory(pBoneVertics, sizeof(DWORD *) * dwBoneNum);
	float **pWeights = new float* [dwBoneNum];
	ZeroMemory(pWeights, sizeof(float *) * dwBoneNum);
	DWORD *CurrentVertexNum = new DWORD[dwBoneNum];        // ÿ����ͷ��Ӱ��Ķ�����

	for(i=0; i<dwBoneNum; i++)
	{
		CurrentVertexNum[i] = pMesh->pSkinInfo->GetNumBoneInfluences(i);
		pBoneVertics[i] = new DWORD[CurrentVertexNum[i]];
		pWeights[i] = new float[CurrentVertexNum[i]];
		pMesh->pSkinInfo->GetBoneInfluence(i, pBoneVertics[i], pWeights[i]);
	}
	
	// �����ͷӰ��Ķ��������Ѷ��������Ĺ�ͷ�ŵ�ǰ�棨�Ӵ�С���У���ֻ���ڵ�һ�ַָ�ⲽ���Ż�����Ҫ
	// pBoneOptimizedMapping[i]�����ŵ�iλ���Ӵ�С���У��Ĺ�ͷ����Ӧ��ʵ��λ��
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
			
	// �õ�ÿ�������������
	DWORD dwFaceNum = pMesh->pSkinMesh->GetNumFaces();
	DWORD *pFaceID = new DWORD[dwFaceNum];
	
	    // �õ����Ա�
	DWORD dwAttributeTableNum = 0;
	pMesh->pSkinMesh->GetAttributeTable(NULL, &dwAttributeTableNum);
	D3DXATTRIBUTERANGE *pAttributeTable = new D3DXATTRIBUTERANGE[dwAttributeTableNum];
	ZeroMemory(pAttributeTable, sizeof(D3DXATTRIBUTERANGE) * dwAttributeTableNum);

	pMesh->pSkinMesh->GetAttributeTable(pAttributeTable, NULL);
		// ��ʼ�����Ա��������䵽��������ȥ
	for(i=0; i<dwAttributeTableNum; i++)
	{
		for(j=pAttributeTable[i].FaceStart; (j-pAttributeTable[i].FaceStart)<pAttributeTable[i].FaceCount; j++)
			pFaceID[j] = pAttributeTable[i].AttribId;
	}

	delete [] pAttributeTable;
	pAttributeTable = NULL;

	// ��ʼɨ��ÿ���棬����Ҫ��Ļ��ͼ���SplitMesh
	DWORD dwFaceMatrixNum;  // ��ǰ���������Ҫ�ľ��󣨹���������
	char *pFaceSign = new char[dwFaceNum];   // ���ĳ����ŵ����Ѵ棬���Ӧ��ŵĸ�����Ԫ�ؼ�Ϊ1����������ѡ�񱾴�ʹ����Щ��
	memset(pFaceSign, 0, dwFaceNum);
	char *pEachFaceSign = new char[dwFaceNum];   // ���ĳ����ŵ����Ѵ棬���Ӧ��ŵĸ�����Ԫ�ؼ�Ϊ1����FaceSign��ͬ������������ֻ��һ��MESH�У�����ͳ�Ƹ�SPLITMESH��������
	memset(pEachFaceSign, 0, dwFaceNum);
	char *pVertexSign = new char[dwVertexNum];   // ��Ӧ�ڶ��㻺������ÿ�����㣬���SPLITMESH���õ������ж��㣬���ڹ���IB���ظ��Ķ��㣬����SPLITMESH�Լ��Ķ���ṹ
	memset(pVertexSign, 0, dwVertexNum);
	DWORD *pVertexMapping = new DWORD[dwVertexNum];   // ��Ӧ��SPLITMESH��ÿ�����㣬�洢��MESH���㻺������λ�ã�����˵VertexMapping[0]�ʹ��SPLITMESH��һ��������ԭMESH�е�λ��
	memset(pVertexMapping, 0, dwVertexNum*sizeof(DWORD));
	DWORD VertexIndics = 0;
	SplitMesh *pSplitMesh = new SplitMesh;


//��MESHCONTAINER��ֵ
	pMesh->pSplitMesh = pSplitMesh;
	pMesh->dwMaxMatrixNum = dwMaxMatrixNum;
	
	//��һ�����ͣ�ѡ�����ָ���ľ������������ѡ��Ӱ�춥�����ĵ�0��15�������ε�����ѭ����һ�ξ��ǵ�16��31������ֱ��ѭ�������һ��Ϊֹ
	//ѭ��ɨ�����е�Subset��ɨ��һ�ξ͵��л�һ��SPLITMESH
	//ע������Ϊ��Ҫ��dwAttributeTableNum��ԭ����MaterialNum�����м�������ĳЩMATERAIL����ûһ��������õ��������Ա����Ŀǡǡ���ڶ������õ���SUBSET����Ŀ���������MATERIAL�޹�
	char pFaceVertexSign[3] = {0,0,0};
	DWORD dwBoneRangeMin = 0, dwBoneRangeMax = dwMaxMatrixNum;           //ÿ��ѭ���У���ǰ�Ĺ���������Χ

	for(n=0; n<dwAttributeTableNum; n++)
	{
		dwBoneRangeMin = 0; dwBoneRangeMax = dwMaxMatrixNum;
		dwBoneRangeMin = dwBoneRangeMin>dwBoneNum ? dwBoneNum : dwBoneRangeMin;
		dwBoneRangeMax = dwBoneRangeMax>dwBoneNum ? dwBoneNum : dwBoneRangeMax;
		
		for(m=0;m<(dwBoneNum/dwMaxMatrixNum+1);m++)
		{
			for(i=0; i<dwFaceNum; i++)
			{
				//����û�д棬����
				if(!pFaceSign[i] && pFaceID[i]==n)
				{	
					memset(pFaceVertexSign, 0, 3);
					//���ж����������Ƿ��ܷ�Χ�ڵľ���Ӱ�죬���ж϶���
					for(j=0,x=0; j<3; j++,x++)
					{
						//�ȵõ���������ֵ
						if(Desc.Format == D3DFMT_INDEX16)
							VertexIndics = *((unsigned short int *)pHeadSource+i*3+j);
						else if(Desc.Format == D3DFMT_INDEX32)
							VertexIndics = *((DWORD *)pHeadSource+i*3+j);

						//��������ֵȥ���������в��ң����ö����ÿ�������Ƿ���Ŀǰ�Ĺ�����Χ��
						for(k=0; k<dwBoneNum; k++)
							for(l=0; l<CurrentVertexNum[pBoneOptimizedMapping[k]]; l++)
								// �ҵ�
								if(*(*(pBoneVertics+pBoneOptimizedMapping[k])+l) == VertexIndics)
								{
									//�ж��Ƿ���Ŀǰ�Ĺ�����Χ�ڣ�����ö�����һ���������ڣ��Ͳ���Ҫ
									if(!(k<dwBoneRangeMax && k>=dwBoneRangeMin))
									{
										goto ttt;
									}
								}
						// ע�����ﲻ���жϣ��Ƿ�����ж��dwMaxMatrixNum��������Ӱ��һ�����㣬��Ϊ���ǲ����ܷ�����^_^
						// ѭ����ϣ��ܲ�ȥttt���������˵�����й������ڷ�Χ�ڣ�Ȼ����ݸ����ҵ�������VB�е���Ų������
						pFaceVertexSign[j] = 1;
								
ttt:
;
					}//end for ���������������Χ


					// �ɹ��ж�һ���棬���Լ���MESH
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
					}//end if�ж�һ����ɹ���

				}// end if����δ��
			}//end for eachface

				//ɨ�������������д��һ��������SPLITMESH������m��n֮�⣬���е�ѭ�����Ʊ�������������
					//����ͳ�ƶ����������Ա���䶥�㻺��������ͬʱ����SPLITMESH��MESH�Ķ���ӳ���ϵ
				for(y=0,temp=0; y<dwVertexNum; y++)
					if(pVertexSign[y])
					{
						pVertexMapping[temp] = y;
						temp++;
					}
					//��ͳ�����������Ա��������������
				for(y=0,temp1=0; y<dwFaceNum; y++)
					if(pEachFaceSign[y])
						temp1++;

					//������û�з��䵽һ�����㣬����������
				if(!temp1 || !temp)
					continue;
					//д����ֵ����
				pSplitMesh->dwVertexNum = temp;
				pSplitMesh->dwFaceNum = temp1;
				pSplitMesh->Type = 1;
				pSplitMesh->dwSubset = n;
				pSplitMesh->dwMatrixNum = dwBoneRangeMax - dwBoneRangeMin;
				pSplitMesh->dwDataVertexSize = dwMaxMatrixModNum * 4 * 2;

					//�����������壨��ΪSPLITMESH�Ƚ�С�����Ծ���16BIT����һ������̬��ģ�;ͱ���ɣ�XIXI��
				if(temp1 > 65535)
				{
					mymessage("ģ��̫�󣬷ָ�ģ�Ͷ���������INDEX_16����ʱ��֧�֣�");
					OutputDebugString("ģ��̫�󣬷ָ�ģ�Ͷ���������INDEX_16����ʱ��֧�֣�");
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
								//���ȷֱ�õ����������������ԭMESH�е�������Ϣ
								VertexIndics = *((unsigned short int *)pHeadSource+y*3+z);
								//�ٲ��ҵ�SPLITMESH�ж�Ӧ���������ֵ��д��
								*((unsigned short int *)p) = GetVertexMapping(pVertexMapping, temp, VertexIndics);
							}
							else if(Desc.Format == D3DFMT_INDEX32)
							{
								//���ȷֱ�õ����������������ԭMESH�е�������Ϣ
								VertexIndics = *((DWORD *)pHeadSource+y*3+z);
								//�ٲ��ҵ�SPLITMESH�ж�Ӧ���������ֵ��д��
								*((unsigned short int *)p) = GetVertexMapping(pVertexMapping, temp, VertexIndics);
							}
							p+=2;
						}
				pSplitMesh->pIB->Unlock();

					//�������㻺��
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

					//�������������壬ע��ÿ������Ĵ�С���ھ�����*4*2��������Ȩ�ض���float������Ҳ������float�����û��ɣ�a0.x֧�ֵģ���������ΪVS���޷�ѭ������ת�����밴��ֵ����
				d3ddevice->CreateVertexBuffer(temp*dwMaxMatrixModNum*4*2, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pSplitMesh->pDB, NULL);
					//FVF: D3DFVF_TEX8|D3DFVF_TEXCOORDSIZE4(0)|D3DFVF_TEXCOORDSIZE4(1)|D3DFVF_TEXCOORDSIZE4(2)|D3DFVF_TEXCOORDSIZE4(3)|D3DFVF_TEXCOORDSIZE4(4)|D3DFVF_TEXCOORDSIZE4(5)|D3DFVF_TEXCOORDSIZE4(6)|D3DFVF_TEXCOORDSIZE4(7)
				pSplitMesh->pDB->Lock(0, temp*dwMaxMatrixModNum*4*2, (void **)&p, 0);
				p1 = p + dwMaxMatrixModNum * 4;   //p1��дȨ���õģ������16�����󻰣������������ǲ�4���Ĵ�����λ�ã�16��float��

						//1.�õ�SPLITMESHÿ��������ԭMESH�е�����ֵ
				for(y=0; y<temp; y++)
				{
					VertexIndics = pVertexMapping[y];
						//2.ͨ����������Ŀǰ�����ķ�Χ��pBoneVertics��pWeight�в��Ҷ�Ӧֵ���϶������ҵ��ģ���д��
					for(k=0,z=0; k<dwBoneNum; k++)
						for(l=0; l<CurrentVertexNum[pBoneOptimizedMapping[k]]; l++)
							// �ҵ�
							if(*(*(pBoneVertics+pBoneOptimizedMapping[k])+l) == VertexIndics)
							{
								//д���������ݺ�Ȩ�أ�ע�������Ǳ��鷶Χ���������������Ҫ��ȥBoneRange��Сֵ
								*((float *)p) = (float)(k - dwBoneRangeMin);    //ע�������Ϊ����ľ���洢Ҳ�ǰ�����������ɵģ����Բ���ӳ�䵽ԭ����˳�򣬻��о���Ϊ�β��ж��Ƿ��ڷ�Χ�ڣ���Ϊ�ܸ���SPLITMESH����ӳ�䵽�������ģ��Ѿ��ض����ڷ�Χ����
								p+=4;
								*((float *)p1) = *(*(pWeights+pBoneOptimizedMapping[k])+l);
								p1+=4;
								z++;  //z�������Ǽ�¼�ö���д�����Ŀ���Ա����油��
							}
						//3.����д�����Ŀ��������Ӱ��ÿ������ľ��������ǲ�ͬ�ģ�������Ҫ��¼������������Ŀ��dwMaxMatrixModNum�м������Ȩ�ض�Ϊ0��������㣬����Ȩ��Ϊ0��ô�˶���Ч�ģ�
					for(k=0; k<(dwMaxMatrixModNum-z); k++)
					{
						*((float *)p) = 0;
						p+=4;
						*((float *)p1) = 0;
						p1+=4;
					}
					//����һ�Σ��������������д���Ͱ�Ȩ�ظղ�д�ĸ����ˣ�������ݻ���:-(
					p+=dwMaxMatrixModNum*4;
					p1+=dwMaxMatrixModNum*4;
				}						
				pSplitMesh->pDB->Unlock();

				//��������ӳ����Ϣ��ע���Ǿ����Ż��Ķ���ԭ���ģ������Ǵ�RangeMin��RangeMax��
				//ע��������ݲ�������д�룬��Ϊ����Ԥ������������Ҫ��ÿ��Draw��ʱ��UpdateHierarchy����д��
				pSplitMesh->pMatrixMapping = new DWORD[dwBoneRangeMax-dwBoneRangeMin];
				for(y=0; y<dwBoneRangeMax-dwBoneRangeMin; y++)
				{
					pSplitMesh->pMatrixMapping[y] = pBoneOptimizedMapping[y+dwBoneRangeMin];
				}
				
				
				//����д��ȫ����������ʼ��һЩ���ݣ�׼��������һ��SPLITMESH��ѭ��
				memset(pVertexSign, 0, dwVertexNum);
				memset(pEachFaceSign, 0, dwFaceNum);
				memset(pVertexMapping, 0, dwVertexNum*sizeof(DWORD));

					//����NEXT SPLITMESH
				pSplitMesh->pNextMesh = new SplitMesh;
				pSplitMesh = pSplitMesh->pNextMesh;

			dwBoneRangeMin += dwMaxMatrixNum;
			dwBoneRangeMax += dwMaxMatrixNum;
			dwBoneRangeMin = dwBoneRangeMin>dwBoneNum ? dwBoneNum : dwBoneRangeMin;
			dwBoneRangeMax = dwBoneRangeMax>dwBoneNum ? dwBoneNum : dwBoneRangeMax;

		}//end for bone range
	}//end for attribute table


	//�ڶ��ַָ�
	memset(pEachFaceSign, 0, dwFaceNum);
	memset(pVertexSign, 0, dwVertexNum);
	memset(pVertexMapping, 0, dwVertexNum*sizeof(DWORD));

	DWORD dwMatrixNum = 0;   // ��ǰ�ָ��MESH���еľ��󣨹���������
	char *pFaceMatrixSign = new char[dwBoneNum]; // ���ĳ����ŵľ����Ѵ棬���Ӧ��ŵĸ�����Ԫ�ؼ�Ϊ1����������ͬһ���������������ظ�ʹ�õľ���
	memset(pFaceMatrixSign, 0, dwBoneNum);
	char *pMatrixSign = new char[dwBoneNum]; // ���ĳ����ŵľ����Ѵ棬���Ӧ��ŵĸ�����Ԫ�ؼ�Ϊ1����������ÿ��SPLITMESH���յľ�������ӳ��
	memset(pMatrixSign, 0, dwBoneNum);

	// ��һ�����͵ķָ���󣬽���ڶ������ͣ���ʣ�±ȽϷ�ɢ�������ºϲ�Ϊһ��SPLITMESH����ν��ɢ����˼����˵Ӱ����������ľ�����ܲ���������������úܿ�
	// �������͵�ÿ��SPLITMESH�е��棬����С�ڵ�������������
	// ֱ�����е��涼���ˣ��ͽ�����ÿ����һ��whileѭ�����൱���Ѿ��洢��һ��SPLITMESH
	while(!CheckAllFacesHaveBeenSaved(pFaceSign, dwFaceNum))
	{
		//ѭ��ɨ�����е�Subset��ɨ��һ�ξ͵��л�һ��SPLITMESH
		//ע������Ϊ��Ҫ��dwAttributeTableNum��ԭ����MaterialNum�����м�������ĳЩMATERAIL����ûһ��������õ��������Ա����Ŀǡǡ���ڶ������õ���SUBSET����Ŀ���������MATERIAL�޹�
		for(n=0; n<dwAttributeTableNum; n++)
		{
			dwMatrixNum = 0;
			for(i=0,dwFaceMatrixNum=0; i<dwFaceNum; i++,dwFaceMatrixNum=0)
				//����û�д棬����
				if(!pFaceSign[i] && pFaceID[i]==n)
				{	
					memset(pFaceMatrixSign, 0, dwBoneNum);
					//�ȼ������������������֮�ͣ����ڸ������������ʱ������
					for(j=0; j<3; j++)
					{
						//�ȵõ���������ֵ
						if(Desc.Format == D3DFMT_INDEX16)
							VertexIndics = *((unsigned short int *)pHeadSource+i*3+j);
						else if(Desc.Format == D3DFMT_INDEX32)
							VertexIndics = *((DWORD *)pHeadSource+i*3+j);

						//��������ֵȥ���������в��ң����ж��ٸ���ͬ�Ĺ�����Ӱ��ö���
						for(k=0; k<dwBoneNum; k++)
							for(l=0; l<CurrentVertexNum[k]; l++)
								//�ҵ�һ���������Ҫ���Ƿ����µĹ���
								if(*(*(pBoneVertics+k)+l) == VertexIndics)
								{
									//�¹�������������1��������Ӧ�ı��
									if(!pMatrixSign[k])
									{
										dwFaceMatrixNum++;
										pFaceMatrixSign[k] = 1;
										pMatrixSign[k] = 1;  //��Ȼ��������1,���������治�ܴ棬��Ҫ��ԭ
									}
								}

					}//end for ���������������֮��

					//������ľ��󣨹�ͷ�������ͳ�����ָ���������������޷�����ֻ������ͬʱ��������������������°�VS��
					if(dwFaceMatrixNum > dwMaxMatrixNum)
						return D3DERR_OUTOFVIDEOMEMORY;
			
					//����������Ѵ�ľ������������������������л��½������SPLITMesh
					if((dwFaceMatrixNum+dwMatrixNum) > dwMaxMatrixNum)
					{
					
						//�ָ�MatrixSign
						for(y=0; y<dwBoneNum; y++)
							if(pFaceMatrixSign[y])
								pMatrixSign[y] = 0;

						//��ʼ�洢����
							//1. �õ��������Ͷ��������Լ�����ӳ��
						for(y=0,temp=0; y<dwVertexNum; y++)
							if(pVertexSign[y])
							{
								pVertexMapping[temp] = y;
								temp++;
							}
						for(y=0,temp1=0; y<dwFaceNum; y++)
							if(pEachFaceSign[y])
								temp1++;

							//������û�з��䵽һ�����㣬����������
						if(!temp1 || !temp)
						{
							dwMatrixNum = 0;
							i--;
							continue;
						}
							//2. д����ֵ����
						pSplitMesh->dwVertexNum = temp;
						pSplitMesh->dwFaceNum = temp1;
						pSplitMesh->Type = 2;
						pSplitMesh->dwSubset = n;
						pSplitMesh->dwMatrixNum = dwMatrixNum;
						pSplitMesh->dwDataVertexSize = dwMaxMatrixModNum * 4 * 2;


							//3. �����������壨��ΪSPLITMESH�Ƚ�С�����Ծ���16BIT����һ������̬��ģ�;ͱ���ɣ�XIXI��
						if(temp1 > 65535)
						{
							mymessage("ģ��̫�󣬷ָ�ģ�Ͷ���������INDEX_16����ʱ��֧�֣�");
								OutputDebugString("ģ��̫�󣬷ָ�ģ�Ͷ���������INDEX_16����ʱ��֧�֣�");
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
										//���ȷֱ�õ����������������ԭMESH�е�������Ϣ
										VertexIndics = *((unsigned short int *)pHeadSource+y*3+z);
										//�ٲ��ҵ�SPLITMESH�ж�Ӧ���������ֵ��д��
										*((unsigned short int *)p) = GetVertexMapping(pVertexMapping, temp, VertexIndics);
									}
									else if(Desc.Format == D3DFMT_INDEX32)
									{
										//���ȷֱ�õ����������������ԭMESH�е�������Ϣ
										VertexIndics = *((DWORD *)pHeadSource+y*3+z);
										//�ٲ��ҵ�SPLITMESH�ж�Ӧ���������ֵ��д��
										*((unsigned short int *)p) = GetVertexMapping(pVertexMapping, temp, VertexIndics);
									}
									p+=2;
								}
						pSplitMesh->pIB->Unlock();

							//4. �������㻺��
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

							//5. �������������壬ע��ÿ������Ĵ�С���ھ�����*4*2��������Ȩ�ض���float������Ҳ������float�����û��ɣ�a0.x֧�ֵģ���������ΪVS���޷�ѭ������ת�����밴��ֵ����
						d3ddevice->CreateVertexBuffer(temp*dwMaxMatrixModNum*4*2, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pSplitMesh->pDB, NULL);
						pSplitMesh->pDB->Lock(0, temp*dwMaxMatrixModNum*4*2, (void **)&p, 0);
						p1 = p + dwMaxMatrixModNum * 4;   //p1��дȨ���õģ������16�����󻰣������������ǲ�4���Ĵ�����λ�ã�16��float��

								//1.�õ�SPLITMESHÿ��������ԭMESH�е�����ֵ
						for(y=0; y<temp; y++)
						{
							VertexIndics = pVertexMapping[y];
								//2.ͨ����������Ŀǰ�����ļ�����pBoneVertics��pWeight�в��Ҷ�Ӧֵ���϶������ҵ��ģ���д��
							for(k=0,z=0,x=0; k<dwBoneNum; k++)
								if(pMatrixSign[k])
								{
									for(l=0; l<CurrentVertexNum[k]; l++)
										// �ҵ�
										if(*(*(pBoneVertics+k)+l) == VertexIndics)
										{
											//д���������ݺ�Ȩ�أ�ע�������Ǳ��鷶Χ���������������Ҫ��ȥBoneRange��Сֵ
											*((float *)p) = (float)x;    //ע�������Ϊ����ľ���洢Ҳ�ǰ�����������ɵģ����Բ���ӳ�䵽ԭ����˳�򣬻��о���Ϊ�β��ж��Ƿ��ڷ�Χ�ڣ���Ϊ�ܸ���SPLITMESH����ӳ�䵽�������ģ��Ѿ��ض����ڷ�Χ����
											p+=4;
											*((float *)p1) = *(*(pWeights+k)+l);
											p1+=4;
											z++;  //z�������Ǽ�¼�ö���д�����Ŀ���Ա����油��
										}
									x++;  //x�������Ǽ�¼��ǰ�ľ���������
								}
								//3.����д�����Ŀ��������Ӱ��ÿ������ľ��������ǲ�ͬ�ģ�������Ҫ��¼������������Ŀ��dwMaxMatrixModNum�м������Ȩ�ض�Ϊ0��������㣬����Ȩ��Ϊ0��ô�˶���Ч�ģ�
							for(k=0; k<(dwMaxMatrixModNum-z); k++)
							{
								*((float *)p) = 0;
								p+=4;
								*((float *)p1) = 0;
								p1+=4;
							}
							//����һ�Σ��������������д���Ͱ�Ȩ�ظղ�д�ĸ����ˣ�������ݻ���:-(
							p+=dwMaxMatrixModNum*4;
							p1+=dwMaxMatrixModNum*4;
						}						
						pSplitMesh->pDB->Unlock();

						//��������ӳ����Ϣ��ע���Ǿ����Ż��Ķ���ԭ���ģ������Ǵ�RangeMin��RangeMax��
						//ע��������ݲ�������д�룬��Ϊ����Ԥ������������Ҫ��ÿ��Draw��ʱ��UpdateHierarchy����д��
						pSplitMesh->pMatrixMapping = new DWORD[dwMatrixNum];

						for(y=0,x=0; y<dwBoneNum; y++)
							if(pMatrixSign[y])
							{
								pSplitMesh->pMatrixMapping[x] = y;
								x++;
							}

						//����д��ȫ����������ʼ��һЩ���ݣ�׼��������һ��SPLITMESH��ѭ��
							//����NEXT SPLITMESH
						pSplitMesh->pNextMesh = new SplitMesh;
						pSplitMesh = pSplitMesh->pNextMesh;

						dwMatrixNum = 0;

						memset(pVertexMapping, 0, dwVertexNum*sizeof(DWORD));
						memset(pVertexSign, 0, dwVertexNum);
						memset(pEachFaceSign, 0, dwFaceNum);
						memset(pFaceMatrixSign, 0, dwBoneNum);
						memset(pMatrixSign, 0, dwBoneNum);

						//�ظ����棬���i��--��������ȥ�ˣ�Ҫ����
						i--;
						continue;
					}//end save and toggle next splitmesh





					//��������Ҫ�󣬽�������Ϊ1����ʾ�Ѵ�
					else
					{
						//���������ݴ���SplitMesh�������ݣ�
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
				}// end for each face and if����δ��




			//����һ��SUBSET�����������������ɨβ�������ǰSPLITMESH���ж�������˵����������һ���棬��Ҫ�������һ��SPLITMESH
							//1. �õ��������Ͷ��������Լ�����ӳ��
						for(y=0,temp=0; y<dwVertexNum; y++)
							if(pVertexSign[y])
							{
								pVertexMapping[temp] = y;
								temp++;
							}
						for(y=0,temp1=0; y<dwFaceNum; y++)
							if(pEachFaceSign[y])
								temp1++;
						
							//������û�з��䵽һ�����㣬˵��SPLITMESHû�ж�������������
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
							//2. д����ֵ����
						pSplitMesh->dwVertexNum = temp;
						pSplitMesh->dwFaceNum = temp1;
						pSplitMesh->Type = 2;
						pSplitMesh->dwSubset = n;
						pSplitMesh->dwMatrixNum = dwMatrixNum;
						pSplitMesh->dwDataVertexSize = dwMaxMatrixModNum * 4 * 2;


							//3. �����������壨��ΪSPLITMESH�Ƚ�С�����Ծ���16BIT����һ������̬��ģ�;ͱ���ɣ�XIXI��
						if(temp1 > 65535)
						{
							mymessage("ģ��̫�󣬷ָ�ģ�Ͷ���������INDEX_16����ʱ��֧�֣�");
								OutputDebugString("ģ��̫�󣬷ָ�ģ�Ͷ���������INDEX_16����ʱ��֧�֣�");
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
										//���ȷֱ�õ����������������ԭMESH�е�������Ϣ
										VertexIndics = *((unsigned short int *)pHeadSource+y*3+z);
										//�ٲ��ҵ�SPLITMESH�ж�Ӧ���������ֵ��д��
										*((unsigned short int *)p) = GetVertexMapping(pVertexMapping, temp, VertexIndics);
									}
									else if(Desc.Format == D3DFMT_INDEX32)
									{
										//���ȷֱ�õ����������������ԭMESH�е�������Ϣ
										VertexIndics = *((DWORD *)pHeadSource+y*3+z);
										//�ٲ��ҵ�SPLITMESH�ж�Ӧ���������ֵ��д��
										*((unsigned short int *)p) = GetVertexMapping(pVertexMapping, temp, VertexIndics);
									}
									p+=2;
								}
						pSplitMesh->pIB->Unlock();

							//4. �������㻺��
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

							//5. �������������壬ע��ÿ������Ĵ�С���ھ�����*4*2��������Ȩ�ض���float������Ҳ������float�����û��ɣ�a0.x֧�ֵģ���������ΪVS���޷�ѭ������ת�����밴��ֵ����
						d3ddevice->CreateVertexBuffer(temp*dwMaxMatrixModNum*4*2, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pSplitMesh->pDB, NULL);
						pSplitMesh->pDB->Lock(0, temp*dwMaxMatrixModNum*4*2, (void **)&p, 0);
						p1 = p + dwMaxMatrixModNum * 4;   //p1��дȨ���õģ������16�����󻰣������������ǲ�4���Ĵ�����λ�ã�16��float��

								//1.�õ�SPLITMESHÿ��������ԭMESH�е�����ֵ
						for(y=0; y<temp; y++)
						{
							VertexIndics = pVertexMapping[y];
								//2.ͨ����������Ŀǰ�����ļ�����pBoneVertics��pWeight�в��Ҷ�Ӧֵ���϶������ҵ��ģ���д��
							for(k=0,z=0,x=0; k<dwBoneNum; k++)
								if(pMatrixSign[k])
								{
									for(l=0; l<CurrentVertexNum[k]; l++)
										// �ҵ�
										if(*(*(pBoneVertics+k)+l) == VertexIndics)
										{
											//д���������ݺ�Ȩ�أ�ע�������Ǳ��鷶Χ���������������Ҫ��ȥBoneRange��Сֵ
											*((float *)p) = (float)x;    //ע�������Ϊ����ľ���洢Ҳ�ǰ�����������ɵģ����Բ���ӳ�䵽ԭ����˳�򣬻��о���Ϊ�β��ж��Ƿ��ڷ�Χ�ڣ���Ϊ�ܸ���SPLITMESH����ӳ�䵽�������ģ��Ѿ��ض����ڷ�Χ����
											p+=4;
											*((float *)p1) = *(*(pWeights+k)+l);
											p1+=4;
											z++;  //z�������Ǽ�¼�ö���д�����Ŀ���Ա����油��
										}
									x++;  //x�������Ǽ�¼��ǰ�ľ���������
								}
								//3.����д�����Ŀ��������Ӱ��ÿ������ľ��������ǲ�ͬ�ģ�������Ҫ��¼������������Ŀ��dwMaxMatrixModNum�м������Ȩ�ض�Ϊ0��������㣬����Ȩ��Ϊ0��ô�˶���Ч�ģ�
							for(k=0; k<(dwMaxMatrixModNum-z); k++)
							{
								*((float *)p) = 0;
								p+=4;
								*((float *)p1) = 0;
								p1+=4;
							}
							//����һ�Σ��������������д���Ͱ�Ȩ�ظղ�д�ĸ����ˣ�������ݻ���:-(
							p+=dwMaxMatrixModNum*4;
							p1+=dwMaxMatrixModNum*4;
						}						
						pSplitMesh->pDB->Unlock();

						//��������ӳ����Ϣ��ע���Ǿ����Ż��Ķ���ԭ���ģ������Ǵ�RangeMin��RangeMax��
						//ע��������ݲ�������д�룬��Ϊ����Ԥ������������Ҫ��ÿ��Draw��ʱ��UpdateHierarchy����д��
						pSplitMesh->pMatrixMapping = new DWORD[dwMatrixNum];

						for(y=0,x=0; y<dwBoneNum; y++)
							if(pMatrixSign[y])
							{
								pSplitMesh->pMatrixMapping[x] = y;
								x++;
							}				
				
						//����д��ȫ����������ʼ��һЩ���ݣ�׼��������һ��SPLITMESH��ѭ��

			//ͳͳ�»��½���MESH��������һ��SUBSET��ɨ��
			pSplitMesh->pNextMesh = new SplitMesh;
			pSplitMesh = pSplitMesh->pNextMesh;

			dwMatrixNum = 0;
			memset(pVertexMapping, 0, dwVertexNum*sizeof(DWORD));
			memset(pVertexSign, 0, dwVertexNum);
			memset(pEachFaceSign, 0, dwFaceNum);
			memset(pFaceMatrixSign, 0, dwBoneNum);
			memset(pMatrixSign, 0, dwBoneNum);
		}//end for����Subset

	}//end while
	
	pIB->Unlock();
	pIB->Release();

	//ÿ������½�SplitMesh�ķ������ᵼ�������һ�����õ�SPLITMESH��ҪDELETE������˳·���SplitMesh������Ŀ
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
	
	//���������ʱ����Ķ���
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














// ����SplitMesh���еľ������ݣ�ʹ��ǰȷ�������Ĵ�������ʼ������û�д���ģ���
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
			// �������еľ��󣬲��ò��㣬�������Ȩֵ��Ϊ0�Ϳ����ˣ���SplitMesh�������Ѿ����ˣ�
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
		OutputDebugString("���ȶ���һ��SkinMesh!!");
		return E_FAIL;
	}
	if(m_pMesh->pSplitMesh || m_pMesh->dwSplitMeshNum)
	{
		OutputDebugString("�Ѿ��ָ����SkinMesh�������ٶ���������!!");
		return S_OK;
	}

	FILE *fp = fopen(szFileName, "rb");
	if(!fp)
	{
		OutputDebugString("��ָ��һ����Ч��·�����ļ�����");
		return D3DERR_INVALIDCALL;
	}

	// ��ȡ�ļ�ͷ��Ϣ
	char szVerifyName[20] = "SPLITMESHVS";
	fread(szVerifyName, 1, sizeof("SPLITMESHVS"), fp);
	if(strcmp(szVerifyName, "SPLITMESHVS"))
	{
		OutputDebugString("�ļ�ͷУ��ʧ�ܣ���ȷ���ļ�����Ч��SplitMesh�����ļ�����");
		return E_FAIL;
	}

	DWORD iData = 0;
	fread(&iData, 1, sizeof(DWORD), fp);
	if(!iData)
	{
		OutputDebugString("�ļ�ͷ����У��ʧ�ܣ���ȷ���ļ�����Ч��SplitMesh�����ļ�����");
		return E_FAIL;
	}
	m_pMesh->dwSplitMeshNum = iData;

	fread(&iData, 1, sizeof(DWORD), fp);
	if(!iData)
	{
		OutputDebugString("�ļ�ͷ����У��ʧ�ܣ���ȷ���ļ�����Ч��SplitMesh�����ļ�����");
		return E_FAIL;
	}
	m_pMesh->dwMaxMatrixNum = iData;




	// ��ȡÿ���ֿ����Ϣ�������µ�SplitMesh
	if(m_pMesh->pSplitMesh)
	{
		OutputDebugString("�Ѿ��ָ����SkinMesh�������ٶ���������!!");
		goto FAILED;
	}


	SplitMesh *pSplitMesh = new SplitMesh;
	m_pMesh->pSplitMesh = pSplitMesh;

	for(DWORD i = 0; i < m_pMesh->dwSplitMeshNum; i++)
	{
		// ��ȡ�ַ�����ʶ�����
		fread(szVerifyName, 1, sizeof("SPLITMESH"), fp);
		fread(&iData, 1, sizeof(DWORD), fp);
		if(strcmp(szVerifyName, "SPLITMESH") || iData != i)
		{
			OutputDebugString("�ļ���ͷУ��ʧ�ܣ���ȷ���ļ�����Ч��SplitMesh�����ļ�����");
			return E_FAIL;
		}


		// ��ȡÿ��SplitMesh�еĶ�������
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
			OutputDebugString("�ļ�������У��ʧ�ܣ���ȷ���ļ�����Ч��SplitMesh�����ļ�����");
			goto FAILED;
		}

		// ��ȡ�Ƕ������ݣ����Ǿ���ӳ����������
		pSplitMesh->pMatrixMapping = new DWORD[pSplitMesh->dwMatrixNum];
		fread(pSplitMesh->pMatrixMapping, pSplitMesh->dwMatrixNum, sizeof(DWORD), fp);


		// ����VB IB DB
		if(pSplitMesh->pVB)
		{
			OutputDebugString("���ж��㻺�����ݣ�");
			goto FAILED;
		}
		if(pSplitMesh->pIB)
		{
			OutputDebugString("���������������ݣ�");
			goto FAILED;
		}
		if(pSplitMesh->pDB)
		{
			OutputDebugString("�������ݻ������ݣ�");
			goto FAILED;
		}

		// �õ���С���������㻺��
		DWORD dwVBSize = 0, dwIBSize = 0, dwDBSize = 0;
		char *pVBData = NULL, *pIBData = NULL, *pDBData = NULL;

		// VB
		fread(&dwVBSize, 1, sizeof(DWORD), fp);
		if(!dwVBSize)
		{
			OutputDebugString("�ļ�������У��ʧ�ܣ���ȷ���ļ�����Ч��SplitMesh�����ļ�����");
			goto FAILED;
		}

		// ����������������ʽ�ģ����Բ���Ҫ��ȡDesc����
		HRESULT hr = d3ddevice->CreateVertexBuffer(dwVBSize, D3DUSAGE_WRITEONLY, m_pMesh->pSkinMesh->GetFVF(), D3DPOOL_DEFAULT, &pSplitMesh->pVB, NULL);
		if(FAILED(hr))
		{
			OutputDebugString("�������㻺��ʧ�ܣ�������Ӳ�����������������Դ治�㣡");
			goto FAILED;
		}
		// Lock���������
		hr = pSplitMesh->pVB->Lock(0, dwVBSize, (void **)&pVBData, 0);
		if(FAILED(hr))
		{
			OutputDebugString("Lock���㻺��ʧ�ܣ���");
			goto FAILED;
		}

		fread(pVBData, 1, dwVBSize, fp);



		// IB
		fread(&dwIBSize, 1, sizeof(DWORD), fp);
		if(!dwIBSize)
		{
			OutputDebugString("�ļ�������У��ʧ�ܣ���ȷ���ļ�����Ч��SplitMesh�����ļ�����");
			goto FAILED;
		}

		hr = d3ddevice->CreateIndexBuffer(dwIBSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pSplitMesh->pIB, NULL);
		if(FAILED(hr))
		{
			OutputDebugString("������������ʧ�ܣ�������Ӳ�����������������Դ治�㣡");
			goto FAILED;
		}

		hr = pSplitMesh->pIB->Lock(0, dwIBSize, (void **)&pIBData, 0);
		if(FAILED(hr))
		{
			OutputDebugString("Lock��������ʧ�ܣ���");
			goto FAILED;
		}

		fread(pIBData, 1, dwIBSize, fp);




		// DB
		fread(&dwDBSize, 1, sizeof(DWORD), fp);
		if(!dwDBSize)
		{
			OutputDebugString("�ļ�������У��ʧ�ܣ���ȷ���ļ�����Ч��SplitMesh�����ļ�����");
			goto FAILED;
		}

		hr = d3ddevice->CreateVertexBuffer(dwDBSize, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pSplitMesh->pDB, NULL);
		if(FAILED(hr))
		{
			OutputDebugString("�������ݻ���ʧ�ܣ�������Ӳ�����������������Դ治�㣡");
			goto FAILED;
		}

		hr = pSplitMesh->pDB->Lock(0, dwDBSize, (void **)&pDBData, 0);
		if(FAILED(hr))
		{
			OutputDebugString("Lock���ݻ���ʧ�ܣ���");
			goto FAILED;
		}

		fread(pDBData, 1, dwDBSize, fp);


		// ������
		pSplitMesh->pVB->Unlock();
		pSplitMesh->pIB->Unlock();
		pSplitMesh->pDB->Unlock();


		// ��ȡ��ϣ������µ�SplitMesh
		pSplitMesh->pNextMesh = new SplitMesh;
		pSplitMesh = pSplitMesh->pNextMesh;
	}


	//ÿ������½�SplitMesh�ķ������ᵼ�������һ�����õ�SPLITMESH��ҪDELETE������˳·���SplitMesh������Ŀ
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
		OutputDebugString("�ָ�Mesh����У��ʧ�ܣ���ȷ���ļ�����Ч��SplitMesh�����ļ�����");
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
		OutputDebugString("���ȶ���һ��SkinMesh!!");
		return E_FAIL;
	}
	if(!m_pMesh->pSplitMesh || !m_pMesh->dwSplitMeshNum)
	{
		OutputDebugString("���Ƚ�SkinMesh�ָ��!!");
		return E_FAIL;
	}

	FILE *fp = fopen(szFileName, "wb");
	if(!fp)
	{
		OutputDebugString("��ָ��һ����Ч��·�����ļ�������֤���̲���ֻ���������㹻��ʣ��ռ䣡");
		return D3DERR_INVALIDCALL;
	}

	// д���ļ�ͷ��Ϣ
	char szFileHeaderName[] = "SPLITMESHVS";
	fwrite(szFileHeaderName, 1, sizeof("SPLITMESHVS"), fp);
	fwrite(&m_pMesh->dwSplitMeshNum, 1, sizeof(DWORD), fp);
	fwrite(&m_pMesh->dwMaxMatrixNum, 1, sizeof(DWORD), fp);

	// д��ÿ���ֿ����Ϣ
	char szFileSegmentName[] = "SPLITMESH";
	SplitMesh *pSplitMesh = m_pMesh->pSplitMesh;

	for(DWORD i = 0; i < m_pMesh->dwSplitMeshNum; i++)
	{
		if(!pSplitMesh)
			break;

		// д���ַ�����ʶ�����
		fwrite(szFileSegmentName, 1, sizeof("SPLITMESH"), fp);
		fwrite(&i, 1, sizeof(DWORD), fp);

		// д��ÿ��SplitMesh�еĶ�������
		if(!pSplitMesh->dwVertexNum || !pSplitMesh->dwFaceNum || !pSplitMesh->dwVertexSize || !pSplitMesh->dwDataVertexSize)
		{
			OutputDebugString("�ָ������SkinMesh���㡢�����������󣡣�");
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

		// д��Ƕ������ݣ����Ǿ���ӳ����������
		if(!pSplitMesh->pMatrixMapping)
		{
			OutputDebugString("�ָ������SkinMesh�������󣡣�û�о���ӳ�����ݣ�");
			goto FAILED;
		}
		fwrite(pSplitMesh->pMatrixMapping, pSplitMesh->dwMatrixNum, sizeof(DWORD), fp);


		// ����VB IB DB
		if(!pSplitMesh->pVB)
		{
			OutputDebugString("�ָ������SkinMesh�������󣡣�û�ж��㻺�����ݣ�");
			goto FAILED;
		}
		if(!pSplitMesh->pIB)
		{
			OutputDebugString("�ָ������SkinMesh�������󣡣�û�������������ݣ�");
			goto FAILED;
		}
		if(!pSplitMesh->pDB)
		{
			OutputDebugString("�ָ������SkinMesh�������󣡣�û�����ݻ������ݣ�");
			goto FAILED;
		}

		// �õ���С��Lock
		D3DVERTEXBUFFER_DESC VBDesc, DBDesc;
		D3DINDEXBUFFER_DESC IBDesc;
		pSplitMesh->pVB->GetDesc(&VBDesc);
		pSplitMesh->pIB->GetDesc(&IBDesc);
		pSplitMesh->pDB->GetDesc(&DBDesc);

		/*��Ϊ�Ƕ���ʽ�ģ���������Desc���ݲ��ñ���
		d3ddevice->CreateVertexBuffer(temp*z, D3DUSAGE_WRITEONLY, pMesh->pSkinMesh->GetFVF(), D3DPOOL_DEFAULT, &pSplitMesh->pVB, NULL);
		HRESULT hr = d3ddevice->CreateIndexBuffer(temp1*3*2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pSplitMesh->pIB, NULL);
		d3ddevice->CreateVertexBuffer(temp*dwMaxMatrixModNum*4*2, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pSplitMesh->pDB, NULL);
		*/

		char *pVBData = NULL, *pIBData = NULL, *pDBData = NULL;
		HRESULT hr = pSplitMesh->pVB->Lock(0, VBDesc.Size, (void **)&pVBData, D3DLOCK_READONLY);
		if(FAILED(hr))
		{
			OutputDebugString("Lock���㻺��ʧ�ܣ���");
			goto FAILED;
		}
		hr = pSplitMesh->pIB->Lock(0, IBDesc.Size, (void **)&pIBData, D3DLOCK_READONLY);
		if(FAILED(hr))
		{
			OutputDebugString("Lock��������ʧ�ܣ���");
			goto FAILED;
		}
		hr = pSplitMesh->pDB->Lock(0, DBDesc.Size, (void **)&pDBData, D3DLOCK_READONLY);
		if(FAILED(hr))
		{
			OutputDebugString("Lock���ݻ���ʧ�ܣ���");
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


		// �洢��ϣ�������һ��
		pSplitMesh = pSplitMesh->pNextMesh;
	}	


	fclose(fp);
	return S_OK;


FAILED:
	fclose(fp);
	return E_FAIL;

}

































// ����ÿ����ó��ı���Ϣ
struct MeshEdge
{
	BYTE uCount;   // �����߳��ֵ�����������������µ���1���������1����˵���������湲�øñߣ�Ϊ0�����Ա�ʾδ��ʼ����״̬
	DWORD dwVertexIndex1, dwVertexIndex2;  // �ñ��������������������е������ţ�Ҫ����Point Representive����������ظ��Ķ���
	bool bAdded;   // �Ƿ��Ѿ�����Degenerated Quad����ʼΪfalse��������count����1������²ſ���Ϊtrue
	D3DXVECTOR3 ShareFaceNormal[2];        // ����ñߵ��淨�ߣ���Ϊһ�������ֻ�ܱ������湲����ֻ������Ԫ�أ�����ֻ��һ�������������ã�ȡ����Count
	D3DXVECTOR3 posVertex[2];           // ���������������꣬0��1Ԫ�طֱ��ӦIndex1��Index2�������Ժ��ʹ��

	const static D3DVERTEXELEMENT9 dclr[6];   //����������

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
	{0,12+12,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},  // �����Ƿ�����չ�ı�־��ֻ��QUAD����������ñ�־��1,����ȫΪ0,��ʾ������չ
	{0,12+12+12,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 1}, // ������������������ķ���
	{0,12+12+12+12,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 2},
	D3DDECL_END()
};


// �ú�������������飬����ҵ�����ߣ�����룬���û�ҵ����������ӣ����ص��ǵ�ǰ���е��ܱ���
DWORD SetEdgeData(MeshEdge *p, DWORD Vertex1, DWORD Vertex2, D3DXVECTOR3 posVertex1, D3DXVECTOR3 posVertex2, D3DXVECTOR3 vecFaceNormal)
{
	UINT i = 0, FindSign = 0;
	while(p[i].uCount && !FindSign)
	{
		// �����е�ѡ�����ҵ��������ﲻ������Ϊ��ִ�е�������β�����Եõ����ֵ
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

	// ���û���ҵ����Ϳ�һ���µ�
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

	// ���㵱ǰ��Ч���ܱ���
	for(i=0; p[i].uCount; i++);
	return i;
}


//FILE *fp=fopen("c:\\log.txt", "w");
//#define LOG(p) fwrite(p, strlen(p), 1, fp);
// ����MESH����Ϊ��յ��壬����в���յĲ��ִ��ڣ��ò��ֺܿ��ܻ���ִ����Ч��
// ���о��������һ���ļ����ж��MESH���ڣ���D3DXLOADʱ���Զ�������MESH���ݶ���ͬһ��MESH�У���������MESH����ú���ʱ��GenerateAdjancyʱ�ͻ�Ƿ�����
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
	DWORD dwEdgeNum = 0;    // �����ظ��ıߵ�������
	DWORD dwVertex1=0, dwVertex2=0, dwVertex3=0;

	DWORD *pAdjancy = new DWORD[3*sizeof(DWORD)*dwFaceNum], *pVertexRep = new DWORD[dwVertexNum*sizeof(DWORD)];      // �����ظ�����Ļ�����

	if(!pAdjancy || !pVertexRep)
		return E_OUTOFMEMORY;
	
	pMesh->GenerateAdjacency(0.00001f, pAdjancy);
	pMesh->ConvertAdjacencyToPointReps(pAdjancy, pVertexRep);
	delete [] pAdjancy;
	
	// �õ�����BUFFER
	if(FAILED(pMesh->GetIndexBuffer(&pSourceIB)))
		return E_FAIL;
	if(FAILED(pMesh->GetVertexBuffer(&pSourceVB)))
		return E_FAIL;

	if(!pSourceIB || !pSourceVB || !dwFaceNum || !dwVertexNum)
		return E_FAIL;


	// �õ�IBָ��
	pSourceIB->GetDesc(&Desc);
	
	if(Desc.Format == D3DFMT_INDEX16)
		if(FAILED(pSourceIB->Lock(0, 2*dwFaceNum*3, (void **)&pHeadSourceIB, 0)))
			return E_FAIL;
	if(Desc.Format == D3DFMT_INDEX32)
		if(FAILED(pSourceIB->Lock(0, 4*dwFaceNum*3, (void **)&pHeadSourceIB, 0)))
			return E_FAIL;

	if(!pHeadSourceIB) return E_FAIL;

	// �õ�VBָ��
	if(FAILED(pSourceVB->Lock(0, dwVertexSize*dwVertexNum, (void **)&pHeadSourceVB, 0)))
		return E_FAIL;
	if(!pHeadSourceVB) return E_FAIL;


	// ����MeshEdge�����ֵ����Ϊ���ظ��ıߣ�����˵��Щ���ò����ģ�
	MeshEdge *pEdge = new MeshEdge[dwFaceNum*3];

	// �����������壬��MeshEdge��ֵ
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
		// �õ��������꣬���淨��
		D3DXVECTOR3 posVertex1 = *((D3DXVECTOR3 *)(pHeadSourceVB+dwVertex1*dwVertexSize));
		D3DXVECTOR3 posVertex2 = *((D3DXVECTOR3 *)(pHeadSourceVB+dwVertex2*dwVertexSize));
		D3DXVECTOR3 posVertex3 = *((D3DXVECTOR3 *)(pHeadSourceVB+dwVertex3*dwVertexSize));

		D3DXVECTOR3 vecFaceNormal;
		D3DXVec3Cross(&vecFaceNormal, &(posVertex1-posVertex3), &(posVertex2-posVertex3));
		D3DXVec3Normalize(&vecFaceNormal, &vecFaceNormal);

		SetEdgeData(pEdge, dwVertex1, dwVertex2, posVertex1, posVertex2, vecFaceNormal);
		SetEdgeData(pEdge, dwVertex1, dwVertex3, posVertex1, posVertex3, vecFaceNormal);
		dwEdgeNum = SetEdgeData(pEdge, dwVertex2, dwVertex3, posVertex2, posVertex3, vecFaceNormal);     // ���ǵõ����һ���������գ��ı�����
	}


	// �õ�����ߵ����������ڹ���Degenerate Quad�����dwQuadNumΪ0�������˹��ˣ��ѵ�ֻ��һ�������Σ���
	for(i=0,dwQuadNum=0; i<dwEdgeNum; i++)
		if(pEdge[i].uCount > 1)
			dwQuadNum++;

	if(!dwQuadNum)
		return E_FAIL;

	// �½����㻺����������岢�õ��û���
	dwVolumnFaceNum = dwFaceNum + dwQuadNum * 2;
	dwVolumnVertexNum = dwVertexNum + dwQuadNum * 4;

	D3DXCreateMesh(dwVolumnFaceNum, dwVolumnVertexNum, D3DXMESH_WRITEONLY | D3DXMESH_MANAGED, MeshEdge::dclr, d3ddevice, ppVolumn);
	dwVolumnVertexSize = D3DXGetFVFVertexSize((*ppVolumn)->GetFVF());
	
	
	if(!ppVolumn)
		return E_FAIL;

	(*ppVolumn)->GetVertexBuffer(&pDestVB);
	(*ppVolumn)->GetIndexBuffer(&pDestIB);

	// �õ�IBָ��
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
		// �ȸ���ԴMESH�Ĳ��֣��ⲿ�ֿ�Ҫ�ɲ�Ҫ��ֻ��Ϊ�����ڲ��ػ�ԭMESH������»���������ģ�ͣ���Ե
	if(VolumnDesc.Format == D3DFMT_INDEX16)
	{
		memcpy(pHeadDestIB, pHeadSourceIB, 2*dwFaceNum*3);
	}
	if(VolumnDesc.Format == D3DFMT_INDEX32)
	{
		memcpy(pHeadDestIB, pHeadSourceIB, 4*dwFaceNum*3);
	}
	
		// ����QUAD������
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


	// �õ�VBָ��
	if(FAILED(pDestVB->Lock(0, dwVolumnVertexSize*dwVolumnVertexNum, (void **)&pHeadDestVB, 0)))
		return E_FAIL;
	if(!pHeadDestVB) return E_FAIL;

		// ����ԭ����MESH��������
	for(i=0; i<dwVertexNum; i++)
	{
		*((D3DXVECTOR3 *)pHeadDestVB + i*5 + 0) = *((D3DXVECTOR3 *)(pHeadSourceVB + i*dwVertexSize));
		*((D3DXVECTOR3 *)pHeadDestVB + i*5 + 1) = D3DXVECTOR3(0, 0, 0);
		*((D3DXVECTOR3 *)pHeadDestVB + i*5 + 2) = D3DXVECTOR3(0, 0, 0);
		*((D3DXVECTOR3 *)pHeadDestVB + i*5 + 3) = D3DXVECTOR3(0, 0, 0);
		*((D3DXVECTOR3 *)pHeadDestVB + i*5 + 4) = D3DXVECTOR3(0, 0, 0);
	}


		// ����QUAD
	for(i=0,j=0; i<dwEdgeNum; i++)
		if(pEdge[i].uCount > 1)
		{
			// ����QUAD���ĸ��������ݣ�˳�򣺲���չ�Ķ���1,��չ�Ķ���1,����չ�Ķ���2,��չ�Ķ���2
				// ������չ�Ķ���1
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 0) = pEdge[i].posVertex[0];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 1) = *((D3DXVECTOR3 *)(pHeadSourceVB + pEdge[i].dwVertexIndex1*dwVertexSize + sizeof(D3DXVECTOR3)));
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 2) = D3DXVECTOR3(0, 0, 0);
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 3) = pEdge[i].ShareFaceNormal[1];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 4) = pEdge[i].ShareFaceNormal[0];
			

				// ������չ�Ķ���2
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 5) = pEdge[i].posVertex[0];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 6) = *((D3DXVECTOR3 *)(pHeadSourceVB + pEdge[i].dwVertexIndex1*dwVertexSize + sizeof(D3DXVECTOR3)));
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 7) = D3DXVECTOR3(1, 1, 1);
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 8) = pEdge[i].ShareFaceNormal[1];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 9) = pEdge[i].ShareFaceNormal[0];
			
			
				// ������չ�Ķ���3
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 10) = pEdge[i].posVertex[1];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 11) = *((D3DXVECTOR3 *)(pHeadSourceVB + pEdge[i].dwVertexIndex2*dwVertexSize + sizeof(D3DXVECTOR3)));
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 12) = D3DXVECTOR3(0, 0, 0);
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 13) = pEdge[i].ShareFaceNormal[1];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 14) = pEdge[i].ShareFaceNormal[0];
			
				// ������չ�Ķ���4
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 15) = pEdge[i].posVertex[1];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 16) = *((D3DXVECTOR3 *)(pHeadSourceVB + pEdge[i].dwVertexIndex2*dwVertexSize + sizeof(D3DXVECTOR3)));
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 17) = D3DXVECTOR3(1, 1, 1);
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 18) = pEdge[i].ShareFaceNormal[1];
			*((D3DXVECTOR3 *)pHeadDestVB + (j*4+dwVertexNum)*5 + 19) = pEdge[i].ShareFaceNormal[0];
			j++;
		}
dwVertexNum=z;
	
	
	// ȫ������
	pDestIB->Unlock();
	pDestVB->Unlock();
	pSourceIB->Unlock();
	pSourceVB->Unlock();
	


	// �õ�IBָ��
	pDestIB->GetDesc(&VolumnDesc);
	
	if(VolumnDesc.Format == D3DFMT_INDEX16)
		if(FAILED(pDestIB->Lock(0, 2*dwVolumnFaceNum*3, (void **)&pHeadDestIB, 0)))
			return E_FAIL;
	if(VolumnDesc.Format == D3DFMT_INDEX32)
		if(FAILED(pDestIB->Lock(0, 4*dwVolumnFaceNum*3, (void **)&pHeadDestIB, 0)))
			return E_FAIL;
			
	// �õ�VBָ��
	if(FAILED(pDestVB->Lock(0, dwVolumnVertexSize*dwVolumnVertexNum, (void **)&pHeadDestVB, 0)))
		return E_FAIL;
	if(!pHeadDestVB) return E_FAIL;


/*
	sprintf(a, "ԭMESH��%d�����㣬��%d�����ظ��ߣ���VOLUMN��%d�����㣬���㻺��������£�\n\n", dwVertexNum, dwQuadNum, dwVolumnVertexNum);
	LOG(a);
	for(i=0; i<dwVolumnVertexNum; i++)
	{
		sprintf(a, "��%d�����㣺\n", i);
		LOG(a);
		aa = *((D3DXVECTOR3 *)pHeadDestVB + i*4);
		sprintf(a, "��������Ϊ�� %f,%f,%f\n", aa.x, aa.y, aa.z);
		LOG(a);
		aa = *((D3DXVECTOR3 *)pHeadDestVB + i*4 + 1);
		sprintf(a, "���Ϊ�� %f,%f,%f\n", aa.x, aa.y, aa.z);
		LOG(a);
		aa = *((D3DXVECTOR3 *)pHeadDestVB + i*4 + 2);
		sprintf(a, "�淨��1Ϊ�� %f,%f,%f\n", aa.x, aa.y, aa.z);
		LOG(a);
		aa = *((D3DXVECTOR3 *)pHeadDestVB + i*4 + 3);
		sprintf(a, "�淨��2Ϊ�� %f,%f,%f\n", aa.x, aa.y, aa.z);
		LOG(a);
		LOG("\n");
	}
	
	sprintf(a, "ԭMESH��%d���棬��%d�����ظ��ߣ���VOLUMN��%d���棬��������������£�\n\n", dwFaceNum, dwQuadNum, dwVolumnFaceNum);
	LOG(a);
	unsigned short int aaa;
	for(i=0; i<dwVolumnFaceNum; i++)
	{
		sprintf(a, "��%d���棺\n", i);
		LOG(a);
		aaa = *((unsigned short int *)pHeadDestIB + i*3 + 0);
		sprintf(a, "����Ϊ�� %d, ", aaa);
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
/*	sprintf(a, "һ����%d�����㣬%d���棬%d�����ظ���\n\n", dwVertexNum, dwFaceNum, dwEdgeNum);
	LOG(a);
	for(i=0; i<dwEdgeNum; i++)
	{
		sprintf(a, "��%d���ߣ�����%d��\n", i, pEdge[i].uCount);
		LOG(a);
		sprintf(a, "����1Ϊ%d�� %f,%f,%f\n", pEdge[i].dwVertexIndex1, pEdge[i].posVertex[0].x, pEdge[i].posVertex[0].y, pEdge[i].posVertex[0].z);
		LOG(a);
		sprintf(a, "����2Ϊ%d�� %f,%f,%f\n", pEdge[i].dwVertexIndex2, pEdge[i].posVertex[1].x, pEdge[i].posVertex[1].y, pEdge[i].posVertex[1].z);
		LOG(a);
		sprintf(a, "�淨��1Ϊ�� %f,%f,%f\n", pEdge[i].ShareFaceNormal[0].x, pEdge[i].ShareFaceNormal[0].y, pEdge[i].ShareFaceNormal[0].z);
		LOG(a);
		sprintf(a, "�淨��2Ϊ�� %f,%f,%f\n", pEdge[i].ShareFaceNormal[1].x, pEdge[i].ShareFaceNormal[1].y, pEdge[i].ShareFaceNormal[1].z);
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



































// ����д��VB����������ֱ����Ϊ����ʹ��
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
    int m_anOldEdge[2];  // �ñ߶�Ӧ��ԭMESH�е������������ţ�PtRep����
    int m_aanNewEdge[2][2]; // vertex indexes of the new edge
	// newEdge[0][0/1]����һ���������ԭMESH�е���������ţ�����˵���ֱ��Ÿñ���������ԭMESH�е��涥�����ֵ���β�����Ҳ��֪����ô˵��������˵�����Indices[i*3+j]�е�i*3+j���ֵ����֪��Ϊɶ����Indices[i*3+j]������
	// newEdge[1][0/1]���ڶ�������ߡ�����������������߹���һ��QUAD��ƽ�е��������
	// ��MESH�Ķ��㻺�壬������ԭMESHÿ���������������������䣬�������壬����Ϊ��λ��������Ⱦһ��ԭMESH���棬�������QUAD��ɣ��ڱ���ԭMESH��ÿ����ʱ��������һ������ߣ���ͬһ���߳��������Σ��ģ���д����MESH����Ȼ��ͨ��PtRep������ֵ���жϵģ����䰡��
	// First subscript = index of the new edge
	// Second subscript = index of the vertex for the edge
	
public:
    CEdgeMapping()
    {
        FillMemory( m_anOldEdge, sizeof(m_anOldEdge), -1 );
        FillMemory( m_aanNewEdge, sizeof(m_aanNewEdge), -1 );
    }
};



// ֻ�Ƿ�����Ч������ֵ���ˣ���δ�����Ҳ�㣬�����е�ƥ���Ҳ��
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
            hr = D3DXCreateMesh( pInputMesh->GetNumFaces() + dwNumEdges * 2,      //��Ϊ�����������滹Ҫ��һ��ԭMESH���棬����Ҫ��NumFaces��dwNumEdges*2��ÿ���涼�������ߣ�ÿ���߶�Ҫ��չΪ�����棨QUAD�������Ծ���Faces * 3 * 2
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

                // ����д��IB�������ţ�����
                int nNextIndex = 0;

                if( pNewVBData && pdwNewIBData )
                {
                    ZeroMemory( pNewVBData, pNewMesh->GetNumVertices() * pNewMesh->GetNumBytesPerVertex() );
                    ZeroMemory( pdwNewIBData, sizeof(DWORD) * pNewMesh->GetNumFaces() * 3 );

                    // ����д��VB��ָ�룬����
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

						// ���⽫ÿ��������ԭ�ⲻ���浽pNextOutVertex�У�ֻ�ǽ�ÿ������ķ��߻����淨�ߣ���������ͬ��
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
                            // ����һ������Ϊδ�õ��������-1�ģ�����ֻҪ�ڶ��������Ϊ-1��˵���ñ�δ��������Ҫpatch
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
											// ��һ����ĺ���֣���ô�����ҵ�nVertSharedΪ2�ıߣ�2�ʹ���i2��i��ͬһ���ߣ���Ȼͬһ���Ǿ��ǹ���ߣ�i����ô������ǰ�汻��ǳ�û������أ�
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
											// ��Ŷ���ҵ�i2��iֻ�ǹ���һ��������ѣ��������жϾ��干���ĸ�����
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
