#pragma once

#include "myd3d.h"
#include "GPGPU.h"
#include "Shader.h"


#define DEEP_OCEAN 0.0f			// 波浪类型，深海
#define SWALLOW_WATER -1.0f		// 波浪类型，浅水




// 海浪所需的参数
struct OCEANWATER_ATTRIBUTE
{
	float fOceanDepth;				// 水深，默认为0，即DEEP_OCEAN
	UINT iWidth, iHeight;			// 空间周期性，即分割格子数
	D3DXVECTOR2 WaterSquare;		// 水面总长宽，世界坐标
	D3DXVECTOR2 VecWindDir;			// 风向，规格化向量
	float fWindSpeed;				// 风速，单位以世界空间为准

	float fWaveHeightScale;			// 波浪高度全局缩放，用于总控波浪高度，只有它可以动态改变，不过在FFTOceanWater类中忽略该项，自己放在VS中进行缩放

	OCEANWATER_ATTRIBUTE()
	{
		fOceanDepth = DEEP_OCEAN;
		iWidth = iHeight = 0;
		WaterSquare = D3DXVECTOR2(0, 0);
		VecWindDir = D3DXVECTOR2(0, 0);
		fWindSpeed = 0.0f;
		fWaveHeightScale = 1.0f;
	}

	// 检验数据有效性
	BOOL CheckAvaliable()
	{
		if(fOceanDepth < 0.0f && fOceanDepth != SWALLOW_WATER)
		{
			OutputDebugString("海浪深度数据无效！\n");
			return FALSE;
		}
		if(fOceanDepth != DEEP_OCEAN)
		{
			OutputDebugString("暂时只支持深海的水面模拟！\n");
			return FALSE;
		}
		// 必须是2的幂，0和1同时被过滤
		if(!CheckPowerOf2(iWidth) || !CheckPowerOf2(iHeight))
		{
			OutputDebugString("分割格子数不符合要求！必须为2的幂\n");
			return FALSE;
		}
		if(WaterSquare.x < 0.000001f || WaterSquare.y < 0.000001f)
		{
			OutputDebugString("海水面积数据无效！\n");
			return FALSE;
		}
		if(absf(D3DXVec2Length(&VecWindDir) - 1.0f) > 0.00001f)
		{
			OutputDebugString("风向不为规格化向量！\n");
			return FALSE;
		}
		if(fWindSpeed < 0.000001f)
		{
			OutputDebugString("风速数据无效！\n");
			return FALSE;
		}
		if(fWaveHeightScale < 0.000001f)
		{
			OutputDebugString("波浪整体缩放数据无效！\n");
			return FALSE;
		}

		return TRUE;
	}
};




// 这里无论设置FP多少，最终供给外界使用（VTF or Lock）的贴图都是FP32，不用担心
class KFFTOceanWater
{
public:
	KFFTOceanWater();
	~KFFTOceanWater() {Release();}

	// 初始化所需的贴图（h0）和数据，传入的参数代表分割网格数，必须为2的幂
	HRESULT Init(OCEANWATER_ATTRIBUTE OceanData, BOOL bGenerateNormal = TRUE, BOOL bGenerateTangent = TRUE);
	void Release();

	// 做海浪拟真（先用h0图生成h图，然后做IFFT），将结果存储到一张高度图中，即FFT中的TexResult
	// 可以加一张高度图，用于影响海面某些区域或某些点的高度（如形成外力划过的波纹、涟漪等），法线、切线等也会被影响
	// 附加高度图的xy分量被用到来影响海面高度和横轴（YX），zw分量保存符号（0或2）
	// 它是由刚计算好的IFFT结果，叠加后保存到m_pTexHeight中
	HRESULT WaterSimulation(float fTime, LPDIRECT3DTEXTURE9 pTexAddonHeightMap = NULL);

	// 设置新的海浪数据，会重新初始化，不要每帧都调用
	// 波浪数据中没有一项可以每帧动态改变的而能沿用原有数据的，所以只要设置就必须重新初始化
	HRESULT SetNewOceanData(OCEANWATER_ATTRIBUTE OceanData)
	{
		if(!OceanData.CheckAvaliable())
			return D3DERR_INVALIDCALL;
		Release();
		V_RETURN(Init(OceanData));
		return S_OK;
	}


	// 得到当前处理好的高度图、法线和切线图纹理的指针，不复制数据，可以直接保存或设置为纹理待用，不过无法乘傅里叶系数
	// 必须在执行WaterSimulation之后才可以得到
	// 得到了法线N和切线T图，副法线B = T×B
	LPDIRECT3DTEXTURE9 GetHeightMap() 
	{
		if(m_iCreateAttrib != 2)
			return NULL;
		return m_pTexHeight;
	}
	LPDIRECT3DTEXTURE9 GetNormalMap() 
	{
		if(m_iCreateAttrib != 2 || !m_bGenerateNormal)
			return NULL;
		return m_pTexNormal;
	}
	LPDIRECT3DTEXTURE9 GetTangentMap() 
	{
		if(m_iCreateAttrib != 2 || !m_bGenerateNormal)
			return NULL;
		return m_pTexTangent;
	}

	//内部函数
private:
	// 从h(0)根据参数和当前时间生成h，用GPU
	HRESULT GenerateH(float fTime);

	// 叠加附加高度图
	HRESULT AddHeightMap(LPDIRECT3DTEXTURE9 pTexAddonHeightMap);

	// 通过模拟好的高度图生成法线和切线
	HRESULT GenerateNormal();
	HRESULT GenerateTangent();
	HRESULT GenerateNormalandTangent();		// 用于MRT，加速运算

	// 初始化h(0)
	HRESULT InitH0();


	//内部函数
private:
	// 计算h0(K)或h0(-K)
	D3DXVECTOR2 CalcH0(int iX, int iY);

	//内部变量
private:
	BOOL m_bGenerateNormal;	// 是否用GPU生成法线和切线图（必须支持PS2.a）
	BOOL m_bGenerateTangent;

	UINT m_iCreateAttrib;	// 1表示已初始化过，2表示已运算过，这样才能得到运算数据TexResult
	OCEANWATER_ATTRIBUTE m_OceanData;	// 用户指定的海浪数据

	// 渲染h时用的Quad
	struct QUADVERTEXTYPE
	{
		float x,y,z;
		float tu,tv,tw,tx;	//后两个未用，只是为了去掉PS编译的错误
	};
	VERTEXSHADER m_VSDrawQuad;
	UINT m_iStride;
	LPDIRECT3DVERTEXBUFFER9 m_pVB;

	// 渲染h时用
	LPDIRECT3DSURFACE9 m_pDepthBuffer;
	PIXELSHADER m_PSGenerateH;

	// 叠加附加高度图
	PIXELSHADER m_PSAddHeightMap;

	// 最终计算好的高度图
	LPDIRECT3DTEXTURE9 m_pTexHeight;

	// 法线、切线图
	LPDIRECT3DTEXTURE9 m_pTexNormal, m_pTexTangent;
	PIXELSHADER m_PSGenerateNormal, m_PSGenerateTangent, m_PSGenerateNormalandTangent;


	// 存放h(0)和h，都是4通道的浮点贴图，要存复数，格式同FFT：实虚部绝对值+实虚部符号（0或2）
	LPDIRECT3DTEXTURE9 m_pTexH;
	LPDIRECT3DTEXTURE9 m_ppTexH0[2];

	// 做IFFT
	KFourierTransform m_IFFT;
};















// 波动方程所需的属性
struct WAVEEQUATION_ATTRIBUTE
{
	UINT iWidth, iHeight;		// 分割格子数
	float fDampCoef;			// 波动的阻尼系数，用于运动学Verlet积分
	float fWaveSpeed;			// 波动方程中加速度的系数C^2，可以作为波速进行控制，为0时表示暂时屏蔽波动
	D3DXVECTOR2 WaterSquare;	// 水域大小（世界坐标），用于计算法/切线
	char szAreaDampTexture[100];// 加速度衰减贴图，用于强制消除某些区域的波动（一般是波动），可选，加速度系数在alpha通道中存储

	WAVEEQUATION_ATTRIBUTE()
	{
		iWidth = iHeight = 0;
		fWaveSpeed = fDampCoef = 1.0f;
		WaterSquare = D3DXVECTOR2(0.0f, 0.0f);
		ZeroMemory(szAreaDampTexture, 100);
	}

	// 检验数据有效性
	BOOL CheckAvaliable()
	{
		// 必须是2的幂，0和1同时被过滤
		if(!iWidth || !iHeight)
		{
			OutputDebugString("分割格子数不可为0！\n");
			return FALSE;
		}
		if(fDampCoef < 0.000001f)
		{
			OutputDebugString("阻尼系数无效！\n");
			return FALSE;
		}
		if(WaterSquare.x < 0.000001f || WaterSquare.y < 0.000001f)
		{
			OutputDebugString("水域面积数据无效！\n");
			return FALSE;
		}


		return TRUE;
	}
};





// 需要SM2.0来生成和模拟波动，需要SM2.x来生成法线/切线图
// 这里无论设置FP多少，最终供给外界使用（VTF or Lock）的贴图都是FP32，不用担心
class KWaveEquationWater
{
public:
	KWaveEquationWater();
	~KWaveEquationWater() {Release();}

	// 初始化所需的贴图和数据，传入的参数包括DampingTexture和分割网格数
	// DampingTexture是在计算加速度时给定的贴图，使加速度在边界处消失
	HRESULT Init(WAVEEQUATION_ATTRIBUTE WaveData, BOOL bGenerateNormal = FALSE, BOOL bGenerateTangent = FALSE);
	void Release();

	// 解波动方程，根据当前设置的某些点的值，进行模拟，并将结果存储到一张高度图中
	// 可以将这个高度图加到FFTWATER里面，形成外力划过的波纹、涟漪等，同时法线、切线等也会被影响
	// xy分量被用到来影响海面高度和横轴（YX），zw分量保存符号（0或2）
	// DeltaTime最好用固定步进，不要用真实时间，通过跳过执行模拟函数的方式来达到高帧数下的速度平衡
	// 考虑到数值稳定的Damp系数，模拟水面无波动时的基准高度（y值）必须为0，如果水面有高度平移，还是用模拟时高度为0 + 平移后再渲染的方式来处理
	HRESULT WaterSimulation(float fDeltaTime);


	// 重设波动
	HRESULT ResetWave();

	// 设置波速，由于时间步进一定，那么可以适应不同帧数的变量，就只有波速一个了
	void SetWaveSpeed(float fSpeed)
	{
		m_WaveData.fWaveSpeed = fSpeed;
	}


	// 设置某些点/区域的高度值，只有通过这个函数，才能打破平衡态，引起波动
	// 由于该函数调用较频繁，为了提速，不使用Lock，而是用Render Point/Rect Primitive To Texture的方法
	// bAddHeight的意思是：将当前高度叠加上去而不是强制用设定值覆盖原高度
	// 点图元中：可以同时设置多个点/区域，X/Y表示中心点坐标（像素整数坐标），pHeight表示高度值
	// 矩形图元中：同时只能处理一个，X/Y表示中心点坐标（像素整数坐标），Range表示矩形长宽（纹理坐标），TexArea表示区域高度纹理（xz通道分别保存高度的绝对值和符号），它才是真正区域中每个像素的高度值，pHeight只作为附加系数
	HRESULT SetPointHeight( UINT iPointNum, UINT *pX, UINT *pY, float *pHeight, BOOL bAddHeight );
	HRESULT SetAreaHeight( UINT iX, UINT iY, D3DXVECTOR2 VecRange, LPDIRECT3DTEXTURE9 pTexArea, float fHeight, BOOL bAddHeight );



	// 设置阻挡图，必须和波动模拟的分辨率相同，其中可通过部分为1，阻挡部分为0
	// 函数将阻挡图转换成ABC Offset图，并置ABC计算标记为true
	// 如果TexObstacle为，则ABC计算标记置为false，用它来禁止任意边界的计算
	// TexOffset是8888格式，且不能为内存纹理。它的作用是当显卡不支持sm2.x时，可以设置Obstacle对应的（提前计算好的）Offset贴图，这样在SM2.0的显卡上也可以实现任意边界
	// 可以连续设置同一张Obstacle图，程序会自动优化，只在第一次设置时计算Offset图
	HRESULT SetObstacleTexture(LPDIRECT3DTEXTURE9 pTexObstacle, LPDIRECT3DTEXTURE9 pTexOffset = NULL);


	// 设置新的波动数据，会重新初始化，不要每帧都调用
	// 只要设置就必须重新初始化
	HRESULT SetNewWaveData(WAVEEQUATION_ATTRIBUTE WaveData)
	{
		if(!WaveData.CheckAvaliable())
			return D3DERR_INVALIDCALL;
		Release();
		V_RETURN(Init(WaveData, m_bGenerateNormal, m_bGenerateTangent));
		return S_OK;
	}


	// 得到当前处理好的高度图、法线和切线图纹理的指针，不复制数据，可以直接保存或设置为纹理待用，不过无法乘傅里叶系数
	// 必须在执行WaterSimulation之后才可以得到
	// 得到了法线N和切线T图，副法线B = T×B
	LPDIRECT3DTEXTURE9 GetHeightMap() 
	{
		if(m_iCreateAttrib != 2)
			return NULL;
		return m_pTexHeight;
	}
	LPDIRECT3DTEXTURE9 GetNormalMap() 
	{
		if(m_iCreateAttrib != 2 || !m_bGenerateNormal)
			return NULL;
		return m_pTexNormal;
	}
	LPDIRECT3DTEXTURE9 GetTangentMap() 
	{
		if(m_iCreateAttrib != 2 || !m_bGenerateTangent)
			return NULL;
		return m_pTexTangent;
	}



	//内部函数
private:
	// 运算核，一个是按Quad画（最后一个参数表示是否用用户自定的矩形区域，仅由SetAreaHight调用），一个是按Point画
	HRESULT CommonComputeQuad(LPDIRECT3DTEXTURE9 pSrcTex1, LPDIRECT3DTEXTURE9 pSrcTex2, LPDIRECT3DTEXTURE9 pSrcTex3, LPDIRECT3DTEXTURE9 pRT1, LPDIRECT3DTEXTURE9 pRT2, PIXELSHADER *pPS, BOOL bUserQuad = FALSE);
	HRESULT CommonComputePoint(UINT iPointNum, LPDIRECT3DTEXTURE9 pSrcTex1, LPDIRECT3DTEXTURE9 pSrcTex2, LPDIRECT3DTEXTURE9 pSrcTex3, LPDIRECT3DTEXTURE9 pRT, PIXELSHADER *pPS);

	// 复制纹理，其实就是一个简单的Copy CommonComputeQuad调用，专门提出来
	HRESULT CopyTexture(LPDIRECT3DTEXTURE9 pTexDst, LPDIRECT3DTEXTURE9 pTexSrc)
	{
		return CommonComputeQuad(pTexSrc, NULL, NULL, pTexDst, NULL, &m_PSCopyTexture);
	}

	// 清除纹理，每个像素清为指定颜色，默认为0
	HRESULT ClearTexture(LPDIRECT3DTEXTURE9 pTexture, LPD3DXVECTOR4 pColor = NULL)
	{
		D3DXVECTOR4 VecColor = D3DXVECTOR4(0, 0, 0, 0);
		if(pColor)
			VecColor = *pColor;
		V_RETURN(d3ddevice->SetPixelShaderConstantF(5, (float *)&VecColor, 1));
		return CommonComputeQuad(NULL, NULL, NULL, pTexture, NULL, &m_PSClearTexture);
	}


	//内部变量
private:
	BOOL m_bABC;			// 是否计算任意边界
	BOOL m_bGenerateNormal;	// 是否用GPU生成法线和切线图（必须支持PS2.a）
	BOOL m_bGenerateTangent;

	float m_fDeltaTime;
	UINT m_iCreateAttrib;	// 1表示已初始化过，2表示已运算过，这样才能得到运算数据TexResult
	WAVEEQUATION_ATTRIBUTE m_WaveData;	// 用户指定的波动数据

	// 顶点缓冲
	struct VERTEXTYPE
	{
		float x,y,z,w;		// XYZRHW
		float tu,tv,tw,tx;	// 纹理坐标：小数，后两个未用，只是为了去掉PS编译的错误
		float tu_int,tv_int,tw_int,tx_int;	// 同上，纹理坐标整数 for Bilinear Filtering
	};
	LPDIRECT3DVERTEXDECLARATION9 m_pDeclaration;
	UINT m_iStride;
	LPDIRECT3DVERTEXBUFFER9 m_pVBQuad, m_pVBQuadUser;	// 默认的大矩形，和一个可由用户指定顶点坐标的矩形，用于纹理式注入
	LPDIRECT3DVERTEXBUFFER9 m_pVBPoint;					// 点图元，用于设置指定点的高度值
	LPDIRECT3DSURFACE9 m_pDepthBuffer;


	// MISCのPixel Shader
	PIXELSHADER m_PSCopyTexture, m_PSClearTexture;

	// 设置或叠加指定点/矩形区域的高度
	PIXELSHADER m_PSSetPointHeight, m_PSSetAreaHeight;
	PIXELSHADER m_PSAddPointHeight, m_PSAddAreaHeight;

	// 临时使用的贴图
	LPDIRECT3DTEXTURE9 m_pTexTemp;

	// 任意边界条件（Arbitary Boundary Conditions）用
		// Update ABC Offset Texture用
	LPDIRECT3DTEXTURE9 m_pTexABCBoundaryToOffset;	// 转换图，存放不同Boundary相加之和对应的临近偏移点坐标数据，初始化一次即可。临时使用，用于计算偏移坐标图
	PIXELSHADER m_PSObstacleToBoundary;				// 计算ABC偏移图的第一步：障碍图转换为边界图
	PIXELSHADER m_PSBoundaryToOffset;				// 方法2：边界图直接转换为偏移图（需要76条指令，ps2.x）

		// 更新任意边界条件用
	LPDIRECT3DTEXTURE9 m_pTexABCBoundary;		// 障碍类型图，存放边界点、障碍点或是非边界点
	LPDIRECT3DTEXTURE9 m_pTexABCOffset;			// ABC每个边界点的偏移，xy和zw分别存放两个临近点的偏移坐标，范围修正为0～1，这里临近点和法向点相同，所以U和P可以公用
	PIXELSHADER m_PSABC_Bounce;					// 更新任意边界条件


	// 最终计算好的高度图及Verlet积分中使用的
	LPDIRECT3DTEXTURE9 m_pTexHeight, m_pTexPrev, m_pTexNow;
	LPDIRECT3DTEXTURE9 m_pTexAreaDamping;	// 用户指定（可选），用于使波动消失在某些区域（主要用于边界处）
	PIXELSHADER m_PSWaveEquation, m_PSWaveEquationWithDampTexture;

	// 法线、切线图
	LPDIRECT3DTEXTURE9 m_pTexNormal, m_pTexTangent;
	PIXELSHADER m_PSGenerateNormal, m_PSGenerateTangent, m_PSGenerateNormalandTangent;
};
