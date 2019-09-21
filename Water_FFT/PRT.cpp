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

	// Options有效性检查
	if(pObject->Options.iBounceNum > MYPRT_MAXBOUNCE || pObject->Options.iLDPRTLobeNum > MYLDPRT_MAXLOBE)
		return D3DERR_INVALIDCALL;
	if(pObject->Options.bSSS && pObject->Options.fLengthScale < 0.0000001f)
	{
		OutputDebugString("SSS Length Scale非法！！");
		return D3DERR_INVALIDCALL;
	}

	// Mesh格式有效性检查，必须有NORMAL！
	if( !(pObject->pMesh->GetFVF() & D3DFVF_NORMAL) )
	{
		OutputDebugString("Mesh没有法线！！先计算好再进行PRT计算！！\n");
		return D3DERR_INVALIDCALL;
	}


	// 创建D3DXPRTEngine
#ifdef MYPRT_USED3DXPRTINTERSECT
	if(FAILED(D3DXCreatePRTEngine(pObject->pMesh, NULL, FALSE, NULL, &m_pD3DXPRTEngine)))
		return E_FAIL;
#endif


	UINT i = 0, j = 0;
	
	// 初始化数据
	// 复制Options
	memcpy(&m_Options, &pObject->Options, sizeof(PRTATTRIBUTE));
	m_pObject = pObject;

	m_pPRTBuffer = pPRTBuffer;
	m_pPRTBuffer->Release();

	// 分配PRT Buffer
	if(FAILED(m_pPRTBuffer->Init(m_iBandNum, m_pObject)))
		return E_FAIL;




	// 初始化模型信息
	if(FAILED(InitGeometry()))
		return E_FAIL;

	// 如果想强制使用自定义的Albedo，就在这里把模型自身的Albedo信息释放掉
	//SAFE_DELETE_ARRAY(m_pDiffuse);




///////////////////////////////////////////////// 下来开始Ray Tracing计算TF向量

	// 先做普通的+Shadow(不加BRDF即Reflectancy, 所以这个是Direct Lighting Irradiance)
	if(FAILED(ComputeDirectLighting(m_pPRTBuffer)))
		return E_FAIL;


	// 把间接光照辐照度作为光源来算光线反弹和SSS
	UINT iBounce = 0, k = 0;

	// 每次反弹的间接光照光源辐照度Buffer，先初始化
	if(m_Options.iBounceNum || m_Options.bSSS)
	{
		// 因为第一次间接光照的光源就是直接光照的结果，所以要多一项
		// 考虑到第一次SSS还要占用一个Buffer的位置，所以再多一项
		LPPRTBUFFER pIndirectBuffer = new PRTBUFFER[m_Options.iBounceNum+2];
		// 临时使用，用于计算每次间接光照作为光源时的散射光能和反弹光能Buffer
		PRTBUFFER BounceBuffer;

		for(iBounce = 0; iBounce < (m_Options.iBounceNum+2); iBounce++)
			if(FAILED(pIndirectBuffer[iBounce].Init(m_iBandNum, m_pObject)))
				return E_FAIL;
		if(FAILED(BounceBuffer.Init(m_iBandNum, m_pObject)))
			return E_FAIL;

		// 先初始化，清零
		for(iBounce = 0; iBounce < (m_Options.iBounceNum+2); iBounce++)
			for(j = MYPRT_REDCHANNEL; j <= MYPRT_BLUECHANNEL; j++)
			{
				if(FAILED(pIndirectBuffer[iBounce].SetToZero(j)))
					return E_FAIL;;
			}
		for(j = MYPRT_REDCHANNEL; j <= MYPRT_BLUECHANNEL; j++)
		{
			// 第一次间接光照光源就是直接光照的结果，所以Indirect[0] = PRTBuffer
			// 同理，第一次间接光照的结果就是第二次间接光照的光源，存放在Indirect[1]中
			if(FAILED(pIndirectBuffer[0].AddBuffer(j, j, m_pPRTBuffer)))
				return E_FAIL;;
			if(FAILED(BounceBuffer.SetToZero(j)))
				return E_FAIL;
		}


		// 下来算无SSS的反弹
		if(!m_Options.bSSS)
		{
			for(iBounce = 0; iBounce < m_Options.iBounceNum; iBounce++)
			{
				if(FAILED(ComputeBounce(pIndirectBuffer+iBounce, &BounceBuffer, NULL)))
					return E_FAIL;

				// 每一次的反弹作为下一次的纯间接光照光源Buffer
				for(j = MYPRT_REDCHANNEL; j <= MYPRT_BLUECHANNEL; j++)
				{
					if(FAILED(pIndirectBuffer[iBounce+1].AddBuffer(j, j, &BounceBuffer)))
						return E_FAIL;
				}
			}// end Indirect Lighting

			// 间接光照计算完成，将每次的间接光照结果加到PRT Buffer上，跳过第一次的（直接光照）
			for(k = 1; k <= m_Options.iBounceNum; k++)
				for(j = MYPRT_REDCHANNEL; j <= MYPRT_BLUECHANNEL; j++)
				{
					if(FAILED(m_pPRTBuffer->AddBuffer(j, j, pIndirectBuffer+k)))
						return E_FAIL;
				}
		}// end if no sss




		// 现在算有SSS的反弹
		else
		{
			// 根据原始辐照度做SSS存放到Indirect Buffer1
			if(FAILED(ComputeSSS(pIndirectBuffer, pIndirectBuffer+1, NULL)))
				return E_FAIL;

			for(iBounce = 1; iBounce <= m_Options.iBounceNum; iBounce++)
			{

				// 遵循光线的传播，根据刚得到的散射光能，再做Bounce，即得到散射光再次辐射到物体表面的光能
				if(FAILED(ComputeBounce(pIndirectBuffer+iBounce, &BounceBuffer, NULL)))
					return E_FAIL;

				// 根据原始辐照度做SSS存放到下一个Indirect Buffer
				if(FAILED(ComputeSSS(&BounceBuffer, pIndirectBuffer+iBounce+1, NULL)))
					return E_FAIL;

			}// end Indirect Lighting


			// Test, Get Pure Scattering Light
			for(j = MYPRT_REDCHANNEL; j <= MYPRT_BLUECHANNEL; j++)
				if(FAILED(m_pPRTBuffer->SetToZero(j)))
					return E_FAIL;


			// 间接光照SSS计算完成，将每次的间接光照SSS结果加到PRT Buffer上，跳过第一次的（直接光照）
			// 因为不反弹的时候也会有一个附加的SSS，所以要多加一项
			for(k = 1; k <= m_Options.iBounceNum+1; k++)
				for(j = MYPRT_REDCHANNEL; j <= MYPRT_BLUECHANNEL; j++)
				{
					if(FAILED(m_pPRTBuffer->AddBuffer(j, j, pIndirectBuffer+k)))
						return E_FAIL;
				}

		}// end if sss


		// 释放临时
		SAFE_DELETE_ARRAY(pIndirectBuffer);
		BounceBuffer.Release();

	}// if bounce or sss



	// 乘Albedo，从辐照度得到最终的辐射度，这里的Albedo也可以乘顶点自身的Albedo（Diffuse）
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


	// 最后做LDPRT，Buffer对Buffer进行，考虑到OnlyLDPRT会改变Stride，所以这里不要用GetPRTInfo
	// 其实多流也可以只选择其中一部分数据映射到输入寄存器的，所以这里暂时不重新分配
	if(m_Options.iLDPRTLobeNum)
		if(FAILED(ComputeLDPRT(m_pPRTBuffer)))
			return E_FAIL;

	// 好，创建PRT BUFFER VB
	if(FAILED(m_pPRTBuffer->ApplyToVB()))
		return E_FAIL;

	// 释放所有临时内部数据

	m_bSimulated = TRUE;
	return S_OK;
}






HRESULT KPRTEngine::InitGeometry()
{
	UINT i = 0, j = 0;

	///////////////////////////////////////////////// 需要将每个模型的顶点缓冲重新复制一份，将原始状态矩阵加在上面，即分配VBList
	LPD3DXMESH pMesh = m_pObject->pMesh;
	D3DXMATRIX MatCombine, MatRotation;
	MatCombine = m_pObject->MatRotation * m_pObject->MatScaling * m_pObject->MatTranslation;
	MatRotation = m_pObject->MatRotation;

	UINT iVertexNum = pMesh->GetNumVertices();
	if(!iVertexNum)
		return E_FAIL;
	m_iVertexNum = iVertexNum;
	m_iCurrentVertexNo = 0;

	// 复制，只复制顶点坐标和法线
	// 得到模型属性
	UINT iStride = pMesh->GetNumBytesPerVertex();
	if(!iStride)
		return E_FAIL;
	DWORD dwFVF = pMesh->GetFVF();
	if(!dwFVF)
		return E_FAIL;

	// 得到源数据
	BYTE *pSourceData = NULL;
	if(FAILED(pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void **)&pSourceData)))
		return E_FAIL;

	// 创建并得到新数据，法线+顶点是24字节，6浮点数，2向量
	SAFE_DELETE_ARRAY(m_pNewVB);
	m_pNewVB = new D3DXVECTOR3[m_iVertexNum * 2];
	if(!m_pNewVB)
		return E_OUTOFMEMORY;

	// 如果有顶点漫反射数据，就创建
	SAFE_DELETE_ARRAY(m_pDiffuse);
	if(dwFVF & D3DFVF_DIFFUSE)
	{
		m_pDiffuse = new D3DXCOLOR[m_iVertexNum];
		if(!m_pDiffuse)
			return E_OUTOFMEMORY;
	}

	// 开始转换数据
	LPD3DXVECTOR3 pSrcData = NULL, pDstData = m_pNewVB;
	DWORD *pSrcDiffuse = NULL;
	LPD3DXCOLOR pDstDiffuse = m_pDiffuse;

	for( j = 0; j < m_iVertexNum; j++, pSourceData+=iStride)
	{
		pSrcData = (LPD3DXVECTOR3)pSourceData;

		// 先写入顶点数据
		D3DXVec3TransformCoord(pDstData, pSrcData, &MatCombine);
		pSrcData++;
		pDstData++;

		// 再写入法线数据
		D3DXVECTOR3 VecNormal(0, 0, 0);
		D3DXVec3TransformCoord(&VecNormal, pSrcData, &MatRotation);
		D3DXVec3Normalize(&VecNormal, &VecNormal);
		if(absf(D3DXVec3Length(&VecNormal) - 1.0f) > 0.0001f)
		{
			OutputDebugString("法线数据有误！！请检查模型的有效性！\n");
			return E_FAIL;
		}
		*pSrcData++;
		*pDstData++ = VecNormal;

		// 判断是否有Diffuse，有就写入，因为Diffuse是接着法线的（中间的FVF流数据类型都不常用，忽略）
		if((dwFVF && D3DFVF_DIFFUSE) && m_pDiffuse)
		{
			pSrcDiffuse = (DWORD *)pSrcData;
			*pDstDiffuse++ = DWCOLORTOVECTOR(*pSrcDiffuse);
		}

	}

	// 写入完毕
	if(FAILED(pMesh->UnlockVertexBuffer()))
		return E_FAIL;



	/////////////////////////////////////////////////// 写入索引缓冲
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



	//////////////////////////////////////////////////////写入加速面和顶点数据
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
		// 先写入面对应的顶点信息
		if(FAILED(GetFaceInfo(i, m_pFaceVertexInfo + i*3, m_pFaceVB + i*6, m_pFaceVB+i*6+3)))
			return E_FAIL;

		// 下来计算面自身的法线信息
		D3DXVec3Normalize(&VecLine1, &(m_pFaceVB[i*6+1]-m_pFaceVB[i*6+0]));
		D3DXVec3Normalize(&VecLine2, &(m_pFaceVB[i*6+2]-m_pFaceVB[i*6+0]));
		D3DXVec3Cross(&VecNormal, &VecLine1, &VecLine2);
		D3DXVec3Normalize(&VecNormal, &VecNormal);
		if(absf(D3DXVec3Length(&VecNormal) - 1.0f) > 0.0001f)
		{
			OutputDebugString("顶点数据有误！！请检查模型的有效性！\n");
			return E_FAIL;
		}

		// 算顶点法线均值
		VecNormalVertex = (m_pFaceVB[i*6+3] + m_pFaceVB[i*6+4] + m_pFaceVB[i*6+5]) / 3.0f;
		// 如果两个法线异向，就将面法线取反，总之跟顶点法线要同向
		if(D3DXVec3Dot(&VecNormalVertex, &VecNormal) < 0.0f)
			VecNormal *= -1.0f;
		// 写入
		m_pFaceNormal[i] = VecNormal;
	}


	/////////////////////////////////////////////////////////写入每个顶点对应的上半球和下半球面信息，用于加速射线跟踪
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

	// 临时使用的
	UINT *pVertexUpperData = new UINT[m_iFaceNum];
	UINT *pVertexLowerData = new UINT[m_iFaceNum];
	if(!pVertexLowerData || !pVertexUpperData)
		return E_OUTOFMEMORY;


	UINT iUpperFaceNum = 0, iLowerFaceNum = 0;
	float fDot1 = 0.0f, fDot2 = 0.0f, fDot3 = 0.0f;

	// 对每个顶点
	for(i = 0; i < m_iVertexNum; i++)
	{
		// 先初始化每个顶点用到的数据
		iUpperFaceNum = iLowerFaceNum = 0;
		VecNormalVertex = m_pNewVB[i*2+1];

		// 先把全部面整个过一遍，将数据填入临时缓冲
		for(j = 0; j < m_iFaceNum; j++)
		{
			// 求三个点分别到当前点的向量
			VecLine1 = m_pFaceVB[j*6] - m_pNewVB[i*2];
			VecLine2 = m_pFaceVB[j*6+1] - m_pNewVB[i*2];
			VecLine3 = m_pFaceVB[j*6+2] - m_pNewVB[i*2];
			D3DXVec3Normalize(&VecLine1, &VecLine1);
			D3DXVec3Normalize(&VecLine2, &VecLine2);
			D3DXVec3Normalize(&VecLine3, &VecLine3);

			// 向量分别和顶点法线求dot
			fDot1 = D3DXVec3Dot(&VecLine1, &VecNormalVertex);
			fDot2 = D3DXVec3Dot(&VecLine2, &VecNormalVertex);
			fDot3 = D3DXVec3Dot(&VecLine3, &VecNormalVertex);
			// Dot只要有一个在上半球，就有可能相交，就得写入上半球缓冲（就是dot > 0）
			if(fDot1 > 0.0001f || fDot2 > 0.0001f || fDot3 > 0.0001f )
			{
				pVertexUpperData[iUpperFaceNum] = j;
				iUpperFaceNum++;
			}
			// 由于有些面可能跨越两个半球，所以不能用上面的+else，必须重新判断，这里是下半球
			if(fDot1 < -0.000001f || fDot2 < -0.000001f || fDot3 < -0.000001f )
			{
				pVertexLowerData[iLowerFaceNum] = j;
				iLowerFaceNum++;
			}
			// 从上面可以看出，平行的面被跳过，因为判断相交的时候是不能要平行的，否则不但影响速度还可能会出现渲染错误
		}// end for each face

		// 写入个数
		m_pVertexFaceCountUpperSphere[i] = iUpperFaceNum;
		m_pVertexFaceCountLowerSphere[i] = iLowerFaceNum;

		// 分配缓冲区
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


		// 把临时缓冲的结果写入新缓冲区
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

	// 先得到模型属性
	if(!m_iFaceNum)
		return FALSE;

	static UINT i = 0, j = 0, iFaceNo = 0;

	// 遍历所有面
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
		// 先得到在当前点上半球内的面序号
		iFaceNo = m_ppVertexFaceInfoUpperSphere[m_iCurrentVertexNo][i];
//		if(FAILED(GetFaceInfo(i, iFaceVertexNo, PtFaces, VecFaces)))
//			return FALSE;

		// 为了提速，自己在函数中完成功能就好了
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

/*		// 这里需要注意：我们要跳过那些共享射线点的面（即射线点在这些面上），且该面的方向必须是接近平行射线方向的
		// 跳过表示不相交，无遮挡
		// D3D的跟踪函数，如果射线点在面上，那么直接就会返回相交
		// 自己的跟踪函数，如果射线点在面上，直接就会返回不相交，所以这个才是我们要的，当然也可以通过判断D3D返回的fLength来去除这种情况
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
			// 得到面法线
			D3DXVECTOR3 VecNormal(0, 0, 0), VecLine1, VecLine2;
			D3DXVec3Normalize(&VecLine1, &(PtFaces[1]-PtFaces[0]));
			D3DXVec3Normalize(&VecLine2, &(PtFaces[2]-PtFaces[0]));
			D3DXVec3Cross(&VecNormal, &VecLine1, &VecLine2);
			D3DXVec3Normalize(&VecNormal, &VecNormal);

			// 和面接近平行就是和法线接近垂直
			float fDot = D3DXVec3Dot(&VecNormal, &VecRay);
			if(absf(fDot) < 0.01f)	// 大概是84度，即大于84度就认为是平行的，跳过
				continue;
		}
*/
		
		// 得到面法线

		// 和面接近平行就是和法线接近垂直
//		float fDot = D3DXVec3Dot(&m_pFaceNormal[iFaceNo], &VecRay);
//		if(absf(fDot) < 0.001f)	// 平行的，跳过
//			continue;

		// 射线跟踪求交
//		bIntersect = GetIntersectTriangle3D(PtStart, VecRay, PtFaces[0], PtFaces[1], PtFaces[2], &PtBary, &p1, NULL, &Intersect);
		bIntersect = D3DXIntersectTri(&PtFaces[0], &PtFaces[1], &PtFaces[2], &PtStart, &VecRay, &p1, &p2, &fLength);
		if(bIntersect && fLength < 0.000001f)	// 点在面上
		{
			D3DXVECTOR3 VecVertexNormal = m_pNewVB[m_iCurrentVertexNo * 2+1];
			PtStart += VecVertexNormal * 0.0001f;
			bIntersect = D3DXIntersectTri(&PtFaces[0], &PtFaces[1], &PtFaces[2], &PtStart, &VecRay, &p1, &p2, &fLength);
		}
		// 有一个相交，说明有遮挡，不可见，直接返回
		if(bIntersect)
			return FALSE;
		else
			continue;
	}

	// 没有相交的，那说明没有遮挡，可见
	return TRUE;
}



BOOL KPRTEngine::GetClosestIntersectPointUpperSphere(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, UINT *pFaceNo, LPD3DXVECTOR3 pPtBaryCentric, float *pLength)
{
	if(!pFaceNo || !pPtBaryCentric || !pLength)
		return FALSE;

	// 先得到模型属性
	if(!m_iFaceNum)
		return FALSE;

	// 遍历所有面
	static D3DXVECTOR3 PtFaces[3], VecFaces[3];
	static UINT iFaceVertexNo[3] = {0, 0, 0};
	static BOOL bIntersect = FALSE;
	
	static UINT i = 0, iFaceNo = 0;

	// Temp Use
	static float fLength = 0.0f;
	static D3DXVECTOR3 PtBaryCentric(0, 0, 0);

	// 输出值
	static int iClosestFaceNo = -1;		// 初始为不存在的面
	static D3DXVECTOR3 PtClosestBaryCentric(0, 0, 0);
	static float fClosestLength = 999999.0f;	// 初始长度为最大
	fClosestLength = 999999.0f;
	iClosestFaceNo = -1;

	for(i = 0; i < m_pVertexFaceCountUpperSphere[m_iCurrentVertexNo]; i++)
	{
		// 先得到在当前点上半球内的面序号
		iFaceNo = m_ppVertexFaceInfoUpperSphere[m_iCurrentVertexNo][i];

		// 为了提速，自己在函数中完成功能就好了
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

		// 求交，注意共面的情况
		bIntersect = D3DXIntersectTri(&PtFaces[0], &PtFaces[1], &PtFaces[2], &PtStart, &VecRay, &PtBaryCentric.y, &PtBaryCentric.z, &fLength);
		if(bIntersect && fLength < 0.000001f)	// 点在面上，注意这里跟上面判断遮挡不同，这里是要判断其他点相对该点点光照贡献度，如果点在面上，这个贡献度就变成自身对自身的贡献，就没意义了，所以要跳过
		{
			//bIntersect = FALSE;
			continue;
		}

		PtBaryCentric.x = 1 - PtBaryCentric.y - PtBaryCentric.z;
		if(PtBaryCentric.x < 0 || PtBaryCentric.x > 1 || PtBaryCentric.y < 0 || PtBaryCentric.y > 1 || PtBaryCentric.z < 0 || PtBaryCentric.z > 1)
			continue;
		// 有一个相交，说明有遮挡，如果比当前的最小长度小，记下长度、重心坐标和面序号
		if(bIntersect && fLength < fClosestLength)
		{
			PtClosestBaryCentric = PtBaryCentric;
			fClosestLength = fLength;
			iClosestFaceNo = iFaceNo;
		}
	}

	// 没有相交的，那说明没有遮挡，可见
	if(iClosestFaceNo == -1)
		return FALSE;

	// 结束，填充数据
	*pFaceNo = iClosestFaceNo;
	*pPtBaryCentric = PtClosestBaryCentric;
	*pLength = fClosestLength;
	return TRUE;
}




BOOL KPRTEngine::GetClosestIntersectPointLowerSphere(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, UINT *pFaceNo, LPD3DXVECTOR3 pPtBaryCentric, float *pLength)
{
	if(!pFaceNo || !pPtBaryCentric || !pLength)
		return FALSE;

	// 先得到模型属性
	if(!m_iFaceNum)
		return FALSE;

	// 遍历所有面
	static D3DXVECTOR3 PtFaces[3], VecFaces[3];
	static UINT iFaceVertexNo[3] = {0, 0, 0};
	static BOOL bIntersect = FALSE;

	static UINT i = 0, iFaceNo = 0;

	// Temp Use
	static float fLength = 0.0f;
	static D3DXVECTOR3 PtBaryCentric(0, 0, 0);

	// 输出值
	static int iClosestFaceNo = -1;		// 初始为不存在的面
	static D3DXVECTOR3 PtClosestBaryCentric(0, 0, 0);
	static float fClosestLength = 999999.0f;	// 初始长度为最大
	fClosestLength = 999999.0f;
	iClosestFaceNo = -1;

//	for(i = 0; i < m_pVertexFaceCountLowerSphere[m_iCurrentVertexNo]; i++)
//	{
		// 先得到在当前点上半球内的面序号
//		iFaceNo = m_ppVertexFaceInfoLowerSphere[m_iCurrentVertexNo][i];
	// Test
	for(iFaceNo = 0; iFaceNo < m_iFaceNum; iFaceNo++)
	{

		// 为了提速，自己在函数中完成功能就好了
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

		// 求交，注意共面的情况
		bIntersect = D3DXIntersectTri(&PtFaces[0], &PtFaces[1], &PtFaces[2], &PtStart, &VecRay, &PtBaryCentric.y, &PtBaryCentric.z, &fLength);
		if(bIntersect && fLength < 0.000001f)	// 点在面上，注意这里跟上面判断遮挡不同，这里是要判断其他点相对该点点光照贡献度，如果点在面上，这个贡献度就变成自身对自身的贡献，就没意义了，所以要跳过
		{
			//bIntersect = FALSE;
			continue;
		}

		PtBaryCentric.x = 1 - PtBaryCentric.y - PtBaryCentric.z;
		if(PtBaryCentric.x < 0 || PtBaryCentric.x > 1 || PtBaryCentric.y < 0 || PtBaryCentric.y > 1 || PtBaryCentric.z < 0 || PtBaryCentric.z > 1)
			continue;
		// 有一个相交，说明有遮挡，如果比当前的最小长度小，记下长度、重心坐标和面序号
		if(bIntersect && fLength < fClosestLength)
		{
			PtClosestBaryCentric = PtBaryCentric;
			fClosestLength = fLength;
			iClosestFaceNo = iFaceNo;
		}
	}

	// 没有相交的，那说明没有遮挡，可见
	if(iClosestFaceNo == -1)
		return FALSE;

	// 结束，填充数据
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

	// 先得到模型属性
	if(!m_iVertexNum)
		return E_FAIL;

	// 下来对每个物体的每个顶点进行PRT采样
	UINT i = 0, j = 0;
	D3DXVECTOR3 Vec3Sample(0.0f, 0.0f, 0.0f), VecNormal(0.0f, 0.0f, 0.0f), PtPosition(0.0f, 0.0f, 0.0f);
	D3DXVECTOR2 Vec2Sample(0.0f, 0.0f);
	float fLambertian = 0.0f, *pY = NULL, *pCoef = new float[m_iBandNum * m_iBandNum];
	BOOL iVisible = 1;

	if(!pCoef)
		return E_OUTOFMEMORY;

	char szInfo[100];	// Debug输出当前正在完成的顶点序号

	// for each vertex
	for(UINT iVertexNo = 0; iVertexNo < m_iVertexNum; iVertexNo++)
	{
		m_iCurrentVertexNo = iVertexNo;
		sprintf(szInfo, "Direct Lighting %.2f%% ： %d / %d\n", (float)iVertexNo/(float)m_iVertexNum*100.0f, iVertexNo, m_iVertexNum);
		OutputDebugString(szInfo);

		// 临时缓冲系数清零，由于我们只是计算Direct的辐照度，只包括是否可见和cos term，所以不牵扯颜色通道，Albedo到最后再乘
		for(i = 0; i < m_iBandNum * m_iBandNum; i++)
			pCoef[i] = 0.0f;

		PtPosition = m_pNewVB[iVertexNo * 2];
		VecNormal = m_pNewVB[iVertexNo * 2 + 1];

		// Monte Carlo Ray Tracing
		for(i = 0; i < m_iSampleNum; i++)
		{
			// 首先确定一条采样射线
			if(FAILED(m_MCSamples.GetSample(i, &Vec3Sample)))
				return E_FAIL;

			// Lambertian Term: Max(Dot, 0)，也就是说必须在上半球，否则跳过
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

	// 由于Direct Lighting不带颜色信息，只是求Shadowed和Cos Term，所以直接复制颜色通道的Buffer即可
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

	// 先得到模型属性
	if(!m_iVertexNum)
		return E_FAIL;

	// 下来对每个物体的每个顶点进行PRT采样
	UINT i = 0, j = 0, k = 0;
	D3DXVECTOR3 Vec3Sample(0.0f, 0.0f, 0.0f), VecNormal(0.0f, 0.0f, 0.0f), PtPosition(0.0f, 0.0f, 0.0f);
	D3DXVECTOR2 Vec2Sample(0.0f, 0.0f);
	float fLambertian = 0.0f, fAlbedo = 1.0f;
	float *pCoef[3] = {NULL, NULL, NULL};		// 临时使用的系数，分别表示RGB，因为反弹要牵扯到辐照度转换为辐射度，所以要乘反照率，三个通道必须分开
	for(k = 0; k < 3; k++)
	{
		pCoef[k] = new float[m_iBandNum * m_iBandNum];
		if(!pCoef[k])
			return E_OUTOFMEMORY;
	}

	float *pTFVector1 = new float[m_iBandNum * m_iBandNum], *pTFVector2 = new float[m_iBandNum * m_iBandNum], *pTFVector3 = new float[m_iBandNum * m_iBandNum], *pTFVector = new float[m_iBandNum * m_iBandNum];
	if(!pTFVector || !pTFVector1 || !pTFVector2 || !pTFVector3)
		return E_OUTOFMEMORY;

	// 求交信息
	BOOL bIntersect = FALSE;
	UINT iIntersectFaceNo = 0;
	D3DXVECTOR3 PtBaryCentric(0, 0, 0);
	float fLength = 0.0f;

	// Debug输出进度信息
	static UINT s_iBounceNo = 0;
	char szInfo[100];	// 当前正在完成的顶点序号
	s_iBounceNo++;	// 每调用一次就递增

	// for each vertex
	for(UINT iVertexNo = 0; iVertexNo < m_iVertexNum; iVertexNo++)
	{
		m_iCurrentVertexNo = iVertexNo;
		sprintf(szInfo, "Bounce %d Lighting %.2f%% ： %d / %d\n", s_iBounceNo, (float)iVertexNo/(float)m_iVertexNum*100.0f, iVertexNo, m_iVertexNum);
		OutputDebugString(szInfo);

		// 临时系数缓冲清零
		for(k = 0; k < 3; k++)
			for(i = 0; i < m_iBandNum * m_iBandNum; i++)
				pCoef[k][i] = 0.0f;

		PtPosition = m_pNewVB[iVertexNo * 2];
		VecNormal = m_pNewVB[iVertexNo * 2 + 1];

		// Monte Carlo Ray Tracing
		for(i = 0; i < m_iSampleNum; i++)
		{
			// 首先确定一条采样射线
			if(FAILED(m_MCSamples.GetSample(i, &Vec3Sample)))
				return E_FAIL;

			// Lambertian Term: Max(Dot, 0)，也就是说必须在上半球，否则跳过
			fLambertian = D3DXVec3Dot(&VecNormal, &Vec3Sample);
			if(fLambertian < 0.00001f)
				continue;

			// Get Closest Intersect Face
#ifdef MYPRT_USED3DXPRTINTERSECT
			bIntersect = m_pD3DXPRTEngine->ClosestRayIntersects(&PtPosition, &Vec3Sample, (DWORD *)&iIntersectFaceNo, &PtBaryCentric.y, &PtBaryCentric.z, &fLength);
			PtBaryCentric.x = 1 - PtBaryCentric.y - PtBaryCentric.z;
			// 非法，表示不相交
			if(PtBaryCentric.x < 0 || PtBaryCentric.x > 1 || PtBaryCentric.y < 0 || PtBaryCentric.y > 1 || PtBaryCentric.z < 0 || PtBaryCentric.z > 1)
				bIntersect = FALSE;
#else
			bIntersect = GetClosestIntersectPointUpperSphere(PtPosition, Vec3Sample, &iIntersectFaceNo, &PtBaryCentric, &fLength);
#endif

			// 没相交说明没有点给它贡献辐照度，跳过
			if(!bIntersect || fLength < 0.000001f)
				continue;

			// Get Vertex TF Vector, Red Only，全Channel的在后面分别复制出来乘不同的Albedo即可
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

				// 得到Albedo，这里的Albedo也可以乘顶点自身的Albedo（Diffuse）
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

	// 先得到模型属性
	if(!m_iVertexNum)
		return E_FAIL;

	// 下来对每个物体的每个顶点进行PRT采样
	UINT i = 0, j = 0, k = 0;
	D3DXVECTOR3 VecRay(0.0f, 0.0f, 0.0f), VecNormal(0.0f, 0.0f, 0.0f), PtPosition(0.0f, 0.0f, 0.0f);
	float fRd = 1.0f, fFt = 0.0f, fr2 = 0.0f;
	float *pCoef[3] = {NULL, NULL, NULL};		// 临时使用的系数，分别表示RGB
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

	// Debug输出进度信息
	static UINT s_iScatterNo = 0;
	char szInfo[100];	// 当前正在完成的顶点序号


	float fSr = 0.0f, fSv = 0.0f, fZr[3], fZv[3], fSqrtExtinct[3], fFdr = 0.0f, fExtinct = 0.0f, fRefrIndex = m_Options.fRelativeIndexofRefraction;
	// 计算Rd提速的东西，Zr/v, fSqrtExtinct，由于这些只是和介质属性相关，所以放到最前面计算一次即可
	fFdr = -1.44f / (fRefrIndex*fRefrIndex) + 0.71f / fRefrIndex + 0.668f + 0.0636f * fRefrIndex;
	for(k = 0; k < 3; k++)
	{
		fExtinct = m_Options.fScatteringCoef[k] + m_Options.fAbsorbation[k];	// 消光系数
		
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
		sprintf(szInfo, "Scatter %d Lighting %.2f%% ： %d / %d\n", s_iScatterNo, (float)iVertexNo/(float)m_iVertexNum*100.0f, iVertexNo, m_iVertexNum);
		OutputDebugString(szInfo);


		PtPosition = m_pNewVB[iVertexNo * 2];
		VecNormal = m_pNewVB[iVertexNo * 2 + 1];

		// 临时系数缓冲清零
		for(k = MYPRT_REDCHANNEL; k <= MYPRT_BLUECHANNEL; k++)
			for(j = 0; j < m_iBandNum * m_iBandNum; j++)
				pCoef[k][j] = 0.0f;

		// Test
		iCurrentVertexNum = 0;

		// Monte Carlo Intergration，遍历所有顶点求SSS贡献度
		for(i = 0; i < m_iVertexNum; i++)
		{
			if(m_iCurrentVertexNo == i)
				continue;

			// 求两点距离r，相同顶点的话就跳过
			VecRay = m_pNewVB[i * 2] - PtPosition;
			fr2 = VecRay.x * VecRay.x + VecRay.y * VecRay.y + VecRay.z * VecRay.z;
			if(fr2 < 0.000001f)
				continue;
			// 根据设置来缩放距离因子，因为这里的距离是平方，所以缩放因子也要平方
			fr2 *= m_Options.fLengthScale * m_Options.fLengthScale;

			// Test Lower Sphere Intergration
//			D3DXVec3Normalize(&VecRay, &VecRay);
//			fLambertian = D3DXVec3Dot(&VecNormal, &VecRay);
//			if(fLambertian > -0.0000001f)
//				continue;

			// Get Vertex TF Vector, Red Only，全Channel的在后面分别复制出来乘不同的Albedo即可
			for(k = MYPRT_REDCHANNEL; k <= MYPRT_BLUECHANNEL; k++)
			{
				// 得到光源点的SH向量
				if(FAILED(pInBuffer->GetVertexPRTInfo(i, pTFVector, k)))
					return E_FAIL;


				// 计算Sr/v
				fSr = sqrtf(fr2 + fZr[k] * fZr[k]);
				fSv = sqrtf(fr2 + fZv[k] * fZv[k]);

				// 得到Rd
				fRd = GetBSSRDF_Rd(fSv, fSr, fZv, fZr, fSqrtExtinct, k);

				// 用TF * Ft * Rd得到散射TF
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

	s_iScatterNo++;	// 每调用一次就递增

	return S_OK;
}



























HRESULT KPRTEngine::ComputeLDPRT(LPPRTBUFFER pBuffer)
{
	if(m_bSimulated || !m_bInit || !m_Options.iLDPRTLobeNum)
		return D3DERR_NOTAVAILABLE;

	if(!m_iVertexNum)
		return E_FAIL;

	// 下来对每个物体的每个顶点进行PRT采样
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

	char szInfo[100];	// Debug输出当前正在完成的顶点序号

	// for each vertex
	for(UINT iVertexNo = 0; iVertexNo < m_iVertexNum; iVertexNo++)
	{
		m_iCurrentVertexNo = iVertexNo;
		sprintf(szInfo, "LDPRT %.2f%% ： %d / %d\n", (float)iVertexNo/(float)m_iVertexNum*100.0f, iVertexNo, m_iVertexNum);
		OutputDebugString(szInfo);

		// 再得到面法线（球面坐标）
		CartesianToSpherical(m_pNewVB[iVertexNo*2+1], &Vec2Normal);

		// Y
		if(FAILED(SHFunction.GenerateSH(Vec2Normal)))
			return E_FAIL;
		pY = SHFunction.GetSHTable();
		if(!pY)
			return E_FAIL;

		// 循环颜色通道
		for(iColorIndex = MYPRT_REDCHANNEL; iColorIndex <= MYPRT_BLUECHANNEL; iColorIndex++)
		{
			// 先得到PRT数据
			if(FAILED(pBuffer->GetVertexPRTInfo(iVertexNo, pPRTCoef, iColorIndex)))
				return E_FAIL;

			// 先求Single Lobe Fitting
			iLobeNo = 0;	// 当前Lobe为0
			for(iIndex = 0, i = 0; i < m_iBandNum; i++)
			{
				fSum = 0.0f;
				for(j = 0; j < 2*i+1; j++, iIndex++)
					fSum += pPRTCoef[iIndex] * pY[iIndex];
				pLDPRTCoef[i] = fSum * 4*D3DX_PI / (float)(2*i+1);
			}
			
			// 循环求Multi Lobe Fitting，其实我们用顶点法线做固定轴，Multi Lobe已经没有意义了，这里暂时不支持
			for(iLobeNo = 1; iLobeNo < iLobeNum; iLobeNo++)
			{
				// 先得到上一次计算的合成向量
				for(iIndex = 0, i = 0; i < m_iBandNum; i++)
				{
					for(j = 0; j < 2*i+1; j++, iIndex++)
						pCombineCoef[iIndex] = pLDPRTCoef[(iLobeNo-1)*m_iBandNum + i] * pY[iIndex];
				}
					
				// 再求余项向量
				for(i = 0; i < m_iBandNum*m_iBandNum; i++)
				{
					pLeavingCoef[i] = pPRTCoef[i] - pCombineCoef[i];
				}
				// 对余项向量做Single Lobe Fitting，写到临时缓冲
				for(iIndex = 0, i = 0; i < m_iBandNum; i++)
				{
					fSum = 0.0f;
					for(j = 0; j < 2*i+1; j++, iIndex++)
						fSum += pLeavingCoef[iIndex] * pY[iIndex];
					pLDPRTTempCoef[i] = fSum * 4*D3DX_PI / (float)(2*i+1);
				}

				// 写到原Buffer中
				for(i = 0; i < m_iBandNum; i++)
				{
					pLDPRTCoef[iLobeNo*m_iBandNum + i] += pLDPRTTempCoef[i];
				}
			}

			// Test 可以在这里重构写入原缓冲，测试数据正确性
/*			// 测试旋转
			D3DXVECTOR3 VecNormal;
			D3DXMATRIX Mat;
			D3DXMatrixRotationZ(&Mat, D3DX_PI);
			D3DXVec3TransformCoord(&VecNormal, &m_pNewVB[iVertexNo*2+1], &Mat);
			// 再得到面法线（球面坐标）
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

	// 处理only LDPRT

	return S_OK;
}
