#include "Physics.h"
#include "Texture.h"


/************************GPU Cloth Simulation*************************/
/////////�ⲿ�ӿ�
KClothSimulation::KClothSimulation()
{
	m_iCreateAttrib = 0;
	m_bGenerateNormal = m_bGenerateTangent = FALSE;
	m_iStride = 0;
	m_fDeltaTime = 0;

	m_iCollideNum = 0;
	m_pCollideData = NULL;

	m_pVBQuad = NULL;
	m_pDepthBuffer = NULL;

	m_pTexPrevP = m_pTexNowP = m_pTexNextP = 0;
	m_pTexPrevP_Sign = m_pTexNowP_Sign = m_pTexNextP_Sign = 0;

	m_pTexTemp[0] = m_pTexTemp[1] = NULL;
	m_pTexTemp_Sign[0] = m_pTexTemp_Sign[1] = NULL;
	m_pTexTempCPU[0] = m_pTexTempCPU[1] = NULL;
	for(UINT i = 0; i < 4; i++)
		m_pTexSpringPairRectangle[i] = m_pTexSpringPairShear[i] = NULL;
	
	m_pTexFixedPoint = NULL;

	m_pTexNormal = m_pTexTangent = NULL;
}

void KClothSimulation::Release()
{
	m_iCreateAttrib = 0;
	m_bGenerateNormal = m_bGenerateTangent = FALSE;
	m_iStride = 0;
	m_fDeltaTime = 0;

	m_iCollideNum = 0;
	SAFE_DELETE_ARRAY(m_pCollideData);

	m_VS.Release();
	SAFE_RELEASE(m_pVBQuad);
	SAFE_RELEASE(m_pDepthBuffer);

	SAFE_RELEASE(m_pTexPrevP);
	SAFE_RELEASE(m_pTexNowP);
	SAFE_RELEASE(m_pTexNextP);

	SAFE_RELEASE(m_pTexPrevP_Sign);
	SAFE_RELEASE(m_pTexNowP_Sign);
	SAFE_RELEASE(m_pTexNextP_Sign);

	SAFE_RELEASE(m_pTexTemp[0]);
	SAFE_RELEASE(m_pTexTemp[1]);
	SAFE_RELEASE(m_pTexTemp_Sign[0]);
	SAFE_RELEASE(m_pTexTemp_Sign[1]);

	SAFE_RELEASE(m_pTexTempCPU[0]);
	SAFE_RELEASE(m_pTexTempCPU[1]);

	for(UINT i = 0; i < 4; i++)
	{
		SAFE_RELEASE(m_pTexSpringPairShear[i]);
		SAFE_RELEASE(m_pTexSpringPairRectangle[i]);
	}

	SAFE_RELEASE(m_pTexFixedPoint);

	SAFE_RELEASE(m_pTexTangent);
	SAFE_RELEASE(m_pTexNormal);

	// PS
	m_PSSetToZero.Release();
	m_PSCopyTexture.Release();
	m_PSGenerateNormal.Release();

	m_PSGenerateNormalandTangent.Release();
	m_PSApplyForce.Release();
	m_PSApplySpring.Release();
	m_PSCollide_Plane.Release();
	m_PSCollide_Sphere.Release();
	m_PSCollide_Box.Release();
	m_PSCollide_Ellipse.Release();
	m_PSInitPosition.Release();
}




HRESULT KClothSimulation::ClothSimulation( CLOTH_ATTRIBUTE ClothData, float fDeltaTime, BOOL bGenerateNormal/* = TRUE*/, BOOL bGenerateTangent/* = FALSE*/ ) 
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// ��鲼�ϲ���������Ҫ�������ߣ�������㷨��
	if(!ClothData.CheckAvailable())
		return D3DERR_INVALIDCALL;
	if(ClothData.iWidth != m_ClothData.iWidth || ClothData.iHeight != m_ClothData.iHeight)
		return D3DERR_INVALIDCALL;
	if(bGenerateTangent && !bGenerateNormal)
		return D3DERR_INVALIDCALL;


	m_ClothData = ClothData;
	m_bGenerateNormal = bGenerateNormal;
	m_bGenerateTangent = bGenerateTangent;



	UINT i = 0;
	m_fDeltaTime = fDeltaTime;



	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա�FFT��Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// Test
	ResetPosition(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(1, 0, 0));
	// ��ʼģ��
		// ������������ǰNextP�����룬ͨ����������ָ���������µ�NextP
//	V_RETURN(ApplyForce());
		// ���ǵ������㵯��������ײ�ÿһ��������NextP���룬�����������NextP
//	for(i = 0; i < m_ClothData.iIterateNum; i++)
	{
//		V_RETURN(ApplySpring());
		V_RETURN(ApplyCollision());
	}
/*	
	// �������ɷ���ͼ������ͼ
	if(m_bGenerateNormal && m_bGenerateTangent)
	{
		V_RETURN(CommonComputeQuad(m_pTexNextP, m_pTexNextP_Sign, NULL, NULL, m_pTexNormal, NULL, &m_PSGenerateNormal));
	}
	else if(m_bGenerateNormal)
	{
		V_RETURN(CommonComputeQuad(m_pTexNextP, m_pTexNextP_Sign, NULL, NULL, m_pTexNormal, m_pTexTangent, &m_PSGenerateNormalandTangent));
	}
*/

	// �ָ�RT����Ȼ���
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);


	// Test
	HRESULT hr = S_OK;
//	hr = D3DXSaveTextureToFile("c:\\box.hdr", D3DXIFF_HDR, m_pTexNextP, NULL);
//	hr = D3DXSaveTextureToFile("c:\\box_Sign.hdr", D3DXIFF_HDR, m_pTexNextP_Sign, NULL);
	hr = hr;

	// ��ɣ�����
	m_iCreateAttrib = 2;
	return S_OK;
}






/****************************ģ���ڲ��ӿ�*****************************/
HRESULT KClothSimulation::ApplyForce()
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// �Ƚ���ָ�룬��һ֡��NowP���PrevP����һ֡����õ�NewP���ڱ��NowP���������룩����һ֡��Prev����������µļ���Ŀ�ĵأ�����֡��NewP
	LPDIRECT3DTEXTURE9 pTexTemp = NULL, pTexTemp_Sign = NULL;
	
	pTexTemp = m_pTexPrevP;
	m_pTexPrevP = m_pTexNowP;
	m_pTexNowP = m_pTexNextP;
	m_pTexNextP = pTexTemp;
	
	pTexTemp_Sign = m_pTexPrevP_Sign;
	m_pTexPrevP_Sign = m_pTexNowP_Sign;
	m_pTexNowP_Sign = m_pTexNextP_Sign;
	m_pTexNextP_Sign = pTexTemp_Sign;


	// ��������������+���������ٶ�
	D3DXVECTOR3 VecForce = m_ClothData.VecWind / m_ClothData.fm + m_ClothData.fGravity * D3DXVECTOR3(0, -1, 0);
	d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(VecForce.x, VecForce.y, VecForce.z, 0), 1);
	d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(m_ClothData.fK, m_ClothData.fK, m_ClothData.fK, m_ClothData.fK), 1);
	V_RETURN(CommonComputeQuad(m_pTexNowP, m_pTexNowP_Sign, m_pTexPrevP, m_pTexPrevP_Sign, m_pTexNextP, m_pTexNextP_Sign, &m_PSApplyForce));

	return S_OK;
}





HRESULT KClothSimulation::ApplySpring()
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// Spring��Ҫ����4�Σ��м���Temp���ؽ�������
	UINT iTempIndex = 0, i = 0;
	// 4�μ���ֱ�ı�׼���ɾ��룬��סҪ����
	float fD[4];
	LPDIRECT3DTEXTURE9 *pTexSpringPair = NULL;
	if(m_ClothData.SpringType == SPRING_RECTANGLE)
	{
		pTexSpringPair = m_pTexSpringPairRectangle;
		// ����ǰ�����Square�����������꣬����������һ����������ķ�ĸҲ�����ټ�1��
		fD[0] = fD[1] = m_ClothData.fSquareWidth / (float)m_ClothData.iWidth;
		fD[2] = fD[3] = m_ClothData.fSquareHeight / (float)m_ClothData.iHeight;
	}
	else if(m_ClothData.SpringType == SPRING_RECTANGLE)
	{
		pTexSpringPair = m_pTexSpringPairShear;
		D3DXVECTOR2 VecXY = D3DXVECTOR2(m_ClothData.fSquareWidth / (float)m_ClothData.iWidth, m_ClothData.fSquareHeight / (float)m_ClothData.iHeight);
		fD[0] = fD[1] = fD[2] = fD[3] = D3DXVec2Length(&VecXY);
	}
	else
		return D3DERR_INVALIDCALL;

// Test	for(i = 0; i < 4; i++)
//		fD[i] = 1.0f / fD[i];

	

	for(i = 0; i < 4; i++)
	{
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(fD[i], fD[i], fD[i], fD[i]), 1);

		// ��һ�Σ���NextP���㵽Temp��
		if(i == 0)
		{
			V_RETURN(CommonComputeQuad(m_pTexNextP, m_pTexNextP_Sign, pTexSpringPair[i], NULL, m_pTexTemp[iTempIndex], m_pTexTemp_Sign[iTempIndex], &m_PSApplySpring));
		}
		// ���һ�Σ������л������㵽P��
		else if(i == 3)
		{
			V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], m_pTexTemp_Sign[iTempIndex], pTexSpringPair[i], NULL, m_pTexNextP, m_pTexNextP_Sign, &m_PSApplySpring));
			//V_RETURN(CopyTexture(m_pTexTemp[iTempIndex], m_pTexNextP));
			//V_RETURN(CopyTexture(m_pTexTemp_Sign[iTempIndex], m_pTexNextP_Sign));
		}
		// �м���̣���Temp�������л�
		else
		{
			V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], m_pTexTemp_Sign[iTempIndex], pTexSpringPair[i], NULL, m_pTexTemp[!iTempIndex], m_pTexTemp_Sign[!iTempIndex], &m_PSApplySpring));
			//V_RETURN(CopyTexture(m_pTexTemp[!iTempIndex], m_pTexTemp[iTempIndex]));
			//V_RETURN(CopyTexture(m_pTexTemp_Sign[!iTempIndex], m_pTexTemp_Sign[iTempIndex]));
			iTempIndex = !iTempIndex;
		}	
	}

	return S_OK;
}





PIXELSHADER * KClothSimulation::SelectCollideShader( COLLIDE_TYPE Type ) 
{
	if(Type == COLLIDE_PLANE)
		return &m_PSCollide_Plane;
	if(Type == COLLIDE_SPHERE)
		return &m_PSCollide_Sphere;
	if(Type == COLLIDE_BOX)
		return &m_PSCollide_Box;
	if(Type == COLLIDE_ELLIPSE)
		return &m_PSCollide_Ellipse;
	return NULL;
}

HRESULT KClothSimulation::SetCollideConstant( UINT iCollideNo ) 
{
	if(!m_pCollideData)
		return D3DERR_NOTAVAILABLE;
	if(iCollideNo >= m_iCollideNum)
		return D3DERR_INVALIDCALL;

	switch(m_pCollideData[iCollideNo].Type)
	{
	case COLLIDE_PLANE:
		// ƽ���У�Axis[0]��ʾ����
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].VecAxis[0].x, m_pCollideData[iCollideNo].VecAxis[0].y, m_pCollideData[iCollideNo].VecAxis[0].z, 0), 1);
		d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].fd, m_pCollideData[iCollideNo].fd, m_pCollideData[iCollideNo].fd, m_pCollideData[iCollideNo].fd), 1);
		break;
	case COLLIDE_SPHERE:
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].PtCentre.x, m_pCollideData[iCollideNo].PtCentre.y, m_pCollideData[iCollideNo].PtCentre.z, 0), 1);
		d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].fRadius, m_pCollideData[iCollideNo].fRadius, m_pCollideData[iCollideNo].fRadius, m_pCollideData[iCollideNo].fRadius), 1);
		break;
	case COLLIDE_BOX:
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].VecAxis[0].x, m_pCollideData[iCollideNo].VecAxis[0].y, m_pCollideData[iCollideNo].VecAxis[0].z, 0), 1);
		d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].VecAxis[1].x, m_pCollideData[iCollideNo].VecAxis[1].y, m_pCollideData[iCollideNo].VecAxis[1].z, 0), 1);
		d3ddevice->SetPixelShaderConstantF(7, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].VecAxis[2].x, m_pCollideData[iCollideNo].VecAxis[2].y, m_pCollideData[iCollideNo].VecAxis[2].z, 0), 1);
		d3ddevice->SetPixelShaderConstantF(8, (float *)&D3DXVECTOR4(1.0f / m_pCollideData[iCollideNo].fAxisLength[0], 1.0f / m_pCollideData[iCollideNo].fAxisLength[1], 1.0f / m_pCollideData[iCollideNo].fAxisLength[2], 0), 1);
		d3ddevice->SetPixelShaderConstantF(9, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].PtCentre.x, m_pCollideData[iCollideNo].PtCentre.y, m_pCollideData[iCollideNo].PtCentre.z, 0), 1);
		break;
	case COLLIDE_ELLIPSE:
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].VecAxis[0].x, m_pCollideData[iCollideNo].VecAxis[0].y, m_pCollideData[iCollideNo].VecAxis[0].z, 0), 1);
		d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].VecAxis[1].x, m_pCollideData[iCollideNo].VecAxis[1].y, m_pCollideData[iCollideNo].VecAxis[1].z, 0), 1);
		d3ddevice->SetPixelShaderConstantF(7, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].VecAxis[2].x, m_pCollideData[iCollideNo].VecAxis[2].y, m_pCollideData[iCollideNo].VecAxis[2].z, 0), 1);
		// �������������ת�þ���
		d3ddevice->SetPixelShaderConstantF(8, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].VecAxis[0].x, m_pCollideData[iCollideNo].VecAxis[1].x, m_pCollideData[iCollideNo].VecAxis[2].x, 0), 1);
		d3ddevice->SetPixelShaderConstantF(9, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].VecAxis[0].y, m_pCollideData[iCollideNo].VecAxis[1].y, m_pCollideData[iCollideNo].VecAxis[2].y, 0), 1);
		d3ddevice->SetPixelShaderConstantF(10, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].VecAxis[0].z, m_pCollideData[iCollideNo].VecAxis[1].z, m_pCollideData[iCollideNo].VecAxis[2].z, 0), 1);
		// ��������
		d3ddevice->SetPixelShaderConstantF(11, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].fAxisLength[0], m_pCollideData[iCollideNo].fAxisLength[1], m_pCollideData[iCollideNo].fAxisLength[2], 0), 1);
		d3ddevice->SetPixelShaderConstantF(12, (float *)&D3DXVECTOR4(m_pCollideData[iCollideNo].PtCentre.x, m_pCollideData[iCollideNo].PtCentre.y, m_pCollideData[iCollideNo].PtCentre.z, 0), 1);
		break;
	}
	return S_OK;
}



HRESULT KClothSimulation::ApplyCollision()
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	UINT iTempIndex = 0, i = 0;

	// NextP��Ϊ���룬ͬ����Ϊ������м���Temp���ؽ�������

	// ���ֻ��һ���ϰ����ô�м���ҪCopyһ��
	if(m_iCollideNum == 1)
	{
		V_RETURN(CopyTexture(m_pTexTemp[iTempIndex], m_pTexNextP));
		V_RETURN(CopyTexture(m_pTexTemp_Sign[iTempIndex], m_pTexNextP_Sign));

		V_RETURN(SetCollideConstant(0));
		V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], m_pTexTemp_Sign[iTempIndex], NULL, NULL, m_pTexNextP, m_pTexNextP_Sign, SelectCollideShader(m_pCollideData[0].Type) ));
	}
	else
	{
		for(i = 0; i < m_iCollideNum; i++)
		{
			// ��һ��
			if(i == 0)
			{
				V_RETURN(SetCollideConstant(i));
				V_RETURN(CommonComputeQuad(m_pTexNextP, m_pTexNextP_Sign, NULL, NULL, m_pTexTemp[iTempIndex], m_pTexTemp_Sign[iTempIndex], SelectCollideShader(m_pCollideData[i].Type) ));
			}
			// ���һ�Σ������л������㵽P��
			else if( i == (m_iCollideNum - 1) )
			{
				V_RETURN(SetCollideConstant(i));
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], m_pTexTemp_Sign[iTempIndex], NULL, NULL, m_pTexNextP, m_pTexNextP_Sign, SelectCollideShader(m_pCollideData[i].Type) ));
			}
			// �м���̣���Temp�������л�
			else
			{
				V_RETURN(SetCollideConstant(i));
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], m_pTexTemp_Sign[iTempIndex], NULL, NULL, m_pTexTemp[!iTempIndex], m_pTexTemp_Sign[!iTempIndex], SelectCollideShader(m_pCollideData[i].Type) ));
				iTempIndex = !iTempIndex;
			}	
		}
	}

	return S_OK;
}





/***************************Set����ⲿ�ӿ�****************************/
HRESULT KClothSimulation::SetFixedPosition(UINT iPointNum, UINT *pX, UINT *pY, LPD3DXVECTOR3 pPosition)
{
	if(!m_iCreateAttrib || !m_pTexFixedPoint || !m_pTexNextP || !m_pTexNextP_Sign)
		return D3DERR_NOTAVAILABLE;
	if(!iPointNum || !pX || !pY)
		return D3DERR_INVALIDCALL;

	D3DLOCKED_RECT RectFixPoint, RectPos, RectPos_Sign;

	V_RETURN(m_pTexFixedPoint->LockRect(0, &RectFixPoint, NULL, 0));

	// Lockλ����ͼʱҪ��ת�浽TempCPU�У���Ȼ�ˣ�ֻ��pPosition��Ϊ��ʱ�Ż����Position��ͼ��ֵ
	if(pPosition)
	{
		V_RETURN(D3DXLoadTextureFromTexture(m_pTexTempCPU[0], m_pTexNextP));
		V_RETURN(D3DXLoadTextureFromTexture(m_pTexTempCPU[1], m_pTexNextP_Sign));

		V_RETURN(m_pTexTempCPU[0]->LockRect(0, &RectPos, NULL, 0));
		V_RETURN(m_pTexTempCPU[1]->LockRect(0, &RectPos_Sign, NULL, 0));
	}


	BYTE *pFixPoint = NULL;
	float16 *pPos16 = NULL, *pPos_Sign16 = NULL;
	float *pPos = NULL, *pPos_Sign = NULL;

	for(UINT i = 0; i < iPointNum; i++)
	{
		// �ȶ�λ��ע��������ͼ��ÿ��������ռ�ֽ����Ĳ�ͬ��4ͨ����FP16��ռ8�ֽڣ���FP32��ռ16�ֽ�
		pFixPoint = (BYTE *)RectFixPoint.pBits + pY[i] * RectFixPoint.Pitch + pX[i];

		if(pPosition)
		{
			pPos16 = (float16 *)((BYTE *)RectPos.pBits + pY[i] * RectPos.Pitch + pX[i]*8);
			pPos_Sign16 = (float16 *)((BYTE *)RectPos_Sign.pBits + pY[i] * RectPos_Sign.Pitch + pX[i]*8);
			pPos = (float *)((BYTE *)RectPos.pBits + pY[i] * RectPos.Pitch + pX[i]*16);
			pPos_Sign = (float *)((BYTE *)RectPos_Sign.pBits + pY[i] * RectPos_Sign.Pitch + pX[i]*16);
		}

		// д�룬�̶����FixֵΪ0
		*pFixPoint++ = 0;

		if(pPosition)
		{
#ifdef USE_FP16
			*pPos16++ = floatToFP16(pPosition[i].x);
			*pPos16++ = floatToFP16(pPosition[i].y);
			*pPos16++ = floatToFP16(pPosition[i].z);

			*pPos_Sign16++ = floatToFP16(pPosition[i].x > 0.0f ? 2.0f : 0.0f);
			*pPos_Sign16++ = floatToFP16(pPosition[i].y > 0.0f ? 2.0f : 0.0f);
			*pPos_Sign16++ = floatToFP16(pPosition[i].z > 0.0f ? 2.0f : 0.0f);
#elif defined USE_FP32
			*pPos++ = pPosition[i].x;
			*pPos++ = pPosition[i].y;
			*pPos++ = pPosition[i].z;

			*pPos_Sign++ = pPosition[i].x > 0.0f ? 2.0f : 0.0f;
			*pPos_Sign++ = pPosition[i].y > 0.0f ? 2.0f : 0.0f;
			*pPos_Sign++ = pPosition[i].z > 0.0f ? 2.0f : 0.0f;
#endif
		}
	}


	V_RETURN(m_pTexFixedPoint->UnlockRect(0));

	if(pPosition)
	{
		V_RETURN(m_pTexTempCPU[0]->UnlockRect(0));
		V_RETURN(m_pTexTempCPU[1]->UnlockRect(0));

		// ������ˢ��λ����ͼ��RT����
		V_RETURN(D3DXLoadTextureFromTexture(m_pTexNextP, m_pTexTempCPU[0]));
		V_RETURN(D3DXLoadTextureFromTexture(m_pTexNextP_Sign, m_pTexTempCPU[1]));
	}

	return S_OK;
}





HRESULT KClothSimulation::FreeFixedPosition(UINT iPointNum, UINT *pX, UINT *pY)
{
	if(!m_iCreateAttrib || !m_pTexFixedPoint)
		return D3DERR_NOTAVAILABLE;
	if(!iPointNum || !pX || !pY)
		return D3DERR_INVALIDCALL;

	D3DLOCKED_RECT RectFixPoint;
	LPDIRECT3DSURFACE9 pSurfFixPoint = NULL;

	V_RETURN(m_pTexFixedPoint->GetSurfaceLevel(0, &pSurfFixPoint));
	V_RETURN(pSurfFixPoint->LockRect(&RectFixPoint, NULL, 0));

	BYTE *pFixPoint = NULL;
	for(UINT i = 0; i < iPointNum; i++)
	{
		// ��λ
		pFixPoint = (BYTE *)RectFixPoint.pBits + pY[i] * RectFixPoint.Pitch + pX[i];
		// д�룬���ɵ��FixֵΪ255
		*pFixPoint++ = 255;
	}

	V_RETURN(pSurfFixPoint->UnlockRect());
	SAFE_RELEASE(pSurfFixPoint);

	return S_OK;
}






HRESULT KClothSimulation::SetCollides(UINT iCollideNum, COLLIDE_ATTRIBUTE *pCollideData)
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;
	if(!iCollideNum || !pCollideData)
		return D3DERR_INVALIDCALL;

	// ��Ч�Լ��
	for(UINT i = 0; i < iCollideNum; i++)
	{
		if(!pCollideData[i].CheckAvailable())
			return D3DERR_INVALIDCALL;
	}

	// �ͷ�ԭ���ģ������µ�
	SAFE_DELETE_ARRAY(m_pCollideData);
	m_pCollideData = new COLLIDE_ATTRIBUTE[iCollideNum];
	if(!m_pCollideData)
		return E_OUTOFMEMORY;

	// ��������
	m_iCollideNum = iCollideNum;
	memcpy(m_pCollideData, pCollideData, m_iCollideNum * sizeof(COLLIDE_ATTRIBUTE));
	return S_OK;
}









/****************************�����ڲ��ӿ�*****************************/
HRESULT KClothSimulation::ClearAllFixPoint()
{
	if(!m_pTexFixedPoint)
		return D3DERR_NOTAVAILABLE;

	D3DLOCKED_RECT Rect;
	LPDIRECT3DSURFACE9 pSurf = NULL;
	V_RETURN(m_pTexFixedPoint->GetSurfaceLevel(0, &pSurf));
	V_RETURN(pSurf->LockRect(&Rect, NULL, 0));

	BYTE *p = NULL;
	for(UINT iY = 0; iY < m_ClothData.iHeight; iY++)
	{
		p = (BYTE *)Rect.pBits + iY * Rect.Pitch;

		for(UINT iX = 0; iX < m_ClothData.iWidth; iX++)
		{
			// FixPointͼֻ��һͨ��alpha��������������ͼ��1.0f��ʾ����
			*p++ = 255;
		}
	}

	V_RETURN(pSurf->UnlockRect());
	SAFE_RELEASE(pSurf);

	return S_OK;
}



HRESULT KClothSimulation::InitSpringPairTexture()
{
	for(UINT i = 0; i < 4; i++)
	{
		if(!m_pTexSpringPairRectangle[i] || !m_pTexSpringPairShear[i])
			return D3DERR_NOTAVAILABLE;
	}

	D3DLOCKED_RECT Rect;
	LPDIRECT3DSURFACE9 pSurf = NULL;
	float16 *p16 = NULL;
	float *p = NULL;
	UINT iX = 0, iY = 0;
	
	UINT iPairNo = 0;
	D3DXVECTOR2 HalfTap = D3DXVECTOR2(0.5f/(float)m_ClothData.iWidth, 0.5f/(float)m_ClothData.iHeight);
	D3DXVECTOR2 PairCoord;

	// �ȵ�һ�֣����ε��ɣ�ż���X������Ե�Yֵ�͵�ǰ����ͬ
	V_RETURN(m_pTexSpringPairRectangle[0]->GetSurfaceLevel(0, &pSurf));
	V_RETURN(pSurf->LockRect(&Rect, NULL, 0));

	for(iY = 0; iY < m_ClothData.iHeight; iY++)
	{
		p16 = (float16 *)((BYTE *)Rect.pBits + iY * Rect.Pitch);
		p = (float *)((BYTE *)Rect.pBits + iY * Rect.Pitch);

		for(iX = 0; iX < m_ClothData.iWidth; iX++)
		{
			// ������Ե����������
				// ����һ����ԣ�����ǵ�һ�������Ա�2����������Ե���ǵ�ǰ��ż�1�������ǵ�ǰ��ż�1
			iPairNo = iX % 2 ? iX-1 : iX+1;
				// �������ֱ���Ϊ��������ô���һ������ǿ��������ģ���ʱ����PairNo��Խ�磬����Ҫǿ��ʣ�µĵ��Լ����Լ����
			if(iPairNo >= m_ClothData.iWidth)
				iPairNo = m_ClothData.iWidth - 1;
			// ��ֵ
			PairCoord.x = (float)iPairNo/(float)m_ClothData.iWidth;
			PairCoord.y = (float)iY/(float)m_ClothData.iHeight;
			// �Ӱ�Tapƫ��
			PairCoord += HalfTap;

			// ���������ͼ����ͨ���ĸ���GR
#ifdef USE_FP16
			*p16++ = floatToFP16(PairCoord.x);
			*p16++ = floatToFP16(PairCoord.y);
#elif defined USE_FP32
			*p++ = PairCoord.x;
			*p++ = PairCoord.y;
#endif
		}

	}

	V_RETURN(pSurf->UnlockRect());
	SAFE_RELEASE(pSurf);





	// �ȵڶ��֣����ε��ɣ������X������Ե�Yֵ�͵�ǰ����ͬ
	V_RETURN(m_pTexSpringPairRectangle[1]->GetSurfaceLevel(0, &pSurf));
	V_RETURN(pSurf->LockRect(&Rect, NULL, 0));

	for(iY = 0; iY < m_ClothData.iHeight; iY++)
	{
		p16 = (float16 *)((BYTE *)Rect.pBits + iY * Rect.Pitch);
		p = (float *)((BYTE *)Rect.pBits + iY * Rect.Pitch);

		for(iX = 0; iX < m_ClothData.iWidth; iX++)
		{
			// ������Ե����������
				// ����һ����ԣ�����ǵ�һ�������ܱ�2����������Ե���ǵ�ǰ��ż�1�������ǵ�ǰ��ż�1
				// ע���һ�������ڲ������������Ը��Աߵ���ԣ����ܺ��Լ���ԣ������ڹ�񻯵�������������ʱ������������
			if(iX == 0)
				iPairNo = 0;
			else
				iPairNo = iX % 2 ? iX+1 : iX-1;
				// �������ֱ���Ϊż������ô��������ǲ��������ģ���ʱ����PairNo��Խ�磬����Ҫǿ�����ĵ��Լ����Լ����
			if(iPairNo >= m_ClothData.iWidth)
				iPairNo = m_ClothData.iWidth - 1;
			// ��ֵ
			PairCoord.x = (float)iPairNo/(float)m_ClothData.iWidth;
			PairCoord.y = (float)iY/(float)m_ClothData.iHeight;
			// �Ӱ�Tapƫ��
			PairCoord += HalfTap;

			// ���������ͼ����ͨ���ĸ���GR
#ifdef USE_FP16
			*p16++ = floatToFP16(PairCoord.x);
			*p16++ = floatToFP16(PairCoord.y);
#elif defined USE_FP32
			*p++ = PairCoord.x;
			*p++ = PairCoord.y;
#endif
		}

	}

	V_RETURN(pSurf->UnlockRect());
	SAFE_RELEASE(pSurf);





	// �ȵ����֣����ε��ɣ�ż���Y������Ե�Xֵ�͵�ǰ����ͬ
	V_RETURN(m_pTexSpringPairRectangle[2]->GetSurfaceLevel(0, &pSurf));
	V_RETURN(pSurf->LockRect(&Rect, NULL, 0));

	for(iY = 0; iY < m_ClothData.iHeight; iY++)
	{
		p16 = (float16 *)((BYTE *)Rect.pBits + iY * Rect.Pitch);
		p = (float *)((BYTE *)Rect.pBits + iY * Rect.Pitch);

		for(iX = 0; iX < m_ClothData.iWidth; iX++)
		{
			// ������Ե����������
			// ����һ����ԣ�����ǵ�һ�������Ա�2����������Ե���ǵ�ǰ��ż�1�������ǵ�ǰ��ż�1
			iPairNo = iY % 2 ? iY-1 : iY+1;
			// �������ֱ���Ϊ��������ô���һ������ǿ��������ģ���ʱ����PairNo��Խ�磬����Ҫǿ��ʣ�µĵ��Լ����Լ����
			if(iPairNo >= m_ClothData.iHeight)
				iPairNo = m_ClothData.iHeight - 1;
			// ��ֵ
			PairCoord.y = (float)iPairNo/(float)m_ClothData.iHeight;
			PairCoord.x = (float)iX/(float)m_ClothData.iWidth;
			// �Ӱ�Tapƫ��
			PairCoord += HalfTap;

			// ���������ͼ����ͨ���ĸ���GR
#ifdef USE_FP16
			*p16++ = floatToFP16(PairCoord.x);
			*p16++ = floatToFP16(PairCoord.y);
#elif defined USE_FP32
			*p++ = PairCoord.x;
			*p++ = PairCoord.y;
#endif
		}

	}

	V_RETURN(pSurf->UnlockRect());
	SAFE_RELEASE(pSurf);





	// �ȵڶ��֣����ε��ɣ������Y������Ե�Xֵ�͵�ǰ����ͬ
	V_RETURN(m_pTexSpringPairRectangle[3]->GetSurfaceLevel(0, &pSurf));
	V_RETURN(pSurf->LockRect(&Rect, NULL, 0));

	for(iY = 0; iY < m_ClothData.iHeight; iY++)
	{
		p16 = (float16 *)((BYTE *)Rect.pBits + iY * Rect.Pitch);
		p = (float *)((BYTE *)Rect.pBits + iY * Rect.Pitch);

		for(iX = 0; iX < m_ClothData.iWidth; iX++)
		{
			// ������Ե����������
			// ����һ����ԣ�����ǵ�һ�������ܱ�2����������Ե���ǵ�ǰ��ż�1�������ǵ�ǰ��ż�1
			// ע���һ�������ڲ������������Ը��Աߵ���ԣ����ܺ��Լ���ԣ������ڹ�񻯵�������������ʱ������������
			if(iY == 0)
				iPairNo = 0;
			else
				iPairNo = iY % 2 ? iY+1 : iY-1;
			// �������ֱ���Ϊż������ô��������ǲ��������ģ���ʱ����PairNo��Խ�磬����Ҫǿ�����ĵ��Լ����Լ����
			if(iPairNo >= m_ClothData.iHeight)
				iPairNo = m_ClothData.iHeight - 1;
			// ��ֵ
			PairCoord.y = (float)iPairNo/(float)m_ClothData.iHeight;
			PairCoord.x = (float)iX/(float)m_ClothData.iWidth;
			// �Ӱ�Tapƫ��
			PairCoord += HalfTap;

			// ���������ͼ����ͨ���ĸ���GR
#ifdef USE_FP16
			*p16++ = floatToFP16(PairCoord.x);
			*p16++ = floatToFP16(PairCoord.y);
#elif defined USE_FP32
			*p++ = PairCoord.x;
			*p++ = PairCoord.y;
#endif
		}

	}

	V_RETURN(pSurf->UnlockRect());
	SAFE_RELEASE(pSurf);





	return S_OK;
}




HRESULT KClothSimulation::ResetPosition(D3DXVECTOR3 PtLTPosition, D3DXVECTOR3 VecHorizon)
{
	if(D3DXVec3Length(&VecHorizon) < 0.00001f)
		return D3DERR_INVALIDCALL;

	D3DXVec3Normalize(&VecHorizon, &VecHorizon);

	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա�FFT��Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// ���¸�λ����λ����ͼ�����﴿���ݳ��������㣬����Ҫ������Դ��ͼ��Ҳ����FixPoint
	d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(PtLTPosition.x, PtLTPosition.y, PtLTPosition.z, 0), 1);
	d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(VecHorizon.x, VecHorizon.y, VecHorizon.z, 0), 1);
	d3ddevice->SetPixelShaderConstantF(7, (float *)&D3DXVECTOR4(m_ClothData.fSquareWidth, m_ClothData.fSquareHeight, 0, 0), 1);
	V_RETURN(CommonComputeQuad(NULL, NULL, NULL, NULL, m_pTexNextP, m_pTexNextP_Sign, &m_PSInitPosition));

	V_RETURN(CopyTexture(m_pTexPrevP, m_pTexNextP));
	V_RETURN(CopyTexture(m_pTexPrevP_Sign, m_pTexNextP_Sign));

	V_RETURN(CopyTexture(m_pTexNowP, m_pTexNextP));
	V_RETURN(CopyTexture(m_pTexNowP_Sign, m_pTexNextP_Sign));

	// �ָ�RT����Ȼ���
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);

	return S_OK;
}













HRESULT KClothSimulation::CommonComputeQuad(LPDIRECT3DTEXTURE9 pSrcTex1, LPDIRECT3DTEXTURE9 pSrcTex2, LPDIRECT3DTEXTURE9 pSrcTex3, LPDIRECT3DTEXTURE9 pSrcTex4, LPDIRECT3DTEXTURE9 pRT1,LPDIRECT3DTEXTURE9 pRT2, PIXELSHADER *pPS)
{
	if(!m_pVBQuad)
		return D3DERR_NOTAVAILABLE;

	if(!pRT1 || !pPS)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	UINT i = 0;

	// ������Ⱦ��Դ��Ŀ��
	V_RETURN(SetTexturedRenderTarget(0, pRT1, NULL));
	SetTexturedRenderTarget(1, pRT2, NULL);	// �����Ҫ�ж�ʧ���ˣ�RT2�������ָ���

	V_RETURN(d3ddevice->SetTexture(0, m_pTexFixedPoint));
	V_RETURN(d3ddevice->SetTexture(1, pSrcTex1));
	V_RETURN(d3ddevice->SetTexture(2, pSrcTex2));
	V_RETURN(d3ddevice->SetTexture(3, pSrcTex3));
	V_RETURN(d3ddevice->SetTexture(4, pSrcTex4));

	// ������Ⱦ����
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	V_RETURN(d3ddevice->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	V_RETURN(d3ddevice->SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(3, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(3, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	V_RETURN(d3ddevice->SetSamplerState(4, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(4, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(4, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(4, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(4, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	// for Bilinear Filtering
	V_RETURN(m_VS.SetConstant(0, &D3DXVECTOR4((float)m_ClothData.iWidth, (float)m_ClothData.iHeight, 0, 0), 1));

	// ����Shader�������Ĵ�����1/2tap��1tap��Dimension��������DeltaTime����c0��c4��
	V_RETURN(pPS->SetPixelShader());
	V_RETURN(pPS->SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_ClothData.iWidth, 0.5f/(float)m_ClothData.iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_ClothData.iWidth, 1.0f/(float)m_ClothData.iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(2, &D3DXVECTOR4(1.0f/(float)m_ClothData.iWidth, 1.0f/(float)m_ClothData.iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(3, &D3DXVECTOR4(0.0f, 0.5f, 1.0f, 2.0f), 1));
	V_RETURN(pPS->SetConstant(4, &D3DXVECTOR4(m_fDeltaTime, m_fDeltaTime, m_fDeltaTime, m_fDeltaTime), 1));
	V_RETURN(d3ddevice->SetVertexShader(m_VS.Handle));
	V_RETURN(d3ddevice->SetFVF(0));

	// ��ʼ��Ⱦ
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VS.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVBQuad, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// �ָ������ڲ������в��趼��ʹ�õڶ���RT���������ֶ��ָ�һ��
	V_RETURN(pPS->RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(1, NULL));

	// ���
	return S_OK;
}








/****************************��ʼ��**********************************/

HRESULT KClothSimulation::Init(CLOTH_ATTRIBUTE ClothData, D3DXVECTOR3 PtLTPosition, D3DXVECTOR3 VecHorizon)
{
	if(m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// ������
	if(!ClothData.CheckAvailable())
		return D3DERR_INVALIDCALL;
	if(D3DXVec3Length(&VecHorizon) < 0.00001f)
		return D3DERR_INVALIDCALL;

	m_ClothData = ClothData;


	D3DFORMAT FormatFourChannel = D3DFMT_A8R8G8B8;
	D3DFORMAT FormatDualChannel = D3DFMT_A8R8G8B8;
	// �����ͳ�ʼ��������ͼ
#ifdef USE_FP16
	FormatFourChannel = D3DFMT_A16B16G16R16F;
	FormatDualChannel = D3DFMT_G16R16F;
#elif defined USE_FP32
	FormatFourChannel = D3DFMT_A32B32G32R32F;
	FormatDualChannel = D3DFMT_G32R32F;
#else
	mymessage("δ����ʹ�ø��������޷���ʼ������ģ�����棡");
	return E_FAIL;
#endif

	// ��������λ����ͼ��ֵ����Shader��ʼ���Ļ�������ŵ�������
	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexPrevP, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexPrevP_Sign, NULL));

	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexNowP, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexNowP_Sign, NULL));

	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexNextP, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexNextP_Sign, NULL));


	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexNormal, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTangent, NULL));

	// ��ʱʹ�õ�
	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTemp[0], NULL));
	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTemp[1], NULL));
	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTemp_Sign[0], NULL));
	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTemp_Sign[1], NULL));

	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_0, FormatFourChannel, D3DPOOL_MANAGED, &m_pTexTempCPU[0], NULL));
	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_0, FormatFourChannel, D3DPOOL_MANAGED, &m_pTexTempCPU[1], NULL));



	// �̶���λ����ͼ����ʼ���Ĳ��������
	V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_0, D3DFMT_A8, D3DPOOL_MANAGED, &m_pTexFixedPoint, NULL));

	// ������Ե���ͼ
	for(UINT i = 0; i < 4; i++)
	{
		V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_0, FormatDualChannel, D3DPOOL_MANAGED, &m_pTexSpringPairRectangle[i], NULL));
		V_RETURN(d3ddevice->CreateTexture(m_ClothData.iWidth, m_ClothData.iHeight, 1, D3DUSAGE_0, FormatDualChannel, D3DPOOL_MANAGED, &m_pTexSpringPairShear[i], NULL));
	}

	V_RETURN(InitSpringPairTexture());





	// ��ʼ��Vertex Buffer
	m_iStride = sizeof(QUADVERTEXTYPE);
	V_RETURN(d3ddevice->CreateVertexBuffer(4 * m_iStride, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVBQuad, NULL));

	QUADVERTEXTYPE QuadVertex[4] = 
	{
		{-1.0f, -1.0f, 0,	0,1,0,0},
		{-1.0f, 1.0f, 0,	0,0,0,0},
		{1.0f, -1.0f, 0,	1,1,0,0},
		{1.0f, 1.0f, 0,		1,0,0,0}
	};
	VOID* pVertexStream;
	V_RETURN(m_pVBQuad->Lock(0, 4 * m_iStride, &pVertexStream, 0));
	memcpy(pVertexStream, QuadVertex, 4 * m_iStride);
	m_pVBQuad->Unlock();


	// ������Ȼ���
	D3DSURFACE_DESC DescDepth;
	LPDIRECT3DSURFACE9 pOldDepth = NULL;

	if(FAILED(d3ddevice->GetDepthStencilSurface(&pOldDepth)))
		return E_FAIL;
	if(FAILED(pOldDepth->GetDesc(&DescDepth)))
		return E_FAIL;
	SAFE_RELEASE(pOldDepth);

	V_RETURN(d3ddevice->CreateDepthStencilSurface(m_ClothData.iWidth, m_ClothData.iHeight, DescDepth.Format, DescDepth.MultiSampleType, DescDepth.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL));





	// ��ʼ��Shader
	D3DVERTEXELEMENT9 Dclr[] = {
		{0,0,D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0,12,D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};

	// MISC
	V_RETURN(m_VS.InitVertexShader("shader\\Cloth\\VS.vsh", Dclr));
	V_RETURN(m_PSSetToZero.InitPixelShader("shader\\Cloth\\ClearTexture.psh"));
	V_RETURN(m_PSCopyTexture.InitPixelShader("shader\\Cloth\\CopyTexture.psh"));

	// λ�úͷ���
	V_RETURN(m_PSInitPosition.InitPixelShader("shader\\Cloth\\Position\\InitPosition.psh"));
	//V_RETURN(m_PSGenerateNormal.InitPixelShader("shader\\Cloth\\AddOn\\GenerateNormal.psh"));
	//V_RETURN(m_PSGenerateTangent.InitPixelShader("shader\\Cloth\\AddOn\\GenerateNormalandTangent.psh"));

	// ����
	V_RETURN(m_PSApplyForce.InitPixelShader("shader\\Cloth\\Position\\ApplyForce.psh"));

	// ����
	V_RETURN(m_PSApplySpring.InitPixelShader("shader\\Cloth\\Position\\ApplySpring.psh"));

	// ��ײ
	V_RETURN(m_PSCollide_Plane.InitPixelShader("shader\\Cloth\\Collision\\Plane.psh"));
	V_RETURN(m_PSCollide_Sphere.InitPixelShader("shader\\Cloth\\Collision\\Sphere.psh"));
	V_RETURN(m_PSCollide_Box.InitPixelShader("shader\\Cloth\\Collision\\Box.psh"));
	V_RETURN(m_PSCollide_Ellipse.InitPixelShader("shader\\Cloth\\Collision\\Ellipse.psh"));


    

	// �����Shader��ʼ��λ����ͼ����Ȼ��Ҫ��ʼ���̶�����ͼ
	V_RETURN(ResetCloth(PtLTPosition, VecHorizon));

	// Test
	HRESULT hr = S_OK;
	if(FAILED(hr = D3DXSaveTextureToFile("c:\\pos.dds", D3DXIFF_DDS, m_pTexNextP, NULL)))
		mymessage("Failed???");
	D3DXSaveTextureToFile("c:\\pos_sign.hdr", D3DXIFF_HDR, m_pTexNextP_Sign, NULL);
	D3DXSaveTextureToFile("c:\\fixpoint.dds", D3DXIFF_DDS, m_pTexFixedPoint, NULL);

	D3DXSaveTextureToFile("c:\\pair_Xż.hdr", D3DXIFF_HDR, m_pTexSpringPairRectangle[0], NULL);
	D3DXSaveTextureToFile("c:\\pair_X��.hdr", D3DXIFF_HDR, m_pTexSpringPairRectangle[1], NULL);
	D3DXSaveTextureToFile("c:\\pair_Yż.hdr", D3DXIFF_HDR, m_pTexSpringPairRectangle[2], NULL);
	D3DXSaveTextureToFile("c:\\pair_Y��.hdr", D3DXIFF_HDR, m_pTexSpringPairRectangle[3], NULL);


	// �ɹ�
	m_iCreateAttrib = 1;
	return S_OK;
}