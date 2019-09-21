#pragma once

#include "myd3d.h"
#include "Shader.h"
#include "complex"
using namespace std;

// 流体计算W的最大步骤（包括平流、扩散、外力、重力、浮力、漩涡重建等），
//计算密度场D也可使用该值，但密度场的步骤要少于W的步骤
#define MAX_FLUID_STEPS 6		


// 流体脉冲发射器的形状类型
enum enuInject_Type
{
	Inject_Sphere,				// 圆形
	Inject_Box,					// 盒形
	Inject_Fire,				// 火焰形，模拟火焰用
	Inject_Texture,				// 任意形状发射器，使用贴图
};


// 待处理的贴图长宽和初始化时指定的长宽相同，必须是2的幂，且不能是内存纹理

// 待处理贴图必须保证有4个rgba通道，必须是浮点格式（FP32或FP16任意），每个像素的xyzw必须为如下格式：
//实部绝对值、虚部绝对值、实部符号值（负数为0.0，正数为2.0）、虚部符号值（负数为0.0，正数为2.0），
//这三个条件不会自动判断，如果不符合的话，运算结果会错误
// 输出贴图的像素数据结构也仍然为上面的格式

// FP16在数据贴图中（Scramble/Butterfly）并没有什么问题，但由于精度低且存储范围小，在临时贴图计算过程中可能会丢失部分数据
// 推荐使用RGBA FP32格式！！没什么问题

// 尽量处理分辨率较低（小于256*256）的贴图，高分辨率的FP32会很耗（效率和显存）
class KFourierTransform
{
public:
	KFourierTransform();
	~KFourierTransform() {Release();}

	// 初始化所需的贴图和数据
	HRESULT Init(UINT iWidth, UINT iHeight);
	void Release();

	// 调用乱序和蝶形运算，用GPU进行FFT和IFFT，这里传入的纹理必须保证rgba必须为如下格式：实部绝对值、虚部绝对值、实部符号值（负数为0.0，正数为2.0）、虚部符号值（负数为0.0，正数为2.0）
	// 所以在传入纹理之前就要做相应的数据采样，保证符合这里的要求
	HRESULT FFT(IDirect3DTexture9 *pTexFFTSource) {return CoreProcess(FALSE, pTexFFTSource);}
	HRESULT IFFT(IDirect3DTexture9 *pTexFFTSource) {return CoreProcess(TRUE, pTexFFTSource);}

	// 得到最终的结果，将内部的TexResult复制到指定的贴图中，该贴图必须提前创建好！
	// 注意这里的参数表示：是否乘傅里叶变换中的系数1/N，因为在前面的处理中是不会乘这个的
	HRESULT GetResultData(IDirect3DTexture9 *pDestTexture, BOOL bMulFourierCoef);

	// 得到当前处理好的纹理指针，不复制数据，可以直接保存或设置为纹理待用，不过无法乘傅里叶系数
	// 必须在CPU、GPU的FFT或IFFT之后才可以使用该函数，否则返回空指针
	IDirect3DTexture9* GetResultTexture() {if(m_iCreateAttrib != 2) return NULL; else return m_pTexResult;}


	// 用CPU来算DFT，结果存放在TexResult中。想算IDFT的话，只需要将第二个参数给TRUE即可，第三个参数是乘傅里叶系数
	// CPU计算的都只支持A32R32G32B32F贴图
	HRESULT DFTCPU(IDirect3DTexture9* pTexFFTSource, BOOL bIDFT, BOOL bMulFourierCoef);
	// 用CPU来算FFT，暂时不可用。这里提供的是网上一种顺序输入，再乱序输出的方法，具体见IFFTCPUCore
	HRESULT FFTCPU(IDirect3DTexture9* pTexFFTSource, BOOL bIDFT, BOOL bMulFourierCoef);


	// 存放最终结果的，无论FFT还是IFFT都在这里面
	PIXELSHADER m_PSGetResult;
	IDirect3DTexture9 *m_pTexResult;


	//内部函数
private:
	// 核心处理函数，根据参数分别处理FFT和IFFT
	HRESULT CoreProcess(BOOL bIFFT, IDirect3DTexture9* pTexFFTSource);

	// 初始化用
	HRESULT InitButterflyX(UINT iLitNo);
	HRESULT InitButterflyY(UINT iLitNo);

	// 内部渲染用，参数表示当前的来源图和目的图，不包含自用的特殊贴图(Scramble和Butterfly数据图),
	//因为迭代次数很多，需要用中间贴图来保存结果，这里的参数就是提供一个切换过程
	HRESULT ScrambleX(IDirect3DTexture9* pSource, IDirect3DTexture9* pRT);
	HRESULT ScrambleY(IDirect3DTexture9* pSource, IDirect3DTexture9* pRT);
	HRESULT ButterflyX(UINT iLitNo, BOOL bIFFT, IDirect3DTexture9* pSource, IDirect3DTexture9* pRT);	// 第一个参数表示当前调用的迭代次数，第二个参数表示是FFT还是IFFT
	HRESULT ButterflyY(UINT iLitNo, BOOL bIFFT, IDirect3DTexture9* pSource, IDirect3DTexture9* pRT);

	// 内部使用，1D FFT运算核心，参数1表示时域输入，参数2表示频域输出，r表示迭代次数，内部动态调用
	void IFFTCPUCore(complex<double> * TD, complex<double> * FD, int r);



	//内部变量
private:
	UINT m_iCreateAttrib;	// 1表示已初始化过，2表示已运算过，这样才能得到运算数据TexResult
	UINT m_iWidth, m_iHeight;	// 总长宽，初始化时确定
	UINT m_iButterflyNumX, m_iButterflyNumY;		// Butterfly迭代次数，等于log2(Width或Height)

	// 新深度缓冲，做FFT时使用
	LPDIRECT3DSURFACE9 m_pDepthBuffer;

	// Quad用
	struct QUADVERTEXTYPE
	{
		float x,y,z;
		float tu,tv,tw,tx;	//后两个未用，只是为了去掉PS编译的错误
	};
	VERTEXSHADER m_VSDrawQuad;
	UINT m_iStride;
	IDirect3DVertexBuffer9* m_pVB;



	// 存放中间临时运算结果的贴图，和最终结果完全一致
	IDirect3DTexture9* m_pTexTempUse[2];


	// 存放Scramble数据的
	PIXELSHADER m_PSScrambleX, m_PSScrambleY;
	IDirect3DTexture9* m_pTexScrambleX;	// 结构：SwapCoordX, 1.0f
	IDirect3DTexture9* m_pTexScrambleY;	// 结构：1.0f, SwapCoordY

	// 存放Butterfly数据的
	PIXELSHADER m_PSIFFTButterflyX, m_PSFFTButterflyX;
	PIXELSHADER m_PSIFFTButterflyY, m_PSFFTButterflyY;
	IDirect3DTexture9 **m_ppTexButterflyX, **m_ppTexButterflyY;
	IDirect3DTexture9 **m_ppTexButterflyX_Sign, **m_ppTexButterflyY_Sign;	// 符号贴图，x分量存储复数结合方式（加减的符号值而已），zw分量存储W的符号值
};

struct FLUID_ATTRIBUTE
{
	// 密度系数，默认为1，密度越大越不容易引起：浓度高低部分，在力作用下，运动的两极分化（互溶性越好），密度越高越像液体，不过不能太高，否则会模拟错误（Project时用）
	float fConsistensy;			
	BOOL bLiquid;		// 是否液体，如果是液体就要计算法线，默认为气体
	// 流体重力，可以为任意方向，它和流体密度场有关，密度越大，重力越大，而且是持续作用的，并非脉冲注入
	D3DXVECTOR2 VecGravity;		
	float fGravityPower;		// 重力的乘方幂（正数），值越大表示重的越重，轻的越轻，值越小表示两极分化越小。为1时表示无作用。在液体中一般小于1，否则会出现不真实的情况
	UINT iIterateNum;	// Jacobi迭代次数
	float fVorticityRefinementCoef;	// 黏度很低的流体上，重建漩涡状态，该值为公式中的epsilon，如果为0就表示不重建漩涡
	float fViscousCoef;	// 黏度系数，如果为0就表示不计算黏性扩散
	float fDensityDampCoef;		// 浓度消散系数，其值必须小于等于1而且不可以太低，否则就显示不出来了
	FLUID_ATTRIBUTE()
	{
		fConsistensy = 1.0f;
		bLiquid = FALSE;
		VecGravity = D3DXVECTOR2(0, 0);
		fGravityPower = 1.0f;
		iIterateNum = 40;	// 尽量在40以上
		fVorticityRefinementCoef = 0.0f;
		fViscousCoef = 0.0f;
		fDensityDampCoef = 1.0f;
	}
	
	BOOL CheckAvailable()
	{
		if(iIterateNum < 10)
		{
			OutputDebugString("迭代次数太低！会造成严重的精度丢失！\n");
			return FALSE;
		}
		if(fConsistensy < 0.01f || fConsistensy > 4.0f)
		{
			OutputDebugString("密度太高或太低，不合法！\n");
			return FALSE;
		}
		if(fViscousCoef < 0.0f)
		{
			OutputDebugString("黏度系数不能为负数！\n");
			return FALSE;
		}
		if(fGravityPower < 0.0f)
		{
			OutputDebugString("重力的幂不能为负数！\n");
			return FALSE;
		}
		if(fDensityDampCoef < 0.5f)
		{
			OutputDebugString("密度消散系数太低！会导致模拟结果错误！\n");
			return FALSE;
		}
		if(fDensityDampCoef > 1.0f)
		{
			OutputDebugString("密度消散系数必须小于等于1！\n");
			return FALSE;
		}
		if(fVorticityRefinementCoef < 0.0f)
		{
			OutputDebugString("漩涡重建系数为负数！不合法！\n");
			return FALSE;
		}
		return TRUE;
	}
};


// 注射流体
struct INJECT_ATTRIBUTE
{
	enuInject_Type Type;		// 发射类型

	// 纹理发射器用，指定纹理必须符合以下格式：作为外力，xy必须经过乘加0.5（外力强度的单位是纹理坐标，所以范围肯定在-1~1之间），zw未用。作为来源则无要求，存放颜色即可
	// 尽量使用和Fluid模拟区域同样分辨率的贴图，这样不需要Shader过滤，提高性能，整数和浮点纹理均可
	IDirect3DTexture9* pTexInjectForce, *pTexInjectSource;
	float fTexInjectForceCoef, fTexInjectSourceCoef;		// 考虑到纹理存放值有范围限制，这里再给定一个发射强度的系数，值可为任意（作用于每个Texel）。0表示暂时取消发射。对外力而言，-1表示纹理值的方向取反

	// 程序发射器用
	UINT iRange;				// 作用范围（圆形半径），单位像素
	D3DXVECTOR2 PtPos;			// 位置点（记得是像素坐标）
	
	// 下面两个的值都和注入强度有关，由于注入是脉冲，所以这里的强度表示连续注入一秒所达到的强度，时间由流体引擎来控制
	// 比如说若VecForceDir长度为0.5，就表示每秒注入的外力强度为0.5，通过平流，则每秒外力将移动0.5个流体区域（即半个流体区域）。
	D3DXVECTOR2 VecForceDir;			// 外力方向，其长度表示外力强度（外力的强度/长度单位为纹理坐标）。仅外力F有效，来源S无效
	D3DXVECTOR3 VecCentreIntensity;		// 中心点强度（即浓度范围0～1），以该强度为准逐渐向外圈衰减，衰减到作用范围外为0。之所以用Vector3是将RGB分开处理。仅来源S有效，外力F无效

	INJECT_ATTRIBUTE()
	{
		Type = Inject_Sphere;
		pTexInjectForce = pTexInjectSource = NULL;
		fTexInjectForceCoef = fTexInjectSourceCoef = 1.0f;

		iRange = 0;
		PtPos.x = PtPos.y = 0;
		VecForceDir.x = VecForceDir.y = 0.0f;
		VecCentreIntensity.x = VecCentreIntensity.y = VecCentreIntensity.z = 1.0f;
	}
	
	// 参数表示总范围
	BOOL CheckAvailable(UINT iWidth, UINT iHeight)
	{
		if(Type == Inject_Texture)
		{
			if(!pTexInjectForce || !pTexInjectSource)
			{
				OutputDebugString("纹理类型的注射器没有指定纹理！\n");
				return FALSE;
			}
		}
		else
		{
			if(!iRange || iRange > iWidth || iRange >= iHeight)
			{
				OutputDebugString("范围数据非法！\n");
				return FALSE;
			}
			if(PtPos.x < 0.0f || PtPos.x >= (float)iWidth || PtPos.y < 0.0f || PtPos.y >= (float)iHeight)
			{
				OutputDebugString("位置点坐标非法！\n");
				return FALSE;
			}
			// 暂时不在这里判断外力和来源强度，在Set中判断
		}

		return TRUE;
	}
};


// 向量/标量场
enum enuFluid_VectorField
{
	FLUID_VELOCITY,		// U
	FLUID_PRESSURE,		// P
	FLUID_DENSITY,		// D
	FLUID_W,			// W
};





// 只需SM2.0，动态计算任意边界需要SM2.a
class KFluidSimulation2D
{
public:
	KFluidSimulation2D();
	~KFluidSimulation2D() {Release();}
	void Release();

	// 初始化Fluid模拟所需的顶点缓冲和贴图数据
	// 参数表示模拟网格的分辨率（任意），可以最终拉伸绘制到屏幕
	HRESULT Init(UINT iWidth, UINT iHeight, BOOL bGenerateNormal = TRUE);

	// 重设为无流体状态，即速度场、压力场、密度场清零
	HRESULT ResetFluid();


	// 开始模拟
	// 要记录下上一次的时间，根据当前时间使用DeltaTime来计算模拟过程
	HRESULT FluidSimulation(FLUID_ATTRIBUTE FluidData, float fDeltaTime);

	// 设置阻挡图，必须和流体模拟的分辨率相同，其中可通过部分为1，阻挡部分为0
	// 函数将阻挡图转换成ABC Offset图，并置ABC计算标记为true
	// 如果TexObstacle为，则ABC计算标记置为false，用它来禁止任意边界的计算
	// TexOffset是8888格式，且不能为内存纹理。它的作用是当显卡不支持sm2.x时，可以设置Obstacle对应的（提前计算好的）Offset贴图，这样在SM2.0的显卡上也可以实现任意边界
	// 可以连续设置同一张Obstacle图，程序会自动优化，只在第一次设置时计算Offset图
	HRESULT SetObstacleTexture(IDirect3DTexture9* pTexObstacle, IDirect3DTexture9* pTexOffset = NULL);

	// 设置瞬时外力和来源，先暂时保存数据，然后在模拟过程中直接作用在速度场U和密度场D上
	HRESULT SetForce(UINT iForceNum, INJECT_ATTRIBUTE* pInjectData);
	HRESULT SetSource(UINT iSourceNum, INJECT_ATTRIBUTE* pInjectData);


	// Get，必须先Simulate才行，只是得到指针，并不复制数据
	IDirect3DTexture9* GetDensityMap()
	{
		if(m_iCreateAttrib != 2)
			return NULL;
		return m_pTexD;
	}
	IDirect3DTexture9* GetNormalMap() 
	{
		if(m_iCreateAttrib != 2 || !m_bGenerateNormal)
			return NULL;
		return m_pTexNormal;
	}


// Simulation时内部使用的：大步骤
private:
	// 平流，对P无效
	HRESULT Advect(enuFluid_VectorField Type);
	// 扩散，对P无效，如有需要才计算（Viscous大于0时）
	HRESULT Diffusion(enuFluid_VectorField Type);
	// 根据设置好的Data加到外力上
	HRESULT ApplyForce();
	// 根据密度设置重力
	HRESULT ApplyGravity();
	// 根据设置好的Data加到来源上
	HRESULT ApplySource();


	// 根据已算好的W计算P，中间包括计算W散度及Jacobi迭代
	HRESULT SolvePoissonPressureEquation();

	// 根据已算好的P计算U，即投影
	HRESULT Project();

	// 根据需要重建漩涡（Vorticity大于0时）
	HRESULT RefineVorticity();

	// 边界条件（包括基本边界和任意边界），参数表示更新哪种场的边界，对三个场都有效，pTexture表示需要被更新的纹理指针，注意是原地更新，输入=输出（里面已经自动进行了CopyToTemp的操作，不用担心会出错）
	HRESULT ApplyBoundaryCondition(enuFluid_VectorField Type, IDirect3DTexture9* pTexture);

	// NormalMap
	HRESULT GenerateNormal();


	
// 模拟时内部使用的：公共步骤
private:
	// 根据障碍图，更新ABC偏移坐标贴图，它仅由SetObstacle函数调用
	HRESULT UpdateABCTexture(IDirect3DTexture9* pTexObstacle);

	// 复制纹理，其实就是一个简单的Copy CommonComputeQuad调用，专门提出来
	HRESULT CopyTexture(IDirect3DTexture9* pTexSrc, IDirect3DTexture9* pTexDst)
	{
		return CommonComputeQuad(pTexSrc, NULL, NULL, pTexDst, &m_PSCopy);	
	}

	// 清除纹理，每个像素清为指定颜色，默认为0
	HRESULT ClearTexture(IDirect3DTexture9* pTexture, LPD3DXVECTOR4 pColor = NULL)
	{
		D3DXVECTOR4 VecColor = D3DXVECTOR4(0, 0, 0, 0);
		if(pColor)
			VecColor = *pColor;
		V_RETURN(d3ddevice->SetPixelShaderConstantF(5, (float *)&VecColor, 1));
		return CommonComputeQuad(NULL, NULL, NULL, pTexture, &m_PSClearTexture);
	}

	// 根据类型选择Inject Shader
	PIXELSHADER *ChooseForceShader(enuInject_Type Type);
	PIXELSHADER *ChooseSourceShader(enuInject_Type Type);


	// 公用Pixel Shader计算步骤，必须提前将常量寄存器设置好
	// 如果用到的贴图只有一层，那么第二、三个参数置NULL
	// 这个是画Quad
	HRESULT CommonComputeQuad(IDirect3DTexture9* pSrcTex1, IDirect3DTexture9* pSrcTex2, IDirect3DTexture9* pSrcTex3, IDirect3DTexture9* pRT, PIXELSHADER *pPS);

	// 参数同上，这个是画Line，对应m_pVBBoundaryLine[iLineNo]，并用D3DPT_LINELSIT
	HRESULT CommonComputeLine(IDirect3DTexture9* pSrcTex1, IDirect3DTexture9* pSrcTex2, IDirect3DTexture9* pSrcTex3, IDirect3DTexture9* pRT, PIXELSHADER *pPS, UINT iLineNo);


// 内部使用变量
public:
	UINT m_iCreateAttrib;
	UINT m_iWidth, m_iHeight;
	BOOL m_bGenerateNormal, m_bABC;	// 是否用GPU生成法线图，是否计算ABC

	float m_fDeltaTime;
	UINT m_iRenderNumPerFrame;		// 当前帧渲染多少次，即调用多少次DrawQuad，调用完FluidSimulation后就可以从它知道渲染负担了

	FLUID_ATTRIBUTE m_FluidData;

	// 瞬时注射数据
	UINT m_iInjectForceNum, m_iInjectSourceNum;
	INJECT_ATTRIBUTE *m_pInjectForce, *m_pInjectSource;

		
		
	// 顶点缓冲
	struct VERTEXTYPE
	{
		float x,y,z,w;		// XYZRHW
		float tu,tv,tw,tx;	// 纹理坐标：小数，后两个未用，只是为了去掉PS编译的错误
		float tu_int,tv_int,tw_int,tx_int;	// 同上，纹理坐标整数 for Bilinear Filtering
	};
	IDirect3DVertexDeclaration9 *m_pDeclaration;
	UINT m_iStride;
	IDirect3DVertexBuffer9* m_pVBQuad, *m_pVBQuadExceptBoundary; // 一个大矩形，和一个不包括边界线的矩形（模拟时用）
	IDirect3DVertexBuffer9* m_pVBBoundaryLine[4];	// 四边的线，用于更新基本边界，四边0～3分别为上下左右
	LPDIRECT3DSURFACE9 m_pDepthBuffer;



	
	
// 贴图部分，向量场都是XY存放绝对值，ZW存放符号（0、2分别表示负数正数），Normal/ABC Offset图，都是乘加0.5，把-1～1转换为0～1存储
public:
	IDirect3DTexture9* m_pTexWTemp[MAX_FLUID_STEPS], *m_pTexDTemp[MAX_FLUID_STEPS];	// 临时使用，按步骤计算W和密度场
	UINT m_iCurrentWIndex, m_iCurrentDIndex;		// 因为计算步骤数不定（根据用户设定不同），所以总是让这两个值代表当前计算好的W和D在上面的哪张贴图中，在计算完所有步骤之后，它们也代表最终数据存放的贴图序号。

	IDirect3DTexture9* m_pTexTemp[2];	// 用于临时切换循环使用，比如循环计算Jacobi迭代、循环叠加FS等
	IDirect3DTexture9* m_pTexBCTemp;	// 用于临时计算边界条件，BC和其他操作可能会混合在一起，所以要单独给一张贴图
	IDirect3DTexture9* m_pTexP, *m_pTexD, *m_pTexU;	// 算好的：压力场、密度场、速度场U，记住要在最后拷贝TexDTemp到TexD中


	// 任意边界条件（Arbitary Boundary Conditions）用
	IDirect3DTexture9* m_pTexABCTypeToOffset;	// 转换图，存放不同的类型对应的临近偏移点坐标数据，初始化一次即可。临时使用，用于计算偏移坐标图
	IDirect3DTexture9* m_pTexABCType;			// ABC类型图，存放计算好的各个边界点开口类型。临时使用，用于计算偏移坐标图
	IDirect3DTexture9* m_pTexABCBoundaryToOffset;	// 转换图，存放不同Boundary相加之和对应的临近偏移点坐标数据，初始化一次即可。临时使用，用于计算偏移坐标图
	
	IDirect3DTexture9* m_pTexABCBoundary;		// 障碍类型图，存放边界点、障碍点或是非边界点
	IDirect3DTexture9* m_pTexABCOffset;			// ABC每个边界点的偏移，xy和zw分别存放两个临近点的偏移坐标，范围修正为0～1，这里临近点和法向点相同，所以U和P可以公用

	// 临时使用，计算W的散度（压力场用）、旋度（漩涡重建用）
	IDirect3DTexture9* m_pTexDivW, *m_pTexCurlW;

	// 法线范围修正到0～1之间，不用加Sign修正
	IDirect3DTexture9* m_pTexNormal;



// Pixel Shader部分
private:
	// MISC
	PIXELSHADER m_PSClearTexture;						// 重置为指定颜色（默认0）,可以用于Reset，也可以用于浓度场D的基本边界条件
	PIXELSHADER m_PSCopy;							// 纹理复制
	PIXELSHADER m_PSGenerateNormal;					// Normal
	
	// Inject
	PIXELSHADER m_PSAddForceSphere, m_PSAddSourceSphere;		// 高斯圆形注射器
	PIXELSHADER m_PSAddForceTexture, m_PSAddSourceTexture;		// 纹理注射器
	PIXELSHADER m_PSAddForceFire, m_PSAddSourceFire;			// 火焰形注射器

	// NSEs
	PIXELSHADER m_PSAdvect_W, m_PSAdvect_D;			// Advect
	PIXELSHADER m_PSGravity;						// Gravity in Liquid
	PIXELSHADER m_PSDiv_W, m_PSProject;				// Div.W, U=W-Grad.P
	PIXELSHADER m_PSJacobi_W, m_PSJacobi_D, m_PSJacobi_P;	// Jacobi Iteration，分别用于处理Diffusion和Poisson Pressure
	PIXELSHADER m_PSCurl_W, m_PSVorticity;			// Vorticity Refinement
	
	// BCs
	PIXELSHADER m_PSBBC_U, m_PSBBC_P;				// 基本边界条件（Basic Boundary Condition），注意BC_U可以同时适用于U和W
	PIXELSHADER m_PSABC_U, m_PSABC_P, m_PSABC_D;	// 任意边界条件（Arbitary Boundary Condition）
			// Update ABC Offset Texture用
	PIXELSHADER m_PSObstacleToBoundary;				// 计算ABC偏移图的第一步：障碍图转换为边界图
	PIXELSHADER m_PSBoundaryToOffset;				// 方法2：边界图直接转换为偏移图（需要76条指令，ps2.x）
	PIXELSHADER m_PSBoundaryToType, m_PSTypeToOffset;	// 方法1：边界图先到开口类型图，然后再转换为偏移图（Pass多，而且需要ps2.a）
};
