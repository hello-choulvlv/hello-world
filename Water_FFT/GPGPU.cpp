#include "GPGPU.h"
#include "Texture.h"

/*****************Fourier Transform***************/
KFourierTransform::KFourierTransform()
{
	m_iCreateAttrib = 0;
	m_iWidth = m_iHeight = 0;
	m_iStride = 0;

	m_ppTexButterflyX = m_ppTexButterflyY = NULL;
	m_ppTexButterflyX_Sign = m_ppTexButterflyY_Sign = NULL;

	m_pTexScrambleX = m_pTexScrambleY = NULL;
	m_pTexResult = NULL;
	m_pTexTempUse[0] = m_pTexTempUse[1] = NULL;
	m_pDepthBuffer = NULL;
	m_pVB = NULL;
}

void KFourierTransform::Release()
{
	m_iCreateAttrib = 0;
	m_iWidth = m_iHeight = 0;
	m_iStride = 0;

	SAFE_RELEASE(m_pDepthBuffer);

	SAFE_RELEASE(m_pTexResult);

	SAFE_RELEASE(m_pTexTempUse[0]);
	SAFE_RELEASE(m_pTexTempUse[1]);

	SAFE_RELEASE(m_pTexScrambleX);
	SAFE_RELEASE(m_pTexScrambleY);

	UINT i = 0;
	if(m_ppTexButterflyX)
	{
		for(i = 0; i < m_iButterflyNumX; i++)
		{
			SAFE_RELEASE(m_ppTexButterflyX[i]);
		}
		SAFE_DELETE_ARRAY(m_ppTexButterflyX);
	}
	if(m_ppTexButterflyX_Sign)
	{
		for(i = 0; i < m_iButterflyNumX; i++)
		{
			SAFE_RELEASE(m_ppTexButterflyX_Sign[i]);
		}
		SAFE_DELETE_ARRAY(m_ppTexButterflyX_Sign);
	}

	if(m_ppTexButterflyY)
	{
		for(i = 0; i < m_iButterflyNumY; i++)
			SAFE_RELEASE(m_ppTexButterflyY[i]);
		SAFE_DELETE_ARRAY(m_ppTexButterflyY);
	}
	if(m_ppTexButterflyY_Sign)
	{
		for(i = 0; i < m_iButterflyNumY; i++)
			SAFE_RELEASE(m_ppTexButterflyY_Sign[i]);
		SAFE_DELETE_ARRAY(m_ppTexButterflyY_Sign);
	}


	SAFE_RELEASE(m_pVB);

	m_VSDrawQuad.Release();
	m_PSGetResult.Release();
	m_PSScrambleX.Release();
	m_PSScrambleY.Release();
	m_PSIFFTButterflyX.Release();
	m_PSIFFTButterflyY.Release();
	m_PSFFTButterflyX.Release();
	m_PSFFTButterflyY.Release();
}




HRESULT KFourierTransform::GetResultData(IDirect3DTexture9* pDestTexture, BOOL bMulFourierCoef)
{
	if(m_iCreateAttrib != 2 || !m_pTexResult)
		return D3DERR_NOTAVAILABLE;

	if(!pDestTexture)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	UINT i = 0;

	// Ŀ����ͼ��Ч�Լ��飬���볤��һ�£�4ͨ�������ʽ�����ұ�����RT
	D3DSURFACE_DESC Desc;
	V_RETURN(pDestTexture->GetLevelDesc(0, &Desc));
	if(Desc.Width != m_iWidth || Desc.Height != m_iHeight || Desc.Usage != D3DUSAGE_RENDERTARGET)
		return D3DERR_WRONGTEXTUREFORMAT;

	if(Desc.Format != D3DFMT_A16B16G16R16F && Desc.Format != D3DFMT_A32B32G32R32F)
		return D3DERR_WRONGTEXTUREFORMAT;



	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա�FFT��Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// ������Ⱦ��Դ��Ŀ��
	SetTexturedRenderTarget(0, pDestTexture, NULL);

	V_RETURN(d3ddevice->SetTexture(0, m_pTexResult));

	// ������Ⱦ����
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	// ����Shader�������Ĵ���
	V_RETURN(m_PSGetResult.SetPixelShader());
	V_RETURN(m_PSGetResult.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));

	float fDividCoef = bMulFourierCoef ? 1.0f / ((float)m_iWidth * (float)m_iHeight) : 1.0f;
	V_RETURN(m_PSGetResult.SetConstant(1, &D3DXVECTOR4(fDividCoef, fDividCoef, 1, 1), 1));

	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));
	V_RETURN(d3ddevice->SetFVF(0));

	// ��ʼ��Ⱦ
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// �ָ�Pixel Shader��RT����Ȼ���
	V_RETURN(m_PSGetResult.RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	// ���
	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);
	return S_OK;
}






HRESULT KFourierTransform::CoreProcess(BOOL bIFFT, IDirect3DTexture9* pTexFFTSource)
{
	if(!m_iCreateAttrib || !m_pTexTempUse || !m_pTexResult)
		return D3DERR_NOTAVAILABLE;

	if(!m_pTexTempUse[0] || !m_pTexTempUse[1])
		return D3DERR_NOTAVAILABLE;

	if(!pTexFFTSource)
		return D3DERR_INVALIDCALL;

	// �����m_Width/Height��ͬ
	D3DSURFACE_DESC Desc;
	pTexFFTSource->GetLevelDesc(0, &Desc);
	if(Desc.Width != m_iWidth || Desc.Height != m_iHeight || Desc.Pool == D3DPOOL_SYSTEMMEM)
		return D3DERR_WRONGTEXTUREFORMAT;



	HRESULT hr = S_OK;
	UINT i = 0;

	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա�FFT��Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));

	// ��ʼ������4��
	UINT iTempUseNo = 0;	// ���ƽ���������ʱʹ�õ���ͼ��

	// X
	// ScrambleXҪ�������ȣ���Ϊ�����Source��ͼ����TempUse��ͼ�������1*Y����ͼ����������Ͳ���Ӱ��ֵ�����У��������˸�����ͼ������
	V_RETURN(ScrambleX(pTexFFTSource, m_pTexTempUse[iTempUseNo]));

	if(m_iWidth > 1)
	{
		// ButterflyX��Butterfly���ݱ����2���ݲ���Ч��������һ�����⴦�����Ϊ1ʱ����ʹ�ã������Ϊ1ʱScramble��Ȼ�ᱣ��ԭ���ݣ�����Ӱ��ģ����Ա�����
		// �������ֻ��1D����ô�����ButterflyY�ͻ�����������Ҫǿ�������TexResult
		if(m_iHeight == 1)
		{
			for(i = 0; i < m_iButterflyNumX - 1; i++)
			{
				// �ϴε�Ŀ����ΪԴ��ȡ����ΪĿ��
				V_RETURN(ButterflyX(i, bIFFT, m_pTexTempUse[iTempUseNo], m_pTexTempUse[!iTempUseNo]));
				iTempUseNo = !iTempUseNo;
			}
			// ���һ�������TexResult
			V_RETURN(ButterflyX(i, bIFFT, m_pTexTempUse[iTempUseNo], m_pTexResult));
			iTempUseNo = !iTempUseNo;
		}
		else
		{
			for(i = 0; i < m_iButterflyNumX; i++)
			{
				// �ϴε�Ŀ����ΪԴ��ȡ����ΪĿ��
				V_RETURN(ButterflyX(i, bIFFT, m_pTexTempUse[iTempUseNo], m_pTexTempUse[!iTempUseNo]));
				iTempUseNo = !iTempUseNo;
			}
		}// end iHeight == 1
	}// end X

	// Y
	if(m_iHeight > 1)
	{
		// ScrambleY��2D FFT
		V_RETURN(ScrambleY(m_pTexTempUse[iTempUseNo], m_pTexTempUse[!iTempUseNo]));
		iTempUseNo = !iTempUseNo;

		// ButterflyY
		for(i = 0; i < m_iButterflyNumY - 1; i++)
		{
			V_RETURN(ButterflyY(i, bIFFT, m_pTexTempUse[iTempUseNo], m_pTexTempUse[!iTempUseNo]));
			iTempUseNo = !iTempUseNo;
		}
		// ���һ��Butterfly��ֱ�������Result
		V_RETURN(ButterflyY(i, bIFFT, m_pTexTempUse[iTempUseNo], m_pTexResult));
	}




	// �ָ�RT
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);


	// ���
	m_iCreateAttrib = 2;
	return S_OK;
}




HRESULT KFourierTransform::ScrambleX(IDirect3DTexture9* pSource, IDirect3DTexture9* pRT)
{
	if(!m_iCreateAttrib || !m_pTexScrambleX)
		return D3DERR_NOTAVAILABLE;

	if(!pSource || !pRT)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	UINT i = 0;

	// ������Ⱦ��Դ��Ŀ��
	V_RETURN(SetTexturedRenderTarget(0, pRT, NULL));

	V_RETURN(d3ddevice->SetTexture(0, m_pTexScrambleX));
	V_RETURN(d3ddevice->SetTexture(1, pSource));

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

	// ����Shader�������Ĵ���
	V_RETURN(m_PSScrambleX.SetPixelShader());
	V_RETURN(m_PSScrambleX.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));
	V_RETURN(d3ddevice->SetFVF(0));

	// ��ʼ��Ⱦ
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// �ָ�Pixel Shader
	V_RETURN(m_PSScrambleX.RestorePixelShader());

	// ���
	return S_OK;
}




HRESULT KFourierTransform::ScrambleY(IDirect3DTexture9* pSource, IDirect3DTexture9* pRT)
{
	if(!m_iCreateAttrib || !m_pTexScrambleY)
		return D3DERR_NOTAVAILABLE;

	if(!pSource || !pRT)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	UINT i = 0;

	// ������Ⱦ��Դ��Ŀ��
	V_RETURN(SetTexturedRenderTarget(0, pRT, NULL));

	V_RETURN(d3ddevice->SetTexture(0, m_pTexScrambleY));
	V_RETURN(d3ddevice->SetTexture(1, pSource));

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

	// ����Shader�������Ĵ���
	V_RETURN(m_PSScrambleY.SetPixelShader());
	V_RETURN(m_PSScrambleY.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));
	V_RETURN(d3ddevice->SetFVF(0));

	// ��ʼ��Ⱦ
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// �ָ�Pixel Shader
	V_RETURN(m_PSScrambleY.RestorePixelShader());

	// ���
	return S_OK;
}






HRESULT KFourierTransform::ButterflyX(UINT iLitNo, BOOL bIFFT, IDirect3DTexture9* pSource, IDirect3DTexture9* pRT)
{
	if(!m_iCreateAttrib || !m_ppTexButterflyX || !m_ppTexButterflyX_Sign)
		return D3DERR_NOTAVAILABLE;

	if(!pSource || !pRT || iLitNo >= m_iButterflyNumX)
		return D3DERR_INVALIDCALL;

	if(!m_ppTexButterflyX[iLitNo] || !m_ppTexButterflyX_Sign[iLitNo])
		return D3DERR_NOTAVAILABLE;

	HRESULT hr = S_OK;
	UINT i = 0;
	PIXELSHADER *pPS = bIFFT ? &m_PSIFFTButterflyX : &m_PSFFTButterflyX;

	// ������Ⱦ��Դ��Ŀ��
	V_RETURN(SetTexturedRenderTarget(0, pRT, NULL));

	V_RETURN(d3ddevice->SetTexture(0, m_ppTexButterflyX[iLitNo]));
	V_RETURN(d3ddevice->SetTexture(1, m_ppTexButterflyX_Sign[iLitNo]));
	V_RETURN(d3ddevice->SetTexture(2, pSource));

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


	// ����Shader�������Ĵ���
	V_RETURN(pPS->SetPixelShader());
	V_RETURN(pPS->SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));
	V_RETURN(d3ddevice->SetFVF(0));

	// ��ʼ��Ⱦ
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// �ָ�Pixel Shader
	V_RETURN(pPS->RestorePixelShader());

	// ���
	return S_OK;
}



HRESULT KFourierTransform::ButterflyY(UINT iLitNo, BOOL bIFFT, IDirect3DTexture9* pSource, IDirect3DTexture9* pRT)
{
	if(!m_iCreateAttrib || !m_ppTexButterflyY || !m_ppTexButterflyY_Sign)
		return D3DERR_NOTAVAILABLE;

	if(!pSource || !pRT || iLitNo >= m_iButterflyNumY)
		return D3DERR_INVALIDCALL;

	if(!m_ppTexButterflyY[iLitNo] || !m_ppTexButterflyY_Sign[iLitNo])
		return D3DERR_NOTAVAILABLE;

	HRESULT hr = S_OK;
	UINT i = 0;
	PIXELSHADER *pPS = bIFFT ? &m_PSIFFTButterflyY : &m_PSFFTButterflyY;

	// ������Ⱦ��Դ��Ŀ��
	V_RETURN(SetTexturedRenderTarget(0, pRT, NULL));

	V_RETURN(d3ddevice->SetTexture(0, m_ppTexButterflyY[iLitNo]));
	V_RETURN(d3ddevice->SetTexture(1, m_ppTexButterflyY_Sign[iLitNo]));
	V_RETURN(d3ddevice->SetTexture(2, pSource));

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


	// ����Shader�������Ĵ���
	V_RETURN(pPS->SetPixelShader());
	V_RETURN(pPS->SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));
	V_RETURN(d3ddevice->SetFVF(0));

	// ��ʼ��Ⱦ
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// �ָ�Pixel Shader
	V_RETURN(pPS->RestorePixelShader());

	// ���
	return S_OK;
}







/****************************��ʼ��**********************************/

HRESULT KFourierTransform::Init(UINT iWidth, UINT iHeight)
{
	if(m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;
	if(!iWidth || !iHeight)
		return D3DERR_INVALIDCALL;

	// ����Ƿ�2���ݣ������2���ݾͻ᷵��0����ע������iWidth��iHeight����һ������1����������iWidth=iHeight=1
	if(iWidth > 1)
		m_iButterflyNumX = CheckPowerOf2(iWidth);
	else
		m_iButterflyNumX = 1;

	if(iHeight > 1)
		m_iButterflyNumY = CheckPowerOf2(iHeight);
	else
		m_iButterflyNumY = 1;

	if(!m_iButterflyNumX || !m_iButterflyNumY || m_iButterflyNumX * m_iButterflyNumY == 1)
		return D3DERR_INVALIDCALL;

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	// ���������ͼ
	HRESULT hr = S_OK;
	UINT i = 0, iX = 0, iY = 0;
	LPDIRECT3DSURFACE9 pSurf = NULL;
	D3DLOCKED_RECT Rect;
	float16 *p16 = NULL;
	float *p = NULL;
	float fValue = 0.0f;
	UINT iSwapNo = 0;




	D3DFORMAT FormatDualChannel = D3DFMT_A8R8G8B8;
	D3DFORMAT FormatFourChannel = D3DFMT_A8R8G8B8;
#ifdef USE_FP16
	FormatDualChannel = D3DFMT_G16R16F;
	FormatFourChannel = D3DFMT_A16B16G16R16F;
#elif defined USE_FP32
	FormatDualChannel = D3DFMT_G32R32F;
	FormatFourChannel = D3DFMT_A32B32G32R32F;
#else
	mymessage("δ����ʹ�ø��������޷���ʼ��FFT���棡");
	return E_FAIL;
#endif

	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexResult, NULL));



	// ��ʼ��Quad
	V_RETURN(d3ddevice->CreateVertexBuffer(4*sizeof(QUADVERTEXTYPE), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVB, NULL));

	QUADVERTEXTYPE QuadVertex[4] = 
	{
		/*
		{-0.5f, -0.5f									,0,1,		0,1,0,0},
		{-0.5f, (float)m_iHeight-0.5f					,0,1,		0,0,0,0},
		{(float)m_iWidth-0.5f, -0.5f					,0,1,		1,1,0,0},
		{(float)m_iWidth-0.5f, (float)m_iHeight-0.5		,0,1,		1,0,0,0}
		*/
		{-1.0f, -1.0f, 0,	0,1,0,0},
		{-1.0f, 1.0f, 0,	0,0,0,0},
		{1.0f, -1.0f, 0,	1,1,0,0},
		{1.0f, 1.0f, 0,		1,0,0,0}
	};

	m_iStride = sizeof(QUADVERTEXTYPE);

	VOID* pVertexStream;
	V_RETURN(m_pVB->Lock(0, 4 * m_iStride, &pVertexStream, 0));
	memcpy(pVertexStream, QuadVertex, 4 * m_iStride);
	m_pVB->Unlock();

	// ��ʼ��Shader
	D3DVERTEXELEMENT9 Dclr[] = {
		{0,0,D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0,12,D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};

	V_RETURN(m_VSDrawQuad.InitVertexShader("shader\\FFT\\Quad.vsh", Dclr));

	V_RETURN(m_PSScrambleX.InitPixelShader("shader\\FFT\\ScrambleX.psh"));
	V_RETURN(m_PSScrambleY.InitPixelShader("shader\\FFT\\ScrambleY.psh"));

	V_RETURN(m_PSGetResult.InitPixelShader("shader\\FFT\\GetResult.psh"));

	V_RETURN(m_PSIFFTButterflyX.InitPixelShader("shader\\FFT\\IFFT_ButterflyX.psh"));
	V_RETURN(m_PSFFTButterflyX.InitPixelShader("shader\\FFT\\FFT_ButterflyX.psh"));
	V_RETURN(m_PSFFTButterflyY.InitPixelShader("shader\\FFT\\FFT_ButterflyY.psh"));
	V_RETURN(m_PSIFFTButterflyY.InitPixelShader("shader\\FFT\\IFFT_ButterflyY.psh"));



	// ������Ȼ���
	D3DSURFACE_DESC DescDepth;
	LPDIRECT3DSURFACE9 pOldDepth = NULL;

	if(FAILED(d3ddevice->GetDepthStencilSurface(&pOldDepth)))
		return E_FAIL;
	if(FAILED(pOldDepth->GetDesc(&DescDepth)))
		return E_FAIL;
	SAFE_RELEASE(pOldDepth);

	V_RETURN(d3ddevice->CreateDepthStencilSurface(iWidth, iHeight, DescDepth.Format, DescDepth.MultiSampleType, DescDepth.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL));


	// ������ʱʹ�õ���ͼ
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTempUse[0], NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTempUse[1], NULL));



	// ��ʼ��Scramble��ͼ--X
	V_RETURN(d3ddevice->CreateTexture(iWidth, 1, 1, D3DUSAGE_0, FormatDualChannel, D3DPOOL_MANAGED, &m_pTexScrambleX, NULL));

	V_RETURN(m_pTexScrambleX->GetSurfaceLevel(0, &pSurf));
	V_RETURN(pSurf->LockRect(&Rect, NULL, 0));

	p16 = (float16 *)((BYTE *)Rect.pBits);	
	p = (float *)((BYTE *)Rect.pBits);


	for(iX=0; iX < iWidth; iX++)
	{
		// �õ��������������
		iSwapNo = ReverseBitOrder(iX, m_iButterflyNumX);
		// ת��������������룬ע���1/2 Tap�Ա��������Ķ���
		fValue = (float)iSwapNo / (float)iWidth + 0.5f / (float)iWidth;

#ifdef USE_FP16
		*p16++ = floatToFP16(fValue);
		*p16++ = floatToFP16(1.0f);
#elif defined USE_FP32
		*p++ = fValue;
		*p++ = 1.0f;
#endif
	}

	V_RETURN(pSurf->UnlockRect());
	SAFE_RELEASE(pSurf);



	// ��ʼ��Scramble��ͼ--Y
	V_RETURN(d3ddevice->CreateTexture(1, iHeight, 1, D3DUSAGE_0, FormatDualChannel, D3DPOOL_MANAGED, &m_pTexScrambleY, NULL));

	V_RETURN(m_pTexScrambleY->GetSurfaceLevel(0, &pSurf));
	V_RETURN(pSurf->LockRect(&Rect, NULL, 0));

	for(iY=0; iY < iHeight; iY++)
	{
		p16 = (float16 *)((BYTE *)Rect.pBits + iY * Rect.Pitch);	
		p = (float *)((BYTE *)Rect.pBits + iY * Rect.Pitch);

		// �õ��������������
		iSwapNo = ReverseBitOrder(iY, m_iButterflyNumY);
		// ת��������������룬ע���1/2 Tap�Ա��������Ķ���
		fValue = (float)iSwapNo / (float)iHeight + 0.5f / (float)iHeight;

#ifdef USE_FP16
		*p16++ = floatToFP16(1.0f);
		*p16++ = floatToFP16(fValue);
#elif defined USE_FP32
		*p++ = 1.0f;
		*p++ = fValue;
#endif
	}

	pSurf->UnlockRect();
	SAFE_RELEASE(pSurf);




	// ��ʼ��Butterfly��ͼ
	if(m_iButterflyNumX)
	{
		m_ppTexButterflyX = new IDirect3DTexture9*[m_iButterflyNumX];
		m_ppTexButterflyX_Sign = new IDirect3DTexture9*[m_iButterflyNumX];
		ZeroMemory(m_ppTexButterflyX, sizeof(IDirect3DTexture9*) * m_iButterflyNumX);
		ZeroMemory(m_ppTexButterflyX_Sign, sizeof(IDirect3DTexture9*) * m_iButterflyNumX);

		for(i = 0; i < m_iButterflyNumX; i++)
		{
			V_RETURN(d3ddevice->CreateTexture(iWidth, 1, 1, D3DUSAGE_0, FormatFourChannel, D3DPOOL_MANAGED, &m_ppTexButterflyX[i], NULL));
			V_RETURN(d3ddevice->CreateTexture(iWidth, 1, 1, D3DUSAGE_0, FormatFourChannel, D3DPOOL_MANAGED, &m_ppTexButterflyX_Sign[i], NULL));
			InitButterflyX(i);
		}
	}


	if(m_iButterflyNumY)
	{
		m_ppTexButterflyY = new IDirect3DTexture9*[m_iButterflyNumY];
		m_ppTexButterflyY_Sign = new IDirect3DTexture9*[m_iButterflyNumY];
		ZeroMemory(m_ppTexButterflyY, sizeof(IDirect3DTexture9*) * m_iButterflyNumY);
		ZeroMemory(m_ppTexButterflyY_Sign, sizeof(IDirect3DTexture9*) * m_iButterflyNumY);

		for(i = 0; i < m_iButterflyNumY; i++)
		{
			V_RETURN(d3ddevice->CreateTexture(1, iHeight, 1, D3DUSAGE_0, FormatFourChannel, D3DPOOL_MANAGED, &m_ppTexButterflyY[i], NULL));
			V_RETURN(d3ddevice->CreateTexture(1, iHeight, 1, D3DUSAGE_0, FormatFourChannel, D3DPOOL_MANAGED, &m_ppTexButterflyY_Sign[i], NULL));
			InitButterflyY(i);
		}
	}


	// �ɹ�
	m_iCreateAttrib = 1;
	return S_OK;
}







HRESULT KFourierTransform::InitButterflyX(UINT iLitNo)
{
	if(iLitNo > m_iButterflyNumX || !m_ppTexButterflyX)
		return E_FAIL;


	HRESULT hr = S_OK;
	UINT i = 0, iX = 0, iY = 0;
	D3DLOCKED_RECT Rect, Rect_Sign;
	float16 *p16 = NULL, *p16_Sign = NULL;
	float *p = NULL, *p_Sign = NULL;
	float fExpValue = 0.0f, fW_re = 0.0f, fW_im = 0.0f;
	D3DXVECTOR4 fData, fData_Sign;


	// Lock��Ready? Go!
	V_RETURN(m_ppTexButterflyX[iLitNo]->LockRect(0, &Rect, NULL, 0));
	p16 = (float16 *)((BYTE *)Rect.pBits);	
	p = (float *)((BYTE *)Rect.pBits);

	V_RETURN(m_ppTexButterflyX_Sign[iLitNo]->LockRect(0, &Rect_Sign, NULL, 0));
	p16_Sign = (float16 *)((BYTE *)Rect_Sign.pBits);	
	p_Sign = (float *)((BYTE *)Rect_Sign.pBits);



	// ������ÿ��Ԫ������û���ⶼ�������ģ����İ�
	UINT iElemNumPerGroup = (UINT)pow(2, iLitNo+1);
	UINT iGroupNum = m_iWidth / iElemNumPerGroup;

	UINT iGroupNo = 0;			// ��ǰ�������ĸ��飬�������ҲҪ�������
	UINT iNoInGroup = 0;		// ��ǰ�����ڸ����е���ţ���ͷ���Ϊ0�����ε�����
	UINT iNoInHalfGroup = 0;	// ��ǰ/��������ڰ����е���ţ����ϰ�����°����һ��Ԫ�صļ��������ͨ��bCur/PairUpper�жϣ�����������صĸ�ֵ��ȣ�

	BOOL bCurUpper = FALSE;		// ��ǰ���������ϰ��黹���°��飬true��ʾ�ϣ�false��ʾ��
	UINT iPairPosition = 0;		// �����������һ�е������
	BOOL bPairUpper = FALSE;	// ������������ϰ��黹���°��飬true��ʾ�ϣ�false��ʾ�£�����bCurUpper��ֵ�ض��෴

	UINT iUXInterval = m_iWidth / 2 / (UINT)powf(2.0f, (float)iLitNo), iUX = 0;	// ����W��ָ��UX��Ҫ�ģ�ÿ�ε���UX�������ص�����������һ�����


	// ��ǰ���������У��Ƕ��ٸ�����һ��
	for(iX=0; iX < m_iWidth; iX++)
	{
		// �����жϵ�ǰ/��������������飬���ڸ����е�������ţ��Լ����ϰ��黹���°��飬�������е����
		iGroupNo = iX / iElemNumPerGroup;
		iNoInGroup = iX % iElemNumPerGroup;
		iNoInHalfGroup = iX % (iElemNumPerGroup / 2);
		// ��ǰԪ�����ϰ���
		if(iNoInGroup < iElemNumPerGroup / 2)
		{
			bCurUpper = TRUE;
			bPairUpper = FALSE;
			iPairPosition = iX + iElemNumPerGroup / 2;

			// ��Cur + Pair * W�͵ģ�Cur�浽A�㣨x��������Pair�浽B�㣨y������
			// ת��������������룬ע���1/2 Tap�Ա��������Ķ���
			fData.x = (float)iX / (float)m_iWidth + 0.5f / (float)m_iWidth;
			fData.y = (float)iPairPosition / (float)m_iWidth + 0.5f / (float)m_iWidth;
			// �õ�W��ָ��
			iUX = iNoInHalfGroup * iUXInterval;
			// ����Wʵ�����鲿
			fExpValue = (float)iUX / (float)m_iWidth;
			fW_re = cosf(2.0f * D3DX_PI * fExpValue);
			fW_im = sinf(2.0f * D3DX_PI * fExpValue);
			// ����ֵд���ݣ���W����ֵ��ŵ�zw�����У����������Ŵ浽Sign��ͼ��
			fData.z = absf(fW_re);
			fData.w = absf(fW_im);
			fData_Sign.z = (fW_re < 0.0f) ? 0.0f : 2.0f;
			fData_Sign.w = (fW_im < 0.0f) ? 0.0f : 2.0f;
			// ��ǰ�������ϰ��飬�Ǽӷ���ʽ��ϣ�����Ӧ������д��Sign��ͼ��
			fData_Sign.x = 2.0f;
			fData_Sign.y = 0.0f;	// yֵδ��
		}
		// ��ǰԪ�����°���
		else
		{
			bCurUpper = FALSE;
			bPairUpper = FALSE;
			iPairPosition = iX - iElemNumPerGroup / 2;

			// ��Pair - Cur * W�͵ģ�Pair�浽A�㣨x��������Cur�浽B�㣨y������
			// ת��������������룬ע���1/2 Tap�Ա��������Ķ���
			fData.x = (float)iPairPosition / (float)m_iWidth + 0.5f / (float)m_iWidth;
			fData.y = (float)iX / (float)m_iWidth + 0.5f / (float)m_iWidth;
			// �õ�W��ָ��
			iUX = iNoInHalfGroup * iUXInterval;
			// ����Wʵ�����鲿
			fExpValue = (float)iUX / (float)m_iWidth;
			fW_re = cosf(2.0f * D3DX_PI * fExpValue);
			fW_im = sinf(2.0f * D3DX_PI * fExpValue);
			// ����ֵд���ݣ���W����ֵ��ŵ�zw�����У����������Ŵ浽Sign��ͼ��
			fData.z = absf(fW_re);
			fData.w = absf(fW_im);
			fData_Sign.z = (fW_re < 0.0f) ? 0.0f : 2.0f;
			fData_Sign.w = (fW_im < 0.0f) ? 0.0f : 2.0f;
			// ��ǰ���ؾ����°��飬��Ҫ����������ϣ�д��Sign��ͼ��
			fData_Sign.x = 0.0f;
			fData_Sign.y = 0.0f;	// yֵδ��
		}


		// д����ͼ
		char a[100] = "";
		for(i = 0; i < 4; i++)
		{
#ifdef USE_FP16
			*p16++ = floatToFP16(fData[i]);
			*p16_Sign++ = floatToFP16(fData_Sign[i]);
#elif defined USE_FP32
			*p++ = fData[i];
			*p_Sign++ = fData_Sign[i];
#endif
		}
	}

	// ����
	V_RETURN(m_ppTexButterflyX[iLitNo]->UnlockRect(0));
	V_RETURN(m_ppTexButterflyX_Sign[iLitNo]->UnlockRect(0));

	return S_OK;
}









HRESULT KFourierTransform::InitButterflyY(UINT iLitNo)
{
	if(iLitNo > m_iButterflyNumY || !m_ppTexButterflyY)
		return E_FAIL;

	D3DXVECTOR4 fData, fData_Sign;
	HRESULT hr = S_OK;
	UINT i = 0, iY = 0;
	D3DLOCKED_RECT Rect, Rect_Sign;
	float16 *p16 = NULL, *p16_Sign = NULL;
	float *p = NULL, *p_Sign = NULL;
	float fExpValue = 0.0f, fW_re = 0.0f, fW_im = 0.0f;


	// Lock
	V_RETURN(m_ppTexButterflyY[iLitNo]->LockRect(0, &Rect, NULL, 0));
	V_RETURN(m_ppTexButterflyY_Sign[iLitNo]->LockRect(0, &Rect_Sign, NULL, 0));


	// ������ÿ��Ԫ������û���ⶼ�������ģ����İ�
	UINT iElemNumPerGroup = (UINT)pow(2, iLitNo+1);
	UINT iGroupNum = m_iHeight / iElemNumPerGroup;

	UINT iGroupNo = 0;			// ��ǰ�������ĸ��飬�������ҲҪ�������
	UINT iNoInGroup = 0;		// ��ǰ�����ڸ����е���ţ���ͷ���Ϊ0�����ε�����
	UINT iNoInHalfGroup = 0;	// ��ǰ/��������ڰ����е���ţ����ϰ�����°����һ��Ԫ�صļ��������ͨ��bCur/PairUpper�жϣ�����������صĸ�ֵ��ȣ�

	BOOL bCurUpper = FALSE;		// ��ǰ���������ϰ��黹���°��飬true��ʾ�ϣ�false��ʾ��
	UINT iPairPosition = 0;		// �����������һ�е������
	BOOL bPairUpper = FALSE;	// ������������ϰ��黹���°��飬true��ʾ�ϣ�false��ʾ�£�����bCurUpper��ֵ�ض��෴

	UINT iUXInterval = m_iHeight / 2 / (UINT)powf(2.0f, (float)iLitNo), iUX = 0;	// ����W��ָ��UX��Ҫ�ģ�ÿ�ε���UX�������ص�����������һ�����


	// ��ǰ���������У��Ƕ��ٸ�����һ��
	for(iY=0; iY < m_iHeight; iY++)
	{
		p16 = (float16 *)((BYTE *)Rect.pBits + iY * Rect.Pitch);	
		p = (float *)((BYTE *)Rect.pBits + iY * Rect.Pitch);

		p16_Sign = (float16 *)((BYTE *)Rect_Sign.pBits + iY * Rect_Sign.Pitch);	
		p_Sign = (float *)((BYTE *)Rect_Sign.pBits + iY * Rect_Sign.Pitch);

		// �����жϵ�ǰ/��������������飬���ڸ����е�������ţ��Լ����ϰ��黹���°��飬�������е����
		iGroupNo = iY / iElemNumPerGroup;
		iNoInGroup = iY % iElemNumPerGroup;
		iNoInHalfGroup = iY % (iElemNumPerGroup / 2);
		// ��ǰԪ�����ϰ���
		if(iNoInGroup < iElemNumPerGroup / 2)
		{
			bCurUpper = TRUE;
			bPairUpper = FALSE;
			iPairPosition = iY + iElemNumPerGroup / 2;

			// ��Cur + Pair * W�͵ģ�Cur�浽A�㣨x��������Pair�浽B�㣨y������
			// ת��������������룬ע���1/2 Tap�Ա��������Ķ���
			fData.x = (float)iY / (float)m_iHeight + 0.5f / (float)m_iHeight;
			fData.y = (float)iPairPosition / (float)m_iHeight + 0.5f / (float)m_iHeight;
			// �õ�W��ָ��
			iUX = iNoInHalfGroup * iUXInterval;
			// ����Wʵ�����鲿
			fExpValue = (float)iUX / (float)m_iHeight;
			fW_re = cosf(2.0f * D3DX_PI * fExpValue);
			fW_im = sinf(2.0f * D3DX_PI * fExpValue);
			// ����ֵд���ݣ���W����ֵ��ŵ�zw�����У����������Ŵ浽Sign��ͼ��
			fData.z = absf(fW_re);
			fData.w = absf(fW_im);
			fData_Sign.z = (fW_re < 0.0f) ? 0.0f : 2.0f;
			fData_Sign.w = (fW_im < 0.0f) ? 0.0f : 2.0f;
			// ��ǰ�������ϰ��飬�Ǽӷ���ʽ��ϣ�����Ӧ������д��Sign��ͼ��
			fData_Sign.x = 2.0f;
			fData_Sign.y = 0.0f;	// yֵδ��
		}
		// ��ǰԪ�����°���
		else
		{
			bCurUpper = FALSE;
			bPairUpper = FALSE;
			iPairPosition = iY - iElemNumPerGroup / 2;

			// ��Pair - Cur * W�͵ģ�Pair�浽A�㣨x��������Cur�浽B�㣨y������
			// ת��������������룬ע���1/2 Tap�Ա��������Ķ���
			fData.x = (float)iPairPosition / (float)m_iHeight + 0.5f / (float)m_iHeight;
			fData.y = (float)iY / (float)m_iHeight + 0.5f / (float)m_iHeight;
			// �õ�W��ָ��
			iUX = iNoInHalfGroup * iUXInterval;
			// ����Wʵ�����鲿
			fExpValue = (float)iUX / (float)m_iHeight;
			fW_re = cosf(2.0f * D3DX_PI * fExpValue);
			fW_im = sinf(2.0f * D3DX_PI * fExpValue);
			// ����ֵд���ݣ���W����ֵ��ŵ�zw�����У����������Ŵ浽Sign��ͼ��
			fData.z = absf(fW_re);
			fData.w = absf(fW_im);
			fData_Sign.z = (fW_re < 0.0f) ? 0.0f : 2.0f;
			fData_Sign.w = (fW_im < 0.0f) ? 0.0f : 2.0f;
			// ��ǰ���ؾ����°��飬��Ҫ����������ϣ�д��Sign��ͼ��
			fData_Sign.x = 0.0f;
			fData_Sign.y = 0.0f;	// yֵδ��
		}


		// д����ͼ
		char a[100] = "";
		for(i = 0; i < 4; i++)
		{
#ifdef USE_FP16
			*p16++ = floatToFP16(fData[i]);
			*p16_Sign++ = floatToFP16(fData_Sign[i]);
#elif defined USE_FP32
			*p++ = fData[i];
			*p_Sign++ = fData_Sign[i];
#endif
		}
	}

	// ����
	V_RETURN(m_ppTexButterflyY[iLitNo]->UnlockRect(0));
	V_RETURN(m_ppTexButterflyY_Sign[iLitNo]->UnlockRect(0));

	return S_OK;
}















/*********************************��������CPU������Ҷ�任************************/

// ��IDFT��log
HRESULT KFourierTransform::DFTCPU(IDirect3DTexture9* pTexFFTSource, BOOL bIDFT, BOOL bMulFourierCoef)
{
	// �����ȳ�ʼ��
	if(!m_iCreateAttrib || !m_pTexResult)
		return D3DERR_NOTAVAILABLE;

	if(!pTexFFTSource)
		return D3DERR_INVALIDCALL;

	// �����m_Width/Height��ͬ�����Ҳ�����PoolDefault
	D3DSURFACE_DESC Desc;
	pTexFFTSource->GetLevelDesc(0, &Desc);
	if(Desc.Width != m_iWidth || Desc.Height != m_iHeight)
		return D3DERR_WRONGTEXTUREFORMAT;
	if(Desc.Format != D3DFMT_A32B32G32R32F)
		return D3DERR_WRONGTEXTUREFORMAT;

	// ��Դ��ͼ��Ŀ����ͼ��������תPool_Default����ͼ
	IDirect3DTexture9* pTexSrc = NULL, *pTexDst = NULL;
	LPDIRECT3DSURFACE9 pSurfSrc = NULL, pSurfDst = NULL, pSurfUser = NULL, pSurfResult = NULL;

	V_RETURN(d3ddevice->CreateTexture(Desc.Width, Desc.Height, 1, 0, Desc.Format, D3DPOOL_SYSTEMMEM, &pTexDst, NULL));
	V_RETURN(d3ddevice->CreateTexture(Desc.Width, Desc.Height, 1, 0, Desc.Format, D3DPOOL_SYSTEMMEM, &pTexSrc, NULL));

	V_RETURN(pTexSrc->GetSurfaceLevel(0, &pSurfSrc));
	V_RETURN(pTexDst->GetSurfaceLevel(0, &pSurfDst));
	V_RETURN(pTexFFTSource->GetSurfaceLevel(0, &pSurfUser));
	V_RETURN(m_pTexResult->GetSurfaceLevel(0, &pSurfResult));

	// ���û�ָ������ͼֱ����䵽��������ͼ��
	V_RETURN(D3DXLoadSurfaceFromSurface(pSurfSrc, NULL, NULL, pSurfUser, NULL, NULL, D3DX_FILTER_NONE, 0));



	FILE *fp = fopen("c:\\dftlog.txt", "w");

	D3DLOCKED_RECT RectSrc, RectDst;
	float *pCur = NULL, *pTemp = NULL;
	D3DXVECTOR4 fSource, fValue, fCurValue;
	float fRe = 0.0f, fIm = 0.0f;
	float fAngle = 0.0;

	pTexSrc->LockRect(0, &RectSrc, NULL, 0);
	pTexDst->LockRect(0, &RectDst, NULL, 0);

	char szLog[100];

	for(UINT iY = 0; iY < m_iHeight; iY++)
	{
		sprintf(szLog, "Y=%d:", iY);
		fwrite(szLog, strlen(szLog), 1, fp);

		for(UINT iX = 0; iX < m_iWidth; iX++)
		{
			// ��λ��ǰ���أ�ͨ������д������ͼ
			pCur = (float *)((BYTE *)RectDst.pBits + iY * RectDst.Pitch);
			pCur += iX * 4;

			// �ȳ�ʼ��ʵ�鲿��ֵ
			fValue[0] = fValue[1] = 0.0f;

			// ����õ��Ӧ��DFT������洢��fValue��
			for(UINT iY_DFT = 0; iY_DFT < m_iWidth; iY_DFT++)
			{
				pTemp = (float *)((BYTE *)RectSrc.pBits + iY_DFT * RectSrc.Pitch);

				for(UINT iX_DFT = 0; iX_DFT < m_iWidth; iX_DFT++)
				{
					// �ȵõ���ǰ���ص�ֵ
					fCurValue[0] = *pTemp++;
					fCurValue[1] = *pTemp++;
					fCurValue[2] = *pTemp++;
					fCurValue[3] = *pTemp++;

					// ���ݷ�����ʵ�鲿��ʵֵ
					if(fCurValue[2] == 0.0)
						fCurValue[0] = -fCurValue[0];
					if(fCurValue[3] == 0.0)
						fCurValue[1] = -fCurValue[1];


					// ����W���ټ���W*F�������˷���
					D3DXVECTOR2 WValue, WFValue;
					fAngle = 2.0f * D3DX_PI * (float)(iX_DFT*iX + iY_DFT*iY) / (float)m_iWidth;
					// ���任��ȡ��
					if(!bIDFT)
						fAngle = -fAngle;

					WValue.x = cosf(fAngle);
					WValue.y = sinf(fAngle);

					WFValue.x = fCurValue.x * WValue.x - fCurValue.y * WValue.y;
					WFValue.y = fCurValue.x * WValue.y + fCurValue.y * WValue.x;

					// �ֱ���ӵ�fValue��ʵ�鲿��
					fValue[0] += WFValue.x;
					fValue[1] += WFValue.y;
				}
			}// end Each Pixel 2D FFT

			// �˸���Ҷϵ��
			if(bMulFourierCoef)
			{
				float fCoef = 1.0f / (float)(m_iWidth * m_iHeight);
				fValue[0] *= fCoef;
				fValue[1] *= fCoef;
			}


			// �洢fValue��ʱ��ע���ʽ��ʵ�鲿����ֵ+���ű��
			if(fValue[0] < 0.0)				
				fValue[2] = 0.0f;
			else
				fValue[2] = 2.0f;

			if(fValue[1] < 0.0)				
				fValue[3] = 0.0f;
			else
				fValue[3] = 2.0f;

			// log��ֻ��¼��������ֵ����ȡ����
			for(UINT i = 0; i < 2; i++)
			{
				sprintf(szLog, "  %.3f,", fValue[i]);
				fwrite(szLog, strlen(szLog), 1, fp);
			}
			// д������ͼ��
			for(int i = 0; i < 4; i++)
			{
				fValue[0] = absf(fValue[0]);
				fValue[1] = absf(fValue[1]);
				*pCur++ = fValue[i];
			}

			// �����Ű�
			if(iX < m_iWidth -1)
			{
				sprintf(szLog, "\n    ");
				fwrite(szLog, strlen(szLog), 1, fp);
			}
			else
			{
				sprintf(szLog, "\n");
				fwrite(szLog, strlen(szLog), 1, fp);
			}
		}
	}
	pTexSrc->UnlockRect(0);
	pTexDst->UnlockRect(0);
	fclose(fp);

	// �ѽ��д��TexResult
	V_RETURN(D3DXLoadSurfaceFromSurface(pSurfResult, NULL, NULL, pSurfDst, NULL, NULL, D3DX_FILTER_NONE, 0));

	// ����
	SAFE_RELEASE(pSurfSrc);
	SAFE_RELEASE(pSurfDst);
	SAFE_RELEASE(pSurfUser);
	SAFE_RELEASE(pSurfResult);

	SAFE_RELEASE(pTexSrc);
	SAFE_RELEASE(pTexDst);

	m_iCreateAttrib = 2;
	return S_OK;
}












void KFourierTransform::IFFTCPUCore(complex<double> * TD, complex<double> * FD, int r)
{
	// ����Ҷ�任����
	LONG	count;

	// ѭ������
	int		i,j,k;

	// �м����
	int		bfsize,p;

	// �Ƕ�
	double	angle;

	complex<double> *W,*X1,*X2,*X;

	// ���㸶��Ҷ�任����
	count = 1 << r;

	// ������������洢��
	W  = new complex<double>[count / 2];
	X1 = new complex<double>[count];
	X2 = new complex<double>[count];

	// �����Ȩϵ��
	for(i = 0; i < count / 2; i++)
	{
		angle = (double)i * D3DX_PI * 2 / (double)count;
		W[i] = complex<double> (cos(angle), sin(angle));
	}

	// ��ʱ���д��X1
	memcpy(X1, TD, sizeof(complex<double>) * count);

	// ���õ����㷨���п��ٸ���Ҷ�任
	for(k = 0; k < r; k++)
	{
		for(j = 0; j < 1 << k; j++)
		{
			bfsize = 1 << (r-k);
			for(i = 0; i < bfsize / 2; i++)
			{
				p = j * bfsize;
				X2[i + p] = X1[i + p] + X1[i + p + bfsize / 2];
				X2[i + p + bfsize / 2] = (X1[i + p] - X1[i + p + bfsize / 2]) * W[i * (1<<k)];
			}
		}
		X  = X1;
		X1 = X2;
		X2 = X;
	}

	// ��������
	for(j = 0; j < count; j++)
	{
		p = 0;
		for(i = 0; i < r; i++)
		{
			if (j&(1<<i))
			{
				p+=1<<(r-i-1);
			}
		}
		FD[j]=X1[p];
	}

	// �ͷ��ڴ�
	delete W;
	delete X1;
	delete X2;
}














/*
// FFT--CPU
HRESULT KFourierTransform::FFTCPU(BOOL bIDFT, IDirect3DTexture9* pTexFFTSource)
{
FILE *fp = fopen("c:\\fftcpulog.txt", "w");

D3DLOCKED_RECT Rect;
float *p = NULL;
float16 *p16 = NULL;
D3DXVECTOR4 fValue;
float fRe = 0.0f, fIm = 0.0f;

g_pSourceTex->LockRect(0, &Rect, NULL, 0);

char szLog[100];
complex<double> *pSourceData = NULL, *pResult = NULL;
UINT iLitNum = CheckPowerOf2(m_iWidth);	// ��������

for(UINT iY = 0; iY < m_iHeight; iY++)
{
sprintf(szLog, "Y=%d:", iY);
fwrite(szLog, strlen(szLog), 1, fp);

p = (float *)((BYTE *)Rect.pBits + iY * Rect.Pitch);

// ��ȡһ�У���Դ�������뺯������CPU��IFFT
pSourceData = new complex<double>[m_iWidth];
pResult = new complex<double>[m_iWidth];

for(UINT iX = 0; iX < m_iWidth; iX++)
{
fValue = *(LPD3DXVECTOR4)p;
p += 4;
pSourceData[iX]._Val[0] = fValue[0];
pSourceData[iX]._Val[1] = fValue[1];
}

IFFTCPUCore(pSourceData, pResult, iLitNum);

for(iX = 0; iX < m_iWidth; iX++)
{
// ��Result��ֵ��fValue���Ա�д��log
fValue[0] = (float)pResult[iX].real();
fValue[1] = (float)pResult[iX].imag();

// �洢fValue��ʱ��ע���ʽ��ʵ�鲿����ֵ+���ű��
if(fValue[0] < 0.0)				
fValue[2] = 0.0f;
else
fValue[2] = 2.0f;

if(fValue[1] < 0.0)				
fValue[3] = 0.0f;
else
fValue[3] = 2.0f;

//fValue[0] = absf(fValue[0]);
//fValue[1] = absf(fValue[1]);


// log
for(UINT i = 0; i < 4; i++)
{
sprintf(szLog, "  %.3f,", fValue[i]);
fwrite(szLog, strlen(szLog), 1, fp);
}

// �����Ű�
if(iX < m_iWidth -1)
{
sprintf(szLog, "\n    ");
fwrite(szLog, strlen(szLog), 1, fp);
}
else
{
sprintf(szLog, "\n");
fwrite(szLog, strlen(szLog), 1, fp);
}
}
}
g_pSourceTex->UnlockRect(0);
fclose(fp);

// ����
m_iCreateAttrib = 2;
return S_OK;
}
*/




























/************************2D Fluid Dynamics Simulation*************************/
/////////�ⲿ�ӿ�
KFluidSimulation2D::KFluidSimulation2D()
{
	m_iCreateAttrib = 0;
	m_iWidth = m_iHeight = 0;
	m_bGenerateNormal = m_bABC = FALSE;
	m_iStride = 0;
	m_fDeltaTime = 0;
	m_iRenderNumPerFrame = 0;

	m_iInjectForceNum = m_iInjectSourceNum = 0;
	m_pInjectForce = m_pInjectSource = NULL;

	m_pVBQuad = m_pVBQuadExceptBoundary = m_pVBBoundaryLine[0] = m_pVBBoundaryLine[1] = m_pVBBoundaryLine[2] = m_pVBBoundaryLine[3] = NULL;

	m_pDepthBuffer = NULL;

	for(UINT i = 0; i < MAX_FLUID_STEPS; i++)
	{
		m_pTexWTemp[i] = m_pTexDTemp[i] = NULL;
	}
	m_iCurrentWIndex = m_iCurrentDIndex = 0;

	m_pTexTemp[0] = m_pTexTemp[1] = NULL;
	m_pTexBCTemp = NULL;
	m_pTexP = m_pTexU = m_pTexD = NULL;

	m_pTexABCBoundary = m_pTexABCBoundaryToOffset = m_pTexABCTypeToOffset = m_pTexABCType = m_pTexABCOffset = NULL;
	m_pTexDivW = m_pTexCurlW = NULL;
	m_pTexNormal = NULL;
}

void KFluidSimulation2D::Release()
{
	m_iCreateAttrib = 0;
	m_iWidth = m_iHeight = 0;
	m_bGenerateNormal = m_bABC = FALSE;
	m_iStride = 0;
	m_fDeltaTime = 0;
	m_iRenderNumPerFrame = 0;

	// Inject
	m_iInjectForceNum = m_iInjectSourceNum = 0;
	SAFE_DELETE_ARRAY(m_pInjectForce);
	SAFE_DELETE_ARRAY(m_pInjectSource);

	// VB
	SAFE_RELEASE(m_pVBQuad);
	SAFE_RELEASE(m_pVBQuadExceptBoundary);
	for(UINT i = 0; i < 4; i++)
		SAFE_RELEASE(m_pVBBoundaryLine[i]);
    
	SAFE_RELEASE(m_pDeclaration);
	SAFE_RELEASE(m_pDepthBuffer);

	// Texture
	for(int i = 0; i < 2; i++)
	{
		SAFE_RELEASE(m_pTexTemp[i]);
	}
	SAFE_RELEASE(m_pTexBCTemp);

	for(int i = 0; i < MAX_FLUID_STEPS; i++)
	{
		SAFE_RELEASE(m_pTexWTemp[i]);
		SAFE_RELEASE(m_pTexDTemp[i]);
	}
	m_iCurrentWIndex = m_iCurrentDIndex = 0;

	SAFE_RELEASE(m_pTexP);
	SAFE_RELEASE(m_pTexD);
	SAFE_RELEASE(m_pTexU);

	SAFE_RELEASE(m_pTexABCTypeToOffset);
	SAFE_RELEASE(m_pTexABCBoundaryToOffset);
	SAFE_RELEASE(m_pTexABCBoundary);
	SAFE_RELEASE(m_pTexABCType);
	SAFE_RELEASE(m_pTexABCOffset);

	SAFE_RELEASE(m_pTexDivW);
	SAFE_RELEASE(m_pTexCurlW);
	SAFE_RELEASE(m_pTexNormal);

	// PS
	m_PSClearTexture.Release();
	m_PSCopy.Release();
	m_PSGenerateNormal.Release();

	m_PSAddForceSphere.Release();
	m_PSAddForceTexture.Release();
	m_PSAddForceFire.Release();
	m_PSAddSourceSphere.Release();
	m_PSAddSourceTexture.Release();
	m_PSAddSourceFire.Release();

	m_PSAdvect_W.Release();
	m_PSAdvect_D.Release();
	m_PSGravity.Release();
	m_PSDiv_W.Release();
	m_PSProject.Release();
	m_PSJacobi_W.Release();
	m_PSJacobi_P.Release();
	m_PSJacobi_D.Release();
	m_PSCurl_W.Release();
	m_PSVorticity.Release();

	m_PSBBC_U.Release();
	m_PSBBC_P.Release();
	m_PSABC_U.Release();
	m_PSABC_P.Release();
	m_PSABC_D.Release();

	m_PSObstacleToBoundary.Release();
	m_PSBoundaryToType.Release();
	m_PSTypeToOffset.Release();
	m_PSBoundaryToOffset.Release();
}





HRESULT KFluidSimulation2D::SetForce(UINT iForceNum, INJECT_ATTRIBUTE* pInjectData)
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;
	if(!iForceNum || !pInjectData)
		return D3DERR_INVALIDCALL;

	// ��Ч�Լ��
	for(UINT i = 0; i < iForceNum; i++)
	{
		if(!pInjectData[i].CheckAvailable(m_iWidth, m_iHeight))
			return D3DERR_INVALIDCALL;
		if(absf(D3DXVec2Length(&pInjectData[i].VecForceDir) - 1.0f) < 0.00001f)
		{
			OutputDebugString("����FΪ0���Ƿ���\n");
			return FALSE;
		}
	}

	// �ͷ�ԭ���ģ������µ�
	SAFE_DELETE_ARRAY(m_pInjectForce);
	m_pInjectForce = new INJECT_ATTRIBUTE[iForceNum];
	if(!m_pInjectForce)
		return E_OUTOFMEMORY;

	
	// ��������
	m_iInjectForceNum = iForceNum;
	memcpy(m_pInjectForce, pInjectData, m_iInjectForceNum * sizeof(INJECT_ATTRIBUTE));
	return S_OK;
}

HRESULT KFluidSimulation2D::SetSource(UINT iSourceNum, INJECT_ATTRIBUTE* pInjectData)
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;
	if(!iSourceNum || !pInjectData)
		return D3DERR_INVALIDCALL;

	// ��Ч�Լ��
	for(UINT i = 0; i < iSourceNum; i++)
	{
		if(!pInjectData[i].CheckAvailable(m_iWidth, m_iHeight))
			return D3DERR_INVALIDCALL;
		if(pInjectData[i].VecCentreIntensity.x < 0.0f || pInjectData[i].VecCentreIntensity.y < 0.0f || pInjectData[i].VecCentreIntensity.z < 0.0f)
		{
			OutputDebugString("��ԴSΪ�������Ƿ���\n");
			return FALSE;
		}
		/*��ʱ����ϴ����Դ
		if(pInjectData[i].VecCentreIntensity.x > 1.0f || pInjectData[i].VecCentreIntensity.y > 1.0f || pInjectData[i].VecCentreIntensity.z > 1.0f)
		{
			OutputDebugString("��ԴS���󣡷Ƿ���\n");
			return FALSE;
		}
		*/
	}

	// �ͷ�ԭ���ģ������µ�
	SAFE_DELETE_ARRAY(m_pInjectSource);
	m_pInjectSource = new INJECT_ATTRIBUTE[iSourceNum];
	if(!m_pInjectSource)
		return E_OUTOFMEMORY;

	// ��������
	m_iInjectSourceNum = iSourceNum;
	memcpy(m_pInjectSource, pInjectData, m_iInjectSourceNum * sizeof(INJECT_ATTRIBUTE));
	return S_OK;
}



HRESULT KFluidSimulation2D::ResetFluid()
{
	if(!m_pTexD || !m_pTexU || !m_pTexP)
		return D3DERR_NOTAVAILABLE;

	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա�FFT��Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// Clear
	V_RETURN(ClearTexture(m_pTexP));
	V_RETURN(ClearTexture(m_pTexD));
	V_RETURN(ClearTexture(m_pTexU));

	// �ָ�RT����Ȼ���
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);



	return S_OK;
}



HRESULT KFluidSimulation2D::SetObstacleTexture(IDirect3DTexture9* pTexObstacle, IDirect3DTexture9* pTexOffset /* = NULL */)
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// ��ֹÿ֡����������ͬ��Obstacleͼ�����µ������½�
	static IDirect3DTexture9* s_pTexture = NULL;

	if(!pTexObstacle)
	{
		s_pTexture = NULL;
		m_bABC = FALSE;
		return S_OK;
	}

	// ��֧��GPU����ObstacleTexture��������Ҫ76��ָ�������û���ֶ�����Offsetͼ��ʧ��
	if(!CheckPS2xSupport(12, FALSE) && !pTexOffset)
		return D3DERR_NOTAVAILABLE;


	// �赲ͼ�ֱ��ʱ��������һ�£�
	D3DSURFACE_DESC Desc;
	V_RETURN(pTexObstacle->GetLevelDesc(0, &Desc));
	if(Desc.Width != m_iWidth || Desc.Height != m_iHeight)
		return D3DERR_WRONGTEXTUREFORMAT;


	// ��ֹÿ֡����������ͬ��Obstacleͼ�����µ������½�
	if(s_pTexture == pTexObstacle)
		return S_OK;
	else
		s_pTexture = pTexObstacle;


	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա�FFT��Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// ��ʼ����

	// ��ת��Ϊ�߽�����ͼ������ComputeQuad�������߽��ߣ�����������Clearһ�£���Ϊ��ͨ�����ͣ�ֵ1��
	V_RETURN(ClearTexture(m_pTexABCBoundary, &D3DXVECTOR4(1, 1, 1, 1)));
	V_RETURN(CommonComputeQuad(pTexObstacle, NULL, NULL, m_pTexABCBoundary, &m_PSObstacleToBoundary));
	
/*	// ����1���߽�ͼ����������ͼ��Ȼ����ת��Ϊƫ������ͼ����Ҫ����pass��ps2.a
	// ��ת��Ϊ��������ͼ������ComputeQuad�������߽��ߣ�����������Clearһ�£���Ϊ��ƫ�����ͣ�ֵ0��
	V_RETURN(ClearTexture(m_pTexABCType, &D3DXVECTOR4(0, 0, 0, 0)));
	V_RETURN(CommonComputeQuad(m_pTexABCBoundary, NULL, NULL, m_pTexABCType, &m_PSBoundaryToType));

	V_RETURN(ClearTexture(m_pTexABCOffset, &D3DXVECTOR4(0.5f, 0.5f, 0.5f, 0.5f)));
	V_RETURN(CommonComputeQuad(m_pTexABCType, m_pTexABCTypeToOffset, NULL, m_pTexABCOffset, &m_PSTypeToOffset));
*/

	// ����2���߽�ͼֱ��ת��ΪABCƫ������ͼ��ע��ƫ������Ҫ�˼�0.5������Ҫ��Ϊ0.5
	V_RETURN(ClearTexture(m_pTexABCOffset, &D3DXVECTOR4(0.5f, 0.5f, 0.5f, 0.5f)));
		// Ӳ��֧�ֵĻ�����GPU�����㣬��֮�͸����ֶ��趨��Offsetͼ
	if(CheckPS2xSupport(12, FALSE))
	{
		V_RETURN(CommonComputeQuad(m_pTexABCBoundary, m_pTexABCBoundaryToOffset, NULL, m_pTexABCOffset, &m_PSBoundaryToOffset));
	}
	else
	{
		V_RETURN(CopyTexture(pTexOffset, m_pTexABCOffset));
	}


	// ����GPU���ɵ�Offsetͼ
//	V_RETURN(D3DXSaveTextureToFile("Offset.dds", D3DXIFF_DDS, m_pTexABCOffset, NULL));



	// �ָ�RT����Ȼ���
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);



	// ���ñ�ǣ�����
	m_bABC = TRUE;
	return S_OK;
}





HRESULT KFluidSimulation2D::FluidSimulation(FLUID_ATTRIBUTE FluidData, float fDeltaTime)
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;
	if(!FluidData.CheckAvailable())
		return D3DERR_INVALIDCALL;
	
	m_FluidData = FluidData;
	m_fDeltaTime = fDeltaTime;

	// Һ���Ҫ���㷨��
	if(m_FluidData.bLiquid)
		m_bGenerateNormal = TRUE;
	else
		m_bGenerateNormal = FALSE;

	// ÿ�ε���ģ��֮ǰ����Ⱦ��������
	m_iRenderNumPerFrame = 0;


	UINT i = 0;

	// �������㣬��ʼÿһ������
	m_iCurrentDIndex = m_iCurrentWIndex = 0;

	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա�FFT��Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// ��ʼ����
	// ������ģ������ж��ǲ�����߽��ߵģ����ǵ��ٽ������Ĳ��裬����Ҫ�ڼ���ǰApply�߽������������Ų���������ֵ���������������ٽ���������ɢ��ȥ
/*	for(i = 0; i < MAX_FLUID_STEPS; i++)
	{
		V_RETURN(ClearTexture(m_pTexWTemp[i]));
		V_RETURN(ClearTexture(m_pTexDTemp[i]));
	}
*/
	// �ȴ����ٶȳ�W
		// ��һ����ƽ��������ǰU���㵽W��ͼ�У���ƽ��ǣ���ٽ�����������Ϊƽ��ֻ�����һ�Σ�û�е���������ֻ��ǰ����һ�α߽���������
	V_RETURN(ApplyBoundaryCondition(FLUID_VELOCITY, m_pTexU));
	V_RETURN(Advect(FLUID_VELOCITY));

		// �ڶ�������������ɢ����ѡ������W���㵽W�������ǵ������̣������ں����ڸ��±߽�����
	if(m_FluidData.fViscousCoef > 0.000001f)
	{
//		V_RETURN(Diffusion(FLUID_W));
	}

		// ������������������ֻ�ڱ߽������ӣ��������ٽ������޹أ���ʱ�����±߽�����
	//	V_RETURN(ClearTexture(m_pTexTemp[0]));
	//	V_RETURN(ClearTexture(m_pTexTemp[1]));
	V_RETURN(ApplyForce());

		// ���Ĳ�������������������С���ܶ���أ����ǳ������еģ�������ע������һ��������
	V_RETURN(ApplyGravity());

		// �ٶȳ�W������ϣ����浽m_pTexWTemp[m_iCurrentWIndex]��

	// ��������W������ѹ����
	//	V_RETURN(ClearTexture(m_pTexTemp[0]));
	//	V_RETURN(ClearTexture(m_pTexTemp[1]));

		// ��Wɢ�ȼ�����ѹ�����̣���Wɢ����Ҫ���ٽ�����������Ҫ��ǰ���±߽�����
	V_RETURN(ApplyBoundaryCondition(FLUID_W, m_pTexWTemp[m_iCurrentWIndex]));
	V_RETURN(SolvePoissonPressureEquation());

		// ѹ����P������ϣ����浽m_pTexP��

	// ���ڼ����ٶȳ�U
		// ǰ���������ٶȳ�W�����ڽ���ͶӰ����m_pTexU�б���������׼���ú���������ã�ͬʱҲ��Ϊ��һ֡ģ�������
		// ����ͶӰ��Ҫ�ٽ�����������P���ݶ�ʱ��������Ҫ��ǰ��P���±߽�����
	V_RETURN(ApplyBoundaryCondition(FLUID_PRESSURE, m_pTexP));
	V_RETURN(Project());
		
		// �ٶȳ�U������ϣ����浽m_pTexU��
	


	// ������ܶȳ�
		// ��һ����ƽ��������ǰD���㵽DTemp�У���ƽ��ǣ���ٽ�����������Ϊƽ��ֻ�����һ�Σ�û�е���������ֻ��ǰ����һ�α߽���������
	V_RETURN(ApplyBoundaryCondition(FLUID_DENSITY, m_pTexD));
	V_RETURN(Advect(FLUID_DENSITY));
		// �ڶ�������������ɢ������ǰ���ٶȳ��Ѿ������ˣ����������
	if(m_FluidData.fViscousCoef > 0.000001f)
	{
//		V_RETURN(Diffusion(FLUID_DENSITY));
	}

		// ��������������Դ��ֻ�ڱ߽������ӣ��������ٽ������޹أ���ʱ�����±߽�����
	V_RETURN(ApplySource());

		// ������������ܶȳ����浽��m_pTexDTemp[m_iCurrentDIndex]�У����ڽ���������m_pTexD�б���������׼���������ã�ͬʱҲ��Ϊ��һ֡ģ�������
		// ��һ��Ҳû���ٽ����������ø��±߽�����
	V_RETURN(CopyTexture(m_pTexDTemp[m_iCurrentDIndex], m_pTexD));


	// Һ�巨��
	if(m_bGenerateNormal)
	{
		V_RETURN(GenerateNormal());
	}




	// �ָ�RT����Ȼ���
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);


	// ���в������
	m_iCreateAttrib = 2;
	return S_OK;
}



/****************************�ڲ��ӿ�--ģ�����**********************************/
HRESULT KFluidSimulation2D::Advect(enuFluid_VectorField Type)
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	if(Type == FLUID_VELOCITY)
	{
		// A��ģ����̵ĵ�һ����ͨ����ǰ�ģ���ʵ����һ�ε�ģ������U�����㵽W[0]��
		float fDamp = 1.0f;	// �ٶȳ����ܼ���ɢϵ�������Ա���Ϊ1
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(fDamp, fDamp, fDamp, fDamp), 1);
		V_RETURN(CommonComputeQuad(m_pTexU, m_pTexU, NULL, m_pTexWTemp[m_iCurrentWIndex], &m_PSAdvect_W));
	}
	else if(Type == FLUID_DENSITY)
	{
		// ͨ����ǰD���Ѿ�����õ�U��������µ�D
		float fDamp = m_FluidData.fDensityDampCoef;
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(fDamp, fDamp, fDamp, fDamp), 1);
		V_RETURN(CommonComputeQuad(m_pTexD, m_pTexU, NULL, m_pTexDTemp[m_iCurrentDIndex], &m_PSAdvect_D));
	}
	else
		return D3DERR_INVALIDCALL;

	return S_OK;
}



HRESULT KFluidSimulation2D::ApplyGravity()
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;
	if(D3DXVec2Length(&m_FluidData.VecGravity) > 0.0f)
	{
		// �����ܶȳ����������������
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_FluidData.VecGravity.x, m_FluidData.VecGravity.y, 0, 0), 1);
		d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(m_FluidData.fGravityPower, m_FluidData.fGravityPower, m_FluidData.fGravityPower, m_FluidData.fGravityPower), 1);
		V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], m_pTexD, NULL, m_pTexWTemp[m_iCurrentWIndex+1], &m_PSGravity));
		m_iCurrentWIndex++;
	}

	// ����
	return S_OK;
}




HRESULT KFluidSimulation2D::Diffusion(enuFluid_VectorField Type)
{
#define SET_CONSTANT \
	d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(fOneByV, fOneByV, fOneByV, fOneByV), 1);\
	d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(fOneByDenominator, fOneByDenominator, fOneByDenominator, fOneByDenominator), 1);\

	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;
	
	if(m_FluidData.fViscousCoef < 0.000001f)
		return D3DERR_INVALIDCALL;

	float fOneByV = 1.0f / m_FluidData.fViscousCoef;
	float fOneByDenominator = 1.0f / (1.0f / m_FluidData.fViscousCoef + 4);

	if(Type == FLUID_W)
	{
		// ����W�����µ�W������һ��Jacobi��������
		UINT iTempIndex = 0;

		// ��һ��
		V_RETURN(ApplyBoundaryCondition(FLUID_W, m_pTexWTemp[m_iCurrentWIndex]));
		SET_CONSTANT;
		V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], m_pTexU, NULL, m_pTexTemp[iTempIndex], &m_PSJacobi_W));

		for(UINT i = 1; i < m_FluidData.iIterateNum; i++)
		{
			// ÿ�ε���ǰҪ���±߽�����
			V_RETURN(ApplyBoundaryCondition(FLUID_W, m_pTexTemp[iTempIndex]));

			// ���һ�Σ������л������㵽��W��
			if(i == (m_FluidData.iIterateNum - 1) )
			{
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], m_pTexU, NULL, m_pTexWTemp[m_iCurrentWIndex+1], &m_PSJacobi_W));
			}
			// �м���̣���Temp�������л�
			else
			{
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], m_pTexU, NULL, m_pTexTemp[!iTempIndex], &m_PSJacobi_W));
				iTempIndex = !iTempIndex;
			}		
		}

		m_iCurrentWIndex++;
	}
	else if(Type == FLUID_DENSITY)
	{
		// ����W�͵�ǰ��D�����µ�D������һ��Jacobi��������
		UINT iTempIndex = 0;

		// ��һ��
		V_RETURN(ApplyBoundaryCondition(FLUID_DENSITY, m_pTexDTemp[m_iCurrentDIndex]));
		SET_CONSTANT;
		V_RETURN(CommonComputeQuad(m_pTexDTemp[m_iCurrentDIndex], NULL, NULL, m_pTexTemp[iTempIndex], &m_PSJacobi_D));

		for(UINT i = 1; i < m_FluidData.iIterateNum; i++)
		{
			// ÿ�ε���ǰҪ���±߽�����
			V_RETURN(ApplyBoundaryCondition(FLUID_DENSITY, m_pTexTemp[iTempIndex]));

			// ���һ�Σ������л������㵽��D��
			if(i == (m_FluidData.iIterateNum - 1) )
			{
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], NULL, NULL, m_pTexDTemp[m_iCurrentDIndex+1], &m_PSJacobi_D));
			}
			// �м���̣���Temp�������л�
			else
			{
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], NULL, NULL, m_pTexTemp[!iTempIndex], &m_PSJacobi_D));
				iTempIndex = !iTempIndex;
			}	
		}

		m_iCurrentDIndex++;
	}
	else return D3DERR_INVALIDCALL;

	// ����
	return S_OK;

#undef SET_CONSTANT
}







PIXELSHADER *KFluidSimulation2D::ChooseForceShader(enuInject_Type Type)
{
	PIXELSHADER *pPS = NULL;
	// ѡ��Shader
	switch(Type)
	{
	case Inject_Sphere:
		pPS = &m_PSAddForceSphere;
		break;
	case Inject_Texture:
		pPS = &m_PSAddForceTexture;
		break;
	case Inject_Fire:
		pPS = &m_PSAddForceFire;
		break;
	}
	return pPS;
}

PIXELSHADER *KFluidSimulation2D::ChooseSourceShader(enuInject_Type Type)
{
	PIXELSHADER *pPS = NULL;
	// ѡ��Shader
	switch(Type)
	{
	case Inject_Sphere:
		pPS = &m_PSAddSourceSphere;
		break;
	case Inject_Texture:
		pPS = &m_PSAddSourceTexture;
		break;
	case Inject_Fire:
		pPS = &m_PSAddSourceFire;
		break;
	}
	return pPS;
}


HRESULT KFluidSimulation2D::ApplyForce()
{
#define SET_CONSTANT \
	if(m_pInjectForce[i].Type == Inject_Texture)\
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_pInjectForce[i].fTexInjectForceCoef, m_pInjectForce[i].fTexInjectForceCoef, m_pInjectForce[i].fTexInjectForceCoef, m_pInjectForce[i].fTexInjectForceCoef), 1);\
	else\
	{\
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_pInjectForce[i].PtPos.x / (float)m_iWidth, m_pInjectForce[i].PtPos.y / (float)m_iHeight, 0, 0), 1);\
		fTemp = (float)m_iWidth / m_pInjectForce[i].iRange;\
		d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(fTemp, fTemp, fTemp, fTemp), 1);\
		d3ddevice->SetPixelShaderConstantF(7, (float *)&D3DXVECTOR4(m_pInjectForce[i].VecForceDir.x, m_pInjectForce[i].VecForceDir.y, 0, 0), 1);\
	}\

	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;
	if(!m_iInjectForceNum || !m_pInjectForce)
		return S_OK;


	// ����m_pTexWTemp[m_iCurrentWIndex]�����m_pTexWTemp[m_iCurrentWIndex+1]���м���TexTemp��ѭ����Ⱦ

	UINT iTempIndex = 0;
	float fTemp = 0.0f;
	UINT i = 0;


	// ���������ע��������Ҫ������Ӧ������
	IDirect3DTexture9* pTexInject = NULL;
	if(m_pInjectForce[i].Type == Inject_Texture)
		pTexInject = m_pInjectForce[i].pTexInjectForce;


		// ���ֻ��һ������ֱ�����
	if(m_iInjectForceNum == 1)
	{
		SET_CONSTANT;
		V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], pTexInject, NULL, m_pTexWTemp[m_iCurrentWIndex+1], ChooseForceShader(m_pInjectForce[i].Type)));
	}
	else
	{
		// ����жദ������һ���ȴ�WTemp�����Temp�У�Ȼ��ÿ�η��������������һ����ʱ�������WTemp
			// ��һ��
		SET_CONSTANT;
		V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], pTexInject, NULL, m_pTexTemp[iTempIndex], ChooseForceShader(m_pInjectForce[i].Type)));

		for(i = 1; i < m_iInjectForceNum; i++)
		{
			// ���һ��
			if(i == (m_iInjectForceNum -1) )
			{
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], pTexInject, NULL, m_pTexWTemp[m_iCurrentWIndex+1], ChooseForceShader(m_pInjectForce[i].Type)));
			}
			// �м���̣���Temp�������л�
			else
			{
				// D��ģ����̵ĵڶ�����ͨ����һ��W�ļ����������㵽�µ�W��
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], pTexInject, NULL, m_pTexTemp[!iTempIndex], ChooseForceShader(m_pInjectForce[i].Type)));
				iTempIndex = !iTempIndex;
			}
		}

	}

	m_iCurrentWIndex++;

	// �������ǵ����˲ʱ����
	m_iInjectForceNum = 0;
	SAFE_DELETE_ARRAY(m_pInjectForce);

	return S_OK;
#undef SET_CONSTANT
}




HRESULT KFluidSimulation2D::ApplySource()
{
#define SET_CONSTANT \
	if(m_pInjectSource[i].Type == Inject_Texture)\
	{\
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_pInjectSource[i].fTexInjectSourceCoef, m_pInjectSource[i].fTexInjectSourceCoef, m_pInjectSource[i].fTexInjectSourceCoef, m_pInjectSource[i].fTexInjectSourceCoef), 1);\
	}\
	else\
	{\
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_pInjectSource[i].PtPos.x / (float)m_iWidth, m_pInjectSource[i].PtPos.y / (float)m_iHeight, 0, 0), 1);\
		fTemp = (float)m_iWidth / m_pInjectSource[i].iRange;\
		d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(fTemp, fTemp, fTemp, fTemp), 1);\
		d3ddevice->SetPixelShaderConstantF(7, (float *)&D3DXVECTOR4(m_pInjectSource[i].VecCentreIntensity.x, m_pInjectSource[i].VecCentreIntensity.y, m_pInjectSource[i].VecCentreIntensity.z, 0), 1);\
	}\



	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;
	if(!m_iInjectSourceNum || !m_pInjectSource)
		return S_OK;


	// ����m_pTexDTemp[m_iCurrentDIndex]�����m_pTexDTemp[m_iCurrentDIndex+1]���м���TexTemp��ѭ����Ⱦ
	
	UINT iTempIndex = 0;
	float fTemp = 0.0f;
	UINT i = 0;

	// ���������ע��������Ҫ������Ӧ������
	IDirect3DTexture9* pTexInject = NULL;
	if(m_pInjectSource[i].Type == Inject_Texture)
		pTexInject = m_pInjectSource[i].pTexInjectSource;


	// ���ֻ��һ����Դ��ֱ�����
	if(m_iInjectSourceNum == 1)
	{
		SET_CONSTANT;
		V_RETURN(CommonComputeQuad(m_pTexDTemp[m_iCurrentDIndex], pTexInject, NULL, m_pTexDTemp[m_iCurrentDIndex+1], ChooseSourceShader(m_pInjectSource[i].Type)));
	}
	else
	{
		// ����жദ��Դ����һ���ȴ�DTemp�����Temp�У�Ȼ��ÿ�η��������������һ����Դʱ�������DTemp
		// ��һ��
		SET_CONSTANT;
		V_RETURN(CommonComputeQuad(m_pTexDTemp[m_iCurrentDIndex], pTexInject, NULL, m_pTexTemp[iTempIndex], ChooseSourceShader(m_pInjectSource[i].Type)));

		for(i = 1; i < m_iInjectSourceNum; i++)
		{
			// ���һ��
			if(i == (m_iInjectSourceNum - 1) )
			{
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], pTexInject, NULL, m_pTexDTemp[m_iCurrentDIndex+1], ChooseSourceShader(m_pInjectSource[i].Type)));
			}
			// �м���̣���Temp�������л�
			else
			{
				// S��ģ����̵ĵ�������ͨ����һ��D�ļ����������㵽�µ�D��
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], pTexInject, NULL, m_pTexTemp[!iTempIndex], ChooseSourceShader(m_pInjectSource[i].Type)));
				iTempIndex = !iTempIndex;
			}	
		}

	}

	m_iCurrentDIndex++;

	// �������ǵ����˲ʱ����
	m_iInjectSourceNum = 0;
	SAFE_DELETE_ARRAY(m_pInjectSource);
	return S_OK;
#undef SET_CONSTANT
}









HRESULT KFluidSimulation2D::SolvePoissonPressureEquation()
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// ����P���ȼ���W��ɢ�ȣ�W��ʱ��������Ѹ�����߽�������
	V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], NULL, NULL, m_pTexDivW, &m_PSDiv_W));
	
	// ����ɢ�ȣ�����һ��ģ���м��������P�������µ�P������һ��Jacobi�������̣����ڵ����л�ʹ���ٽ��������ݶȣ�������ÿ�ε���ǰ��Ҫ���±߽�����
	UINT iTempIndex = 0;
	
		// ��һ�ε���
	V_RETURN(ApplyBoundaryCondition(FLUID_PRESSURE, m_pTexP));
	V_RETURN(CommonComputeQuad(m_pTexP, m_pTexDivW, NULL, m_pTexTemp[iTempIndex], &m_PSJacobi_P));



	for(UINT i = 1; i < m_FluidData.iIterateNum; i++)
	{
		// ÿ�ε���ǰҪ���±߽�����
		V_RETURN(ApplyBoundaryCondition(FLUID_PRESSURE, m_pTexTemp[iTempIndex]));

		// ���һ�Σ������л������㵽P��
		if(i == (m_FluidData.iIterateNum - 1) )
		{
			V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], m_pTexDivW, NULL, m_pTexP, &m_PSJacobi_P));
		}
		// �м���̣���Temp�������л�
		else
		{
			V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], m_pTexDivW, NULL, m_pTexTemp[!iTempIndex], &m_PSJacobi_P));
			iTempIndex = !iTempIndex;
		}	
	}

	return S_OK;
}




HRESULT KFluidSimulation2D::Project()
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// ����W��P����U
	d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(1.0f / m_FluidData.fConsistensy, 1.0f / m_FluidData.fConsistensy, 1.0f / m_FluidData.fConsistensy, 1.0f / m_FluidData.fConsistensy), 1);
	V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], m_pTexP, NULL, m_pTexU, &m_PSProject));
	return S_OK;
}



HRESULT KFluidSimulation2D::RefineVorticity()
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// �ȼ���CurlW
	V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], NULL, NULL, m_pTexCurlW, &m_PSCurl_W));
	// ����CurlW��W�����µ�W
	V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], m_pTexCurlW, NULL, m_pTexWTemp[m_iCurrentWIndex+1], &m_PSVorticity));
	m_iCurrentWIndex++;

	return S_OK;
}





HRESULT KFluidSimulation2D::ApplyBoundaryCondition(enuFluid_VectorField Type, IDirect3DTexture9* pTexture)
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	UINT iBoundaryIndex = 0;

	// ע������ҪClearһ�£����������Copy�����������������߽��ߣ��ڱ߽紦���л�������ٽ��㣬���Ի���Ҫ�ѱ߽�㴦��ֵ��Ϊ0
	V_RETURN(ClearTexture(m_pTexBCTemp));

	// �����Ҫ�Ļ����ȼ�������߽磨����߽���������߽����������Ȼ��ƣ�����TexU��TexU
	if(m_bABC)
	{
		// �ȸ��Ƶ���ʱ��ͼ�д���
		V_RETURN(CopyTexture(pTexture, m_pTexBCTemp));

		if(Type == FLUID_VELOCITY)
		{
			// ��ǰU��Offsetһ�����Ϊ�µ�U
			V_RETURN(CommonComputeQuad(m_pTexBCTemp, m_pTexABCBoundary, m_pTexABCOffset, pTexture, &m_PSABC_U));
		}
		else if(Type == FLUID_W)
		{
			// ��ǰW��Offsetһ�����Ϊ�µ�W
			V_RETURN(CommonComputeQuad(m_pTexBCTemp, m_pTexABCBoundary, m_pTexABCOffset, pTexture, &m_PSABC_U));
		}
		else if(Type == FLUID_PRESSURE)
		{
			// ��ǰP��Offsetһ�����Ϊ�µ�P
			V_RETURN(CommonComputeQuad(m_pTexBCTemp, m_pTexABCBoundary, m_pTexABCOffset, pTexture, &m_PSABC_P));
		}
		else if(Type == FLUID_DENSITY)
		{
			// ��ǰP���ϰ�ͼOffsetһ�����Ϊ�µ�D
			V_RETURN(CommonComputeQuad(m_pTexBCTemp, m_pTexABCBoundary, NULL, pTexture, &m_PSABC_D));
		}
	}


	// �ټ�������߽磬��������ֻ��Ҫд��4���߽��ߣ����Բ���Ҫ����Copy

	// ƫ�Ʋ����ˣ�0��3�ֱ��ʾ�������ұ߽����ϵ�ƫ�ƣ��������ã�
	D3DXVECTOR4 VecBBCOffset[4] = 
	{
		D3DXVECTOR4(0, 1, 0, 0),	// �ϱ߽�����Ѱַ
		D3DXVECTOR4(0, -1, 0, 0),	// �±߽�����Ѱַ
		D3DXVECTOR4(1, 0, 0, 0),	// ��߽�����Ѱַ
		D3DXVECTOR4(-1, 0, 0, 0),	// �ұ߽�����Ѱַ
	};

	// �ȸ��Ƶ���ʱ��ͼ�д��ã����ҪDisappear�������Ͱѳ����Ĵ���6��Ϊ0
	V_RETURN(CopyTexture(pTexture, m_pTexBCTemp));

	if(Type == FLUID_VELOCITY)
	{
		for(iBoundaryIndex = 0; iBoundaryIndex < 4; iBoundaryIndex++)
		{
			// ��ǰU����Ϊ�µ�U
			d3ddevice->SetPixelShaderConstantF(5, (float *)&VecBBCOffset[iBoundaryIndex], 1);
			d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(-1, -1, -1, -1), 1);	// No-Slip�ٶȳ��߽��ٶȱ�������0��Scale��-1���������ʧ�������͸�0
			V_RETURN(CommonComputeLine(m_pTexBCTemp, NULL, NULL, pTexture, &m_PSBBC_U, iBoundaryIndex));
		}
	}
	else if(Type == FLUID_W)
	{
		for(iBoundaryIndex = 0; iBoundaryIndex < 4; iBoundaryIndex++)
		{
			// ��ǰW����Ϊ�µ�W
			d3ddevice->SetPixelShaderConstantF(5, (float *)&VecBBCOffset[iBoundaryIndex], 1);
			d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(-1, -1, -1, -1), 1);	// No-Slip�ٶȳ��߽��ٶȱ�������0��Scale��-1���������ʧ�������͸�0
			V_RETURN(CommonComputeLine(m_pTexBCTemp, NULL, NULL, pTexture, &m_PSBBC_U, iBoundaryIndex));
		}
	}
	else if(Type == FLUID_PRESSURE)
	{
		for(iBoundaryIndex = 0; iBoundaryIndex < 4; iBoundaryIndex++)
		{
			// ��ǰP����Ϊ�µ�P
			d3ddevice->SetPixelShaderConstantF(5, (float *)&VecBBCOffset[iBoundaryIndex], 1);
			d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(1, 1, 1, 1), 1);	// ѹ�����߽�ѹ���仯��Ϊ0��Ҳ����˵Ҫ����סScale��1���������ʧ�������͸�0
			V_RETURN(CommonComputeLine(m_pTexBCTemp, NULL, NULL, pTexture, &m_PSBBC_P, iBoundaryIndex));
		}
	}
	else if(Type == FLUID_DENSITY)
	{
		for(iBoundaryIndex = 0; iBoundaryIndex < 4; iBoundaryIndex++)
		{
			// ��ǰD����Ϊ�µ�D��ע��D�Ļ����߽���ֵȫΪ0�����������Pixel Shaderֱ����SetToZero������Ҳ����Ҫ������Դ��
			V_RETURN(d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(0, 0, 0, 0), 1));
			V_RETURN(CommonComputeLine(NULL, NULL, NULL, pTexture, &m_PSClearTexture, iBoundaryIndex));
		}
	}
	else
		return D3DERR_INVALIDCALL;

	
	return S_OK;
}



HRESULT KFluidSimulation2D::GenerateNormal()
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// ����D������Normal
	V_RETURN(CommonComputeQuad(m_pTexD, NULL, NULL, m_pTexNormal, &m_PSGenerateNormal));

	// �ǵ�ͬʱ��Ҫ����߽�
	for(UINT i = 0; i < 4; i++)
		V_RETURN(CommonComputeLine(m_pTexD, NULL, NULL, m_pTexNormal, &m_PSGenerateNormal, i));

	return S_OK;
}











/****************************�ڲ��ӿ�--����**********************************/
HRESULT KFluidSimulation2D::CommonComputeQuad(IDirect3DTexture9* pSrcTex1, IDirect3DTexture9* pSrcTex2, IDirect3DTexture9* pSrcTex3, IDirect3DTexture9* pRT, PIXELSHADER *pPS)
{
	if(!m_pVBQuad || !m_pVBQuadExceptBoundary)
		return D3DERR_NOTAVAILABLE;

	if(!pRT || !pPS)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	UINT i = 0;

	// ������Ⱦ��Դ��Ŀ��
	V_RETURN(SetTexturedRenderTarget(0, pRT, NULL));

	V_RETURN(d3ddevice->SetTexture(0, pSrcTex1));
	V_RETURN(d3ddevice->SetTexture(1, pSrcTex2));
	V_RETURN(d3ddevice->SetTexture(2, pSrcTex3));

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


	// ����Declaration
	V_RETURN(d3ddevice->SetVertexShader(NULL));
	V_RETURN(d3ddevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE4(0) | D3DFVF_TEXCOORDSIZE4(1)));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_pDeclaration));

	// ����PS�������Ĵ�����1/2tap��1tap��TextureDimension����c0��c1��c2��
	V_RETURN(pPS->SetPixelShader());
	V_RETURN(pPS->SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_iWidth, 1.0f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(2, &D3DXVECTOR4((float)m_iWidth, (float)m_iHeight, 0, 0), 1));
		// һЩ�����͵�ǰʱ����
	V_RETURN(pPS->SetConstant(3, &D3DXVECTOR4(0, 0.5f, 1.0f, 2.0f), 1));
	V_RETURN(pPS->SetConstant(4, &D3DXVECTOR4(m_fDeltaTime, m_fDeltaTime, m_fDeltaTime, m_fDeltaTime), 1));


	// ��ʼ��Ⱦ���ǵ�ֻ��Clear���ô���Σ�ģ����̺�Copy���ò�������С���Σ��߽����ͨ������߽�����д��ģ�����ҪCopy��
	if(pPS == &m_PSClearTexture)
	{
		V_RETURN(d3ddevice->SetStreamSource(0, m_pVBQuad, 0, m_iStride));
	}
	else
	{
		V_RETURN(d3ddevice->SetStreamSource(0, m_pVBQuadExceptBoundary, 0, m_iStride));
	}

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// �ָ�Pixel Shader
	V_RETURN(pPS->RestorePixelShader());

	// ��ɣ���Ⱦ������1
	m_iRenderNumPerFrame++;
	return S_OK;
}

HRESULT KFluidSimulation2D::CommonComputeLine(IDirect3DTexture9* pSrcTex1, IDirect3DTexture9* pSrcTex2, IDirect3DTexture9* pSrcTex3, IDirect3DTexture9* pRT, PIXELSHADER *pPS, UINT iLineNo)
{
	if(iLineNo > 3)
		return D3DERR_INVALIDCALL;

	if(!m_iCreateAttrib || !m_pVBBoundaryLine[iLineNo])
		return D3DERR_NOTAVAILABLE;

	if(!pRT || !pPS)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	UINT i = 0;

	// ������Ⱦ��Դ��Ŀ��
	V_RETURN(SetTexturedRenderTarget(0, pRT, NULL));

	V_RETURN(d3ddevice->SetTexture(0, pSrcTex1));
	V_RETURN(d3ddevice->SetTexture(1, pSrcTex2));
	V_RETURN(d3ddevice->SetTexture(2, pSrcTex3));

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


	// ����Declaration
	V_RETURN(d3ddevice->SetVertexDeclaration(m_pDeclaration));
	V_RETURN(d3ddevice->SetVertexShader(NULL));
	V_RETURN(d3ddevice->SetFVF(0));

	// ����PS�������Ĵ�����1/2tap��1tap��TextureDimension����c0��c1��c2��
	V_RETURN(pPS->SetPixelShader());
	V_RETURN(pPS->SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_iWidth, 1.0f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(2, &D3DXVECTOR4((float)m_iWidth, (float)m_iHeight, 0, 0), 1));
	// һЩ�����͵�ǰʱ����
	V_RETURN(pPS->SetConstant(3, &D3DXVECTOR4(0, 0.5f, 1.0f, 2.0f), 1));
	V_RETURN(pPS->SetConstant(4, &D3DXVECTOR4(m_fDeltaTime, m_fDeltaTime, m_fDeltaTime, m_fDeltaTime), 1));

	// ��ʼ��Ⱦ
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVBBoundaryLine[iLineNo], 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_LINELIST, 0, 1));
	V_RETURN(d3ddevice->EndScene());

	// �ָ�Pixel Shader
	V_RETURN(pPS->RestorePixelShader());

	// ���
	return S_OK;
}






/****************************��ʼ��**********************************/

HRESULT KFluidSimulation2D::Init(UINT iWidth, UINT iHeight, BOOL bGenerateNormal /* = TRUE */)
{
	if(m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// ������
	if(!iWidth || !iHeight)
		return D3DERR_INVALIDCALL;
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_bGenerateNormal = bGenerateNormal;

	m_iRenderNumPerFrame = 0;


	D3DFORMAT FormatFourChannel = D3DFMT_A8R8G8B8;
	// �����ͳ�ʼ��������ͼ
#ifdef USE_FP16
	FormatFourChannel = D3DFMT_A16B16G16R16F;
#elif defined USE_FP32
	FormatFourChannel = D3DFMT_A32B32G32R32F;
#else
	mymessage("δ����ʹ�ø��������޷���ʼ������ģ�����棡");
	return E_FAIL;
#endif

	// Temp��ͼ
	for(UINT i = 0; i < MAX_FLUID_STEPS; i++)
	{
		V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexWTemp[i], NULL));
		V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexDTemp[i], NULL));
	}
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTemp[0], NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTemp[1], NULL));

	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexBCTemp, NULL));

	// ���ձ���������ͼ
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexP, NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexU, NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexD, NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexNormal, NULL));

	// Offset�����˼�0.5���޷�ֻ��0��0.5��1����ֵ��û��Ҫ�ø�����ͼ
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexABCOffset, NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexABCBoundary, NULL));	
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexABCType, NULL));

	// ����м��ض��������ͼ
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexDivW, NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexCurlW, NULL));

	



	// ��ʼ��ABC���Ͷ�Ӧ��ƫ�Ƶ�����ת��ͼ����13�ֱ߽翪�����ͣ�������Offsetֻ��-1 0 1���֣�����д��ĵ�ʱ��Ҫ�˼�0.5
	// ����������1D��ͼ���Ƚ�С��Ϊ��д�����ݵľ�ȷ�����������FP32����
	V_RETURN(d3ddevice->CreateTexture(13, 1, 1, D3DUSAGE_0, D3DFMT_A32B32G32R32F, D3DPOOL_MANAGED, &m_pTexABCTypeToOffset, NULL));
	V_RETURN(d3ddevice->CreateTexture(600, 1, 1, D3DUSAGE_0, D3DFMT_A32B32G32R32F, D3DPOOL_MANAGED, &m_pTexABCBoundaryToOffset, NULL));
	
	D3DLOCKED_RECT Rect;
	LPDIRECT3DSURFACE9 pSurf = NULL;
	float fValue = 0.0f;
	float *p = NULL;

	D3DXVECTOR4 pTypeData[13] = 
	{
		D3DXVECTOR4(0, 0, 0, 0),

		D3DXVECTOR4(1, 0, 0, -1),
		D3DXVECTOR4(-1, 0, 0, 1),

		D3DXVECTOR4(-1, 0, 0, -1),
		D3DXVECTOR4(1, 0, 0, 1),

		D3DXVECTOR4(1, 0, 0, 1),
		D3DXVECTOR4(-1, 0, 0, -1),

		D3DXVECTOR4(-1, 0, 0, 1),
		D3DXVECTOR4(1, 0, 0, -1),

		D3DXVECTOR4(-1, 0, -1, 0),
		D3DXVECTOR4(1, 0, 1, 0),

		D3DXVECTOR4(0, -1, 0, -1),
		D3DXVECTOR4(0, 1, 0, 1),
	};


	// Type To Offset
	V_RETURN(m_pTexABCTypeToOffset->LockRect(0, &Rect, NULL, 0));
	p = (float *)((BYTE *)Rect.pBits);
	for(UINT iX=0; iX < 13; iX++)
	{
		for(UINT i = 0; i < 4; i++)
		{
			// �˼�0.5��������ƫ��ֵӳ��Ϊ0��1
			fValue = pTypeData[iX][i] * 0.5f + 0.5f;
			*p++ = fValue;
		}
	}
	V_RETURN(m_pTexABCTypeToOffset->UnlockRect(0));



	// Boundary To Offset
	D3DXVECTOR4 VecBoundaryToOffset;
	V_RETURN(m_pTexABCBoundaryToOffset->LockRect(0, &Rect, NULL, 0));
	p = (float *)((BYTE *)Rect.pBits);
	for(UINT iX=0; iX < 600; iX++)
	{
		switch(iX)
		{
			// ��ȷ��12�ֱ߽�����ֵ��Ӧ��ƫ����
			case 30:
				VecBoundaryToOffset = D3DXVECTOR4(1, 0, 0, -1);
				break;
			case 440:
				VecBoundaryToOffset = D3DXVECTOR4(-1, 0, 0, 1);
				break;
			case 50:
				VecBoundaryToOffset = D3DXVECTOR4(-1, 0, 0, -1);
				break;
			case 260:
				VecBoundaryToOffset = D3DXVECTOR4(1, 0, 0, 1);
				break;
			case 21:
				VecBoundaryToOffset = D3DXVECTOR4(1, 0, 0, 1);
				break;
			case 521:
				VecBoundaryToOffset = D3DXVECTOR4(-1, 0, 0, -1);
				break;
			case 41:
				VecBoundaryToOffset = D3DXVECTOR4(-1, 0, 0, 1);
				break;
			case 341:
				VecBoundaryToOffset = D3DXVECTOR4(1, 0, 0, -1);
				break;
			case 411:
				VecBoundaryToOffset = D3DXVECTOR4(-1, 0, -1, 0);
				break;
			case 211:
				VecBoundaryToOffset = D3DXVECTOR4(1, 0, 1, 0);
				break;
			case 160:
				VecBoundaryToOffset = D3DXVECTOR4(0, -1, 0, -1);
				break;
			case 70:
				VecBoundaryToOffset = D3DXVECTOR4(0, 1, 0, 1);
				break;

			// �����������߽�ֵ�ģ�һ�ɸ�0
			default:
				VecBoundaryToOffset = D3DXVECTOR4(0, 0, 0, 0);
			break;
		}

		for(UINT i = 0; i < 4; i++)
		{
			// �˼�0.5��������ƫ��ֵӳ��Ϊ0��1
			fValue = VecBoundaryToOffset[i] * 0.5f + 0.5f;
			*p++ = fValue;
		}
	}
	V_RETURN(m_pTexABCBoundaryToOffset->UnlockRect(0));






	// Vertex Delcaration: No-VS XYZRHW Quad
	D3DVERTEXELEMENT9 Dclr[] = {
		{0,0,D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0},
		{0,16,D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		{0,16+16,D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
		D3DDECL_END()
	};
	V_RETURN(d3ddevice->CreateVertexDeclaration(Dclr, &m_pDeclaration));


	// ��ʼ��Vertex Buffer��һ��Quad���ĸ�Line
	m_iStride = sizeof(VERTEXTYPE);
	V_RETURN(d3ddevice->CreateVertexBuffer(4 * m_iStride, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVBQuad, NULL));

//	����λ�ã�	1	3
//				0	2
	VERTEXTYPE QuadVertex[4] = 
	{
		{0, (float)m_iHeight,					0, 1,		0,1,0,0,		0,(float)m_iHeight,0,0},
		{0, 0,									0, 1,		0,0,0,0,		0,0,0,0},
		{(float)m_iWidth, (float)m_iHeight,		0, 1,		1,1,0,0,		(float)m_iWidth,(float)m_iHeight,0,0},
		{(float)m_iWidth, 0,					0, 1,		1,0,0,0,		(float)m_iWidth,0,0,0}
	};
	VOID* pVertexStream = NULL;
	V_RETURN(m_pVBQuad->Lock(0, 4 * m_iStride, &pVertexStream, 0));
	memcpy(pVertexStream, QuadVertex, 4 * m_iStride);
	m_pVBQuad->Unlock();


	V_RETURN(d3ddevice->CreateVertexBuffer(4 * m_iStride, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVBQuadExceptBoundary, NULL));
	VERTEXTYPE QuadExceptBoundaryVertex[4] = 
	{
		{1, (float)m_iHeight-1,					0, 1,		0+1.0f/(float)m_iWidth,	1-1.0f/(float)m_iHeight,0,0,		0+1,(float)m_iHeight-1,0,0},
		{1, 1,									0, 1,		0+1.0f/(float)m_iWidth,	0+1.0f/(float)m_iHeight,0,0,		0+1,0+1,0,0},
		{(float)m_iWidth-1, (float)m_iHeight-1,	0, 1,		1-1.0f/(float)m_iWidth,	1-1.0f/(float)m_iHeight,0,0,		(float)m_iWidth-1,(float)m_iHeight-1,0,0},
		{(float)m_iWidth-1, 1,					0, 1,		1-1.0f/(float)m_iWidth,	0+1.0f/(float)m_iHeight,0,0,		(float)m_iWidth-1,0+1,0,0}
	};
	V_RETURN(m_pVBQuadExceptBoundary->Lock(0, 4 * m_iStride, &pVertexStream, 0));
	memcpy(pVertexStream, QuadExceptBoundaryVertex, 4 * m_iStride);
	m_pVBQuadExceptBoundary->Unlock();


	VERTEXTYPE LineVertex[8] = 
	{
		// �ϱ߽�13
		{0, 0,									0, 1,		0,0,0,0,		0,0,0,0},
		{(float)m_iWidth-1, 0,					0, 1,		1-1.0f/(float)m_iWidth,0,0,0,		(float)m_iWidth-1,0,0,0},
		// �±߽�02
		{0, (float)m_iHeight-1,					0, 1,		0,1-1.0f/(float)m_iHeight,0,0,		0,(float)m_iHeight-1,0,0},
		{(float)m_iWidth-1, (float)m_iHeight-1,		0, 1,		1-1.0f/(float)m_iWidth,1-1.0f/(float)m_iHeight,0,0,		(float)m_iWidth-1,(float)m_iHeight-1,0,0},
		// ��߽�10
		{0, 0,									0, 1,		0,0,0,0,		0,0,0,0},
		{0, (float)m_iHeight-1,					0, 1,		0,1-1.0f/(float)m_iHeight,0,0,		0,(float)m_iHeight-1,0,0},
		// �ұ߽�32
		{(float)m_iWidth-1, 0,					0, 1,		1-1.0f/(float)m_iWidth,0,0,0,		(float)m_iWidth-1,0,0,0},
		{(float)m_iWidth-1, (float)m_iHeight-1,		0, 1,		1-1.0f/(float)m_iWidth,1-1.0f/(float)m_iHeight,0,0,		(float)m_iWidth-1,(float)m_iHeight-1,0,0},
	};
	for(UINT i = 0; i < 4; i++)
	{
		V_RETURN(d3ddevice->CreateVertexBuffer(2 * m_iStride, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVBBoundaryLine[i], NULL));
		V_RETURN(m_pVBBoundaryLine[i]->Lock(0, 2 * m_iStride, &pVertexStream, 0));
		memcpy(pVertexStream, LineVertex + 2*i, 2 * m_iStride);
		m_pVBBoundaryLine[i]->Unlock();
	}




	// ������Ȼ���
	D3DSURFACE_DESC DescDepth;
	LPDIRECT3DSURFACE9 pOldDepth = NULL;

	if(FAILED(d3ddevice->GetDepthStencilSurface(&pOldDepth)))
		return E_FAIL;
	if(FAILED(pOldDepth->GetDesc(&DescDepth)))
		return E_FAIL;
	SAFE_RELEASE(pOldDepth);

	V_RETURN(d3ddevice->CreateDepthStencilSurface(iWidth, iHeight, DescDepth.Format, DescDepth.MultiSampleType, DescDepth.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL));





	// ��ʼ��Shader
	// MISC
	V_RETURN(m_PSClearTexture.InitPixelShader("shader\\Fluid\\ClearTexture.psh"));
	V_RETURN(m_PSCopy.InitPixelShader("shader\\Fluid\\CopyTexture.psh"));
	if(CheckPS2xSupport(12, FALSE))
	{
		V_RETURN(m_PSGenerateNormal.InitPixelShader("shader\\Fluid\\Normal\\Cross-Product GenerateNormal.psh"));
	}
	else
	{
		V_RETURN(m_PSGenerateNormal.InitPixelShader("shader\\Fluid\\Normal\\Add-Sub GenerateNormal.psh"));
	}

	// Inject
	V_RETURN(m_PSAddForceSphere.InitPixelShader("shader\\Fluid\\Inject\\AddForce_Sphere.psh"));
	V_RETURN(m_PSAddForceTexture.InitPixelShader("shader\\Fluid\\Inject\\AddForce_Texture.psh"));
	V_RETURN(m_PSAddForceFire.InitPixelShader("shader\\Fluid\\Inject\\AddForce_Sphere.psh"));
	V_RETURN(m_PSAddSourceSphere.InitPixelShader("shader\\Fluid\\Inject\\AddSource_Sphere.psh"));
	V_RETURN(m_PSAddSourceTexture.InitPixelShader("shader\\Fluid\\Inject\\AddSource_Texture.psh"));
	V_RETURN(m_PSAddSourceFire.InitPixelShader("shader\\Fluid\\Inject\\AddSource_Fire.psh"));

	// NSEs
	V_RETURN(m_PSAdvect_W.InitPixelShader("shader\\Fluid\\NSE\\Advect.psh"));
	V_RETURN(m_PSAdvect_D.InitPixelShader("shader\\Fluid\\NSE\\Advect.psh"));
	V_RETURN(m_PSGravity.InitPixelShader("shader\\Fluid\\NSE\\Gravity.psh"));
	V_RETURN(m_PSDiv_W.InitPixelShader("shader\\Fluid\\NSE\\Div_W.psh"));
	V_RETURN(m_PSProject.InitPixelShader("shader\\Fluid\\NSE\\Project.psh"));
	V_RETURN(m_PSJacobi_P.InitPixelShader("shader\\Fluid\\NSE\\Jacobi_P.psh"));
	V_RETURN(m_PSJacobi_W.InitPixelShader("shader\\Fluid\\NSE\\Jacobi_W.psh"));
	V_RETURN(m_PSJacobi_D.InitPixelShader("shader\\Fluid\\NSE\\Jacobi_D.psh"));
/*�����ؽ�����ʱ��֧��	V_RETURN(m_PSCurl_W.InitPixelShader("shader\\Fluid\\NSE\\Curl_W.psh"));
	V_RETURN(m_PSVorticity.InitPixelShader("shader\\Fluid\\NSE\\Vorticity.psh"));
*/

	// BCs
	V_RETURN(m_PSBBC_U.InitPixelShader("shader\\Fluid\\BC\\BBC.psh"));
	V_RETURN(m_PSBBC_P.InitPixelShader("shader\\Fluid\\BC\\BBC.psh"));

	V_RETURN(m_PSABC_U.InitPixelShader("shader\\Fluid\\BC\\ABC_Velocity.psh"));
	V_RETURN(m_PSABC_P.InitPixelShader("shader\\Fluid\\BC\\ABC_P.psh"));
	V_RETURN(m_PSABC_D.InitPixelShader("shader\\Fluid\\BC\\ABC_D.psh"));
	V_RETURN(m_PSObstacleToBoundary.InitPixelShader("shader\\Fluid\\Obstacle\\ObstacleToBoundary.psh"));
	if(CheckPS2xSupport(12, FALSE))
		V_RETURN(m_PSBoundaryToOffset.InitPixelShader("shader\\Fluid\\Obstacle\\BoundaryToOffset.psh"));

		// ����������ʱ����
	//V_RETURN(m_PSBoundaryToType.InitPixelShader("shader\\Fluid\\Obstacle\\BoundaryToType.psh"));
	//V_RETURN(m_PSTypeToOffset.InitPixelShader("shader\\Fluid\\Obstacle\\TypeToOffset.psh"));




	// ��ʼʱ�Զ���������״̬
	V_RETURN(ResetFluid());



	// �ɹ�
	m_iCreateAttrib = 1;
	return S_OK;
}