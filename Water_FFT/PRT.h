#pragma once

class KBaseMesh;

#define MYPRT_REDCHANNEL 0
#define MYPRT_GREENCHANNEL 1
#define MYPRT_BLUECHANNEL 2

#define MYPRT_USED3DXPRTINTERSECT		// ʹ��D3DX�е��󽻺�����ͬ����Ч����Ч��ȴ���Լ�д�ĺ����߼�ʮ������-_-���ˣ�������

typedef struct PRTMESH
{
	LPD3DXMESH pMesh;
	D3DXMATRIX MatRotation;
	D3DXMATRIX MatTranslation;
	D3DXMATRIX MatScaling;	// �мǲ���������ţ����ģ���а������������ǿ�����ŵĽ�����ǵ���ģ�Ϳ����֮��λ�ô���
	PRTATTRIBUTE Options;
	
	char szMeshName[MAX_PATH];
	
	PRTMESH()
	{
		pMesh = NULL;
		D3DXMatrixIdentity(&MatRotation);
		D3DXMatrixIdentity(&MatTranslation);
		D3DXMatrixIdentity(&MatScaling);
		strcpy(szMeshName, "");
	}
	~PRTMESH()
	{
		pMesh = NULL;
		D3DXMatrixIdentity(&MatRotation);
		D3DXMatrixIdentity(&MatTranslation);
		D3DXMatrixIdentity(&MatScaling);
		strcpy(szMeshName, "");
	}

}* LPPRTMESH;





// ������һ��Buffer����Ҫ��������Ϣ
typedef struct PRTBUFFER
{
	BOOL m_bInit;								// Sign
	float *m_pBufferData[3];					// R,G,B��PRT Data
	UINT m_iStride;								// Buffer Size per vertex: Bytes (According to BandNum and LDPRT LobeNum)
	UINT m_iStridePRT;							// PRT size per vertex (before LDPRT)
	UINT m_iBandNum, m_iLobeNum;				// BandNum and LDPRT Lobe Num
	UINT m_iVertexNum;							// Mesh VertexNum
	PRTATTRIBUTE m_Options;						// ��Buffer��Ӧ��PRT Attrib

	// �ṹ��	ǰ��m_iStridePRT���ֽ���PRT���ݣ�iBand*iBand����������ռ��iBand*iBand/4������Ĵ���
	//			������LDPRT���ݣ�ռ��m_Stride - m_StridePRT���ֽڣ�iBand*iLobeNum����������ռ��iBand*iLobeNum/4������Ĵ���
	//			�ܼ�m_iStride���ֽڣ����m_iStride = m_iStridePRT��˵��û��LDPRT��ֻ��PRT
	LPDIRECT3DVERTEXBUFFER9 m_pPRTBuffer[3];	// Temporary create from PRTData��Using in Final Rendering

	PRTBUFFER()
	{
		m_bInit = FALSE;
		m_pPRTBuffer[0] = m_pPRTBuffer[1] = m_pPRTBuffer[2] = NULL;
		m_pBufferData[0] = m_pBufferData[1] = m_pBufferData[2] = NULL;
		m_iStride = m_iStridePRT = 0;
		m_iBandNum = 0;
		m_iLobeNum = 0;
		m_iVertexNum = 0;
	}
	~PRTBUFFER()
	{
		Release();
	}

	void Release()
	{
		m_bInit = FALSE;
		SAFE_RELEASE(m_pPRTBuffer[0]);
		SAFE_RELEASE(m_pPRTBuffer[1]);
		SAFE_RELEASE(m_pPRTBuffer[2]);
		SAFE_DELETE_ARRAY(m_pBufferData[0]);
		SAFE_DELETE_ARRAY(m_pBufferData[1]);
		SAFE_DELETE_ARRAY(m_pBufferData[2]);
		m_iStride = m_iStridePRT = 0;
		m_iBandNum = 0;
		m_iLobeNum = 0;
		m_iVertexNum = 0;
	}

	HRESULT Init(UINT iBandNum, LPPRTMESH pPRTMesh)
	{
		if(m_bInit)
			return D3DERR_NOTAVAILABLE;
		if(!pPRTMesh)
			return D3DERR_INVALIDCALL;
		if(iBandNum > MYSH_MAXORDER || iBandNum < MYSH_MINORDER)
			return D3DERR_INVALIDCALL;
		if(pPRTMesh->Options.iBounceNum > MYPRT_MAXBOUNCE || pPRTMesh->Options.iLDPRTLobeNum > MYLDPRT_MAXLOBE)
			return D3DERR_INVALIDCALL;

		// ����LDPRT��OnlyLDPRT�����һ���������£��������
		UINT iLobeNum = pPRTMesh->Options.iLDPRTLobeNum;
		UINT iMonomoniaNum = iBandNum * iBandNum +  iLobeNum * iBandNum;

		m_iStride = iMonomoniaNum * 4;
		m_iStridePRT = iBandNum * iBandNum * 4;

		UINT iVertexNum = pPRTMesh->pMesh->GetNumVertices();

		for(UINT i = 0; i < 3; i++)
		{
			// Create Vertex Buffer
			SAFE_RELEASE(m_pPRTBuffer[i]);
			if(FAILED(d3ddevice->CreateVertexBuffer(m_iStride * iVertexNum, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pPRTBuffer[i], NULL)))
				return D3DERR_OUTOFVIDEOMEMORY;

			// Create Data Buffer
			SAFE_DELETE_ARRAY(m_pBufferData[i]);
			m_pBufferData[i] = new float[iMonomoniaNum * iVertexNum];
			if(!m_pBufferData[i])
				return E_OUTOFMEMORY;
			// Clear
			if(FAILED(SetToZero(i)))
				return E_FAIL;
		}

		m_iBandNum = iBandNum;
		m_iLobeNum = iLobeNum;
		m_iVertexNum = iVertexNum;

		memcpy(&m_Options, &pPRTMesh->Options, sizeof(PRTATTRIBUTE));

		m_bInit = TRUE;
		return S_OK;
	}



	// ������PRT Simulation֮��ִ��
	HRESULT SaveBufferToFile(char *pszFileName)
	{
		// ��Ч�Լ��
		if(!m_bInit || !m_iVertexNum || !m_iBandNum)
			return D3DERR_NOTAVAILABLE;
		if(!pszFileName)
			return D3DERR_INVALIDCALL;
		if(m_Options.iLDPRTLobeNum != m_iLobeNum)
			return D3DERR_INVALIDCALL;

		// ���ļ�
		FILE *fp = fopen(pszFileName, "wb");
		if(!fp)
			return E_FAIL;

		// ���е��ַ���������\0
		// �ļ�ͷ��Old: "PRT"+ SampleNum + BandNum + LobeNum + VertexNum + FaceNum + PRTATTRIBUTE
		// New:			"PRT"+ BandNum + LobeNum + VertexNum + PRTATTRIBUTE
		UINT ii = 0;
		if(ii=fwrite("PRT", 3, 1, fp) != 1)
			return E_FAIL;
		if(fwrite(&m_iBandNum, 4, 1, fp) != 1)
			return E_FAIL;
		if(fwrite(&m_iLobeNum, 4, 1, fp) != 1)
			return E_FAIL;
		if(fwrite(&m_iVertexNum, 4, 1, fp) != 1)
			return E_FAIL;
		if(fwrite(&m_Options, sizeof(PRTATTRIBUTE), 1, fp) != 1)
			return E_FAIL;


		// �������ݣ�"R/G/B" + Stride + sizeof PRTBuffer1 + PRTBufferData1����3��Buffer��ʾRGB
		UINT iWriteNum = 0, iBufferSize = 0;
		char szBufferName[3] = {'R', 'G', 'B'};
		for(UINT iIndex = 0; iIndex < 3; iIndex++)
		{
			if(fwrite(&szBufferName[iIndex], 1, 1, fp) != 1)
				return E_FAIL;
			if(fwrite(&m_iStride, 4, 1, fp) != 1)
				return E_FAIL;
			iBufferSize = m_iStride * m_iVertexNum;
			if(fwrite(&iBufferSize, 4, 1, fp) != 1)
				return E_FAIL;

			iWriteNum = fwrite(m_pBufferData[iIndex], m_iStride, m_iVertexNum, fp);
			if(iWriteNum != m_iVertexNum)
				return E_FAIL;
		}

		// ����
		fclose(fp);
		return S_OK;
	}

	// ע��ԭ�е�PRT Buffer��Attrib���ݻᱻ�ļ��е����ݸ���
	// ֻҪ��ʼ���������Զ�ζ�ȡ������Ҫע���ʼ����ģ�����ݺ�Buffer���ݵ�ƥ��Ŷ
	HRESULT LoadBufferFromFile(char *pszFileName)
	{
		if(!m_bInit)
			return D3DERR_NOTAVAILABLE;
		if(!pszFileName)
			return D3DERR_INVALIDCALL;


		// ���ļ�
		FILE *fp = fopen(pszFileName, "rb");
		if(!fp)
			return E_FAIL;

		// ���е��ַ���������\0
		// �ļ�ͷ��"PRT"+ SampleNum + BandNum + LobeNum + VertexNum + FaceNum + PRTATTRIBUTE
		UINT iData = 0;

		char szName[10] = "";
		if(fread(szName, 3, 1, fp) != 1)
			return E_FAIL;
		if(szName[0] != 'P' || szName[1] != 'R' || szName[2] != 'T')
			return E_FAIL;

		UINT iBandNum = 0;
		if(fread(&iBandNum, 4, 1, fp) != 1)
			return E_FAIL;
		if(iBandNum != m_iBandNum)
		{
			OutputDebugString("�ļ����ݺ͵�ǰBuffer��ƥ�䣨Band����һ�£���\n");
			return E_FAIL;
		}

		UINT iLobeNum = 0, iVertexNum = 0;
		if(fread(&iLobeNum, 4, 1, fp) != 1)
			return E_FAIL;
		if(fread(&iVertexNum, 4, 1, fp) != 1)
			return E_FAIL;
		if(iVertexNum != m_iVertexNum)
		{
			OutputDebugString("�ļ����ݺ͵�ǰBuffer��ƥ�䣨��������һ�£���\n");
			return E_FAIL;
		}

		if(fread(&m_Options, sizeof(PRTATTRIBUTE), 1, fp) != 1)
			return E_FAIL;
		if(m_Options.iLDPRTLobeNum != m_iLobeNum)
		{
			OutputDebugString("�ļ����ݺ͵�ǰBuffer��ƥ�䣨LDPRT Lobe����һ�£���\n");
				return E_FAIL;
		}
		if(m_Options.iBounceNum > MYPRT_MAXBOUNCE)
		{
			OutputDebugString("�ļ����ݲ��Ϸ��������������󣩣�\n");
			return E_FAIL;
		}

		// �������ݣ�"R/G/B" + Stride + sizeof PRTBuffer1 + PRTBufferData1����3��Buffer��ʾRGB
		UINT iReadNum = 0, iBufferSize = 0;
		char szBufferName[3] = {'R', 'G', 'B'};
		char szReadBuffer[3] = "";
		for(UINT iIndex = 0; iIndex < 3; iIndex++)
		{
			// У��BUFFERͷ
			if(fread(&szReadBuffer[iIndex], 1, 1, fp) != 1)
				return E_FAIL;
			if(szReadBuffer[iIndex] != szBufferName[iIndex])
				return E_FAIL;

			// ����У�飬Stride��BufferSize��ǰ���Ѿ�������ˣ������ٴμ���Ƿ����
			if(fread(&iData, 4, 1, fp) != 1)
				return E_FAIL;
			if(!iData || m_iStride != iData)
			{
				OutputDebugString("�ļ����ݺ͵�ǰBuffer��ƥ�䣨Stride��һ�£���\n");
				return E_FAIL;
			}

			iBufferSize = m_iStride * m_iVertexNum;
			if(fread(&iData, 4, 1, fp) != 1)
				return E_FAIL;
			if(!iData || iData != iBufferSize)
			{
				OutputDebugString("�ļ����ݺ͵�ǰBuffer��ƥ�䣨���ݴ�С��һ�£���\n");
				return E_FAIL;
			}

			// ��ȡ����
			iReadNum = fread(m_pBufferData[iIndex], m_iStride, m_iVertexNum, fp);
			if(iReadNum != m_iVertexNum)
				return E_FAIL;
		}

		if(FAILED(ApplyToVB()))
			return E_FAIL;


		// ���������ֻ���PRT���ݣ���mesh�йص�����ȫ���ǿյģ����Բ���simulate
		fclose(fp);
		return S_OK;
	}








	// �õ�����PRTϵ������ָ����ָ���еõ����ݣ����Ƶ�pOutData������Ԥ�ȷ���ÿռ䣩������Ӱ��LDPRT����
	// Stride�ڿ�ʼ�ͼ�����ˣ�ÿ��Buffer��RGB������һ����
	HRESULT GetVertexPRTInfo(UINT iVertexNo, float *pOutData, UINT iIndex = MYPRT_REDCHANNEL)
	{
		if(!m_bInit || !pOutData || iIndex > 2)
			return D3DERR_INVALIDCALL;

		if(iVertexNo >= m_iVertexNum)
			return D3DERR_INVALIDCALL;

		// Get
		float *pData = m_pBufferData[iIndex] + m_iStride/4 * iVertexNo;
		memcpy(pOutData, pData, m_iBandNum * m_iBandNum * sizeof(float));

		return S_OK;	
	}

	// Set R/G/B PRT Coefficients��Index 0/1/2 indicates R/G/B seperately
	// no LDPRT
	HRESULT SetVertexPRTInfo(UINT iVertexNo, float *pInData, UINT iIndex = MYPRT_REDCHANNEL)
	{
		if(!m_bInit || !pInData || iIndex > 2)
			return D3DERR_INVALIDCALL;

		if(iVertexNo >= m_iVertexNum)
			return D3DERR_INVALIDCALL;

		// Set
		float *pData = m_pBufferData[iIndex] + m_iStride/4 * iVertexNo;
		memcpy(pData, pInData, m_iBandNum * m_iBandNum * sizeof(float));

		return S_OK;
	}


	// Get LDPRT Data, no influence of PRT
	// pOutData Must Allocate iBandNum * iLobeNum float buffer first
	HRESULT GetVertexLDPRTInfo(UINT iVertexNo, float *pOutData, UINT iIndex = MYPRT_REDCHANNEL)
	{
		if(iVertexNo >= m_iVertexNum || !pOutData || iIndex > 2)
			return D3DERR_INVALIDCALL;

		if(!m_bInit || !m_iLobeNum)
			return D3DERR_NOTAVAILABLE;

		// Locate Vertex Data Head
		float *pData = m_pBufferData[iIndex] + m_iStride/4 * iVertexNo;
		// Skip PRT Data, Locate to LDPRT Data
		pData += m_iBandNum * m_iBandNum;
		// Get
		memcpy(pOutData, pData, m_iBandNum * m_iLobeNum * sizeof(float));
		return S_OK;	
	}

	// Set LDPRT Data, no influence of PRT
	// pOutData Must Allocate iBandNum * iLobeNum float buffer first
	HRESULT SetVertexLDPRTInfo(UINT iVertexNo, float *pInData, UINT iIndex = MYPRT_REDCHANNEL)
	{
		if(iVertexNo >= m_iVertexNum || !pInData || iIndex > 2)
			return D3DERR_INVALIDCALL;

		if(!m_bInit || !m_iLobeNum)
			return D3DERR_NOTAVAILABLE;

		// Locate Vertex Data Head
		float *pData = m_pBufferData[iIndex] + m_iStride/4 * iVertexNo;
		// Skip PRT Data, Locate to LDPRT Data
		pData += m_iBandNum * m_iBandNum;
		// Get
		memcpy(pData, pInData, m_iBandNum * m_iLobeNum * sizeof(float));

		return S_OK;	
	}




	// Copy R/G/B PRT Coefficients��Index 0/1/2 indicates R/G/B seperately
	// contain LDPRT
	HRESULT CopyPRTBuffer(UINT iSourceIndex, UINT iDestIndex)
	{
		if(!m_bInit || iDestIndex > 2 || iSourceIndex > 2 || iSourceIndex == iDestIndex)
			return D3DERR_INVALIDCALL;

		// Copy
		float *pSourceData = m_pBufferData[iSourceIndex];
		float *pDestData = m_pBufferData[iDestIndex];

		memcpy(pDestData, pSourceData, m_iVertexNum * m_iStride);

		return S_OK;	
	}


	// Multiply Albedo to all PRT Coefficients��Index 0/1/2 indicates R/G/B seperately
	// No LDPRT
	HRESULT MultiplyAlbedo(float fAlbedo, UINT iIndex = MYPRT_REDCHANNEL)
	{
		if(!m_bInit || iIndex > 2)
			return D3DERR_INVALIDCALL;

		// Multiply
		float *pData = m_pBufferData[iIndex];
		for(UINT i = 0; i < m_iVertexNum; i++)
		{
			pData = m_pBufferData[iIndex] + i * m_iStride/4;
			for(UINT j = 0; j < m_iBandNum*m_iBandNum; j++, pData++)
				*pData *= fAlbedo;
		}

		return S_OK;	
	}

	// Multiply Albedo to one vertex's PRT Coefficients��Index 0/1/2 indicates R/G/B seperately
	// No LDPRT
	HRESULT MultiplyVertexAlbedo(float fAlbedo, UINT iVertexNo, UINT iIndex = MYPRT_REDCHANNEL)
	{
		if(!m_bInit || iIndex > 2 || iVertexNo >= m_iVertexNum)
			return D3DERR_INVALIDCALL;

		// Multiply
		float *pData = m_pBufferData[iIndex] + iVertexNo * m_iStride/4;
		for(UINT j = 0; j < m_iBandNum*m_iBandNum; j++, pData++)
			*pData *= fAlbedo;

		return S_OK;	
	}



	// ������Buffer�ӵ��Լ�����, ��������ͬ��Band����Lobe��������
	// contain LDPRT
	HRESULT AddBuffer(UINT iSelfIndex, UINT iSourceIndex, PRTBUFFER *pSourceBuffer)
	{
		if(!pSourceBuffer || iSourceIndex > 2 || iSelfIndex > 2)
			return D3DERR_INVALIDCALL;
		if(!m_bInit || !pSourceBuffer->m_bInit || !pSourceBuffer->m_pPRTBuffer[iSourceIndex])
			return D3DERR_NOTAVAILABLE;

		// Pointer
		float *pSourceData = pSourceBuffer->m_pBufferData[iSourceIndex];
		float *pDestData = m_pBufferData[iSelfIndex];

		// Add
		for(UINT i = 0; i < m_iVertexNum * m_iStride / 4; i++, pSourceData++, pDestData++)
			*pDestData += *pSourceData;

		return S_OK;	
	}


	// Set to Zero��contain LDPRT
	HRESULT SetToZero(UINT iIndex = MYPRT_REDCHANNEL)
	{
		if(iIndex > 2)
			return D3DERR_INVALIDCALL;
		if(!m_pBufferData[iIndex])
			return D3DERR_NOTAVAILABLE;

		// Set
		float *pData = m_pBufferData[iIndex];
		for(UINT i = 0; i < m_iVertexNum * m_iStride / 4; i++, pData++)
			*pData = 0.0f;

		return S_OK;	
	}

	// ��PRT����������ϣ���Ҫִ�иú����������ݸ��Ƶ�VB���Ժ�Ϳ���ֱ����VB��Ⱦ������LDPRT
	HRESULT ApplyToVB()
	{
		if(!m_bInit)
			return D3DERR_INVALIDCALL;

		float *pSrcData = NULL, *pDstData = NULL;
		for(UINT iIndex = 0; iIndex < 3; iIndex++)
		{
			// Get Pointer
			pSrcData = m_pBufferData[iIndex];
			if(FAILED(m_pPRTBuffer[iIndex]->Lock(0, m_iVertexNum * m_iStride, (void **)&pDstData, 0)))
				return E_FAIL;
			// Copy
			for(UINT i = 0; i < m_iVertexNum * m_iStride / 4; i++, pDstData++, pSrcData++)
				*pDstData = *pSrcData;
			// Over
			if(FAILED(m_pPRTBuffer[iIndex]->Unlock()))
				return E_FAIL;
		}
		
		return S_OK;
	}



}* LPPRTBUFFER;










// Precomputed Radiance Transfer, include Self-Shadow Term, Light Bounce, Local-Deformable and Sub-Surface Scattering
// һ��ֻ֧�ֶ�һ��ģ�ͽ���PRT���㣬���ڹ����������ԣ���ҪԤ�ȶ�����ģ�ͼ��㣬Ȼ����SplitMesh��PRTBuffer�ָ�������ָ�Mesh�Ķ��㻺����ͬ��
class KPRTEngine
{
public:
	KPRTEngine()
	{
		m_bInit = m_bSimulated = FALSE;
		m_iBandNum = 0;
		m_iSampleNum = 0;
		m_iCurrentVertexNo = 0;
		m_iVertexNum = m_iFaceNum = 0;
		m_pObject = NULL;
		m_pPRTBuffer = NULL;

		m_pD3DXPRTEngine = NULL;

		SAFE_DELETE_ARRAY(m_pNewVB);
		SAFE_DELETE_ARRAY(m_pNewIB);
		SAFE_DELETE_ARRAY(m_pDiffuse);

		SAFE_DELETE_ARRAY(m_pFaceVB);
		SAFE_DELETE_ARRAY(m_pFaceVertexInfo);
		SAFE_DELETE_ARRAY(m_pFaceNormal);

		SAFE_DELETE_ARRAY(m_pVertexFaceCountUpperSphere);
		SAFE_DELETE_ARRAY(m_pVertexFaceCountLowerSphere);

		SAFE_DELETE_ARRAY(m_ppVertexFaceInfoUpperSphere);
		SAFE_DELETE_ARRAY(m_ppVertexFaceInfoLowerSphere);
	}
	~KPRTEngine()
	{
		Release();
	}

	void Release()
	{
		m_bInit = m_bSimulated = FALSE;
		m_iBandNum = 0;
		m_iSampleNum = 0;
		m_iCurrentVertexNo = 0;
		m_iVertexNum = m_iFaceNum = 0;
		m_pObject = NULL;
		m_pPRTBuffer = NULL;
		m_MCSamples.Release();
		
		SAFE_RELEASE(m_pD3DXPRTEngine);

		SAFE_DELETE_ARRAY(m_pNewVB);
		SAFE_DELETE_ARRAY(m_pNewIB);
		SAFE_DELETE_ARRAY(m_pDiffuse);
		SAFE_DELETE_ARRAY(m_pFaceVB);
		SAFE_DELETE_ARRAY(m_pFaceVertexInfo);
		SAFE_DELETE_ARRAY(m_pFaceNormal);

		// �ͷŶ�ά����
		for(UINT i = 0; i < m_iVertexNum; i++)
		{
			SAFE_DELETE_ARRAY(m_ppVertexFaceInfoUpperSphere[i]);
			SAFE_DELETE_ARRAY(m_ppVertexFaceInfoLowerSphere[i]);
		}
		SAFE_DELETE_ARRAY(m_pVertexFaceCountUpperSphere);
		SAFE_DELETE_ARRAY(m_pVertexFaceCountLowerSphere);

		SAFE_DELETE_ARRAY(m_ppVertexFaceInfoUpperSphere);
		SAFE_DELETE_ARRAY(m_ppVertexFaceInfoLowerSphere);
	}


	HRESULT Init(UINT iBandNum, UINT iSampleNum)
	{
		if(m_bInit)
			return D3DERR_NOTAVAILABLE;
		if(!iSampleNum)
			return D3DERR_INVALIDCALL;
		if(iBandNum > MYSH_MAXORDER || iBandNum < MYSH_MINORDER)
			return D3DERR_INVALIDCALL;

		HRESULT hr = m_MCSamples.Init(iSampleNum, iBandNum);
		if(FAILED(hr))
			return hr;

		m_iSampleNum = iSampleNum;
		m_iBandNum = iBandNum;
		m_bInit = TRUE;
		return S_OK;
	}


	// ����ụ��Ӱ����������飬����ǰ���뽫ÿ������ĳ�ʼ�������úã�
	// ͨ��ÿ��KBaseMesh�õ���ʼ״̬��������ʱ���㻺�������� Mesh�ĳ�ʼ״̬��λ�ơ����š�����ת����ȥ������ProcessVertices��
	// Ȼ���ÿ��Mesh����Projection����ÿ�������Ӧ��PRT��LDPRTϵ�������´����Ķ���ϵ�����壨�ڲ���
	// �����֮��Ϳ���GetBuffer�ˣ�Getһ��֮�󣬿�����SetOption��Simulate����GetBuffer
	HRESULT PRTSimulation(LPPRTMESH pObject, LPPRTBUFFER pPRTBuffer);


private:
	// �ڲ�ʹ�ã���ʼ��ģ�������Ϣ������PRTSimulation�е����߸���
	HRESULT InitGeometry();


	// ����ļ����������Ǵ�ĳ��������Χ�����������ߣ�����һ�ξ��ܼ���һ�������TF������ͨ���ڲ��ļ������������Ƶ�ǰ����
	// �������ʼ��PRTֱ�ӹ������ݣ����ﲻ�˷����ʣ����Լ����������ֱ�ӹ��յķ��ն�
	HRESULT ComputeDirectLighting(LPPRTBUFFER pBuffer);
	// SSS���������SSS�ӵ��Ѽ���õ�PRT�����ϣ����ݵ�ǰ��������弰���㣬Χ�Ƹö�����������Χ���ж����SSS����
	// ��������ID3DXPRTENGINE
	HRESULT ComputeSSS(LPPRTBUFFER pInBuffer, LPPRTBUFFER pOutBuffer, LPPRTBUFFER pAddBuffer);
	// ������߷���
	HRESULT ComputeBounce(LPPRTBUFFER pInBuffer, LPPRTBUFFER pOutBuffer, LPPRTBUFFER pAddBuffer);

	// ��һ����������������ͳһ��������LDPRT
	// ��������õ�PRT���ݼ���LDPRT������������������ж��㣬���ݷ���������Single Lobe Fitting��������д��ԭPRT Buffer��PRT�����Ѱ���LDPRT���ݵĿռ䣩
	// ���������ֻҪLDPRT�����ڼ���ǰ�·���ռ䣬�����֮���ͷ�ԭPRTָ�룬��ָ��ָ���·����Buffer
	HRESULT ComputeLDPRT(LPPRTBUFFER pBuffer);




private:
	// ��ʼ����أ�һ����ʼ���Ͳ��ܸı�
	BOOL			m_bInit;				// �Ƿ��ʼ������������Init
	BOOL			m_bSimulated;			// �Ƿ�����PRT��������PRTSimulation
	UINT			m_iBandNum;				// Band��
	UINT			m_iSampleNum;			// Sample����
	KMCSample		m_MCSamples;			// Monte Carlo Random Samples

	// ������Щ������ʱʹ�õģ�ֻ������ʱ�������ݵ���;��Simulation�������ͷŵ��ˣ�ΩһĿ�ľ��ǰѽ�����������Ȼ��浽ָ����Buffer��
	// Simulation���
	LPPRTMESH		m_pObject;				// �����Mesh����
	LPPRTBUFFER		m_pPRTBuffer;			// PRTϵ�����ݣ��ڲ�����ã�Ȼ����ʱ���ظ�������ʹ�ã����������ͷſ��ƻ����ڸ�����
	PRTATTRIBUTE	m_Options;				// PRT��������ʵpOjbect�оͰ����˸������Ϊ�˷�����д����͵��������

	// �ڲ�ģ�����
	UINT			m_iCurrentVertexNo;		// ���ֵ��ʾ��ǰ����PRTSimulation�д�������ĸ����㣬�Ա����ڲ������������߸��ٵ�ʱ�������ö��㼰����������
	LPD3DXVECTOR3	m_pNewVB;				// ���徭��ԭʼ����任������ݣ��������߸��٣���������������ͷ���
	UINT			*m_pNewIB;				// ����ԭ������������壬���ڼ��٣���Lock̫����
	LPD3DXCOLOR		m_pDiffuse;				// ���������Diffuse���������Ͳ���ȫ�ֵ�Albedo�ˣ��������Բ����ڣ�����ģ���������ԣ�
	UINT			m_iVertexNum, m_iFaceNum;
	
	// �������
	LPD3DXVECTOR3	m_pFaceVB;				// �����ŵ����ݣ�ÿ�����������㣨���д����ظ����㣬ֻ���������٣���λ�úͷ������ݶ��������水˳����
											// �����i�����ŵĵ�j�����㣺����Ϊm_pFaceVB[i*6+j]������Ϊm_pFaceVB[i*6+3+j]
	UINT			*m_pFaceVertexInfo;		// �����ŵ����ݣ�ÿ���������������ţ����ڼ���
											// �����i�����ŵĵ�j�����㣺���Ϊm_pFaceVertexInfo[i*3+j]
	
	LPD3DXVECTOR3	m_pFaceNormal;			// �����Ǹ�FaceVB��ŵ������Ӧ�Ķ������ݣ��������ŵ���������ķ������ݣ���i����ķ��߾���pNormal[i]
	UINT			*m_pVertexFaceCountUpperSphere;	// ��������ÿ�������Ӧ�ģ�������/�°���������������ڿ���ѡ���棬��i�������Ӧ����/�°�����������pCount[i]
	UINT			*m_pVertexFaceCountLowerSphere;
	UINT			**m_ppVertexFaceInfoUpperSphere;	// ��������ÿ�������Ӧ�ģ�������/�°�������������б����ڿ���ѡ���棬�������߸����ٶ�
	UINT			**m_ppVertexFaceInfoLowerSphere;	// ��i�������Ӧ�ĵ�n�������棨n < pCount[i]������ppFaceInfo[i][n]


	// ��D3D������������
	LPD3DXPRTENGINE	m_pD3DXPRTEngine;

	



	// �ڲ�ʹ�ú���
	// �õ�������Ϣ����VBList�еõ���ʼ����任������Ϣ
	HRESULT KPRTEngine::GetVertexInfo(UINT iVertexNo, LPD3DXVECTOR3 pPtPosition, LPD3DXVECTOR3 pVecNormal)
	{
		if(!pPtPosition || !pVecNormal)
			return D3DERR_INVALIDCALL;
		if(!m_pObject->pMesh || !m_pNewVB)
			return D3DERR_NOTAVAILABLE;

		if(iVertexNo >= m_iVertexNum)
			return D3DERR_INVALIDCALL;

		LPD3DXVECTOR3 pData = m_pNewVB + iVertexNo * 2;
		*pPtPosition = *pData++;
		*pVecNormal = *pData++;

		return S_OK;	
	}

	// �õ�����Ϣ���ȵõ��������ٸ�������ֵ��VBList�еõ�������Ϣ��3���ֱ���A/B/C�㣬����˳������˳�򣩴�0��2
	HRESULT KPRTEngine::GetFaceInfo(UINT iFaceNo, UINT *pFaceVertexNo, D3DXVECTOR3 pPtPosition[3], D3DXVECTOR3 pVecNormal[3])
	{
		if(!pPtPosition || !pVecNormal)
			return D3DERR_INVALIDCALL;

		if(!m_pObject->pMesh || !m_pNewIB)
			return D3DERR_NOTAVAILABLE;

		if(iFaceNo >= m_iFaceNum)
			return D3DERR_INVALIDCALL;

		// Get Indics Information
		UINT *pData = m_pNewIB + iFaceNo * 3;

		for(UINT i = 0; i < 3; i++, pData++)
		{
			pFaceVertexNo[i] = *pData;
			if(FAILED(GetVertexInfo(*pData, &pPtPosition[i], &pVecNormal[i])))
				return E_FAIL;
		}

		return S_OK;	
	}



	// �õ��Ƿ����ཻ�����Ϣ��ûһ�����ཻ�Ļ�����FALSE��������һ�����ཻ�ͷ���TRUE����ͨ����ʼ���õ���KBaseMesh�б������������ڼ���Shadowed
	BOOL GetVisibleUpperSphere(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay);
	// �õ�������ཻ����Ϣ��ûһ�����ཻ�Ļ�����FALSE����ͨ����ʼ���õ���KBaseMesh�б��������������������Ƿ���ֵ����ʾ���ཻ�����ĸ����У���������ţ���������������ཻ���ȣ����ڼ���Bounce��SSS
	BOOL GetClosestIntersectPointUpperSphere(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, UINT *pFaceNo, LPD3DXVECTOR3 pPtBaryCentric, float *pLength);
	BOOL GetClosestIntersectPointLowerSphere(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, UINT *pFaceNo, LPD3DXVECTOR3 pPtBaryCentric, float *pLength);


	// ����Options�еĲ�����Ϣ�;���������ɢ�乱�׶�Rd������SSS
	// ����Zv Zr����ǰ���ݽ���������õģ�����Ϊ�˼��پͲ��ظ�������
	// Sv Sr����ǰ���ݵ�ǰ�ཻ����õ�ƫ�ƾ���
	// fSqrtExtinct = sqrt(3*absorb*scatter)
	float GetBSSRDF_Rd(float fSv, float fSr, float fZv[3], float fZr[3], float fSqrtExtinct[3], UINT iColorIndex)
	{
		static float fRd = 0.0f, fExtinct = 0.0f;
		fExtinct = m_Options.fScatteringCoef[iColorIndex] + m_Options.fAbsorbation[iColorIndex];
		// �⼸���ڷ�ĸ�ϵ�������Ϸ�
		if(fExtinct < 0.0000001f || fSv < 0.000001f || fSr < 0.000001f)
			return 0.0f;

//		float fExpSr = powf(2.71828183f, -fSqrtExtinct[iColorIndex] * fSr);
//		float fExpSv = powf(2.71828183f, -fSqrtExtinct[iColorIndex] * fSv);
		float fExpSr = expf(-fSqrtExtinct[iColorIndex] * fSr);
		float fExpSv = expf(-fSqrtExtinct[iColorIndex] * fSv);

		float fPart_r = fZr[iColorIndex] * (1 / fSr + fSqrtExtinct[iColorIndex]) * fExpSr / (fSr * fSr);
		float fPart_v = fZv[iColorIndex] * (1 / fSv + fSqrtExtinct[iColorIndex]) * fExpSv / (fSv * fSv);
		float fRdCoef = m_Options.fScatteringCoef[iColorIndex] / (4.0f * D3DX_PI * fExtinct);	// fRd��ǰ����ۺ�ϵ��
		fRd = fRdCoef * (fPart_r + fPart_v);

		return fRd;
	}

};