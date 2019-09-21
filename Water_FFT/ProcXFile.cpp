#include "Myd3d.h"
#include "include/rmxftmpl.h"
#include "ProcXFile.h"

/******************************Parse X File Class*****************************/
HRESULT PARSEXFILE::Parse(LPSTR pFileName)
{
	LPD3DXFILE pDXFile=NULL;
	LPD3DXFILEDATA pFileData=NULL;
	LPD3DXFILEENUMOBJECT pEnumObject=NULL;
	SIZE_T iNum=0, i=0;
	HRESULT hr = E_FAIL;
	if(pFileName == NULL)
		return E_FAIL;

	//����һ���յ�X�ļ�����
	if(FAILED(D3DXFileCreate(&pDXFile)))
		return E_FAIL;

	//ע��ͨ��ģ��
	if(FAILED(pDXFile->RegisterTemplates(D3DRM_XTEMPLATES, D3DRM_XTEMPLATE_BYTES)))
		return E_FAIL;

	//����X�ļ��е�ģ�弯�϶���
	if(FAILED(pDXFile->CreateEnumObject(pFileName, D3DXF_FILELOAD_FROMFILE, &pEnumObject)))
	{
		SAFE_RELEASE(pDXFile);
		return E_FAIL;
	}

	//������һ����������ģ�壬�����Ĺ�����ʹ��ParseChildObject�����ݹ�
	if(FAILED(pEnumObject->GetChildren(&iNum)))
		return E_FAIL;
	for(i=0; i<iNum; i++)
	{
		if(FAILED(pEnumObject->GetChild(i, &pFileData)))
		{
			SAFE_RELEASE(pEnumObject);
			return E_FAIL;
		}

		//û�и�FILEDATA�������ÿգ���һ�㣬����DataҲ�ÿգ�Depth��0
		hr = PickDataInObject(pFileData, NULL, 0, NULL, FALSE);
		SAFE_RELEASE(pFileData);
		if(FAILED(hr))
			break;		
	}

	SAFE_RELEASE(pEnumObject);
	SAFE_RELEASE(pDXFile);
	return S_OK;
}


HRESULT PARSEXFILE::ParseChildObject(LPD3DXFILEDATA pFileData, DWORD Depth, void **Data, BOOL ForceReference)
{
	LPD3DXFILEDATA pSubData=NULL;
	HRESULT hr;
	SIZE_T iNum=0, i=0;
	if(pFileData == NULL)
		return E_FAIL;

	//ɨ����ģ��������ֵܽڵ�
	if(FAILED(pFileData->GetChildren(&iNum)))
	{
		return E_FAIL;
	}
	for(i=0; i<iNum; i++)
	{
		if(FAILED(pFileData->GetChild(i, &pSubData)))
		{
			return E_FAIL;
		}
		//�ֱ���ݲ�ѯ���������Instance��Reference
		// ���������ط��������ˣ�ע�����һ�������������ģ����referenced����ô��ǿ��TRUE��������ǣ������õ��øú�����reference״̬�����ˣ�
		hr = PickDataInObject(pSubData, pFileData, Depth+1, Data, pSubData->IsReference()?TRUE:ForceReference);
		SAFE_RELEASE(pSubData);
		if(FAILED(hr))
		{
			return E_FAIL;
		}
	}// end while
	return S_OK;
}

char * PARSEXFILE::GetObjectName(LPD3DXFILEDATA pFileData)
{
	DWORD dwSize = 0;
	char *p=NULL;
	if(pFileData == NULL)
		return NULL;
	if(FAILED(pFileData->GetName(NULL, &dwSize)))
		return NULL;
	if(dwSize) p = new char[dwSize];
	if(FAILED(pFileData->GetName(p, &dwSize)) || p == NULL)
		return NULL;
	return p;
}

HRESULT PARSEXFILE::GetObjectType(LPD3DXFILEDATA pFileData, GUID *p)
{
	if(pFileData == NULL || p == NULL)
		return E_FAIL;
	return pFileData->GetType(p);
}

HRESULT PARSEXFILE::GetObjectData(LPD3DXFILEDATA pFileData, SIZE_T dwSize, void *pData)
{
	SIZE_T dwGetSize=0;
	char *pSource = NULL;
	if(pFileData == NULL)
		return E_FAIL;
	
	if(FAILED(pFileData->Lock(&dwGetSize, (const void **)&pSource)))
		return E_FAIL;
	
	if(dwGetSize == dwSize)
		memcpy(pData, pSource, dwSize);
	
	if(FAILED(pFileData->Unlock()))
		return E_FAIL;
	
	return S_OK;
}












/****************************��������****************************/
HRESULT PARSEFRAME::LoadFromX(D3DXMESHCONTAINER_EX **ppMesh,
									 D3DXFRAME_EX **ppFrame,
									 char *Filename,
									 char *TexturePath,
									 DWORD NewFVF,
									 DWORD LoadFlags)
{	
	if(!Filename || !TexturePath)
		return E_FAIL;
	if(m_CreateAttrib)
		return S_OK;

	// ����PARSE����
	m_TexturePath = TexturePath;
	m_NewFVF      = NewFVF;
	m_LoadFlags   = LoadFlags;
	m_Flags       = ((!ppMesh)?0:1) | ((!ppFrame)?0:2);
	
	// ��ʼ��ָ��
	m_RootFrame   = NULL;
	m_RootMesh    = NULL;
	
	// �����ļ�
	if(FAILED(Parse(Filename)))
		return E_FAIL;
	
	// �洢ppFrameMatrices��pBone������Ϣ�������ǰ��D3DXMESHCONTAINER_EX��˵��
	if(ppMesh && ppFrame && m_RootMesh && m_RootFrame) {
		
		// ��������MESHCONTAINER
		D3DXMESHCONTAINER_EX *pMesh = m_RootMesh;
		while(pMesh) 
		{			
			// ֻ�Ƕ�SKINMESH��Ч
			if(pMesh->pSkinInfo) 
			{				
				// ����ռ䣬ÿ�������Ӧһ��BONE
				DWORD NumBones = pMesh->pSkinInfo->GetNumBones();
				pMesh->ppFrameMatrices = new D3DXMATRIX*[NumBones];
				ZeroMemory(pMesh->ppFrameMatrices, sizeof(LPD3DXMATRIX) * NumBones);
				pMesh->pBoneMatrices   = new D3DXMATRIX[NumBones];
				
				// ��ÿ��BONE����FRAME�������ҵ�BONEͬ����FRAME
				for(DWORD i=0;i<NumBones;i++) 
				{
					const char *BoneName = pMesh->pSkinInfo->GetBoneName(i);
					D3DXFRAME_EX *pFrame = m_RootFrame->Find(BoneName);
					
					// ����ҵ����ͽ�ppFrameMatrices��Ԫ��ָ���ӦBONE�̳к�ľ����Ҳ������ÿ�
					if(pFrame)
						pMesh->ppFrameMatrices[i] = &pFrame->matCombined;
					else
						pMesh->ppFrameMatrices[i] = NULL;
				}
			}
			
			pMesh = (D3DXMESHCONTAINER_EX*)pMesh->pNextMeshContainer;
		} 
	}
	
	// �Ƚ϶��ĵĵط��������������Mesh��ֵΪm_RootMesh��Ȼ���m_RootMesh�ÿգ�ΪʲôҪ�ÿհ������Ǹ���Ҫ����ͷ��ʦ��
	if(ppMesh) 
	{
		*ppMesh = m_RootMesh;
		m_RootMesh = NULL;
	}
	else 
	{
		// Ҫ����Ϊ�յĻ�����m_RootMeshȫ�������ݣ�����Parse�˰���ģ�
		ReleaseMeshContainer(m_RootMesh);
		delete m_RootMesh;
		m_RootMesh = NULL;
	}
	
	// Frame��������һ����
	if(ppFrame) 
	{
		*ppFrame = m_RootFrame;
		m_RootFrame = NULL;
	}
	else 
	{
		ReleaseFrame(m_RootFrame);
		delete m_RootFrame;
		m_RootFrame = NULL;
	}
	
	m_CreateAttrib = 1;
	return S_OK;
}

HRESULT PARSEFRAME::LoadSkinMeshInParse(D3DXMESHCONTAINER_EX **ppMesh,
                 ID3DXFileData *pDataObj,
                 char *TexturePath,
                 DWORD NewFVF,
                 DWORD LoadFlags)
{
  ID3DXMesh *pLoadMesh = NULL;
  ID3DXSkinInfo *pSkin = NULL;
  HRESULT hr;

  if(!ppMesh || !pDataObj || !TexturePath)
    return E_FAIL;

  // ���½���MESHʹ��
  DWORD TempLoadFlags = LoadFlags;
  if(NewFVF)
    TempLoadFlags = D3DXMESH_SYSTEMMEM;

  // ����
  ID3DXBuffer *MaterialBuffer = NULL, *AdjacencyBuffer = NULL;
  DWORD NumMaterials;
  if(FAILED(hr=D3DXLoadSkinMeshFromXof(pDataObj, TempLoadFlags,
                                       d3ddevice, &AdjacencyBuffer,
                                       &MaterialBuffer, NULL,
                                       &NumMaterials, &pSkin,
                                       &pLoadMesh)))
    return hr;

  // �������SKINMESH���ͷŵ�
  if(pSkin && !pSkin->GetNumBones())
    SAFE_RELEASE(pSkin);

  // ���ǵ����ܻ����µ�FVF���󣬾��½�һ��ָ��FVF��MESH
  if(NewFVF) {
    ID3DXMesh *pTempMesh = NULL;

    if(FAILED(hr=pLoadMesh->CloneMeshFVF(LoadFlags, NewFVF, d3ddevice
		, &pTempMesh))) {
      SAFE_RELEASE(pLoadMesh);
      SAFE_RELEASE(pSkin);
      SAFE_RELEASE(MaterialBuffer);
      SAFE_RELEASE(AdjacencyBuffer);
      return hr;
    }

    // �ͷŵ�ԭ����D3DXLOAD��ȡ��MESH
    SAFE_RELEASE(pLoadMesh);
    pLoadMesh = pTempMesh; pTempMesh = NULL;
  }
 
  // �½�һ��MESHCONTAINER_EX���ú����ĵ�һ������ֻ�Ǹ�ָ�룬��δָ�����Ŀռ䣬�����ǽ���ָ��ָ������Ľṹ��
  D3DXMESHCONTAINER_EX *pMesh = new D3DXMESHCONTAINER_EX();
  ZeroMemory(pMesh, sizeof(D3DXMESHCONTAINER_EX));
  *ppMesh = pMesh;

  // ���ýṹ�壬�洢���ơ����ʡ���ͼ��MESH֮��
  DWORD Size;
  pDataObj->GetName(NULL, &Size);
  if(Size) {
    pMesh->Name = new char[Size];
    pDataObj->GetName(pMesh->Name, &Size);
  }
  pMesh->MeshData.Type = D3DXMESHTYPE_MESH;
  pMesh->MeshData.pMesh = pLoadMesh; pLoadMesh = NULL;
  pMesh->pSkinInfo = pSkin; pSkin = NULL;

  // �洢���ڽ���Ϣ
  DWORD AdjSize = AdjacencyBuffer->GetBufferSize();
  if(AdjSize) {
    pMesh->pAdjacency = (DWORD*)new char[AdjSize];
    memcpy(pMesh->pAdjacency, AdjacencyBuffer->GetBufferPointer(), AdjSize);
  }
  SAFE_RELEASE(AdjacencyBuffer);

  // ����һ��MESH�ı���
  if(pMesh->pSkinInfo)
    pMesh->MeshData.pMesh->CloneMeshFVF(D3DXMESH_MANAGED, 
                                        pMesh->MeshData.pMesh->GetFVF(), 
                                        d3ddevice, &pMesh->pSkinMesh);

  // �������ʺ���ͼ
  if(!(pMesh->NumMaterials = NumMaterials)) 
  {
    pMesh->NumMaterials = 1;
    pMesh->pMaterials   = new D3DXMATERIAL[1];
    ZeroMemory(&pMesh->pMaterials[0], sizeof(D3DXMATERIAL));
    pMesh->pTextures    = new IDirect3DTexture9*[1];
	ZeroMemory(pMesh->pTextures, sizeof(LPDIRECT3DTEXTURE9));

    pMesh->pMaterials[0].MatD3D.Diffuse.r = 1.0f;
    pMesh->pMaterials[0].MatD3D.Diffuse.g = 1.0f;
    pMesh->pMaterials[0].MatD3D.Diffuse.b = 1.0f;
    pMesh->pMaterials[0].MatD3D.Diffuse.a = 1.0f;
    pMesh->pMaterials[0].MatD3D.Ambient   = pMesh->pMaterials[0].MatD3D.Diffuse;
    pMesh->pMaterials[0].MatD3D.Specular  = pMesh->pMaterials[0].MatD3D.Diffuse;
    pMesh->pMaterials[0].pTextureFilename = NULL;
    pMesh->pTextures[0]                   = NULL;
  } 
  
  else 
  {
    D3DXMATERIAL *Materials = (D3DXMATERIAL*)MaterialBuffer->GetBufferPointer();
    pMesh->pMaterials = new D3DXMATERIAL[pMesh->NumMaterials];
	ZeroMemory(pMesh->pMaterials, sizeof(D3DXMATERIAL) * pMesh->NumMaterials);
    pMesh->pTextures  = new IDirect3DTexture9*[pMesh->NumMaterials];
	ZeroMemory(pMesh->pMaterials, sizeof(LPDIRECT3DTEXTURE9) * pMesh->NumMaterials);

    for(DWORD i=0;i<pMesh->NumMaterials;i++) 
	{
      pMesh->pMaterials[i].MatD3D = Materials[i].MatD3D;
      pMesh->pMaterials[i].MatD3D.Ambient = pMesh->pMaterials[i].MatD3D.Diffuse;

      // ������ͼ
      pMesh->pTextures[i] = NULL;
      if(Materials[i].pTextureFilename) 
	  {
        char TextureFile[MAX_PATH];
        sprintf(TextureFile, "%s\\%s", TexturePath, 
                             Materials[i].pTextureFilename);
        D3DXCreateTextureFromFile(d3ddevice,
                                  TextureFile,
                                  &pMesh->pTextures[i]);
      }
    }
  }
  SAFE_RELEASE(MaterialBuffer);

  // �Ż�MESH
  pMesh->MeshData.pMesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT, NULL, NULL, NULL, NULL);

  // ���
  pMesh = NULL;

  return S_OK;
}




HRESULT PARSEFRAME::PickDataInObject(LPD3DXFILEDATA pSubData, LPD3DXFILEDATA pParentData, DWORD Depth, void **Data, BOOL IsReference)
{
	if(pSubData == NULL)
		return E_FAIL;
	GUID Guid;

	GetObjectType(pSubData, &Guid);	
	// ����FRAME�̳У���������ݣ�ֻ����Է�Referenceģ�壩
	if(Guid == TID_D3DRMFrame && IsReference == FALSE && m_Flags & 2)
	{	
		// ����һ����ģ�壬���ƺ�FileDataһ��
		D3DXFRAME_EX *pFrame = new D3DXFRAME_EX();
		ZeroMemory(pFrame, sizeof(D3DXFRAME_EX));

		if(pFrame == NULL)
		{
			return E_FAIL;
		}
		pFrame->Name = GetObjectName(pSubData);

		// ��ֲ��ĵط�������DATAָ���������������Ի�һ�����̣���Ȼ��������
		// ֻ�д�����ģ���ʱ��DATA�Ż�ΪNULL���Ὣ���еĶ���ģ����������Ϊsibling��ע��m_rootframeÿ�ζ����ƶ�һ����ֱ���Ƶ����һ������ģ��Ϊֹ����������ͷָ������Ƿ����ƶ��ģ������ƶ����µ�����֮��ԭ�����Ǹ��ͱ���ȷ�ĸ�ֵ�ˣ�����next��ָ���·����
		// ���ֻ�ǵ�������Ļ�����ס������Զ����ԭ���ṹ����ֵ�ָ��ָ��root��Ȼ��root��ָ���´�����
		// Data��������if֮ǰ���ܱ�֤������һ���½���Frameָ�룬��������if֮���ܱ�֤���Ǳ����½���Frameָ��
		// m_RootFrame�ܱ�֤�����´�����Frame
		if(Data == NULL) 
		{
			// �����еĶ���ģ������Ϊsibling�����m_rootframeδ���䣨�������һ������ģ��ʱ�������Զ������
			pFrame->pFrameSibling = m_RootFrame;
			m_RootFrame = pFrame; pFrame = NULL;
			Data = (void**)&m_RootFrame;
		}
		else {
			// ��ģ�壬ȫ��Ϊchild��ע��ݹ���øú�����˳��������һ��������ײ�ĺ��ӣ�Ȼ�������ϵݹ�
			// ����֮�󣬻���һ�Σ�Data��ŵײ㺢�ӵĸ��ף��µľ���ΪSibling������Data�ĺ��ӣ����ڶ��䣬���ǿֲ�����ʦ���Ǵ�ʦ���ⶼ�������
			D3DXFRAME_EX *pFramePtr = (D3DXFRAME_EX*)*Data;
			pFrame->pFrameSibling = pFramePtr->pFrameFirstChild;
			pFramePtr->pFrameFirstChild = pFrame; pFrame = NULL;
			Data = (void**)&pFramePtr->pFrameFirstChild;
		}
	}
	// ����Frame���󣬱Ƚϼ򵥣�ע�������if������֮��Data���Ǳ����½���Frameָ���ˣ�����ֱ���������������
	GetObjectType(pSubData, &Guid);
	if(Guid == TID_D3DRMFrameTransformMatrix && IsReference == FALSE && m_Flags & 2 && Data)
	{
		D3DXFRAME_EX *Frame = (D3DXFRAME_EX*)*Data;
		if(Frame) 
		{
			GetObjectData(pSubData, sizeof(D3DXMATRIX), (void *)&(Frame->TransformationMatrix));
			Frame->matOriginal = Frame->TransformationMatrix;
		}
	}
	// ����MESHģ�壬��ȡSKINMESH
	GetObjectType(pSubData, &Guid);
	if(Guid == TID_D3DRMMesh && m_Flags & 1) 
	{
		// ��Reference Meshģ��
		if(IsReference == FALSE) 
		{
			//mymessage(GetObjectName(pSubData));
			D3DXMESHCONTAINER_EX *pMesh = NULL;
			//ͨ������������������Ǹ���ָ�����һ���ṹ��
			LoadSkinMeshInParse(&pMesh, pSubData, m_TexturePath, m_NewFVF, m_LoadFlags);
			
			// ��ֻ����ͨ����������FRAMEһ����m_RootMesh��ͣ���ƣ��Ƿ����
			if(pMesh) 
			{
				pMesh->pNextMeshContainer = m_RootMesh;
				m_RootMesh = pMesh; pMesh = NULL;
				
				// ��MESHCONTAINER����FRAME�ϣ�DATA��������ø�FRAMEһ������һ��ģ�岻��
				if(Data) 
				{
					D3DXFRAME_EX *pFrame = (D3DXFRAME_EX*)*Data;
					if(m_Flags & 2 && pFrame)
						pFrame->pMeshContainer = m_RootMesh;
				}
			}
		}//end non-reference
		
		// Reference Meshģ��
		else {
				// �ο�ģ��ض�������ʵ��ģ�壬�����ǲ��ô���MESHCONTAINER��
				if(Data) 
				{
					D3DXFRAME_EX *pFrame = (D3DXFRAME_EX*)*Data;
					if(m_Flags & 2 && m_RootMesh && pFrame) 
					{
						// �ҵ�ǰ���Ƿ����Ѵ�����ͬ��MESHCONTAINER�����û�У����Ǹ���ָ��
						char *Name = GetObjectName(pSubData);
						if(Name) 
						{
							// �ҵ��ˣ�����ȥ
							pFrame->pMeshContainer = m_RootMesh->Find(Name);
							delete [] Name; Name = NULL;
						}
					}
				}
			}//end reference
	}// end type=TID_MESH

	//�ݹ�
	return(ParseChildObject(pSubData, Depth, Data, IsReference));
}


void PARSEFRAME::Release()
{
	if(!m_CreateAttrib)
		return;
	//ReleaseMeshContainer(m_RootMesh);
	delete m_RootMesh;  m_RootMesh  = NULL;
	//ReleaseFrame(m_RootFrame);
	delete m_RootFrame; m_RootFrame = NULL;
	m_CreateAttrib = 0;
}


PARSEFRAME::PARSEFRAME()
{
	m_CreateAttrib = 0;
	m_TexturePath = NULL;
	m_Flags       = 3;
	m_NewFVF      = 0;
	delete m_RootMesh;
	m_RootMesh    = NULL;
	delete m_RootFrame;
	m_RootFrame   = NULL;
}

PARSEFRAME::~PARSEFRAME()
{
	Release();
}



void ReleaseSplitMesh(SplitMesh *p)
{
	if(!p)
		return;
	if(p->pNextMesh)
		ReleaseSplitMesh(p->pNextMesh);
	else if(p)
	{
		p->dwSubset = 0;
		p->Type = 0;
		p->dwMatrixNum = 0;
		p->dwFaceNum = 0;
		p->dwVertexNum = 0;
		p->dwVertexSize = 0;
		p->dwDataVertexSize = 0;
		p->FVF = D3DFVF_XYZ;
		SAFE_RELEASE(p->pIB);
		SAFE_RELEASE(p->pVB);
		SAFE_RELEASE(p->pDB);
		delete [] p->pMatrix;
		p->pMatrix = NULL;
		delete [] p->pMatrixMapping;
		p->pMatrixMapping = NULL;
	}		
}


void PARSEFRAME::ReleaseMeshContainer(D3DXMESHCONTAINER_EX *p)
{
	if(!p)
		return;
	if(p->pNextMeshContainer)
		ReleaseMeshContainer((D3DXMESHCONTAINER_EX *)(p->pNextMeshContainer));
	else if(p)
	{
		delete p->Name;
		p->Name = NULL;

		if(p->MeshData.Type == D3DXMESHTYPE_MESH)
			SAFE_RELEASE(p->MeshData.pMesh);
		if(p->MeshData.Type == D3DXMESHTYPE_PATCHMESH)
			SAFE_RELEASE(p->MeshData.pPatchMesh);
		if(p->MeshData.Type == D3DXMESHTYPE_PMESH)
			SAFE_RELEASE(p->MeshData.pPMesh);
		
		delete p->pMaterials->pTextureFilename;
		p->pMaterials->pTextureFilename = NULL;
		
		SAFE_RELEASE(p->pSkinInfo);
		delete [] p->pAdjacency;
		p->pAdjacency = NULL;

		if(p->pTextures)
		{
			for(UINT i=0; i<p->NumMaterials; i++)
				SAFE_RELEASE((p->pTextures)[i]);
			delete [] p->pTextures;
			p->pTextures = NULL;
		}

		SAFE_RELEASE(p->pSkinMesh);
		delete [] p->pBoneMatrices;
		p->pBoneMatrices = NULL;
		delete [] p->ppFrameMatrices;
		p->ppFrameMatrices = NULL;

		ReleaseSplitMesh(p->pSplitMesh);
		delete p->pSplitMesh;
		p->pSplitMesh = NULL;
	}
}

void PARSEFRAME::ReleaseFrame(D3DXFRAME_EX *p)
{
	if(!p)
		return;
	// Release sibling frames
	D3DXFRAME_EX *pFramePtr;
	if((pFramePtr = (D3DXFRAME_EX*)p->pFrameSibling))
		ReleaseFrame(pFramePtr);
		
	// Release child frames
	if((pFramePtr = (D3DXFRAME_EX*)p->pFrameFirstChild))
		ReleaseFrame(pFramePtr);

	delete p->Name;
	p->Name = NULL;
	/**********ע��ReleaseMeshContainer((D3DXMESHCONTAINER_EX *)(p->pMeshContainer));
	delete p->pMeshContainer;**********/
	p->pMeshContainer = NULL;
}


HRESULT PARSEFRAME::UpdateMesh(D3DXMESHCONTAINER_EX *pMesh)
{
	if(!pMesh || !m_CreateAttrib)
		return E_FAIL;
	if(!pMesh->MeshData.pMesh || !pMesh->pSkinMesh || !pMesh->pSkinInfo)
		return E_FAIL;
	if(!pMesh->pBoneMatrices || !pMesh->ppFrameMatrices)
		return E_FAIL;
	
	// ��ÿ����������������pBone����Offset * Hierarchy��
	for(DWORD i=0;i<pMesh->pSkinInfo->GetNumBones();i++) 
	{
		pMesh->pBoneMatrices[i] = (*pMesh->pSkinInfo->GetBoneOffsetMatrix(i));
		if(pMesh->ppFrameMatrices[i])
			pMesh->pBoneMatrices[i] *= (*pMesh->ppFrameMatrices[i]);
	}
	
	// ��ʼ����MESH�������ݣ�ע����ҪͬʱLOCKԴMESH��Ŀ��MESH��ֻ��Ҫ����VB������Ҫ����IB
	void *SrcPtr, *DestPtr;
	pMesh->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&SrcPtr);
	pMesh->pSkinMesh->LockVertexBuffer(0, (void**)&DestPtr);
	
	pMesh->pSkinInfo->UpdateSkinnedMesh(pMesh->pBoneMatrices, NULL, SrcPtr, DestPtr);
	
	pMesh->pSkinMesh->UnlockVertexBuffer();
	pMesh->MeshData.pMesh->UnlockVertexBuffer();
	
	// ���
	return S_OK;
}














/****************************Animation************************/
SKINANIMATION::SKINANIMATION()
{
	m_CreateAttrib = 0;
	m_pRootAnimationSet = NULL;
}
SKINANIMATION::~SKINANIMATION()
{
	Release();
}

void SKINANIMATION::Release()
{
	if(!m_CreateAttrib)
		return;
	//ReleaseAnimationSet(m_pRootAnimationSet);
	delete m_pRootAnimationSet;
	m_pRootAnimationSet = NULL;
	m_CreateAttrib = 0;
}

void SKINANIMATION::ReleaseAnimation(Animation *p)
{
	if(!p)
		return;
	if(p->pNextAnimation)
		ReleaseAnimation(p->pNextAnimation);
	else if(p)
	{
		delete p->pName;
		p->pName = NULL;
		delete p->pBone;
		p->pBone = NULL;
		delete p->pNextAnimation;
		p->pNextAnimation = NULL;
		delete [] p->pRotation;
		p->pRotation = NULL;
		delete [] p->pTranslation;
		p->pTranslation = NULL;
		delete [] p->pScaling;
		p->pScaling = NULL;
		delete [] p->pMatrix;
		p->pMatrix = NULL;
	}
}

void SKINANIMATION::ReleaseAnimationSet(AnimationSet *p)
{
	if(!p)
		return;
	if(p->pNextAnimationSet)
		ReleaseAnimationSet(p->pNextAnimationSet);
	else if(p)
	{
		delete p->pName;
		p->pName = NULL;
		delete p->pNextAnimationSet;
		p->pNextAnimationSet = NULL;
		ReleaseAnimation(p->pHeadAnimation);
	}
}


HRESULT SKINANIMATION::LoadFromX(LPSTR pFileName)
{
	if(m_CreateAttrib)
		return S_OK;
	// ���������m_RootAnimationSet
	if(FAILED(Parse(pFileName)))
		return E_FAIL;
	
	m_CreateAttrib = 1;
	return S_OK;
}

HRESULT SKINANIMATION::SetAnimation(LPSTR pAnimationSetName, UINT iAnimationSetIndex, DWORD dwTime, BOOL bLoop)
{
	if(!m_pRootAnimationSet || !m_CreateAttrib)
		return E_FAIL;
	Animation *pAnimation = NULL;
	AnimationSet *pAnimationSet = m_pRootAnimationSet;
	UINT i = 0;
	float fMixScale = 0;
	D3DXQUATERNION QResult;
	D3DXVECTOR3 VResult;

	// �ȱ������еĶ������ҵ�����ƥ��ģ����������Ч���Ͱ����ֲ��ң����������Ч���Ͱ��������ң��������ȼ��ߣ�
	if(pAnimationSetName)
		for(; pAnimationSet ; pAnimationSet=pAnimationSet->pNextAnimationSet)
			if(0 == strcmp(pAnimationSet->pName, pAnimationSetName))
				break;
			else
				return E_FAIL;
	else
		for(i=0; i<iAnimationSetIndex; i++)
		{
			pAnimationSet=pAnimationSet->pNextAnimationSet;
			if(!pAnimationSet)
				return E_FAIL;
		}

	// �����Ƿ�ѭ����������ʱ�䣬�����ѭ������ֹͣ�����Ķ�����
	if(dwTime >= pAnimationSet->TimeLength)
	{
		dwTime = (bLoop) ? (dwTime%pAnimationSet->TimeLength) : pAnimationSet->TimeLength;
	}
	// �����ö��������е�Animation����������յĲ�ֵ���󲢴�ŵ���Ӧ��Bone��
	pAnimation = pAnimationSet->pHeadAnimation;
	while(pAnimation)
	{
		// ���û�к͹���������������Animation
		if(!pAnimation->pBone)
		{
			pAnimation = pAnimation->pNextAnimation;
			continue;
		}

		D3DXMatrixIdentity(&pAnimation->pBone->TransformationMatrix);
		// ��������������
		switch(pAnimation->dwType)
		{
		case 0:
			// ��ת���ж��Ƿ���Ч
			if(!pAnimation->dwRotationNum || !pAnimation->pRotation)
				break;
			TimeFloatKeysQuaternion *pQKey1, *pQKey2;
			pQKey1 = pQKey2 = pAnimation->pRotation;
		
			// ���ȸ���ʱ�����Ӧ��ѡ�������ؼ�֡
			for(i=0; i<pAnimation->dwRotationNum; i++)
			{
				if((pAnimation->pRotation)[i].dwTime <= dwTime)
					pQKey1 = pAnimation->pRotation+i;
				if((pAnimation->pRotation)[i].dwTime >= dwTime)
				{
					pQKey2 = pAnimation->pRotation+i;
					break;
				}
			}
			
			// ���ݵ�ǰʱ��͵�һ���ؼ�֡��ʱ����������ϱ���ϵ����ע����������ؼ�֡��ȣ�����time�Ѿ����ù��������ľ�ͷ������ֱ��ȡ����һ����ֵ����
			// ����ҲҪע��dwTime������0����ôpKey1/2Ҳ�����0����ʱ�ķ�ĸ�����ٺ�
			if(pQKey2->dwTime == pQKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pQKey1->dwTime) / (float)(pQKey2->dwTime-pQKey1->dwTime);
			// �����Ͻ������ת��Ϊ����洢
			D3DXQuaternionSlerp(&QResult, &pQKey1->Quaternion, &pQKey2->Quaternion, fMixScale);
			D3DXMatrixRotationQuaternion(&pAnimation->pBone->TransformationMatrix, &QResult);
			break;
		
		case 1:
			// ���ţ��ж��Ƿ���Ч
			if(!pAnimation->dwScalingNum || !pAnimation->pScaling)
				break;
			TimeFloatKeysVector *pSKey1, *pSKey2;
			pSKey1 = pSKey2 = pAnimation->pScaling;
			
			// ���ȸ���ʱ�����Ӧ��ѡ�������ؼ�֡
			for(i=0; i<pAnimation->dwScalingNum; i++)
			{
				if((pAnimation->pScaling)[i].dwTime <= dwTime)
					pSKey1 = pAnimation->pScaling+i;
				if((pAnimation->pScaling)[i].dwTime >= dwTime)
				{
					pSKey2 = pAnimation->pScaling+i;
					break;
				}
			}
			
			// ���ݵ�ǰʱ��͵�һ���ؼ�֡��ʱ����������ϱ���ϵ����ע����������ؼ�֡��ȣ�����time�Ѿ����ù��������ľ�ͷ������ֱ��ȡ����һ����ֵ����
			// ����ҲҪע��dwTime������0����ôpKey1/2Ҳ�����0����ʱ�ķ�ĸ�����ٺ�
			if(pSKey2->dwTime == pSKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pSKey1->dwTime) / (float)(pSKey2->dwTime-pSKey1->dwTime);
			// �����Ͻ������ת��Ϊ����洢
			VResult = (pSKey2->Vec - pSKey1->Vec) * fMixScale + pSKey1->Vec;
			D3DXMatrixScaling(&pAnimation->pBone->TransformationMatrix, VResult.x, VResult.y, VResult.z);
			break;

		case 2:
			// ƽ�ƣ��ж��Ƿ���Ч
			if(!pAnimation->dwTranslationNum || !pAnimation->pTranslation)
				break;
			TimeFloatKeysVector *pTKey1, *pTKey2;
			pTKey1 = pTKey2 = pAnimation->pTranslation;
			
			// ���ȸ���ʱ�����Ӧ��ѡ�������ؼ�֡
			for(i=0; i<pAnimation->dwTranslationNum; i++)
			{
				if((pAnimation->pTranslation)[i].dwTime <= dwTime)
					pTKey1 = pAnimation->pTranslation+i;
				if((pAnimation->pTranslation)[i].dwTime >= dwTime)
				{
					pTKey2 = pAnimation->pTranslation+i;
					break;
				}
			}
			
			// ���ݵ�ǰʱ��͵�һ���ؼ�֡��ʱ����������ϱ���ϵ����ע����������ؼ�֡��ȣ�����time�Ѿ����ù��������ľ�ͷ������ֱ��ȡ����һ����ֵ����
			// ����ҲҪע��dwTime������0����ôpKey1/2Ҳ�����0����ʱ�ķ�ĸ�����ٺ�
			if(pTKey2->dwTime == pTKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pTKey1->dwTime) / (float)(pTKey2->dwTime-pTKey1->dwTime);
			// �����Ͻ������ת��Ϊ����洢
			VResult = (pTKey2->Vec - pTKey1->Vec) * fMixScale + pTKey1->Vec;
			D3DXMatrixTranslation(&pAnimation->pBone->TransformationMatrix, VResult.x, VResult.y, VResult.z);
			break;

		case 4:
			// �����ж��Ƿ���Ч
			if(!pAnimation->dwMatrixNum || !pAnimation->pMatrix)
				break;
			TimeFloatKeysMatrix *pMKey1, *pMKey2;
			pMKey1 = pMKey2 = pAnimation->pMatrix;
			
			// ���ȸ���ʱ�����Ӧ��ѡ�������ؼ�֡
			for(i=0; i<pAnimation->dwMatrixNum; i++)
			{
				if((pAnimation->pMatrix)[i].dwTime <= dwTime)
					pMKey1 = pAnimation->pMatrix+i;
				if((pAnimation->pMatrix)[i].dwTime >= dwTime)
				{
					pMKey2 = pAnimation->pMatrix+i;
					break;
				}
			}
			
			// ���ݵ�ǰʱ��͵�һ���ؼ�֡��ʱ����������ϱ���ϵ����ע����������ؼ�֡��ȣ�����time�Ѿ����ù��������ľ�ͷ������ֱ��ȡ����һ����ֵ����
			// ����ҲҪע��dwTime������0����ôpKey1/2Ҳ�����0����ʱ�ķ�ĸ�����ٺ�
			if(pMKey2->dwTime == pMKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pMKey1->dwTime) / (float)(pMKey2->dwTime-pMKey1->dwTime);
			// �����Ͻ������ת��Ϊ����洢
			pAnimation->pBone->TransformationMatrix = (pMKey2->Mat - pMKey1->Mat) * fMixScale + pMKey1->Mat;

			break;
		}//end switch
		
		pAnimation = pAnimation->pNextAnimation;
	}
	
	return S_OK;
}

HRESULT SKINANIMATION::AnimationBlend(LPSTR pAnimationSetName, UINT iAnimationSetIndex, DWORD dwTime, BOOL bLoop, float fScale)
{
	if(!m_pRootAnimationSet || !m_CreateAttrib)
		return E_FAIL;
	Animation *pAnimation = NULL;
	AnimationSet *pAnimationSet = m_pRootAnimationSet;
	UINT i = 0;
	float fMixScale = 0;
	D3DXMATRIX matResult, matOffset;
	D3DXQUATERNION QResult;
	D3DXVECTOR3 VResult;

	// �ȱ������еĶ������ҵ�����ƥ��ģ����������Ч���Ͱ����ֲ��ң����������Ч���Ͱ��������ң��������ȼ��ߣ�
	if(pAnimationSetName)
		for(; pAnimationSet ; pAnimationSet=pAnimationSet->pNextAnimationSet)
			if(0 == strcmp(pAnimationSet->pName, pAnimationSetName))
				break;
			else
				return E_FAIL;
	else
		for(i=0; i<iAnimationSetIndex; i++)
		{
			pAnimationSet=pAnimationSet->pNextAnimationSet;
			if(!pAnimationSet)
				return E_FAIL;
		}

	// �����Ƿ�ѭ����������ʱ�䣬�����ѭ������ֹͣ�����Ķ�����
	if(dwTime >= pAnimationSet->TimeLength)
	{
		dwTime = (bLoop) ? (dwTime%pAnimationSet->TimeLength) : pAnimationSet->TimeLength;
	}
	// �����ö��������е�Animation����������յĲ�ֵ���󲢴�ŵ���Ӧ��Bone��
	pAnimation = pAnimationSet->pHeadAnimation;
	while(pAnimation)
	{
		// ���û�к͹���������������Animation
		if(!pAnimation->pBone)
		{
			pAnimation = pAnimation->pNextAnimation;
			continue;
		}

		D3DXMatrixIdentity(&matResult);
		D3DXMatrixIdentity(&matOffset);

		// ��������������
		switch(pAnimation->dwType)
		{
		case 0:
			// ��ת���ж��Ƿ���Ч
			if(!pAnimation->dwRotationNum || !pAnimation->pRotation)
				break;
			TimeFloatKeysQuaternion *pQKey1, *pQKey2;
			pQKey1 = pQKey2 = pAnimation->pRotation;
		
			// ���ȸ���ʱ�����Ӧ��ѡ�������ؼ�֡
			for(i=0; i<pAnimation->dwRotationNum; i++)
			{
				if((pAnimation->pRotation)[i].dwTime <= dwTime)
					pQKey1 = pAnimation->pRotation+i;
				if((pAnimation->pRotation)[i].dwTime >= dwTime)
				{
					pQKey2 = pAnimation->pRotation+i;
					break;
				}
			}
			
			// ���ݵ�ǰʱ��͵�һ���ؼ�֡��ʱ����������ϱ���ϵ����ע����������ؼ�֡��ȣ�����time�Ѿ����ù��������ľ�ͷ������ֱ��ȡ����һ����ֵ����
			// ����ҲҪע��dwTime������0����ôpKey1/2Ҳ�����0����ʱ�ķ�ĸ�����ٺ�
			if(pQKey2->dwTime == pQKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pQKey1->dwTime) / (float)(pQKey2->dwTime-pQKey1->dwTime);
			// �����Ͻ������ת��Ϊ����洢
			D3DXQuaternionSlerp(&QResult, &pQKey1->Quaternion, &pQKey2->Quaternion, fMixScale);
			D3DXMatrixRotationQuaternion(&matResult, &QResult);
			break;
		
		case 1:
			// ���ţ��ж��Ƿ���Ч
			if(!pAnimation->dwScalingNum || !pAnimation->pScaling)
				break;
			TimeFloatKeysVector *pSKey1, *pSKey2;
			pSKey1 = pSKey2 = pAnimation->pScaling;
			
			// ���ȸ���ʱ�����Ӧ��ѡ�������ؼ�֡
			for(i=0; i<pAnimation->dwScalingNum; i++)
			{
				if((pAnimation->pScaling)[i].dwTime <= dwTime)
					pSKey1 = pAnimation->pScaling+i;
				if((pAnimation->pScaling)[i].dwTime >= dwTime)
				{
					pSKey2 = pAnimation->pScaling+i;
					break;
				}
			}
			
			// ���ݵ�ǰʱ��͵�һ���ؼ�֡��ʱ����������ϱ���ϵ����ע����������ؼ�֡��ȣ�����time�Ѿ����ù��������ľ�ͷ������ֱ��ȡ����һ����ֵ����
			// ����ҲҪע��dwTime������0����ôpKey1/2Ҳ�����0����ʱ�ķ�ĸ�����ٺ�
			if(pSKey2->dwTime == pSKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pSKey1->dwTime) / (float)(pSKey2->dwTime-pSKey1->dwTime);
			// �����Ͻ������ת��Ϊ����洢
			VResult = (pSKey2->Vec - pSKey1->Vec) * fMixScale + pSKey1->Vec;
			D3DXMatrixScaling(&matResult, VResult.x, VResult.y, VResult.z);
			break;

		case 2:
			// ƽ�ƣ��ж��Ƿ���Ч
			if(!pAnimation->dwTranslationNum || !pAnimation->pTranslation)
				break;
			TimeFloatKeysVector *pTKey1, *pTKey2;
			pTKey1 = pTKey2 = pAnimation->pTranslation;
			
			// ���ȸ���ʱ�����Ӧ��ѡ�������ؼ�֡
			for(i=0; i<pAnimation->dwTranslationNum; i++)
			{
				if((pAnimation->pTranslation)[i].dwTime <= dwTime)
					pTKey1 = pAnimation->pTranslation+i;
				if((pAnimation->pTranslation)[i].dwTime >= dwTime)
				{
					pTKey2 = pAnimation->pTranslation+i;
					break;
				}
			}
			
			// ���ݵ�ǰʱ��͵�һ���ؼ�֡��ʱ����������ϱ���ϵ����ע����������ؼ�֡��ȣ�����time�Ѿ����ù��������ľ�ͷ������ֱ��ȡ����һ����ֵ����
			// ����ҲҪע��dwTime������0����ôpKey1/2Ҳ�����0����ʱ�ķ�ĸ�����ٺ�
			if(pTKey2->dwTime == pTKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pTKey1->dwTime) / (float)(pTKey2->dwTime-pTKey1->dwTime);
			// �����Ͻ������ת��Ϊ����洢
			VResult = (pTKey2->Vec - pTKey1->Vec) * fMixScale + pTKey1->Vec;
			D3DXMatrixTranslation(&matResult, VResult.x, VResult.y, VResult.z);
			break;

		case 4:
			// �����ж��Ƿ���Ч
			if(!pAnimation->dwMatrixNum || !pAnimation->pMatrix)
				break;
			TimeFloatKeysMatrix *pMKey1, *pMKey2;
			pMKey1 = pMKey2 = pAnimation->pMatrix;
			
			// ���ȸ���ʱ�����Ӧ��ѡ�������ؼ�֡
			for(i=0; i<pAnimation->dwMatrixNum; i++)
			{
				if((pAnimation->pMatrix)[i].dwTime <= dwTime)
					pMKey1 = pAnimation->pMatrix+i;
				if((pAnimation->pMatrix)[i].dwTime >= dwTime)
				{
					pMKey2 = pAnimation->pMatrix+i;
					break;
				}
			}
			
			// ���ݵ�ǰʱ��͵�һ���ؼ�֡��ʱ����������ϱ���ϵ����ע����������ؼ�֡��ȣ�����time�Ѿ����ù��������ľ�ͷ������ֱ��ȡ����һ����ֵ����
			// ����ҲҪע��dwTime������0����ôpKey1/2Ҳ�����0����ʱ�ķ�ĸ�����ٺ�
			if(pMKey2->dwTime == pMKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pMKey1->dwTime) / (float)(pMKey2->dwTime-pMKey1->dwTime);
			// �����Ͻ������ת��Ϊ����洢
			matResult = (pMKey2->Mat - pMKey1->Mat) * fMixScale + pMKey1->Mat;
			break;
		}//end switch
		
		// �Ѿ���������¶����Ĳ�ֵ�������ڿ�ʼ���
		matOffset = matResult - pAnimation->pBone->matOriginal;
		matOffset *= fScale;
		pAnimation->pBone->TransformationMatrix += matOffset;

		// ����ѭ��
		pAnimation = pAnimation->pNextAnimation;
	}

	return S_OK;
}








HRESULT SKINANIMATION::Map(LPSTR pAnimationSetName, UINT iAnimationSetIndex, D3DXFRAME_EX *pRootFrame)
{
	if(!pRootFrame || !m_pRootAnimationSet || !m_pRootAnimationSet->pHeadAnimation || !m_CreateAttrib)
		return E_FAIL;

	AnimationSet *pAnimationSet = m_pRootAnimationSet;

	// �ȱ������еĶ������ҵ�����ƥ��ģ����������Ч���Ͱ����ֲ��ң����������Ч���Ͱ��������ң��������ȼ��ߣ�
	if(pAnimationSetName)
		for(; pAnimationSet ; pAnimationSet=pAnimationSet->pNextAnimationSet)
			if(0 == strcmp(pAnimationSet->pName, pAnimationSetName))
				break;
			else
				return E_FAIL;
			else
				for(UINT i=0; i<iAnimationSetIndex; i++)
				{
					pAnimationSet=pAnimationSet->pNextAnimationSet;
					if(!pAnimationSet)
						return E_FAIL;
				}

	// ����ÿ��Animation��ӳ�䵽��Ӧ��BONE(D3DXFRAME_EX)
	D3DXFRAME_EX *pFrame = NULL;
	Animation *pAnimation = pAnimationSet->pHeadAnimation;
	for(; pAnimation ; pAnimation=pAnimation->pNextAnimation)
	{
		pFrame = pRootFrame->Find(pAnimation->pName);
		// �����Ҳ�����Ҳ�����ÿ�
		pAnimation->pBone = pFrame;
	}
	return S_OK;
}

HRESULT SKINANIMATION::PickDataInObject(LPD3DXFILEDATA pSubData, LPD3DXFILEDATA pParentData, DWORD Depth, void **Data, BOOL IsReference)
{
	if(pSubData == NULL)
		return E_FAIL;

	GUID Guid;
	Animation *pAnimation = NULL;

	// ����AnimationSet����
	GetObjectType(pSubData, &Guid);
	if(Guid == TID_D3DRMAnimationSet)
	{	
		// ����һ����AnimationSet�����ƺ�FileDataһ��
		AnimationSet *pAnimationSet = new AnimationSet;
		ZeroMemory(pAnimationSet, sizeof(AnimationSet));

		if(pAnimationSet == NULL)
			return E_FAIL;
		pAnimationSet->pName = GetObjectName(pSubData);

		// ��������������ס������Զ����ԭ���ṹ���Nextָ��ָ��root��Ȼ��root��ָ���´�����
		// ��������֮��m_pRootAnimationSet��Զָ�����´�����AnimationSet���ں������ֱ����
		// ������Ȼ�Ƿ���������m_pRootAnimationSet��ָ�����һ�������ģ�����Ϊ�������������������ģ�����˳������ν��
		pAnimationSet->pNextAnimationSet = m_pRootAnimationSet;
		m_pRootAnimationSet = pAnimationSet;
	}

	// ������ǰAnimationSet��Animation����pHeadAnimation��m_RootAnimationSetһ�����÷������Ž�����
	GetObjectType(pSubData, &Guid);
	if(Guid == TID_D3DRMAnimation && m_pRootAnimationSet)
	{
		pAnimation = new Animation;
		ZeroMemory(pAnimation, sizeof(Animation));

		pAnimation->pNextAnimation = m_pRootAnimationSet->pHeadAnimation;
		m_pRootAnimationSet->pHeadAnimation = pAnimation;
	}


	// ���ģ�����ÿ�����������AnimationKey����
	GetObjectType(pSubData, &Guid);
	if(Guid == TID_D3DRMAnimationKey && m_pRootAnimationSet && m_pRootAnimationSet->pHeadAnimation)
	{
		// �õ�Դ����ָ���Ŀ������ָ�룬֮������DWORD����Ϊ����4�ֽڣ�����ͨ��dword��float��
		pAnimation = m_pRootAnimationSet->pHeadAnimation;
		DWORD *p = NULL, Length = 0;
		SIZE_T uSize = 0;
		UINT i = 0;
		if(FAILED(pSubData->Lock(&uSize, (const void **)&p)))
			return E_FAIL;

		pAnimation->dwType = *p;
		p++;
		switch(pAnimation->dwType)
		{
		// ��ת
		case 0:
			pAnimation->dwRotationNum = *p;
			p++;
			pAnimation->pRotation = new TimeFloatKeysQuaternion[pAnimation->dwRotationNum];
			ZeroMemory(pAnimation->pRotation, sizeof(TimeFloatKeysQuaternion) * pAnimation->dwRotationNum);

			for(i=0; i<pAnimation->dwRotationNum; i++)
			{
				// �õ���Animation���˶�ʱ�䣬��������ʱ�䣬����ʱ����Զ�������
				(pAnimation->pRotation)[i].dwTime = *p;
				if(m_pRootAnimationSet->TimeLength < (pAnimation->pRotation)[i].dwTime)
				{
					m_pRootAnimationSet->TimeLength = (pAnimation->pRotation)[i].dwTime;
				}
				p++;
				Length = *p;
				p++;
				// memset QUATERNION�����������ص�=��ֱ�Ӹ�ֵ
				(pAnimation->pRotation)[i].Quaternion = *(D3DXQUATERNION *)p;
				p += Length;
			}
			break;
		
		// ����
		case 1:
			pAnimation->dwScalingNum = *p;
			p++;
			pAnimation->pScaling = new TimeFloatKeysVector[pAnimation->dwScalingNum];
			ZeroMemory(pAnimation->pScaling, sizeof(TimeFloatKeysVector) * pAnimation->dwScalingNum);
			
			for(i=0; i<pAnimation->dwScalingNum; i++)
			{
				// �õ���Animation���˶�ʱ�䣬��������ʱ�䣬����ʱ����Զ�������
				(pAnimation->pScaling)[i].dwTime = *p;
				if(m_pRootAnimationSet->TimeLength < (pAnimation->pScaling)[i].dwTime)
				{
					m_pRootAnimationSet->TimeLength = (pAnimation->pScaling)[i].dwTime;
				}
				p++;
				Length = *p;
				p++;
				// memset VEC�����������ص�=��ֱ�Ӹ�ֵ
				(pAnimation->pScaling)[i].Vec = *(D3DXVECTOR3 *)p;
				p += Length;
			}
			break;
			
			// ƽ��
		case 2:
			pAnimation->dwTranslationNum = *p;
			p++;
			pAnimation->pTranslation = new TimeFloatKeysVector[pAnimation->dwTranslationNum];
			for(i=0; i<pAnimation->dwTranslationNum; i++)
			{
				// �õ���Animation���˶�ʱ�䣬��������ʱ�䣬����ʱ����Զ�������
				(pAnimation->pTranslation)[i].dwTime = *p;
				if(m_pRootAnimationSet->TimeLength < (pAnimation->pTranslation)[i].dwTime)
				{
					m_pRootAnimationSet->TimeLength = (pAnimation->pTranslation)[i].dwTime;
				}
				p++;
				Length = *p;
				p++;
				// memset VEC�����������ص�=��ֱ�Ӹ�ֵ
				(pAnimation->pTranslation)[i].Vec = *(D3DXVECTOR3 *)p;
				p += Length;
			}
			break;
			
			// ����
		case 4:
			pAnimation->dwMatrixNum = *p;
			p++;
			pAnimation->pMatrix = new TimeFloatKeysMatrix[pAnimation->dwMatrixNum];
			for(i=0; i<pAnimation->dwMatrixNum; i++)
			{
				// �õ���Animation���˶�ʱ�䣬��������ʱ�䣬����ʱ����Զ�������
				(pAnimation->pMatrix)[i].dwTime = *p;
				if(m_pRootAnimationSet->TimeLength < (pAnimation->pMatrix)[i].dwTime)
				{
					m_pRootAnimationSet->TimeLength = (pAnimation->pMatrix)[i].dwTime;
				}
				p++;
				Length = *p;
				p++;
				// memset���󣬿��������ص�=��ֱ�Ӹ�ֵ
				(pAnimation->pMatrix)[i].Mat = *(D3DXMATRIX *)p;
				p+=Length;
			}
			break;
		}
		pSubData->Unlock();
	}


	// ����FRAME NAME��Referenceģ�壬�����֣������Ժ�ӳ��ANIMATION��BONE�������ﲻӳ�䣬��Ϊ���麯��û���ṩg_pFrame������
	GetObjectType(pSubData, &Guid);
	if(Guid == TID_D3DRMFrame && IsReference && m_pRootAnimationSet && m_pRootAnimationSet->pHeadAnimation)
	{
		// ȷ��FRAME REFERENCE���ϲ���ANIMATION��������������������
		if(pParentData)
		{
			GetObjectType(pParentData, &Guid);
		}
		if(pParentData && Guid == TID_D3DRMAnimation)
		{
			m_pRootAnimationSet->pHeadAnimation->pName = GetObjectName(pSubData);
		}
		// �����ټ����ˣ���������return�ͻ�ݹ鴦��referenceģ�����ģ�壨����Ҫ����������ģ�壩
		return TRUE;
	}

	//�ݹ�
	return(ParseChildObject(pSubData, Depth, Data, IsReference));
}