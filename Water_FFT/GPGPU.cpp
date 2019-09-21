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

	// 目的贴图有效性检验，必须长宽一致，4通道浮点格式，而且必须是RT
	D3DSURFACE_DESC Desc;
	V_RETURN(pDestTexture->GetLevelDesc(0, &Desc));
	if(Desc.Width != m_iWidth || Desc.Height != m_iHeight || Desc.Usage != D3DUSAGE_RENDERTARGET)
		return D3DERR_WRONGTEXTUREFORMAT;

	if(Desc.Format != D3DFMT_A16B16G16R16F && Desc.Format != D3DFMT_A32B32G32R32F)
		return D3DERR_WRONGTEXTUREFORMAT;



	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便FFT渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// 设置渲染来源和目的
	SetTexturedRenderTarget(0, pDestTexture, NULL);

	V_RETURN(d3ddevice->SetTexture(0, m_pTexResult));

	// 设置渲染参数
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	// 设置Shader及常量寄存器
	V_RETURN(m_PSGetResult.SetPixelShader());
	V_RETURN(m_PSGetResult.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));

	float fDividCoef = bMulFourierCoef ? 1.0f / ((float)m_iWidth * (float)m_iHeight) : 1.0f;
	V_RETURN(m_PSGetResult.SetConstant(1, &D3DXVECTOR4(fDividCoef, fDividCoef, 1, 1), 1));

	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));
	V_RETURN(d3ddevice->SetFVF(0));

	// 开始渲染
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// 恢复Pixel Shader和RT、深度缓冲
	V_RETURN(m_PSGetResult.RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	// 完成
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

	// 必须和m_Width/Height相同
	D3DSURFACE_DESC Desc;
	pTexFFTSource->GetLevelDesc(0, &Desc);
	if(Desc.Width != m_iWidth || Desc.Height != m_iHeight || Desc.Pool == D3DPOOL_SYSTEMMEM)
		return D3DERR_WRONGTEXTUREFORMAT;



	HRESULT hr = S_OK;
	UINT i = 0;

	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便FFT渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));

	// 开始处理，分4步
	UINT iTempUseNo = 0;	// 控制交换两个临时使用的贴图号

	// X
	// ScrambleX要放在最先，因为必须从Source贴图处理到TempUse贴图，如果是1*Y的贴图，这个操作就不会影响值的排列，等于起到了复制贴图的作用
	V_RETURN(ScrambleX(pTexFFTSource, m_pTexTempUse[iTempUseNo]));

	if(m_iWidth > 1)
	{
		// ButterflyX，Butterfly数据必须对2的幂才有效，这里做一个特殊处理，宽度为1时不能使用（但宽度为1时Scramble仍然会保留原数据，不会影响的，所以保留）
		// 如果本身只有1D，那么下面的ButterflyY就会跳过，这里要强制输出到TexResult
		if(m_iHeight == 1)
		{
			for(i = 0; i < m_iButterflyNumX - 1; i++)
			{
				// 上次的目的作为源，取反作为目的
				V_RETURN(ButterflyX(i, bIFFT, m_pTexTempUse[iTempUseNo], m_pTexTempUse[!iTempUseNo]));
				iTempUseNo = !iTempUseNo;
			}
			// 最后一次输出到TexResult
			V_RETURN(ButterflyX(i, bIFFT, m_pTexTempUse[iTempUseNo], m_pTexResult));
			iTempUseNo = !iTempUseNo;
		}
		else
		{
			for(i = 0; i < m_iButterflyNumX; i++)
			{
				// 上次的目的作为源，取反作为目的
				V_RETURN(ButterflyX(i, bIFFT, m_pTexTempUse[iTempUseNo], m_pTexTempUse[!iTempUseNo]));
				iTempUseNo = !iTempUseNo;
			}
		}// end iHeight == 1
	}// end X

	// Y
	if(m_iHeight > 1)
	{
		// ScrambleY：2D FFT
		V_RETURN(ScrambleY(m_pTexTempUse[iTempUseNo], m_pTexTempUse[!iTempUseNo]));
		iTempUseNo = !iTempUseNo;

		// ButterflyY
		for(i = 0; i < m_iButterflyNumY - 1; i++)
		{
			V_RETURN(ButterflyY(i, bIFFT, m_pTexTempUse[iTempUseNo], m_pTexTempUse[!iTempUseNo]));
			iTempUseNo = !iTempUseNo;
		}
		// 最后一次Butterfly，直接输出到Result
		V_RETURN(ButterflyY(i, bIFFT, m_pTexTempUse[iTempUseNo], m_pTexResult));
	}




	// 恢复RT
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);


	// 完成
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

	// 设置渲染来源和目的
	V_RETURN(SetTexturedRenderTarget(0, pRT, NULL));

	V_RETURN(d3ddevice->SetTexture(0, m_pTexScrambleX));
	V_RETURN(d3ddevice->SetTexture(1, pSource));

	// 设置渲染参数
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

	// 设置Shader及常量寄存器
	V_RETURN(m_PSScrambleX.SetPixelShader());
	V_RETURN(m_PSScrambleX.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));
	V_RETURN(d3ddevice->SetFVF(0));

	// 开始渲染
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// 恢复Pixel Shader
	V_RETURN(m_PSScrambleX.RestorePixelShader());

	// 完成
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

	// 设置渲染来源和目的
	V_RETURN(SetTexturedRenderTarget(0, pRT, NULL));

	V_RETURN(d3ddevice->SetTexture(0, m_pTexScrambleY));
	V_RETURN(d3ddevice->SetTexture(1, pSource));

	// 设置渲染参数
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

	// 设置Shader及常量寄存器
	V_RETURN(m_PSScrambleY.SetPixelShader());
	V_RETURN(m_PSScrambleY.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));
	V_RETURN(d3ddevice->SetFVF(0));

	// 开始渲染
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// 恢复Pixel Shader
	V_RETURN(m_PSScrambleY.RestorePixelShader());

	// 完成
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

	// 设置渲染来源和目的
	V_RETURN(SetTexturedRenderTarget(0, pRT, NULL));

	V_RETURN(d3ddevice->SetTexture(0, m_ppTexButterflyX[iLitNo]));
	V_RETURN(d3ddevice->SetTexture(1, m_ppTexButterflyX_Sign[iLitNo]));
	V_RETURN(d3ddevice->SetTexture(2, pSource));

	// 设置渲染参数
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


	// 设置Shader及常量寄存器
	V_RETURN(pPS->SetPixelShader());
	V_RETURN(pPS->SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));
	V_RETURN(d3ddevice->SetFVF(0));

	// 开始渲染
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// 恢复Pixel Shader
	V_RETURN(pPS->RestorePixelShader());

	// 完成
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

	// 设置渲染来源和目的
	V_RETURN(SetTexturedRenderTarget(0, pRT, NULL));

	V_RETURN(d3ddevice->SetTexture(0, m_ppTexButterflyY[iLitNo]));
	V_RETURN(d3ddevice->SetTexture(1, m_ppTexButterflyY_Sign[iLitNo]));
	V_RETURN(d3ddevice->SetTexture(2, pSource));

	// 设置渲染参数
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


	// 设置Shader及常量寄存器
	V_RETURN(pPS->SetPixelShader());
	V_RETURN(pPS->SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));
	V_RETURN(d3ddevice->SetFVF(0));

	// 开始渲染
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// 恢复Pixel Shader
	V_RETURN(pPS->RestorePixelShader());

	// 完成
	return S_OK;
}







/****************************初始化**********************************/

HRESULT KFourierTransform::Init(UINT iWidth, UINT iHeight)
{
	if(m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;
	if(!iWidth || !iHeight)
		return D3DERR_INVALIDCALL;

	// 检查是否2的幂，如果非2的幂就会返回0，但注意允许iWidth或iHeight其中一个等于1，但不允许iWidth=iHeight=1
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

	// 创建结果贴图
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
	mymessage("未定义使用浮点纹理，无法初始化FFT引擎！");
	return E_FAIL;
#endif

	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexResult, NULL));



	// 初始化Quad
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

	// 初始化Shader
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



	// 创建深度缓冲
	D3DSURFACE_DESC DescDepth;
	LPDIRECT3DSURFACE9 pOldDepth = NULL;

	if(FAILED(d3ddevice->GetDepthStencilSurface(&pOldDepth)))
		return E_FAIL;
	if(FAILED(pOldDepth->GetDesc(&DescDepth)))
		return E_FAIL;
	SAFE_RELEASE(pOldDepth);

	V_RETURN(d3ddevice->CreateDepthStencilSurface(iWidth, iHeight, DescDepth.Format, DescDepth.MultiSampleType, DescDepth.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL));


	// 创建临时使用的贴图
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTempUse[0], NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTempUse[1], NULL));



	// 初始化Scramble贴图--X
	V_RETURN(d3ddevice->CreateTexture(iWidth, 1, 1, D3DUSAGE_0, FormatDualChannel, D3DPOOL_MANAGED, &m_pTexScrambleX, NULL));

	V_RETURN(m_pTexScrambleX->GetSurfaceLevel(0, &pSurf));
	V_RETURN(pSurf->LockRect(&Rect, NULL, 0));

	p16 = (float16 *)((BYTE *)Rect.pBits);	
	p = (float *)((BYTE *)Rect.pBits);


	for(iX=0; iX < iWidth; iX++)
	{
		// 得到反序后的像素序号
		iSwapNo = ReverseBitOrder(iX, m_iButterflyNumX);
		// 转换成纹理坐标存入，注意加1/2 Tap以便像素中心对齐
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



	// 初始化Scramble贴图--Y
	V_RETURN(d3ddevice->CreateTexture(1, iHeight, 1, D3DUSAGE_0, FormatDualChannel, D3DPOOL_MANAGED, &m_pTexScrambleY, NULL));

	V_RETURN(m_pTexScrambleY->GetSurfaceLevel(0, &pSurf));
	V_RETURN(pSurf->LockRect(&Rect, NULL, 0));

	for(iY=0; iY < iHeight; iY++)
	{
		p16 = (float16 *)((BYTE *)Rect.pBits + iY * Rect.Pitch);	
		p = (float *)((BYTE *)Rect.pBits + iY * Rect.Pitch);

		// 得到反序后的像素序号
		iSwapNo = ReverseBitOrder(iY, m_iButterflyNumY);
		// 转换成纹理坐标存入，注意加1/2 Tap以便像素中心对齐
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




	// 初始化Butterfly贴图
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


	// 成功
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


	// Lock，Ready? Go!
	V_RETURN(m_ppTexButterflyX[iLitNo]->LockRect(0, &Rect, NULL, 0));
	p16 = (float16 *)((BYTE *)Rect.pBits);	
	p = (float *)((BYTE *)Rect.pBits);

	V_RETURN(m_ppTexButterflyX_Sign[iLitNo]->LockRect(0, &Rect_Sign, NULL, 0));
	p16_Sign = (float16 *)((BYTE *)Rect_Sign.pBits);	
	p_Sign = (float *)((BYTE *)Rect_Sign.pBits);



	// 组数和每组元素数，没问题都能整除的，放心吧
	UINT iElemNumPerGroup = (UINT)pow(2, iLitNo+1);
	UINT iGroupNum = m_iWidth / iElemNumPerGroup;

	UINT iGroupNo = 0;			// 当前像素在哪个组，配对像素也要在这个组
	UINT iNoInGroup = 0;		// 当前像素在该组中的序号（组头序号为0，依次递增）
	UINT iNoInHalfGroup = 0;	// 当前/配对像素在半组中的序号（和上半组或下半组第一个元素的间隔，具体通过bCur/PairUpper判断，两个配对像素的该值相等）

	BOOL bCurUpper = FALSE;		// 当前像素是在上半组还是下半组，true表示上，false表示下
	UINT iPairPosition = 0;		// 配对像素在这一行的总序号
	BOOL bPairUpper = FALSE;	// 配对像素是在上半组还是下半组，true表示上，false表示下，它和bCurUpper的值必定相反

	UINT iUXInterval = m_iWidth / 2 / (UINT)powf(2.0f, (float)iLitNo), iUX = 0;	// 计算W的指数UX需要的，每次迭代UX沿着像素递增，都会有一个间隔


	// 当前迭代次数中，是多少个像素一组
	for(iX=0; iX < m_iWidth; iX++)
	{
		// 首先判断当前/配对像素是在哪组，及在该组中的两个序号，以及在上半组还是下半组，及半组中的序号
		iGroupNo = iX / iElemNumPerGroup;
		iNoInGroup = iX % iElemNumPerGroup;
		iNoInHalfGroup = iX % (iElemNumPerGroup / 2);
		// 当前元素在上半组
		if(iNoInGroup < iElemNumPerGroup / 2)
		{
			bCurUpper = TRUE;
			bPairUpper = FALSE;
			iPairPosition = iX + iElemNumPerGroup / 2;

			// 是Cur + Pair * W型的，Cur存到A点（x分量），Pair存到B点（y分量）
			// 转换成纹理坐标存入，注意加1/2 Tap以便像素中心对齐
			fData.x = (float)iX / (float)m_iWidth + 0.5f / (float)m_iWidth;
			fData.y = (float)iPairPosition / (float)m_iWidth + 0.5f / (float)m_iWidth;
			// 得到W的指数
			iUX = iNoInHalfGroup * iUXInterval;
			// 计算W实部和虚部
			fExpValue = (float)iUX / (float)m_iWidth;
			fW_re = cosf(2.0f * D3DX_PI * fExpValue);
			fW_im = sinf(2.0f * D3DX_PI * fExpValue);
			// 根据值写数据，将W绝对值存放到zw分量中，并将正负号存到Sign贴图中
			fData.z = absf(fW_re);
			fData.w = absf(fW_im);
			fData_Sign.z = (fW_re < 0.0f) ? 0.0f : 2.0f;
			fData_Sign.w = (fW_im < 0.0f) ? 0.0f : 2.0f;
			// 当前像素在上半组，是加法方式结合，将对应的数据写入Sign贴图中
			fData_Sign.x = 2.0f;
			fData_Sign.y = 0.0f;	// y值未用
		}
		// 当前元素在下半组
		else
		{
			bCurUpper = FALSE;
			bPairUpper = FALSE;
			iPairPosition = iX - iElemNumPerGroup / 2;

			// 是Pair - Cur * W型的，Pair存到A点（x分量），Cur存到B点（y分量）
			// 转换成纹理坐标存入，注意加1/2 Tap以便像素中心对齐
			fData.x = (float)iPairPosition / (float)m_iWidth + 0.5f / (float)m_iWidth;
			fData.y = (float)iX / (float)m_iWidth + 0.5f / (float)m_iWidth;
			// 得到W的指数
			iUX = iNoInHalfGroup * iUXInterval;
			// 计算W实部和虚部
			fExpValue = (float)iUX / (float)m_iWidth;
			fW_re = cosf(2.0f * D3DX_PI * fExpValue);
			fW_im = sinf(2.0f * D3DX_PI * fExpValue);
			// 根据值写数据，将W绝对值存放到zw分量中，并将正负号存到Sign贴图中
			fData.z = absf(fW_re);
			fData.w = absf(fW_im);
			fData_Sign.z = (fW_re < 0.0f) ? 0.0f : 2.0f;
			fData_Sign.w = (fW_im < 0.0f) ? 0.0f : 2.0f;
			// 当前像素就在下半组，需要给减号来结合，写入Sign贴图中
			fData_Sign.x = 0.0f;
			fData_Sign.y = 0.0f;	// y值未用
		}


		// 写入贴图
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

	// 结束
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


	// 组数和每组元素数，没问题都能整除的，放心吧
	UINT iElemNumPerGroup = (UINT)pow(2, iLitNo+1);
	UINT iGroupNum = m_iHeight / iElemNumPerGroup;

	UINT iGroupNo = 0;			// 当前像素在哪个组，配对像素也要在这个组
	UINT iNoInGroup = 0;		// 当前像素在该组中的序号（组头序号为0，依次递增）
	UINT iNoInHalfGroup = 0;	// 当前/配对像素在半组中的序号（和上半组或下半组第一个元素的间隔，具体通过bCur/PairUpper判断，两个配对像素的该值相等）

	BOOL bCurUpper = FALSE;		// 当前像素是在上半组还是下半组，true表示上，false表示下
	UINT iPairPosition = 0;		// 配对像素在这一行的总序号
	BOOL bPairUpper = FALSE;	// 配对像素是在上半组还是下半组，true表示上，false表示下，它和bCurUpper的值必定相反

	UINT iUXInterval = m_iHeight / 2 / (UINT)powf(2.0f, (float)iLitNo), iUX = 0;	// 计算W的指数UX需要的，每次迭代UX沿着像素递增，都会有一个间隔


	// 当前迭代次数中，是多少个像素一组
	for(iY=0; iY < m_iHeight; iY++)
	{
		p16 = (float16 *)((BYTE *)Rect.pBits + iY * Rect.Pitch);	
		p = (float *)((BYTE *)Rect.pBits + iY * Rect.Pitch);

		p16_Sign = (float16 *)((BYTE *)Rect_Sign.pBits + iY * Rect_Sign.Pitch);	
		p_Sign = (float *)((BYTE *)Rect_Sign.pBits + iY * Rect_Sign.Pitch);

		// 首先判断当前/配对像素是在哪组，及在该组中的两个序号，以及在上半组还是下半组，及半组中的序号
		iGroupNo = iY / iElemNumPerGroup;
		iNoInGroup = iY % iElemNumPerGroup;
		iNoInHalfGroup = iY % (iElemNumPerGroup / 2);
		// 当前元素在上半组
		if(iNoInGroup < iElemNumPerGroup / 2)
		{
			bCurUpper = TRUE;
			bPairUpper = FALSE;
			iPairPosition = iY + iElemNumPerGroup / 2;

			// 是Cur + Pair * W型的，Cur存到A点（x分量），Pair存到B点（y分量）
			// 转换成纹理坐标存入，注意加1/2 Tap以便像素中心对齐
			fData.x = (float)iY / (float)m_iHeight + 0.5f / (float)m_iHeight;
			fData.y = (float)iPairPosition / (float)m_iHeight + 0.5f / (float)m_iHeight;
			// 得到W的指数
			iUX = iNoInHalfGroup * iUXInterval;
			// 计算W实部和虚部
			fExpValue = (float)iUX / (float)m_iHeight;
			fW_re = cosf(2.0f * D3DX_PI * fExpValue);
			fW_im = sinf(2.0f * D3DX_PI * fExpValue);
			// 根据值写数据，将W绝对值存放到zw分量中，并将正负号存到Sign贴图中
			fData.z = absf(fW_re);
			fData.w = absf(fW_im);
			fData_Sign.z = (fW_re < 0.0f) ? 0.0f : 2.0f;
			fData_Sign.w = (fW_im < 0.0f) ? 0.0f : 2.0f;
			// 当前像素在上半组，是加法方式结合，将对应的数据写入Sign贴图中
			fData_Sign.x = 2.0f;
			fData_Sign.y = 0.0f;	// y值未用
		}
		// 当前元素在下半组
		else
		{
			bCurUpper = FALSE;
			bPairUpper = FALSE;
			iPairPosition = iY - iElemNumPerGroup / 2;

			// 是Pair - Cur * W型的，Pair存到A点（x分量），Cur存到B点（y分量）
			// 转换成纹理坐标存入，注意加1/2 Tap以便像素中心对齐
			fData.x = (float)iPairPosition / (float)m_iHeight + 0.5f / (float)m_iHeight;
			fData.y = (float)iY / (float)m_iHeight + 0.5f / (float)m_iHeight;
			// 得到W的指数
			iUX = iNoInHalfGroup * iUXInterval;
			// 计算W实部和虚部
			fExpValue = (float)iUX / (float)m_iHeight;
			fW_re = cosf(2.0f * D3DX_PI * fExpValue);
			fW_im = sinf(2.0f * D3DX_PI * fExpValue);
			// 根据值写数据，将W绝对值存放到zw分量中，并将正负号存到Sign贴图中
			fData.z = absf(fW_re);
			fData.w = absf(fW_im);
			fData_Sign.z = (fW_re < 0.0f) ? 0.0f : 2.0f;
			fData_Sign.w = (fW_im < 0.0f) ? 0.0f : 2.0f;
			// 当前像素就在下半组，需要给减号来结合，写入Sign贴图中
			fData_Sign.x = 0.0f;
			fData_Sign.y = 0.0f;	// y值未用
		}


		// 写入贴图
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

	// 结束
	V_RETURN(m_ppTexButterflyY[iLitNo]->UnlockRect(0));
	V_RETURN(m_ppTexButterflyY_Sign[iLitNo]->UnlockRect(0));

	return S_OK;
}















/*********************************以下是用CPU做傅里叶变换************************/

// 做IDFT并log
HRESULT KFourierTransform::DFTCPU(IDirect3DTexture9* pTexFFTSource, BOOL bIDFT, BOOL bMulFourierCoef)
{
	// 必须先初始化
	if(!m_iCreateAttrib || !m_pTexResult)
		return D3DERR_NOTAVAILABLE;

	if(!pTexFFTSource)
		return D3DERR_INVALIDCALL;

	// 必须和m_Width/Height相同，而且不能是PoolDefault
	D3DSURFACE_DESC Desc;
	pTexFFTSource->GetLevelDesc(0, &Desc);
	if(Desc.Width != m_iWidth || Desc.Height != m_iHeight)
		return D3DERR_WRONGTEXTUREFORMAT;
	if(Desc.Format != D3DFMT_A32B32G32R32F)
		return D3DERR_WRONGTEXTUREFORMAT;

	// 来源贴图和目的贴图，用于中转Pool_Default的贴图
	IDirect3DTexture9* pTexSrc = NULL, *pTexDst = NULL;
	LPDIRECT3DSURFACE9 pSurfSrc = NULL, pSurfDst = NULL, pSurfUser = NULL, pSurfResult = NULL;

	V_RETURN(d3ddevice->CreateTexture(Desc.Width, Desc.Height, 1, 0, Desc.Format, D3DPOOL_SYSTEMMEM, &pTexDst, NULL));
	V_RETURN(d3ddevice->CreateTexture(Desc.Width, Desc.Height, 1, 0, Desc.Format, D3DPOOL_SYSTEMMEM, &pTexSrc, NULL));

	V_RETURN(pTexSrc->GetSurfaceLevel(0, &pSurfSrc));
	V_RETURN(pTexDst->GetSurfaceLevel(0, &pSurfDst));
	V_RETURN(pTexFFTSource->GetSurfaceLevel(0, &pSurfUser));
	V_RETURN(m_pTexResult->GetSurfaceLevel(0, &pSurfResult));

	// 将用户指定的贴图直接填充到创建的贴图中
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
			// 定位当前像素，通过它来写入结果贴图
			pCur = (float *)((BYTE *)RectDst.pBits + iY * RectDst.Pitch);
			pCur += iX * 4;

			// 先初始化实虚部的值
			fValue[0] = fValue[1] = 0.0f;

			// 计算该点对应的DFT结果，存储到fValue中
			for(UINT iY_DFT = 0; iY_DFT < m_iWidth; iY_DFT++)
			{
				pTemp = (float *)((BYTE *)RectSrc.pBits + iY_DFT * RectSrc.Pitch);

				for(UINT iX_DFT = 0; iX_DFT < m_iWidth; iX_DFT++)
				{
					// 先得到当前像素的值
					fCurValue[0] = *pTemp++;
					fCurValue[1] = *pTemp++;
					fCurValue[2] = *pTemp++;
					fCurValue[3] = *pTemp++;

					// 根据符号置实虚部的实值
					if(fCurValue[2] == 0.0)
						fCurValue[0] = -fCurValue[0];
					if(fCurValue[3] == 0.0)
						fCurValue[1] = -fCurValue[1];


					// 计算W，再计算W*F（复数乘法）
					D3DXVECTOR2 WValue, WFValue;
					fAngle = 2.0f * D3DX_PI * (float)(iX_DFT*iX + iY_DFT*iY) / (float)m_iWidth;
					// 正变换就取反
					if(!bIDFT)
						fAngle = -fAngle;

					WValue.x = cosf(fAngle);
					WValue.y = sinf(fAngle);

					WFValue.x = fCurValue.x * WValue.x - fCurValue.y * WValue.y;
					WFValue.y = fCurValue.x * WValue.y + fCurValue.y * WValue.x;

					// 分别叠加到fValue的实虚部中
					fValue[0] += WFValue.x;
					fValue[1] += WFValue.y;
				}
			}// end Each Pixel 2D FFT

			// 乘傅里叶系数
			if(bMulFourierCoef)
			{
				float fCoef = 1.0f / (float)(m_iWidth * m_iHeight);
				fValue[0] *= fCoef;
				fValue[1] *= fCoef;
			}


			// 存储fValue的时候注意格式，实虚部绝对值+符号标记
			if(fValue[0] < 0.0)				
				fValue[2] = 0.0f;
			else
				fValue[2] = 2.0f;

			if(fValue[1] < 0.0)				
				fValue[3] = 0.0f;
			else
				fValue[3] = 2.0f;

			// log，只记录真正的数值，不取符号
			for(UINT i = 0; i < 2; i++)
			{
				sprintf(szLog, "  %.3f,", fValue[i]);
				fwrite(szLog, strlen(szLog), 1, fp);
			}
			// 写入结果贴图并
			for(int i = 0; i < 4; i++)
			{
				fValue[0] = absf(fValue[0]);
				fValue[1] = absf(fValue[1]);
				*pCur++ = fValue[i];
			}

			// 换行排版
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

	// 把结果写入TexResult
	V_RETURN(D3DXLoadSurfaceFromSurface(pSurfResult, NULL, NULL, pSurfDst, NULL, NULL, D3DX_FILTER_NONE, 0));

	// 结束
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
	// 付立叶变换点数
	LONG	count;

	// 循环变量
	int		i,j,k;

	// 中间变量
	int		bfsize,p;

	// 角度
	double	angle;

	complex<double> *W,*X1,*X2,*X;

	// 计算付立叶变换点数
	count = 1 << r;

	// 分配运算所需存储器
	W  = new complex<double>[count / 2];
	X1 = new complex<double>[count];
	X2 = new complex<double>[count];

	// 计算加权系数
	for(i = 0; i < count / 2; i++)
	{
		angle = (double)i * D3DX_PI * 2 / (double)count;
		W[i] = complex<double> (cos(angle), sin(angle));
	}

	// 将时域点写入X1
	memcpy(X1, TD, sizeof(complex<double>) * count);

	// 采用蝶形算法进行快速付立叶变换
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

	// 重新排序
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

	// 释放内存
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
UINT iLitNum = CheckPowerOf2(m_iWidth);	// 迭代次数

for(UINT iY = 0; iY < m_iHeight; iY++)
{
sprintf(szLog, "Y=%d:", iY);
fwrite(szLog, strlen(szLog), 1, fp);

p = (float *)((BYTE *)Rect.pBits + iY * Rect.Pitch);

// 读取一行，将源数据输入函数，用CPU做IFFT
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
// 将Result赋值给fValue，以便写入log
fValue[0] = (float)pResult[iX].real();
fValue[1] = (float)pResult[iX].imag();

// 存储fValue的时候注意格式，实虚部绝对值+符号标记
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

// 换行排版
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

// 结束
m_iCreateAttrib = 2;
return S_OK;
}
*/




























/************************2D Fluid Dynamics Simulation*************************/
/////////外部接口
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

	// 有效性检查
	for(UINT i = 0; i < iForceNum; i++)
	{
		if(!pInjectData[i].CheckAvailable(m_iWidth, m_iHeight))
			return D3DERR_INVALIDCALL;
		if(absf(D3DXVec2Length(&pInjectData[i].VecForceDir) - 1.0f) < 0.00001f)
		{
			OutputDebugString("外力F为0！非法！\n");
			return FALSE;
		}
	}

	// 释放原来的，创建新的
	SAFE_DELETE_ARRAY(m_pInjectForce);
	m_pInjectForce = new INJECT_ATTRIBUTE[iForceNum];
	if(!m_pInjectForce)
		return E_OUTOFMEMORY;

	
	// 拷贝数据
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

	// 有效性检查
	for(UINT i = 0; i < iSourceNum; i++)
	{
		if(!pInjectData[i].CheckAvailable(m_iWidth, m_iHeight))
			return D3DERR_INVALIDCALL;
		if(pInjectData[i].VecCentreIntensity.x < 0.0f || pInjectData[i].VecCentreIntensity.y < 0.0f || pInjectData[i].VecCentreIntensity.z < 0.0f)
		{
			OutputDebugString("来源S为负数！非法！\n");
			return FALSE;
		}
		/*暂时允许较大的来源
		if(pInjectData[i].VecCentreIntensity.x > 1.0f || pInjectData[i].VecCentreIntensity.y > 1.0f || pInjectData[i].VecCentreIntensity.z > 1.0f)
		{
			OutputDebugString("来源S过大！非法！\n");
			return FALSE;
		}
		*/
	}

	// 释放原来的，创建新的
	SAFE_DELETE_ARRAY(m_pInjectSource);
	m_pInjectSource = new INJECT_ATTRIBUTE[iSourceNum];
	if(!m_pInjectSource)
		return E_OUTOFMEMORY;

	// 拷贝数据
	m_iInjectSourceNum = iSourceNum;
	memcpy(m_pInjectSource, pInjectData, m_iInjectSourceNum * sizeof(INJECT_ATTRIBUTE));
	return S_OK;
}



HRESULT KFluidSimulation2D::ResetFluid()
{
	if(!m_pTexD || !m_pTexU || !m_pTexP)
		return D3DERR_NOTAVAILABLE;

	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便FFT渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// Clear
	V_RETURN(ClearTexture(m_pTexP));
	V_RETURN(ClearTexture(m_pTexD));
	V_RETURN(ClearTexture(m_pTexU));

	// 恢复RT和深度缓冲
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

	// 防止每帧连续设置相同的Obstacle图，导致的性能下降
	static IDirect3DTexture9* s_pTexture = NULL;

	if(!pTexObstacle)
	{
		s_pTexture = NULL;
		m_bABC = FALSE;
		return S_OK;
	}

	// 不支持GPU生成ObstacleTexture，至少需要76条指令，而且又没有手动设置Offset图，失败
	if(!CheckPS2xSupport(12, FALSE) && !pTexOffset)
		return D3DERR_NOTAVAILABLE;


	// 阻挡图分辨率必须和纹理一致！
	D3DSURFACE_DESC Desc;
	V_RETURN(pTexObstacle->GetLevelDesc(0, &Desc));
	if(Desc.Width != m_iWidth || Desc.Height != m_iHeight)
		return D3DERR_WRONGTEXTUREFORMAT;


	// 防止每帧连续设置相同的Obstacle图，导致的性能下降
	if(s_pTexture == pTexObstacle)
		return S_OK;
	else
		s_pTexture = pTexObstacle;


	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便FFT渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// 开始处理

	// 先转换为边界类型图，由于ComputeQuad不包括边界线，所以先整个Clear一下，置为可通过类型（值1）
	V_RETURN(ClearTexture(m_pTexABCBoundary, &D3DXVECTOR4(1, 1, 1, 1)));
	V_RETURN(CommonComputeQuad(pTexObstacle, NULL, NULL, m_pTexABCBoundary, &m_PSObstacleToBoundary));
	
/*	// 方法1：边界图到开口类型图，然后再转换为偏移坐标图，需要两个pass和ps2.a
	// 再转换为开口类型图，由于ComputeQuad不包括边界线，所以先整个Clear一下，置为无偏移类型（值0）
	V_RETURN(ClearTexture(m_pTexABCType, &D3DXVECTOR4(0, 0, 0, 0)));
	V_RETURN(CommonComputeQuad(m_pTexABCBoundary, NULL, NULL, m_pTexABCType, &m_PSBoundaryToType));

	V_RETURN(ClearTexture(m_pTexABCOffset, &D3DXVECTOR4(0.5f, 0.5f, 0.5f, 0.5f)));
	V_RETURN(CommonComputeQuad(m_pTexABCType, m_pTexABCTypeToOffset, NULL, m_pTexABCOffset, &m_PSTypeToOffset));
*/

	// 方法2：边界图直接转换为ABC偏移坐标图，注意偏移坐标要乘加0.5，所以要清为0.5
	V_RETURN(ClearTexture(m_pTexABCOffset, &D3DXVECTOR4(0.5f, 0.5f, 0.5f, 0.5f)));
		// 硬件支持的话就用GPU来计算，反之就复制手动设定的Offset图
	if(CheckPS2xSupport(12, FALSE))
	{
		V_RETURN(CommonComputeQuad(m_pTexABCBoundary, m_pTexABCBoundaryToOffset, NULL, m_pTexABCOffset, &m_PSBoundaryToOffset));
	}
	else
	{
		V_RETURN(CopyTexture(pTexOffset, m_pTexABCOffset));
	}


	// 保存GPU生成的Offset图
//	V_RETURN(D3DXSaveTextureToFile("Offset.dds", D3DXIFF_DDS, m_pTexABCOffset, NULL));



	// 恢复RT和深度缓冲
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);



	// 设置标记，结束
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

	// 液体就要计算法线
	if(m_FluidData.bLiquid)
		m_bGenerateNormal = TRUE;
	else
		m_bGenerateNormal = FALSE;

	// 每次调用模拟之前，渲染次数清零
	m_iRenderNumPerFrame = 0;


	UINT i = 0;

	// 索引清零，开始每一步运算
	m_iCurrentDIndex = m_iCurrentWIndex = 0;

	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便FFT渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// 开始处理
	// 由于在模拟过程中都是不处理边界线的，考虑到临近采样的步骤，所以要在计算前Apply边界条件，这样才不会计算出错值，更不会让它在临近采样中扩散出去
/*	for(i = 0; i < MAX_FLUID_STEPS; i++)
	{
		V_RETURN(ClearTexture(m_pTexWTemp[i]));
		V_RETURN(ClearTexture(m_pTexDTemp[i]));
	}
*/
	// 先处理速度场W
		// 第一步：平流（将当前U计算到W贴图中），平流牵扯临近采样，但因为平流只需计算一次，没有迭代，所以只在前更新一次边界条件即可
	V_RETURN(ApplyBoundaryCondition(FLUID_VELOCITY, m_pTexU));
	V_RETURN(Advect(FLUID_VELOCITY));

		// 第二步，计算黏度扩散（可选），从W计算到W，由于是迭代过程，所以在函数内更新边界条件
	if(m_FluidData.fViscousCoef > 0.000001f)
	{
//		V_RETURN(Diffusion(FLUID_W));
	}

		// 第三步，增加外力（只在边界内增加），它和临近采样无关，暂时不更新边界条件
	//	V_RETURN(ClearTexture(m_pTexTemp[0]));
	//	V_RETURN(ClearTexture(m_pTexTemp[1]));
	V_RETURN(ApplyForce());

		// 第四步，计算重力，重力大小和密度相关，它是持续进行的，并非像注入外力一样是脉冲
	V_RETURN(ApplyGravity());

		// 速度场W计算完毕，保存到m_pTexWTemp[m_iCurrentWIndex]中

	// 下来根据W来计算压力场
	//	V_RETURN(ClearTexture(m_pTexTemp[0]));
	//	V_RETURN(ClearTexture(m_pTexTemp[1]));

		// 求W散度及泊松压力方程，求W散度需要做临近采样，所以要提前更新边界条件
	V_RETURN(ApplyBoundaryCondition(FLUID_W, m_pTexWTemp[m_iCurrentWIndex]));
	V_RETURN(SolvePoissonPressureEquation());

		// 压力场P计算完毕，保存到m_pTexP中

	// 现在计算速度场U
		// 前面计算出的速度场W，现在将它投影处理到m_pTexU中保存起来，准备让后续步骤调用，同时也作为下一帧模拟的输入
		// 由于投影需要临近采样（计算P的梯度时），所以要提前对P更新边界条件
	V_RETURN(ApplyBoundaryCondition(FLUID_PRESSURE, m_pTexP));
	V_RETURN(Project());
		
		// 速度场U计算完毕，保存到m_pTexU中
	


	// 最后处理密度场
		// 第一步：平流（将当前D计算到DTemp中），平流牵扯临近采样，但因为平流只需计算一次，没有迭代，所以只在前更新一次边界条件即可
	V_RETURN(ApplyBoundaryCondition(FLUID_DENSITY, m_pTexD));
	V_RETURN(Advect(FLUID_DENSITY));
		// 第二步，处理黏度扩散，由于前面速度场已经处理了，这里就跳过
	if(m_FluidData.fViscousCoef > 0.000001f)
	{
//		V_RETURN(Diffusion(FLUID_DENSITY));
	}

		// 第三步，增加来源（只在边界内增加），它和临近采样无关，暂时不更新边界条件
	V_RETURN(ApplySource());

		// 计算出的最终密度场保存到了m_pTexDTemp[m_iCurrentDIndex]中，现在将它拷贝到m_pTexD中保存起来，准备让外界调用，同时也作为下一帧模拟的输入
		// 这一步也没有临近采样，不用更新边界条件
	V_RETURN(CopyTexture(m_pTexDTemp[m_iCurrentDIndex], m_pTexD));


	// 液体法线
	if(m_bGenerateNormal)
	{
		V_RETURN(GenerateNormal());
	}




	// 恢复RT和深度缓冲
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);


	// 所有步骤完成
	m_iCreateAttrib = 2;
	return S_OK;
}



/****************************内部接口--模拟过程**********************************/
HRESULT KFluidSimulation2D::Advect(enuFluid_VectorField Type)
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	if(Type == FLUID_VELOCITY)
	{
		// A是模拟过程的第一步，通过当前的（其实是上一次的模拟结果）U，计算到W[0]中
		float fDamp = 1.0f;	// 速度场不能加消散系数，所以必须为1
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(fDamp, fDamp, fDamp, fDamp), 1);
		V_RETURN(CommonComputeQuad(m_pTexU, m_pTexU, NULL, m_pTexWTemp[m_iCurrentWIndex], &m_PSAdvect_W));
	}
	else if(Type == FLUID_DENSITY)
	{
		// 通过当前D和已经计算好的U，计算出新的D
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
		// 根据密度场来增加流体的重力
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_FluidData.VecGravity.x, m_FluidData.VecGravity.y, 0, 0), 1);
		d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(m_FluidData.fGravityPower, m_FluidData.fGravityPower, m_FluidData.fGravityPower, m_FluidData.fGravityPower), 1);
		V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], m_pTexD, NULL, m_pTexWTemp[m_iCurrentWIndex+1], &m_PSGravity));
		m_iCurrentWIndex++;
	}

	// 结束
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
		// 根据W计算新的W，这是一个Jacobi迭代过程
		UINT iTempIndex = 0;

		// 第一次
		V_RETURN(ApplyBoundaryCondition(FLUID_W, m_pTexWTemp[m_iCurrentWIndex]));
		SET_CONSTANT;
		V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], m_pTexU, NULL, m_pTexTemp[iTempIndex], &m_PSJacobi_W));

		for(UINT i = 1; i < m_FluidData.iIterateNum; i++)
		{
			// 每次迭代前要更新边界条件
			V_RETURN(ApplyBoundaryCondition(FLUID_W, m_pTexTemp[iTempIndex]));

			// 最后一次，结束切换，计算到新W中
			if(i == (m_FluidData.iIterateNum - 1) )
			{
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], m_pTexU, NULL, m_pTexWTemp[m_iCurrentWIndex+1], &m_PSJacobi_W));
			}
			// 中间过程，在Temp中来回切换
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
		// 根据W和当前的D计算新的D，这是一个Jacobi迭代过程
		UINT iTempIndex = 0;

		// 第一次
		V_RETURN(ApplyBoundaryCondition(FLUID_DENSITY, m_pTexDTemp[m_iCurrentDIndex]));
		SET_CONSTANT;
		V_RETURN(CommonComputeQuad(m_pTexDTemp[m_iCurrentDIndex], NULL, NULL, m_pTexTemp[iTempIndex], &m_PSJacobi_D));

		for(UINT i = 1; i < m_FluidData.iIterateNum; i++)
		{
			// 每次迭代前要更新边界条件
			V_RETURN(ApplyBoundaryCondition(FLUID_DENSITY, m_pTexTemp[iTempIndex]));

			// 最后一次，结束切换，计算到新D中
			if(i == (m_FluidData.iIterateNum - 1) )
			{
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], NULL, NULL, m_pTexDTemp[m_iCurrentDIndex+1], &m_PSJacobi_D));
			}
			// 中间过程，在Temp中来回切换
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

	// 结束
	return S_OK;

#undef SET_CONSTANT
}







PIXELSHADER *KFluidSimulation2D::ChooseForceShader(enuInject_Type Type)
{
	PIXELSHADER *pPS = NULL;
	// 选择Shader
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
	// 选择Shader
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


	// 输入m_pTexWTemp[m_iCurrentWIndex]，输出m_pTexWTemp[m_iCurrentWIndex+1]，中间用TexTemp来循环渲染

	UINT iTempIndex = 0;
	float fTemp = 0.0f;
	UINT i = 0;


	// 如果是纹理注入器，就要设置相应的纹理
	IDirect3DTexture9* pTexInject = NULL;
	if(m_pInjectForce[i].Type == Inject_Texture)
		pTexInject = m_pInjectForce[i].pTexInjectForce;


		// 如果只有一处力，直接输出
	if(m_iInjectForceNum == 1)
	{
		SET_CONSTANT;
		V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], pTexInject, NULL, m_pTexWTemp[m_iCurrentWIndex+1], ChooseForceShader(m_pInjectForce[i].Type)));
	}
	else
	{
		// 如果有多处力，第一次先从WTemp输出到Temp中，然后每次反复，到处理最后一个力时，输出到WTemp
			// 第一次
		SET_CONSTANT;
		V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], pTexInject, NULL, m_pTexTemp[iTempIndex], ChooseForceShader(m_pInjectForce[i].Type)));

		for(i = 1; i < m_iInjectForceNum; i++)
		{
			// 最后一次
			if(i == (m_iInjectForceNum -1) )
			{
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], pTexInject, NULL, m_pTexWTemp[m_iCurrentWIndex+1], ChooseForceShader(m_pInjectForce[i].Type)));
			}
			// 中间过程，在Temp中来回切换
			else
			{
				// D是模拟过程的第二步，通过上一步W的计算结果，计算到新的W中
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], pTexInject, NULL, m_pTexTemp[!iTempIndex], ChooseForceShader(m_pInjectForce[i].Type)));
				iTempIndex = !iTempIndex;
			}
		}

	}

	m_iCurrentWIndex++;

	// 结束，记得清掉瞬时数据
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


	// 输入m_pTexDTemp[m_iCurrentDIndex]，输出m_pTexDTemp[m_iCurrentDIndex+1]，中间用TexTemp来循环渲染
	
	UINT iTempIndex = 0;
	float fTemp = 0.0f;
	UINT i = 0;

	// 如果是纹理注入器，就要设置相应的纹理
	IDirect3DTexture9* pTexInject = NULL;
	if(m_pInjectSource[i].Type == Inject_Texture)
		pTexInject = m_pInjectSource[i].pTexInjectSource;


	// 如果只有一处来源，直接输出
	if(m_iInjectSourceNum == 1)
	{
		SET_CONSTANT;
		V_RETURN(CommonComputeQuad(m_pTexDTemp[m_iCurrentDIndex], pTexInject, NULL, m_pTexDTemp[m_iCurrentDIndex+1], ChooseSourceShader(m_pInjectSource[i].Type)));
	}
	else
	{
		// 如果有多处来源，第一次先从DTemp输出到Temp中，然后每次反复，到处理最后一个来源时，输出到DTemp
		// 第一次
		SET_CONSTANT;
		V_RETURN(CommonComputeQuad(m_pTexDTemp[m_iCurrentDIndex], pTexInject, NULL, m_pTexTemp[iTempIndex], ChooseSourceShader(m_pInjectSource[i].Type)));

		for(i = 1; i < m_iInjectSourceNum; i++)
		{
			// 最后一次
			if(i == (m_iInjectSourceNum - 1) )
			{
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], pTexInject, NULL, m_pTexDTemp[m_iCurrentDIndex+1], ChooseSourceShader(m_pInjectSource[i].Type)));
			}
			// 中间过程，在Temp中来回切换
			else
			{
				// S是模拟过程的第三步，通过上一步D的计算结果，计算到新的D中
				SET_CONSTANT;
				V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], pTexInject, NULL, m_pTexTemp[!iTempIndex], ChooseSourceShader(m_pInjectSource[i].Type)));
				iTempIndex = !iTempIndex;
			}	
		}

	}

	m_iCurrentDIndex++;

	// 结束，记得清掉瞬时数据
	m_iInjectSourceNum = 0;
	SAFE_DELETE_ARRAY(m_pInjectSource);
	return S_OK;
#undef SET_CONSTANT
}









HRESULT KFluidSimulation2D::SolvePoissonPressureEquation()
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// 计算P，先计算W的散度（W此时已算好且已更新完边界条件）
	V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], NULL, NULL, m_pTexDivW, &m_PSDiv_W));
	
	// 根据散度，和上一次模拟中计算出来的P，计算新的P，这是一个Jacobi迭代过程，由于迭代中会使用临近采样（梯度），所以每次迭代前都要更新边界条件
	UINT iTempIndex = 0;
	
		// 第一次迭代
	V_RETURN(ApplyBoundaryCondition(FLUID_PRESSURE, m_pTexP));
	V_RETURN(CommonComputeQuad(m_pTexP, m_pTexDivW, NULL, m_pTexTemp[iTempIndex], &m_PSJacobi_P));



	for(UINT i = 1; i < m_FluidData.iIterateNum; i++)
	{
		// 每次迭代前要更新边界条件
		V_RETURN(ApplyBoundaryCondition(FLUID_PRESSURE, m_pTexTemp[iTempIndex]));

		// 最后一次，结束切换，计算到P中
		if(i == (m_FluidData.iIterateNum - 1) )
		{
			V_RETURN(CommonComputeQuad(m_pTexTemp[iTempIndex], m_pTexDivW, NULL, m_pTexP, &m_PSJacobi_P));
		}
		// 中间过程，在Temp中来回切换
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

	// 根据W和P计算U
	d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(1.0f / m_FluidData.fConsistensy, 1.0f / m_FluidData.fConsistensy, 1.0f / m_FluidData.fConsistensy, 1.0f / m_FluidData.fConsistensy), 1);
	V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], m_pTexP, NULL, m_pTexU, &m_PSProject));
	return S_OK;
}



HRESULT KFluidSimulation2D::RefineVorticity()
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// 先计算CurlW
	V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], NULL, NULL, m_pTexCurlW, &m_PSCurl_W));
	// 根据CurlW和W计算新的W
	V_RETURN(CommonComputeQuad(m_pTexWTemp[m_iCurrentWIndex], m_pTexCurlW, NULL, m_pTexWTemp[m_iCurrentWIndex+1], &m_PSVorticity));
	m_iCurrentWIndex++;

	return S_OK;
}





HRESULT KFluidSimulation2D::ApplyBoundaryCondition(enuFluid_VectorField Type, IDirect3DTexture9* pTexture)
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	UINT iBoundaryIndex = 0;

	// 注意这里要Clear一下，由于下面的Copy操作并不包括基本边界线，在边界处理中会采样到临近点，所以还是要把边界点处的值清为0
	V_RETURN(ClearTexture(m_pTexBCTemp));

	// 如果需要的话，先计算任意边界（任意边界包含基本边界区域，所以先绘制），从TexU到TexU
	if(m_bABC)
	{
		// 先复制到临时贴图中待用
		V_RETURN(CopyTexture(pTexture, m_pTexBCTemp));

		if(Type == FLUID_VELOCITY)
		{
			// 当前U和Offset一起计算为新的U
			V_RETURN(CommonComputeQuad(m_pTexBCTemp, m_pTexABCBoundary, m_pTexABCOffset, pTexture, &m_PSABC_U));
		}
		else if(Type == FLUID_W)
		{
			// 当前W和Offset一起计算为新的W
			V_RETURN(CommonComputeQuad(m_pTexBCTemp, m_pTexABCBoundary, m_pTexABCOffset, pTexture, &m_PSABC_U));
		}
		else if(Type == FLUID_PRESSURE)
		{
			// 当前P和Offset一起计算为新的P
			V_RETURN(CommonComputeQuad(m_pTexBCTemp, m_pTexABCBoundary, m_pTexABCOffset, pTexture, &m_PSABC_P));
		}
		else if(Type == FLUID_DENSITY)
		{
			// 当前P和障碍图Offset一起计算为新的D
			V_RETURN(CommonComputeQuad(m_pTexBCTemp, m_pTexABCBoundary, NULL, pTexture, &m_PSABC_D));
		}
	}


	// 再计算基本边界，由于这里只需要写入4条边界线，所以不需要迭代Copy

	// 偏移采样核，0～3分别表示上下左右边界线上的偏移（求方向导数用）
	D3DXVECTOR4 VecBBCOffset[4] = 
	{
		D3DXVECTOR4(0, 1, 0, 0),	// 上边界向下寻址
		D3DXVECTOR4(0, -1, 0, 0),	// 下边界向上寻址
		D3DXVECTOR4(1, 0, 0, 0),	// 左边界向右寻址
		D3DXVECTOR4(-1, 0, 0, 0),	// 右边界向左寻址
	};

	// 先复制到临时贴图中待用，如果要Disappear条件，就把常量寄存器6置为0
	V_RETURN(CopyTexture(pTexture, m_pTexBCTemp));

	if(Type == FLUID_VELOCITY)
	{
		for(iBoundaryIndex = 0; iBoundaryIndex < 4; iBoundaryIndex++)
		{
			// 当前U计算为新的U
			d3ddevice->SetPixelShaderConstantF(5, (float *)&VecBBCOffset[iBoundaryIndex], 1);
			d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(-1, -1, -1, -1), 1);	// No-Slip速度场边界速度必须趋于0，Scale给-1，如果是消失条件，就给0
			V_RETURN(CommonComputeLine(m_pTexBCTemp, NULL, NULL, pTexture, &m_PSBBC_U, iBoundaryIndex));
		}
	}
	else if(Type == FLUID_W)
	{
		for(iBoundaryIndex = 0; iBoundaryIndex < 4; iBoundaryIndex++)
		{
			// 当前W计算为新的W
			d3ddevice->SetPixelShaderConstantF(5, (float *)&VecBBCOffset[iBoundaryIndex], 1);
			d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(-1, -1, -1, -1), 1);	// No-Slip速度场边界速度必须趋于0，Scale给-1，如果是消失条件，就给0
			V_RETURN(CommonComputeLine(m_pTexBCTemp, NULL, NULL, pTexture, &m_PSBBC_U, iBoundaryIndex));
		}
	}
	else if(Type == FLUID_PRESSURE)
	{
		for(iBoundaryIndex = 0; iBoundaryIndex < 4; iBoundaryIndex++)
		{
			// 当前P计算为新的P
			d3ddevice->SetPixelShaderConstantF(5, (float *)&VecBBCOffset[iBoundaryIndex], 1);
			d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(1, 1, 1, 1), 1);	// 压力场边界压力变化率为0，也就是说要保持住Scale给1，如果是消失条件，就给0
			V_RETURN(CommonComputeLine(m_pTexBCTemp, NULL, NULL, pTexture, &m_PSBBC_P, iBoundaryIndex));
		}
	}
	else if(Type == FLUID_DENSITY)
	{
		for(iBoundaryIndex = 0; iBoundaryIndex < 4; iBoundaryIndex++)
		{
			// 当前D计算为新的D，注意D的基本边界上值全为0，所以这里的Pixel Shader直接用SetToZero，而且也不需要设置来源了
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

	// 根据D来计算Normal
	V_RETURN(CommonComputeQuad(m_pTexD, NULL, NULL, m_pTexNormal, &m_PSGenerateNormal));

	// 记得同时还要计算边界
	for(UINT i = 0; i < 4; i++)
		V_RETURN(CommonComputeLine(m_pTexD, NULL, NULL, m_pTexNormal, &m_PSGenerateNormal, i));

	return S_OK;
}











/****************************内部接口--公用**********************************/
HRESULT KFluidSimulation2D::CommonComputeQuad(IDirect3DTexture9* pSrcTex1, IDirect3DTexture9* pSrcTex2, IDirect3DTexture9* pSrcTex3, IDirect3DTexture9* pRT, PIXELSHADER *pPS)
{
	if(!m_pVBQuad || !m_pVBQuadExceptBoundary)
		return D3DERR_NOTAVAILABLE;

	if(!pRT || !pPS)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	UINT i = 0;

	// 设置渲染来源和目的
	V_RETURN(SetTexturedRenderTarget(0, pRT, NULL));

	V_RETURN(d3ddevice->SetTexture(0, pSrcTex1));
	V_RETURN(d3ddevice->SetTexture(1, pSrcTex2));
	V_RETURN(d3ddevice->SetTexture(2, pSrcTex3));

	// 设置渲染参数
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


	// 设置Declaration
	V_RETURN(d3ddevice->SetVertexShader(NULL));
	V_RETURN(d3ddevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE4(0) | D3DFVF_TEXCOORDSIZE4(1)));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_pDeclaration));

	// 设置PS及常量寄存器（1/2tap、1tap和TextureDimension，在c0、c1和c2）
	V_RETURN(pPS->SetPixelShader());
	V_RETURN(pPS->SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_iWidth, 1.0f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(2, &D3DXVECTOR4((float)m_iWidth, (float)m_iHeight, 0, 0), 1));
		// 一些常数和当前时间间隔
	V_RETURN(pPS->SetConstant(3, &D3DXVECTOR4(0, 0.5f, 1.0f, 2.0f), 1));
	V_RETURN(pPS->SetConstant(4, &D3DXVECTOR4(m_fDeltaTime, m_fDeltaTime, m_fDeltaTime, m_fDeltaTime), 1));


	// 开始渲染，记得只有Clear才用大矩形，模拟过程和Copy都用不带外框的小矩形（边界点是通过计算边界条件写入的，不需要Copy）
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

	// 恢复Pixel Shader
	V_RETURN(pPS->RestorePixelShader());

	// 完成，渲染次数加1
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

	// 设置渲染来源和目的
	V_RETURN(SetTexturedRenderTarget(0, pRT, NULL));

	V_RETURN(d3ddevice->SetTexture(0, pSrcTex1));
	V_RETURN(d3ddevice->SetTexture(1, pSrcTex2));
	V_RETURN(d3ddevice->SetTexture(2, pSrcTex3));

	// 设置渲染参数
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


	// 设置Declaration
	V_RETURN(d3ddevice->SetVertexDeclaration(m_pDeclaration));
	V_RETURN(d3ddevice->SetVertexShader(NULL));
	V_RETURN(d3ddevice->SetFVF(0));

	// 设置PS及常量寄存器（1/2tap、1tap和TextureDimension，在c0、c1和c2）
	V_RETURN(pPS->SetPixelShader());
	V_RETURN(pPS->SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_iWidth, 0.5f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_iWidth, 1.0f/(float)m_iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(2, &D3DXVECTOR4((float)m_iWidth, (float)m_iHeight, 0, 0), 1));
	// 一些常数和当前时间间隔
	V_RETURN(pPS->SetConstant(3, &D3DXVECTOR4(0, 0.5f, 1.0f, 2.0f), 1));
	V_RETURN(pPS->SetConstant(4, &D3DXVECTOR4(m_fDeltaTime, m_fDeltaTime, m_fDeltaTime, m_fDeltaTime), 1));

	// 开始渲染
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVBBoundaryLine[iLineNo], 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_LINELIST, 0, 1));
	V_RETURN(d3ddevice->EndScene());

	// 恢复Pixel Shader
	V_RETURN(pPS->RestorePixelShader());

	// 完成
	return S_OK;
}






/****************************初始化**********************************/

HRESULT KFluidSimulation2D::Init(UINT iWidth, UINT iHeight, BOOL bGenerateNormal /* = TRUE */)
{
	if(m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// 检查参数
	if(!iWidth || !iHeight)
		return D3DERR_INVALIDCALL;
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_bGenerateNormal = bGenerateNormal;

	m_iRenderNumPerFrame = 0;


	D3DFORMAT FormatFourChannel = D3DFMT_A8R8G8B8;
	// 创建和初始化数据贴图
#ifdef USE_FP16
	FormatFourChannel = D3DFMT_A16B16G16R16F;
#elif defined USE_FP32
	FormatFourChannel = D3DFMT_A32B32G32R32F;
#else
	mymessage("未定义使用浮点纹理，无法初始化流体模拟引擎！");
	return E_FAIL;
#endif

	// Temp贴图
	for(UINT i = 0; i < MAX_FLUID_STEPS; i++)
	{
		V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexWTemp[i], NULL));
		V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexDTemp[i], NULL));
	}
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTemp[0], NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTemp[1], NULL));

	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexBCTemp, NULL));

	// 最终保存结果的贴图
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexP, NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexU, NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexD, NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexNormal, NULL));

	// Offset经过乘加0.5后无非只有0、0.5和1三种值，没必要用浮点贴图
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexABCOffset, NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexABCBoundary, NULL));	
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexABCType, NULL));

	// 存放中间特定结果的贴图
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexDivW, NULL));
	V_RETURN(d3ddevice->CreateTexture(iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexCurlW, NULL));

	



	// 初始化ABC类型对应的偏移点坐标转换图（共13种边界开口类型）。由于Offset只有-1 0 1三种，所以写入的的时候要乘加0.5
	// 由于这两张1D贴图都比较小，为了写入数据的精确起见，这里用FP32纹理
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
			// 乘加0.5，把坐标偏移值映射为0～1
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
			// 先确定12种边界所在值对应的偏移量
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

			// 不属于上述边界值的，一律给0
			default:
				VecBoundaryToOffset = D3DXVECTOR4(0, 0, 0, 0);
			break;
		}

		for(UINT i = 0; i < 4; i++)
		{
			// 乘加0.5，把坐标偏移值映射为0～1
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


	// 初始化Vertex Buffer，一个Quad，四个Line
	m_iStride = sizeof(VERTEXTYPE);
	V_RETURN(d3ddevice->CreateVertexBuffer(4 * m_iStride, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVBQuad, NULL));

//	顶点位置：	1	3
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
		// 上边界13
		{0, 0,									0, 1,		0,0,0,0,		0,0,0,0},
		{(float)m_iWidth-1, 0,					0, 1,		1-1.0f/(float)m_iWidth,0,0,0,		(float)m_iWidth-1,0,0,0},
		// 下边界02
		{0, (float)m_iHeight-1,					0, 1,		0,1-1.0f/(float)m_iHeight,0,0,		0,(float)m_iHeight-1,0,0},
		{(float)m_iWidth-1, (float)m_iHeight-1,		0, 1,		1-1.0f/(float)m_iWidth,1-1.0f/(float)m_iHeight,0,0,		(float)m_iWidth-1,(float)m_iHeight-1,0,0},
		// 左边界10
		{0, 0,									0, 1,		0,0,0,0,		0,0,0,0},
		{0, (float)m_iHeight-1,					0, 1,		0,1-1.0f/(float)m_iHeight,0,0,		0,(float)m_iHeight-1,0,0},
		// 右边界32
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




	// 创建深度缓冲
	D3DSURFACE_DESC DescDepth;
	LPDIRECT3DSURFACE9 pOldDepth = NULL;

	if(FAILED(d3ddevice->GetDepthStencilSurface(&pOldDepth)))
		return E_FAIL;
	if(FAILED(pOldDepth->GetDesc(&DescDepth)))
		return E_FAIL;
	SAFE_RELEASE(pOldDepth);

	V_RETURN(d3ddevice->CreateDepthStencilSurface(iWidth, iHeight, DescDepth.Format, DescDepth.MultiSampleType, DescDepth.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL));





	// 初始化Shader
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
/*漩涡重建，暂时不支持	V_RETURN(m_PSCurl_W.InitPixelShader("shader\\Fluid\\NSE\\Curl_W.psh"));
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

		// 下面两个暂时不用
	//V_RETURN(m_PSBoundaryToType.InitPixelShader("shader\\Fluid\\Obstacle\\BoundaryToType.psh"));
	//V_RETURN(m_PSTypeToOffset.InitPixelShader("shader\\Fluid\\Obstacle\\TypeToOffset.psh"));




	// 初始时自动重设流体状态
	V_RETURN(ResetFluid());



	// 成功
	m_iCreateAttrib = 1;
	return S_OK;
}