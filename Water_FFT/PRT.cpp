#include "myd3d.h"
#include "ProcXFile.h"
#include "MeshAnim.h"
#include "sh.h"
#include "PRT.h"




HRESULT KPRTEngine::PRTSimulation(LPPRTMESH pObject, LPPRTBUFFER pPRTBuffer)
{
	if(!m_bInit)
		return D3DERR_NOTAVAILABLE;
	if(!pObject || !pPRTBuffer)
		return D3DERR_INVALIDCALL;
	if(!pObject->pMesh)
		return D3DERR_INVALIDCALL;

	// Options��Ч�Լ��
	if(pObject->Options.iBounceNum > MYPRT_MAXBOUNCE || pObject->Options.iLDPRTLobeNum > MYLDPRT_MAXLOBE)
		return D3DERR_INVALIDCALL;
	if(pObject->Options.bSSS && pObject->Options.fLengthScale < 0.0000001f)
	{
		OutputDebugString("SSS Length Scale�Ƿ�����");
		return D3DERR_INVALIDCALL;
	}

	// Mesh��ʽ��Ч�Լ�飬������NORMAL��
	if( !(pObject->pMesh->GetFVF() & D3DFVF_NORMAL) )
	{
		OutputDebugString("Meshû�з��ߣ����ȼ�����ٽ���PRT���㣡��\n");
		return D3DERR_INVALIDCALL;
	}


	// ����D3DXPRTEngine
#ifdef MYPRT_USED3DXPRTINTERSECT
	if(FAILED(D3DXCreatePRTEngine(pObject->pMesh, NULL, FALSE, NULL, &m_pD3DXPRTEngine)))
		return E_FAIL;
#endif


	UINT i = 0, j = 0;
	
	// ��ʼ������
	// ����Options
	memcpy(&m_Options, &pObject->Options, sizeof(PRTATTRIBUTE));
	m_pObject = pObject;

	m_pPRTBuffer = pPRTBuffer;
	m_pPRTBuffer->Release();

	// ����PRT Buffer
	if(FAILED(m_pPRTBuffer->Init(m_iBandNum, m_pObject)))
		return E_FAIL;




	// ��ʼ��ģ����Ϣ
	if(FAILED(InitGeometry()))
		return E_FAIL;

	// �����ǿ��ʹ���Զ����Albedo�����������ģ�������Albedo��Ϣ�ͷŵ�
	//SAFE_DELETE_ARRAY(m_pDiffuse);




///////////////////////////////////////////////// ������ʼRay Tracing����TF����

	// ������ͨ��+Shadow(����BRDF��Reflectancy, ���������Direct Lighting Irradiance)
	if(FAILED(ComputeDirectLighting(m_pPRTBuffer)))
		return E_FAIL;


	// �Ѽ�ӹ��շ��ն���Ϊ��Դ������߷�����SSS
	UINT iBounce = 0, k = 0;

	// ÿ�η����ļ�ӹ��չ�Դ���ն�Buffer���ȳ�ʼ��
	if(m_Options.iBounceNum || m_Options.bSSS)
	{
		// ��Ϊ��һ�μ�ӹ��յĹ�Դ����ֱ�ӹ��յĽ��������Ҫ��һ��
		// ���ǵ���һ��SSS��Ҫռ��һ��Buffer��λ�ã������ٶ�һ��
		LPPRTBUFFER pIndirectBuffer = new PRTBUFFER[m_Options.iBounceNum+2];
		// ��ʱʹ�ã����ڼ���ÿ�μ�ӹ�����Ϊ��Դʱ��ɢ����ܺͷ�������Buffer
		PRTBUFFER BounceBuffer;

		for(iBounce = 0; iBounce < (m_Options.iBounceNum+2); iBounce++)
			if(FAILED(pIndirectBuffer[iBounce].Init(m_iBandNum, m_pObject)))
				return E_FAIL;
		if(FAILED(BounceBuffer.Init(m_iBandNum, m_pObject)))
			return E_FAIL;

		// �ȳ�ʼ��������
		for(iBounce = 0; iBounce < (m_Options.iBounceNum+2); iBounce++)
			for(j = MYPRT_REDCHANNEL; j <= MYPRT_BLUECHANNEL; j++)
			{
				if(FAILED(pIndirectBuffer[iBounce].SetToZero(j)))
					return E_FAIL;;
			}
		for(j = MYPRT_REDCHANNEL; j <= MYPRT_BLUECHANNEL; j++)
		{
			// ��һ�μ�ӹ��չ�Դ����ֱ�ӹ��յĽ��������Indirect[0] = PRTBuffer
			// ͬ����һ�μ�ӹ��յĽ�����ǵڶ��μ�ӹ��յĹ�Դ�������Indirect[1]��
			if(FAILED(pIndirectBuffer[0].AddBuffer(j, j, m_pPRTBuffer)))
				return E_FAIL;;
			if(FAILED(BounceBuffer.SetToZero(j)))
				return E_FAIL;
		}


		// ��������SSS�ķ���
		if(!m_Options.bSSS)
		{
			for(iBounce = 0; iBounce < m_Options.iBounceNum; iBounce++)
			{
				if(FAILED(ComputeBounce(pIndirectBuffer+iBounce, &BounceBuffer, NULL)))
					return E_FAIL;

				// ÿһ�εķ�����Ϊ��һ�εĴ���ӹ��չ�ԴBuffer
				for(j = MYPRT_REDCHANNEL; j <= MYPRT_BLUECHANNEL; j++)
				{
					if(FAILED(pIndirectBuffer[iBounce+1].AddBuffer(j, j, &BounceBuffer)))
						return E_FAIL;
				}
			}// end Indirect Lighting

			// ��ӹ��ռ�����ɣ���ÿ�εļ�ӹ��ս���ӵ�PRT Buffer�ϣ�������һ�εģ�ֱ�ӹ��գ�
			for(k = 1; k <= m_Options.iBounceNum; k++)
				for(j = MYPRT_REDCHANNEL; j <= MYPRT_BLUECHANNEL; j++)
				{
					if(FAILED(m_pPRTBuffer->AddBuffer(j, j, pIndirectBuffer+k)))
						return E_FAIL;
				}
		}// end if no sss




		// ��������SSS�ķ���
		else
		{
			// ����ԭʼ���ն���SSS��ŵ�Indirect Buffer1
			if(FAILED(ComputeSSS(pIndirectBuffer, pIndirectBuffer+1, NULL)))
				return E_FAIL;

			for(iBounce = 1; iBounce <= m_Options.iBounceNum; iBounce++)
			{

				// ��ѭ���ߵĴ��������ݸյõ���ɢ����ܣ�����Bounce�����õ�ɢ����ٴη��䵽�������Ĺ���
				if(FAILED(ComputeBounce(pIndirectBuffer+iBounce, &BounceBuffer, NULL)))
					return E_FAIL;

				// ����ԭʼ���ն���SSS��ŵ���һ��Indirect Buffer
				if(FAILED(ComputeSSS(&BounceBuffer, pIndirectBuffer+iBounce+1, NULL)))
					return E_FAIL;

			}// end Indirect Lighting


			// Test, Get Pure Scattering Light
			for(j = MYPRT_REDCHANNEL; j <= MYPRT_BLUECHANNEL; j++)
				if(FAILED(m_pPRTBuffer->SetToZero(j)))
					return E_FAIL;


			// ��ӹ���SSS������ɣ���ÿ�εļ�ӹ���SSS����ӵ�PRT Buffer�ϣ�������һ�εģ�ֱ�ӹ��գ�
			// ��Ϊ��������ʱ��Ҳ����һ�����ӵ�SSS������Ҫ���һ��
			for(k = 1; k <= m_Options.iBounceNum+1; k++)
				for(j = MYPRT_REDCHANNEL; j <= MYPRT_BLUECHANNEL; j++)
				{
					if(FAILED(m_pPRTBuffer->AddBuffer(j, j, pIndirectBuffer+k)))
						return E_FAIL;
				}

		}// end if sss


		// �ͷ���ʱ
		SAFE_DELETE_ARRAY(pIndirectBuffer);
		BounceBuffer.Release();

	}// if bounce or sss



	// ��Albedo���ӷ��նȵõ����յķ���ȣ������AlbedoҲ���Գ˶��������Albedo��Diffuse��
	for(i = MYPRT_REDCHANNEL; i <= MYPRT_BLUECHANNEL; i++)
	{
		if(m_pDiffuse)
		{
			float fAlbedo = 0.0f;
			for(j = 0; j < m_iVertexNum; j++)
			{
				if(i == MYPRT_REDCHANNEL)
					fAlbedo = m_pDiffuse[j].r;
				if(i == MYPRT_GREENCHANNEL)
					fAlbedo = m_pDiffuse[j].g;
				if(i == MYPRT_BLUECHANNEL)
					fAlbedo = m_pDiffuse[j].b;

				if(FAILED(m_pPRTBuffer->MultiplyVertexAlbedo(fAlbedo, j, i)))
					return E_FAIL;
			}
		}
		else
		{
			if(FAILED(m_pPRTBuffer->MultiplyAlbedo(m_Options.fAlbedo[i], i)))
				return E_FAIL;
		}
	}


	// �����LDPRT��Buffer��Buffer���У����ǵ�OnlyLDPRT��ı�Stride���������ﲻҪ��GetPRTInfo
	// ��ʵ����Ҳ����ֻѡ������һ��������ӳ�䵽����Ĵ����ģ�����������ʱ�����·���
	if(m_Options.iLDPRTLobeNum)
		if(FAILED(ComputeLDPRT(m_pPRTBuffer)))
			return E_FAIL;

	// �ã�����PRT BUFFER VB
	if(FAILED(m_pPRTBuffer->ApplyToVB()))
		return E_FAIL;

	// �ͷ�������ʱ�ڲ�����

	m_bSimulated = TRUE;
	return S_OK;
}






HRESULT KPRTEngine::InitGeometry()
{
	UINT i = 0, j = 0;

	///////////////////////////////////////////////// ��Ҫ��ÿ��ģ�͵Ķ��㻺�����¸���һ�ݣ���ԭʼ״̬����������棬������VBList
	LPD3DXMESH pMesh = m_pObject->pMesh;
	D3DXMATRIX MatCombine, MatRotation;
	MatCombine = m_pObject->MatRotation * m_pObject->MatScaling * m_pObject->MatTranslation;
	MatRotation = m_pObject->MatRotation;

	UINT iVertexNum = pMesh->GetNumVertices();
	if(!iVertexNum)
		return E_FAIL;
	m_iVertexNum = iVertexNum;
	m_iCurrentVertexNo = 0;

	// ���ƣ�ֻ���ƶ�������ͷ���
	// �õ�ģ������
	UINT iStride = pMesh->GetNumBytesPerVertex();
	if(!iStride)
		return E_FAIL;
	DWORD dwFVF = pMesh->GetFVF();
	if(!dwFVF)
		return E_FAIL;

	// �õ�Դ����
	BYTE *pSourceData = NULL;
	if(FAILED(pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void **)&pSourceData)))
		return E_FAIL;

	// �������õ������ݣ�����+������24�ֽڣ�6��������2����
	SAFE_DELETE_ARRAY(m_pNewVB);
	m_pNewVB = new D3DXVECTOR3[m_iVertexNum * 2];
	if(!m_pNewVB)
		return E_OUTOFMEMORY;

	// ����ж������������ݣ��ʹ���
	SAFE_DELETE_ARRAY(m_pDiffuse);
	if(dwFVF & D3DFVF_DIFFUSE)
	{
		m_pDiffuse = new D3DXCOLOR[m_iVertexNum];
		if(!m_pDiffuse)
			return E_OUTOFMEMORY;
	}

	// ��ʼת������
	LPD3DXVECTOR3 pSrcData = NULL, pDstData = m_pNewVB;
	DWORD *pSrcDiffuse = NULL;
	LPD3DXCOLOR pDstDiffuse = m_pDiffuse;

	for( j = 0; j < m_iVertexNum; j++, pSourceData+=iStride)
	{
		pSrcData = (LPD3DXVECTOR3)pSourceData;

		// ��д�붥������
		D3DXVec3TransformCoord(pDstData, pSrcData, &MatCombine);
		pSrcData++;
		pDstData++;

		// ��д�뷨������
		D3DXVECTOR3 VecNormal(0, 0, 0);
		D3DXVec3TransformCoord(&VecNormal, pSrcData, &MatRotation);
		D3DXVec3Normalize(&VecNormal, &VecNormal);
		if(absf(D3DXVec3Length(&VecNormal) - 1.0f) > 0.0001f)
		{
			OutputDebugString("�����������󣡣�����ģ�͵���Ч�ԣ�\n");
			return E_FAIL;
		}
		*pSrcData++;
		*pDstData++ = VecNormal;

		// �ж��Ƿ���Diffuse���о�д�룬��ΪDiffuse�ǽ��ŷ��ߵģ��м��FVF���������Ͷ������ã����ԣ�
		if((dwFVF && D3DFVF_DIFFUSE) && m_pDiffuse)
		{
			pSrcDiffuse = (DWORD *)pSrcData;
			*pDstDiffuse++ = DWCOLORTOVECTOR(*pSrcDiffuse);
		}

	}

	// д�����
	if(FAILED(pMesh->UnlockVertexBuffer()))
		return E_FAIL;



	/////////////////////////////////////////////////// д����������
	UINT iFaceNum = pMesh->GetNumFaces();
	if(!iFaceNum)
		return E_FAIL;
	m_iFaceNum = iFaceNum;

	SAFE_DELETE_ARRAY(m_pNewIB);
	m_pNewIB = new UINT[m_iFaceNum * 3];
	if(!m_pNewIB)
		return E_OUTOFMEMORY;

	// Get Indics Information
	BYTE *pDataHead = NULL;
	// Whether 16-bit or 32-bit Indics
	DWORD dwOptions = pMesh->GetOptions();
	if(FAILED(pMesh->LockIndexBuffer(D3DLOCK_READONLY, (void **)&pDataHead)))
		return E_FAIL;

	UINT *pDstIndexData = m_pNewIB;
	for(i = 0; i < m_iFaceNum * 3; i++, pDstIndexData++)
	{
		if(dwOptions & D3DXMESH_32BIT)
		{
			UINT *pSrcIndexData = (UINT *)pDataHead + i;
			*pDstIndexData = *pSrcIndexData;
		}
		else
		{
			WORD *pSrcIndexData = (WORD *)pDataHead + i;
			WORD wTemp = 0;
			wTemp = *pSrcIndexData;
			*pDstIndexData = (UINT)wTemp;
		}
	}

	if(FAILED(pMesh->UnlockIndexBuffer()))
		return E_FAIL;



	//////////////////////////////////////////////////////д�������Ͷ�������
	SAFE_DELETE_ARRAY(m_pFaceVB);
	SAFE_DELETE_ARRAY(m_pFaceNormal);
	SAFE_DELETE_ARRAY(m_pFaceVertexInfo);
	m_pFaceVB = new D3DXVECTOR3[m_iFaceNum * 3 * 2];
	m_pFaceNormal = new D3DXVECTOR3[m_iFaceNum];
	m_pFaceVertexInfo = new UINT[m_iFaceNum * 3];
	if(!m_pFaceVB || !m_pFaceNormal || !m_pFaceVertexInfo)
		return E_OUTOFMEMORY;

	D3DXVECTOR3 VecNormal(0, 0, 0), VecNormalVertex(0, 0, 0), VecLine1, VecLine2, VecLine3;
	for(i = 0; i < m_iFaceNum; i++)
	{
		// ��д�����Ӧ�Ķ�����Ϣ
		if(FAILED(GetFaceInfo(i, m_pFaceVertexInfo + i*3, m_pFaceVB + i*6, m_pFaceVB+i*6+3)))
			return E_FAIL;

		// ��������������ķ�����Ϣ
		D3DXVec3Normalize(&VecLine1, &(m_pFaceVB[i*6+1]-m_pFaceVB[i*6+0]));
		D3DXVec3Normalize(&VecLine2, &(m_pFaceVB[i*6+2]-m_pFaceVB[i*6+0]));
		D3DXVec3Cross(&VecNormal, &VecLine1, &VecLine2);
		D3DXVec3Normalize(&VecNormal, &VecNormal);
		if(absf(D3DXVec3Length(&VecNormal) - 1.0f) > 0.0001f)
		{
			OutputDebugString("�����������󣡣�����ģ�͵���Ч�ԣ�\n");
			return E_FAIL;
		}

		// �㶥�㷨�߾�ֵ
		VecNormalVertex = (m_pFaceVB[i*6+3] + m_pFaceVB[i*6+4] + m_pFaceVB[i*6+5]) / 3.0f;
		// ��������������򣬾ͽ��淨��ȡ������֮�����㷨��Ҫͬ��
		if(D3DXVec3Dot(&VecNormalVertex, &VecNormal) < 0.0f)
			VecNormal *= -1.0f;
		// д��
		m_pFaceNormal[i] = VecNormal;
	}


	/////////////////////////////////////////////////////////д��ÿ�������Ӧ���ϰ�����°�������Ϣ�����ڼ������߸���
#ifndef MYPRT_USED3DXPRTINTERSECT
	SAFE_DELETE_ARRAY(m_pVertexFaceCountUpperSphere);
	SAFE_DELETE_ARRAY(m_pVertexFaceCountLowerSphere);

	SAFE_DELETE_ARRAY(m_ppVertexFaceInfoUpperSphere);
	SAFE_DELETE_ARRAY(m_ppVertexFaceInfoLowerSphere);

	m_pVertexFaceCountUpperSphere = new UINT[m_iVertexNum];
	m_pVertexFaceCountLowerSphere = new UINT[m_iVertexNum];
	m_ppVertexFaceInfoUpperSphere = new UINT*[m_iVertexNum];
	m_ppVertexFaceInfoLowerSphere = new UINT*[m_iVertexNum];

	if(!m_pVertexFaceCountUpperSphere || !m_pVertexFaceCountLowerSphere || !m_ppVertexFaceInfoUpperSphere || !m_ppVertexFaceInfoLowerSphere)
		return E_OUTOFMEMORY;

	// ��ʱʹ�õ�
	UINT *pVertexUpperData = new UINT[m_iFaceNum];
	UINT *pVertexLowerData = new UINT[m_iFaceNum];
	if(!pVertexLowerData || !pVertexUpperData)
		return E_OUTOFMEMORY;


	UINT iUpperFaceNum = 0, iLowerFaceNum = 0;
	float fDot1 = 0.0f, fDot2 = 0.0f, fDot3 = 0.0f;

	// ��ÿ������
	for(i = 0; i < m_iVertexNum; i++)
	{
		// �ȳ�ʼ��ÿ�������õ�������
		iUpperFaceNum = iLowerFaceNum = 0;
		VecNormalVertex = m_pNewVB[i*2+1];

		// �Ȱ�ȫ����������һ�飬������������ʱ����
		for(j = 0; j < m_iFaceNum; j++)
		{
			// ��������ֱ𵽵�ǰ�������
			VecLine1 = m_pFaceVB[j*6] - m_pNewVB[i*2];
			VecLine2 = m_pFaceVB[j*6+1] - m_pNewVB[i*2];
			VecLine3 = m_pFaceVB[j*6+2] - m_pNewVB[i*2];
			D3DXVec3Normalize(&VecLine1, &VecLine1);
			D3DXVec3Normalize(&VecLine2, &VecLine2);
			D3DXVec3Normalize(&VecLine3, &VecLine3);

			// �����ֱ�Ͷ��㷨����dot
			fDot1 = D3DXVec3Dot(&VecLine1, &VecNormalVertex);
			fDot2 = D3DXVec3Dot(&VecLine2, &VecNormalVertex);
			fDot3 = D3DXVec3Dot(&VecLine3, &VecNormalVertex);
			// DotֻҪ��һ�����ϰ��򣬾��п����ཻ���͵�д���ϰ��򻺳壨����dot > 0��
			if(fDot1 > 0.0001f || fDot2 > 0.0001f || fDot3 > 0.0001f )
			{
				pVertexUpperData[iUpperFaceNum] = j;
				iUpperFaceNum++;
			}
			// ������Щ����ܿ�Խ�����������Բ����������+else�����������жϣ��������°���
			if(fDot1 < -0.000001f || fDot2 < -0.000001f || fDot3 < -0.000001f )
			{
				pVertexLowerData[iLowerFaceNum] = j;
				iLowerFaceNum++;
			}
			// ��������Կ�����ƽ�е��汻��������Ϊ�ж��ཻ��ʱ���ǲ���Ҫƽ�еģ����򲻵�Ӱ���ٶȻ����ܻ������Ⱦ����
		}// end for each face

		// д�����
		m_pVertexFaceCountUpperSphere[i] = iUpperFaceNum;
		m_pVertexFaceCountLowerSphere[i] = iLowerFaceNum;

		// ���仺����
		m_ppVertexFaceInfoUpperSphere[i] = NULL;
		if(iUpperFaceNum)
		{
			m_ppVertexFaceInfoUpperSphere[i] = new UINT[iUpperFaceNum];
			if(!m_ppVertexFaceInfoUpperSphere[i])
				return E_OUTOFMEMORY;
		}

		m_ppVertexFaceInfoLowerSphere[i] = NULL;
		if(iLowerFaceNum)
		{
			m_ppVertexFaceInfoLowerSphere[i] = new UINT[iLowerFaceNum];
			if(!m_ppVertexFaceInfoLowerSphere[i])
				return E_OUTOFMEMORY;
		}


		// ����ʱ����Ľ��д���»�����
		for(j = 0; j < iUpperFaceNum; j++)
			m_ppVertexFaceInfoUpperSphere[i][j] = pVertexUpperData[j];
		for(j = 0; j < iLowerFaceNum; j++)
			m_ppVertexFaceInfoLowerSphere[i][j] = pVertexLowerData[j];

	}// end for each vertex
	SAFE_DELETE_ARRAY(pVertexLowerData);
	SAFE_DELETE_ARRAY(pVertexUpperData);
#endif

	return S_OK;
}



/************************************************************************/
/*				Compute Intersect			                            */
/************************************************************************/

BOOL KPRTEngine::GetVisibleUpperSphere(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay)
{
//	if(D3DXVec3Length(&VecRay) == 0.0f)
//		return FALSE;

	// �ȵõ�ģ������
	if(!m_iFaceNum)
		return FALSE;

	static UINT i = 0, j = 0, iFaceNo = 0;

	// ����������
	static D3DXVECTOR3 PtFaces[3], VecFaces[3];
	static UINT iFaceVertexNo[3] = {0, 0, 0};
	static BOOL bIntersect = FALSE;

	// Temp Use
	static float p1, p2, fLength = 0.0f;
	static D3DXVECTOR3 PtBary;
	//static MYRAYTRACING_INTERSECT Intersect;

	static UINT *pFaceData = NULL;
	static float *pVertexData = NULL;

	for(i = 0; i < m_pVertexFaceCountUpperSphere[m_iCurrentVertexNo]; i++)
	{
		// �ȵõ��ڵ�ǰ���ϰ����ڵ������
		iFaceNo = m_ppVertexFaceInfoUpperSphere[m_iCurrentVertexNo][i];
//		if(FAILED(GetFaceInfo(i, iFaceVertexNo, PtFaces, VecFaces)))
//			return FALSE;

		// Ϊ�����٣��Լ��ں�������ɹ��ܾͺ���
		PtFaces[0].x = m_pFaceVB[iFaceNo * 6].x;
		PtFaces[0].y = m_pFaceVB[iFaceNo * 6].y;
		PtFaces[0].z = m_pFaceVB[iFaceNo * 6].z;

		PtFaces[1].x = m_pFaceVB[iFaceNo * 6+1].x;
		PtFaces[1].y = m_pFaceVB[iFaceNo * 6+1].y;
		PtFaces[1].z = m_pFaceVB[iFaceNo * 6+1].z;

		PtFaces[2].x = m_pFaceVB[iFaceNo * 6+2].x;
		PtFaces[2].y = m_pFaceVB[iFaceNo * 6+2].y;
		PtFaces[2].z = m_pFaceVB[iFaceNo * 6+2].z;

		VecFaces[0].x = m_pFaceVB[iFaceNo * 6+3].x;
		VecFaces[0].y = m_pFaceVB[iFaceNo * 6+3].y;
		VecFaces[0].z = m_pFaceVB[iFaceNo * 6+3].z;

		VecFaces[1].x = m_pFaceVB[iFaceNo * 6+4].x;
		VecFaces[1].y = m_pFaceVB[iFaceNo * 6+4].y;
		VecFaces[1].z = m_pFaceVB[iFaceNo * 6+4].z;

		VecFaces[2].x = m_pFaceVB[iFaceNo * 6+5].x;
		VecFaces[2].y = m_pFaceVB[iFaceNo * 6+5].y;
		VecFaces[2].z = m_pFaceVB[iFaceNo * 6+5].z;

		iFaceVertexNo[0] = m_pFaceVertexInfo[iFaceNo * 3];
		iFaceVertexNo[1] = m_pFaceVertexInfo[iFaceNo * 3+1];
		iFaceVertexNo[2] = m_pFaceVertexInfo[iFaceNo * 3+2];

/*		// ������Ҫע�⣺����Ҫ������Щ�������ߵ���棨�����ߵ�����Щ���ϣ����Ҹ���ķ�������ǽӽ�ƽ�����߷����
		// ������ʾ���ཻ�����ڵ�
		// D3D�ĸ��ٺ�����������ߵ������ϣ���ôֱ�Ӿͻ᷵���ཻ
		// �Լ��ĸ��ٺ�����������ߵ������ϣ�ֱ�Ӿͻ᷵�ز��ཻ�����������������Ҫ�ģ���ȻҲ����ͨ���ж�D3D���ص�fLength��ȥ���������
		bSameFace = FALSE;
		if(m_iCurrentVertexNo == iFaceVertexNo[0] || m_iCurrentVertexNo == iFaceVertexNo[1] || m_iCurrentVertexNo == iFaceVertexNo[2])
			bSameFace = TRUE;
		for(j = 0; j < 3; j++)
			if(PtFaces[j].x == PtStart.x && PtFaces[j].y == PtStart.y && PtFaces[j].z == PtStart.z)
			{
				bSameFace = TRUE;
				break;
			}		
		if(bSameFace)
		{
			// �õ��淨��
			D3DXVECTOR3 VecNormal(0, 0, 0), VecLine1, VecLine2;
			D3DXVec3Normalize(&VecLine1, &(PtFaces[1]-PtFaces[0]));
			D3DXVec3Normalize(&VecLine2, &(PtFaces[2]-PtFaces[0]));
			D3DXVec3Cross(&VecNormal, &VecLine1, &VecLine2);
			D3DXVec3Normalize(&VecNormal, &VecNormal);

			// ����ӽ�ƽ�о��Ǻͷ��߽ӽ���ֱ
			float fDot = D3DXVec3Dot(&VecNormal, &VecRay);
			if(absf(fDot) < 0.01f)	// �����84�ȣ�������84�Ⱦ���Ϊ��ƽ�еģ�����
				continue;
		}
*/
		
		// �õ��淨��

		// ����ӽ�ƽ�о��Ǻͷ��߽ӽ���ֱ
//		float fDot = D3DXVec3Dot(&m_pFaceNormal[iFaceNo], &VecRay);
//		if(absf(fDot) < 0.001f)	// ƽ�еģ�����
//			continue;

		// ���߸�����
//		bIntersect = GetIntersectTriangle3D(PtStart, VecRay, PtFaces[0], PtFaces[1], PtFaces[2], &PtBary, &p1, NULL, &Intersect);
		bIntersect = D3DXIntersectTri(&PtFaces[0], &PtFaces[1], &PtFaces[2], &PtStart, &VecRay, &p1, &p2, &fLength);
		if(bIntersect && fLength < 0.000001f)	// ��������
		{
			D3DXVECTOR3 VecVertexNormal = m_pNewVB[m_iCurrentVertexNo * 2+1];
			PtStart += VecVertexNormal * 0.0001f;
			bIntersect = D3DXIntersectTri(&PtFaces[0], &PtFaces[1], &PtFaces[2], &PtStart, &VecRay, &p1, &p2, &fLength);
		}
		// ��һ���ཻ��˵�����ڵ������ɼ���ֱ�ӷ���
		if(bIntersect)
			return FALSE;
		else
			continue;
	}

	// û���ཻ�ģ���˵��û���ڵ����ɼ�
	return TRUE;
}



BOOL KPRTEngine::GetClosestIntersectPointUpperSphere(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, UINT *pFaceNo, LPD3DXVECTOR3 pPtBaryCentric, float *pLength)
{
	if(!pFaceNo || !pPtBaryCentric || !pLength)
		return FALSE;

	// �ȵõ�ģ������
	if(!m_iFaceNum)
		return FALSE;

	// ����������
	static D3DXVECTOR3 PtFaces[3], VecFaces[3];
	static UINT iFaceVertexNo[3] = {0, 0, 0};
	static BOOL bIntersect = FALSE;
	
	static UINT i = 0, iFaceNo = 0;

	// Temp Use
	static float fLength = 0.0f;
	static D3DXVECTOR3 PtBaryCentric(0, 0, 0);

	// ���ֵ
	static int iClosestFaceNo = -1;		// ��ʼΪ�����ڵ���
	static D3DXVECTOR3 PtClosestBaryCentric(0, 0, 0);
	static float fClosestLength = 999999.0f;	// ��ʼ����Ϊ���
	fClosestLength = 999999.0f;
	iClosestFaceNo = -1;

	for(i = 0; i < m_pVertexFaceCountUpperSphere[m_iCurrentVertexNo]; i++)
	{
		// �ȵõ��ڵ�ǰ���ϰ����ڵ������
		iFaceNo = m_ppVertexFaceInfoUpperSphere[m_iCurrentVertexNo][i];

		// Ϊ�����٣��Լ��ں�������ɹ��ܾͺ���
		PtFaces[0].x = m_pFaceVB[iFaceNo * 6].x;
		PtFaces[0].y = m_pFaceVB[iFaceNo * 6].y;
		PtFaces[0].z = m_pFaceVB[iFaceNo * 6].z;

		PtFaces[1].x = m_pFaceVB[iFaceNo * 6+1].x;
		PtFaces[1].y = m_pFaceVB[iFaceNo * 6+1].y;
		PtFaces[1].z = m_pFaceVB[iFaceNo * 6+1].z;

		PtFaces[2].x = m_pFaceVB[iFaceNo * 6+2].x;
		PtFaces[2].y = m_pFaceVB[iFaceNo * 6+2].y;
		PtFaces[2].z = m_pFaceVB[iFaceNo * 6+2].z;

		VecFaces[0].x = m_pFaceVB[iFaceNo * 6+3].x;
		VecFaces[0].y = m_pFaceVB[iFaceNo * 6+3].y;
		VecFaces[0].z = m_pFaceVB[iFaceNo * 6+3].z;

		VecFaces[1].x = m_pFaceVB[iFaceNo * 6+4].x;
		VecFaces[1].y = m_pFaceVB[iFaceNo * 6+4].y;
		VecFaces[1].z = m_pFaceVB[iFaceNo * 6+4].z;

		VecFaces[2].x = m_pFaceVB[iFaceNo * 6+5].x;
		VecFaces[2].y = m_pFaceVB[iFaceNo * 6+5].y;
		VecFaces[2].z = m_pFaceVB[iFaceNo * 6+5].z;

		iFaceVertexNo[0] = m_pFaceVertexInfo[iFaceNo * 3];
		iFaceVertexNo[1] = m_pFaceVertexInfo[iFaceNo * 3+1];
		iFaceVertexNo[2] = m_pFaceVertexInfo[iFaceNo * 3+2];

		// �󽻣�ע�⹲������
		bIntersect = D3DXIntersectTri(&PtFaces[0], &PtFaces[1], &PtFaces[2], &PtStart, &VecRay, &PtBaryCentric.y, &PtBaryCentric.z, &fLength);
		if(bIntersect && fLength < 0.000001f)	// �������ϣ�ע������������ж��ڵ���ͬ��������Ҫ�ж���������Ըõ����չ��׶ȣ�����������ϣ�������׶Ⱦͱ�����������Ĺ��ף���û�����ˣ�����Ҫ����
		{
			//bIntersect = FALSE;
			continue;
		}

		PtBaryCentric.x = 1 - PtBaryCentric.y - PtBaryCentric.z;
		if(PtBaryCentric.x < 0 || PtBaryCentric.x > 1 || PtBaryCentric.y < 0 || PtBaryCentric.y > 1 || PtBaryCentric.z < 0 || PtBaryCentric.z > 1)
			continue;
		// ��һ���ཻ��˵�����ڵ�������ȵ�ǰ����С����С�����³��ȡ���������������
		if(bIntersect && fLength < fClosestLength)
		{
			PtClosestBaryCentric = PtBaryCentric;
			fClosestLength = fLength;
			iClosestFaceNo = iFaceNo;
		}
	}

	// û���ཻ�ģ���˵��û���ڵ����ɼ�
	if(iClosestFaceNo == -1)
		return FALSE;

	// �������������
	*pFaceNo = iClosestFaceNo;
	*pPtBaryCentric = PtClosestBaryCentric;
	*pLength = fClosestLength;
	return TRUE;
}




BOOL KPRTEngine::GetClosestIntersectPointLowerSphere(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, UINT *pFaceNo, LPD3DXVECTOR3 pPtBaryCentric, float *pLength)
{
	if(!pFaceNo || !pPtBaryCentric || !pLength)
		return FALSE;

	// �ȵõ�ģ������
	if(!m_iFaceNum)
		return FALSE;

	// ����������
	static D3DXVECTOR3 PtFaces[3], VecFaces[3];
	static UINT iFaceVertexNo[3] = {0, 0, 0};
	static BOOL bIntersect = FALSE;

	static UINT i = 0, iFaceNo = 0;

	// Temp Use
	static float fLength = 0.0f;
	static D3DXVECTOR3 PtBaryCentric(0, 0, 0);

	// ���ֵ
	static int iClosestFaceNo = -1;		// ��ʼΪ�����ڵ���
	static D3DXVECTOR3 PtClosestBaryCentric(0, 0, 0);
	static float fClosestLength = 999999.0f;	// ��ʼ����Ϊ���
	fClosestLength = 999999.0f;
	iClosestFaceNo = -1;

//	for(i = 0; i < m_pVertexFaceCountLowerSphere[m_iCurrentVertexNo]; i++)
//	{
		// �ȵõ��ڵ�ǰ���ϰ����ڵ������
//		iFaceNo = m_ppVertexFaceInfoLowerSphere[m_iCurrentVertexNo][i];
	// Test
	for(iFaceNo = 0; iFaceNo < m_iFaceNum; iFaceNo++)
	{

		// Ϊ�����٣��Լ��ں�������ɹ��ܾͺ���
		PtFaces[0].x = m_pFaceVB[iFaceNo * 6].x;
		PtFaces[0].y = m_pFaceVB[iFaceNo * 6].y;
		PtFaces[0].z = m_pFaceVB[iFaceNo * 6].z;

		PtFaces[1].x = m_pFaceVB[iFaceNo * 6+1].x;
		PtFaces[1].y = m_pFaceVB[iFaceNo * 6+1].y;
		PtFaces[1].z = m_pFaceVB[iFaceNo * 6+1].z;

		PtFaces[2].x = m_pFaceVB[iFaceNo * 6+2].x;
		PtFaces[2].y = m_pFaceVB[iFaceNo * 6+2].y;
		PtFaces[2].z = m_pFaceVB[iFaceNo * 6+2].z;

		VecFaces[0].x = m_pFaceVB[iFaceNo * 6+3].x;
		VecFaces[0].y = m_pFaceVB[iFaceNo * 6+3].y;
		VecFaces[0].z = m_pFaceVB[iFaceNo * 6+3].z;

		VecFaces[1].x = m_pFaceVB[iFaceNo * 6+4].x;
		VecFaces[1].y = m_pFaceVB[iFaceNo * 6+4].y;
		VecFaces[1].z = m_pFaceVB[iFaceNo * 6+4].z;

		VecFaces[2].x = m_pFaceVB[iFaceNo * 6+5].x;
		VecFaces[2].y = m_pFaceVB[iFaceNo * 6+5].y;
		VecFaces[2].z = m_pFaceVB[iFaceNo * 6+5].z;

		iFaceVertexNo[0] = m_pFaceVertexInfo[iFaceNo * 3];
		iFaceVertexNo[1] = m_pFaceVertexInfo[iFaceNo * 3+1];
		iFaceVertexNo[2] = m_pFaceVertexInfo[iFaceNo * 3+2];

		// �󽻣�ע�⹲������
		bIntersect = D3DXIntersectTri(&PtFaces[0], &PtFaces[1], &PtFaces[2], &PtStart, &VecRay, &PtBaryCentric.y, &PtBaryCentric.z, &fLength);
		if(bIntersect && fLength < 0.000001f)	// �������ϣ�ע������������ж��ڵ���ͬ��������Ҫ�ж���������Ըõ����չ��׶ȣ�����������ϣ�������׶Ⱦͱ�����������Ĺ��ף���û�����ˣ�����Ҫ����
		{
			//bIntersect = FALSE;
			continue;
		}

		PtBaryCentric.x = 1 - PtBaryCentric.y - PtBaryCentric.z;
		if(PtBaryCentric.x < 0 || PtBaryCentric.x > 1 || PtBaryCentric.y < 0 || PtBaryCentric.y > 1 || PtBaryCentric.z < 0 || PtBaryCentric.z > 1)
			continue;
		// ��һ���ཻ��˵�����ڵ�������ȵ�ǰ����С����С�����³��ȡ���������������
		if(bIntersect && fLength < fClosestLength)
		{
			PtClosestBaryCentric = PtBaryCentric;
			fClosestLength = fLength;
			iClosestFaceNo = iFaceNo;
		}
	}

	// û���ཻ�ģ���˵��û���ڵ����ɼ�
	if(iClosestFaceNo == -1)
		return FALSE;

	// �������������
	*pFaceNo = iClosestFaceNo;
	*pPtBaryCentric = PtClosestBaryCentric;
	*pLength = fClosestLength;
	return TRUE;
}

















/************************************************************************/
/*				Compute Direct/Bounce/SSS/LDPRT                         */
/************************************************************************/

HRESULT KPRTEngine::ComputeDirectLighting(LPPRTBUFFER pBuffer)
{
	if(m_bSimulated || !m_bInit)
		return D3DERR_NOTAVAILABLE;

	// �ȵõ�ģ������
	if(!m_iVertexNum)
		return E_FAIL;

	// ������ÿ�������ÿ���������PRT����
	UINT i = 0, j = 0;
	D3DXVECTOR3 Vec3Sample(0.0f, 0.0f, 0.0f), VecNormal(0.0f, 0.0f, 0.0f), PtPosition(0.0f, 0.0f, 0.0f);
	D3DXVECTOR2 Vec2Sample(0.0f, 0.0f);
	float fLambertian = 0.0f, *pY = NULL, *pCoef = new float[m_iBandNum * m_iBandNum];
	BOOL iVisible = 1;

	if(!pCoef)
		return E_OUTOFMEMORY;

	char szInfo[100];	// Debug�����ǰ������ɵĶ������

	// for each vertex
	for(UINT iVertexNo = 0; iVertexNo < m_iVertexNum; iVertexNo++)
	{
		m_iCurrentVertexNo = iVertexNo;
		sprintf(szInfo, "Direct Lighting %.2f%% �� %d / %d\n", (float)iVertexNo/(float)m_iVertexNum*100.0f, iVertexNo, m_iVertexNum);
		OutputDebugString(szInfo);

		// ��ʱ����ϵ�����㣬��������ֻ�Ǽ���Direct�ķ��նȣ�ֻ�����Ƿ�ɼ���cos term�����Բ�ǣ����ɫͨ����Albedo������ٳ�
		for(i = 0; i < m_iBandNum * m_iBandNum; i++)
			pCoef[i] = 0.0f;

		PtPosition = m_pNewVB[iVertexNo * 2];
		VecNormal = m_pNewVB[iVertexNo * 2 + 1];

		// Monte Carlo Ray Tracing
		for(i = 0; i < m_iSampleNum; i++)
		{
			// ����ȷ��һ����������
			if(FAILED(m_MCSamples.GetSample(i, &Vec3Sample)))
				return E_FAIL;

			// Lambertian Term: Max(Dot, 0)��Ҳ����˵�������ϰ��򣬷�������
			fLambertian = D3DXVec3Dot(&VecNormal, &Vec3Sample);
			if(fLambertian < 0.00001f)
				continue;
			
			// Visible Test
			if(m_Options.bShadowed)
			{
#ifdef MYPRT_USED3DXPRTINTERSECT
				iVisible = !m_pD3DXPRTEngine->ShadowRayIntersects(&PtPosition, &Vec3Sample);
#else
				iVisible = GetVisibleUpperSphere(PtPosition, Vec3Sample);
#endif

				if(iVisible)
					iVisible = TRUE;
			}
			else
				iVisible = TRUE;
				
			// Y
			pY = m_MCSamples.GetSHTable(i);
			if(!pY)
				return E_FAIL;
			
			// Product
			for(j = 0; j < m_iBandNum * m_iBandNum; j++)
				pCoef[j] += (float)iVisible * fLambertian * pY[j];
		}

		// Monte Carlo Intergration
		for(j = 0; j < m_iBandNum * m_iBandNum; j++)
			pCoef[j] = pCoef[j] * 4.0f * D3DX_PI / (float)m_iSampleNum;
		if(FAILED(pBuffer->SetVertexPRTInfo(iVertexNo, pCoef)))
			return E_FAIL;

		// Test
		//OutputSH(pCoef, 4);
	}

	// ����Direct Lighting������ɫ��Ϣ��ֻ����Shadowed��Cos Term������ֱ�Ӹ�����ɫͨ����Buffer����
	if(FAILED(pBuffer->CopyPRTBuffer(0, 1)))
		return E_FAIL;
	if(FAILED(pBuffer->CopyPRTBuffer(0, 2)))
		return E_FAIL;
	SAFE_DELETE_ARRAY(pCoef);
	return S_OK;
}








HRESULT KPRTEngine::ComputeBounce(LPPRTBUFFER pInBuffer, LPPRTBUFFER pOutBuffer, LPPRTBUFFER pAddBuffer)
{
	if(m_bSimulated || !m_bInit)
		return D3DERR_NOTAVAILABLE;
	if(!pInBuffer || !pOutBuffer)
		return D3DERR_INVALIDCALL;

	// �ȵõ�ģ������
	if(!m_iVertexNum)
		return E_FAIL;

	// ������ÿ�������ÿ���������PRT����
	UINT i = 0, j = 0, k = 0;
	D3DXVECTOR3 Vec3Sample(0.0f, 0.0f, 0.0f), VecNormal(0.0f, 0.0f, 0.0f), PtPosition(0.0f, 0.0f, 0.0f);
	D3DXVECTOR2 Vec2Sample(0.0f, 0.0f);
	float fLambertian = 0.0f, fAlbedo = 1.0f;
	float *pCoef[3] = {NULL, NULL, NULL};		// ��ʱʹ�õ�ϵ�����ֱ��ʾRGB����Ϊ����Ҫǣ�������ն�ת��Ϊ����ȣ�����Ҫ�˷����ʣ�����ͨ������ֿ�
	for(k = 0; k < 3; k++)
	{
		pCoef[k] = new float[m_iBandNum * m_iBandNum];
		if(!pCoef[k])
			return E_OUTOFMEMORY;
	}

	float *pTFVector1 = new float[m_iBandNum * m_iBandNum], *pTFVector2 = new float[m_iBandNum * m_iBandNum], *pTFVector3 = new float[m_iBandNum * m_iBandNum], *pTFVector = new float[m_iBandNum * m_iBandNum];
	if(!pTFVector || !pTFVector1 || !pTFVector2 || !pTFVector3)
		return E_OUTOFMEMORY;

	// ����Ϣ
	BOOL bIntersect = FALSE;
	UINT iIntersectFaceNo = 0;
	D3DXVECTOR3 PtBaryCentric(0, 0, 0);
	float fLength = 0.0f;

	// Debug���������Ϣ
	static UINT s_iBounceNo = 0;
	char szInfo[100];	// ��ǰ������ɵĶ������
	s_iBounceNo++;	// ÿ����һ�ξ͵���

	// for each vertex
	for(UINT iVertexNo = 0; iVertexNo < m_iVertexNum; iVertexNo++)
	{
		m_iCurrentVertexNo = iVertexNo;
		sprintf(szInfo, "Bounce %d Lighting %.2f%% �� %d / %d\n", s_iBounceNo, (float)iVertexNo/(float)m_iVertexNum*100.0f, iVertexNo, m_iVertexNum);
		OutputDebugString(szInfo);

		// ��ʱϵ����������
		for(k = 0; k < 3; k++)
			for(i = 0; i < m_iBandNum * m_iBandNum; i++)
				pCoef[k][i] = 0.0f;

		PtPosition = m_pNewVB[iVertexNo * 2];
		VecNormal = m_pNewVB[iVertexNo * 2 + 1];

		// Monte Carlo Ray Tracing
		for(i = 0; i < m_iSampleNum; i++)
		{
			// ����ȷ��һ����������
			if(FAILED(m_MCSamples.GetSample(i, &Vec3Sample)))
				return E_FAIL;

			// Lambertian Term: Max(Dot, 0)��Ҳ����˵�������ϰ��򣬷�������
			fLambertian = D3DXVec3Dot(&VecNormal, &Vec3Sample);
			if(fLambertian < 0.00001f)
				continue;

			// Get Closest Intersect Face
#ifdef MYPRT_USED3DXPRTINTERSECT
			bIntersect = m_pD3DXPRTEngine->ClosestRayIntersects(&PtPosition, &Vec3Sample, (DWORD *)&iIntersectFaceNo, &PtBaryCentric.y, &PtBaryCentric.z, &fLength);
			PtBaryCentric.x = 1 - PtBaryCentric.y - PtBaryCentric.z;
			// �Ƿ�����ʾ���ཻ
			if(PtBaryCentric.x < 0 || PtBaryCentric.x > 1 || PtBaryCentric.y < 0 || PtBaryCentric.y > 1 || PtBaryCentric.z < 0 || PtBaryCentric.z > 1)
				bIntersect = FALSE;
#else
			bIntersect = GetClosestIntersectPointUpperSphere(PtPosition, Vec3Sample, &iIntersectFaceNo, &PtBaryCentric, &fLength);
#endif

			// û�ཻ˵��û�е�������׷��նȣ�����
			if(!bIntersect || fLength < 0.000001f)
				continue;

			// Get Vertex TF Vector, Red Only��ȫChannel���ں���ֱ��Ƴ����˲�ͬ��Albedo����
			for(k = MYPRT_REDCHANNEL; k <= MYPRT_BLUECHANNEL; k++)
			{
				if(FAILED(pInBuffer->GetVertexPRTInfo(m_pFaceVertexInfo[iIntersectFaceNo*3], pTFVector1, k)))
					return E_FAIL;
				if(FAILED(pInBuffer->GetVertexPRTInfo(m_pFaceVertexInfo[iIntersectFaceNo*3+1], pTFVector2, k)))
					return E_FAIL;
				if(FAILED(pInBuffer->GetVertexPRTInfo(m_pFaceVertexInfo[iIntersectFaceNo*3+2], pTFVector3, k)))
					return E_FAIL;
				// Get Pixel TF Vector, Using Linear Interpolation
				for(j = 0; j < m_iBandNum * m_iBandNum; j++)
					pTFVector[j] = (PtBaryCentric.x * pTFVector1[j]) + (PtBaryCentric.y * pTFVector2[j]) + (PtBaryCentric.z * pTFVector3[j]);

				// �õ�Albedo�������AlbedoҲ���Գ˶��������Albedo��Diffuse��
				if(m_pDiffuse)
				{
					if(k == MYPRT_REDCHANNEL)
					{
						fAlbedo = m_pDiffuse[m_pFaceVertexInfo[iIntersectFaceNo*3]].r + m_pDiffuse[m_pFaceVertexInfo[iIntersectFaceNo*3+1]].r + m_pDiffuse[m_pFaceVertexInfo[iIntersectFaceNo*3+2]].r;
						fAlbedo /= 3.0f;
					}
					if(k == MYPRT_GREENCHANNEL)
					{
						fAlbedo = m_pDiffuse[m_pFaceVertexInfo[iIntersectFaceNo*3]].g + m_pDiffuse[m_pFaceVertexInfo[iIntersectFaceNo*3+1]].g + m_pDiffuse[m_pFaceVertexInfo[iIntersectFaceNo*3+2]].g;
						fAlbedo /= 3.0f;
					}
					if(k == MYPRT_BLUECHANNEL)
					{
						fAlbedo = m_pDiffuse[m_pFaceVertexInfo[iIntersectFaceNo*3]].b + m_pDiffuse[m_pFaceVertexInfo[iIntersectFaceNo*3+1]].b + m_pDiffuse[m_pFaceVertexInfo[iIntersectFaceNo*3+2]].b;
						fAlbedo /= 3.0f;
					}
				}
				else
					fAlbedo = m_Options.fAlbedo[k];

				// Indirect Lighting = Previous Lighting * cos term * Albedo
				for(j = 0; j < m_iBandNum * m_iBandNum; j++)
					pCoef[k][j] += fAlbedo * fLambertian * pTFVector[j] / D3DX_PI;
			}
		}

		// Monte Carlo Intergration
		for(k = 0; k < 3; k++)
		{
			for(j = 0; j < m_iBandNum * m_iBandNum; j++)
				pCoef[k][j] = pCoef[k][j] * 4.0f * D3DX_PI / (float)m_iSampleNum;
			if(FAILED(pOutBuffer->SetVertexPRTInfo(iVertexNo, pCoef[k], k)))
				return E_FAIL;
		}
		

		// Test
		//OutputSH(pCoef, 4);
	}
	SAFE_DELETE_ARRAY(pCoef[0]);
	SAFE_DELETE_ARRAY(pCoef[1]);
	SAFE_DELETE_ARRAY(pCoef[2]);
	SAFE_DELETE_ARRAY(pTFVector);
	SAFE_DELETE_ARRAY(pTFVector1);
	SAFE_DELETE_ARRAY(pTFVector2);
	SAFE_DELETE_ARRAY(pTFVector3);

	return S_OK;
}









HRESULT KPRTEngine::ComputeSSS(LPPRTBUFFER pInBuffer, LPPRTBUFFER pOutBuffer, LPPRTBUFFER pAddBuffer)
{
	if(m_bSimulated || !m_bInit)
		return D3DERR_NOTAVAILABLE;
	if(!pInBuffer || !pOutBuffer)
		return D3DERR_INVALIDCALL;

	// �ȵõ�ģ������
	if(!m_iVertexNum)
		return E_FAIL;

	// ������ÿ�������ÿ���������PRT����
	UINT i = 0, j = 0, k = 0;
	D3DXVECTOR3 VecRay(0.0f, 0.0f, 0.0f), VecNormal(0.0f, 0.0f, 0.0f), PtPosition(0.0f, 0.0f, 0.0f);
	float fRd = 1.0f, fFt = 0.0f, fr2 = 0.0f;
	float *pCoef[3] = {NULL, NULL, NULL};		// ��ʱʹ�õ�ϵ�����ֱ��ʾRGB
	for(k = 0; k < 3; k++)
	{
		pCoef[k] = new float[m_iBandNum * m_iBandNum];
		if(!pCoef[k])
			return E_OUTOFMEMORY;
	}

	float *pTFVector = new float[m_iBandNum * m_iBandNum];
	if(!pTFVector)
		return E_OUTOFMEMORY;

	float fLength = 0.0f;

	// Debug���������Ϣ
	static UINT s_iScatterNo = 0;
	char szInfo[100];	// ��ǰ������ɵĶ������


	float fSr = 0.0f, fSv = 0.0f, fZr[3], fZv[3], fSqrtExtinct[3], fFdr = 0.0f, fExtinct = 0.0f, fRefrIndex = m_Options.fRelativeIndexofRefraction;
	// ����Rd���ٵĶ�����Zr/v, fSqrtExtinct��������Щֻ�Ǻͽ���������أ����Էŵ���ǰ�����һ�μ���
	fFdr = -1.44f / (fRefrIndex*fRefrIndex) + 0.71f / fRefrIndex + 0.668f + 0.0636f * fRefrIndex;
	for(k = 0; k < 3; k++)
	{
		fExtinct = m_Options.fScatteringCoef[k] + m_Options.fAbsorbation[k];	// ����ϵ��
		
		fSqrtExtinct[k] = sqrtf(3.0f * fExtinct * m_Options.fAbsorbation[k]);

		fZr[k] = 1.0f / fExtinct;
		fZv[k] = fZr[k] + fZr[k] * 1.33333333f * (1+fFdr) / (1-fFdr);
	}


	// Test
	float fLambertian = 0.0f;
	UINT iCurrentVertexNum = 0;

	// for each vertex
	for(UINT iVertexNo = 0; iVertexNo < m_iVertexNum; iVertexNo++)
	{
		m_iCurrentVertexNo = iVertexNo;
		sprintf(szInfo, "Scatter %d Lighting %.2f%% �� %d / %d\n", s_iScatterNo, (float)iVertexNo/(float)m_iVertexNum*100.0f, iVertexNo, m_iVertexNum);
		OutputDebugString(szInfo);


		PtPosition = m_pNewVB[iVertexNo * 2];
		VecNormal = m_pNewVB[iVertexNo * 2 + 1];

		// ��ʱϵ����������
		for(k = MYPRT_REDCHANNEL; k <= MYPRT_BLUECHANNEL; k++)
			for(j = 0; j < m_iBandNum * m_iBandNum; j++)
				pCoef[k][j] = 0.0f;

		// Test
		iCurrentVertexNum = 0;

		// Monte Carlo Intergration���������ж�����SSS���׶�
		for(i = 0; i < m_iVertexNum; i++)
		{
			if(m_iCurrentVertexNo == i)
				continue;

			// ���������r����ͬ����Ļ�������
			VecRay = m_pNewVB[i * 2] - PtPosition;
			fr2 = VecRay.x * VecRay.x + VecRay.y * VecRay.y + VecRay.z * VecRay.z;
			if(fr2 < 0.000001f)
				continue;
			// �������������ž������ӣ���Ϊ����ľ�����ƽ����������������ҲҪƽ��
			fr2 *= m_Options.fLengthScale * m_Options.fLengthScale;

			// Test Lower Sphere Intergration
//			D3DXVec3Normalize(&VecRay, &VecRay);
//			fLambertian = D3DXVec3Dot(&VecNormal, &VecRay);
//			if(fLambertian > -0.0000001f)
//				continue;

			// Get Vertex TF Vector, Red Only��ȫChannel���ں���ֱ��Ƴ����˲�ͬ��Albedo����
			for(k = MYPRT_REDCHANNEL; k <= MYPRT_BLUECHANNEL; k++)
			{
				// �õ���Դ���SH����
				if(FAILED(pInBuffer->GetVertexPRTInfo(i, pTFVector, k)))
					return E_FAIL;


				// ����Sr/v
				fSr = sqrtf(fr2 + fZr[k] * fZr[k]);
				fSv = sqrtf(fr2 + fZv[k] * fZv[k]);

				// �õ�Rd
				fRd = GetBSSRDF_Rd(fSv, fSr, fZv, fZr, fSqrtExtinct, k);

				// ��TF * Ft * Rd�õ�ɢ��TF
				for(j = 0; j < m_iBandNum * m_iBandNum; j++)
					pCoef[k][j] += fRd * pTFVector[j];

			}// end for color index

			// Test
			iCurrentVertexNum++;

		}// end for Intergration

		// Monte Carlo Intergration Coeffcient
		for(k = 0; k < 3; k++)
		{
			for(j = 0; j < m_iBandNum * m_iBandNum; j++)
				pCoef[k][j] = pCoef[k][j] * 4.0f * D3DX_PI / (float)iCurrentVertexNum;//m_iVertexNum;
			if(FAILED(pOutBuffer->SetVertexPRTInfo(iVertexNo, pCoef[k], k)))
				return E_FAIL;
		}


		// Test
		//OutputSH(pCoef, 4);
	}// end for each vertex

	SAFE_DELETE_ARRAY(pCoef[0]);
	SAFE_DELETE_ARRAY(pCoef[1]);
	SAFE_DELETE_ARRAY(pCoef[2]);
	SAFE_DELETE_ARRAY(pTFVector);

	s_iScatterNo++;	// ÿ����һ�ξ͵���

	return S_OK;
}



























HRESULT KPRTEngine::ComputeLDPRT(LPPRTBUFFER pBuffer)
{
	if(m_bSimulated || !m_bInit || !m_Options.iLDPRTLobeNum)
		return D3DERR_NOTAVAILABLE;

	if(!m_iVertexNum)
		return E_FAIL;

	// ������ÿ�������ÿ���������PRT����
	UINT i = 0, j = 0, k=0, iColorIndex = 0, iIndex = 0;
	UINT iLobeNum = m_Options.iLDPRTLobeNum, iLobeNo = 0;
	SHTable SHFunction;
	if(FAILED(SHFunction.Init(m_iBandNum)))
		return E_FAIL;
	
	D3DXVECTOR2 Vec2Normal(0.0f, 0.0f);

	float fSum = 0.0f, *pY = NULL;
	float *pPRTCoef = new float[m_iBandNum*m_iBandNum], *pLDPRTCoef = new float[m_iBandNum*iLobeNum], *pLDPRTTempCoef = new float[m_iBandNum], *pCombineCoef = new float[m_iBandNum*m_iBandNum], *pLeavingCoef = new float[m_iBandNum*m_iBandNum];
	if(!pPRTCoef || !pLDPRTCoef || !pLDPRTTempCoef || !pCombineCoef || !pLeavingCoef)
		return E_OUTOFMEMORY;

	char szInfo[100];	// Debug�����ǰ������ɵĶ������

	// for each vertex
	for(UINT iVertexNo = 0; iVertexNo < m_iVertexNum; iVertexNo++)
	{
		m_iCurrentVertexNo = iVertexNo;
		sprintf(szInfo, "LDPRT %.2f%% �� %d / %d\n", (float)iVertexNo/(float)m_iVertexNum*100.0f, iVertexNo, m_iVertexNum);
		OutputDebugString(szInfo);

		// �ٵõ��淨�ߣ��������꣩
		CartesianToSpherical(m_pNewVB[iVertexNo*2+1], &Vec2Normal);

		// Y
		if(FAILED(SHFunction.GenerateSH(Vec2Normal)))
			return E_FAIL;
		pY = SHFunction.GetSHTable();
		if(!pY)
			return E_FAIL;

		// ѭ����ɫͨ��
		for(iColorIndex = MYPRT_REDCHANNEL; iColorIndex <= MYPRT_BLUECHANNEL; iColorIndex++)
		{
			// �ȵõ�PRT����
			if(FAILED(pBuffer->GetVertexPRTInfo(iVertexNo, pPRTCoef, iColorIndex)))
				return E_FAIL;

			// ����Single Lobe Fitting
			iLobeNo = 0;	// ��ǰLobeΪ0
			for(iIndex = 0, i = 0; i < m_iBandNum; i++)
			{
				fSum = 0.0f;
				for(j = 0; j < 2*i+1; j++, iIndex++)
					fSum += pPRTCoef[iIndex] * pY[iIndex];
				pLDPRTCoef[i] = fSum * 4*D3DX_PI / (float)(2*i+1);
			}
			
			// ѭ����Multi Lobe Fitting����ʵ�����ö��㷨�����̶��ᣬMulti Lobe�Ѿ�û�������ˣ�������ʱ��֧��
			for(iLobeNo = 1; iLobeNo < iLobeNum; iLobeNo++)
			{
				// �ȵõ���һ�μ���ĺϳ�����
				for(iIndex = 0, i = 0; i < m_iBandNum; i++)
				{
					for(j = 0; j < 2*i+1; j++, iIndex++)
						pCombineCoef[iIndex] = pLDPRTCoef[(iLobeNo-1)*m_iBandNum + i] * pY[iIndex];
				}
					
				// ������������
				for(i = 0; i < m_iBandNum*m_iBandNum; i++)
				{
					pLeavingCoef[i] = pPRTCoef[i] - pCombineCoef[i];
				}
				// ������������Single Lobe Fitting��д����ʱ����
				for(iIndex = 0, i = 0; i < m_iBandNum; i++)
				{
					fSum = 0.0f;
					for(j = 0; j < 2*i+1; j++, iIndex++)
						fSum += pLeavingCoef[iIndex] * pY[iIndex];
					pLDPRTTempCoef[i] = fSum * 4*D3DX_PI / (float)(2*i+1);
				}

				// д��ԭBuffer��
				for(i = 0; i < m_iBandNum; i++)
				{
					pLDPRTCoef[iLobeNo*m_iBandNum + i] += pLDPRTTempCoef[i];
				}
			}

			// Test �����������ع�д��ԭ���壬����������ȷ��
/*			// ������ת
			D3DXVECTOR3 VecNormal;
			D3DXMATRIX Mat;
			D3DXMatrixRotationZ(&Mat, D3DX_PI);
			D3DXVec3TransformCoord(&VecNormal, &m_pNewVB[iVertexNo*2+1], &Mat);
			// �ٵõ��淨�ߣ��������꣩
			CartesianToSpherical(VecNormal, &Vec2Normal);

			// Y
			if(FAILED(SHFunction.GenerateSH(Vec2Normal)))
				return E_FAIL;
			pY = SHFunction.GetSHTable();
			if(!pY)
				return E_FAIL;

			for(iIndex = 0, i = 0; i < m_iBandNum; i++)
			{
				for(j = 0; j < 2*i+1; j++, iIndex++)
				{
					pCombineCoef[iIndex] = 0.0f;
					for(k = 0; k < iLobeNum; k++)
					{
						pCombineCoef[iIndex] += pLDPRTCoef[k*m_iBandNum + i] * pY[iIndex];
					}
				}
				if(FAILED(pBuffer->SetVertexPRTInfo(iVertexNo, pCombineCoef, iColorIndex)))
					return E_FAIL;
			}
*/

			// Write LDPRT Data
			if(FAILED(pBuffer->SetVertexLDPRTInfo(iVertexNo, pLDPRTCoef, iColorIndex)))
				return E_FAIL;
		}
	}

	SAFE_DELETE_ARRAY(pPRTCoef);
	SAFE_DELETE_ARRAY(pLDPRTCoef);
	SAFE_DELETE_ARRAY(pLDPRTTempCoef);
	SAFE_DELETE_ARRAY(pCombineCoef);
	SAFE_DELETE_ARRAY(pLeavingCoef);

	// ����only LDPRT

	return S_OK;
}
