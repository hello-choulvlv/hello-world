#pragma once

// 分割SKINMESH，VS SKINNING专用
struct SplitMesh
{
	// 本块MESH具有的SUBSET ID，每块都具有相同的ID
	DWORD dwSubset;
	
	// 类型：1 使用排序算法生成的SPLITMESH（即第一种），选择影响顶点的MatrixNum个矩阵（依次向后），把它们影响的所有面都加入到一个MESH（必须是三个顶点都得被这MatrixNum个矩阵所影响）
	// 类型：2 直接遍历所有面，将该面用到的矩阵和后面的面累加（重复的矩阵不算），只要不超出所允许的矩阵最大数，就加入到一个MESH
	// 其实没什么用，只是做个标识 凸-_-凸
	char Type;

	// 本块自身用到的矩阵数（和与4对齐无关）
	// 在类型1中，它就是dwBoneRangeMax-Min，在前几块都是等于dwMaxMatrixNum，在最后一块就取剩下的，比如说35块骨头，最大矩阵数为16，则分为三块，前两块是16，最后一块该值为3
	// 在类型2中，它一定是小于等于最大允许矩阵数的，是真实用到的矩阵数（可以用于VS2.0以后的动态分支）
	// 另外，该值不能太大，因为索引/权重是作为输入流的，而VS的输入寄存器只有16个，所以就决定了该值一定得小于32，其实算上顶点坐标、法线、切线……，应该说该值一般而言最大也只有二十几了
	DWORD dwMatrixNum;
	
	// 本块的矩阵数据，有dwMatrixNum个矩阵，因为每次绘制的时候都要存，所以目前暂时屏蔽了，用以节省存储容量（直接存到常量寄存器就行了，没必要多此一举）
	D3DXMATRIX *pMatrix;
	// 本块的矩阵映射数据，它是索引值，MatrixMapping[i]说明了在dwMatrixNum中第i个矩阵对应到实际骨骼中第几个矩阵
	// 这个东西很好，通过它完全达到了与类型无关的效果 :-)
	DWORD *pMatrixMapping;


	// 参数，用于渲染
	DWORD dwFaceNum, dwVertexNum, dwVertexSize, dwDataVertexSize, FVF;

	// 本块具有的顶点缓冲，跟MESHCONTAINER无关，只是存放本MESH用到的顶点，这样可以将显存占用调到最小
	LPDIRECT3DVERTEXBUFFER9 pVB;

	// 本块具有的索引缓冲，也是跟MESHCONTAINER无关
	LPDIRECT3DINDEXBUFFER9 pIB;
	
	// 本块具有的数据缓冲，用于第二个流，存放每个顶点的权值和索引值，因为每个顶点的权值或索引数目都不相同，所以对每个顶点都统一一下，让该数目取最大值，即跟SplitMeshVS函数中的参数：最大矩阵数相同
	// 不过考虑到对输入寄存器的操作必须是以4为准（4个分量），所以将该矩阵数还要处理一下，即补足4的倍数，比如说15个矩阵，就要补齐16个数据
	// 顶点的顺序和pVB相同
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

//所有的MESH当设备丢失时都必须释放，无论何种POOL

/*
这里要说一下MATRIX，用的过程比较混乱，首先每个FRAME都存放对应骨骼的原始矩阵Original，原有的TransformationMatrix用于存储临时数据Original或Animation（包括BLEND），下来就是继承，Combined永远存储继承结果，并和MESHCONTAINER中的ppFrameMatrix保持一一对应的关系
每个MESHCONTAINER中都有pBoneMatrix，它存放最后的混合结果，即继承*Offset
*/


/***************************新继承的D3DXFRAME和D3DXMESHCONTAINER************************/
struct D3DXFRAME_EX : D3DXFRAME
{
  D3DXMATRIX matCombined;   // 继承后的矩阵，由UpdateHierarchy来刷新，它只是存储继承结果，截至到下一次继承之前将不再改变，即和OFFSET矩阵无关
  D3DXMATRIX matOriginal;   // FrameTransformationMatrix的原始矩阵，创建后就永远不再改变
  
  //这里没有写另一个很重要的矩阵，TransformationMatrix，是D3DXFRAME自带的，它始终作为一个中间值来进行
  //比如说需要把original或动画矩阵继承下去的话，就要用它作为中间矩阵来存储original或动画矩阵
  
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
  
  // 查找，注意g_pFrame这种传进LOADFROMX函数，就变成链表的最后一个了，即最后一层孩子的最后一个兄弟，如果要正序的话，就要先find头部
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

  // 将所有的FrameTransformationMatrix重设为Origin
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


  // 将所有矩阵继承下去，参数是ROOT BONE矩阵（整个MESH的世界矩阵）
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
  // MESHCONTAINER真垃圾，只有纹理文件名，居然没有纹理指针，这里新建一个方便使用
  // 注意纹理是和材质合起来的，同一个MESH会有不同的材质，用数组存储，这里的纹理和材质一样，每个元素表示第几种纹理，对应第几种的材质
  IDirect3DTexture9 **pTextures;
  //临时备份的MESH，专门供SKINMESH使用，每次就渲染它即可
  //要是非SKINMESH（即SKININFO为空），它就是空指针了，千万小心，要提前判断，绘制的时候就用MeshData.pMesh来进行
  ID3DXMesh          *pSkinMesh;

  //ppFrameMatrics是一个指针数组，每个元素分别指向每个Bone的继承矩阵，即每个BONE对应FRAME的matCombined
  //pBone...是存储一个矩阵数组，分别是针对每个Bone的最终变换矩阵，就是*ppFrameMatrics × OffsetMatrix，临时使用，通过UpdateMesh刷新
  D3DXMATRIX        **ppFrameMatrices;
  D3DXMATRIX         *pBoneMatrices;

  // 分割成小块的属性和分割的总块数，只用于VS SKINNING
  // 最大矩阵数，和SplitMesh函数的参数相同，这里注意，VS源程序要和矩阵数对应，比如说有15个矩阵，就只能处理15
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





/***************分析X文件的基础类*******************/
class PARSEXFILE
{
public:
	//分析X文件的主函数，一般在继承类中通过Load函数来调用，它枚举了所有的顶级模板，并分别调用ParseObject来分析每个顶级模板从属的子模板
	//FileFormat是文件格式，TXT或BIN或ZIP
	HRESULT Parse(LPSTR pFileName);
	//分析子模板，只是通过PickDataInObject来进行调用和递归，除了PickDataInObject以外，永远不会有函数直接调用它
	HRESULT ParseChildObject(LPD3DXFILEDATA pFileData, DWORD Depth, void **Data, BOOL ForceReference=FALSE);
	
	//获得指定FileData的信息，helper function，可以不使用
	//得到的指针都是new出来的，需要在没用的时候手动delete
	char *GetObjectName(LPD3DXFILEDATA pFileData);
	HRESULT GetObjectType(LPD3DXFILEDATA pFileData, GUID *p);
	HRESULT GetObjectData(LPD3DXFILEDATA pFileData, SIZE_T dwSize, void *pData);
	
protected:
	//在分析子模板的时候，会调用该函数，一般而言该函数都要重新定义，如果不重新定义，将不会获得任何有用的信息
	//该函数的作用是在递归前加上分析具体的模板ID或NAME的代码，把有用的数据提取到继承类的数据结构中，然后再继续递归
	//Depth是递归到第几个子层了，IsReference是表示该FileData是否为Reference模板
	//ParentData其实未用，主要是利用Data指针，它总是存储上一级的第一个frame，为空时表示无上一级，即subdata是root frame
	//在此函数中总只有一个FileData，所以可以放心的判断ID来分类处理，不会数据混乱
	virtual HRESULT PickDataInObject(LPD3DXFILEDATA pSubData, LPD3DXFILEDATA pParentData, DWORD Depth, void **Data, BOOL IsReference)
	{
		//这就是递归分析的基础，在结尾一定要加上
		return(ParseChildObject(pSubData, Depth, Data, IsReference));
	}
};






/*********************动画数据结构*******************/
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
	char *pName;	// 这块骨骼的运动名称
	D3DXFRAME_EX *pBone;	// 对应的骨骼指针
	Animation *pNextAnimation;	// 下一个骨骼的运动指针

	// 运动类型，0124分别是旋转、缩放、平移和矩阵
	DWORD dwType;
	
	// 分别针对上面四种的数据，旋转、缩放、平移、矩阵动作的个数即具体数据
	// 具体数据分别是根据文件中的顺序来存放的，第N帧数据的就是第N个元素
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
	char *pName;        // 该动作的名称
	DWORD TimeLength;   // 动作总时间长度
	
	AnimationSet *pNextAnimationSet;   // 下一个动作指针
	
	Animation *pHeadAnimation;    // 第一块骨骼的运动指针，是倒序存放的，也就是说Head是最后一块骨骼的，通过NEXT来访问前面的，不过骨骼无所谓次序，根据名字查找映射即可

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
	// 第一种运动的指针，倒序存放的，也就是说ROOT其实是最后一种运动的，通过NEXT来访问前面的，不过运动无所谓次序，根据名字查找映射即可
	AnimationSet *m_pRootAnimationSet;
	
	HRESULT LoadFromX(LPSTR pFileName);
	// 映射指定动作的所有ANIMATION到指定的BONE（D3DXFRAME_EX），之所以要把它独立出来，是因为这样可以将同一套动画数据用于不同的MESHCONTAINER，或将不同的动画数据用于同一个MESHCONTAINER
	// 你甚至可以在绘制的时候动态改变映射关系，从而影响下面的SetAnimation和AnimationBlend
	// 第一个参数和第二个参数取其一即可，如果第一个置空则用第二个，总之名字的优先级高
	HRESULT Map(LPSTR pAnimationSetName, UINT iAnimationSetIndex, D3DXFRAME_EX *pRootFrame);
	
	// 设置每帧的动画（用Animation影响Frame），类中都用真实时间（对应X文件），如果要减速或加速，可以将dwTime在调用的时候乘一个倍数
	// 调用此函数前必须先正确的Map，就是说Map的AnimationSet和该函数的AnimationSet完全保持一致
	// 第一个参数和第二个参数取其一即可，如果第一个置空则用第二个，总之名字的优先级高
	HRESULT SetAnimation(LPSTR pAnimationSetName, UINT iAnimationSetIndex, DWORD dwTime, BOOL bLoop);

	// 动画混合（用Animation影响Frame），其他注意事项都同SetAnimation，包括正确的MAP
	// 甚至可以用不同的SkinAnimation对象来做，第一个对象SetAnimation，第二个对象AnimationBlend，只要分开Map，影响的是同一个MESHCONTAINER
	// fScale是混合比例，动画混合是基于偏移量的，该值越大表示混合的新动作所占比例越大，可以大于1
	HRESULT AnimationBlend(LPSTR pAnimationSetName, UINT iAnimationSetIndex, DWORD dwTime, BOOL bLoop, float fScale);

	void Release();

	SKINANIMATION();
	~SKINANIMATION();

protected:
	HRESULT PickDataInObject(LPD3DXFILEDATA pSubData, LPD3DXFILEDATA pParentData, DWORD Depth, void **Data, BOOL IsReference);

private:
	UINT m_CreateAttrib;
	// 内部使用，用来递归释放链表（已放弃，使用析构来释放）
	void ReleaseAnimationSet(AnimationSet *pAnimationSet);
	void ReleaseAnimation(Animation *pAnimation);
};



/*********************骨骼数据*****************/
// 先说下基本思路，无非就是把数据存储好，然后通过递归遍历来将每块骨骼的两种矩阵存放到单个MeshContainer中（每个MESH都有）
// 第一种是继承下来的（只继承），第二种是继承加上offset，得到这个矩阵后就简单了，直接根据骨骼的影响系数来Update顶点数据即可。
class PARSEFRAME: public PARSEXFILE
{
public:
    // 决定读什么数据，1表示只读MESHCONTAINER，2表示只读FRAME，3表示都读，初始3，同时也会根据LOAD函数前两个参数来改变，如果某个为空，就不读
    // 1 = mesh, 2 = frames, 3= both
    DWORD                 m_Flags;

	PARSEFRAME();
	~PARSEFRAME();

	// 用该函数来建立一个链表，分别以ppMesh和ppFrame为头指针，在这里面同时创建ANIMATION和VERTEXMATRICS MESH
	HRESULT LoadFromX(D3DXMESHCONTAINER_EX **ppMesh, D3DXFRAME_EX **ppFrame, char *Filename, char *TexturePath = ".\\", DWORD NewFVF = 0, DWORD LoadFlags = D3DXMESH_SYSTEMMEM);
	// 只是将指针置空，记住，不会将LOAD时传进来的指针链表删掉，要手动进行
	void Release();
	
	// 更新骨骼继承矩阵，第二个参数就是ROOT BONE的矩阵，简单的用法就是针对整个MESH的世界变换
	void UpdateHierarchy(D3DXFRAME_EX *pFrame, D3DXMATRIX *pMat = NULL)
	{
		if(pFrame) pFrame->UpdateHierarchy(pMat);
	}

	// 根据骨骼矩阵信息（MeshContainer中的pBone和ppFrame）更新MESH的顶点坐标
	HRESULT UpdateMesh(D3DXMESHCONTAINER_EX *pMesh);
	
	// 用来释放递归的MESHCONTAINER_EX和FRAME_EX，（已放弃，使用析构来释放）
	void ReleaseMeshContainer(D3DXMESHCONTAINER_EX *p);
	void ReleaseFrame(D3DXFRAME_EX *p);

protected:
	HRESULT PickDataInObject(LPD3DXFILEDATA pSubData, LPD3DXFILEDATA pParentData, DWORD Depth, void **Data, BOOL IsReference);

private:
	// 贴图文件的新路径
    char                 *m_TexturePath;
	// 新的FVF需求，初始为0，表示不转换（内部自动进行的，不用管）
    DWORD                 m_NewFVF;
	// 标志，D3DXMESH枚举，D3DXLOADSKINMESHFROMXOF和CLONEMESH时候用，初始为D3DXMESH_MANAGED
    DWORD                 m_LoadFlags;
	
    // 这是临时使用的，读取完就没用了，链表会转移到Load中的指针（前两个参数）上面
    D3DXMESHCONTAINER_EX *m_RootMesh;
    D3DXFRAME_EX         *m_RootFrame;	

	UINT				m_CreateAttrib;

	// 内部使用的函数，用来读取TID_D3DRMMESH
	HRESULT LoadSkinMeshInParse(D3DXMESHCONTAINER_EX **ppMesh, ID3DXFileData *pDataObj, char *TexturePath, DWORD NewFVF, DWORD LoadFlags);
};
