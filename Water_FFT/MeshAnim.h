#pragma once

class VERTEXSHADER;
struct VERTEXBINDINFO;
typedef VERTEXBINDINFO* LPVERTEXBINDINFO;


// 根据纹理文件名和CubeMap文件，裁剪对应的6个面创建到这些纹理指针中，Dimension表示每个Cube面的分辨率，默认256*256
// HDR SHOP2下不到，只能手动解决了，哎～这些类越来越乱，Format根据USE_FP宏来设
// 6个图像文件会保存到当前目录，记得poz.hdr，要翻转180度，就是把Y轴倒过来
HRESULT g_SplitHDRCrossTexture(LPSTR szCubeFileName, DWORD dwDimension = 256);


/******************************所有Mesh共有的属性************************/
class KBaseMesh
{
public:
	// 需要继承的基函数
		// 得到绑定点信息
	virtual HRESULT GetBindVertexInfo(LPVERTEXBINDINFO pVertexBindInfo, UINT iVertexNo) = 0;
	virtual HRESULT GetBindVertexBoneMatrix(LPVERTEXBINDINFO pVertexBindInfo, LPD3DXMATRIX pMatBoneCombine) = 0;
	
		// 得到Mesh的基本信息
	virtual LPD3DXMESH GetMesh() = 0;


public:
	KBaseMesh()
	{
		D3DXMatrixIdentity(&m_MatBone);
		D3DXMatrixIdentity(&m_MatRotationBone);
		D3DXMatrixIdentity(&m_MatRotationCombine);

		D3DXMatrixIdentity(&m_MatTranslationOrigin);
		D3DXMatrixIdentity(&m_MatScaling);
		D3DXMatrixIdentity(&m_MatRotationOrigin);

		D3DXMatrixIdentity(&m_MatRotationHierarchy);
		D3DXMatrixIdentity(&m_MatTranslationBind);
		D3DXMatrixIdentity(&m_MatTranslationLocal);
	}

////////////////////////////////////////////Set

	// 设置模型的自身状态，主要是缩放和旋转，为空表示没有该种变换
	void SetOriginTransform(LPD3DXVECTOR3 pVecScaling = NULL, LPD3DXMATRIX pMatRotation = NULL, LPD3DXVECTOR3 pVecTranslation = NULL)
	{
		if(pVecScaling)
			D3DXMatrixScaling(&m_MatScaling, pVecScaling->x, pVecScaling->y, pVecScaling->z);
		else
			D3DXMatrixScaling(&m_MatScaling, 1.0f, 1.0f, 1.0f);

		if(pVecTranslation)
			D3DXMatrixTranslation(&m_MatTranslationOrigin, pVecTranslation->x, pVecTranslation->y, pVecTranslation->z);
		else
			D3DXMatrixTranslation(&m_MatTranslationOrigin, 0.0f, 0.0f, 0.0f);

		if(pMatRotation)
			m_MatRotationOrigin = *pMatRotation;
		else
			D3DXMatrixIdentity(&m_MatRotationOrigin);
	}


	// 设置模型相对于模型空间最终的位移，位移量只在这里设置，一般OriginTranslation都不用的，传VECTOR和MATRIX都可以，重载的
	// 如果是被绑定的根模型，这个就代表它在世界空间的位置，如果是绑定在别人身上的模型，那这个就代表局部偏移，即以绑定点为局部空间原点的偏移量
	void SetTranslationLocal(LPD3DXVECTOR3 pVecTranslation = NULL)
	{
		if(pVecTranslation)
			D3DXMatrixTranslation(&m_MatTranslationLocal, pVecTranslation->x, pVecTranslation->y, pVecTranslation->z);
		else
			D3DXMatrixIdentity(&m_MatTranslationLocal);
	}
	void SetTranslationLocal(LPD3DXMATRIX pMatTranslation = NULL)
	{
		if(pMatTranslation)
			m_MatTranslationLocal = *pMatTranslation;
		else
			D3DXMatrixIdentity(&m_MatTranslationLocal);
	}



	// 设置当前模型的骨骼矩阵，同时设定旋转部分，非骨骼模型就不用设置了，用于计算对下层的继承矩阵
	void SetBindVertexBoneMatrix(LPD3DXMATRIX pMatBone = NULL)
	{
		if(pMatBone)
		{
			m_MatBone = *pMatBone;
			m_MatRotationBone = m_MatBone;
			m_MatRotationBone._41 = m_MatRotationBone._42 = m_MatRotationBone._43 = 0.0f;
		}
		else
		{
			D3DXMatrixIdentity(&m_MatBone);
			D3DXMatrixIdentity(&m_MatRotationBone);
		}
	}


	// 把上层模型得到的旋转继承矩阵设置进来，为空表示无上层模型，通过上层模型的GetRotationHierarchy来得到
	void SetRotationHierarchy(LPD3DXMATRIX pMatRotation)
	{
		if(pMatRotation)
			m_MatRotationHierarchy = *pMatRotation;
		else
			D3DXMatrixIdentity(&m_MatRotationHierarchy);
	}


///////////////////////////////////////////Get
	// 得到模型的初始状态矩阵，主要是缩放和旋转，为空表示跳过Get，返回值是将三个组合好的矩阵
	D3DXMATRIX GetOriginTransform(LPD3DXMATRIX pMatScaling = NULL, LPD3DXMATRIX pMatRotation = NULL, LPD3DXMATRIX pMatTranslation = NULL)
	{
		if(pMatScaling)
			*pMatScaling = m_MatScaling;

		if(pMatTranslation)
			*pMatTranslation = m_MatTranslationOrigin;

		if(pMatRotation)
			*pMatRotation = m_MatRotationOrigin;

		return m_MatScaling * m_MatTranslationOrigin * m_MatRotationOrigin;
	}


	// 得到当前模型给其下层模型的旋转继承矩阵，必须按照Rotation Bone * Origin * Hierarchy预先计算好Combine矩阵！！
	HRESULT GetRotationHierarchy(LPD3DXMATRIX pMatRotation)
	{
		if(!pMatRotation)
			return D3DERR_INVALIDCALL;
		*pMatRotation = m_MatRotationBone * m_MatRotationOrigin * m_MatRotationHierarchy;
		return S_OK;
	}


	// 得到最终最终最终的世界矩阵，直接d3ddevice/VS->SetTransform即可，必须预先计算好Hierarchy和Translation矩阵！
	// 如果自身又是骨骼动画，那么先做骨骼变换，再乘该矩阵
	HRESULT GetWorldTransform(LPD3DXMATRIX pMatWorld)
	{
		if(!pMatWorld)
			return D3DERR_INVALIDCALL;
		*pMatWorld = m_MatTranslationOrigin * m_MatScaling * m_MatRotationOrigin * m_MatRotationHierarchy * m_MatTranslationBind * m_MatTranslationLocal;
		return S_OK;
	}



/////////////////////////////////////Calculate Translation

// 调用这些函数计算前必须设置好所有的Bind点和所有矩阵：Bone Origin TranslationLocal  RotationHierarchy!!!!!!!!!!!11
	// 计算父模型绑定点最终显示的位置，由于父模型的位置是定的，所以必须把所有的矩阵全应用上，得到最终位置
	D3DXVECTOR3 GetPriPosition(D3DXVECTOR3 pPtPosition)
	{
		D3DXMATRIX Mat;
		D3DXVECTOR3 VecPosition = D3DXVECTOR3(0, 0, 0);
		
		GetWorldTransform(&Mat);
		// 有骨骼动画的，必须进行骨骼变换
		Mat = m_MatBone * Mat;

		D3DXVec3TransformCoord(&VecPosition, &pPtPosition, &Mat);

		return VecPosition;
	}

	// 计算子模型参考绑定点的位置，由于子模型的位置是参考的（它的最终位置是父模型绑定点的位置），所以只需要把除了位移矩阵之外的所有的矩阵应用上即可
	D3DXVECTOR3 GetSubPosition(D3DXVECTOR3 pPtPosition)
	{
		// 和上面的区别在与它不加TranslationLocal，因为这只是一个修正，先移到和父模型点重合后才给一个Local
		// 还有它没有加TransHierarchy，因为这个还没有计算出来，就是要通过TransHierechy才能把它移到和父模型重合嘛，所以这里必须是没有加位移的状态
		D3DXMATRIX Mat = m_MatBone * m_MatTranslationOrigin * m_MatScaling * m_MatRotationOrigin * m_MatRotationHierarchy;
		D3DXVECTOR3 VecPosition = D3DXVECTOR3(0, 0, 0);

		D3DXVec3TransformCoord(&VecPosition, &pPtPosition, &Mat);

		return VecPosition;
	}

	// 绑定位移矩阵是计算出来的，不是继承的，更不能直接Set。根模型不能Calculate，必须为Identity
	void CalculateTranslationBind(D3DXVECTOR3 VecPriPosition, D3DXVECTOR3 VecSubPosition)
	{
		// 计算位移差
		D3DXVECTOR3 Vec = VecPriPosition - VecSubPosition;
		D3DXMatrixTranslation(&m_MatTranslationBind, Vec.x, Vec.y, Vec.z);
	}





public:
	D3DXMATRIX m_MatBone;					// 绑定点对应的骨骼矩阵（完整，加平移）
	D3DXMATRIX m_MatRotationBone;			// 绑定点对应骨骼矩阵的旋转部分（去掉平移量即可，骨骼矩阵是没有缩放的）
	D3DXMATRIX m_MatRotationCombine;		// 就是绑定点对应的Bone * OriginRotation * HierarchyRotation，也代表它对其下层的继承旋转矩阵

	// 最终模型的世界矩阵由以下5个矩阵顺序依次乘得到
	D3DXMATRIX m_MatTranslationOrigin;		// 自身在模型空间的偏移矩阵，为了修正初始位置，除非建模上出现偏差，一般不用，所以这里暂时不可设置，总为Identity，通过SetOrigin来设置即可，不需要特殊计算
	D3DXMATRIX m_MatScaling;				// 自身的缩放矩阵，模型的缩放最终只能有一个，通过SetOrigin来设置即可，不需要特殊计算
	D3DXMATRIX m_MatRotationOrigin;			// 自身在模型空间的初始朝向，只表现当前手动控制的朝向，像绑定朝向这种非自身的属性并不在这体现，通过SetOrigin来设置即可，不需要特殊计算

	D3DXMATRIX m_MatRotationHierarchy;		// 继承自上层模型的最终旋转矩阵，仅用于绑定，从上层模型那得到，而上层模型对下层的继承旋转矩阵等于上层的RotationCombine
	D3DXMATRIX m_MatTranslationBind;		// 用于移动使绑定点重合的位移矩阵，计算出子、父模型的位移差后，相加就可得到该矩阵
	D3DXMATRIX m_MatTranslationLocal;		// 局部位移矩阵，如果是绑定的根模型，它用于决定整体的世界偏移，如果是子模型，那么它表示局部偏移
};









/******************************普通Mesh***********************/
class NORMALMESH : public KBaseMesh
{
public:
	LPD3DXMESH Mesh;
	D3DMATERIAL9* Material;
	LPDIRECT3DTEXTURE9* Texture;
	char m_pTextureFileName[20][100];	// 纹理文件名，每个Subset对应一个或多个，如果Subset过多，小心会写坏其他数据！
	DWORD SubsetNum, m_dwAttribTableNum;
	LPD3DXATTRIBUTERANGE m_pAttribTable;
	LPDIRECT3DVERTEXDECLARATION9 m_pDeclaration;
	
	NORMALMESH();
	~NORMALMESH();
	void Release();             //释放读取模型生成的所有资源，也可等退出时(DESTROY CLASS)自动执行
	
	
	HRESULT LoadFromFile(LPSTR pFilename, LPSTR pPathName = NULL, D3DFORMAT Format = D3DFMT_A8R8G8B8);   //读取文件，赋给所有的变量，建立所有的材质和纹理，失败返回MYD3D自定义代码

	//此函数是自定义指定子集的纹理文件的（因为模型中的纹理信息不一定和实际纹理存放位置相同而且有时需要改变原有纹理）
	//它会自动创建该子集的纹理，可和SUBSETNUM一起使用，如果纹理已被创建可以先手动删除指定子集的纹理再创建自定义纹理
	//另外，如果该模型本身无纹理（纹理坐标信息），则此项就算设置了也没有用
	HRESULT SetUserTexture(DWORD SubsetNo, LPSTR UserTextureFilename);
	
	//自动完成以上所有步骤，只将纹理和光源混合，但需要先读入X模型，自己编的时候可以参照一下具体处理顺序
	// 第一个参数表示是否强制设置纹理，若纹理读取失败，参数为真表示强制设置（即自动禁用），为假表示忽略，不settexture，用于自定义纹理如CUBEMAP
	// 后两个参数用于设置Vertex Declarataion/Shader，如果它们存在，就会将FVF置空，否则使用FVF
	// 如果是FVF会自动设置第一层的纹理混合，如果不是，需要在绘制前自己设定纹理层混合参数
	// 无论FVF还是Shader，必须绘制前指定Transform
	HRESULT DrawAll(bool bAutoDisable = true, LPDIRECT3DVERTEXDECLARATION9 pDeclaration = NULL, LPDIRECT3DVERTEXSHADER9 pVS = NULL);

	// 得到绑定点坐标
	HRESULT GetBindVertexInfo(LPVERTEXBINDINFO pVertexBindInfo, UINT iVertexNo);
	// 为了继承给出的，没用
	HRESULT GetBindVertexBoneMatrix(LPVERTEXBINDINFO pVertexBindInfo, LPD3DXMATRIX pMatBoneCombine) {return D3DERR_INVALIDCALL;}

	// 得到模型信息
	LPD3DXMESH GetMesh()
	{
		return Mesh;
	}
};





/****************************骨骼Mesh********************************/
class SKINMESH : public KBaseMesh
{
public:
	SKINANIMATION SkinAnimation;
	PARSEFRAME ParseFrame;
	// 永远指向当前的Frame和Mesh（主要用于多MESHCONTAINER）
	D3DXFRAME_EX *m_pFrame;
	D3DXMESHCONTAINER_EX *m_pMesh;

	SKINMESH();
	~SKINMESH()
	{
		Release();
	}
	void Release();

	HRESULT LoadFromX(LPSTR pFileName, char *pTexturePath = ".\\", DWORD NewFVF = 0, DWORD LoadFlags = D3DXMESH_SYSTEMMEM);

	// 重设（回到初始位置），其实仅仅是重设时间而已
	void Reset();
	// 重新设置属性，不会Reset的
	void Set(DWORD dwFPS, float fSpeed, BOOL bLoop, BOOL bInverse = FALSE);
	// 得到当前持续的时间（从开始画的时候算起，即当前SKINMESH已经动作了多久）
	DWORD GetElapseTime();

	// 设置绑定点，得到指定顶点所对应的骨骼信息，用于模型绑定（只是预计算，不包含具体的矩阵）
	HRESULT GetBindVertexInfo(LPVERTEXBINDINFO pVertexBindInfo, UINT iVertexNo);
	// 调用上一个函数之后，根据当前SkinMesh的播放实况和缩放比例得到骨骼矩阵（组合）和Translation矩阵
	HRESULT GetBindVertexBoneMatrix(LPVERTEXBINDINFO pVertexBindInfo, LPD3DXMATRIX pMatBoneCombine);
	// 得到模型信息
	LPD3DXMESH GetMesh()
	{
		return m_pMesh->MeshData.pMesh;
	}

	// 绘制（不使用VS）
	HRESULT Draw(LPSTR pAnimationSetName, UINT iAnimationSetIndex);
	
	// 分割原始MESH到内定的结构中，使每一小块具有不超过参数个矩阵，而且具有同样的Subset属性，VS专用
	// 最大矩阵数即为分割准则，使每小块MESH所用到的骨骼数不超过该值。该值为小于30的非零数，和总骨骼数无关，就算大于骨骼数，也会自动剪裁
	// 如果第一个函数为空，则说明分割的是当前的MESHCONTAINER（即m_pMesh），若第二个参数为非法（0或大于30），则返回D3DERR_INVALIDCALL
	// 注意若函数返回D3DERR_OUTOFVIDEOMEMORY，则说明某些面（仅一个面）所需要的骨头总数就超过了指定的矩阵数，除了加大矩阵数以外，别无他法
	// 若函数返回E_INVALIDARG，说明模型太大导致分割的模型顶点数大于65535，即超过INDEX_16，这种情况就修改一下源程序吧
	
	// 经实测，基本上第一次扫描就可以将80%的面存入3～5个MESH中，第二次扫描基本上可以让每个小MESH具有100～200个面，所以说1W个面的MESH，分割下来应该说将近20个小MESH，渲染切换的性能损失可以忽略不计
	// 该操作的执行时间和矩阵数有很大的关系，矩阵数越大扫描速度越快
	HRESULT SplitMeshVS(D3DXMESHCONTAINER_EX *pMesh = NULL, DWORD dwMaxMatrixNum = 16);

	// 绘制当前的MESH（使用VS），将会自动设置矩阵和索引、权重到常量寄存器，它占用常量寄存器的个数在渲染每块MESH时都不同，但最多为SplitMesh指定的最大矩阵数的4倍
	// 对应的VS声明必须和数据流一致，即最大矩阵数取被4整除后，再除以4，比如说矩阵数为13，那么声明里面，数据流就是16/4 = 4个索引加4个权重，每个都是16字节
	// 对应的VS源程序必须和矩阵数对应，有多少个矩阵，就进行多少次矩阵变换和叠加（被4整除范围内允许，比如说矩阵数为13，那么在VS中矩阵变换13～16次都是允许的，后面权值都为0，变换不变换都不影响）
	// 另外会用到常数4（索引值×4），它在73号常量寄存器的xyz，另外iStartConstantIndex会在C73.w
	HRESULT DrawVS(LPSTR pAnimationSetName, UINT iAnimationSetIndex);

	// 绘制之前设置（仅限VS），第一个参数是起始常量寄存器号，即矩阵从该寄存器号开始，初始为0
	// 最后一个参数是世界矩阵，如果不需要世界变换就设为空即可
	// 注意它只是改变类内部的参数，并没有实际操作（比如SetConstant之类）
	HRESULT SetVS(UINT iStartConstantIndex, VERTEXSHADER *pVS, D3DXMATRIX *pMatWorld = NULL);

	
	// 用于VS，加速分割模型来使用的，可以预先将分割好的模型存为文件，初始化时直接读入，这样可以去掉耗时的CPU分割操作
	// 它只保存SplitMesh结构链表中的内容，像骨骼继承关系等信息仍然在SkinMesh文件中，所以必须和原SkinMesh文件配套使用
	// 用法很简单：	先LoadSkinMeshFromX，再SplitMeshVS，最后SaveSplitMesh即可保存为一个文件
	//				读入的时候先LoadSkinMeshFromX，再LoadSplitMesh即可正常渲染
	HRESULT SaveSplitMeshToFile(char *szFileName);
	HRESULT LoadSplitMeshFromFile(char *szFileName);
	// 文件结构：头部 "SPLITMESHVS" + Dword + Dword）：前后两个Dword表示分割后总的模型数和指定的最大Matrix数量（即SplitMeshVS中的参数）
	// 下来是每个SplitMesh的信息："SPLITMESH" + Dword：表示第Dword号SplitMesh
	// 紧跟一些单的定长数据，依次为：dwSubset, Type, dwMatrixNum, dwFaceNum, dwVertexNum, dwVertexSize, dwDataVertexSize, FVF（对应SplitMesh结构中的数据）
	// 下来是dwMatrixNum个DWORD，表示pMatrixMapping矩阵映射数据
	// 下来是VB，先是DWORD表示数据块长度，然后就是整个VB的数据
	// 下来是IB，先是DWORD表示数据块长度，然后就是整个IB的数据
	// 下来是DB，先是DWORD表示数据块长度，然后就是整个DB的数据
	// 进行下一个SplitMesh循环
	// 缓冲的Desc并不保存，因为那些属性值是定的

	
private:
	// 备份的源根FRAME和MESH
	D3DXFRAME_EX *m_pRootFrame;
	D3DXMESHCONTAINER_EX *m_pRootMesh;
	// StartTime保存真正开始渲染时的时间，Reset可以将之重设，LastTime和CurrentTime是用来根据FPS来决定本次是否渲染的
	LARGE_INTEGER StartTime, LastTime, CurrentTime;
	// 显示模型的帧数，限定值，循环DRAW的时候会用来判断是否需要UPDATE HIERARCHY和UPDATE MESH
	float m_fFPS;
	// 动画速度，以正常速度的倍数为准，越大说明越快
	float m_fSpeed;
	// 是否循环和倒放
	BOOL m_bLoop, m_bInverse;

	// 创建标志，1为SKINMESH（只能CPU），2为SPLITMESH（可以用VS）
	UINT m_CreateAttrib;

	// 用于VS SKINNING，一些常数
	VERTEXSHADER *m_pVertexShader;
	UINT m_iStartConstantIndex;
	D3DXMATRIX m_matWorld;

	// 用于VS SKINNING，设置常量寄存器，暂时屏蔽掉设置SPLITMESH中的MATRIX数据
	HRESULT SetMatrixData(UINT iStartConstantIndex, SplitMesh *pSplitMesh, D3DXMESHCONTAINER_EX *pMesh);
	
};

// 可以设置支持多ANIM MESHContainer



















// 每个模型对应的一个绑定点的信息
struct VERTEXBINDINFO
{
	// 单个模型的信息
	BOOL bSkinned;				// 是否是骨骼模型
	KBaseMesh *pMesh;			// 模型指针，仅用于方便调用函数

	// 绑定信息
	DWORD dwVertexNo;			// 绑定顶点在数据流中的序号
	D3DXVECTOR3 PtPosition;		// 绑定顶点坐标（原始）

	// 下面仅在骨骼模型中有效
	DWORD dwBoneNum;			// 影响它的矩阵总数目
	float fWeight[8];			// 每个矩阵对应的权重
	DWORD pMatIndex[8];			// 每个矩阵对应在骨骼中的序号


	VERTEXBINDINFO()
	{
		pMesh = NULL;
		dwVertexNo = 0;
		dwBoneNum = 0;
		for(int i = 0; i < 8 ; i++)
		{
			fWeight[i] = 0.0f;
			pMatIndex[i] = 0;
		}
		PtPosition = D3DXVECTOR3(0, 0, 0);
	}
	~VERTEXBINDINFO()
	{
		pMesh = NULL;
		dwVertexNo = 0;
		dwBoneNum = 0;
		for(int i = 0; i < 8 ; i++)
		{
			fWeight[i] = 0.0f;
			pMatIndex[i] = 0;
		}
		PtPosition = D3DXVECTOR3(0, 0, 0);
	}
};


// 辅助类，用于方便得到绑定矩阵的流程，每个对象只对应一对绑定，如果一个父模型被多个子模型绑定，那么就弄多个对象
// 多个对象可以用树来表示，不过做的时候必须从上层到下层逐渐推进，同层间顺序无所谓
// 一个父模型可以对应多个子模型，不过一个子模型只能对应一个父模型！！父模型对应多个子模型的情况下，这些类中父模型的世界矩阵都是一样的，放心啦
class KBindPair
{
public:
	KBindPair()
	{
		bSetBind = FALSE;
	}
	~KBindPair()
	{
		bSetBind = FALSE;
	}

	// 除了两个模型及其绑定点序号，以及是否是骨骼模型的标记
	HRESULT SetBind(KBaseMesh *pPriMesh, UINT iPriVertexNo, BOOL bPriSkinned, KBaseMesh *pSubMesh, UINT iSubVertexNo, BOOL bSubSkinned)
	{
		if(!pPriMesh || !pSubMesh)
			return D3DERR_INVALIDCALL;

		Farther.pMesh = pPriMesh;
		Farther.bSkinned = bPriSkinned;
		HRESULT hr = S_OK;
		if(Farther.bSkinned)
			hr = Farther.pMesh->GetBindVertexInfo(&Farther, iPriVertexNo);
		if(FAILED(hr))
			return hr;



		Child.pMesh = pSubMesh;
		Child.bSkinned = bSubSkinned;
		hr = Child.pMesh->GetBindVertexInfo(&Child, iSubVertexNo);
		if(FAILED(hr))
			return hr;

		bSetBind = TRUE;
		return S_OK;
	}

	// 在调用它之前必须设置好两个模型的Origin、TranslationLocal，因为是针对模型本身的，所以即使有重复的绑定类，模型本身也只需要设置一次即可
	HRESULT GetWorldMatrix(LPD3DXMATRIX pMatFarther, LPD3DXMATRIX pMatChild)
	{
		if(!bSetBind)
			return D3DERR_INVALIDCALL;
		
		// 得到父模型的骨骼矩阵
		D3DXMATRIX Mat;
		if(Farther.bSkinned)
		{
			HRESULT hr = Farther.pMesh->GetBindVertexBoneMatrix(&Farther, &Mat);
			if(FAILED(hr))
				return hr;
			Farther.pMesh->SetBindVertexBoneMatrix(&Mat);
		}

		// 得到子模型的骨骼矩阵
		if(Child.bSkinned)
		{
			HRESULT hr = Child.pMesh->GetBindVertexBoneMatrix(&Child, &Mat);
			if(FAILED(hr))
				return hr;
			Child.pMesh->SetBindVertexBoneMatrix(&Mat);
		}

		// 得到父模型对子模型的旋转继承矩阵
		Farther.pMesh->GetRotationHierarchy(&Mat);
		
		// 设置子模型对下层的旋转继承矩阵，这一步并不影响子模型，影响的是下层模型，作用并不会马上体现出来
		Child.pMesh->SetRotationHierarchy(&Mat);
		
		// 得到父模型的顶点位移
		D3DXVECTOR3 PriPosition(0, 0, 0), SubPosition(0, 0, 0);
		PriPosition = Farther.pMesh->GetPriPosition(Farther.PtPosition);

		// 得到子模型的顶点位移
		SubPosition = Child.pMesh->GetSubPosition(Child.PtPosition);

		// 得到子模型的顶点偏移矩阵
		Child.pMesh->CalculateTranslationBind(PriPosition, SubPosition);

		// 得到父子最终的世界矩阵
		if(pMatFarther)
			Farther.pMesh->GetWorldTransform(pMatFarther);
		if(pMatChild)
			Child.pMesh->GetWorldTransform(pMatChild);

		return S_OK;
	}

private:
	BOOL bSetBind;
	VERTEXBINDINFO Farther, Child;

};










// 贴CubeMap的SkyDome，初始在原点的单位简单模型，通过device->SetTransform来改变它的位置、大小和旋转
class KSkyDome
{
public:
	KSkyDome()
	{
		m_pDeclaration = NULL;
		m_pMesh = NULL;
		m_bInit = FALSE;
	}
	~KSkyDome()
	{
		Release();
	}

	void Release()
	{
		SAFE_RELEASE(m_pMesh);
		SAFE_RELEASE(m_pDeclaration);
		m_bInit = FALSE;
	}

	// 类型三种，0、1、2分别代表盒子、球体和圆柱，都是单位模型，长宽、半径都为1，在原点
	HRESULT Init(UINT iType = 0)
	{
		if(m_bInit)
			return D3DERR_NOTAVAILABLE;
		if(iType > 2)
			return D3DERR_INVALIDCALL;

		Release();

		HRESULT hr = S_OK;

		// 创建Mesh，用这些函数创建出的Mesh只有坐标和法线
		LPD3DXMESH pMesh = NULL;
		switch(iType)
		{
		case 0:
			hr = D3DXCreateBox(d3ddevice, 2.0f, 2.0f, 2.0f, &pMesh, NULL);
			break;
		case 1:
			hr = D3DXCreateSphere(d3ddevice, 1.0f, 50, 50, &pMesh, NULL);
			break;
		case 2:
			hr = D3DXCreateCylinder(d3ddevice, 1.0f, 1.0f, 2.0f, 50, 50, &pMesh, NULL);
			break;
		}

		if(FAILED(hr) || !pMesh)
			return E_FAIL;

		DWORD dwFVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0);
		if(FAILED(hr = pMesh->CloneMeshFVF(D3DXMESH_WRITEONLY, dwFVF, d3ddevice, &m_pMesh)))
			return E_FAIL;
		SAFE_RELEASE(pMesh);

		// 创建Vertex Delcaration
		D3DVERTEXELEMENT9 Dclr[MAX_FVF_DECL_SIZE];
		D3DXDeclaratorFromFVF(dwFVF, Dclr);
		V_RETURN(d3ddevice->CreateVertexDeclaration(Dclr, &m_pDeclaration));

		// 填充VB，只需要填充纹理坐标即可
		D3DXVECTOR3 *pVBData = NULL, PtPosition(0, 0, 0);

		if(FAILED(hr = m_pMesh->LockVertexBuffer(0, (void **)&pVBData)))
			return S_OK;
		
		// 填充
		for(UINT i = 0; i < m_pMesh->GetNumVertices(); i++)
		{
			PtPosition = *pVBData;
			pVBData += 2;	// 跳过顶点和法线
			*pVBData++ = PtPosition;	// 写入纹理坐标（等于顶点坐标）
		}

		if(FAILED(hr = m_pMesh->UnlockVertexBuffer()))
			return S_OK;

		m_bInit = TRUE;
		return S_OK;

	}

	
	HRESULT Draw(LPDIRECT3DCUBETEXTURE9 pCubeMap)
	{
		if(!m_bInit || !m_pMesh)
			return D3DERR_NOTAVAILABLE;
		if(!pCubeMap)
			return D3DERR_INVALIDCALL;

		if(FAILED(d3ddevice->SetTexture(0, pCubeMap)))
		{
			OutputDebugString("纹理格式无效！可能是内存纹理！请检查Cubemap设置！\n");
			return E_FAIL;
		}


		// 设置渲染状态
		SetTextureColorMix(0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
		SetTextureColorMix(1, D3DTOP_DISABLE, D3DTA_TEXTURE, D3DTA_DIFFUSE);

		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

		// 可能会在Skybox内部，所以强制Cull-None
		DWORD dwCullMode = 0;
		d3ddevice->GetRenderState(D3DRS_CULLMODE, &dwCullMode);
		d3ddevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);


		// 设置渲染参数
		d3ddevice->SetPixelShader(NULL);
		d3ddevice->SetVertexShader(NULL);
		d3ddevice->SetVertexDeclaration(m_pDeclaration);
		d3ddevice->SetFVF(0);

		// 渲染
		if(FAILED(d3ddevice->BeginScene()))
			return E_FAIL;

		if(FAILED(m_pMesh->DrawSubset(0)))
			return E_FAIL;

		if(FAILED(d3ddevice->EndScene()))
			return E_FAIL;

		// 恢复Cull-Mode
		d3ddevice->SetRenderState(D3DRS_CULLMODE, dwCullMode);

		return S_OK;
	}

private:
	BOOL m_bInit;
	LPD3DXMESH m_pMesh;
	LPDIRECT3DVERTEXDECLARATION9 m_pDeclaration;




	// 自己的Skybox，其实没用了，但是舍不得删掉代码啊……哭
	HRESULT InitMySkyBox()
	{
		if(m_bInit)
			return D3DERR_NOTAVAILABLE;

		Release();

		HRESULT hr = S_OK;

		// 创建Mesh
		if(FAILED(hr = D3DXCreateMeshFVF(12, 8, 0, D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0), d3ddevice, &m_pMesh)))
			return E_FAIL;

		// 填充VB
		LPD3DXVECTOR3 pVBData = NULL;
		if(FAILED(hr = m_pMesh->LockVertexBuffer(0, (void **)&pVBData)))
			return S_OK;
		// Y-Pos上的四个点，俯视的话，按顺序排列为：左上右上右下左下
		*pVBData++ = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);	// 顶点坐标和纹理坐标，因为是CubeMap，所以两个坐标相同
		*pVBData++ = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);

		*pVBData++ = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
		*pVBData++ = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

		*pVBData++ = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
		*pVBData++ = D3DXVECTOR3(1.0f, 1.0f, -1.0f);

		*pVBData++ = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
		*pVBData++ = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);


		// Y-Neg上的四个点，俯视的话，按顺序排列为：左上右上右下左下
		*pVBData++ = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
		*pVBData++ = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);

		*pVBData++ = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
		*pVBData++ = D3DXVECTOR3(1.0f, -1.0f, 1.0f);

		*pVBData++ = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
		*pVBData++ = D3DXVECTOR3(1.0f, -1.0f, -1.0f);

		*pVBData++ = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
		*pVBData++ = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);

		if(FAILED(hr = m_pMesh->UnlockVertexBuffer()))
			return S_OK;


		// 填充IB
		WORD *pIBData = NULL;
		if(FAILED(hr = m_pMesh->LockIndexBuffer(0, (void **)&pIBData)))
			return S_OK;

		// X-Neg上的两个三角形
		*pIBData++ = 0;	*pIBData++ = 3;	*pIBData++ = 7;
		*pIBData++ = 0;	*pIBData++ = 7;	*pIBData++ = 4;

		// X-Pos上的两个三角形
		*pIBData++ = 2;	*pIBData++ = 1;	*pIBData++ = 5;
		*pIBData++ = 2;	*pIBData++ = 5;	*pIBData++ = 6;

		// Y-Neg上的两个三角形
		*pIBData++ = 7;	*pIBData++ = 6;	*pIBData++ = 5;
		*pIBData++ = 7;	*pIBData++ = 5;	*pIBData++ = 4;

		// Y-Pos上的两个三角形
		*pIBData++ = 0;	*pIBData++ = 1;	*pIBData++ = 2;
		*pIBData++ = 0;	*pIBData++ = 2;	*pIBData++ = 3;

		// Z-Pos上的两个三角形
		*pIBData++ = 3;	*pIBData++ = 2;	*pIBData++ = 6;
		*pIBData++ = 3;	*pIBData++ = 6;	*pIBData++ = 7;

		// Z-Neg上的两个三角形
		*pIBData++ = 4;	*pIBData++ = 5;	*pIBData++ = 1;
		*pIBData++ = 4;	*pIBData++ = 1;	*pIBData++ = 0;


		if(FAILED(hr = m_pMesh->UnlockIndexBuffer()))
			return S_OK;

		m_bInit = TRUE;
		return S_OK;
	}
};
