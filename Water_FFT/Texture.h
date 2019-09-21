#pragma once

// 任意贴图之间的复制，比较慢
HRESULT D3DXLoadTextureFromTexture(LPDIRECT3DTEXTURE9 pTexDst, LPDIRECT3DTEXTURE9 pTexSrc);
// 从六个图片文件名创建一张CubeMap，文件名数组依次为“X正、负；Y正、负；Z正、负轴的贴图”
// POOL_MANAGED，仅作为俭省天空盒子的存储容量而做
HRESULT CreateCubeMapFromSixFiles(char **ppszFileName, LPDIRECT3DCUBETEXTURE9 *ppCubeTex, UINT iLength, UINT iMip = 0, D3DFORMAT Format = D3DFMT_A8R8G8B8);
// 将贴图中像素的值log出来，支持任意2D贴图
HRESULT TextureLogOut(char *pszFileName, LPDIRECT3DTEXTURE9 pSrcTex);


//此类可保存和恢复当前两个面的指针，供渲染到纹理使用
//先初始化，再渲染，最后设置渲染好的纹理即可
//注意一定要跳过需要贴图的物体哦
class RENDERTOTEXTURE
{
public:
	RENDERTOTEXTURE();
	~RENDERTOTEXTURE();
	void Release();
	//初始化纹理，并没有内容，SIZE是分辨率
	HRESULT InitTexture(UINT Size);
	//渲染，要给定VIEW参数，还有PROJ的视角弧度值（长宽的范围）ARG, 另外HEADX/Z不用给了，都是0，用法参考CUBEMAP的渲染函数
	HRESULT RenderToTexture(float Eyex, float Eyey, float Eyez, float Lookx, float Looky, float Lookz, float Heady, float Arg, UINT *Mask);
	//设置当前纹理
	HRESULT SetTexture(DWORD Stage);

private:
	//需要保存的东东
	LPDIRECT3DSURFACE9 OldDepthBuffer, OldBackBuffer;
	//渲染到这个表面，并用新的深度缓冲
	LPDIRECT3DSURFACE9 DepthBuffer;
	LPDIRECT3DTEXTURE9 RenderTexture;
	UINT CreateAttrib;//创建标志,1为已创建，未渲染，2未已渲染
};





//使用提示：1、渲染环境到表面的函数需要放到主循环Render的后面，即每帧渲染结束，因为这样不影响每个物体的TSS,RS等设置
//			2、SetTexture会自动使用反射向量纹理坐标变换，贴映射图完成后要Restore，以免影响其他物体。
//          3、每次渲染环境到纹理时要注意设好Mask，需要贴图的物体必须要跳过
class CUBEMAP
{
public:
	CUBEMAP();
	~CUBEMAP();
	void Release();
	//从文件建立立方纹理
	HRESULT InitCubeMap(LPSTR Filename);
	//建立一个环境映射的立方纹理
	HRESULT InitCubeMap(UINT Size);
	//渲染环境到表面，只能用于环境映射，渲染前自动备份，渲染后自动恢复
	//Eye表示观看环境的中心位置在哪，Mask是数组，在ModuleNum个物体中需要屏蔽的对应序号就设为0,需要映射的就设为1
	HRESULT RenderToSurface(float Eyex, float Eyey, float Eyez, UINT *Mask);
	//设置当前立方纹理到第Stage层
	HRESULT SetTexture(DWORD Stage);
	void RestoreSettings();

private:
	//建立贴图是否成功，还有建立的是什么类型的，1表示环境，2表示从文件，0表示未建立
	UINT CreateAttrib;
	//设置纹理到哪层
	UINT CubeMapStage;
	//立方纹理
	LPDIRECT3DCUBETEXTURE9 CubeMap;
	//保存旧表面
	LPDIRECT3DSURFACE9 OldDepthBuffer, OldBackBuffer;
	//新的Z缓冲，需要初始化
	LPDIRECT3DSURFACE9 DepthBuffer;
};







//注意使用类中的SetTexture函数时必须先将环境和底图SETTEXTURE，而且坐标必须自己指定好，其他的就不用管了
//如果设备不支持，会弹出警告对话框，所有的初始化、设置纹理和恢复设置都会失效
//若设备不支持的话需要自己修改纹理层的设置，去掉凹凸层，将环境层和底层设为ALPHA混合，但要在渲染完恢复两层原来的渲染状态，否则会影响全局的效果。
class BUMPMAP
{
public:
	BUMPMAP();
	~BUMPMAP();
	void Release();
	//得到设备是否支持凹凸映射的信息
	HRESULT GetSupport();
	//从文件读取，并将凹凸贴图信息初始化，第二个参数是凹凸信息类型
	HRESULT InitFromFile(LPSTR Filename, D3DFORMAT Format);
	//设置凹凸纹理到指定层并设置该层的TSS属性，下层必须还有一个环境纹理，两个重载函数分别对应无亮度信息和有亮度的，必须和Format一致。
	HRESULT SetTexture(DWORD Stage, float M00, float M01, float M10, float M11, float O, float S);
	HRESULT SetTexture(DWORD Stage, float M00, float M01, float M10, float M11);
	//恢复纹理层设置
	void RestoreSettings();

private:
	//保存纹理层设置，内部调用
	void Save();
	//存放信息纹理和SETTEXTURE设置的层数
	LPDIRECT3DTEXTURE9 BumpMap;
	DWORD BumpStage;
	int CreateAttrib; //1为已创建并初始化，-1为不支持
	//备份3个STAGE的TSS状态，RESTORESETTINGS用
	DWORD ColorOP_1, ColorArg1_1, ColorArg2_1, TexIndex_1;
	DWORD ColorOP_2, ColorArg1_2, ColorArg2_2, TexIndex_2;
	DWORD ColorOP_3, ColorArg1_3, ColorArg2_3, TexIndex_3;
};










//注意使用类中的SetTexture函数时必须先将环境和底图SETTEXTURE，而且坐标必须自己指定好，其他的就不用管了
class NORMALMAP
{
public:
	NORMALMAP();
	~NORMALMAP();
	void Release();
	//从文件读取，并将法线图信息初始化，Scale是法向量的倍数，DX是用D3DX自动生成法线图的
	HRESULT InitFromFile(LPSTR Filename, double Scale);
	HRESULT InitFromFileDX(LPSTR Filename, float Scale);
	//自己填充法线数据，需要自己修改代码
	HRESULT InitFromUser(UINT size);
	//填充高光乘方查表数据，Power是多少次方
	HRESULT InitPowerMap(UINT size, double Power);
	//填充各向异性光照图数据
	HRESULT InitAnisotropyMap(UINT size);
	//填充各向异性方向图数据，Power是全局光照强度，0～1之间，越大表示强度越高，光照越集中，反之表示强度越小，光扩散范围越大
	HRESULT InitAnisotropyDirMap(UINT size, double Power);

	//设置法向量图到指定层并设置该层的TSS属性
	//三个坐标是光照方向向量，不过注意这个是从像素指向光源的方向向量哦
	HRESULT SetTexture(DWORD Stage, float Lx, float Ly, float Lz);
	//恢复纹理层设置，仅用于SetTexture函数
	void RestoreSettings();

	//下来是分别设置法向量图、高光乘方图和各向异性光照图的，仅用于PS，所以它并不改变TSS设置，只是简单的SETTEXTURE而已
	HRESULT SetNormalMap(DWORD Stage);
	//注意设置了POWER图，必须手动将该层纹理提交设为CLAMP
	HRESULT SetPowerMap(DWORD Stage);
	HRESULT SetAnisotropyMap(DWORD Stage);
	HRESULT SetAnisotropyDirMap(DWORD Stage);
	

private:
	//存放信息纹理和SETTEXTURE设置的层数
	LPDIRECT3DTEXTURE9 NormalMap;
	//存放高光乘方查表图，仅用于PS
	LPDIRECT3DTEXTURE9 PowerMap;
	//存放各向异性光照和方向图，仅用于PS
	LPDIRECT3DTEXTURE9 AnisotropyMap;
	LPDIRECT3DTEXTURE9 AnisotropyDirMap;
	UINT CreateNormalAttrib; //1为已创建并初始化
	UINT CreatePowerAttrib; //1为已创建并初始化
	UINT CreateAnisotropyAttrib; //1为已创建并初始化
	UINT CreateAnisotropyDirAttrib; //1为已创建并初始化
	//备份STAGE的TSS状态，RESTORESETTINGS用
	DWORD NormalStage;
	DWORD ColorOP, ColorArg1, ColorArg2;
};








//这个是用来做像素光照的，初始化的CUBEMAP专门用来进行归一化向量查表，将纹理坐标设为需要转换的N或L向量，然后采样出来的颜色值就已经是归一化过的了，进行DOT3即可
//大小选256比较合适，512支持显卡的没256多，另外MIPMAP在这个CUBEMAP中是禁止的，（效果好一点），还有最重要的就是纹理过滤一定要设为POINT，LINEAR插值的时候会影响向量长度，效果会变差
class NORMALCUBEMAP
{
public:
	NORMALCUBEMAP();
	~NORMALCUBEMAP();
	void Release();
	//建立一个查询归一化向量的立方纹理
	HRESULT InitCubeMap(UINT Size);
	//建立一个查询衰减系数的普通纹理
	HRESULT InitAttenMap(UINT Size);
	//设置当前立方纹理到第Stage层
	HRESULT SetCubeTexture(DWORD Stage);
	//设置当前衰减纹理到第Stage层
	HRESULT SetAttenTexture(DWORD Stage);

private:
	//建立贴图是否成功，还有建立的是什么类型的，1表示环境，2表示从文件，0表示未建立
	UINT CreateAttrib, CreateAttribAtten;
	//立方纹理
	LPDIRECT3DCUBETEXTURE9 CubeMap;
	//衰减纹理
	LPDIRECT3DTEXTURE9 AttenMap;
};





// 这个类只是简化操作罢了，并不牵扯到Shader
class KShadowMap
{
public:
	KShadowMap();
	~KShadowMap();

	// 初始化转换贴图，创建阴影贴图
	HRESULT Init(UINT iSize = 512);
	void Release();

	// 设置阴影贴图为RT，准备进行渲染（MRT = 1），参数是转换纹理所在层
	HRESULT RenderShadowTexture(DWORD dwStage);
	// 渲染好阴影贴图之后一定要恢复旧的RT和深度缓冲
	HRESULT RestoreRenderTarget();
	// 设置渲染好的阴影贴图，准备渲染物体阴影，注意考虑到转换纹理，所以它要占两层贴图
	HRESULT SetShadowTexture(DWORD dwStage);

private:
	// 建立转换贴图是否成功，还有是否渲染过阴影贴图，1表示未渲染过阴影贴图，2表示都成功
	UINT m_iCreateAttrib;
	// 新旧深度缓冲和后台缓冲，保存状态和临时使用
	LPDIRECT3DSURFACE9 m_pOldBackBuffer, m_pOldDepthBuffer, m_pDepthBuffer;

	// 转换浮点深度到整数颜色
	LPDIRECT3DTEXTURE9 m_pTexConvertor;
	// 阴影贴图
	LPDIRECT3DTEXTURE9 m_pTexShadowMap;
	LPDIRECT3DSURFACE9 m_pSurfShadowMap;
};



















// 这个类只是简化操作罢了，并不牵扯到Shader，注意只能用于点光源
// 用法：SaveRenderTarget();  for(i = 0; i < 6; i++){ RenderShadowTexture(); 自定义的RenderToShadowMap(); } RestoreRenderTarget();  SetShadowTexture(); 自定义的RenderShadow();
class KOmniShadowMap
{
public:
	KOmniShadowMap();
	~KOmniShadowMap();

	// 初始化转换贴图，创建阴影贴图
	HRESULT Init(UINT iSize = 512);
	void Release();

	// 设置阴影贴图为RT，准备进行渲染（不用MRT），在设置之后必须用同样的渲染语句来画，所以先后要设置6次，表示不同的面
	HRESULT RenderShadowTexture(UINT iFaceNo, D3DXVECTOR3 PtPointLight);
	// 渲染阴影贴图前一定要保存RT和深度缓冲，渲染好阴影贴图之后一定要恢复旧的RT和深度缓冲
	HRESULT SaveRenderTarget();
	HRESULT RestoreRenderTarget();
	// 设置渲染好的阴影贴图，准备渲染物体阴影，注意考虑到两张数据贴图，所以一共要占三层
	HRESULT SetShadowTexture(DWORD dwStage);
	// 得到View矩阵，必须在RenderShadowTexture之后进行
	D3DXMATRIX GetViewMatrix(UINT iFaceNo) {if(iFaceNo > 5) iFaceNo = 5;	return m_MatView[iFaceNo];}

	// 阴影贴图
	LPDIRECT3DCUBETEXTURE9 m_pTexShadowMap;

private:
	// 建立转换贴图是否成功，还有是否渲染过阴影贴图，1表示未渲染过阴影贴图，2表示都成功
	UINT m_iCreateAttrib;
	// 6个面的VIEW矩阵（矩阵序号对应Cube面序号）
	D3DXMATRIX m_MatView[6];
	// 新旧深度缓冲和后台缓冲，保存状态和临时使用
	LPDIRECT3DSURFACE9 m_pOldBackBuffer, m_pOldDepthBuffer, m_pDepthBuffer;

	// 存放面序号的
	LPDIRECT3DCUBETEXTURE9 m_pTexFaceIndex;
	// 存放VIEW矩阵的
	LPDIRECT3DTEXTURE9 m_pTexViewMatrix;
	LPDIRECT3DTEXTURE9 m_pTexViewMatrixFix;
};










// 用法：Init;   while(1){ Clear(); Render(); }
class KFPRenderTargetClear
{
public:
	KFPRenderTargetClear();
	~KFPRenderTargetClear();

	// 初始化刷新贴图，支持CubeMap，当然只是按照其中一个面的进行
	HRESULT Init(LPDIRECT3DTEXTURE9 pSourceTex, float fR = 1.0f, float fG = 1.0f, float fB = 1.0f, float fA = 1.0f);
	HRESULT Init(LPDIRECT3DCUBETEXTURE9 pSourceTex, float fR = 1.0f, float fG = 1.0f, float fB = 1.0f, float fA = 1.0f);
	void Release();

	// 将指定的贴图Clear，注意该贴图和初始化时传入的贴图格式、大小必须完全一致！
	HRESULT Clear(LPDIRECT3DTEXTURE9 pSourceTex, float fR = 1.0f, float fG = 1.0f, float fB = 1.0f, float fA = 1.0f);
	HRESULT Clear(LPDIRECT3DCUBETEXTURE9 pSourceTex, UINT iFaceNo, float fR = 1.0f, float fG = 1.0f, float fB = 1.0f, float fA = 1.0f);

private:
	// 将指定值写入刷新贴图中，可以用于中途换值，不过该操作比较慢
	HRESULT RefreshFloatValue(float fR, float fG, float fB, float fA);


	// 建立贴图是否成功
	UINT m_iCreateAttrib;

	// 存放Float数据的，用于每帧Clear FP Texture
	LPDIRECT3DTEXTURE9 m_pTexData;

	// 存放初始化时指定贴图的参数，用于参数的错误校正，避免在Clear时指定格式不正确的贴图
	D3DSURFACE_DESC m_Desc;

	// 存放Clear的颜色，用于自动刷新Data贴图
	float m_fR, m_fG, m_fB, m_fA;
};



















// 用于RT和纹理之间的复制、格式转换等，通常用于StretchRect不支持的情况，如浮点纹理格式转换
// 需要SM2.0（For Shader Bilinear Filtering）
class KTextureCopy
{
public:
	KTextureCopy();
	~KTextureCopy() {Release();}

	// 初始化Shader和Quad用，纹理在中定义
	HRESULT Init();

	// 复制指定的贴图，如果分辨率相同，就用Point Sample，如果分辨率不同，就用Bilinear Filtering，根据第三个参数的不同，使用硬件Filter和Shader Filter
	// 根据贴图大小动态分配DepthBuffer，用iWidth/iHeight提速
	HRESULT TextureCopy(LPDIRECT3DTEXTURE9 pTexDst, LPDIRECT3DTEXTURE9 pTexSrc, BOOL bShaderFilter = TRUE);
	void Release();

private:
	// 建立贴图是否成功
	UINT m_iCreateAttrib;
	UINT m_iWidth, m_iHeight;		// 临时使用保存贴图，决定是否初始化

	// Copy时用的Quad
	struct QUADVERTEXTYPE
	{
		float x,y,z;
		float tu,tv,tw,tx;	//后两个未用，只是为了去掉PS编译的错误
	};
	UINT m_iStride;
	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DSURFACE9 m_pDepthBuffer;

	LPDIRECT3DVERTEXSHADER9 m_pVSDrawQuad;
	LPDIRECT3DVERTEXDECLARATION9 m_pDeclaration;

	// Copy Pixel Shader
	LPDIRECT3DPIXELSHADER9 m_pPSCopyPoint, m_pPSCopyBilinear;
};