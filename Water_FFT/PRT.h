#pragma once

class KBaseMesh;

#define MYPRT_REDCHANNEL 0
#define MYPRT_GREENCHANNEL 1
#define MYPRT_BLUECHANNEL 2

#define MYPRT_USED3DXPRTINTERSECT		// 使用D3DX中的求交函数，同样的效果，效率却比自己写的函数高几十倍……-_-算了，认命吧

typedef struct PRTMESH
{
	LPD3DXMESH pMesh;
	D3DXMATRIX MatRotation;
	D3DXMATRIX MatTranslation;
	D3DXMATRIX MatScaling;	// 切记不能随便缩放！如果模型中包含多块封闭区域，强行缩放的结果就是导致模型块与块之间位置错乱
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





// 保存了一个Buffer所需要的所有信息
typedef struct PRTBUFFER
{
	BOOL m_bInit;								// Sign
	float *m_pBufferData[3];					// R,G,B，PRT Data
	UINT m_iStride;								// Buffer Size per vertex: Bytes (According to BandNum and LDPRT LobeNum)
	UINT m_iStridePRT;							// PRT size per vertex (before LDPRT)
	UINT m_iBandNum, m_iLobeNum;				// BandNum and LDPRT Lobe Num
	UINT m_iVertexNum;							// Mesh VertexNum
	PRTATTRIBUTE m_Options;						// 该Buffer对应的PRT Attrib

	// 结构：	前面m_iStridePRT个字节是PRT数据，iBand*iBand个浮点数，占用iBand*iBand/4个输入寄存器
	//			后面是LDPRT数据，占用m_Stride - m_StridePRT个字节，iBand*iLobeNum个浮点数，占用iBand*iLobeNum/4个输入寄存器
	//			总计m_iStride个字节，如果m_iStride = m_iStridePRT，说明没有LDPRT，只有PRT
	LPDIRECT3DVERTEXBUFFER9 m_pPRTBuffer[3];	// Temporary create from PRTData，Using in Final Rendering

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

		// 加上LDPRT，OnlyLDPRT是最后一步才做的事，这里忽略
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



	// 必须在PRT Simulation之后执行
	HRESULT SaveBufferToFile(char *pszFileName)
	{
		// 有效性检查
		if(!m_bInit || !m_iVertexNum || !m_iBandNum)
			return D3DERR_NOTAVAILABLE;
		if(!pszFileName)
			return D3DERR_INVALIDCALL;
		if(m_Options.iLDPRTLobeNum != m_iLobeNum)
			return D3DERR_INVALIDCALL;

		// 打开文件
		FILE *fp = fopen(pszFileName, "wb");
		if(!fp)
			return E_FAIL;

		// 所有的字符串都不带\0
		// 文件头：Old: "PRT"+ SampleNum + BandNum + LobeNum + VertexNum + FaceNum + PRTATTRIBUTE
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


		// 具体数据："R/G/B" + Stride + sizeof PRTBuffer1 + PRTBufferData1，共3个Buffer表示RGB
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

		// 结束
		fclose(fp);
		return S_OK;
	}

	// 注意原有的PRT Buffer和Attrib数据会被文件中的数据覆盖
	// 只要初始化过，可以多次读取，不过要注意初始化的模型数据和Buffer数据的匹配哦
	HRESULT LoadBufferFromFile(char *pszFileName)
	{
		if(!m_bInit)
			return D3DERR_NOTAVAILABLE;
		if(!pszFileName)
			return D3DERR_INVALIDCALL;


		// 打开文件
		FILE *fp = fopen(pszFileName, "rb");
		if(!fp)
			return E_FAIL;

		// 所有的字符串都不带\0
		// 文件头："PRT"+ SampleNum + BandNum + LobeNum + VertexNum + FaceNum + PRTATTRIBUTE
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
			OutputDebugString("文件内容和当前Buffer不匹配（Band数不一致）！\n");
			return E_FAIL;
		}

		UINT iLobeNum = 0, iVertexNum = 0;
		if(fread(&iLobeNum, 4, 1, fp) != 1)
			return E_FAIL;
		if(fread(&iVertexNum, 4, 1, fp) != 1)
			return E_FAIL;
		if(iVertexNum != m_iVertexNum)
		{
			OutputDebugString("文件内容和当前Buffer不匹配（顶点数不一致）！\n");
			return E_FAIL;
		}

		if(fread(&m_Options, sizeof(PRTATTRIBUTE), 1, fp) != 1)
			return E_FAIL;
		if(m_Options.iLDPRTLobeNum != m_iLobeNum)
		{
			OutputDebugString("文件内容和当前Buffer不匹配（LDPRT Lobe数不一致）！\n");
				return E_FAIL;
		}
		if(m_Options.iBounceNum > MYPRT_MAXBOUNCE)
		{
			OutputDebugString("文件内容不合法（反弹次数过大）！\n");
			return E_FAIL;
		}

		// 具体数据："R/G/B" + Stride + sizeof PRTBuffer1 + PRTBufferData1，共3个Buffer表示RGB
		UINT iReadNum = 0, iBufferSize = 0;
		char szBufferName[3] = {'R', 'G', 'B'};
		char szReadBuffer[3] = "";
		for(UINT iIndex = 0; iIndex < 3; iIndex++)
		{
			// 校验BUFFER头
			if(fread(&szReadBuffer[iIndex], 1, 1, fp) != 1)
				return E_FAIL;
			if(szReadBuffer[iIndex] != szBufferName[iIndex])
				return E_FAIL;

			// 数据校验，Stride和BufferSize在前面已经算出来了，这里再次检测是否相等
			if(fread(&iData, 4, 1, fp) != 1)
				return E_FAIL;
			if(!iData || m_iStride != iData)
			{
				OutputDebugString("文件内容和当前Buffer不匹配（Stride不一致）！\n");
				return E_FAIL;
			}

			iBufferSize = m_iStride * m_iVertexNum;
			if(fread(&iData, 4, 1, fp) != 1)
				return E_FAIL;
			if(!iData || iData != iBufferSize)
			{
				OutputDebugString("文件内容和当前Buffer不匹配（数据大小不一致）！\n");
				return E_FAIL;
			}

			// 读取数据
			iReadNum = fread(m_pBufferData[iIndex], m_iStride, m_iVertexNum, fp);
			if(iReadNum != m_iVertexNum)
				return E_FAIL;
		}

		if(FAILED(ApplyToVB()))
			return E_FAIL;


		// 结束，这个只填充PRT数据，跟mesh有关的数据全部是空的，所以不算simulate
		fclose(fp);
		return S_OK;
	}








	// 得到顶点PRT系数，从指定的指针中得到数据，复制到pOutData（必须预先分配好空间），不会影响LDPRT数据
	// Stride在开始就计算好了，每个Buffer（RGB）都是一样的
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

	// Set R/G/B PRT Coefficients，Index 0/1/2 indicates R/G/B seperately
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




	// Copy R/G/B PRT Coefficients，Index 0/1/2 indicates R/G/B seperately
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


	// Multiply Albedo to all PRT Coefficients，Index 0/1/2 indicates R/G/B seperately
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

	// Multiply Albedo to one vertex's PRT Coefficients，Index 0/1/2 indicates R/G/B seperately
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



	// 把其他Buffer加到自己身上, 必须是相同的Band数和Lobe数！！！
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


	// Set to Zero，contain LDPRT
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

	// 当PRT数据设置完毕，就要执行该函数，将数据复制到VB，以后就可以直接用VB渲染，包括LDPRT
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
// 一次只支持对一个模型进行PRT计算，对于骨骼动画而言，需要预先对整个模型计算，然后用SplitMesh将PRTBuffer分割（方法跟分割Mesh的顶点缓冲相同）
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

		// 释放二维数组
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


	// 传入会互相影响的物体数组，调用前必须将每个物体的初始矩阵设置好！
	// 通过每个KBaseMesh得到初始状态，创建临时顶点缓冲用来把 Mesh的初始状态（位移、缩放、朝向）转换过去（调用ProcessVertices）
	// 然后对每个Mesh进行Projection，把每个顶点对应的PRT和LDPRT系数存入新创建的顶点系数缓冲（内部）
	// 计算好之后就可以GetBuffer了，Get一次之后，可以再SetOption再Simulate，再GetBuffer
	HRESULT PRTSimulation(LPPRTMESH pObject, LPPRTBUFFER pPRTBuffer);


private:
	// 内部使用，初始化模型相关信息，用于PRTSimulation中的射线跟踪
	HRESULT InitGeometry();


	// 下面的几个函数都是从某个顶点周围发出采样射线，调用一次就能计算一个顶点的TF向量，通过内部的几个变量来控制当前顶点
	// 计算最初始的PRT直接光照数据，这里不乘反照率，所以计算出来的是直接光照的辐照度
	HRESULT ComputeDirectLighting(LPPRTBUFFER pBuffer);
	// SSS，将计算的SSS加到已计算好的PRT数据上，根据当前计算的物体及顶点，围绕该顶点来计算周围所有顶点的SSS积分
	// 参数仿照ID3DXPRTENGINE
	HRESULT ComputeSSS(LPPRTBUFFER pInBuffer, LPPRTBUFFER pOutBuffer, LPPRTBUFFER pAddBuffer);
	// 计算光线反弹
	HRESULT ComputeBounce(LPPRTBUFFER pInBuffer, LPPRTBUFFER pOutBuffer, LPPRTBUFFER pAddBuffer);

	// 另一个独立函数，用于统一在最后计算LDPRT
	// 根据已算好的PRT数据计算LDPRT，遍历所有物体的所有顶点，根据法线来进行Single Lobe Fitting，将输入写入原PRT Buffer（PRT数据已包含LDPRT数据的空间）
	// 如果设置了只要LDPRT，就在计算前新分配空间，计算好之后释放原PRT指针，将指针指向新分配的Buffer
	HRESULT ComputeLDPRT(LPPRTBUFFER pBuffer);




private:
	// 初始化相关，一旦初始化就不能改变
	BOOL			m_bInit;				// 是否初始化过，即调用Init
	BOOL			m_bSimulated;			// 是否计算过PRT，即调用PRTSimulation
	UINT			m_iBandNum;				// Band数
	UINT			m_iSampleNum;			// Sample数量
	KMCSample		m_MCSamples;			// Monte Carlo Random Samples

	// 下面这些都是临时使用的，只是做临时保存数据的用途，Simulation结束就释放掉了，惟一目的就是把结果计算出来，然后存到指定的Buffer中
	// Simulation相关
	LPPRTMESH		m_pObject;				// 传入的Mesh参数
	LPPRTBUFFER		m_pPRTBuffer;			// PRT系数数据，内部分配好，然后临时返回给调用者使用，总体分配和释放控制还是在该类中
	PRTATTRIBUTE	m_Options;				// PRT参数，其实pOjbect中就包含了该项，这里为了方便书写代码就单独提出来

	// 内部模型相关
	UINT			m_iCurrentVertexNo;		// 这个值表示当前正在PRTSimulation中处理的是哪个顶点，以便在内部其他函数射线跟踪的时候跳过该顶点及其所属的面
	LPD3DXVECTOR3	m_pNewVB;				// 物体经过原始矩阵变换后的数据，用于射线跟踪，仅保留顶点坐标和法线
	UINT			*m_pNewIB;				// 拷贝原物体的索引缓冲，用于加速（用Lock太慢）
	LPD3DXCOLOR		m_pDiffuse;				// 顶点自身的Diffuse，有了它就不用全局的Albedo了，但它可以不存在（根据模型自身属性）
	UINT			m_iVertexNum, m_iFaceNum;
	
	// 提速相关
	LPD3DXVECTOR3	m_pFaceVB;				// 按面存放的数据，每个面三个顶点（会有大量重复顶点，只是用于提速）的位置和法线数据都在这里面按顺序存放
											// 假设第i个面存放的第j个顶点：坐标为m_pFaceVB[i*6+j]，法线为m_pFaceVB[i*6+3+j]
	UINT			*m_pFaceVertexInfo;		// 按面存放的数据，每个面三个顶点的序号，用于加速
											// 假设第i个面存放的第j个顶点：序号为m_pFaceVertexInfo[i*3+j]
	
	LPD3DXVECTOR3	m_pFaceNormal;			// 上面那个FaceVB存放的是面对应的顶点数据，而这个存放的是面自身的法线数据，第i个面的法线就是pNormal[i]
	UINT			*m_pVertexFaceCountUpperSphere;	// 这两个是每个顶点对应的，在它上/下半球的面数量，用于快速选择面，第i个顶点对应的上/下半球面数就是pCount[i]
	UINT			*m_pVertexFaceCountLowerSphere;
	UINT			**m_ppVertexFaceInfoUpperSphere;	// 这两个是每个顶点对应的，在它上/下半球具体的面序号列表，用于快速选择面，加速射线跟踪速度
	UINT			**m_ppVertexFaceInfoLowerSphere;	// 第i个顶点对应的第n个半球面（n < pCount[i]）就是ppFaceInfo[i][n]


	// 用D3D的引擎来加速
	LPD3DXPRTENGINE	m_pD3DXPRTEngine;

	



	// 内部使用函数
	// 得到顶点信息，从VBList中得到初始矩阵变换过的信息
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

	// 得到面信息，先得到索引，再根据索引值从VBList中得到顶点信息，3个分别是A/B/C点，绘制顺序（索引顺序）从0～2
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



	// 得到是否有相交面的信息（没一个面相交的话返回FALSE，有任意一个面相交就返回TRUE），通过初始化得到的KBaseMesh列表来遍历，用于计算Shadowed
	BOOL GetVisibleUpperSphere(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay);
	// 得到最近的相交面信息（没一个面相交的话返回FALSE），通过初始化得到的KBaseMesh列表来遍历，后三个参数是返回值，表示该相交点在哪个面中（面索引序号），及重心坐标和相交长度，用于计算Bounce和SSS
	BOOL GetClosestIntersectPointUpperSphere(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, UINT *pFaceNo, LPD3DXVECTOR3 pPtBaryCentric, float *pLength);
	BOOL GetClosestIntersectPointLowerSphere(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, UINT *pFaceNo, LPD3DXVECTOR3 pPtBaryCentric, float *pLength);


	// 根据Options中的材质信息和距离来计算散射贡献度Rd，用于SSS
	// 其中Zv Zr是提前根据介质属性算好的，这里为了加速就不重复运算了
	// Sv Sr是提前根据当前相交点算好的偏移距离
	// fSqrtExtinct = sqrt(3*absorb*scatter)
	float GetBSSRDF_Rd(float fSv, float fSr, float fZv[3], float fZr[3], float fSqrtExtinct[3], UINT iColorIndex)
	{
		static float fRd = 0.0f, fExtinct = 0.0f;
		fExtinct = m_Options.fScatteringCoef[iColorIndex] + m_Options.fAbsorbation[iColorIndex];
		// 这几个在分母上的量必须合法
		if(fExtinct < 0.0000001f || fSv < 0.000001f || fSr < 0.000001f)
			return 0.0f;

//		float fExpSr = powf(2.71828183f, -fSqrtExtinct[iColorIndex] * fSr);
//		float fExpSv = powf(2.71828183f, -fSqrtExtinct[iColorIndex] * fSv);
		float fExpSr = expf(-fSqrtExtinct[iColorIndex] * fSr);
		float fExpSv = expf(-fSqrtExtinct[iColorIndex] * fSv);

		float fPart_r = fZr[iColorIndex] * (1 / fSr + fSqrtExtinct[iColorIndex]) * fExpSr / (fSr * fSr);
		float fPart_v = fZv[iColorIndex] * (1 / fSv + fSqrtExtinct[iColorIndex]) * fExpSv / (fSv * fSv);
		float fRdCoef = m_Options.fScatteringCoef[iColorIndex] / (4.0f * D3DX_PI * fExtinct);	// fRd最前面的综合系数
		fRd = fRdCoef * (fPart_r + fPart_v);

		return fRd;
	}

};