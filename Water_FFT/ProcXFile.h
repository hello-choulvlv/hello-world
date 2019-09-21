#pragma once

// �ָ�SKINMESH��VS SKINNINGר��
struct SplitMesh
{
	// ����MESH���е�SUBSET ID��ÿ�鶼������ͬ��ID
	DWORD dwSubset;
	
	// ���ͣ�1 ʹ�������㷨���ɵ�SPLITMESH������һ�֣���ѡ��Ӱ�춥���MatrixNum������������󣩣�������Ӱ��������涼���뵽һ��MESH���������������㶼�ñ���MatrixNum��������Ӱ�죩
	// ���ͣ�2 ֱ�ӱ��������棬�������õ��ľ���ͺ�������ۼӣ��ظ��ľ����㣩��ֻҪ������������ľ�����������ͼ��뵽һ��MESH
	// ��ʵûʲô�ã�ֻ��������ʶ ͹-_-͹
	char Type;

	// ���������õ��ľ�����������4�����޹أ�
	// ������1�У�������dwBoneRangeMax-Min����ǰ���鶼�ǵ���dwMaxMatrixNum�������һ���ȡʣ�µģ�����˵35���ͷ����������Ϊ16�����Ϊ���飬ǰ������16�����һ���ֵΪ3
	// ������2�У���һ����С�ڵ����������������ģ�����ʵ�õ��ľ���������������VS2.0�Ժ�Ķ�̬��֧��
	// ���⣬��ֵ����̫����Ϊ����/Ȩ������Ϊ�������ģ���VS������Ĵ���ֻ��16�������Ծ;����˸�ֵһ����С��32����ʵ���϶������ꡢ���ߡ����ߡ�����Ӧ��˵��ֵһ��������Ҳֻ�ж�ʮ����
	DWORD dwMatrixNum;
	
	// ����ľ������ݣ���dwMatrixNum��������Ϊÿ�λ��Ƶ�ʱ��Ҫ�棬����Ŀǰ��ʱ�����ˣ����Խ�ʡ�洢������ֱ�Ӵ浽�����Ĵ��������ˣ�û��Ҫ���һ�٣�
	D3DXMATRIX *pMatrix;
	// ����ľ���ӳ�����ݣ���������ֵ��MatrixMapping[i]˵������dwMatrixNum�е�i�������Ӧ��ʵ�ʹ����еڼ�������
	// ��������ܺã�ͨ������ȫ�ﵽ���������޹ص�Ч�� :-)
	DWORD *pMatrixMapping;


	// ������������Ⱦ
	DWORD dwFaceNum, dwVertexNum, dwVertexSize, dwDataVertexSize, FVF;

	// ������еĶ��㻺�壬��MESHCONTAINER�޹أ�ֻ�Ǵ�ű�MESH�õ��Ķ��㣬�������Խ��Դ�ռ�õ�����С
	LPDIRECT3DVERTEXBUFFER9 pVB;

	// ������е��������壬Ҳ�Ǹ�MESHCONTAINER�޹�
	LPDIRECT3DINDEXBUFFER9 pIB;
	
	// ������е����ݻ��壬���ڵڶ����������ÿ�������Ȩֵ������ֵ����Ϊÿ�������Ȩֵ��������Ŀ������ͬ�����Զ�ÿ�����㶼ͳһһ�£��ø���Ŀȡ���ֵ������SplitMeshVS�����еĲ���������������ͬ
	// �������ǵ�������Ĵ����Ĳ�����������4Ϊ׼��4�������������Խ��þ�������Ҫ����һ�£�������4�ı���������˵15�����󣬾�Ҫ����16������
	// �����˳���pVB��ͬ
	LPDIRECT3DVERTEXBUFFER9 pDB;

	SplitMesh *pNextMesh;

	SplitMesh()
	{
		dwSubset = 0;
		Type = 0;
		dwMatrixNum = 0;
		dwFaceNum = 0;
		dwVertexNum = 0;
		dwVertexSize = 0;
		dwDataVertexSize = 0;
		FVF = D3DFVF_XYZ;
		pIB = NULL;
		pVB = NULL;
		pDB = NULL;
		pNextMesh = NULL;
		pMatrixMapping = NULL;
		pMatrix = NULL;
	}
	~SplitMesh()
	{
		dwSubset = 0;
		Type = 0;
		dwMatrixNum = 0;
		dwFaceNum = 0;
		dwVertexNum = 0;
		dwVertexSize = 0;
		dwDataVertexSize = 0;
		FVF = D3DFVF_XYZ;
		SAFE_RELEASE(pIB);
		SAFE_RELEASE(pVB);
		SAFE_RELEASE(pDB);
		delete pNextMesh;
		pNextMesh = NULL;
		delete pMatrix;
		pMatrix = NULL;
		delete pMatrixMapping;
		pMatrixMapping = NULL;
	}
};

void ReleaseSplitMesh(SplitMesh *p);

//���е�MESH���豸��ʧʱ�������ͷţ����ۺ���POOL

/*
����Ҫ˵һ��MATRIX���õĹ��̱Ƚϻ��ң�����ÿ��FRAME����Ŷ�Ӧ������ԭʼ����Original��ԭ�е�TransformationMatrix���ڴ洢��ʱ����Original��Animation������BLEND�����������Ǽ̳У�Combined��Զ�洢�̳н��������MESHCONTAINER�е�ppFrameMatrix����һһ��Ӧ�Ĺ�ϵ
ÿ��MESHCONTAINER�ж���pBoneMatrix����������Ļ�Ͻ�������̳�*Offset
*/


/***************************�¼̳е�D3DXFRAME��D3DXMESHCONTAINER************************/
struct D3DXFRAME_EX : D3DXFRAME
{
  D3DXMATRIX matCombined;   // �̳к�ľ�����UpdateHierarchy��ˢ�£���ֻ�Ǵ洢�̳н������������һ�μ̳�֮ǰ�����ٸı䣬����OFFSET�����޹�
  D3DXMATRIX matOriginal;   // FrameTransformationMatrix��ԭʼ���󣬴��������Զ���ٸı�
  
  //����û��д��һ������Ҫ�ľ���TransformationMatrix����D3DXFRAME�Դ��ģ���ʼ����Ϊһ���м�ֵ������
  //����˵��Ҫ��original�򶯻�����̳���ȥ�Ļ�����Ҫ������Ϊ�м�������洢original�򶯻�����
  
  D3DXFRAME_EX()
  {
    Name = NULL;
    pMeshContainer = NULL;
    pFrameSibling = pFrameFirstChild = NULL;
    D3DXMatrixIdentity(&matCombined);
    D3DXMatrixIdentity(&matOriginal);
    D3DXMatrixIdentity(&TransformationMatrix);
  }

  ~D3DXFRAME_EX()
  { 
    delete [] Name;          Name = NULL;
	delete pMeshContainer;	 pMeshContainer = NULL;
	
    delete pFrameSibling;    pFrameSibling = NULL;
    delete pFrameFirstChild; pFrameFirstChild = NULL;
  }
  
  // ���ң�ע��g_pFrame���ִ���LOADFROMX�������ͱ����������һ���ˣ������һ�㺢�ӵ����һ���ֵܣ����Ҫ����Ļ�����Ҫ��findͷ��
  D3DXFRAME_EX *Find(const char *FrameName)
  {
    D3DXFRAME_EX *pFrame, *pFramePtr;

    // Return this frame instance if name matched
    if(Name && FrameName && !strcmp(FrameName, Name))
      return this;

    // Scan siblings
    if((pFramePtr = (D3DXFRAME_EX*)pFrameSibling)) {
      if((pFrame = pFramePtr->Find(FrameName)))
        return pFrame;
    }

    // Scan children
    if((pFramePtr = (D3DXFRAME_EX*)pFrameFirstChild)) {
      if((pFrame = pFramePtr->Find(FrameName)))
        return pFrame;
    }

    return NULL;
  }

  // �����е�FrameTransformationMatrix����ΪOrigin
  void Reset()
  {
    // Copy original matrix
    TransformationMatrix = matOriginal;

    // Reset sibling frames
    D3DXFRAME_EX *pFramePtr;
    if((pFramePtr = (D3DXFRAME_EX*)pFrameSibling))
      pFramePtr->Reset();

    // Reset child frames
    if((pFramePtr = (D3DXFRAME_EX*)pFrameFirstChild))
      pFramePtr->Reset();
  }


  // �����о���̳���ȥ��������ROOT BONE��������MESH���������
  void UpdateHierarchy(D3DXMATRIX *matTransformation = NULL)
  {
    D3DXFRAME_EX *pFramePtr;
    D3DXMATRIX matIdentity;

    // Use an identity matrix if none passed
    if(!matTransformation) {
      D3DXMatrixIdentity(&matIdentity);
      matTransformation = &matIdentity;
    }

    // Combine matrices w/supplied transformation matrix
    matCombined = TransformationMatrix * (*matTransformation);

    // Combine w/sibling frames
    if((pFramePtr = (D3DXFRAME_EX*)pFrameSibling))
      pFramePtr->UpdateHierarchy(matTransformation);

    // Combine w/child frames
    if((pFramePtr = (D3DXFRAME_EX*)pFrameFirstChild))
      pFramePtr->UpdateHierarchy(&matCombined);
  }

  
  void Count(DWORD *Num)
  {
    // Error checking
    if(!Num)
      return;

    // Increase count of frames
    (*Num)+=1;

    // Process sibling frames
    D3DXFRAME_EX *pFrame;
    if((pFrame=(D3DXFRAME_EX*)pFrameSibling))
      pFrame->Count(Num);

    // Process child frames
    if((pFrame=(D3DXFRAME_EX*)pFrameFirstChild))
      pFrame->Count(Num);
  }
};





struct D3DXMESHCONTAINER_EX : D3DXMESHCONTAINER
{
  // MESHCONTAINER��������ֻ�������ļ�������Ȼû������ָ�룬�����½�һ������ʹ��
  // ע�������ǺͲ��ʺ������ģ�ͬһ��MESH���в�ͬ�Ĳ��ʣ�������洢�����������Ͳ���һ����ÿ��Ԫ�ر�ʾ�ڼ���������Ӧ�ڼ��ֵĲ���
  IDirect3DTexture9 **pTextures;
  //��ʱ���ݵ�MESH��ר�Ź�SKINMESHʹ�ã�ÿ�ξ���Ⱦ������
  //Ҫ�Ƿ�SKINMESH����SKININFOΪ�գ��������ǿ�ָ���ˣ�ǧ��С�ģ�Ҫ��ǰ�жϣ����Ƶ�ʱ�����MeshData.pMesh������
  ID3DXMesh          *pSkinMesh;

  //ppFrameMatrics��һ��ָ�����飬ÿ��Ԫ�طֱ�ָ��ÿ��Bone�ļ̳о��󣬼�ÿ��BONE��ӦFRAME��matCombined
  //pBone...�Ǵ洢һ���������飬�ֱ������ÿ��Bone�����ձ任���󣬾���*ppFrameMatrics �� OffsetMatrix����ʱʹ�ã�ͨ��UpdateMeshˢ��
  D3DXMATRIX        **ppFrameMatrices;
  D3DXMATRIX         *pBoneMatrices;

  // �ָ��С������Ժͷָ���ܿ�����ֻ����VS SKINNING
  // ������������SplitMesh�����Ĳ�����ͬ������ע�⣬VSԴ����Ҫ�;�������Ӧ������˵��15�����󣬾�ֻ�ܴ���15
  SplitMesh          *pSplitMesh;
  DWORD               dwSplitMeshNum;
  DWORD               dwMaxMatrixNum;

  D3DXMESHCONTAINER_EX()
  {
    Name               = NULL;
    MeshData.pMesh     = NULL;
    pMaterials         = NULL;
    pEffects           = NULL;
    NumMaterials       = 0;
	dwSplitMeshNum     = 0;
	dwMaxMatrixNum     = 0;
    pAdjacency         = NULL;
    pSkinInfo          = NULL;
    pNextMeshContainer = NULL;
    pTextures          = NULL;
    pSkinMesh          = NULL;
	pSplitMesh         = NULL;
    ppFrameMatrices    = NULL;
    pBoneMatrices      = NULL;
  }

  ~D3DXMESHCONTAINER_EX()
  {
	DWORD  i  = 0;
    if(pTextures && NumMaterials) 
	{
      for(i=0;i<NumMaterials;i++)
        SAFE_RELEASE(pTextures[i]);
    }

    delete [] pTextures;       pTextures = NULL;
    NumMaterials = 0;
	dwSplitMeshNum = 0;
	dwMaxMatrixNum = 0;

    delete [] Name;            Name = NULL;
    delete [] pMaterials;      pMaterials = NULL;
    delete pEffects;           pEffects = NULL;

	delete pSplitMesh;         pSplitMesh = NULL;

    delete [] pAdjacency;      pAdjacency = NULL;
    delete [] ppFrameMatrices; ppFrameMatrices = NULL;
    delete [] pBoneMatrices;   pBoneMatrices = NULL;

    SAFE_RELEASE(MeshData.pMesh);
    SAFE_RELEASE(pSkinInfo);
    SAFE_RELEASE(pSkinMesh);
	
    delete pNextMeshContainer; pNextMeshContainer = NULL;
  }

  D3DXMESHCONTAINER_EX *Find(char *MeshName)
  {
    D3DXMESHCONTAINER_EX *pMesh, *pMeshPtr;

    // Return this mesh instance if name matched
    if(Name && MeshName && !strcmp(MeshName, Name))
      return this;

    // Scan next in list
    if((pMeshPtr = (D3DXMESHCONTAINER_EX*)pNextMeshContainer)) {
      if((pMesh = pMeshPtr->Find(MeshName)))
        return pMesh;
    }

    // Return none found
    return NULL;
  }
};





/***************����X�ļ��Ļ�����*******************/
class PARSEXFILE
{
public:
	//����X�ļ�����������һ���ڼ̳�����ͨ��Load���������ã���ö�������еĶ���ģ�壬���ֱ����ParseObject������ÿ������ģ���������ģ��
	//FileFormat���ļ���ʽ��TXT��BIN��ZIP
	HRESULT Parse(LPSTR pFileName);
	//������ģ�壬ֻ��ͨ��PickDataInObject�����е��ú͵ݹ飬����PickDataInObject���⣬��Զ�����к���ֱ�ӵ�����
	HRESULT ParseChildObject(LPD3DXFILEDATA pFileData, DWORD Depth, void **Data, BOOL ForceReference=FALSE);
	
	//���ָ��FileData����Ϣ��helper function�����Բ�ʹ��
	//�õ���ָ�붼��new�����ģ���Ҫ��û�õ�ʱ���ֶ�delete
	char *GetObjectName(LPD3DXFILEDATA pFileData);
	HRESULT GetObjectType(LPD3DXFILEDATA pFileData, GUID *p);
	HRESULT GetObjectData(LPD3DXFILEDATA pFileData, SIZE_T dwSize, void *pData);
	
protected:
	//�ڷ�����ģ���ʱ�򣬻���øú�����һ����Ըú�����Ҫ���¶��壬��������¶��壬���������κ����õ���Ϣ
	//�ú������������ڵݹ�ǰ���Ϸ��������ģ��ID��NAME�Ĵ��룬�����õ�������ȡ���̳�������ݽṹ�У�Ȼ���ټ����ݹ�
	//Depth�ǵݹ鵽�ڼ����Ӳ��ˣ�IsReference�Ǳ�ʾ��FileData�Ƿ�ΪReferenceģ��
	//ParentData��ʵδ�ã���Ҫ������Dataָ�룬�����Ǵ洢��һ���ĵ�һ��frame��Ϊ��ʱ��ʾ����һ������subdata��root frame
	//�ڴ˺�������ֻ��һ��FileData�����Կ��Է��ĵ��ж�ID�����ദ���������ݻ���
	virtual HRESULT PickDataInObject(LPD3DXFILEDATA pSubData, LPD3DXFILEDATA pParentData, DWORD Depth, void **Data, BOOL IsReference)
	{
		//����ǵݹ�����Ļ������ڽ�βһ��Ҫ����
		return(ParseChildObject(pSubData, Depth, Data, IsReference));
	}
};






/*********************�������ݽṹ*******************/
struct TimeFloatKeysVector
{
	DWORD dwTime;
	D3DXVECTOR3 Vec;
	TimeFloatKeysVector()
	{
		dwTime = 0;
		Vec = D3DXVECTOR3(0,0,0);
	}
};
struct TimeFloatKeysQuaternion
{
	DWORD dwTime;
	D3DXQUATERNION Quaternion;
	TimeFloatKeysQuaternion()
	{
		dwTime = 0;
		Quaternion = D3DXQUATERNION(0,0,0,0);
	}
};
struct TimeFloatKeysMatrix
{
	DWORD dwTime;
	D3DXMATRIX Mat;
	TimeFloatKeysMatrix()
	{
		dwTime = 0;
		D3DXMatrixIdentity(&Mat);
	}
};




struct Animation
{
	char *pName;	// ���������˶�����
	D3DXFRAME_EX *pBone;	// ��Ӧ�Ĺ���ָ��
	Animation *pNextAnimation;	// ��һ���������˶�ָ��

	// �˶����ͣ�0124�ֱ�����ת�����š�ƽ�ƺ;���
	DWORD dwType;
	
	// �ֱ�����������ֵ����ݣ���ת�����š�ƽ�ơ��������ĸ�������������
	// �������ݷֱ��Ǹ����ļ��е�˳������ŵģ���N֡���ݵľ��ǵ�N��Ԫ��
	DWORD dwRotationNum;
	TimeFloatKeysQuaternion *pRotation;

	DWORD dwScalingNum;
	TimeFloatKeysVector *pScaling;

	DWORD dwTranslationNum;
	TimeFloatKeysVector *pTranslation;

	DWORD dwMatrixNum;
	TimeFloatKeysMatrix *pMatrix;

	Animation()
	{
		pName = NULL;
		pBone = NULL;
		pNextAnimation = NULL;
		dwType = INT_MAX;
		dwRotationNum = 0;
		dwScalingNum = 0;
		dwMatrixNum = 0;
		pRotation = NULL;
		pScaling = NULL;
		pTranslation = NULL;
		pMatrix = NULL;
	}
	~Animation()
	{
		delete pName; pName = NULL;
		delete pRotation; pRotation = NULL;
		delete pScaling; pScaling = NULL;
		delete pTranslation; pTranslation = NULL;
		delete pMatrix; pMatrix = NULL;

		delete pNextAnimation; pNextAnimation = NULL;
	}
};


struct AnimationSet
{
	char *pName;        // �ö���������
	DWORD TimeLength;   // ������ʱ�䳤��
	
	AnimationSet *pNextAnimationSet;   // ��һ������ָ��
	
	Animation *pHeadAnimation;    // ��һ��������˶�ָ�룬�ǵ����ŵģ�Ҳ����˵Head�����һ������ģ�ͨ��NEXT������ǰ��ģ�������������ν���򣬸������ֲ���ӳ�伴��

	AnimationSet()
	{
		pName = NULL;
		TimeLength = 0;
		pNextAnimationSet = NULL;
		pHeadAnimation = NULL;
	}
	~AnimationSet()
	{
		delete pName; pName = NULL;
		delete pHeadAnimation; pHeadAnimation = NULL;
		delete pNextAnimationSet; pNextAnimationSet = NULL;
	}
};


class SKINANIMATION: public PARSEXFILE
{
public:
	// ��һ���˶���ָ�룬�����ŵģ�Ҳ����˵ROOT��ʵ�����һ���˶��ģ�ͨ��NEXT������ǰ��ģ������˶�����ν���򣬸������ֲ���ӳ�伴��
	AnimationSet *m_pRootAnimationSet;
	
	HRESULT LoadFromX(LPSTR pFileName);
	// ӳ��ָ������������ANIMATION��ָ����BONE��D3DXFRAME_EX����֮����Ҫ������������������Ϊ�������Խ�ͬһ�׶����������ڲ�ͬ��MESHCONTAINER���򽫲�ͬ�Ķ�����������ͬһ��MESHCONTAINER
	// �����������ڻ��Ƶ�ʱ��̬�ı�ӳ���ϵ���Ӷ�Ӱ�������SetAnimation��AnimationBlend
	// ��һ�������͵ڶ�������ȡ��һ���ɣ������һ���ÿ����õڶ�������֮���ֵ����ȼ���
	HRESULT Map(LPSTR pAnimationSetName, UINT iAnimationSetIndex, D3DXFRAME_EX *pRootFrame);
	
	// ����ÿ֡�Ķ�������AnimationӰ��Frame�������ж�����ʵʱ�䣨��ӦX�ļ��������Ҫ���ٻ���٣����Խ�dwTime�ڵ��õ�ʱ���һ������
	// ���ô˺���ǰ��������ȷ��Map������˵Map��AnimationSet�͸ú�����AnimationSet��ȫ����һ��
	// ��һ�������͵ڶ�������ȡ��һ���ɣ������һ���ÿ����õڶ�������֮���ֵ����ȼ���
	HRESULT SetAnimation(LPSTR pAnimationSetName, UINT iAnimationSetIndex, DWORD dwTime, BOOL bLoop);

	// ������ϣ���AnimationӰ��Frame��������ע�����ͬSetAnimation��������ȷ��MAP
	// ���������ò�ͬ��SkinAnimation������������һ������SetAnimation���ڶ�������AnimationBlend��ֻҪ�ֿ�Map��Ӱ�����ͬһ��MESHCONTAINER
	// fScale�ǻ�ϱ�������������ǻ���ƫ�����ģ���ֵԽ���ʾ��ϵ��¶�����ռ����Խ�󣬿��Դ���1
	HRESULT AnimationBlend(LPSTR pAnimationSetName, UINT iAnimationSetIndex, DWORD dwTime, BOOL bLoop, float fScale);

	void Release();

	SKINANIMATION();
	~SKINANIMATION();

protected:
	HRESULT PickDataInObject(LPD3DXFILEDATA pSubData, LPD3DXFILEDATA pParentData, DWORD Depth, void **Data, BOOL IsReference);

private:
	UINT m_CreateAttrib;
	// �ڲ�ʹ�ã������ݹ��ͷ������ѷ�����ʹ���������ͷţ�
	void ReleaseAnimationSet(AnimationSet *pAnimationSet);
	void ReleaseAnimation(Animation *pAnimation);
};



/*********************��������*****************/
// ��˵�»���˼·���޷Ǿ��ǰ����ݴ洢�ã�Ȼ��ͨ���ݹ��������ÿ����������־����ŵ�����MeshContainer�У�ÿ��MESH���У�
// ��һ���Ǽ̳������ģ�ֻ�̳У����ڶ����Ǽ̳м���offset���õ���������ͼ��ˣ�ֱ�Ӹ��ݹ�����Ӱ��ϵ����Update�������ݼ��ɡ�
class PARSEFRAME: public PARSEXFILE
{
public:
    // ������ʲô���ݣ�1��ʾֻ��MESHCONTAINER��2��ʾֻ��FRAME��3��ʾ��������ʼ3��ͬʱҲ�����LOAD����ǰ�����������ı䣬���ĳ��Ϊ�գ��Ͳ���
    // 1 = mesh, 2 = frames, 3= both
    DWORD                 m_Flags;

	PARSEFRAME();
	~PARSEFRAME();

	// �øú���������һ�������ֱ���ppMesh��ppFrameΪͷָ�룬��������ͬʱ����ANIMATION��VERTEXMATRICS MESH
	HRESULT LoadFromX(D3DXMESHCONTAINER_EX **ppMesh, D3DXFRAME_EX **ppFrame, char *Filename, char *TexturePath = ".\\", DWORD NewFVF = 0, DWORD LoadFlags = D3DXMESH_SYSTEMMEM);
	// ֻ�ǽ�ָ���ÿգ���ס�����ὫLOADʱ��������ָ������ɾ����Ҫ�ֶ�����
	void Release();
	
	// ���¹����̳о��󣬵ڶ�����������ROOT BONE�ľ��󣬼򵥵��÷������������MESH������任
	void UpdateHierarchy(D3DXFRAME_EX *pFrame, D3DXMATRIX *pMat = NULL)
	{
		if(pFrame) pFrame->UpdateHierarchy(pMat);
	}

	// ���ݹ���������Ϣ��MeshContainer�е�pBone��ppFrame������MESH�Ķ�������
	HRESULT UpdateMesh(D3DXMESHCONTAINER_EX *pMesh);
	
	// �����ͷŵݹ��MESHCONTAINER_EX��FRAME_EX�����ѷ�����ʹ���������ͷţ�
	void ReleaseMeshContainer(D3DXMESHCONTAINER_EX *p);
	void ReleaseFrame(D3DXFRAME_EX *p);

protected:
	HRESULT PickDataInObject(LPD3DXFILEDATA pSubData, LPD3DXFILEDATA pParentData, DWORD Depth, void **Data, BOOL IsReference);

private:
	// ��ͼ�ļ�����·��
    char                 *m_TexturePath;
	// �µ�FVF���󣬳�ʼΪ0����ʾ��ת�����ڲ��Զ����еģ����ùܣ�
    DWORD                 m_NewFVF;
	// ��־��D3DXMESHö�٣�D3DXLOADSKINMESHFROMXOF��CLONEMESHʱ���ã���ʼΪD3DXMESH_MANAGED
    DWORD                 m_LoadFlags;
	
    // ������ʱʹ�õģ���ȡ���û���ˣ������ת�Ƶ�Load�е�ָ�루ǰ��������������
    D3DXMESHCONTAINER_EX *m_RootMesh;
    D3DXFRAME_EX         *m_RootFrame;	

	UINT				m_CreateAttrib;

	// �ڲ�ʹ�õĺ�����������ȡTID_D3DRMMESH
	HRESULT LoadSkinMeshInParse(D3DXMESHCONTAINER_EX **ppMesh, ID3DXFileData *pDataObj, char *TexturePath, DWORD NewFVF, DWORD LoadFlags);
};
