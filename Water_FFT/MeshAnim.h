#pragma once

class VERTEXSHADER;
struct VERTEXBINDINFO;
typedef VERTEXBINDINFO* LPVERTEXBINDINFO;


// ���������ļ�����CubeMap�ļ����ü���Ӧ��6���洴������Щ����ָ���У�Dimension��ʾÿ��Cube��ķֱ��ʣ�Ĭ��256*256
// HDR SHOP2�²�����ֻ���ֶ�����ˣ�������Щ��Խ��Խ�ң�Format����USE_FP������
// 6��ͼ���ļ��ᱣ�浽��ǰĿ¼���ǵ�poz.hdr��Ҫ��ת180�ȣ����ǰ�Y�ᵹ����
HRESULT g_SplitHDRCrossTexture(LPSTR szCubeFileName, DWORD dwDimension = 256);


/******************************����Mesh���е�����************************/
class KBaseMesh
{
public:
	// ��Ҫ�̳еĻ�����
		// �õ��󶨵���Ϣ
	virtual HRESULT GetBindVertexInfo(LPVERTEXBINDINFO pVertexBindInfo, UINT iVertexNo) = 0;
	virtual HRESULT GetBindVertexBoneMatrix(LPVERTEXBINDINFO pVertexBindInfo, LPD3DXMATRIX pMatBoneCombine) = 0;
	
		// �õ�Mesh�Ļ�����Ϣ
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

	// ����ģ�͵�����״̬����Ҫ�����ź���ת��Ϊ�ձ�ʾû�и��ֱ任
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


	// ����ģ�������ģ�Ϳռ����յ�λ�ƣ�λ����ֻ���������ã�һ��OriginTranslation�����õģ���VECTOR��MATRIX�����ԣ����ص�
	// ����Ǳ��󶨵ĸ�ģ�ͣ�����ʹ�����������ռ��λ�ã�����ǰ��ڱ������ϵ�ģ�ͣ�������ʹ���ֲ�ƫ�ƣ����԰󶨵�Ϊ�ֲ��ռ�ԭ���ƫ����
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



	// ���õ�ǰģ�͵Ĺ�������ͬʱ�趨��ת���֣��ǹ���ģ�;Ͳ��������ˣ����ڼ�����²�ļ̳о���
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


	// ���ϲ�ģ�͵õ�����ת�̳о������ý�����Ϊ�ձ�ʾ���ϲ�ģ�ͣ�ͨ���ϲ�ģ�͵�GetRotationHierarchy���õ�
	void SetRotationHierarchy(LPD3DXMATRIX pMatRotation)
	{
		if(pMatRotation)
			m_MatRotationHierarchy = *pMatRotation;
		else
			D3DXMatrixIdentity(&m_MatRotationHierarchy);
	}


///////////////////////////////////////////Get
	// �õ�ģ�͵ĳ�ʼ״̬������Ҫ�����ź���ת��Ϊ�ձ�ʾ����Get������ֵ�ǽ�������Ϻõľ���
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


	// �õ���ǰģ�͸����²�ģ�͵���ת�̳о��󣬱��밴��Rotation Bone * Origin * HierarchyԤ�ȼ����Combine���󣡣�
	HRESULT GetRotationHierarchy(LPD3DXMATRIX pMatRotation)
	{
		if(!pMatRotation)
			return D3DERR_INVALIDCALL;
		*pMatRotation = m_MatRotationBone * m_MatRotationOrigin * m_MatRotationHierarchy;
		return S_OK;
	}


	// �õ������������յ��������ֱ��d3ddevice/VS->SetTransform���ɣ�����Ԥ�ȼ����Hierarchy��Translation����
	// ����������ǹ�����������ô���������任���ٳ˸þ���
	HRESULT GetWorldTransform(LPD3DXMATRIX pMatWorld)
	{
		if(!pMatWorld)
			return D3DERR_INVALIDCALL;
		*pMatWorld = m_MatTranslationOrigin * m_MatScaling * m_MatRotationOrigin * m_MatRotationHierarchy * m_MatTranslationBind * m_MatTranslationLocal;
		return S_OK;
	}



/////////////////////////////////////Calculate Translation

// ������Щ��������ǰ�������ú����е�Bind������о���Bone Origin TranslationLocal  RotationHierarchy!!!!!!!!!!!11
	// ���㸸ģ�Ͱ󶨵�������ʾ��λ�ã����ڸ�ģ�͵�λ���Ƕ��ģ����Ա�������еľ���ȫӦ���ϣ��õ�����λ��
	D3DXVECTOR3 GetPriPosition(D3DXVECTOR3 pPtPosition)
	{
		D3DXMATRIX Mat;
		D3DXVECTOR3 VecPosition = D3DXVECTOR3(0, 0, 0);
		
		GetWorldTransform(&Mat);
		// �й��������ģ�������й����任
		Mat = m_MatBone * Mat;

		D3DXVec3TransformCoord(&VecPosition, &pPtPosition, &Mat);

		return VecPosition;
	}

	// ������ģ�Ͳο��󶨵��λ�ã�������ģ�͵�λ���ǲο��ģ���������λ���Ǹ�ģ�Ͱ󶨵��λ�ã�������ֻ��Ҫ�ѳ���λ�ƾ���֮������еľ���Ӧ���ϼ���
	D3DXVECTOR3 GetSubPosition(D3DXVECTOR3 pPtPosition)
	{
		// ���������������������TranslationLocal����Ϊ��ֻ��һ�����������Ƶ��͸�ģ�͵��غϺ�Ÿ�һ��Local
		// ������û�м�TransHierarchy����Ϊ�����û�м������������Ҫͨ��TransHierechy���ܰ����Ƶ��͸�ģ���غ���������������û�м�λ�Ƶ�״̬
		D3DXMATRIX Mat = m_MatBone * m_MatTranslationOrigin * m_MatScaling * m_MatRotationOrigin * m_MatRotationHierarchy;
		D3DXVECTOR3 VecPosition = D3DXVECTOR3(0, 0, 0);

		D3DXVec3TransformCoord(&VecPosition, &pPtPosition, &Mat);

		return VecPosition;
	}

	// ��λ�ƾ����Ǽ�������ģ����Ǽ̳еģ�������ֱ��Set����ģ�Ͳ���Calculate������ΪIdentity
	void CalculateTranslationBind(D3DXVECTOR3 VecPriPosition, D3DXVECTOR3 VecSubPosition)
	{
		// ����λ�Ʋ�
		D3DXVECTOR3 Vec = VecPriPosition - VecSubPosition;
		D3DXMatrixTranslation(&m_MatTranslationBind, Vec.x, Vec.y, Vec.z);
	}





public:
	D3DXMATRIX m_MatBone;					// �󶨵��Ӧ�Ĺ���������������ƽ�ƣ�
	D3DXMATRIX m_MatRotationBone;			// �󶨵��Ӧ�����������ת���֣�ȥ��ƽ�������ɣ�����������û�����ŵģ�
	D3DXMATRIX m_MatRotationCombine;		// ���ǰ󶨵��Ӧ��Bone * OriginRotation * HierarchyRotation��Ҳ�����������²�ļ̳���ת����

	// ����ģ�͵��������������5������˳�����γ˵õ�
	D3DXMATRIX m_MatTranslationOrigin;		// ������ģ�Ϳռ��ƫ�ƾ���Ϊ��������ʼλ�ã����ǽ�ģ�ϳ���ƫ�һ�㲻�ã�����������ʱ�������ã���ΪIdentity��ͨ��SetOrigin�����ü��ɣ�����Ҫ�������
	D3DXMATRIX m_MatScaling;				// ��������ž���ģ�͵���������ֻ����һ����ͨ��SetOrigin�����ü��ɣ�����Ҫ�������
	D3DXMATRIX m_MatRotationOrigin;			// ������ģ�Ϳռ�ĳ�ʼ����ֻ���ֵ�ǰ�ֶ����Ƶĳ�����󶨳������ַ���������Բ����������֣�ͨ��SetOrigin�����ü��ɣ�����Ҫ�������

	D3DXMATRIX m_MatRotationHierarchy;		// �̳����ϲ�ģ�͵�������ת���󣬽����ڰ󶨣����ϲ�ģ���ǵõ������ϲ�ģ�Ͷ��²�ļ̳���ת��������ϲ��RotationCombine
	D3DXMATRIX m_MatTranslationBind;		// �����ƶ�ʹ�󶨵��غϵ�λ�ƾ��󣬼�����ӡ���ģ�͵�λ�Ʋ����ӾͿɵõ��þ���
	D3DXMATRIX m_MatTranslationLocal;		// �ֲ�λ�ƾ�������ǰ󶨵ĸ�ģ�ͣ������ھ������������ƫ�ƣ��������ģ�ͣ���ô����ʾ�ֲ�ƫ��
};









/******************************��ͨMesh***********************/
class NORMALMESH : public KBaseMesh
{
public:
	LPD3DXMESH Mesh;
	D3DMATERIAL9* Material;
	LPDIRECT3DTEXTURE9* Texture;
	char m_pTextureFileName[20][100];	// �����ļ�����ÿ��Subset��Ӧһ�����������Subset���࣬С�Ļ�д���������ݣ�
	DWORD SubsetNum, m_dwAttribTableNum;
	LPD3DXATTRIBUTERANGE m_pAttribTable;
	LPDIRECT3DVERTEXDECLARATION9 m_pDeclaration;
	
	NORMALMESH();
	~NORMALMESH();
	void Release();             //�ͷŶ�ȡģ�����ɵ�������Դ��Ҳ�ɵ��˳�ʱ(DESTROY CLASS)�Զ�ִ��
	
	
	HRESULT LoadFromFile(LPSTR pFilename, LPSTR pPathName = NULL, D3DFORMAT Format = D3DFMT_A8R8G8B8);   //��ȡ�ļ����������еı������������еĲ��ʺ�����ʧ�ܷ���MYD3D�Զ������

	//�˺������Զ���ָ���Ӽ��������ļ��ģ���Ϊģ���е�������Ϣ��һ����ʵ��������λ����ͬ������ʱ��Ҫ�ı�ԭ������
	//�����Զ��������Ӽ��������ɺ�SUBSETNUMһ��ʹ�ã���������ѱ������������ֶ�ɾ��ָ���Ӽ��������ٴ����Զ�������
	//���⣬�����ģ�ͱ�������������������Ϣ������������������Ҳû����
	HRESULT SetUserTexture(DWORD SubsetNo, LPSTR UserTextureFilename);
	
	//�Զ�����������в��裬ֻ������͹�Դ��ϣ�����Ҫ�ȶ���Xģ�ͣ��Լ����ʱ����Բ���һ�¾��崦��˳��
	// ��һ��������ʾ�Ƿ�ǿ�����������������ȡʧ�ܣ�����Ϊ���ʾǿ�����ã����Զ����ã���Ϊ�ٱ�ʾ���ԣ���settexture�������Զ���������CUBEMAP
	// ������������������Vertex Declarataion/Shader��������Ǵ��ڣ��ͻὫFVF�ÿգ�����ʹ��FVF
	// �����FVF���Զ����õ�һ��������ϣ�������ǣ���Ҫ�ڻ���ǰ�Լ��趨������ϲ���
	// ����FVF����Shader���������ǰָ��Transform
	HRESULT DrawAll(bool bAutoDisable = true, LPDIRECT3DVERTEXDECLARATION9 pDeclaration = NULL, LPDIRECT3DVERTEXSHADER9 pVS = NULL);

	// �õ��󶨵�����
	HRESULT GetBindVertexInfo(LPVERTEXBINDINFO pVertexBindInfo, UINT iVertexNo);
	// Ϊ�˼̳и����ģ�û��
	HRESULT GetBindVertexBoneMatrix(LPVERTEXBINDINFO pVertexBindInfo, LPD3DXMATRIX pMatBoneCombine) {return D3DERR_INVALIDCALL;}

	// �õ�ģ����Ϣ
	LPD3DXMESH GetMesh()
	{
		return Mesh;
	}
};





/****************************����Mesh********************************/
class SKINMESH : public KBaseMesh
{
public:
	SKINANIMATION SkinAnimation;
	PARSEFRAME ParseFrame;
	// ��Զָ��ǰ��Frame��Mesh����Ҫ���ڶ�MESHCONTAINER��
	D3DXFRAME_EX *m_pFrame;
	D3DXMESHCONTAINER_EX *m_pMesh;

	SKINMESH();
	~SKINMESH()
	{
		Release();
	}
	void Release();

	HRESULT LoadFromX(LPSTR pFileName, char *pTexturePath = ".\\", DWORD NewFVF = 0, DWORD LoadFlags = D3DXMESH_SYSTEMMEM);

	// ���裨�ص���ʼλ�ã�����ʵ����������ʱ�����
	void Reset();
	// �����������ԣ�����Reset��
	void Set(DWORD dwFPS, float fSpeed, BOOL bLoop, BOOL bInverse = FALSE);
	// �õ���ǰ������ʱ�䣨�ӿ�ʼ����ʱ�����𣬼���ǰSKINMESH�Ѿ������˶�ã�
	DWORD GetElapseTime();

	// ���ð󶨵㣬�õ�ָ����������Ӧ�Ĺ�����Ϣ������ģ�Ͱ󶨣�ֻ��Ԥ���㣬����������ľ���
	HRESULT GetBindVertexInfo(LPVERTEXBINDINFO pVertexBindInfo, UINT iVertexNo);
	// ������һ������֮�󣬸��ݵ�ǰSkinMesh�Ĳ���ʵ�������ű����õ�����������ϣ���Translation����
	HRESULT GetBindVertexBoneMatrix(LPVERTEXBINDINFO pVertexBindInfo, LPD3DXMATRIX pMatBoneCombine);
	// �õ�ģ����Ϣ
	LPD3DXMESH GetMesh()
	{
		return m_pMesh->MeshData.pMesh;
	}

	// ���ƣ���ʹ��VS��
	HRESULT Draw(LPSTR pAnimationSetName, UINT iAnimationSetIndex);
	
	// �ָ�ԭʼMESH���ڶ��Ľṹ�У�ʹÿһС����в��������������󣬶��Ҿ���ͬ����Subset���ԣ�VSר��
	// ����������Ϊ�ָ�׼��ʹÿС��MESH���õ��Ĺ�������������ֵ����ֵΪС��30�ķ����������ܹ������޹أ�������ڹ�������Ҳ���Զ�����
	// �����һ������Ϊ�գ���˵���ָ���ǵ�ǰ��MESHCONTAINER����m_pMesh�������ڶ�������Ϊ�Ƿ���0�����30�����򷵻�D3DERR_INVALIDCALL
	// ע������������D3DERR_OUTOFVIDEOMEMORY����˵��ĳЩ�棨��һ���棩����Ҫ�Ĺ�ͷ�����ͳ�����ָ���ľ����������˼Ӵ���������⣬��������
	// ����������E_INVALIDARG��˵��ģ��̫���·ָ��ģ�Ͷ���������65535��������INDEX_16������������޸�һ��Դ�����
	
	// ��ʵ�⣬�����ϵ�һ��ɨ��Ϳ��Խ�80%�������3��5��MESH�У��ڶ���ɨ������Ͽ�����ÿ��СMESH����100��200���棬����˵1W�����MESH���ָ�����Ӧ��˵����20��СMESH����Ⱦ�л���������ʧ���Ժ��Բ���
	// �ò�����ִ��ʱ��;������кܴ�Ĺ�ϵ��������Խ��ɨ���ٶ�Խ��
	HRESULT SplitMeshVS(D3DXMESHCONTAINER_EX *pMesh = NULL, DWORD dwMaxMatrixNum = 16);

	// ���Ƶ�ǰ��MESH��ʹ��VS���������Զ����þ����������Ȩ�ص������Ĵ�������ռ�ó����Ĵ����ĸ�������Ⱦÿ��MESHʱ����ͬ�������ΪSplitMeshָ��������������4��
	// ��Ӧ��VS���������������һ�£�����������ȡ��4�������ٳ���4������˵������Ϊ13����ô�������棬����������16/4 = 4��������4��Ȩ�أ�ÿ������16�ֽ�
	// ��Ӧ��VSԴ�������;�������Ӧ���ж��ٸ����󣬾ͽ��ж��ٴξ���任�͵��ӣ���4������Χ����������˵������Ϊ13����ô��VS�о���任13��16�ζ�������ģ�����Ȩֵ��Ϊ0���任���任����Ӱ�죩
	// ������õ�����4������ֵ��4��������73�ų����Ĵ�����xyz������iStartConstantIndex����C73.w
	HRESULT DrawVS(LPSTR pAnimationSetName, UINT iAnimationSetIndex);

	// ����֮ǰ���ã�����VS������һ����������ʼ�����Ĵ����ţ�������ӸüĴ����ſ�ʼ����ʼΪ0
	// ���һ����������������������Ҫ����任����Ϊ�ռ���
	// ע����ֻ�Ǹı����ڲ��Ĳ�������û��ʵ�ʲ���������SetConstant֮�ࣩ
	HRESULT SetVS(UINT iStartConstantIndex, VERTEXSHADER *pVS, D3DXMATRIX *pMatWorld = NULL);

	
	// ����VS�����ٷָ�ģ����ʹ�õģ�����Ԥ�Ƚ��ָ�õ�ģ�ʹ�Ϊ�ļ�����ʼ��ʱֱ�Ӷ��룬��������ȥ����ʱ��CPU�ָ����
	// ��ֻ����SplitMesh�ṹ�����е����ݣ�������̳й�ϵ����Ϣ��Ȼ��SkinMesh�ļ��У����Ա����ԭSkinMesh�ļ�����ʹ��
	// �÷��ܼ򵥣�	��LoadSkinMeshFromX����SplitMeshVS�����SaveSplitMesh���ɱ���Ϊһ���ļ�
	//				�����ʱ����LoadSkinMeshFromX����LoadSplitMesh����������Ⱦ
	HRESULT SaveSplitMeshToFile(char *szFileName);
	HRESULT LoadSplitMeshFromFile(char *szFileName);
	// �ļ��ṹ��ͷ�� "SPLITMESHVS" + Dword + Dword����ǰ������Dword��ʾ�ָ���ܵ�ģ������ָ�������Matrix��������SplitMeshVS�еĲ�����
	// ������ÿ��SplitMesh����Ϣ��"SPLITMESH" + Dword����ʾ��Dword��SplitMesh
	// ����һЩ���Ķ������ݣ�����Ϊ��dwSubset, Type, dwMatrixNum, dwFaceNum, dwVertexNum, dwVertexSize, dwDataVertexSize, FVF����ӦSplitMesh�ṹ�е����ݣ�
	// ������dwMatrixNum��DWORD����ʾpMatrixMapping����ӳ������
	// ������VB������DWORD��ʾ���ݿ鳤�ȣ�Ȼ���������VB������
	// ������IB������DWORD��ʾ���ݿ鳤�ȣ�Ȼ���������IB������
	// ������DB������DWORD��ʾ���ݿ鳤�ȣ�Ȼ���������DB������
	// ������һ��SplitMeshѭ��
	// �����Desc�������棬��Ϊ��Щ����ֵ�Ƕ���

	
private:
	// ���ݵ�Դ��FRAME��MESH
	D3DXFRAME_EX *m_pRootFrame;
	D3DXMESHCONTAINER_EX *m_pRootMesh;
	// StartTime����������ʼ��Ⱦʱ��ʱ�䣬Reset���Խ�֮���裬LastTime��CurrentTime����������FPS�����������Ƿ���Ⱦ��
	LARGE_INTEGER StartTime, LastTime, CurrentTime;
	// ��ʾģ�͵�֡�����޶�ֵ��ѭ��DRAW��ʱ��������ж��Ƿ���ҪUPDATE HIERARCHY��UPDATE MESH
	float m_fFPS;
	// �����ٶȣ��������ٶȵı���Ϊ׼��Խ��˵��Խ��
	float m_fSpeed;
	// �Ƿ�ѭ���͵���
	BOOL m_bLoop, m_bInverse;

	// ������־��1ΪSKINMESH��ֻ��CPU����2ΪSPLITMESH��������VS��
	UINT m_CreateAttrib;

	// ����VS SKINNING��һЩ����
	VERTEXSHADER *m_pVertexShader;
	UINT m_iStartConstantIndex;
	D3DXMATRIX m_matWorld;

	// ����VS SKINNING�����ó����Ĵ�������ʱ���ε�����SPLITMESH�е�MATRIX����
	HRESULT SetMatrixData(UINT iStartConstantIndex, SplitMesh *pSplitMesh, D3DXMESHCONTAINER_EX *pMesh);
	
};

// ��������֧�ֶ�ANIM MESHContainer



















// ÿ��ģ�Ͷ�Ӧ��һ���󶨵����Ϣ
struct VERTEXBINDINFO
{
	// ����ģ�͵���Ϣ
	BOOL bSkinned;				// �Ƿ��ǹ���ģ��
	KBaseMesh *pMesh;			// ģ��ָ�룬�����ڷ�����ú���

	// ����Ϣ
	DWORD dwVertexNo;			// �󶨶������������е����
	D3DXVECTOR3 PtPosition;		// �󶨶������꣨ԭʼ��

	// ������ڹ���ģ������Ч
	DWORD dwBoneNum;			// Ӱ�����ľ�������Ŀ
	float fWeight[8];			// ÿ�������Ӧ��Ȩ��
	DWORD pMatIndex[8];			// ÿ�������Ӧ�ڹ����е����


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


// �����࣬���ڷ���õ��󶨾�������̣�ÿ������ֻ��Ӧһ�԰󶨣����һ����ģ�ͱ������ģ�Ͱ󶨣���ô��Ū�������
// ������������������ʾ����������ʱ�������ϲ㵽�²����ƽ���ͬ���˳������ν
// һ����ģ�Ϳ��Զ�Ӧ�����ģ�ͣ�����һ����ģ��ֻ�ܶ�Ӧһ����ģ�ͣ�����ģ�Ͷ�Ӧ�����ģ�͵�����£���Щ���и�ģ�͵����������һ���ģ�������
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

	// ��������ģ�ͼ���󶨵���ţ��Լ��Ƿ��ǹ���ģ�͵ı��
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

	// �ڵ�����֮ǰ�������ú�����ģ�͵�Origin��TranslationLocal����Ϊ�����ģ�ͱ���ģ����Լ�ʹ���ظ��İ��࣬ģ�ͱ���Ҳֻ��Ҫ����һ�μ���
	HRESULT GetWorldMatrix(LPD3DXMATRIX pMatFarther, LPD3DXMATRIX pMatChild)
	{
		if(!bSetBind)
			return D3DERR_INVALIDCALL;
		
		// �õ���ģ�͵Ĺ�������
		D3DXMATRIX Mat;
		if(Farther.bSkinned)
		{
			HRESULT hr = Farther.pMesh->GetBindVertexBoneMatrix(&Farther, &Mat);
			if(FAILED(hr))
				return hr;
			Farther.pMesh->SetBindVertexBoneMatrix(&Mat);
		}

		// �õ���ģ�͵Ĺ�������
		if(Child.bSkinned)
		{
			HRESULT hr = Child.pMesh->GetBindVertexBoneMatrix(&Child, &Mat);
			if(FAILED(hr))
				return hr;
			Child.pMesh->SetBindVertexBoneMatrix(&Mat);
		}

		// �õ���ģ�Ͷ���ģ�͵���ת�̳о���
		Farther.pMesh->GetRotationHierarchy(&Mat);
		
		// ������ģ�Ͷ��²����ת�̳о�����һ������Ӱ����ģ�ͣ�Ӱ������²�ģ�ͣ����ò������������ֳ���
		Child.pMesh->SetRotationHierarchy(&Mat);
		
		// �õ���ģ�͵Ķ���λ��
		D3DXVECTOR3 PriPosition(0, 0, 0), SubPosition(0, 0, 0);
		PriPosition = Farther.pMesh->GetPriPosition(Farther.PtPosition);

		// �õ���ģ�͵Ķ���λ��
		SubPosition = Child.pMesh->GetSubPosition(Child.PtPosition);

		// �õ���ģ�͵Ķ���ƫ�ƾ���
		Child.pMesh->CalculateTranslationBind(PriPosition, SubPosition);

		// �õ��������յ��������
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










// ��CubeMap��SkyDome����ʼ��ԭ��ĵ�λ��ģ�ͣ�ͨ��device->SetTransform���ı�����λ�á���С����ת
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

	// �������֣�0��1��2�ֱ������ӡ������Բ�������ǵ�λģ�ͣ������뾶��Ϊ1����ԭ��
	HRESULT Init(UINT iType = 0)
	{
		if(m_bInit)
			return D3DERR_NOTAVAILABLE;
		if(iType > 2)
			return D3DERR_INVALIDCALL;

		Release();

		HRESULT hr = S_OK;

		// ����Mesh������Щ������������Meshֻ������ͷ���
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

		// ����Vertex Delcaration
		D3DVERTEXELEMENT9 Dclr[MAX_FVF_DECL_SIZE];
		D3DXDeclaratorFromFVF(dwFVF, Dclr);
		V_RETURN(d3ddevice->CreateVertexDeclaration(Dclr, &m_pDeclaration));

		// ���VB��ֻ��Ҫ����������꼴��
		D3DXVECTOR3 *pVBData = NULL, PtPosition(0, 0, 0);

		if(FAILED(hr = m_pMesh->LockVertexBuffer(0, (void **)&pVBData)))
			return S_OK;
		
		// ���
		for(UINT i = 0; i < m_pMesh->GetNumVertices(); i++)
		{
			PtPosition = *pVBData;
			pVBData += 2;	// ��������ͷ���
			*pVBData++ = PtPosition;	// д���������꣨���ڶ������꣩
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
			OutputDebugString("�����ʽ��Ч���������ڴ���������Cubemap���ã�\n");
			return E_FAIL;
		}


		// ������Ⱦ״̬
		SetTextureColorMix(0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
		SetTextureColorMix(1, D3DTOP_DISABLE, D3DTA_TEXTURE, D3DTA_DIFFUSE);

		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

		// ���ܻ���Skybox�ڲ�������ǿ��Cull-None
		DWORD dwCullMode = 0;
		d3ddevice->GetRenderState(D3DRS_CULLMODE, &dwCullMode);
		d3ddevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);


		// ������Ⱦ����
		d3ddevice->SetPixelShader(NULL);
		d3ddevice->SetVertexShader(NULL);
		d3ddevice->SetVertexDeclaration(m_pDeclaration);
		d3ddevice->SetFVF(0);

		// ��Ⱦ
		if(FAILED(d3ddevice->BeginScene()))
			return E_FAIL;

		if(FAILED(m_pMesh->DrawSubset(0)))
			return E_FAIL;

		if(FAILED(d3ddevice->EndScene()))
			return E_FAIL;

		// �ָ�Cull-Mode
		d3ddevice->SetRenderState(D3DRS_CULLMODE, dwCullMode);

		return S_OK;
	}

private:
	BOOL m_bInit;
	LPD3DXMESH m_pMesh;
	LPDIRECT3DVERTEXDECLARATION9 m_pDeclaration;




	// �Լ���Skybox����ʵû���ˣ������᲻��ɾ�����밡������
	HRESULT InitMySkyBox()
	{
		if(m_bInit)
			return D3DERR_NOTAVAILABLE;

		Release();

		HRESULT hr = S_OK;

		// ����Mesh
		if(FAILED(hr = D3DXCreateMeshFVF(12, 8, 0, D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0), d3ddevice, &m_pMesh)))
			return E_FAIL;

		// ���VB
		LPD3DXVECTOR3 pVBData = NULL;
		if(FAILED(hr = m_pMesh->LockVertexBuffer(0, (void **)&pVBData)))
			return S_OK;
		// Y-Pos�ϵ��ĸ��㣬���ӵĻ�����˳������Ϊ������������������
		*pVBData++ = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);	// ����������������꣬��Ϊ��CubeMap����������������ͬ
		*pVBData++ = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);

		*pVBData++ = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
		*pVBData++ = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

		*pVBData++ = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
		*pVBData++ = D3DXVECTOR3(1.0f, 1.0f, -1.0f);

		*pVBData++ = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
		*pVBData++ = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);


		// Y-Neg�ϵ��ĸ��㣬���ӵĻ�����˳������Ϊ������������������
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


		// ���IB
		WORD *pIBData = NULL;
		if(FAILED(hr = m_pMesh->LockIndexBuffer(0, (void **)&pIBData)))
			return S_OK;

		// X-Neg�ϵ�����������
		*pIBData++ = 0;	*pIBData++ = 3;	*pIBData++ = 7;
		*pIBData++ = 0;	*pIBData++ = 7;	*pIBData++ = 4;

		// X-Pos�ϵ�����������
		*pIBData++ = 2;	*pIBData++ = 1;	*pIBData++ = 5;
		*pIBData++ = 2;	*pIBData++ = 5;	*pIBData++ = 6;

		// Y-Neg�ϵ�����������
		*pIBData++ = 7;	*pIBData++ = 6;	*pIBData++ = 5;
		*pIBData++ = 7;	*pIBData++ = 5;	*pIBData++ = 4;

		// Y-Pos�ϵ�����������
		*pIBData++ = 0;	*pIBData++ = 1;	*pIBData++ = 2;
		*pIBData++ = 0;	*pIBData++ = 2;	*pIBData++ = 3;

		// Z-Pos�ϵ�����������
		*pIBData++ = 3;	*pIBData++ = 2;	*pIBData++ = 6;
		*pIBData++ = 3;	*pIBData++ = 6;	*pIBData++ = 7;

		// Z-Neg�ϵ�����������
		*pIBData++ = 4;	*pIBData++ = 5;	*pIBData++ = 1;
		*pIBData++ = 4;	*pIBData++ = 1;	*pIBData++ = 0;


		if(FAILED(hr = m_pMesh->UnlockIndexBuffer()))
			return S_OK;

		m_bInit = TRUE;
		return S_OK;
	}
};
