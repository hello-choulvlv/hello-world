#pragma once

#include "myd3d.h"
#include "GPGPU.h"
#include "Shader.h"


#define DEEP_OCEAN 0.0f			// �������ͣ��
#define SWALLOW_WATER -1.0f		// �������ͣ�ǳˮ




// ��������Ĳ���
struct OCEANWATER_ATTRIBUTE
{
	float fOceanDepth;				// ˮ�Ĭ��Ϊ0����DEEP_OCEAN
	UINT iWidth, iHeight;			// �ռ������ԣ����ָ������
	D3DXVECTOR2 WaterSquare;		// ˮ���ܳ�����������
	D3DXVECTOR2 VecWindDir;			// ���򣬹������
	float fWindSpeed;				// ���٣���λ������ռ�Ϊ׼

	float fWaveHeightScale;			// ���˸߶�ȫ�����ţ������ܿز��˸߶ȣ�ֻ�������Զ�̬�ı䣬������FFTOceanWater���к��Ը���Լ�����VS�н�������

	OCEANWATER_ATTRIBUTE()
	{
		fOceanDepth = DEEP_OCEAN;
		iWidth = iHeight = 0;
		WaterSquare = D3DXVECTOR2(0, 0);
		VecWindDir = D3DXVECTOR2(0, 0);
		fWindSpeed = 0.0f;
		fWaveHeightScale = 1.0f;
	}

	// ����������Ч��
	BOOL CheckAvaliable()
	{
		if(fOceanDepth < 0.0f && fOceanDepth != SWALLOW_WATER)
		{
			OutputDebugString("�������������Ч��\n");
			return FALSE;
		}
		if(fOceanDepth != DEEP_OCEAN)
		{
			OutputDebugString("��ʱֻ֧�����ˮ��ģ�⣡\n");
			return FALSE;
		}
		// ������2���ݣ�0��1ͬʱ������
		if(!CheckPowerOf2(iWidth) || !CheckPowerOf2(iHeight))
		{
			OutputDebugString("�ָ������������Ҫ�󣡱���Ϊ2����\n");
			return FALSE;
		}
		if(WaterSquare.x < 0.000001f || WaterSquare.y < 0.000001f)
		{
			OutputDebugString("��ˮ���������Ч��\n");
			return FALSE;
		}
		if(absf(D3DXVec2Length(&VecWindDir) - 1.0f) > 0.00001f)
		{
			OutputDebugString("����Ϊ���������\n");
			return FALSE;
		}
		if(fWindSpeed < 0.000001f)
		{
			OutputDebugString("����������Ч��\n");
			return FALSE;
		}
		if(fWaveHeightScale < 0.000001f)
		{
			OutputDebugString("������������������Ч��\n");
			return FALSE;
		}

		return TRUE;
	}
};




// ������������FP���٣����չ������ʹ�ã�VTF or Lock������ͼ����FP32�����õ���
class KFFTOceanWater
{
public:
	KFFTOceanWater();
	~KFFTOceanWater() {Release();}

	// ��ʼ���������ͼ��h0�������ݣ�����Ĳ�������ָ�������������Ϊ2����
	HRESULT Init(OCEANWATER_ATTRIBUTE OceanData, BOOL bGenerateNormal = TRUE, BOOL bGenerateTangent = TRUE);
	void Release();

	// ���������棨����h0ͼ����hͼ��Ȼ����IFFT����������洢��һ�Ÿ߶�ͼ�У���FFT�е�TexResult
	// ���Լ�һ�Ÿ߶�ͼ������Ӱ�캣��ĳЩ�����ĳЩ��ĸ߶ȣ����γ����������Ĳ��ơ������ȣ������ߡ����ߵ�Ҳ�ᱻӰ��
	// ���Ӹ߶�ͼ��xy�������õ���Ӱ�캣��߶Ⱥͺ��ᣨYX����zw����������ţ�0��2��
	// �����ɸռ���õ�IFFT��������Ӻ󱣴浽m_pTexHeight��
	HRESULT WaterSimulation(float fTime, LPDIRECT3DTEXTURE9 pTexAddonHeightMap = NULL);

	// �����µĺ������ݣ������³�ʼ������Ҫÿ֡������
	// ����������û��һ�����ÿ֡��̬�ı�Ķ�������ԭ�����ݵģ�����ֻҪ���þͱ������³�ʼ��
	HRESULT SetNewOceanData(OCEANWATER_ATTRIBUTE OceanData)
	{
		if(!OceanData.CheckAvaliable())
			return D3DERR_INVALIDCALL;
		Release();
		V_RETURN(Init(OceanData));
		return S_OK;
	}


	// �õ���ǰ����õĸ߶�ͼ�����ߺ�����ͼ�����ָ�룬���������ݣ�����ֱ�ӱ��������Ϊ������ã������޷��˸���Ҷϵ��
	// ������ִ��WaterSimulation֮��ſ��Եõ�
	// �õ��˷���N������Tͼ��������B = T��B
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

	//�ڲ�����
private:
	// ��h(0)���ݲ����͵�ǰʱ������h����GPU
	HRESULT GenerateH(float fTime);

	// ���Ӹ��Ӹ߶�ͼ
	HRESULT AddHeightMap(LPDIRECT3DTEXTURE9 pTexAddonHeightMap);

	// ͨ��ģ��õĸ߶�ͼ���ɷ��ߺ�����
	HRESULT GenerateNormal();
	HRESULT GenerateTangent();
	HRESULT GenerateNormalandTangent();		// ����MRT����������

	// ��ʼ��h(0)
	HRESULT InitH0();


	//�ڲ�����
private:
	// ����h0(K)��h0(-K)
	D3DXVECTOR2 CalcH0(int iX, int iY);

	//�ڲ�����
private:
	BOOL m_bGenerateNormal;	// �Ƿ���GPU���ɷ��ߺ�����ͼ������֧��PS2.a��
	BOOL m_bGenerateTangent;

	UINT m_iCreateAttrib;	// 1��ʾ�ѳ�ʼ������2��ʾ����������������ܵõ���������TexResult
	OCEANWATER_ATTRIBUTE m_OceanData;	// �û�ָ���ĺ�������

	// ��Ⱦhʱ�õ�Quad
	struct QUADVERTEXTYPE
	{
		float x,y,z;
		float tu,tv,tw,tx;	//������δ�ã�ֻ��Ϊ��ȥ��PS����Ĵ���
	};
	VERTEXSHADER m_VSDrawQuad;
	UINT m_iStride;
	LPDIRECT3DVERTEXBUFFER9 m_pVB;

	// ��Ⱦhʱ��
	LPDIRECT3DSURFACE9 m_pDepthBuffer;
	PIXELSHADER m_PSGenerateH;

	// ���Ӹ��Ӹ߶�ͼ
	PIXELSHADER m_PSAddHeightMap;

	// ���ռ���õĸ߶�ͼ
	LPDIRECT3DTEXTURE9 m_pTexHeight;

	// ���ߡ�����ͼ
	LPDIRECT3DTEXTURE9 m_pTexNormal, m_pTexTangent;
	PIXELSHADER m_PSGenerateNormal, m_PSGenerateTangent, m_PSGenerateNormalandTangent;


	// ���h(0)��h������4ͨ���ĸ�����ͼ��Ҫ�渴������ʽͬFFT��ʵ�鲿����ֵ+ʵ�鲿���ţ�0��2��
	LPDIRECT3DTEXTURE9 m_pTexH;
	LPDIRECT3DTEXTURE9 m_ppTexH0[2];

	// ��IFFT
	KFourierTransform m_IFFT;
};















// �����������������
struct WAVEEQUATION_ATTRIBUTE
{
	UINT iWidth, iHeight;		// �ָ������
	float fDampCoef;			// ����������ϵ���������˶�ѧVerlet����
	float fWaveSpeed;			// ���������м��ٶȵ�ϵ��C^2��������Ϊ���ٽ��п��ƣ�Ϊ0ʱ��ʾ��ʱ���β���
	D3DXVECTOR2 WaterSquare;	// ˮ���С���������꣩�����ڼ��㷨/����
	char szAreaDampTexture[100];// ���ٶ�˥����ͼ������ǿ������ĳЩ����Ĳ�����һ���ǲ���������ѡ�����ٶ�ϵ����alphaͨ���д洢

	WAVEEQUATION_ATTRIBUTE()
	{
		iWidth = iHeight = 0;
		fWaveSpeed = fDampCoef = 1.0f;
		WaterSquare = D3DXVECTOR2(0.0f, 0.0f);
		ZeroMemory(szAreaDampTexture, 100);
	}

	// ����������Ч��
	BOOL CheckAvaliable()
	{
		// ������2���ݣ�0��1ͬʱ������
		if(!iWidth || !iHeight)
		{
			OutputDebugString("�ָ����������Ϊ0��\n");
			return FALSE;
		}
		if(fDampCoef < 0.000001f)
		{
			OutputDebugString("����ϵ����Ч��\n");
			return FALSE;
		}
		if(WaterSquare.x < 0.000001f || WaterSquare.y < 0.000001f)
		{
			OutputDebugString("ˮ�����������Ч��\n");
			return FALSE;
		}


		return TRUE;
	}
};





// ��ҪSM2.0�����ɺ�ģ�Ⲩ������ҪSM2.x�����ɷ���/����ͼ
// ������������FP���٣����չ������ʹ�ã�VTF or Lock������ͼ����FP32�����õ���
class KWaveEquationWater
{
public:
	KWaveEquationWater();
	~KWaveEquationWater() {Release();}

	// ��ʼ���������ͼ�����ݣ�����Ĳ�������DampingTexture�ͷָ�������
	// DampingTexture���ڼ�����ٶ�ʱ��������ͼ��ʹ���ٶ��ڱ߽紦��ʧ
	HRESULT Init(WAVEEQUATION_ATTRIBUTE WaveData, BOOL bGenerateNormal = FALSE, BOOL bGenerateTangent = FALSE);
	void Release();

	// �Ⲩ�����̣����ݵ�ǰ���õ�ĳЩ���ֵ������ģ�⣬��������洢��һ�Ÿ߶�ͼ��
	// ���Խ�����߶�ͼ�ӵ�FFTWATER���棬�γ����������Ĳ��ơ������ȣ�ͬʱ���ߡ����ߵ�Ҳ�ᱻӰ��
	// xy�������õ���Ӱ�캣��߶Ⱥͺ��ᣨYX����zw����������ţ�0��2��
	// DeltaTime����ù̶���������Ҫ����ʵʱ�䣬ͨ������ִ��ģ�⺯���ķ�ʽ���ﵽ��֡���µ��ٶ�ƽ��
	// ���ǵ���ֵ�ȶ���Dampϵ����ģ��ˮ���޲���ʱ�Ļ�׼�߶ȣ�yֵ������Ϊ0�����ˮ���и߶�ƽ�ƣ�������ģ��ʱ�߶�Ϊ0 + ƽ�ƺ�����Ⱦ�ķ�ʽ������
	HRESULT WaterSimulation(float fDeltaTime);


	// ���貨��
	HRESULT ResetWave();

	// ���ò��٣�����ʱ�䲽��һ������ô������Ӧ��ͬ֡���ı�������ֻ�в���һ����
	void SetWaveSpeed(float fSpeed)
	{
		m_WaveData.fWaveSpeed = fSpeed;
	}


	// ����ĳЩ��/����ĸ߶�ֵ��ֻ��ͨ��������������ܴ���ƽ��̬�����𲨶�
	// ���ڸú������ý�Ƶ����Ϊ�����٣���ʹ��Lock��������Render Point/Rect Primitive To Texture�ķ���
	// bAddHeight����˼�ǣ�����ǰ�߶ȵ�����ȥ������ǿ�����趨ֵ����ԭ�߶�
	// ��ͼԪ�У�����ͬʱ���ö����/����X/Y��ʾ���ĵ����꣨�����������꣩��pHeight��ʾ�߶�ֵ
	// ����ͼԪ�У�ͬʱֻ�ܴ���һ����X/Y��ʾ���ĵ����꣨�����������꣩��Range��ʾ���γ����������꣩��TexArea��ʾ����߶�����xzͨ���ֱ𱣴�߶ȵľ���ֵ�ͷ��ţ�������������������ÿ�����صĸ߶�ֵ��pHeightֻ��Ϊ����ϵ��
	HRESULT SetPointHeight( UINT iPointNum, UINT *pX, UINT *pY, float *pHeight, BOOL bAddHeight );
	HRESULT SetAreaHeight( UINT iX, UINT iY, D3DXVECTOR2 VecRange, LPDIRECT3DTEXTURE9 pTexArea, float fHeight, BOOL bAddHeight );



	// �����赲ͼ������Ͳ���ģ��ķֱ�����ͬ�����п�ͨ������Ϊ1���赲����Ϊ0
	// �������赲ͼת����ABC Offsetͼ������ABC������Ϊtrue
	// ���TexObstacleΪ����ABC��������Ϊfalse����������ֹ����߽�ļ���
	// TexOffset��8888��ʽ���Ҳ���Ϊ�ڴ��������������ǵ��Կ���֧��sm2.xʱ����������Obstacle��Ӧ�ģ���ǰ����õģ�Offset��ͼ��������SM2.0���Կ���Ҳ����ʵ������߽�
	// ������������ͬһ��Obstacleͼ��������Զ��Ż���ֻ�ڵ�һ������ʱ����Offsetͼ
	HRESULT SetObstacleTexture(LPDIRECT3DTEXTURE9 pTexObstacle, LPDIRECT3DTEXTURE9 pTexOffset = NULL);


	// �����µĲ������ݣ������³�ʼ������Ҫÿ֡������
	// ֻҪ���þͱ������³�ʼ��
	HRESULT SetNewWaveData(WAVEEQUATION_ATTRIBUTE WaveData)
	{
		if(!WaveData.CheckAvaliable())
			return D3DERR_INVALIDCALL;
		Release();
		V_RETURN(Init(WaveData, m_bGenerateNormal, m_bGenerateTangent));
		return S_OK;
	}


	// �õ���ǰ����õĸ߶�ͼ�����ߺ�����ͼ�����ָ�룬���������ݣ�����ֱ�ӱ��������Ϊ������ã������޷��˸���Ҷϵ��
	// ������ִ��WaterSimulation֮��ſ��Եõ�
	// �õ��˷���N������Tͼ��������B = T��B
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



	//�ڲ�����
private:
	// ����ˣ�һ���ǰ�Quad�������һ��������ʾ�Ƿ����û��Զ��ľ������򣬽���SetAreaHight���ã���һ���ǰ�Point��
	HRESULT CommonComputeQuad(LPDIRECT3DTEXTURE9 pSrcTex1, LPDIRECT3DTEXTURE9 pSrcTex2, LPDIRECT3DTEXTURE9 pSrcTex3, LPDIRECT3DTEXTURE9 pRT1, LPDIRECT3DTEXTURE9 pRT2, PIXELSHADER *pPS, BOOL bUserQuad = FALSE);
	HRESULT CommonComputePoint(UINT iPointNum, LPDIRECT3DTEXTURE9 pSrcTex1, LPDIRECT3DTEXTURE9 pSrcTex2, LPDIRECT3DTEXTURE9 pSrcTex3, LPDIRECT3DTEXTURE9 pRT, PIXELSHADER *pPS);

	// ����������ʵ����һ���򵥵�Copy CommonComputeQuad���ã�ר�������
	HRESULT CopyTexture(LPDIRECT3DTEXTURE9 pTexDst, LPDIRECT3DTEXTURE9 pTexSrc)
	{
		return CommonComputeQuad(pTexSrc, NULL, NULL, pTexDst, NULL, &m_PSCopyTexture);
	}

	// �������ÿ��������Ϊָ����ɫ��Ĭ��Ϊ0
	HRESULT ClearTexture(LPDIRECT3DTEXTURE9 pTexture, LPD3DXVECTOR4 pColor = NULL)
	{
		D3DXVECTOR4 VecColor = D3DXVECTOR4(0, 0, 0, 0);
		if(pColor)
			VecColor = *pColor;
		V_RETURN(d3ddevice->SetPixelShaderConstantF(5, (float *)&VecColor, 1));
		return CommonComputeQuad(NULL, NULL, NULL, pTexture, NULL, &m_PSClearTexture);
	}


	//�ڲ�����
private:
	BOOL m_bABC;			// �Ƿ��������߽�
	BOOL m_bGenerateNormal;	// �Ƿ���GPU���ɷ��ߺ�����ͼ������֧��PS2.a��
	BOOL m_bGenerateTangent;

	float m_fDeltaTime;
	UINT m_iCreateAttrib;	// 1��ʾ�ѳ�ʼ������2��ʾ����������������ܵõ���������TexResult
	WAVEEQUATION_ATTRIBUTE m_WaveData;	// �û�ָ���Ĳ�������

	// ���㻺��
	struct VERTEXTYPE
	{
		float x,y,z,w;		// XYZRHW
		float tu,tv,tw,tx;	// �������꣺С����������δ�ã�ֻ��Ϊ��ȥ��PS����Ĵ���
		float tu_int,tv_int,tw_int,tx_int;	// ͬ�ϣ������������� for Bilinear Filtering
	};
	LPDIRECT3DVERTEXDECLARATION9 m_pDeclaration;
	UINT m_iStride;
	LPDIRECT3DVERTEXBUFFER9 m_pVBQuad, m_pVBQuadUser;	// Ĭ�ϵĴ���Σ���һ�������û�ָ����������ľ��Σ���������ʽע��
	LPDIRECT3DVERTEXBUFFER9 m_pVBPoint;					// ��ͼԪ����������ָ����ĸ߶�ֵ
	LPDIRECT3DSURFACE9 m_pDepthBuffer;


	// MISC��Pixel Shader
	PIXELSHADER m_PSCopyTexture, m_PSClearTexture;

	// ���û����ָ����/��������ĸ߶�
	PIXELSHADER m_PSSetPointHeight, m_PSSetAreaHeight;
	PIXELSHADER m_PSAddPointHeight, m_PSAddAreaHeight;

	// ��ʱʹ�õ���ͼ
	LPDIRECT3DTEXTURE9 m_pTexTemp;

	// ����߽�������Arbitary Boundary Conditions����
		// Update ABC Offset Texture��
	LPDIRECT3DTEXTURE9 m_pTexABCBoundaryToOffset;	// ת��ͼ����Ų�ͬBoundary���֮�Ͷ�Ӧ���ٽ�ƫ�Ƶ��������ݣ���ʼ��һ�μ��ɡ���ʱʹ�ã����ڼ���ƫ������ͼ
	PIXELSHADER m_PSObstacleToBoundary;				// ����ABCƫ��ͼ�ĵ�һ�����ϰ�ͼת��Ϊ�߽�ͼ
	PIXELSHADER m_PSBoundaryToOffset;				// ����2���߽�ͼֱ��ת��Ϊƫ��ͼ����Ҫ76��ָ�ps2.x��

		// ��������߽�������
	LPDIRECT3DTEXTURE9 m_pTexABCBoundary;		// �ϰ�����ͼ����ű߽�㡢�ϰ�����ǷǱ߽��
	LPDIRECT3DTEXTURE9 m_pTexABCOffset;			// ABCÿ���߽���ƫ�ƣ�xy��zw�ֱ��������ٽ����ƫ�����꣬��Χ����Ϊ0��1�������ٽ���ͷ������ͬ������U��P���Թ���
	PIXELSHADER m_PSABC_Bounce;					// ��������߽�����


	// ���ռ���õĸ߶�ͼ��Verlet������ʹ�õ�
	LPDIRECT3DTEXTURE9 m_pTexHeight, m_pTexPrev, m_pTexNow;
	LPDIRECT3DTEXTURE9 m_pTexAreaDamping;	// �û�ָ������ѡ��������ʹ������ʧ��ĳЩ������Ҫ���ڱ߽紦��
	PIXELSHADER m_PSWaveEquation, m_PSWaveEquationWithDampTexture;

	// ���ߡ�����ͼ
	LPDIRECT3DTEXTURE9 m_pTexNormal, m_pTexTangent;
	PIXELSHADER m_PSGenerateNormal, m_PSGenerateTangent, m_PSGenerateNormalandTangent;
};
