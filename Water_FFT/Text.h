#pragma once
#include "Shader.h"

#define MAX_GLARE_LINE_NUM 8

// Post Processing来源：场景、法线图、深度图
enum POSTPROCESSING_SOURCETYPE
{
	POSTRTTYPE_SCENE,
	POSTRTTYPE_NORMAL,
	POSTRTTYPE_DEPTH
};

// 预定义的类型，在下面的GlareData.Init中使用
enum GlareTypeDef
{
	Glare_PerpendicularCross,	// 十字形互相垂直，4个方向，即Cheap Len，旋转后相当于SpectralCross和CrossScreen
	Glare_PerspectCross,		// 十字形互相不垂直，4个方向，椭圆形的“X”
	Glare_SnowCross,			// 6个方向，一个横向，竖向是两条斜线组成的“X”，长度相同
	Glare_SpetralSnowCross,		// 6个方向，一个横向，竖向是两条斜线组成的“X”，长度不同，竖向两条斜线短
	Glare_SunnyCross,			// 8个方向，长度相同
	Glare_SpetralSunnyCross,	// 8个方向，长度不同，正十字长，斜十字短
};


//所有的字体当设备丢失时都必须释放，无论何种POOL
class MYTEXT
{
public:
	tagRECT Rect;       //输出范围(窗口坐标)
	DWORD TextFormat;  //对齐方式，DrawText Format Flags  (DT_XXX)

	float Deviation;     //描述字体圆滑程度，越小越圆滑
	float Extrusion;    //描述字体Z深度（即厚度）

	MYTEXT();
	~MYTEXT();
	void Release();

	//2D TEXT
	HRESULT InitFont2D(UINT FontSize, LPSTR FontName);
	void Set2DStyle(UINT Left, UINT Right, UINT Up, UINT Down, DWORD Format);
	//输出，初始默认是从起点开始，不剪切，如果需要特殊的对齐效果请执行Set2DStyle
	HRESULT DrawText2D(LPSTR Content, UINT Length, UINT Up, UINT Left, unsigned char r, unsigned char g, unsigned char b);
	//重载函数，可使用半透明，加了alpha值。
	HRESULT DrawText2D(LPSTR Content, UINT Length, UINT Up, UINT Left, unsigned char r, unsigned char g, unsigned char b, unsigned char alpha);

	
	//3D TEXT，可以支持中文字体中的英文及字符，但不支持中文。注意画的时候，打开灯光的情况下一定要设置材质，反之一定得关闭灯光，否则会显示有误
	//deviation表示圆滑程度（其实就是顶点间隔），越小表示顶点数越多，出来的模型也越圆滑，TrueType字体的一个属性，不能小于0。如果为0，则使用字体的默认值，一般为0.0xx
	//extrusion表示模型厚度，不要太厚，值为标准世界空间的坐标长度
	HRESULT InitFont3D(UINT FontSize, LPSTR FontName, LPSTR Content, float deviation, float extrusion);
	void SetMaterial(float DiffuseR, float DiffuseG, float DiffuseB, float SpecularR, float SpecularG, float SpecularB, float EmissiveR, float EmissiveG, float EmissiveB, float Power);
	HRESULT DrawText3D();

private:
    UINT Create2DFontAttrib;     //2D字体创建标志
	LPD3DXFONT DFont;  //D3D使用的2D字体
	LPD3DXMESH Mesh;   //D3D使用的3D字体模型
	D3DMATERIAL9 Material;  //3D模型的材质


//对齐格式的宏：DT_
//常用的：CALCRECT  NOCLIP  TOP LEFT RIGHT BOTTOM  CENTER  VCENTER
//还有：INTERNAL   WORDBREAK   SINGLELINE   EXPANTABS  TABSTOP   EXTERNALLEADING   NOPREFIX
};







//哈哈，好怀念，DOS->D3D，真是感慨万分，用法都相同
//一切自动完成，就像输出TEXT一样，不会影响任何设置的
//还有注意的是掩码和像素颜色无关，它是像素的ALPHA值，设置也是如此，其实应该叫ymalpha才对
//对没有alpha通道信息的图片进行alpha混合的时候，可以在调用PUTBMP之前用TSS_ALPHAOP设置通道为TFACTOR
//还有顶点的alpha值是越小越透明，跟GRAPH16刚好相反
class MYIMAGE
{
public:
	UINT AlphaEnable;     //是否允许半透明输出，要在程序中动态改变
	UINT TextureNum;            //已创建的纹理数，初始为0

	MYIMAGE();
	~MYIMAGE();
	//得到本模型从开始到现在的时间，单位秒（GETTICKCOUNT返回值1个单位约为55毫秒）
	float GetTime();
	void Release();

	//将指定顶点数据拷入显存内的顶点缓冲区，BufferPoint是顶点数据数组的指针，PointNum是表示有多少个顶点，EPointSize是每个顶点数据的大小，一般是sizeof(CUSTOMVERTEX)，即VertexShader所定义的顶点的储存大小
	HRESULT Init();
	//设置掩码颜色，TESTFUNC是EQUAL
	HRESULT Setymcolor(unsigned char a);
	//设置4个顶点的DIFFUSE，依次分别表示左上、右上、左下、右下的点
	void Setcolor(unsigned char r0, unsigned char g0, unsigned char b0, unsigned char a0, unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1, unsigned char r2, unsigned char g2, unsigned char b2, unsigned char a2, unsigned char r3, unsigned char g3, unsigned char b3, unsigned char a3);

	//创建第TEXTURENUM个纹理，纹理总数自动加1，不能超过最大可创建纹理数
	HRESULT Loadbmp(LPSTR Filename);
	//设置指定纹理，只用于设置ALPHA通道图，必须大于0,设置后用ALPHAOP=selectarg1, alphaarg1=通道图层数即可，不过用完记得恢复设置
	HRESULT Setalphabmp(UINT picno, int picx, int picy, int width, int height);
	//什么时候不用ALPHA通道图了，必须用这个恢复
	void Restorealphabmp();

	//输出，可以和ALPHA通道图共同作用的
	HRESULT Putbmp(UINT picno, int startx, int starty, int endx, int endy, int picx, int picy, int width, int height, UINT ym);
	HRESULT Putcolorbmp(UINT picno, int startx, int starty, int endx, int endy, int picx, int picy, int width, int height, UINT ym);
	HRESULT Putcolor(int startx, int starty, int endx, int endy, UINT ym);

private:
	//保存和恢复纹理、纹理层、渲染状态设置
	void Save();
	void Restore();
	DWORD Time; //时间控制
	UINT EachPointSize;   //每个顶点所占存储空间的大小，一般是sizeof(MYIMAGEVERTEXTYPE)
	UINT CreateAttrib;  //属性，0表示未初始化

	//自定义数据
	DWORD ymcolor;
	//是否设置了ALPHA通道图
	UINT alphabmpenable;
	//顶点数据和纹理数据
	struct MYIMAGEVERTEXTYPE
	{
		float x,y,z,rhw;
		DWORD diffuse;
		float tu,tv;
		float atu,atv;
	}MyImageVertex[4];  //数据流

	LPDIRECT3DVERTEXBUFFER9 VertexBuffer;  //顶点缓冲区
	LPDIRECT3DTEXTURE9 Texture[MYD3D_MODULETEXTURENUM];   //纹理，最多8个，和层数无关
	DWORD VertexShader;         //顶点格式,D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1

	//下面是保存的东西
	LPDIRECT3DBASETEXTURE9 BackupTexture, AlphaTexture;
	DWORD ColorOP, ColorArg1, ColorArg2;
	DWORD ColorOP_1, ColorArg1_1, ColorArg2_1;
	DWORD AlphaOP, AlphaArg1, AlphaArg2;
	DWORD AlphaOP_1, AlphaArg1_1, AlphaArg2_1;
	DWORD AlphaBlend, SrcBlend, DestBlend;
	DWORD AlphaTest, TestFunc;
};






// Glare辐射线数据
struct GlareLineData
{
	D3DXVECTOR2 VecDir;	// 方向
	UINT iLength;		// 长度，单位是像素，注意不要太大
	float fAttenuation;	// 衰减系数，单位%，值必须大于0，也可以大于1表示更亮，默认为1。它代表总体采样的一个缩放比率。注意衰减度不能过低，否则Glare效果就会大大减弱，甚至可能会看不出
	D3DCOLOR Color;		// 颜色，默认为白色
	
	GlareLineData()
	{
		VecDir = D3DXVECTOR2(0, 0);
		iLength = 0;
		fAttenuation = 1.0f;
		Color = D3DCOLOR_ARGB(255, 255, 255, 255);
	}
};

// Glare数据
struct GlareData
{
	UINT m_iLineNum;	// 多少条辐射线（方向），考虑到最终的Glare贴图层叠加，该值不可大于MAX_GLARE_LINE_NUM
	D3DXMATRIX m_MatRotate;	// 整体旋转
	GlareLineData *m_pLineData;	// 辐射线数据

	GlareData()
	{
		m_iLineNum = 0;
		D3DXMatrixIdentity(&m_MatRotate);
		m_pLineData = NULL;
	}
	~GlareData()
	{
		Release();
	}


	void Release()
	{
		m_iLineNum = 0;
		D3DXMatrixIdentity(&m_MatRotate);
		SAFE_DELETE_ARRAY(m_pLineData);
	}

	// 设置整体旋转，2D面上只能绕Z轴转动
	void SetRotate(float fAngle)
	{
		D3DXMatrixRotationZ(&m_MatRotate, fAngle);
	}


	// 根据设置初始化Glare数据，可多次初始化
	HRESULT SetGlareData(UINT iLineNum, LPD3DXVECTOR2 pVecDir, UINT *pLength, float *pAttenuation = NULL, LPD3DCOLOR pColor = NULL)
	{
		if(!iLineNum || iLineNum > MAX_GLARE_LINE_NUM || !pVecDir || !pLength)
			return D3DERR_INVALIDCALL;

		// 可多次初始化
		if(m_iLineNum != iLineNum)
		{
			SAFE_DELETE_ARRAY(m_pLineData);
			m_pLineData = new GlareLineData[iLineNum];
		}
		else
			m_pLineData = new GlareLineData[iLineNum];

		m_iLineNum = iLineNum;

		UINT i = 0;
		for(i = 0; i < m_iLineNum; i++)
		{
			D3DXVec2Normalize(&m_pLineData[i].VecDir, &pVecDir[i]);
			m_pLineData[i].iLength = pLength[i];
		}
		if(pAttenuation)
		{
			for(i = 0; i < m_iLineNum; i++)
				m_pLineData[i].fAttenuation = pAttenuation[i];
		}
		if(pColor)
		{
			for(i = 0; i < m_iLineNum; i++)
				m_pLineData[i].Color = pColor[i];
		}
		
		return S_OK;
	}

	// 重载函数，使用定义好的Glare模板
	HRESULT SetGlareData(GlareTypeDef enuGlareType)
	{
		if(enuGlareType == Glare_PerpendicularCross)
		{
			D3DXVECTOR2 VecDir[4] = {D3DXVECTOR2(1, 0),	D3DXVECTOR2(-1, 0), D3DXVECTOR2(0, 1), D3DXVECTOR2(0, -1)};
			UINT iLength[4] = {15, 15, 15, 15};
			V_RETURN(SetGlareData(4, VecDir, iLength));
		}

		if(enuGlareType == Glare_PerspectCross)
		{
			D3DXVECTOR2 VecDir[4] = {D3DXVECTOR2(1.7f, 1),	D3DXVECTOR2(1.7f, -1), D3DXVECTOR2(-1.7f, 1), D3DXVECTOR2(-1.7f, -1)};
			UINT iLength[4] = {15, 15, 15, 15};
			V_RETURN(SetGlareData(4, VecDir, iLength));
		}

		if(enuGlareType == Glare_SnowCross)
		{
			D3DXVECTOR2 VecDir[6] = {D3DXVECTOR2(1, 0),	D3DXVECTOR2(-1, 0), D3DXVECTOR2(1, 1), D3DXVECTOR2(1, -1), D3DXVECTOR2(-1, 1), D3DXVECTOR2(-1, -1)};
			UINT iLength[6] = {15, 15, 15, 15, 15, 15};
			V_RETURN(SetGlareData(6, VecDir, iLength));
		}

		if(enuGlareType == Glare_SpetralSnowCross)
		{
			D3DXVECTOR2 VecDir[6] = {D3DXVECTOR2(1, 0),	D3DXVECTOR2(-1, 0), D3DXVECTOR2(1, 1), D3DXVECTOR2(1, -1), D3DXVECTOR2(-1, 1), D3DXVECTOR2(-1, -1)};
			UINT iLength[6] = {15, 15, 8, 8, 8, 8};
			V_RETURN(SetGlareData(6, VecDir, iLength));
		}

		if(enuGlareType == Glare_SunnyCross)
		{
			D3DXVECTOR2 VecDir[8] = {D3DXVECTOR2(1, 0),	D3DXVECTOR2(-1, 0), D3DXVECTOR2(0, 1), D3DXVECTOR2(0, -1), D3DXVECTOR2(1, 1), D3DXVECTOR2(1, -1), D3DXVECTOR2(-1, 1), D3DXVECTOR2(-1, -1)};
			UINT iLength[8] = {30, 30, 30, 30, 30, 30, 30, 30};
			float fCoef[8] = {0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f};
			V_RETURN(SetGlareData(8, VecDir, iLength, fCoef));
		}

		if(enuGlareType == Glare_SpetralSunnyCross)
		{
			D3DXVECTOR2 VecDir[8] = {D3DXVECTOR2(1, 0),	D3DXVECTOR2(-1, 0), D3DXVECTOR2(0, 1), D3DXVECTOR2(0, -1), D3DXVECTOR2(1, 1), D3DXVECTOR2(1, -1), D3DXVECTOR2(-1, 1), D3DXVECTOR2(-1, -1)};
			UINT iLength[8] = {30, 30, 30, 30, 15, 15, 15, 15};
			float fCoef[8] = {0.7f, 0.7f, 0.7f, 0.7f, 0.5f, 0.5f, 0.5f, 0.5f};
			V_RETURN(SetGlareData(8, VecDir, iLength, fCoef));
		}

		return S_OK;
	}

};



















/*创建Filter VS声明时的模板
D3DVERTEXELEMENT9 dclr[] = {
	{0,0,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0,12,D3DDECLTYPE_FLOAT2 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};*/
class KPostProcess
{
public:
	KPostProcess();
	~KPostProcess();
	void Release();
	/***************系统功能接口********************/

	// 只是初始化Full-Screen Quad的顶点缓冲和Shader罢了，因为纹理格式和当前渲染的RT格式有关，所以都放到StartRender中进行
	HRESULT Init();
	
	// 专门用于得到当前处理过的纹理（强制转换为原大），比如EdgeGlow和SoftShadow。这里是复制出来的，所以要手动释放
	HRESULT GetCurrentImage(LPDIRECT3DTEXTURE9 *ppTex);

	// Start/End Render之间输出的画面将被拦截到该对象内部的等大小RT上，输出包括普通的Draw和PostProcess的HDR/EndProcess
	// 括号中的设置如果不是false（即使用MRT），就表示需要拦截深度图和法线图，那么就要设置MRT（设置该参数是为了提高速度，如果不用的话就不设置MRT，可以提升不少性能）
	HRESULT StartRender(bool bHDR = false, bool bMRT = false);
	HRESULT EndRender();
	
	// 开始拦截MRT
	HRESULT StartMRT(bool bNormal = true, bool bDepth = true);
	HRESULT EndMRT();
	// 这一对函数表明开始处理，Start是初始化内部数据，然后运行需要的PostProcess函数即可，最后让EndProcess用指定的shader将Process好的画面合成到原来的RT上（等于直接输出了，不用再Draw）
	// 同一个PostProcess对象在调用一次Start/EndRender后可以多次调用Start/EndProcess，处理的类型都可以不一样。该函数会自动重置ScaleScene->Temp1及m_pTex/RT指针，所以同一个KPost对象可以同时处理Bloom、Edge和Gaussian，通过多次调用Start/EndProcess来实现
	HRESULT StartProcess();
	// 只是重置色彩场景到Temp1，并重置m_pTex/RT指针，为了在同一个Process中同时处理色彩场景和法线、深度图
	HRESULT ResetProcess();

	
	// 在EndProcess中使用的shader，表示如何将被Start/EndRender拦截的原始LDR画面（即m_pSourceScene或m_pToneMap）和使用者指定的图像作为纹理绘制到当前的RT上去
	// 这些纹理（1～7）连同被拦截的画面（m_pSourceScene/m_pToneMap作为第0层纹理）使用自定义的shader绘制到当前RT上，用于多个PostProcessing之间的混合，比如软阴影、EdgeGlow等

	// 如果pixel shader指针为空，就直接Copy到当前RT上，如果不为空则使用该shader混合，shader所需要的特殊常量、渲染参数需要在EndProcess之前设置好
	// VERTEX SHADER就用MultiTextureBlend.vsh，8层的纹理都被正确的传入，在PS中直接寻址即可
	
	// 自定义的纹理，比如预先被别的Post对象处理过的软阴影、EdgeGlow之类，支持自定义多达7层的纹理（0是内部m_pTexture，1～7是自定图像）
	// 根据TexNum来指定一共有多少个纹理，序号必须和ppTexture数据对应，默认是无
	
	// HDR混合也是在这里做，前面需要得到所有的纹理，Bloom/Glare/EdgeGlow/SoftShadow，并预先计算好ToneMap，它存放在第0层（调用HDR ToneMapping的话就会自动取代SourceScene）
	HRESULT EndProcess(PIXELSHADER* pPS = NULL, UINT iTexNum = 0, LPDIRECT3DTEXTURE9 *ppTexture = NULL);

	// 替代EndProcess，强制结束一个PostProcess，除了重置内部数据和恢复状态、原RT以外什么都不做，一般要和GetCurrentImage连用（比如软阴影）
	HRESULT ForceEndProcess();

	/**************Post Process功能接口******************/
	// 高斯函数、基于法线图的边缘增强是暴露给外部，可以重复使用，而且是同样大小的，所以可以在函数内部交换m_pTex/RT的指针（适用于Temp1/Temp2）
	HRESULT Gaussian1D(bool bHorizon, float fRadius = 5.0f);
	HRESULT Gaussian5x5(float fRadius = 20.0f);
	HRESULT EdgeEnhanced(bool bIncr = true, bool bEdgeColorWhite = true);	// 边缘增强，true表示增加，false表示削弱，可多次使用

	// 强制将法线或深度原大图（根据参数）进行Edge Detection并存储到临时中，然后转存到pTemp1并重置m_pTex/RT，这样即可以让场景、深度兼得，又可以避免法线图内插或求均值时产生误差
	// 它和HDR/LDR场景是并行的，完全不影响原场景输出，只作为一个附加的图像，最终必定在pTemp1中
	HRESULT EdgeDetection(POSTPROCESSING_SOURCETYPE ProcRTType = POSTRTTYPE_NORMAL, float fThreshold = 0.0f, bool bEdgeColorWhite = true);

	// 把场景或EDGE图经过高斯模糊处理，它存放在m_pTexture中，用GetImage得到即可，这种模糊图不需要太清晰，所以比较小，不像Tone Mapping一样有专用的贴图，所以必须用GetImage复制出来
	HRESULT EdgeGlow(POSTPROCESSING_SOURCETYPE ProcRTType = POSTRTTYPE_NORMAL, float fThreshold = 0.0f, bool bEdgeColorWhite = true);	//高级组合特效，将当前的m_pTex（就是已经用户处理好的边缘指针）BrighterPass和高斯模糊处理，得到纯EdgeGlow图

	

	// 设定使用的Glare数据
	HRESULT SetGlare(GlareData *pGlare)
	{
		if(!pGlare)
		{
			m_bGlare = false;
			OutputDebugString("Glare已关闭！");
			return S_OK;
		}

		if(!pGlare->m_iLineNum || pGlare->m_iLineNum > MAX_GLARE_LINE_NUM)
			return D3DERR_INVALIDCALL;
		
		m_GlareData.Release();

		m_GlareData.m_iLineNum = pGlare->m_iLineNum;
		m_GlareData.m_MatRotate = pGlare->m_MatRotate;
		m_GlareData.m_pLineData = new GlareLineData[m_GlareData.m_iLineNum];
		for(UINT i = 0; i < m_GlareData.m_iLineNum; i++)
		{
			D3DXVec2Normalize(&m_GlareData.m_pLineData[i].VecDir, &pGlare->m_pLineData[i].VecDir);
			m_GlareData.m_pLineData[i].iLength = pGlare->m_pLineData[i].iLength;
			m_GlareData.m_pLineData[i].fAttenuation = pGlare->m_pLineData[i].fAttenuation;
			m_GlareData.m_pLineData[i].Color = pGlare->m_pLineData[i].Color;
		}

		m_bGlare = true;
		return S_OK;
	}

	// 这两个函数都是用定死的源RT，所以在任何时候都可以随意使用而不会影响m_pTex/RT指针
	// 但是Bloom是放到TempRT中的，所以执行完后需要立即GetImage；而HDR2LDR更是连目的RT都是专用的，直接保存返回的指针即可
	// Bloom代表HDR周围的一切效果，光晕和Glare都包含在里面
	HRESULT Bloom(float fKeyValue=0, float fClampValue=0, float fOffset=20.0f);	// 根据HDR设置来处理HDR Bloom还是LDR Bloom
	// 只是映射HDR到LDR的操作而已，映射完毕m_pTex指向专用贴图ToneMapRT，EndProcess时会自动设置为第0层纹理
	HRESULT HDR2LDR(float fKeyValue = 0.18f);
	// 模拟Depth Of Field，参数是聚焦平面、远近平面，具体请参考DOF原理，使用DOF前一定要指定FP16或FP32，否则会强制整数纹理，出现错误结果
	// 最后一个参数是模糊强度，就是COF的最大半径，该值越大效果越模糊，默认3.0，注意太低的话可能看不出什么模糊效果哦
	HRESULT DOF(float fFocalPlane, float fNearPlane = 0.0f, float fFarPlane = 0.0f, float fBlurPower = 3.0f);
	
	

	
private:
	// 内部使用的组合功能处理函数
	HRESULT Resize();	// 根据纹理属性自动选择Copy、shader过滤Resize、硬件过滤Resize
	HRESULT MeasureLuminance();	// 统计全局亮度，自动根据shader版本选择ps2.0还是更高的操作
	HRESULT BrightPass(float fKeyValue, float fClampValue, float fOffset);	// KeyValue在LDR中就是Bright-Coef，fOffset只在HDR中有效，从ScaleScene到temp1，强制置指针
	HRESULT CalcAdapt();	// 这完全是内部操作，不会使用m_pTex/RT指针
	// 不能交换pTex/pRT指针，因为牵扯的纹理太广，必须用外部接口调用它们时手动交换指针以确保数据正确性
	// ToneMap需要清晰的大图，所以直接在原大的图像上面操作，需要专用贴图来保存结果，这个函数将图像转换到它专用的贴图中，即从SourceScene->ToneMap，不牵扯m_pTex/RT指针
	HRESULT ToneMapping(float fKeyValue);

	// 内部使用的基本功能处理函数，适用于不同大小或多个过程，所以不要在内部交换指针
	HRESULT Glare();		// 一次或多次模糊所有方向的贴图并存至对应的Glare贴图（完全内部调用，不改变m_pTex/RT指针）
	HRESULT AddGlare();		// 将所有这些Glare贴图叠加到BrightPass中（完全内部调用，不改变m_pTex/RT指针）
	HRESULT DOFShader(LPDIRECT3DTEXTURE9 pTexSource, LPDIRECT3DTEXTURE9 pTexBlurred, float fFocalPlane, float fNearPlane, float fFarPlane, float fBlurPower);	//必须ps.2.a的硬件才能用！
	HRESULT MeasureLuminanceShader();	//这完全是内部操作，不会使用m_pTex/RT指针，直接将数据拷贝到m_pLumAdaptFrameRT上
	HRESULT MeasureLuminanceShaderPS20();	// 同上，它用于ps2.0，要增加一个pass来绘制，将log和ds4x4分开，放心，采样像素不多，不会影响性能
	HRESULT CalcAdaptShader(float fFrameTime, float fIdleTime);	//这完全是内部操作，不会使用m_pTex/RT指针
	HRESULT CopyResize(bool bUseOldRT = false);	// 大小相同的情况下就是Copy，不同时就是Resize，启用硬件纹理过滤。它有个特殊的作用，就是无视m_pRT直接输出到m_pOldRT，因为这两个指针类型不同，不能让m_pRT等于OldRT，而在EndProcess中可能又需要输出到OldRT Surface
	HRESULT DownScale2x2();
	HRESULT DownScale4x4();
	HRESULT BilinearResize(bool bUseOldRT = false);	//强制用Shader过滤，性能较低。它有个特殊的作用，就是无视m_pRT直接输出到m_pOldRT，因为这两个指针类型不同，不能让m_pRT等于OldRT，而在EndProcess中可能又需要输出到OldRT Surface
	HRESULT EdgeDetectionShader(POSTPROCESSING_SOURCETYPE ProcRTType, float fThreshold, bool bEdgeColorWhite);	//根据类型做具体的纯Shader边缘检测，不用交换指针，完全由组合函数控制
	

	/**************内部变量*******************/
	struct MYIMAGEVERTEXTYPE
	{
		float x,y,z;
		float tu,tv;
	};
	
	D3DSURFACE_DESC m_DescRT, m_DescDepth;		// 原RT、Depth缓冲的描述，每次都会在StartRender中初始化
	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer, m_pBilinearVertexBuffer;	// 顶点缓冲区，第二个用于FP Bilinear Filtering
	LPDIRECT3DSTATEBLOCK9 m_pStateBlock;		// 保存渲染参数和状态

	// CreateAttrib:	1表示初始化成功 2表示已经StartRender 3表示已经EndRender（尚未StartProcess或已经EndProcess）  4表示已经StartProcess
	// MRTAttrib:		1表示创建MRT成功(StartRender),还未StartMRT任何一个  2表示已经StartMRT Normal   3表示已经StartMRT Depth   4表示Normal/Depth都已经StartMRT
	//					5 6 7分别表示Normal/Depth/Both 都已经EndMRT
	UINT m_iCreateAttrib, m_iMRTAttrib;
	
	// 分别表示是否已经计算过全局亮度、自动适应亮度和边缘检测，因为Lum/Adapt这种公用步骤，在Bloom/Tone中都有可能调用，为了提高性能在这里做标记以便判断
	bool m_bMeasureLuminance, m_bCalcAdapt, m_bCalcEdge, m_bCalcToneMap;

	// 拦截设置，是否MRT，是否HDR，根据选择跳过创建和其他操作，以提高性能
	bool m_bMRT, m_bHDR;

	// 是否使用Glare，只有调用SetGlare后，Glare才有效
	bool m_bGlare;
	GlareData m_GlareData;	// 用户设定的Glare数据
	

	/********************* 内部Texture ************************/
	// 图像处理的时候是关掉Z缓冲的，附加的这些Z缓冲只是为了匹配相同大小的RT，而不至于渲染出错
	// 所有的贴图格式都和OldRT一样（如果提前启动了HDR，那么这些贴图都变成HDR，否则为普通整数纹理）
	
	// 调用StartRender时的RT及深度缓冲，备份用
	LPDIRECT3DSURFACE9 m_pOldRT, m_pOldDepth;
	
	// 新建一个最大的深度缓冲，其实不需要Z缓冲的，但这里为了在渲染RT时设置上去，不然会出错
	LPDIRECT3DSURFACE9 m_pFullSceneDepth;

	// 原场景图和新场景图，等大
	LPDIRECT3DTEXTURE9 m_pSourceSceneRT;
	// 场景的法线图和深度图，和原图等大，用于MRT，在StartRender的时候根据参数设置，NormalEdge是临时使用的，用于将Normal无损转换为Edge，都是原大
	LPDIRECT3DTEXTURE9 m_pDepthSceneRT, m_pNormalSceneRT;

	// 临时使用的贴图，NormalEdge是原大，存放边缘图，DepthEdge是1/4，用于暂存线性过滤后的深度图
	LPDIRECT3DTEXTURE9 m_pEdgeTemp1RT, m_pEdgeTemp2RT, m_pDepthEdgeRT;
		
	// 场景缩小1/4（所有Post Processing的来源），和场景缩放到256x256（只用于统计全局亮度），256TempRT是用于ps2.0上先计算log再DownScale临时使用的
	LPDIRECT3DTEXTURE9 m_pScaleSceneRT, m_p256x256SceneRT, m_p256x256TempRT;
	
	// 临时使用，1/4原图大小，纯内部操作，整数纹理（可以很大程度上提升性能，尤其是不需要Bilinear了）
	LPDIRECT3DTEXTURE9 m_pTemp1RT, m_pTemp2RT;
	
	// 适应亮度数据(R16F)，1x1的贴图，表示当前场景亮度、上次移动前的场景亮度、本帧计算出来的适应亮度（即直接用来映射到LDR的亮度）
	LPDIRECT3DTEXTURE9 m_pLumAdaptPrevFrameRT, m_pLumAdaptCurrentFrameRT;

	// 经过处理后的ToneMap，原图大小（Bloom要求精度不高，所以用1/4大小就行了，在pTempRT中）
	LPDIRECT3DTEXTURE9 m_pToneMapRT;

	// 渲染好的Glare贴图，每张代表一个拖尾方向，而且每张都可能会被沿同一个方向模糊多次（增加拖尾长度）
	LPDIRECT3DTEXTURE9 m_ppGlareRT[MAX_GLARE_LINE_NUM];	// 最多MAX_GLARE_LINE_NUM个方向
	LPDIRECT3DTEXTURE9 m_pGlareTemp1RT, m_pGlareTemp2RT;	// 临时使用，用于多次模糊
	
	// 经过处理后的Depth Of Field纹理，原图大小，LDR
	LPDIRECT3DTEXTURE9 m_pDOFRT;

	// 将256x256Scene(256)->1x1的贴图(R16F)，所以该贴图从0到3必须是64*64 16*16 4*4 1*1，只用于统计全局亮度，最后一个永远是即时计算出本帧的HDR全局亮度（和适应、映射无关，同一个HDR场景，无论怎样适应、映射，全局亮度都是完全一样的）
	LPDIRECT3DTEXTURE9 m_ppTexDownTo1[4];

	// 指针，永远指向当前RT、当前Texture和当前适合的Depth，内部操作
	LPDIRECT3DTEXTURE9 m_pTexture, m_pRT;
	LPDIRECT3DSURFACE9 m_pDepth;	// 事实上永远指向m_pFullSceneDepth，这里只是方便操作罢了，把这三个归类到一起
	
	
	/********************* 内部Shader ************************/
	
	// 公用绘制QUAD、shader线性过滤和处理最终场景的Vertex Shader
	VERTEXSHADER m_VSDrawQuad, m_VSBilinearResize, m_VSMultiTextureBlend;
	
	// 高斯模糊
	PIXELSHADER m_PSGaussianH, m_PSGaussianV, m_PSGaussian5x5;
	
	// DownScale，经常用于求和的平均值，所以必须保证寻址到Texel中心，而且让它不要内插，即POINT过滤
	PIXELSHADER m_PSDownScale4x4, m_PSDownScale2x2, m_PSCopyResize;

	// 复制纹理用shader进行线性过滤，用于Bloom中FP32或FP16到原大的过程（其他地方尽量不要使用）
	PIXELSHADER m_PSBilinearResize;
	
	// 统计全局亮度（中间的步骤就是DownScale），First是用log求和代替求和，Last是用exp求和代替求和
	PIXELSHADER m_PSLumLogFirst, m_PSLumLogFirstWithoutDS4x4, m_PSLumLogLast;
	// 计算人眼自动适应
	PIXELSHADER m_PSCalcAdapt;
	
	// 边缘检测及边缘宽度修正
	PIXELSHADER m_PSSobel3x3, m_PSNormal2x2, m_PSDepth2x2;
	PIXELSHADER m_PSEdgeIncrWhite, m_PSEdgeDecrWhite, m_PSEdgeIncrBlack, m_PSEdgeDecrBlack;
	
	// HDR相关，ToneMapping、Bloom
	PIXELSHADER m_PSToneMapping, m_PSBrightPass, m_PSBrightPassLDR;

	// Glare
	PIXELSHADER m_PSGlare, m_PSAddGlare;

	// 其他相关
	PIXELSHADER m_PSDOF;	// Depth Of Field
	
	
	/**************纯内部接口（方便调用）******************/

	// 清除所有RT DEPTH、指针和标记（除了顶点缓冲和Shader以外都在这里清除）
	void FreeTex();

	// 用StretchRect进行同大小纹理复制
	HRESULT CopyTexture(LPDIRECT3DTEXTURE9 pTexSrc, LPDIRECT3DTEXTURE9 pTexDst);
	
	// 得到1D高斯函数权重
	float GetGaussianWeight(float fX, float fY, float fRadius);

	// 设置高斯参数到常量寄存器，1D是-8～8的（H或V由使用哪个shader来决定），5x5就表示5x5（其他任何参数都没有），Radius越大，图像越模糊
	void SetGaussian1D(float fRadius);	//设置到c10/11/12为权重   c31为Fix
	void SetGaussian5x5(float fRadius);	//设置到c20/21/22/23为权重   c31为Fix

};