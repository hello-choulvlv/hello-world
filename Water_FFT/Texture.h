#pragma once

// ������ͼ֮��ĸ��ƣ��Ƚ���
HRESULT D3DXLoadTextureFromTexture(LPDIRECT3DTEXTURE9 pTexDst, LPDIRECT3DTEXTURE9 pTexSrc);
// ������ͼƬ�ļ�������һ��CubeMap���ļ�����������Ϊ��X��������Y��������Z�����������ͼ��
// POOL_MANAGED������Ϊ��ʡ��պ��ӵĴ洢��������
HRESULT CreateCubeMapFromSixFiles(char **ppszFileName, LPDIRECT3DCUBETEXTURE9 *ppCubeTex, UINT iLength, UINT iMip = 0, D3DFORMAT Format = D3DFMT_A8R8G8B8);
// ����ͼ�����ص�ֵlog������֧������2D��ͼ
HRESULT TextureLogOut(char *pszFileName, LPDIRECT3DTEXTURE9 pSrcTex);


//����ɱ���ͻָ���ǰ�������ָ�룬����Ⱦ������ʹ��
//�ȳ�ʼ��������Ⱦ�����������Ⱦ�õ�������
//ע��һ��Ҫ������Ҫ��ͼ������Ŷ
class RENDERTOTEXTURE
{
public:
	RENDERTOTEXTURE();
	~RENDERTOTEXTURE();
	void Release();
	//��ʼ��������û�����ݣ�SIZE�Ƿֱ���
	HRESULT InitTexture(UINT Size);
	//��Ⱦ��Ҫ����VIEW����������PROJ���ӽǻ���ֵ������ķ�Χ��ARG, ����HEADX/Z���ø��ˣ�����0���÷��ο�CUBEMAP����Ⱦ����
	HRESULT RenderToTexture(float Eyex, float Eyey, float Eyez, float Lookx, float Looky, float Lookz, float Heady, float Arg, UINT *Mask);
	//���õ�ǰ����
	HRESULT SetTexture(DWORD Stage);

private:
	//��Ҫ����Ķ���
	LPDIRECT3DSURFACE9 OldDepthBuffer, OldBackBuffer;
	//��Ⱦ��������棬�����µ���Ȼ���
	LPDIRECT3DSURFACE9 DepthBuffer;
	LPDIRECT3DTEXTURE9 RenderTexture;
	UINT CreateAttrib;//������־,1Ϊ�Ѵ�����δ��Ⱦ��2δ����Ⱦ
};





//ʹ����ʾ��1����Ⱦ����������ĺ�����Ҫ�ŵ���ѭ��Render�ĺ��棬��ÿ֡��Ⱦ��������Ϊ������Ӱ��ÿ�������TSS,RS������
//			2��SetTexture���Զ�ʹ�÷���������������任����ӳ��ͼ��ɺ�ҪRestore������Ӱ���������塣
//          3��ÿ����Ⱦ����������ʱҪע�����Mask����Ҫ��ͼ���������Ҫ����
class CUBEMAP
{
public:
	CUBEMAP();
	~CUBEMAP();
	void Release();
	//���ļ�������������
	HRESULT InitCubeMap(LPSTR Filename);
	//����һ������ӳ�����������
	HRESULT InitCubeMap(UINT Size);
	//��Ⱦ���������棬ֻ�����ڻ���ӳ�䣬��Ⱦǰ�Զ����ݣ���Ⱦ���Զ��ָ�
	//Eye��ʾ�ۿ�����������λ�����ģ�Mask�����飬��ModuleNum����������Ҫ���εĶ�Ӧ��ž���Ϊ0,��Ҫӳ��ľ���Ϊ1
	HRESULT RenderToSurface(float Eyex, float Eyey, float Eyez, UINT *Mask);
	//���õ�ǰ����������Stage��
	HRESULT SetTexture(DWORD Stage);
	void RestoreSettings();

private:
	//������ͼ�Ƿ�ɹ������н�������ʲô���͵ģ�1��ʾ������2��ʾ���ļ���0��ʾδ����
	UINT CreateAttrib;
	//���������Ĳ�
	UINT CubeMapStage;
	//��������
	LPDIRECT3DCUBETEXTURE9 CubeMap;
	//����ɱ���
	LPDIRECT3DSURFACE9 OldDepthBuffer, OldBackBuffer;
	//�µ�Z���壬��Ҫ��ʼ��
	LPDIRECT3DSURFACE9 DepthBuffer;
};







//ע��ʹ�����е�SetTexture����ʱ�����Ƚ������͵�ͼSETTEXTURE��������������Լ�ָ���ã������ľͲ��ù���
//����豸��֧�֣��ᵯ������Ի������еĳ�ʼ������������ͻָ����ö���ʧЧ
//���豸��֧�ֵĻ���Ҫ�Լ��޸����������ã�ȥ����͹�㣬��������͵ײ���ΪALPHA��ϣ���Ҫ����Ⱦ��ָ�����ԭ������Ⱦ״̬�������Ӱ��ȫ�ֵ�Ч����
class BUMPMAP
{
public:
	BUMPMAP();
	~BUMPMAP();
	void Release();
	//�õ��豸�Ƿ�֧�ְ�͹ӳ�����Ϣ
	HRESULT GetSupport();
	//���ļ���ȡ��������͹��ͼ��Ϣ��ʼ�����ڶ��������ǰ�͹��Ϣ����
	HRESULT InitFromFile(LPSTR Filename, D3DFORMAT Format);
	//���ð�͹����ָ���㲢���øò��TSS���ԣ��²���뻹��һ�����������������غ����ֱ��Ӧ��������Ϣ�������ȵģ������Formatһ�¡�
	HRESULT SetTexture(DWORD Stage, float M00, float M01, float M10, float M11, float O, float S);
	HRESULT SetTexture(DWORD Stage, float M00, float M01, float M10, float M11);
	//�ָ����������
	void RestoreSettings();

private:
	//������������ã��ڲ�����
	void Save();
	//�����Ϣ�����SETTEXTURE���õĲ���
	LPDIRECT3DTEXTURE9 BumpMap;
	DWORD BumpStage;
	int CreateAttrib; //1Ϊ�Ѵ�������ʼ����-1Ϊ��֧��
	//����3��STAGE��TSS״̬��RESTORESETTINGS��
	DWORD ColorOP_1, ColorArg1_1, ColorArg2_1, TexIndex_1;
	DWORD ColorOP_2, ColorArg1_2, ColorArg2_2, TexIndex_2;
	DWORD ColorOP_3, ColorArg1_3, ColorArg2_3, TexIndex_3;
};










//ע��ʹ�����е�SetTexture����ʱ�����Ƚ������͵�ͼSETTEXTURE��������������Լ�ָ���ã������ľͲ��ù���
class NORMALMAP
{
public:
	NORMALMAP();
	~NORMALMAP();
	void Release();
	//���ļ���ȡ����������ͼ��Ϣ��ʼ����Scale�Ƿ������ı�����DX����D3DX�Զ����ɷ���ͼ��
	HRESULT InitFromFile(LPSTR Filename, double Scale);
	HRESULT InitFromFileDX(LPSTR Filename, float Scale);
	//�Լ���䷨�����ݣ���Ҫ�Լ��޸Ĵ���
	HRESULT InitFromUser(UINT size);
	//���߹�˷�������ݣ�Power�Ƕ��ٴη�
	HRESULT InitPowerMap(UINT size, double Power);
	//���������Թ���ͼ����
	HRESULT InitAnisotropyMap(UINT size);
	//���������Է���ͼ���ݣ�Power��ȫ�ֹ���ǿ�ȣ�0��1֮�䣬Խ���ʾǿ��Խ�ߣ�����Խ���У���֮��ʾǿ��ԽС������ɢ��ΧԽ��
	HRESULT InitAnisotropyDirMap(UINT size, double Power);

	//���÷�����ͼ��ָ���㲢���øò��TSS����
	//���������ǹ��շ�������������ע������Ǵ�����ָ���Դ�ķ�������Ŷ
	HRESULT SetTexture(DWORD Stage, float Lx, float Ly, float Lz);
	//�ָ���������ã�������SetTexture����
	void RestoreSettings();

	//�����Ƿֱ����÷�����ͼ���߹�˷�ͼ�͸������Թ���ͼ�ģ�������PS�������������ı�TSS���ã�ֻ�Ǽ򵥵�SETTEXTURE����
	HRESULT SetNormalMap(DWORD Stage);
	//ע��������POWERͼ�������ֶ����ò������ύ��ΪCLAMP
	HRESULT SetPowerMap(DWORD Stage);
	HRESULT SetAnisotropyMap(DWORD Stage);
	HRESULT SetAnisotropyDirMap(DWORD Stage);
	

private:
	//�����Ϣ�����SETTEXTURE���õĲ���
	LPDIRECT3DTEXTURE9 NormalMap;
	//��Ÿ߹�˷����ͼ��������PS
	LPDIRECT3DTEXTURE9 PowerMap;
	//��Ÿ������Թ��պͷ���ͼ��������PS
	LPDIRECT3DTEXTURE9 AnisotropyMap;
	LPDIRECT3DTEXTURE9 AnisotropyDirMap;
	UINT CreateNormalAttrib; //1Ϊ�Ѵ�������ʼ��
	UINT CreatePowerAttrib; //1Ϊ�Ѵ�������ʼ��
	UINT CreateAnisotropyAttrib; //1Ϊ�Ѵ�������ʼ��
	UINT CreateAnisotropyDirAttrib; //1Ϊ�Ѵ�������ʼ��
	//����STAGE��TSS״̬��RESTORESETTINGS��
	DWORD NormalStage;
	DWORD ColorOP, ColorArg1, ColorArg2;
};








//��������������ع��յģ���ʼ����CUBEMAPר���������й�һ���������������������Ϊ��Ҫת����N��L������Ȼ�������������ɫֵ���Ѿ��ǹ�һ�������ˣ�����DOT3����
//��Сѡ256�ȽϺ��ʣ�512֧���Կ���û256�࣬����MIPMAP�����CUBEMAP���ǽ�ֹ�ģ���Ч����һ�㣩����������Ҫ�ľ����������һ��Ҫ��ΪPOINT��LINEAR��ֵ��ʱ���Ӱ���������ȣ�Ч������
class NORMALCUBEMAP
{
public:
	NORMALCUBEMAP();
	~NORMALCUBEMAP();
	void Release();
	//����һ����ѯ��һ����������������
	HRESULT InitCubeMap(UINT Size);
	//����һ����ѯ˥��ϵ������ͨ����
	HRESULT InitAttenMap(UINT Size);
	//���õ�ǰ����������Stage��
	HRESULT SetCubeTexture(DWORD Stage);
	//���õ�ǰ˥��������Stage��
	HRESULT SetAttenTexture(DWORD Stage);

private:
	//������ͼ�Ƿ�ɹ������н�������ʲô���͵ģ�1��ʾ������2��ʾ���ļ���0��ʾδ����
	UINT CreateAttrib, CreateAttribAtten;
	//��������
	LPDIRECT3DCUBETEXTURE9 CubeMap;
	//˥������
	LPDIRECT3DTEXTURE9 AttenMap;
};





// �����ֻ�Ǽ򻯲������ˣ�����ǣ����Shader
class KShadowMap
{
public:
	KShadowMap();
	~KShadowMap();

	// ��ʼ��ת����ͼ��������Ӱ��ͼ
	HRESULT Init(UINT iSize = 512);
	void Release();

	// ������Ӱ��ͼΪRT��׼��������Ⱦ��MRT = 1����������ת���������ڲ�
	HRESULT RenderShadowTexture(DWORD dwStage);
	// ��Ⱦ����Ӱ��ͼ֮��һ��Ҫ�ָ��ɵ�RT����Ȼ���
	HRESULT RestoreRenderTarget();
	// ������Ⱦ�õ���Ӱ��ͼ��׼����Ⱦ������Ӱ��ע�⿼�ǵ�ת������������Ҫռ������ͼ
	HRESULT SetShadowTexture(DWORD dwStage);

private:
	// ����ת����ͼ�Ƿ�ɹ��������Ƿ���Ⱦ����Ӱ��ͼ��1��ʾδ��Ⱦ����Ӱ��ͼ��2��ʾ���ɹ�
	UINT m_iCreateAttrib;
	// �¾���Ȼ���ͺ�̨���壬����״̬����ʱʹ��
	LPDIRECT3DSURFACE9 m_pOldBackBuffer, m_pOldDepthBuffer, m_pDepthBuffer;

	// ת��������ȵ�������ɫ
	LPDIRECT3DTEXTURE9 m_pTexConvertor;
	// ��Ӱ��ͼ
	LPDIRECT3DTEXTURE9 m_pTexShadowMap;
	LPDIRECT3DSURFACE9 m_pSurfShadowMap;
};



















// �����ֻ�Ǽ򻯲������ˣ�����ǣ����Shader��ע��ֻ�����ڵ��Դ
// �÷���SaveRenderTarget();  for(i = 0; i < 6; i++){ RenderShadowTexture(); �Զ����RenderToShadowMap(); } RestoreRenderTarget();  SetShadowTexture(); �Զ����RenderShadow();
class KOmniShadowMap
{
public:
	KOmniShadowMap();
	~KOmniShadowMap();

	// ��ʼ��ת����ͼ��������Ӱ��ͼ
	HRESULT Init(UINT iSize = 512);
	void Release();

	// ������Ӱ��ͼΪRT��׼��������Ⱦ������MRT����������֮�������ͬ������Ⱦ��������������Ⱥ�Ҫ����6�Σ���ʾ��ͬ����
	HRESULT RenderShadowTexture(UINT iFaceNo, D3DXVECTOR3 PtPointLight);
	// ��Ⱦ��Ӱ��ͼǰһ��Ҫ����RT����Ȼ��壬��Ⱦ����Ӱ��ͼ֮��һ��Ҫ�ָ��ɵ�RT����Ȼ���
	HRESULT SaveRenderTarget();
	HRESULT RestoreRenderTarget();
	// ������Ⱦ�õ���Ӱ��ͼ��׼����Ⱦ������Ӱ��ע�⿼�ǵ�����������ͼ������һ��Ҫռ����
	HRESULT SetShadowTexture(DWORD dwStage);
	// �õ�View���󣬱�����RenderShadowTexture֮�����
	D3DXMATRIX GetViewMatrix(UINT iFaceNo) {if(iFaceNo > 5) iFaceNo = 5;	return m_MatView[iFaceNo];}

	// ��Ӱ��ͼ
	LPDIRECT3DCUBETEXTURE9 m_pTexShadowMap;

private:
	// ����ת����ͼ�Ƿ�ɹ��������Ƿ���Ⱦ����Ӱ��ͼ��1��ʾδ��Ⱦ����Ӱ��ͼ��2��ʾ���ɹ�
	UINT m_iCreateAttrib;
	// 6�����VIEW���󣨾�����Ŷ�ӦCube����ţ�
	D3DXMATRIX m_MatView[6];
	// �¾���Ȼ���ͺ�̨���壬����״̬����ʱʹ��
	LPDIRECT3DSURFACE9 m_pOldBackBuffer, m_pOldDepthBuffer, m_pDepthBuffer;

	// �������ŵ�
	LPDIRECT3DCUBETEXTURE9 m_pTexFaceIndex;
	// ���VIEW�����
	LPDIRECT3DTEXTURE9 m_pTexViewMatrix;
	LPDIRECT3DTEXTURE9 m_pTexViewMatrixFix;
};










// �÷���Init;   while(1){ Clear(); Render(); }
class KFPRenderTargetClear
{
public:
	KFPRenderTargetClear();
	~KFPRenderTargetClear();

	// ��ʼ��ˢ����ͼ��֧��CubeMap����Ȼֻ�ǰ�������һ����Ľ���
	HRESULT Init(LPDIRECT3DTEXTURE9 pSourceTex, float fR = 1.0f, float fG = 1.0f, float fB = 1.0f, float fA = 1.0f);
	HRESULT Init(LPDIRECT3DCUBETEXTURE9 pSourceTex, float fR = 1.0f, float fG = 1.0f, float fB = 1.0f, float fA = 1.0f);
	void Release();

	// ��ָ������ͼClear��ע�����ͼ�ͳ�ʼ��ʱ�������ͼ��ʽ����С������ȫһ�£�
	HRESULT Clear(LPDIRECT3DTEXTURE9 pSourceTex, float fR = 1.0f, float fG = 1.0f, float fB = 1.0f, float fA = 1.0f);
	HRESULT Clear(LPDIRECT3DCUBETEXTURE9 pSourceTex, UINT iFaceNo, float fR = 1.0f, float fG = 1.0f, float fB = 1.0f, float fA = 1.0f);

private:
	// ��ָ��ֵд��ˢ����ͼ�У�����������;��ֵ�������ò����Ƚ���
	HRESULT RefreshFloatValue(float fR, float fG, float fB, float fA);


	// ������ͼ�Ƿ�ɹ�
	UINT m_iCreateAttrib;

	// ���Float���ݵģ�����ÿ֡Clear FP Texture
	LPDIRECT3DTEXTURE9 m_pTexData;

	// ��ų�ʼ��ʱָ����ͼ�Ĳ��������ڲ����Ĵ���У����������Clearʱָ����ʽ����ȷ����ͼ
	D3DSURFACE_DESC m_Desc;

	// ���Clear����ɫ�������Զ�ˢ��Data��ͼ
	float m_fR, m_fG, m_fB, m_fA;
};



















// ����RT������֮��ĸ��ơ���ʽת���ȣ�ͨ������StretchRect��֧�ֵ�������縡�������ʽת��
// ��ҪSM2.0��For Shader Bilinear Filtering��
class KTextureCopy
{
public:
	KTextureCopy();
	~KTextureCopy() {Release();}

	// ��ʼ��Shader��Quad�ã��������ж���
	HRESULT Init();

	// ����ָ������ͼ������ֱ�����ͬ������Point Sample������ֱ��ʲ�ͬ������Bilinear Filtering�����ݵ����������Ĳ�ͬ��ʹ��Ӳ��Filter��Shader Filter
	// ������ͼ��С��̬����DepthBuffer����iWidth/iHeight����
	HRESULT TextureCopy(LPDIRECT3DTEXTURE9 pTexDst, LPDIRECT3DTEXTURE9 pTexSrc, BOOL bShaderFilter = TRUE);
	void Release();

private:
	// ������ͼ�Ƿ�ɹ�
	UINT m_iCreateAttrib;
	UINT m_iWidth, m_iHeight;		// ��ʱʹ�ñ�����ͼ�������Ƿ��ʼ��

	// Copyʱ�õ�Quad
	struct QUADVERTEXTYPE
	{
		float x,y,z;
		float tu,tv,tw,tx;	//������δ�ã�ֻ��Ϊ��ȥ��PS����Ĵ���
	};
	UINT m_iStride;
	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DSURFACE9 m_pDepthBuffer;

	LPDIRECT3DVERTEXSHADER9 m_pVSDrawQuad;
	LPDIRECT3DVERTEXDECLARATION9 m_pDeclaration;

	// Copy Pixel Shader
	LPDIRECT3DPIXELSHADER9 m_pPSCopyPoint, m_pPSCopyBilinear;
};