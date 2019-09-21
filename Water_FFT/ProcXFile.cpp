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

	//创建一个空的X文件对象
	if(FAILED(D3DXFileCreate(&pDXFile)))
		return E_FAIL;

	//注册通用模板
	if(FAILED(pDXFile->RegisterTemplates(D3DRM_XTEMPLATES, D3DRM_XTEMPLATE_BYTES)))
		return E_FAIL;

	//创建X文件中的模板集合对象
	if(FAILED(pDXFile->CreateEnumObject(pFileName, D3DXF_FILELOAD_FROMFILE, &pEnumObject)))
	{
		SAFE_RELEASE(pDXFile);
		return E_FAIL;
	}

	//分析第一级的所有子模板，分析的过程中使用ParseChildObject函数递归
	if(FAILED(pEnumObject->GetChildren(&iNum)))
		return E_FAIL;
	for(i=0; i<iNum; i++)
	{
		if(FAILED(pEnumObject->GetChild(i, &pFileData)))
		{
			SAFE_RELEASE(pEnumObject);
			return E_FAIL;
		}

		//没有父FILEDATA，所以置空，第一层，所以Data也置空，Depth置0
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

	//扫描子模板的所有兄弟节点
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
		//分别根据查询结果来处理Instance和Reference
		// 靠，这个鬼地方害死我了，注意最后一个参数，如果该模板是referenced，那么就强迫TRUE，如果不是，就沿用调用该函数的reference状态（晕了）
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












/****************************骨骼动画****************************/
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

	// 设置PARSE属性
	m_TexturePath = TexturePath;
	m_NewFVF      = NewFVF;
	m_LoadFlags   = LoadFlags;
	m_Flags       = ((!ppMesh)?0:1) | ((!ppFrame)?0:2);
	
	// 初始化指针
	m_RootFrame   = NULL;
	m_RootMesh    = NULL;
	
	// 分析文件
	if(FAILED(Parse(Filename)))
		return E_FAIL;
	
	// 存储ppFrameMatrices和pBone矩阵信息，具体见前面D3DXMESHCONTAINER_EX的说明
	if(ppMesh && ppFrame && m_RootMesh && m_RootFrame) {
		
		// 遍历整个MESHCONTAINER
		D3DXMESHCONTAINER_EX *pMesh = m_RootMesh;
		while(pMesh) 
		{			
			// 只是对SKINMESH有效
			if(pMesh->pSkinInfo) 
			{				
				// 分配空间，每个矩阵对应一个BONE
				DWORD NumBones = pMesh->pSkinInfo->GetNumBones();
				pMesh->ppFrameMatrices = new D3DXMATRIX*[NumBones];
				ZeroMemory(pMesh->ppFrameMatrices, sizeof(LPD3DXMATRIX) * NumBones);
				pMesh->pBoneMatrices   = new D3DXMATRIX[NumBones];
				
				// 对每个BONE，在FRAME链表中找到BONE同名的FRAME
				for(DWORD i=0;i<NumBones;i++) 
				{
					const char *BoneName = pMesh->pSkinInfo->GetBoneName(i);
					D3DXFRAME_EX *pFrame = m_RootFrame->Find(BoneName);
					
					// 如果找到，就将ppFrameMatrices的元素指向对应BONE继承后的矩阵，找不到就置空
					if(pFrame)
						pMesh->ppFrameMatrices[i] = &pFrame->matCombined;
					else
						pMesh->ppFrameMatrices[i] = NULL;
				}
			}
			
			pMesh = (D3DXMESHCONTAINER_EX*)pMesh->pNextMeshContainer;
		} 
	}
	
	// 比较恶心的地方，将参数里面的Mesh赋值为m_RootMesh，然后给m_RootMesh置空（为什么要置空啊，有那个必要吗？猪头大师）
	if(ppMesh) 
	{
		*ppMesh = m_RootMesh;
		m_RootMesh = NULL;
	}
	else 
	{
		// 要参数为空的话，把m_RootMesh全清掉（真狠，辛苦Parse了半天的）
		ReleaseMeshContainer(m_RootMesh);
		delete m_RootMesh;
		m_RootMesh = NULL;
	}
	
	// Frame，和上面一样的
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

  // 供新建的MESH使用
  DWORD TempLoadFlags = LoadFlags;
  if(NewFVF)
    TempLoadFlags = D3DXMESH_SYSTEMMEM;

  // 核心
  ID3DXBuffer *MaterialBuffer = NULL, *AdjacencyBuffer = NULL;
  DWORD NumMaterials;
  if(FAILED(hr=D3DXLoadSkinMeshFromXof(pDataObj, TempLoadFlags,
                                       d3ddevice, &AdjacencyBuffer,
                                       &MaterialBuffer, NULL,
                                       &NumMaterials, &pSkin,
                                       &pLoadMesh)))
    return hr;

  // 如果不是SKINMESH就释放掉
  if(pSkin && !pSkin->GetNumBones())
    SAFE_RELEASE(pSkin);

  // 考虑到可能会有新的FVF需求，就新建一个指定FVF的MESH
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

    // 释放掉原来用D3DXLOAD读取的MESH
    SAFE_RELEASE(pLoadMesh);
    pLoadMesh = pTempMesh; pTempMesh = NULL;
  }
 
  // 新建一个MESHCONTAINER_EX，该函数的第一个参数只是个指针，并未指向分配的空间，这里是将该指针指向分配后的结构体
  D3DXMESHCONTAINER_EX *pMesh = new D3DXMESHCONTAINER_EX();
  ZeroMemory(pMesh, sizeof(D3DXMESHCONTAINER_EX));
  *ppMesh = pMesh;

  // 填充该结构体，存储名称、材质、贴图、MESH之类
  DWORD Size;
  pDataObj->GetName(NULL, &Size);
  if(Size) {
    pMesh->Name = new char[Size];
    pDataObj->GetName(pMesh->Name, &Size);
  }
  pMesh->MeshData.Type = D3DXMESHTYPE_MESH;
  pMesh->MeshData.pMesh = pLoadMesh; pLoadMesh = NULL;
  pMesh->pSkinInfo = pSkin; pSkin = NULL;

  // 存储面邻接信息
  DWORD AdjSize = AdjacencyBuffer->GetBufferSize();
  if(AdjSize) {
    pMesh->pAdjacency = (DWORD*)new char[AdjSize];
    memcpy(pMesh->pAdjacency, AdjacencyBuffer->GetBufferPointer(), AdjSize);
  }
  SAFE_RELEASE(AdjacencyBuffer);

  // 建立一个MESH的备份
  if(pMesh->pSkinInfo)
    pMesh->MeshData.pMesh->CloneMeshFVF(D3DXMESH_MANAGED, 
                                        pMesh->MeshData.pMesh->GetFVF(), 
                                        d3ddevice, &pMesh->pSkinMesh);

  // 建立材质和贴图
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

      // 创建贴图
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

  // 优化MESH
  pMesh->MeshData.pMesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT, NULL, NULL, NULL, NULL);

  // 完成
  pMesh = NULL;

  return S_OK;
}




HRESULT PARSEFRAME::PickDataInObject(LPD3DXFILEDATA pSubData, LPD3DXFILEDATA pParentData, DWORD Depth, void **Data, BOOL IsReference)
{
	if(pSubData == NULL)
		return E_FAIL;
	GUID Guid;

	GetObjectType(pSubData, &Guid);	
	// 建立FRAME继承（不填充数据，只是针对非Reference模板）
	if(Guid == TID_D3DRMFrame && IsReference == FALSE && m_Flags & 2)
	{	
		// 建立一个新模板，名称和FileData一样
		D3DXFRAME_EX *pFrame = new D3DXFRAME_EX();
		ZeroMemory(pFrame, sizeof(D3DXFRAME_EX));

		if(pFrame == NULL)
		{
			return E_FAIL;
		}
		pFrame->Name = GetObjectName(pSubData);

		// 最恐怖的地方，根据DATA指针来建立链表，可以画一下流程，不然很难理解的
		// 只有处理顶层模板的时候，DATA才会为NULL，会将所有的顶层模板依次连接为sibling，注意m_rootframe每次都会移动一个，直到移到最后一个顶层模板为止，所以链表头指针等于是反向移动的，但它移动到新的上面之后，原来的那个就被正确的赋值了，而且next是指向新分配的
		// 如果只是单向链表的话，记住这里永远都是原来结构体的兄弟指针指向root，然后root再指向新创建的
		// Data在这两个if之前，总保证的是上一次新建的Frame指针，在这两个if之后，总保证的是本次新建的Frame指针
		// m_RootFrame总保证是最新创建的Frame
		if(Data == NULL) 
		{
			// 将所有的顶层模板连接为sibling，如果m_rootframe未分配（即处理第一个顶层模板时），会自动分配的
			pFrame->pFrameSibling = m_RootFrame;
			m_RootFrame = pFrame; pFrame = NULL;
			Data = (void**)&m_RootFrame;
		}
		else {
			// 子模板，全连为child，注意递归调用该函数的顺序，它是先一口气到最底层的孩子，然后再向上递归
			// 到底之后，回搠一次，Data存放底层孩子的父亲，新的就作为Sibling被连到Data的孩子，见第二句，真是恐怖，大师就是大师，这都能想出来
			D3DXFRAME_EX *pFramePtr = (D3DXFRAME_EX*)*Data;
			pFrame->pFrameSibling = pFramePtr->pFrameFirstChild;
			pFramePtr->pFrameFirstChild = pFrame; pFrame = NULL;
			Data = (void**)&pFramePtr->pFrameFirstChild;
		}
	}
	// 设置Frame矩阵，比较简单，注意上面的if处理完之后，Data就是本次新建的Frame指针了，可以直接用它来填充数据
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
	// 遇到MESH模板，读取SKINMESH
	GetObjectType(pSubData, &Guid);
	if(Guid == TID_D3DRMMesh && m_Flags & 1) 
	{
		// 非Reference Mesh模板
		if(IsReference == FALSE) 
		{
			//mymessage(GetObjectName(pSubData));
			D3DXMESHCONTAINER_EX *pMesh = NULL;
			//通过这个函数，给上面那个空指针分配一个结构体
			LoadSkinMeshInParse(&pMesh, pSubData, m_TexturePath, m_NewFVF, m_LoadFlags);
			
			// 这只是普通链表，不过跟FRAME一样，m_RootMesh不停后移，是反向的
			if(pMesh) 
			{
				pMesh->pNextMeshContainer = m_RootMesh;
				m_RootMesh = pMesh; pMesh = NULL;
				
				// 将MESHCONTAINER连到FRAME上，DATA在这的作用跟FRAME一样，第一个模板不连
				if(Data) 
				{
					D3DXFRAME_EX *pFrame = (D3DXFRAME_EX*)*Data;
					if(m_Flags & 2 && pFrame)
						pFrame->pMeshContainer = m_RootMesh;
				}
			}
		}//end non-reference
		
		// Reference Mesh模板
		else {
				// 参考模板必定是引用实例模板，所以是不用创建MESHCONTAINER的
				if(Data) 
				{
					D3DXFRAME_EX *pFrame = (D3DXFRAME_EX*)*Data;
					if(m_Flags & 2 && m_RootMesh && pFrame) 
					{
						// 找到前面是否有已创建的同名MESHCONTAINER，如果没有，就是个空指针
						char *Name = GetObjectName(pSubData);
						if(Name) 
						{
							// 找到了，连上去
							pFrame->pMeshContainer = m_RootMesh->Find(Name);
							delete [] Name; Name = NULL;
						}
					}
				}
			}//end reference
	}// end type=TID_MESH

	//递归
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
	/**********注掉ReleaseMeshContainer((D3DXMESHCONTAINER_EX *)(p->pMeshContainer));
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
	
	// 对每个骨骼，计算它的pBone矩阵（Offset * Hierarchy）
	for(DWORD i=0;i<pMesh->pSkinInfo->GetNumBones();i++) 
	{
		pMesh->pBoneMatrices[i] = (*pMesh->pSkinInfo->GetBoneOffsetMatrix(i));
		if(pMesh->ppFrameMatrices[i])
			pMesh->pBoneMatrices[i] *= (*pMesh->ppFrameMatrices[i]);
	}
	
	// 开始更新MESH顶点数据，注意需要同时LOCK源MESH和目的MESH，只需要更新VB，不需要更新IB
	void *SrcPtr, *DestPtr;
	pMesh->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&SrcPtr);
	pMesh->pSkinMesh->LockVertexBuffer(0, (void**)&DestPtr);
	
	pMesh->pSkinInfo->UpdateSkinnedMesh(pMesh->pBoneMatrices, NULL, SrcPtr, DestPtr);
	
	pMesh->pSkinMesh->UnlockVertexBuffer();
	pMesh->MeshData.pMesh->UnlockVertexBuffer();
	
	// 完成
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
	// 获得数据在m_RootAnimationSet
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

	// 先遍历所有的动作，找到名字匹配的，如果名字有效，就按名字查找，如果索引有效，就按索引查找（名字优先级高）
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

	// 根据是否循环来计算总时间，如果不循环，就停止在最后的动作点
	if(dwTime >= pAnimationSet->TimeLength)
	{
		dwTime = (bLoop) ? (dwTime%pAnimationSet->TimeLength) : pAnimationSet->TimeLength;
	}
	// 遍历该动作下所有的Animation，计算出最终的插值矩阵并存放到对应的Bone中
	pAnimation = pAnimationSet->pHeadAnimation;
	while(pAnimation)
	{
		// 如果没有和骨骼相连，跳过该Animation
		if(!pAnimation->pBone)
		{
			pAnimation = pAnimation->pNextAnimation;
			continue;
		}

		D3DXMatrixIdentity(&pAnimation->pBone->TransformationMatrix);
		// 根据类型来处理
		switch(pAnimation->dwType)
		{
		case 0:
			// 旋转，判断是否有效
			if(!pAnimation->dwRotationNum || !pAnimation->pRotation)
				break;
			TimeFloatKeysQuaternion *pQKey1, *pQKey2;
			pQKey1 = pQKey2 = pAnimation->pRotation;
		
			// 首先根据时间计算应该选哪两个关键帧
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
			
			// 根据当前时间和第一个关键帧的时间差来计算混合比例系数，注意如果两个关键帧相等（比如time已经到该骨骼动作的尽头），就直接取其中一个的值即可
			// 另外也要注意dwTime若等于0，那么pKey1/2也会等于0，这时的分母……嘿嘿
			if(pQKey2->dwTime == pQKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pQKey1->dwTime) / (float)(pQKey2->dwTime-pQKey1->dwTime);
			// 计算混合结果，并转换为矩阵存储
			D3DXQuaternionSlerp(&QResult, &pQKey1->Quaternion, &pQKey2->Quaternion, fMixScale);
			D3DXMatrixRotationQuaternion(&pAnimation->pBone->TransformationMatrix, &QResult);
			break;
		
		case 1:
			// 缩放，判断是否有效
			if(!pAnimation->dwScalingNum || !pAnimation->pScaling)
				break;
			TimeFloatKeysVector *pSKey1, *pSKey2;
			pSKey1 = pSKey2 = pAnimation->pScaling;
			
			// 首先根据时间计算应该选哪两个关键帧
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
			
			// 根据当前时间和第一个关键帧的时间差来计算混合比例系数，注意如果两个关键帧相等（比如time已经到该骨骼动作的尽头），就直接取其中一个的值即可
			// 另外也要注意dwTime若等于0，那么pKey1/2也会等于0，这时的分母……嘿嘿
			if(pSKey2->dwTime == pSKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pSKey1->dwTime) / (float)(pSKey2->dwTime-pSKey1->dwTime);
			// 计算混合结果，并转换为矩阵存储
			VResult = (pSKey2->Vec - pSKey1->Vec) * fMixScale + pSKey1->Vec;
			D3DXMatrixScaling(&pAnimation->pBone->TransformationMatrix, VResult.x, VResult.y, VResult.z);
			break;

		case 2:
			// 平移，判断是否有效
			if(!pAnimation->dwTranslationNum || !pAnimation->pTranslation)
				break;
			TimeFloatKeysVector *pTKey1, *pTKey2;
			pTKey1 = pTKey2 = pAnimation->pTranslation;
			
			// 首先根据时间计算应该选哪两个关键帧
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
			
			// 根据当前时间和第一个关键帧的时间差来计算混合比例系数，注意如果两个关键帧相等（比如time已经到该骨骼动作的尽头），就直接取其中一个的值即可
			// 另外也要注意dwTime若等于0，那么pKey1/2也会等于0，这时的分母……嘿嘿
			if(pTKey2->dwTime == pTKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pTKey1->dwTime) / (float)(pTKey2->dwTime-pTKey1->dwTime);
			// 计算混合结果，并转换为矩阵存储
			VResult = (pTKey2->Vec - pTKey1->Vec) * fMixScale + pTKey1->Vec;
			D3DXMatrixTranslation(&pAnimation->pBone->TransformationMatrix, VResult.x, VResult.y, VResult.z);
			break;

		case 4:
			// 矩阵，判断是否有效
			if(!pAnimation->dwMatrixNum || !pAnimation->pMatrix)
				break;
			TimeFloatKeysMatrix *pMKey1, *pMKey2;
			pMKey1 = pMKey2 = pAnimation->pMatrix;
			
			// 首先根据时间计算应该选哪两个关键帧
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
			
			// 根据当前时间和第一个关键帧的时间差来计算混合比例系数，注意如果两个关键帧相等（比如time已经到该骨骼动作的尽头），就直接取其中一个的值即可
			// 另外也要注意dwTime若等于0，那么pKey1/2也会等于0，这时的分母……嘿嘿
			if(pMKey2->dwTime == pMKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pMKey1->dwTime) / (float)(pMKey2->dwTime-pMKey1->dwTime);
			// 计算混合结果，并转换为矩阵存储
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

	// 先遍历所有的动作，找到名字匹配的，如果名字有效，就按名字查找，如果索引有效，就按索引查找（名字优先级高）
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

	// 根据是否循环来计算总时间，如果不循环，就停止在最后的动作点
	if(dwTime >= pAnimationSet->TimeLength)
	{
		dwTime = (bLoop) ? (dwTime%pAnimationSet->TimeLength) : pAnimationSet->TimeLength;
	}
	// 遍历该动作下所有的Animation，计算出最终的插值矩阵并存放到对应的Bone中
	pAnimation = pAnimationSet->pHeadAnimation;
	while(pAnimation)
	{
		// 如果没有和骨骼相连，跳过该Animation
		if(!pAnimation->pBone)
		{
			pAnimation = pAnimation->pNextAnimation;
			continue;
		}

		D3DXMatrixIdentity(&matResult);
		D3DXMatrixIdentity(&matOffset);

		// 根据类型来处理
		switch(pAnimation->dwType)
		{
		case 0:
			// 旋转，判断是否有效
			if(!pAnimation->dwRotationNum || !pAnimation->pRotation)
				break;
			TimeFloatKeysQuaternion *pQKey1, *pQKey2;
			pQKey1 = pQKey2 = pAnimation->pRotation;
		
			// 首先根据时间计算应该选哪两个关键帧
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
			
			// 根据当前时间和第一个关键帧的时间差来计算混合比例系数，注意如果两个关键帧相等（比如time已经到该骨骼动作的尽头），就直接取其中一个的值即可
			// 另外也要注意dwTime若等于0，那么pKey1/2也会等于0，这时的分母……嘿嘿
			if(pQKey2->dwTime == pQKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pQKey1->dwTime) / (float)(pQKey2->dwTime-pQKey1->dwTime);
			// 计算混合结果，并转换为矩阵存储
			D3DXQuaternionSlerp(&QResult, &pQKey1->Quaternion, &pQKey2->Quaternion, fMixScale);
			D3DXMatrixRotationQuaternion(&matResult, &QResult);
			break;
		
		case 1:
			// 缩放，判断是否有效
			if(!pAnimation->dwScalingNum || !pAnimation->pScaling)
				break;
			TimeFloatKeysVector *pSKey1, *pSKey2;
			pSKey1 = pSKey2 = pAnimation->pScaling;
			
			// 首先根据时间计算应该选哪两个关键帧
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
			
			// 根据当前时间和第一个关键帧的时间差来计算混合比例系数，注意如果两个关键帧相等（比如time已经到该骨骼动作的尽头），就直接取其中一个的值即可
			// 另外也要注意dwTime若等于0，那么pKey1/2也会等于0，这时的分母……嘿嘿
			if(pSKey2->dwTime == pSKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pSKey1->dwTime) / (float)(pSKey2->dwTime-pSKey1->dwTime);
			// 计算混合结果，并转换为矩阵存储
			VResult = (pSKey2->Vec - pSKey1->Vec) * fMixScale + pSKey1->Vec;
			D3DXMatrixScaling(&matResult, VResult.x, VResult.y, VResult.z);
			break;

		case 2:
			// 平移，判断是否有效
			if(!pAnimation->dwTranslationNum || !pAnimation->pTranslation)
				break;
			TimeFloatKeysVector *pTKey1, *pTKey2;
			pTKey1 = pTKey2 = pAnimation->pTranslation;
			
			// 首先根据时间计算应该选哪两个关键帧
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
			
			// 根据当前时间和第一个关键帧的时间差来计算混合比例系数，注意如果两个关键帧相等（比如time已经到该骨骼动作的尽头），就直接取其中一个的值即可
			// 另外也要注意dwTime若等于0，那么pKey1/2也会等于0，这时的分母……嘿嘿
			if(pTKey2->dwTime == pTKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pTKey1->dwTime) / (float)(pTKey2->dwTime-pTKey1->dwTime);
			// 计算混合结果，并转换为矩阵存储
			VResult = (pTKey2->Vec - pTKey1->Vec) * fMixScale + pTKey1->Vec;
			D3DXMatrixTranslation(&matResult, VResult.x, VResult.y, VResult.z);
			break;

		case 4:
			// 矩阵，判断是否有效
			if(!pAnimation->dwMatrixNum || !pAnimation->pMatrix)
				break;
			TimeFloatKeysMatrix *pMKey1, *pMKey2;
			pMKey1 = pMKey2 = pAnimation->pMatrix;
			
			// 首先根据时间计算应该选哪两个关键帧
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
			
			// 根据当前时间和第一个关键帧的时间差来计算混合比例系数，注意如果两个关键帧相等（比如time已经到该骨骼动作的尽头），就直接取其中一个的值即可
			// 另外也要注意dwTime若等于0，那么pKey1/2也会等于0，这时的分母……嘿嘿
			if(pMKey2->dwTime == pMKey1->dwTime)
				fMixScale = 1;
			else
				fMixScale = (float)(dwTime - pMKey1->dwTime) / (float)(pMKey2->dwTime-pMKey1->dwTime);
			// 计算混合结果，并转换为矩阵存储
			matResult = (pMKey2->Mat - pMKey1->Mat) * fMixScale + pMKey1->Mat;
			break;
		}//end switch
		
		// 已经计算出了新动作的插值矩阵，现在开始混合
		matOffset = matResult - pAnimation->pBone->matOriginal;
		matOffset *= fScale;
		pAnimation->pBone->TransformationMatrix += matOffset;

		// 继续循环
		pAnimation = pAnimation->pNextAnimation;
	}

	return S_OK;
}








HRESULT SKINANIMATION::Map(LPSTR pAnimationSetName, UINT iAnimationSetIndex, D3DXFRAME_EX *pRootFrame)
{
	if(!pRootFrame || !m_pRootAnimationSet || !m_pRootAnimationSet->pHeadAnimation || !m_CreateAttrib)
		return E_FAIL;

	AnimationSet *pAnimationSet = m_pRootAnimationSet;

	// 先遍历所有的动作，找到名字匹配的，如果名字有效，就按名字查找，如果索引有效，就按索引查找（名字优先级高）
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

	// 遍历每个Animation，映射到对应的BONE(D3DXFRAME_EX)
	D3DXFRAME_EX *pFrame = NULL;
	Animation *pAnimation = pAnimationSet->pHeadAnimation;
	for(; pAnimation ; pAnimation=pAnimation->pNextAnimation)
	{
		pFrame = pRootFrame->Find(pAnimation->pName);
		// 就算找不到，也可以置空
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

	// 建立AnimationSet链表
	GetObjectType(pSubData, &Guid);
	if(Guid == TID_D3DRMAnimationSet)
	{	
		// 建立一个新AnimationSet，名称和FileData一样
		AnimationSet *pAnimationSet = new AnimationSet;
		ZeroMemory(pAnimationSet, sizeof(AnimationSet));

		if(pAnimationSet == NULL)
			return E_FAIL;
		pAnimationSet->pName = GetObjectName(pSubData);

		// 建立单向链表，记住这里永远都是原来结构体的Next指针指向root，然后root再指向新创建的
		// 在这两句之后，m_pRootAnimationSet永远指向最新创建的AnimationSet，在后面可以直接用
		// 这里虽然是反向建立，即m_pRootAnimationSet是指向最后一个动作的，但因为动作是用名字来索引的，所以顺序无所谓了
		pAnimationSet->pNextAnimationSet = m_pRootAnimationSet;
		m_pRootAnimationSet = pAnimationSet;
	}

	// 建立当前AnimationSet的Animation链表，pHeadAnimation和m_RootAnimationSet一样的用法（倒着建立）
	GetObjectType(pSubData, &Guid);
	if(Guid == TID_D3DRMAnimation && m_pRootAnimationSet)
	{
		pAnimation = new Animation;
		ZeroMemory(pAnimation, sizeof(Animation));

		pAnimation->pNextAnimation = m_pRootAnimationSet->pHeadAnimation;
		m_pRootAnimationSet->pHeadAnimation = pAnimation;
	}


	// 核心，拷贝每块骨骼的所有AnimationKey数据
	GetObjectType(pSubData, &Guid);
	if(Guid == TID_D3DRMAnimationKey && m_pRootAnimationSet && m_pRootAnimationSet->pHeadAnimation)
	{
		// 得到源数据指针和目的数据指针，之所以用DWORD是因为都是4字节，可以通用dword和float数
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
		// 旋转
		case 0:
			pAnimation->dwRotationNum = *p;
			p++;
			pAnimation->pRotation = new TimeFloatKeysQuaternion[pAnimation->dwRotationNum];
			ZeroMemory(pAnimation->pRotation, sizeof(TimeFloatKeysQuaternion) * pAnimation->dwRotationNum);

			for(i=0; i<pAnimation->dwRotationNum; i++)
			{
				// 得到该Animation的运动时间，并更新总时间，让总时间永远保持最大
				(pAnimation->pRotation)[i].dwTime = *p;
				if(m_pRootAnimationSet->TimeLength < (pAnimation->pRotation)[i].dwTime)
				{
					m_pRootAnimationSet->TimeLength = (pAnimation->pRotation)[i].dwTime;
				}
				p++;
				Length = *p;
				p++;
				// memset QUATERNION，可以用重载的=来直接赋值
				(pAnimation->pRotation)[i].Quaternion = *(D3DXQUATERNION *)p;
				p += Length;
			}
			break;
		
		// 缩放
		case 1:
			pAnimation->dwScalingNum = *p;
			p++;
			pAnimation->pScaling = new TimeFloatKeysVector[pAnimation->dwScalingNum];
			ZeroMemory(pAnimation->pScaling, sizeof(TimeFloatKeysVector) * pAnimation->dwScalingNum);
			
			for(i=0; i<pAnimation->dwScalingNum; i++)
			{
				// 得到该Animation的运动时间，并更新总时间，让总时间永远保持最大
				(pAnimation->pScaling)[i].dwTime = *p;
				if(m_pRootAnimationSet->TimeLength < (pAnimation->pScaling)[i].dwTime)
				{
					m_pRootAnimationSet->TimeLength = (pAnimation->pScaling)[i].dwTime;
				}
				p++;
				Length = *p;
				p++;
				// memset VEC，可以用重载的=来直接赋值
				(pAnimation->pScaling)[i].Vec = *(D3DXVECTOR3 *)p;
				p += Length;
			}
			break;
			
			// 平移
		case 2:
			pAnimation->dwTranslationNum = *p;
			p++;
			pAnimation->pTranslation = new TimeFloatKeysVector[pAnimation->dwTranslationNum];
			for(i=0; i<pAnimation->dwTranslationNum; i++)
			{
				// 得到该Animation的运动时间，并更新总时间，让总时间永远保持最大
				(pAnimation->pTranslation)[i].dwTime = *p;
				if(m_pRootAnimationSet->TimeLength < (pAnimation->pTranslation)[i].dwTime)
				{
					m_pRootAnimationSet->TimeLength = (pAnimation->pTranslation)[i].dwTime;
				}
				p++;
				Length = *p;
				p++;
				// memset VEC，可以用重载的=来直接赋值
				(pAnimation->pTranslation)[i].Vec = *(D3DXVECTOR3 *)p;
				p += Length;
			}
			break;
			
			// 矩阵
		case 4:
			pAnimation->dwMatrixNum = *p;
			p++;
			pAnimation->pMatrix = new TimeFloatKeysMatrix[pAnimation->dwMatrixNum];
			for(i=0; i<pAnimation->dwMatrixNum; i++)
			{
				// 得到该Animation的运动时间，并更新总时间，让总时间永远保持最大
				(pAnimation->pMatrix)[i].dwTime = *p;
				if(m_pRootAnimationSet->TimeLength < (pAnimation->pMatrix)[i].dwTime)
				{
					m_pRootAnimationSet->TimeLength = (pAnimation->pMatrix)[i].dwTime;
				}
				p++;
				Length = *p;
				p++;
				// memset矩阵，可以用重载的=来直接赋值
				(pAnimation->pMatrix)[i].Mat = *(D3DXMATRIX *)p;
				p+=Length;
			}
			break;
		}
		pSubData->Unlock();
	}


	// 遇到FRAME NAME的Reference模板，置名字（用于以后映射ANIMATION到BONE，但这里不映射，因为该虚函数没有提供g_pFrame参数）
	GetObjectType(pSubData, &Guid);
	if(Guid == TID_D3DRMFrame && IsReference && m_pRootAnimationSet && m_pRootAnimationSet->pHeadAnimation)
	{
		// 确保FRAME REFERENCE的上层是ANIMATION，不至于误认无用数据
		if(pParentData)
		{
			GetObjectType(pParentData, &Guid);
		}
		if(pParentData && Guid == TID_D3DRMAnimation)
		{
			m_pRootAnimationSet->pHeadAnimation->pName = GetObjectName(pSubData);
		}
		// 不能再继续了，否则到最后的return就会递归处理reference模板的子模板（不需要处理它的子模板）
		return TRUE;
	}

	//递归
	return(ParseChildObject(pSubData, Depth, Data, IsReference));
}