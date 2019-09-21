#pragma once

#include "myd3d.h"
#include "Shader.h"
#include "complex"
using namespace std;

// �������W������裨����ƽ������ɢ�������������������������ؽ��ȣ���
//�����ܶȳ�DҲ��ʹ�ø�ֵ�����ܶȳ��Ĳ���Ҫ����W�Ĳ���
#define MAX_FLUID_STEPS 6		


// �������巢��������״����
enum enuInject_Type
{
	Inject_Sphere,				// Բ��
	Inject_Box,					// ����
	Inject_Fire,				// �����Σ�ģ�������
	Inject_Texture,				// ������״��������ʹ����ͼ
};


// ���������ͼ����ͳ�ʼ��ʱָ���ĳ�����ͬ��������2���ݣ��Ҳ������ڴ�����

// ��������ͼ���뱣֤��4��rgbaͨ���������Ǹ����ʽ��FP32��FP16���⣩��ÿ�����ص�xyzw����Ϊ���¸�ʽ��
//ʵ������ֵ���鲿����ֵ��ʵ������ֵ������Ϊ0.0������Ϊ2.0�����鲿����ֵ������Ϊ0.0������Ϊ2.0����
//���������������Զ��жϣ���������ϵĻ��������������
// �����ͼ���������ݽṹҲ��ȻΪ����ĸ�ʽ

// FP16��������ͼ�У�Scramble/Butterfly����û��ʲô���⣬�����ھ��ȵ��Ҵ洢��ΧС������ʱ��ͼ��������п��ܻᶪʧ��������
// �Ƽ�ʹ��RGBA FP32��ʽ����ûʲô����

// ��������ֱ��ʽϵͣ�С��256*256������ͼ���߷ֱ��ʵ�FP32��ܺģ�Ч�ʺ��Դ棩
class KFourierTransform
{
public:
	KFourierTransform();
	~KFourierTransform() {Release();}

	// ��ʼ���������ͼ������
	HRESULT Init(UINT iWidth, UINT iHeight);
	void Release();

	// ��������͵������㣬��GPU����FFT��IFFT�����ﴫ���������뱣֤rgba����Ϊ���¸�ʽ��ʵ������ֵ���鲿����ֵ��ʵ������ֵ������Ϊ0.0������Ϊ2.0�����鲿����ֵ������Ϊ0.0������Ϊ2.0��
	// �����ڴ�������֮ǰ��Ҫ����Ӧ�����ݲ�������֤���������Ҫ��
	HRESULT FFT(IDirect3DTexture9 *pTexFFTSource) {return CoreProcess(FALSE, pTexFFTSource);}
	HRESULT IFFT(IDirect3DTexture9 *pTexFFTSource) {return CoreProcess(TRUE, pTexFFTSource);}

	// �õ����յĽ�������ڲ���TexResult���Ƶ�ָ������ͼ�У�����ͼ������ǰ�����ã�
	// ע������Ĳ�����ʾ���Ƿ�˸���Ҷ�任�е�ϵ��1/N����Ϊ��ǰ��Ĵ������ǲ���������
	HRESULT GetResultData(IDirect3DTexture9 *pDestTexture, BOOL bMulFourierCoef);

	// �õ���ǰ����õ�����ָ�룬���������ݣ�����ֱ�ӱ��������Ϊ������ã������޷��˸���Ҷϵ��
	// ������CPU��GPU��FFT��IFFT֮��ſ���ʹ�øú��������򷵻ؿ�ָ��
	IDirect3DTexture9* GetResultTexture() {if(m_iCreateAttrib != 2) return NULL; else return m_pTexResult;}


	// ��CPU����DFT����������TexResult�С�����IDFT�Ļ���ֻ��Ҫ���ڶ���������TRUE���ɣ������������ǳ˸���Ҷϵ��
	// CPU����Ķ�ֻ֧��A32R32G32B32F��ͼ
	HRESULT DFTCPU(IDirect3DTexture9* pTexFFTSource, BOOL bIDFT, BOOL bMulFourierCoef);
	// ��CPU����FFT����ʱ�����á������ṩ��������һ��˳�����룬����������ķ����������IFFTCPUCore
	HRESULT FFTCPU(IDirect3DTexture9* pTexFFTSource, BOOL bIDFT, BOOL bMulFourierCoef);


	// ������ս���ģ�����FFT����IFFT����������
	PIXELSHADER m_PSGetResult;
	IDirect3DTexture9 *m_pTexResult;


	//�ڲ�����
private:
	// ���Ĵ����������ݲ����ֱ���FFT��IFFT
	HRESULT CoreProcess(BOOL bIFFT, IDirect3DTexture9* pTexFFTSource);

	// ��ʼ����
	HRESULT InitButterflyX(UINT iLitNo);
	HRESULT InitButterflyY(UINT iLitNo);

	// �ڲ���Ⱦ�ã�������ʾ��ǰ����Դͼ��Ŀ��ͼ�����������õ�������ͼ(Scramble��Butterfly����ͼ),
	//��Ϊ���������ܶ࣬��Ҫ���м���ͼ��������������Ĳ��������ṩһ���л�����
	HRESULT ScrambleX(IDirect3DTexture9* pSource, IDirect3DTexture9* pRT);
	HRESULT ScrambleY(IDirect3DTexture9* pSource, IDirect3DTexture9* pRT);
	HRESULT ButterflyX(UINT iLitNo, BOOL bIFFT, IDirect3DTexture9* pSource, IDirect3DTexture9* pRT);	// ��һ��������ʾ��ǰ���õĵ����������ڶ���������ʾ��FFT����IFFT
	HRESULT ButterflyY(UINT iLitNo, BOOL bIFFT, IDirect3DTexture9* pSource, IDirect3DTexture9* pRT);

	// �ڲ�ʹ�ã�1D FFT������ģ�����1��ʾʱ�����룬����2��ʾƵ�������r��ʾ�����������ڲ���̬����
	void IFFTCPUCore(complex<double> * TD, complex<double> * FD, int r);



	//�ڲ�����
private:
	UINT m_iCreateAttrib;	// 1��ʾ�ѳ�ʼ������2��ʾ����������������ܵõ���������TexResult
	UINT m_iWidth, m_iHeight;	// �ܳ�����ʼ��ʱȷ��
	UINT m_iButterflyNumX, m_iButterflyNumY;		// Butterfly��������������log2(Width��Height)

	// ����Ȼ��壬��FFTʱʹ��
	LPDIRECT3DSURFACE9 m_pDepthBuffer;

	// Quad��
	struct QUADVERTEXTYPE
	{
		float x,y,z;
		float tu,tv,tw,tx;	//������δ�ã�ֻ��Ϊ��ȥ��PS����Ĵ���
	};
	VERTEXSHADER m_VSDrawQuad;
	UINT m_iStride;
	IDirect3DVertexBuffer9* m_pVB;



	// ����м���ʱ����������ͼ�������ս����ȫһ��
	IDirect3DTexture9* m_pTexTempUse[2];


	// ���Scramble���ݵ�
	PIXELSHADER m_PSScrambleX, m_PSScrambleY;
	IDirect3DTexture9* m_pTexScrambleX;	// �ṹ��SwapCoordX, 1.0f
	IDirect3DTexture9* m_pTexScrambleY;	// �ṹ��1.0f, SwapCoordY

	// ���Butterfly���ݵ�
	PIXELSHADER m_PSIFFTButterflyX, m_PSFFTButterflyX;
	PIXELSHADER m_PSIFFTButterflyY, m_PSFFTButterflyY;
	IDirect3DTexture9 **m_ppTexButterflyX, **m_ppTexButterflyY;
	IDirect3DTexture9 **m_ppTexButterflyX_Sign, **m_ppTexButterflyY_Sign;	// ������ͼ��x�����洢������Ϸ�ʽ���Ӽ��ķ���ֵ���ѣ���zw�����洢W�ķ���ֵ
};

struct FLUID_ATTRIBUTE
{
	// �ܶ�ϵ����Ĭ��Ϊ1���ܶ�Խ��Խ����������Ũ�ȸߵͲ��֣����������£��˶��������ֻ���������Խ�ã����ܶ�Խ��Խ��Һ�壬��������̫�ߣ������ģ�����Projectʱ�ã�
	float fConsistensy;			
	BOOL bLiquid;		// �Ƿ�Һ�壬�����Һ���Ҫ���㷨�ߣ�Ĭ��Ϊ����
	// ��������������Ϊ���ⷽ�����������ܶȳ��йأ��ܶ�Խ������Խ�󣬶����ǳ������õģ���������ע��
	D3DXVECTOR2 VecGravity;		
	float fGravityPower;		// �����ĳ˷��ݣ���������ֵԽ���ʾ�ص�Խ�أ����Խ�ᣬֵԽС��ʾ�����ֻ�ԽС��Ϊ1ʱ��ʾ�����á���Һ����һ��С��1���������ֲ���ʵ�����
	UINT iIterateNum;	// Jacobi��������
	float fVorticityRefinementCoef;	// �Ⱥܵ͵������ϣ��ؽ�����״̬����ֵΪ��ʽ�е�epsilon�����Ϊ0�ͱ�ʾ���ؽ�����
	float fViscousCoef;	// ��ϵ�������Ϊ0�ͱ�ʾ�����������ɢ
	float fDensityDampCoef;		// Ũ����ɢϵ������ֵ����С�ڵ���1���Ҳ�����̫�ͣ��������ʾ��������
	FLUID_ATTRIBUTE()
	{
		fConsistensy = 1.0f;
		bLiquid = FALSE;
		VecGravity = D3DXVECTOR2(0, 0);
		fGravityPower = 1.0f;
		iIterateNum = 40;	// ������40����
		fVorticityRefinementCoef = 0.0f;
		fViscousCoef = 0.0f;
		fDensityDampCoef = 1.0f;
	}
	
	BOOL CheckAvailable()
	{
		if(iIterateNum < 10)
		{
			OutputDebugString("��������̫�ͣ���������صľ��ȶ�ʧ��\n");
			return FALSE;
		}
		if(fConsistensy < 0.01f || fConsistensy > 4.0f)
		{
			OutputDebugString("�ܶ�̫�߻�̫�ͣ����Ϸ���\n");
			return FALSE;
		}
		if(fViscousCoef < 0.0f)
		{
			OutputDebugString("��ϵ������Ϊ������\n");
			return FALSE;
		}
		if(fGravityPower < 0.0f)
		{
			OutputDebugString("�������ݲ���Ϊ������\n");
			return FALSE;
		}
		if(fDensityDampCoef < 0.5f)
		{
			OutputDebugString("�ܶ���ɢϵ��̫�ͣ��ᵼ��ģ��������\n");
			return FALSE;
		}
		if(fDensityDampCoef > 1.0f)
		{
			OutputDebugString("�ܶ���ɢϵ������С�ڵ���1��\n");
			return FALSE;
		}
		if(fVorticityRefinementCoef < 0.0f)
		{
			OutputDebugString("�����ؽ�ϵ��Ϊ���������Ϸ���\n");
			return FALSE;
		}
		return TRUE;
	}
};


// ע������
struct INJECT_ATTRIBUTE
{
	enuInject_Type Type;		// ��������

	// ���������ã�ָ���������������¸�ʽ����Ϊ������xy���뾭���˼�0.5������ǿ�ȵĵ�λ���������꣬���Է�Χ�϶���-1~1֮�䣩��zwδ�á���Ϊ��Դ����Ҫ�󣬴����ɫ����
	// ����ʹ�ú�Fluidģ������ͬ���ֱ��ʵ���ͼ����������ҪShader���ˣ�������ܣ������͸����������
	IDirect3DTexture9* pTexInjectForce, *pTexInjectSource;
	float fTexInjectForceCoef, fTexInjectSourceCoef;		// ���ǵ�������ֵ�з�Χ���ƣ������ٸ���һ������ǿ�ȵ�ϵ����ֵ��Ϊ���⣨������ÿ��Texel����0��ʾ��ʱȡ�����䡣���������ԣ�-1��ʾ����ֵ�ķ���ȡ��

	// ����������
	UINT iRange;				// ���÷�Χ��Բ�ΰ뾶������λ����
	D3DXVECTOR2 PtPos;			// λ�õ㣨�ǵ����������꣩
	
	// ����������ֵ����ע��ǿ���йأ�����ע�������壬���������ǿ�ȱ�ʾ����ע��һ�����ﵽ��ǿ�ȣ�ʱ������������������
	// ����˵��VecForceDir����Ϊ0.5���ͱ�ʾÿ��ע�������ǿ��Ϊ0.5��ͨ��ƽ������ÿ���������ƶ�0.5���������򣨼�����������򣩡�
	D3DXVECTOR2 VecForceDir;			// ���������䳤�ȱ�ʾ����ǿ�ȣ�������ǿ��/���ȵ�λΪ�������꣩��������F��Ч����ԴS��Ч
	D3DXVECTOR3 VecCentreIntensity;		// ���ĵ�ǿ�ȣ���Ũ�ȷ�Χ0��1�����Ը�ǿ��Ϊ׼������Ȧ˥����˥�������÷�Χ��Ϊ0��֮������Vector3�ǽ�RGB�ֿ���������ԴS��Ч������F��Ч

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
	
	// ������ʾ�ܷ�Χ
	BOOL CheckAvailable(UINT iWidth, UINT iHeight)
	{
		if(Type == Inject_Texture)
		{
			if(!pTexInjectForce || !pTexInjectSource)
			{
				OutputDebugString("�������͵�ע����û��ָ������\n");
				return FALSE;
			}
		}
		else
		{
			if(!iRange || iRange > iWidth || iRange >= iHeight)
			{
				OutputDebugString("��Χ���ݷǷ���\n");
				return FALSE;
			}
			if(PtPos.x < 0.0f || PtPos.x >= (float)iWidth || PtPos.y < 0.0f || PtPos.y >= (float)iHeight)
			{
				OutputDebugString("λ�õ�����Ƿ���\n");
				return FALSE;
			}
			// ��ʱ���������ж���������Դǿ�ȣ���Set���ж�
		}

		return TRUE;
	}
};


// ����/������
enum enuFluid_VectorField
{
	FLUID_VELOCITY,		// U
	FLUID_PRESSURE,		// P
	FLUID_DENSITY,		// D
	FLUID_W,			// W
};





// ֻ��SM2.0����̬��������߽���ҪSM2.a
class KFluidSimulation2D
{
public:
	KFluidSimulation2D();
	~KFluidSimulation2D() {Release();}
	void Release();

	// ��ʼ��Fluidģ������Ķ��㻺�����ͼ����
	// ������ʾģ������ķֱ��ʣ����⣩����������������Ƶ���Ļ
	HRESULT Init(UINT iWidth, UINT iHeight, BOOL bGenerateNormal = TRUE);

	// ����Ϊ������״̬�����ٶȳ���ѹ�������ܶȳ�����
	HRESULT ResetFluid();


	// ��ʼģ��
	// Ҫ��¼����һ�ε�ʱ�䣬���ݵ�ǰʱ��ʹ��DeltaTime������ģ�����
	HRESULT FluidSimulation(FLUID_ATTRIBUTE FluidData, float fDeltaTime);

	// �����赲ͼ�����������ģ��ķֱ�����ͬ�����п�ͨ������Ϊ1���赲����Ϊ0
	// �������赲ͼת����ABC Offsetͼ������ABC������Ϊtrue
	// ���TexObstacleΪ����ABC��������Ϊfalse����������ֹ����߽�ļ���
	// TexOffset��8888��ʽ���Ҳ���Ϊ�ڴ��������������ǵ��Կ���֧��sm2.xʱ����������Obstacle��Ӧ�ģ���ǰ����õģ�Offset��ͼ��������SM2.0���Կ���Ҳ����ʵ������߽�
	// ������������ͬһ��Obstacleͼ��������Զ��Ż���ֻ�ڵ�һ������ʱ����Offsetͼ
	HRESULT SetObstacleTexture(IDirect3DTexture9* pTexObstacle, IDirect3DTexture9* pTexOffset = NULL);

	// ����˲ʱ��������Դ������ʱ�������ݣ�Ȼ����ģ�������ֱ���������ٶȳ�U���ܶȳ�D��
	HRESULT SetForce(UINT iForceNum, INJECT_ATTRIBUTE* pInjectData);
	HRESULT SetSource(UINT iSourceNum, INJECT_ATTRIBUTE* pInjectData);


	// Get��������Simulate���У�ֻ�ǵõ�ָ�룬������������
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


// Simulationʱ�ڲ�ʹ�õģ�����
private:
	// ƽ������P��Ч
	HRESULT Advect(enuFluid_VectorField Type);
	// ��ɢ����P��Ч��������Ҫ�ż��㣨Viscous����0ʱ��
	HRESULT Diffusion(enuFluid_VectorField Type);
	// �������úõ�Data�ӵ�������
	HRESULT ApplyForce();
	// �����ܶ���������
	HRESULT ApplyGravity();
	// �������úõ�Data�ӵ���Դ��
	HRESULT ApplySource();


	// ��������õ�W����P���м��������Wɢ�ȼ�Jacobi����
	HRESULT SolvePoissonPressureEquation();

	// ��������õ�P����U����ͶӰ
	HRESULT Project();

	// ������Ҫ�ؽ����У�Vorticity����0ʱ��
	HRESULT RefineVorticity();

	// �߽����������������߽������߽磩��������ʾ�������ֳ��ı߽磬������������Ч��pTexture��ʾ��Ҫ�����µ�����ָ�룬ע����ԭ�ظ��£�����=����������Ѿ��Զ�������CopyToTemp�Ĳ��������õ��Ļ����
	HRESULT ApplyBoundaryCondition(enuFluid_VectorField Type, IDirect3DTexture9* pTexture);

	// NormalMap
	HRESULT GenerateNormal();


	
// ģ��ʱ�ڲ�ʹ�õģ���������
private:
	// �����ϰ�ͼ������ABCƫ��������ͼ��������SetObstacle��������
	HRESULT UpdateABCTexture(IDirect3DTexture9* pTexObstacle);

	// ����������ʵ����һ���򵥵�Copy CommonComputeQuad���ã�ר�������
	HRESULT CopyTexture(IDirect3DTexture9* pTexSrc, IDirect3DTexture9* pTexDst)
	{
		return CommonComputeQuad(pTexSrc, NULL, NULL, pTexDst, &m_PSCopy);	
	}

	// �������ÿ��������Ϊָ����ɫ��Ĭ��Ϊ0
	HRESULT ClearTexture(IDirect3DTexture9* pTexture, LPD3DXVECTOR4 pColor = NULL)
	{
		D3DXVECTOR4 VecColor = D3DXVECTOR4(0, 0, 0, 0);
		if(pColor)
			VecColor = *pColor;
		V_RETURN(d3ddevice->SetPixelShaderConstantF(5, (float *)&VecColor, 1));
		return CommonComputeQuad(NULL, NULL, NULL, pTexture, &m_PSClearTexture);
	}

	// ��������ѡ��Inject Shader
	PIXELSHADER *ChooseForceShader(enuInject_Type Type);
	PIXELSHADER *ChooseSourceShader(enuInject_Type Type);


	// ����Pixel Shader���㲽�裬������ǰ�������Ĵ������ú�
	// ����õ�����ͼֻ��һ�㣬��ô�ڶ�������������NULL
	// ����ǻ�Quad
	HRESULT CommonComputeQuad(IDirect3DTexture9* pSrcTex1, IDirect3DTexture9* pSrcTex2, IDirect3DTexture9* pSrcTex3, IDirect3DTexture9* pRT, PIXELSHADER *pPS);

	// ����ͬ�ϣ�����ǻ�Line����Ӧm_pVBBoundaryLine[iLineNo]������D3DPT_LINELSIT
	HRESULT CommonComputeLine(IDirect3DTexture9* pSrcTex1, IDirect3DTexture9* pSrcTex2, IDirect3DTexture9* pSrcTex3, IDirect3DTexture9* pRT, PIXELSHADER *pPS, UINT iLineNo);


// �ڲ�ʹ�ñ���
public:
	UINT m_iCreateAttrib;
	UINT m_iWidth, m_iHeight;
	BOOL m_bGenerateNormal, m_bABC;	// �Ƿ���GPU���ɷ���ͼ���Ƿ����ABC

	float m_fDeltaTime;
	UINT m_iRenderNumPerFrame;		// ��ǰ֡��Ⱦ���ٴΣ������ö��ٴ�DrawQuad��������FluidSimulation��Ϳ��Դ���֪����Ⱦ������

	FLUID_ATTRIBUTE m_FluidData;

	// ˲ʱע������
	UINT m_iInjectForceNum, m_iInjectSourceNum;
	INJECT_ATTRIBUTE *m_pInjectForce, *m_pInjectSource;

		
		
	// ���㻺��
	struct VERTEXTYPE
	{
		float x,y,z,w;		// XYZRHW
		float tu,tv,tw,tx;	// �������꣺С����������δ�ã�ֻ��Ϊ��ȥ��PS����Ĵ���
		float tu_int,tv_int,tw_int,tx_int;	// ͬ�ϣ������������� for Bilinear Filtering
	};
	IDirect3DVertexDeclaration9 *m_pDeclaration;
	UINT m_iStride;
	IDirect3DVertexBuffer9* m_pVBQuad, *m_pVBQuadExceptBoundary; // һ������Σ���һ���������߽��ߵľ��Σ�ģ��ʱ�ã�
	IDirect3DVertexBuffer9* m_pVBBoundaryLine[4];	// �ıߵ��ߣ����ڸ��»����߽磬�ı�0��3�ֱ�Ϊ��������
	LPDIRECT3DSURFACE9 m_pDepthBuffer;



	
	
// ��ͼ���֣�����������XY��ž���ֵ��ZW��ŷ��ţ�0��2�ֱ��ʾ������������Normal/ABC Offsetͼ�����ǳ˼�0.5����-1��1ת��Ϊ0��1�洢
public:
	IDirect3DTexture9* m_pTexWTemp[MAX_FLUID_STEPS], *m_pTexDTemp[MAX_FLUID_STEPS];	// ��ʱʹ�ã����������W���ܶȳ�
	UINT m_iCurrentWIndex, m_iCurrentDIndex;		// ��Ϊ���㲽���������������û��趨��ͬ��������������������ֵ����ǰ����õ�W��D�������������ͼ�У��ڼ��������в���֮������Ҳ�����������ݴ�ŵ���ͼ��š�

	IDirect3DTexture9* m_pTexTemp[2];	// ������ʱ�л�ѭ��ʹ�ã�����ѭ������Jacobi������ѭ������FS��
	IDirect3DTexture9* m_pTexBCTemp;	// ������ʱ����߽�������BC�������������ܻ�����һ������Ҫ������һ����ͼ
	IDirect3DTexture9* m_pTexP, *m_pTexD, *m_pTexU;	// ��õģ�ѹ�������ܶȳ����ٶȳ�U����סҪ����󿽱�TexDTemp��TexD��


	// ����߽�������Arbitary Boundary Conditions����
	IDirect3DTexture9* m_pTexABCTypeToOffset;	// ת��ͼ����Ų�ͬ�����Ͷ�Ӧ���ٽ�ƫ�Ƶ��������ݣ���ʼ��һ�μ��ɡ���ʱʹ�ã����ڼ���ƫ������ͼ
	IDirect3DTexture9* m_pTexABCType;			// ABC����ͼ����ż���õĸ����߽�㿪�����͡���ʱʹ�ã����ڼ���ƫ������ͼ
	IDirect3DTexture9* m_pTexABCBoundaryToOffset;	// ת��ͼ����Ų�ͬBoundary���֮�Ͷ�Ӧ���ٽ�ƫ�Ƶ��������ݣ���ʼ��һ�μ��ɡ���ʱʹ�ã����ڼ���ƫ������ͼ
	
	IDirect3DTexture9* m_pTexABCBoundary;		// �ϰ�����ͼ����ű߽�㡢�ϰ�����ǷǱ߽��
	IDirect3DTexture9* m_pTexABCOffset;			// ABCÿ���߽���ƫ�ƣ�xy��zw�ֱ��������ٽ����ƫ�����꣬��Χ����Ϊ0��1�������ٽ���ͷ������ͬ������U��P���Թ���

	// ��ʱʹ�ã�����W��ɢ�ȣ�ѹ�����ã������ȣ������ؽ��ã�
	IDirect3DTexture9* m_pTexDivW, *m_pTexCurlW;

	// ���߷�Χ������0��1֮�䣬���ü�Sign����
	IDirect3DTexture9* m_pTexNormal;



// Pixel Shader����
private:
	// MISC
	PIXELSHADER m_PSClearTexture;						// ����Ϊָ����ɫ��Ĭ��0��,��������Reset��Ҳ��������Ũ�ȳ�D�Ļ����߽�����
	PIXELSHADER m_PSCopy;							// ������
	PIXELSHADER m_PSGenerateNormal;					// Normal
	
	// Inject
	PIXELSHADER m_PSAddForceSphere, m_PSAddSourceSphere;		// ��˹Բ��ע����
	PIXELSHADER m_PSAddForceTexture, m_PSAddSourceTexture;		// ����ע����
	PIXELSHADER m_PSAddForceFire, m_PSAddSourceFire;			// ������ע����

	// NSEs
	PIXELSHADER m_PSAdvect_W, m_PSAdvect_D;			// Advect
	PIXELSHADER m_PSGravity;						// Gravity in Liquid
	PIXELSHADER m_PSDiv_W, m_PSProject;				// Div.W, U=W-Grad.P
	PIXELSHADER m_PSJacobi_W, m_PSJacobi_D, m_PSJacobi_P;	// Jacobi Iteration���ֱ����ڴ���Diffusion��Poisson Pressure
	PIXELSHADER m_PSCurl_W, m_PSVorticity;			// Vorticity Refinement
	
	// BCs
	PIXELSHADER m_PSBBC_U, m_PSBBC_P;				// �����߽�������Basic Boundary Condition����ע��BC_U����ͬʱ������U��W
	PIXELSHADER m_PSABC_U, m_PSABC_P, m_PSABC_D;	// ����߽�������Arbitary Boundary Condition��
			// Update ABC Offset Texture��
	PIXELSHADER m_PSObstacleToBoundary;				// ����ABCƫ��ͼ�ĵ�һ�����ϰ�ͼת��Ϊ�߽�ͼ
	PIXELSHADER m_PSBoundaryToOffset;				// ����2���߽�ͼֱ��ת��Ϊƫ��ͼ����Ҫ76��ָ�ps2.x��
	PIXELSHADER m_PSBoundaryToType, m_PSTypeToOffset;	// ����1���߽�ͼ�ȵ���������ͼ��Ȼ����ת��Ϊƫ��ͼ��Pass�࣬������Ҫps2.a��
};
