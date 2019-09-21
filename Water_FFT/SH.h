#pragma once

#include "Shader.h"


// ��������LDPRT Lobe��
#define MYLDPRT_MAXLOBE 1		// LDPRT Max Lobes Num�������ǹ̶������ᣬ����Mutli Lobe��û�����ˣ�������̶�����ô��Ҫռ�ö���Ĵ洢����������ʱֻ֧��1 Lobe
#define MYPRT_MAXBOUNCE 30		// Max Light Bounce Order
#define MYSH_MAXORDER 6			// PRT Max Bands Num, Recommanded: 4 Bands!!!
#define MYSH_MINORDER 2
#define MYSH_MAXSAMPLENUM 65536	// Monte Carlo Ray Tracing Max Sample Rays Num
#define MYSH_MINSAMPLENUM 16


void OutputSH(float *pData, UINT iBandNum);



// PRT Attrib Struct��Describes the effects in PRT simulation
// Mirror: Albedo = 1.0 and No SSS
struct PRTATTRIBUTE
{
	float fAlbedo[3];		// Surface Albedo, R/G/B

	BOOL bShadowed;			// Whether Self-Shadow Term
	
	UINT iBounceNum;		// Times when light bounced, 0 indicates direct lighting(no bounce)

	float fLengthScale;		// Length Scale in SSS������ֵ��ʾ�����嶥��֮���������ţ���ԽС���Ҳ��ԽС��͸���Ҳ��Խ��
	UINT iLDPRTLobeNum;		// Number of LDPRT Lobe, 0 indicates no need to LDPRT Coefficients
	
	BOOL bSSS;				// Whether Contain SubSurface Scattering
	// SSS Material, Valid only when 3S is enabled
	float fRelativeIndexofRefraction;	// ��������ʣ��������/������ʣ�������/����
	float fAbsorbation[3];				// Media Absorbation Coefficient��R/G/B
	float fScatteringCoef[3];			// Media Scattering Coefficient��R/G/B��Notion Absorb+Scatter = Extinct

	PRTATTRIBUTE()
	{
		fAlbedo[0] = fAlbedo[1] = fAlbedo[2] = 1.0f;
		bShadowed = bSSS = FALSE;
		fLengthScale = 1.0f;
		iBounceNum = iLDPRTLobeNum = 0;
		fRelativeIndexofRefraction = 1.0f;
		fAbsorbation[0] = fAbsorbation[1] = fAbsorbation[2] = 0.0f;
		fScatteringCoef[0] = fScatteringCoef[1] = fScatteringCoef[2] = 0.0f;
	}

	~PRTATTRIBUTE()
	{
		fAlbedo[0] = fAlbedo[1] = fAlbedo[2] = 1.0f;
		bShadowed = bSSS = FALSE;
		fLengthScale = 1.0f;
		iBounceNum = iLDPRTLobeNum = 0;
		fRelativeIndexofRefraction = 1.0f;
		fAbsorbation[0] = fAbsorbation[1] = fAbsorbation[2] = 0.0f;
		fScatteringCoef[0] = fScatteringCoef[1] = fScatteringCoef[2] = 0.0f;
	}
};





// SH Polymonia Y(Theta, Phi)
class SHTable
{
public:
	SHTable()
	{
		m_pTableData = NULL;
		m_pTable = NULL;
		m_iMaxBandNum = 0;
		m_fPhi = m_fTheta = 0.0f;
		m_bInit = m_bGenerate = FALSE;
	}
	~SHTable()
	{
		Release();
	}

	void Release()
	{
		SAFE_DELETE_ARRAY(m_pTableData);

		if(m_pTable)
			for(UINT i = 0; i < m_iMaxBandNum; i++)
				SAFE_DELETE_ARRAY(m_pTable[i]);
		
		SAFE_DELETE_ARRAY(m_pTable);
		m_iMaxBandNum = 0;
		m_fPhi = m_fTheta = 0.0f;
		m_bInit = m_bGenerate = FALSE;

	}

	// ��ʼ����ֻ�ܳ�ʼ��һ�Σ���Ҫ�����Ƕ�����Band��Table���飬����������Ҫ��̬����
	// iMaxBand��ʾ����м���Band����������Band���+1
	HRESULT Init(UINT iBandNum)
	{
		if(iBandNum > MYSH_MAXORDER || iBandNum < MYSH_MINORDER)
			return D3DERR_INVALIDCALL;
		if(m_bInit)
			return D3DERR_NOTAVAILABLE;

		m_iMaxBandNum = iBandNum;

		SAFE_DELETE_ARRAY(m_pTableData);
		SAFE_DELETE_ARRAY(m_pTable);

		// �����ռ�
		UINT iSize = GetTotalMonoNum();
		m_pTableData = new float[iSize];

		m_pTable = new float*[m_iMaxBandNum];

		if(!m_pTable || !m_pTableData)
		{
			OutputDebugString("�����ڴ�ʧ�ܣ������Ƿ��Ǵ����SHϵ���ռ�С��Band��Ӧ�Ĵ�С������\n");
			return E_OUTOFMEMORY;
		}

		ZeroMemory(m_pTable, sizeof(float) * m_iMaxBandNum);

		for(UINT i = 0; i < m_iMaxBandNum; i++)
		{
			iSize = GetBandMonoNum(i);
			m_pTable[i] = new float[iSize];
			if(!m_pTable[i])
			{
				OutputDebugString("�����ڴ�ʧ�ܣ������Ƿ��Ǵ����SHϵ���ռ�С��Band��Ӧ�Ĵ�С������\n");
				return E_OUTOFMEMORY;
			}
		}

		m_bInit = TRUE;
		return S_OK;
	}

	// Load from memory, Using in KMCSamples
	// ������������Ͳ�������Init��Generate��������������Init��Generate֮ǰ��
	// ÿ��SH�� 14 + 4*BandNum*BandNum �ֽ�
	HRESULT LoadFromMemory(void *pData)
	{
		if(!pData)
			return D3DERR_INVALIDCALL;
		if(m_bInit || m_bGenerate)
			return D3DERR_NOTAVAILABLE;

		// Data Format: "SH" + BandNum + fTehta + fPhi + Data(4*bandnum*bandnum)
		char *szHead = (char *)pData;
		if(*szHead++ != 'S' || *szHead++ != 'H')
			return E_FAIL;
		
		UINT *pBandNum = (UINT *)szHead;
		UINT iBandNum = *pBandNum++;

		if(iBandNum > MYSH_MAXORDER || iBandNum < MYSH_MINORDER)
			return D3DERR_INVALIDCALL;

		m_iMaxBandNum = iBandNum;

		// �����ռ�
		SAFE_DELETE_ARRAY(m_pTableData);
		SAFE_DELETE_ARRAY(m_pTable);

		m_pTableData = new float[GetTotalMonoNum()];
		m_pTable = new float*[m_iMaxBandNum];

		for(UINT i = 0; i < m_iMaxBandNum; i++)
			m_pTable[i] = new float[GetBandMonoNum(i)];

		// �������
		float *pSHDataHead = (float *)pBandNum;
		m_fTheta = *pSHDataHead++;
		m_fPhi = *pSHDataHead++;

		float *pSHData = pSHDataHead;

		for(UINT j = 0; j < m_iMaxBandNum; j++)
			for(int i = 0; i < GetBandMonoNum(j); i++)
				m_pTable[j][i] = *pSHData++;

		pSHData = pSHDataHead;
		for(int i = 0; i < m_iMaxBandNum*m_iMaxBandNum; i++)
			m_pTableData[i] = *pSHData++;


		if(!m_pTable || !m_pTableData)
			return E_OUTOFMEMORY;


		m_bInit = TRUE;
		m_bGenerate = TRUE;
		return S_OK;
	}
	

	// Save to memory, Using in KMCSamples
	// ������Init��Generate֮����
	// ÿ��SH�� 14 + 4*BandNum*BandNum �ֽ�
	HRESULT SaveToMemory(void *pData)
	{
		if(!pData)
			return D3DERR_INVALIDCALL;
		if(!m_bInit || !m_bGenerate)
			return D3DERR_NOTAVAILABLE;

		// Data Format: "SH" + BandNum + fTheta + fPhi + Data(4*bandnum*bandnum)
		char *szHead = (char *)pData;
		*szHead++ = 'S';
		*szHead++ = 'H';

		UINT *pBandNum = (UINT *)szHead;
		*pBandNum++ = m_iMaxBandNum;

		// �������
		float *pSHData = (float *)pBandNum;
		*pSHData++ = m_fTheta;
		*pSHData++ = m_fPhi;

		for(UINT i = 0; i < m_iMaxBandNum*m_iMaxBandNum; i++)
			*pSHData++ = m_pTableData[i];

		return S_OK;
	}














	HRESULT GenerateSH(D3DXVECTOR2 VecSphereCoord)
	{
		return GenerateSH(VecSphereCoord.x, VecSphereCoord.y);
	}

	// ��һ�������Ǻʹ�ֱ�ᣨZ���ļнǣ��ڶ����������ڵ��棨XZ����ͶӰ�ͺ��ᣨX���ļн�
	// �˺��������ޱȣ�����ֻҪ�ǲ���Ҫ�ı�Ĳ������ߣ��������һ�δ�Ϊ�ļ���ÿ��Ԥ�����ʱ��ֱ�Ӷ��ļ�����
	HRESULT GenerateSH(float fTheta, float fPhi)
	{
		if(!m_bInit)
			return D3DERR_NOTAVAILABLE;

		m_fPhi = fPhi;
		m_fTheta = fTheta;

		// ������������Lengendre���
		HRESULT hr = GenerateLengendreMonomonial(cosf(fTheta));
		if( FAILED( hr ))
			return hr;

		// ��ÿ��Lengendre����ʽ��SH����Ȼ������ԭ����
		int l = 0, m = 0;
		UINT iAbsm = 0;
		float fPlm = 0.0f, fSH = 0.0f, fK = 0.0f, fTriangleFunc = 0.0f;
		float fTemp1 = 0.0f, fTemp2 = 0.0f;

		for(l=0; l<(int)m_iMaxBandNum; l++)
			for(m=-l; m<=l; m++)
			{
				iAbsm = (m<0) ? -m : m;

				// ����K
				fTemp1 = (float)CalcFactorial(l-iAbsm) / (float)CalcFactorial(l+iAbsm);
				fTemp2 = (float)(2.0f*l+1.0f) / (4.0f*D3DX_PI);
				fK = sqrtf(fTemp2 * fTemp1);

				// �������Ǻ���
				if(m < 0)
					fTriangleFunc = 1.41421356237f * sin(iAbsm * fPhi);
				else if(m > 0)
					fTriangleFunc = 1.41421356237f * cos(iAbsm * fPhi);
				else
					fTriangleFunc = 1.0f;

				//float e = 1.0f;
				//fTriangleFunc = powf(e, m * fPhi);

				// �õ�P(l,abs(m))
				fPlm = GetLengendreMonomonial(l, iAbsm);
				// ����SH
				fSH = fK * fTriangleFunc * fPlm;
				if( !SetLengendreMonomonial(l, m, fSH) )
					return E_FAIL;
			}

			// ���ɽ��������m_pTableData
			float *pData = m_pTableData;
			for(l=0; l<(int)m_iMaxBandNum; l++)
			{
				for(m=-l; m<=l; m++)
				{
					*pData = GetLengendreMonomonial(l, m);
					pData++;
				}
			}

			m_bGenerate = TRUE;
			return S_OK;
	}

	// Get Shperical Harmonic Monomonial Value��iBand From 0 To m_iMaxBand-1
	float GetSH(UINT iBand, int iIndex)
	{
		if(!m_bInit || !m_bGenerate)
			return 0.0f;

		if((UINT)abs(iIndex) > iBand)
			return 0.0f;

		float fSH = GetLengendreMonomonial(iBand, iIndex);
		return fSH;
	}

	// Get Shperical Harmonics, All Polymonials
	float *GetSHTable()
	{
		if(!m_bInit || !m_pTableData || !m_bGenerate)
			return NULL;

		return m_pTableData;
	}


private:
	float *m_pTableData;		// ����ָ�룬��pTable������ȫһ�£�ֻ�ǰ�˳�����һ��ģ�����ʹ��
	float **m_pTable;			// ��ͷָ�룬ÿ�ű�������Bandͷָ�룬ָ��ÿ��Band�����ݣ���̬���㣬��������Ƶ�pTableData
	UINT m_iMaxBandNum;			// ��������Band��ţ��û�ָ��
	float m_fPhi, m_fTheta;		// SH����
	BOOL m_bInit, m_bGenerate;	// ���


	// Generate
	HRESULT GenerateLengendreMonomonial(float fX)
	{
		if(!m_bInit)
			return D3DERR_NOTAVAILABLE;

		int l = 0, m = 0, iTemp1 = 0, iTemp2 = 0;
		float fTemp1 = 0.0f, fTemp2 = 0.0f, fTemp3 = 0.0f, fValue = 0.0f;

		// �Ȱ��չ���b����ÿ��Bandβ��������
		for(l=0,m=0; l<(int)m_iMaxBandNum; l++,m++)
		{
			if( l==0 && m==0 )
				SetLengendreMonomonial(l, m, 1.0f);
			else
			{
				fTemp1 = (float)pow(-1.0, m);

				iTemp1 = CalcDoubleFactorial(2*m - 1);
				fTemp2 = (float)iTemp1;

				fTemp3 = 1 - fX * fX;
				fTemp3 = powf(fTemp3, (float)m / 2.0f);

				fTemp1 = fTemp1 * fTemp2 * fTemp3;
				SetLengendreMonomonial(l, m, fTemp1);
			}
		}

		// �ٸ��ݹ���c���������ɵ�β�������ɸ�Band�����ڶ���������
		float fLowBand1 = 0.0f, fLowBand2 = 0.0f;
		for(m=0; m<(int)m_iMaxBandNum-1; m++)
		{
			l = m + 1;
			fTemp1 = fX * ( 2*(float)m + 1 );
			fLowBand1 = GetLengendreMonomonial(m, m);
			fTemp1 *= fLowBand1;
			SetLengendreMonomonial(l, m, fTemp1);
		}

		// ���չ���a��������������
		for(l=2; l<(int)m_iMaxBandNum; l++)
		{
			for(m=0; m<=(l-2); m++)
			{
				fLowBand1 = GetLengendreMonomonial(l-1, m);
				fTemp1 = fX * (float)(2*l-1) * fLowBand1;

				fLowBand2 = GetLengendreMonomonial(l-2, m);
				fTemp2 = (float)(l+m-1) * fLowBand2;

				fTemp3 = (fTemp1 - fTemp2) / (float)(l - m);
				SetLengendreMonomonial(l, m, fTemp3);
			}
		}


		// �������еĸ�Index����ʵ�ò��ϣ�ֻ�ǳ�ʼ��һ�°���
		for(l=0; l<(int)m_iMaxBandNum; l++)
		{
			for(m=-l; m<0; m++)
			{
				fTemp1 = GetLengendreMonomonial(l, abs(m));
				SetLengendreMonomonial(l, m, 0.0f);
			}
		}

		// ���ɽ��������m_pTableData
		float *pData = m_pTableData;
		for(l=0; l<(int)m_iMaxBandNum; l++)
		{
			for(m=-l; m<=l; m++)
			{
				*pData = GetLengendreMonomonial(l, m);
				pData++;
			}
		}

		return S_OK;
	}



	// Get Standard Associated Lengendre Polymonial Value, Must Init First
	BOOL SetLengendreMonomonial(UINT iBand, int iIndex, float fValue)
	{
		if(!m_bInit)
			return FALSE;
		if((UINT)abs(iIndex) > iBand)
			return FALSE;

		m_pTable[iBand][GetMemPosInBand(iBand, iIndex)] = fValue;
		return TRUE;
	}
	float GetLengendreMonomonial(UINT iBand, int iIndex)
	{
		if(!m_bInit)
			return 0.0f;
		if((UINT)abs(iIndex) > iBand)
			return 0.0f;

		return m_pTable[iBand][GetMemPosInBand(iBand, iIndex)];
	}



	// Get Total Band Monomonial Num
	UINT GetTotalMonoNum()
	{
		return m_iMaxBandNum * m_iMaxBandNum;
	}
	// Get Single Band Monomonial Num, Must Init First
	UINT GetBandMonoNum(UINT iBand)
	{
		return iBand * 2 + 1;
	}
	// Get Position From Memory in a single band, as if -Band = 0
	int GetMemPosInBand(UINT iBand, int iIndex)
	{
		if((UINT)abs(iIndex) > iBand)
			return 0;

		return iIndex + (int)iBand;
	}
	// Get Position From Memory in all bands, as if -Band = 0
	UINT GetMemPosInTotal(UINT iBand, int iIndex)
	{
		if((UINT)abs(iIndex) > iBand)
			return 0;

		UINT iNum = iBand * iBand;
		iNum += GetMemPosInBand(iBand, iIndex);
		return iNum;
	}
};























typedef SHTable* LPSHTABLE;
// Monte Carlo Intergration, Random Samples, Each sample is a Spherical Coordinate
class KMCSample
{
public:
	KMCSample()
	{
		m_bInit = FALSE;
		m_iSampleNum = 0;
		m_pVecSamples = NULL;
		m_pVecSamplesCarte = NULL;
		m_pSHTable = NULL;
	}
	~KMCSample()
	{
		Release();
	}

	void Release()
	{
		m_bInit = FALSE;
		m_iSampleNum = 0;
		SAFE_DELETE_ARRAY(m_pVecSamples);
		SAFE_DELETE_ARRAY(m_pVecSamplesCarte);
		SAFE_DELETE_ARRAY(m_pSHTable);
	}


	HRESULT Init(UINT iSampleNum, UINT iBandNum)
	{
		if(iSampleNum < MYSH_MINSAMPLENUM || iSampleNum > MYSH_MAXSAMPLENUM)
			return D3DERR_INVALIDCALL;
		if(m_bInit)
			return D3DERR_NOTAVAILABLE;

		float fSqrt = sqrtf((float)iSampleNum);
		float fSqrt_Int = (float)((UINT)fSqrt);
		if((fSqrt-fSqrt_Int) > 0.000001f)
		{
			OutputDebugString("��������������ĳ��������ƽ����");
			return D3DERR_INVALIDCALL;
		}

		SAFE_DELETE_ARRAY(m_pVecSamples);
		SAFE_DELETE_ARRAY(m_pVecSamplesCarte);
		SAFE_DELETE_ARRAY(m_pSHTable);

		m_pVecSamples = new D3DXVECTOR2[iSampleNum];
		m_pVecSamplesCarte = new D3DXVECTOR3[iSampleNum];
		m_pSHTable = new SHTable[iSampleNum];
		if(!m_pVecSamples || !m_pSHTable || !m_pVecSamplesCarte)
		{
			SAFE_DELETE_ARRAY(m_pVecSamples);
			SAFE_DELETE_ARRAY(m_pVecSamplesCarte);
			SAFE_DELETE_ARRAY(m_pSHTable);
			return E_OUTOFMEMORY;
		}

/*
		// Generate Hemi-Sphere Sample Point/Ray in Spherical Coordinate
		float fTheta = 0.0f, fPhi = 0.0f;
		for(UINT i = 0; i < iSampleNum; i++)
		{
			// fTheta is the angle between vector and Z-axis, range from 0 to Pi in Sphere
			fTheta = randomf(D3DX_PI);
			// fPhi is the angle between Projection vector and X-axis, range from 0 to 2Pi
			fPhi = randomf(2.0f * D3DX_PI);

			m_pVecSamples[i] = D3DXVECTOR2(fTheta, fPhi);
			SphericalToCartesian(m_pVecSamples[i], &m_pVecSamplesCarte[i]);
			if(FAILED(m_pSHTable[i].Init(iBandNum)))
				return E_FAIL;
			if(FAILED(m_pSHTable[i].GenerateSH(fTheta, fPhi)))
				return E_FAIL;
		}
*/

		// Generate Hemi-Sphere Sample Point/Ray in Spherical Coordinate
		float fTheta = 0.0f, fPhi = 0.0f;
		float fX = 0.0f, fY = 0.0f;
		UINT iIndex = 0;

		for(UINT j = 0; j < (UINT)fSqrt; j++)
			for(UINT i = 0; i < (UINT)fSqrt; i++)
			{
				fX = ((float)i + randomf(1.0f)) / fSqrt;
				fY = ((float)j + randomf(1.0f)) / fSqrt;
				// fTheta is the angle between vector and Z-axis, range from 0 to Pi in Sphere
				fTheta = 2.0f * acosf(sqrtf(1.0f - fX));
				// fPhi is the angle between Projection vector and X-axis, range from 0 to 2Pi
				fPhi = 2.0f * D3DX_PI * fY;

				// Fill Data
				iIndex = (UINT)fSqrt * j + i;
				m_pVecSamples[iIndex] = D3DXVECTOR2(fTheta, fPhi);
				SphericalToCartesian(m_pVecSamples[iIndex], &m_pVecSamplesCarte[iIndex]);
				if(FAILED(m_pSHTable[iIndex].Init(iBandNum)))
					return E_FAIL;
				if(FAILED(m_pSHTable[iIndex].GenerateSH(fTheta, fPhi)))
					return E_FAIL;
		}

		m_iSampleNum = iSampleNum;
		m_bInit = TRUE;
		return S_OK;
	}

	// Get Sample in Spherical Coordinate
	HRESULT GetSample(UINT iSampleNo, LPD3DXVECTOR2 pVecSample)
	{
		if(!m_bInit || !m_pVecSamples || iSampleNo > m_iSampleNum || !pVecSample)
			return D3DERR_INVALIDCALL;

		*pVecSample = m_pVecSamples[iSampleNo];
		return S_OK;
	}

	// Get Sample in Cartesian Coordinate
	HRESULT GetSample(UINT iSampleNo, LPD3DXVECTOR3 pVecSample)
	{
		if(!m_bInit || !m_pVecSamplesCarte || iSampleNo > m_iSampleNum || !pVecSample)
			return D3DERR_INVALIDCALL;

		*pVecSample = m_pVecSamplesCarte[iSampleNo];
		return S_OK;
	}
	float *GetSHTable(UINT iSampleNo)
	{
		if(!m_bInit || !m_pSHTable || iSampleNo > m_iSampleNum)
			return NULL;

		return m_pSHTable[iSampleNo].GetSHTable();
	}

	UINT GetSampleNum()
	{
		return m_iSampleNum;
	}

private:
	BOOL			m_bInit;
	UINT			m_iSampleNum;
	LPD3DXVECTOR2	m_pVecSamples;	// Spherical Coordinate
	LPD3DXVECTOR3	m_pVecSamplesCarte;	// Cartesian Coordinate
	LPSHTABLE		m_pSHTable;		// Y(Sample), one Y for each sample
};













// ������������һ��Y������PS�в�������LDPRT��Ҳ����ֱ����GPU��Project Cubemap
// Ҫ��ǳ��ߣ�����Ҫ��FP32��Cubemap����MRT��ps2.a����ʵ����sm3.0��
// ����Ϊ�˷�����࣬���к�SH��CubeMap��صĶ��ŵ���������
class KSHCubeMap
{
public:
	KSHCubeMap()
	{
		m_ppCubeMapSH = NULL;
		m_bInit = FALSE;
		m_iBandNum = 0;
		m_iSampleNum = 0;
		m_iTextureNum = 0;

		m_pVB = NULL;
		m_pVertexDeclaration = NULL;
		m_iVBStride = 0;

		m_pTexSamples = NULL;
		m_pTexSystemMem = NULL;
		m_pTexProjectR = m_pTexProjectG = m_pTexProjectB = NULL;
	}
	~KSHCubeMap()
	{
		Release();
	}

	void Release()
	{
		for(UINT i = 0; i < m_iTextureNum; i++)
			SAFE_RELEASE(m_ppCubeMapSH[i]);
		SAFE_DELETE_ARRAY(m_ppCubeMapSH);

		m_bInit = FALSE;
		m_iBandNum = 0;
		m_iSampleNum = 0;
		m_iTextureNum = 0;

		SAFE_RELEASE(m_pVB);
		m_iVBStride = 0;

		SAFE_RELEASE(m_pTexSamples);
		
		SAFE_RELEASE(m_pTexProjectR);
		SAFE_RELEASE(m_pTexProjectG);
		SAFE_RELEASE(m_pTexProjectB);

		SAFE_RELEASE(m_pTexSystemMem);

		SAFE_RELEASE(m_pVertexDeclaration);
		m_PS.Release();
	}


	HRESULT Init(UINT iBandNum, UINT iSampleNum, char *szPS, UINT iResolution = 64);

	// Project a Cubemap use GPU������ʹ���ڲ���SH CubeMap�����Բ�����Static
	HRESULT GPUProjectCubeMap(LPDIRECT3DCUBETEXTURE9 pCubeMap, float *pSHCoefficients[3]);



	// ���漸��ֻ�ǹ��ߺ�����Ϊ�˷�������ǹ��ൽ������
		// Project a cubemap into Spherical Harmonics(SH) Basis
	static HRESULT ProjectCubeMap(LPDIRECT3DCUBETEXTURE9 pCubeMap, UINT iSampleNum, UINT iBandNum, float *pSHCoefficients[3]);
		// ����ͬ�ϣ�ֻ�����������صģ�������������㣬����Ҫ���ܶ�
	static HRESULT ProjectCubeMap(LPDIRECT3DCUBETEXTURE9 pCubeMap, UINT iBandNum, float *pSHCoefficients[3]);
		// ����SHϵ���ع�һ��CubeMap��������������
	static HRESULT ReconstructCubeMap(LPDIRECT3DCUBETEXTURE9 pCubeMap, UINT iBandNum, float *pSHCoefficients[3]);
	

private:
	// Generate SH(Y) to Cubemap�� Samples to Texture, �ڲ�ʹ��
	HRESULT GenerateSHCubeMap();
	HRESULT GenerateSampleMap();


	// ����SH�Ƕ�Band��һ��Cubemapֻ�ܴ�4��ͨ����4������ʽ������Ҫ������ͼ
	LPDIRECT3DCUBETEXTURE9 *m_ppCubeMapSH;
	UINT m_iBandNum, m_iSampleNum;
	UINT m_iTextureNum;
	BOOL m_bInit;
	KMCSample m_Samples;

	LPDIRECT3DVERTEXBUFFER9 m_pVB;	// ����N���㣨��һ���ߣ��ֱ���0��3��4��7��8��11������Project����ʽ��N����Band���Ĳ�ͬ����ͬ����ʵ��N=m_iTextureNum��
	UINT m_iVBStride;				// VBÿ�������Stride���ڲ�ʹ��

	LPDIRECT3DVERTEXDECLARATION9 m_pVertexDeclaration;
	PIXELSHADER m_PS;	// ����GPU Projection��Pixel Shader����Ϊ������Ļ���꣬����Vetex Shader���ԣ�

	LPDIRECT3DTEXTURE9	m_pTexSamples;	// ��Ų��������ݣ�������������SampleNum * 1��С
	
	LPDIRECT3DTEXTURE9	m_pTexProjectR;	// ����������ͶӰ�����������ÿ��������N���㣨��N*1��С��N������m_pVB��Nһ�£�����MRTһ��д�룬Ȼ���Ƶ��������ʱ����Lock������SH����ֵ
	LPDIRECT3DTEXTURE9	m_pTexProjectG;
	LPDIRECT3DTEXTURE9	m_pTexProjectB;

	LPDIRECT3DTEXTURE9	m_pTexSystemMem; // ��ʱʹ�ã�����GetRenderTargetData��Lock

	
	// ���ߺ�����Ҫʹ�õ��ڲ�����
	static HRESULT OperateCubeMapPixelData(BOOL bGet, LPDIRECT3DCUBETEXTURE9 pCubeMap, D3DXVECTOR3 VecRay, void *pPixelData, UINT iPixelSize /* = 4 */, UINT iMip /* = 0 */);
	static HRESULT GetCubeMapVector(D3DCUBEMAP_FACES iFace, UINT iResolutionX, UINT iResolutionY, UINT iX, UINT iY, LPD3DXVECTOR3 pVecCoord);
	static HRESULT GetCubeMapPixelData(LPDIRECT3DCUBETEXTURE9 pCubeMap, D3DXVECTOR3 VecRay, void *pPixelData, UINT iPixelSize = 4, UINT iMip = 0);
	static HRESULT SetCubeMapPixelData(LPDIRECT3DCUBETEXTURE9 pCubeMap, D3DXVECTOR3 VecRay, void *pPixelData, UINT iPixelSize = 4, UINT iMip = 0);

};