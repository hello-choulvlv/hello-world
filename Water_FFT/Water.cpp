#include "myd3d.h"
#include "Water.h"







/************************Ocean Water Simulation*************************/
/////////外部接口
KFFTOceanWater::KFFTOceanWater()
{
	m_iCreateAttrib = 0;
	m_iStride = 0;
	m_bGenerateNormal = m_bGenerateTangent = FALSE;

	m_pTexH = m_ppTexH0[0] = m_ppTexH0[1] = NULL;
	m_pTexHeight = m_pTexNormal = m_pTexTangent = NULL;
	m_pDepthBuffer = NULL;
	m_pVB = NULL;
}

void KFFTOceanWater::Release()
{
	m_iCreateAttrib = 0;
	m_iStride = 0;
	m_bGenerateNormal = m_bGenerateTangent = FALSE;

	SAFE_RELEASE(m_pDepthBuffer);

	SAFE_RELEASE(m_pTexH);
	SAFE_RELEASE(m_ppTexH0[0]);
	SAFE_RELEASE(m_ppTexH0[1]);

	SAFE_RELEASE(m_pTexHeight);
	SAFE_RELEASE(m_pTexNormal);
	SAFE_RELEASE(m_pTexTangent);

	SAFE_RELEASE(m_pVB);

	m_VSDrawQuad.Release();
	m_IFFT.Release();

	m_PSGenerateH.Release();
	m_PSAddHeightMap.Release();
	m_PSGenerateNormal.Release();
	m_PSGenerateTangent.Release();
	m_PSGenerateNormalandTangent.Release();
}



HRESULT KFFTOceanWater::WaterSimulation(float fTime, LPDIRECT3DTEXTURE9 pTexAddonHeightMap /* = NULL */)
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// 先生成H
	V_RETURN(GenerateH(fTime));

	// 再做IFFT，得到海浪高度图
	V_RETURN(m_IFFT.IFFT(m_pTexH));

	// 在源图上叠加指定的海浪高度图到m_pTexHeight，如果没有叠加的，就直接将源复制到m_pTexHeight
	if(pTexAddonHeightMap)
	{
		V_RETURN(AddHeightMap(pTexAddonHeightMap));
	}
	else
	{
		V_RETURN(m_IFFT.GetResultData(m_pTexHeight, FALSE));
	}

	// 生成法线图
	// 都需要生成且支持MRT时
	if(m_bGenerateNormal && m_bGenerateTangent && d3dcaps.NumSimultaneousRTs > 1)
	{
		V_RETURN(GenerateNormalandTangent());
	}
	else
	{
		if(m_bGenerateNormal)
			V_RETURN(GenerateNormal());
		if(m_bGenerateTangent)
			V_RETURN(GenerateTangent());
	}

	// 完成
	m_iCreateAttrib = 2;
	return S_OK;
}





HRESULT KFFTOceanWater::AddHeightMap(LPDIRECT3DTEXTURE9 pTexAddonHeightMap)
{
	if(!m_iCreateAttrib || !m_IFFT.GetResultTexture() || !m_pTexHeight || !pTexAddonHeightMap)
		return D3DERR_NOTAVAILABLE;

	// 附加贴图有效性检查，必须是4通道（不一定是浮点），不能是内存纹理
	D3DSURFACE_DESC Desc;
	V_RETURN(pTexAddonHeightMap->GetLevelDesc(0, &Desc));
	if(Desc.Pool == D3DPOOL_SYSTEMMEM)
		return D3DERR_WRONGTEXTUREFORMAT;
	if(Desc.Format != D3DFMT_A8R8G8B8 && Desc.Format != D3DFMT_A16B16G16R16 && Desc.Format != D3DFMT_A16B16G16R16F && Desc.Format != D3DFMT_A32B32G32R32F)
		return D3DERR_WRONGTEXTUREFORMAT;


	HRESULT hr = S_OK;
	UINT i = 0;

	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// 设置渲染来源和目的
	V_RETURN(SetTexturedRenderTarget(0, m_pTexHeight, NULL));
	V_RETURN(d3ddevice->SetTexture(0, m_IFFT.GetResultTexture()));
	V_RETURN(d3ddevice->SetTexture(1, pTexAddonHeightMap));

	// 设置渲染参数
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE));


	// 设置Shader
	V_RETURN(m_PSAddHeightMap.SetPixelShader());
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));

	// 设置常量寄存器
	// 0.5 tap
	V_RETURN(m_PSAddHeightMap.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_OceanData.iWidth, 0.5f/(float)m_OceanData.iHeight, 0, 0), 1));

	// 开始渲染
	V_RETURN(d3ddevice->SetFVF(0));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// 恢复Pixel Shader和RT、深度缓冲
	V_RETURN(m_PSAddHeightMap.RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	// 完成
	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);
	return S_OK;
}



/********************生成法线切线图的内部函数*********************/

HRESULT KFFTOceanWater::GenerateTangent()
{
	if(!m_iCreateAttrib || !m_pTexHeight)
		return D3DERR_NOTAVAILABLE;

	HRESULT hr = S_OK;
	UINT i = 0;

	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// 设置渲染来源和目的
	V_RETURN(SetTexturedRenderTarget(0, m_pTexTangent, NULL));
	V_RETURN(d3ddevice->SetTexture(0, m_pTexHeight));

	// 设置渲染参数
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	// 设置Shader
	V_RETURN(m_PSGenerateTangent.SetPixelShader());
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));

	// 设置常量寄存器
	// 0.5 tap
	V_RETURN(m_PSGenerateTangent.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_OceanData.iWidth, 0.5f/(float)m_OceanData.iHeight, 0, 0), 1));
	// 1 tap
	V_RETURN(m_PSGenerateTangent.SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_OceanData.iWidth, 1.0f/(float)m_OceanData.iHeight, 0, 0), 1));
	// 海水面积
	V_RETURN(m_PSGenerateTangent.SetConstant(2, &D3DXVECTOR4(m_OceanData.WaterSquare.x, m_OceanData.WaterSquare.y, 0, 0), 1));
	// 高度缩放
	V_RETURN(m_PSGenerateTangent.SetConstant(3, &D3DXVECTOR4(m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale), 1));



	// 开始渲染
	V_RETURN(d3ddevice->SetFVF(0));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// 恢复Pixel Shader和RT、深度缓冲
	V_RETURN(m_PSGenerateTangent.RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	// 完成
	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);
	return S_OK;
}

HRESULT KFFTOceanWater::GenerateNormal()
{
	if(!m_iCreateAttrib || !m_pTexHeight)
		return D3DERR_NOTAVAILABLE;

	HRESULT hr = S_OK;
	UINT i = 0;

	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// 设置渲染来源和目的
	V_RETURN(SetTexturedRenderTarget(0, m_pTexNormal, NULL));
	V_RETURN(d3ddevice->SetTexture(0, m_pTexHeight));

	// 设置渲染参数
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	// 设置Shader
	V_RETURN(m_PSGenerateNormal.SetPixelShader());
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));

	// 设置常量寄存器
	// 0.5 tap
	V_RETURN(m_PSGenerateNormal.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_OceanData.iWidth, 0.5f/(float)m_OceanData.iHeight, 0, 0), 1));
	// 1 tap
	V_RETURN(m_PSGenerateNormal.SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_OceanData.iWidth, 1.0f/(float)m_OceanData.iHeight, 0, 0), 1));
	// 海水面积
	V_RETURN(m_PSGenerateNormal.SetConstant(2, &D3DXVECTOR4(m_OceanData.WaterSquare.x, m_OceanData.WaterSquare.y, 0, 0), 1));
	// 高度缩放
	V_RETURN(m_PSGenerateNormal.SetConstant(3, &D3DXVECTOR4(m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale), 1));



	// 开始渲染
	V_RETURN(d3ddevice->SetFVF(0));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// 恢复Pixel Shader和RT、深度缓冲
	V_RETURN(m_PSGenerateNormal.RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	// 完成
	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);
	return S_OK;
}


HRESULT KFFTOceanWater::GenerateNormalandTangent()
{
	if(!m_iCreateAttrib || !m_pTexHeight)
		return D3DERR_NOTAVAILABLE;

	HRESULT hr = S_OK;
	UINT i = 0;

	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer[2] = {NULL, NULL}, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer[0]));
	d3ddevice->GetRenderTarget(1, &pOldBackBuffer[1]);	// 没有MRT的话会自动返回D3DERR_NOTFOUND的！但并非错误
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// 设置渲染来源和目的（MRT）
	V_RETURN(SetTexturedRenderTarget(0, m_pTexNormal, NULL));
	V_RETURN(SetTexturedRenderTarget(1, m_pTexTangent, NULL));
	V_RETURN(d3ddevice->SetTexture(0, m_pTexHeight));

	// 设置渲染参数
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	// 设置Shader
	V_RETURN(m_PSGenerateNormalandTangent.SetPixelShader());
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));

	// 设置常量寄存器
	// 0.5 tap
	V_RETURN(m_PSGenerateNormalandTangent.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_OceanData.iWidth, 0.5f/(float)m_OceanData.iHeight, 0, 0), 1));
	// 1 tap
	V_RETURN(m_PSGenerateNormalandTangent.SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_OceanData.iWidth, 1.0f/(float)m_OceanData.iHeight, 0, 0), 1));
	// 海水面积
	V_RETURN(m_PSGenerateNormalandTangent.SetConstant(2, &D3DXVECTOR4(m_OceanData.WaterSquare.x, m_OceanData.WaterSquare.y, 0, 0), 1));
	// 高度缩放
	V_RETURN(m_PSGenerateNormalandTangent.SetConstant(3, &D3DXVECTOR4(m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale), 1));



	// 开始渲染
	V_RETURN(d3ddevice->SetFVF(0));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// 恢复Pixel Shader和RT、深度缓冲
	V_RETURN(m_PSGenerateNormalandTangent.RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer[0]));
	V_RETURN(d3ddevice->SetRenderTarget(1, pOldBackBuffer[1]));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	// 完成
	SAFE_RELEASE(pOldBackBuffer[0]);
	SAFE_RELEASE(pOldBackBuffer[1]);
	SAFE_RELEASE(pOldDepthBuffer);
	return S_OK;
}





HRESULT KFFTOceanWater::GenerateH(float fTime)
{
	if(!m_iCreateAttrib || !m_ppTexH0[0] || !m_ppTexH0[1])
		return D3DERR_NOTAVAILABLE;

	HRESULT hr = S_OK;
	UINT i = 0;

	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// 设置渲染来源和目的
	V_RETURN(SetTexturedRenderTarget(0, m_pTexH, NULL));
	V_RETURN(d3ddevice->SetTexture(0, m_ppTexH0[0]));
	V_RETURN(d3ddevice->SetTexture(1, m_ppTexH0[1]));

	// 设置渲染参数
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	// 设置Shader
	V_RETURN(m_PSGenerateH.SetPixelShader());
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));

	// 设置常量寄存器
	// 0.5 tap
	V_RETURN(m_PSGenerateH.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_OceanData.iWidth, 0.5f/(float)m_OceanData.iHeight, 0, 0), 1));
	// Current Time
	V_RETURN(m_PSGenerateH.SetConstant(1, &D3DXVECTOR4(fTime, fTime, fTime, fTime), 1));
	// D3D SinCos Constant
	V_RETURN(m_PSGenerateH.SetConstant(30, &D3DXVECTOR4(D3DSINCOSCONST1), 1));
	V_RETURN(m_PSGenerateH.SetConstant(31, &D3DXVECTOR4(D3DSINCOSCONST2), 1));


	// 开始渲染
	V_RETURN(d3ddevice->SetFVF(0));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// 恢复Pixel Shader和RT、深度缓冲
	V_RETURN(m_PSGenerateH.RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	// 完成
	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);
	return S_OK;
}





/****************************初始化**********************************/

HRESULT KFFTOceanWater::Init(OCEANWATER_ATTRIBUTE OceanData, BOOL bGenerateNormal /* = TRUE */, BOOL bGenerateTangent /* = TRUE */)
{
	if(m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// 检查海浪数据是否合法
	if(!OceanData.CheckAvaliable())
		return D3DERR_INVALIDCALL;
	m_OceanData = OceanData;

	// 不用检查是否2的幂，如果非法，初始化IFFT会失败
	V_RETURN(m_IFFT.Init(m_OceanData.iWidth, m_OceanData.iHeight));

	m_bGenerateNormal = bGenerateNormal;
	m_bGenerateTangent = bGenerateTangent;


	D3DFORMAT FormatFourChannel = D3DFMT_A8R8G8B8;
	// 创建和初始化数据贴图
#ifdef USE_FP16
	FormatFourChannel = D3DFMT_A16B16G16R16F;
#elif defined USE_FP32
	FormatFourChannel = D3DFMT_A32B32G32R32F;
#else
	mymessage("未定义使用浮点纹理，无法初始化海浪引擎！");
	return E_FAIL;
#endif

	V_RETURN(d3ddevice->CreateTexture(m_OceanData.iWidth, m_OceanData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexH, NULL));

	// 注意这里，这三张是要供给外界使用的，无论是用CPU Lock还是用VTF，都只能支持FP32的贴图！
	V_RETURN(d3ddevice->CreateTexture(m_OceanData.iWidth, m_OceanData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pTexHeight, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_OceanData.iWidth, m_OceanData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pTexNormal, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_OceanData.iWidth, m_OceanData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pTexTangent, NULL));

	V_RETURN(InitH0());


	// 初始化Quad
	V_RETURN(d3ddevice->CreateVertexBuffer(4*sizeof(QUADVERTEXTYPE), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVB, NULL));

	QUADVERTEXTYPE QuadVertex[4] = 
	{
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
	V_RETURN(m_PSGenerateH.InitPixelShader("shader\\OceanWaterSimulation\\GenerateH.psh"));
	V_RETURN(m_PSAddHeightMap.InitPixelShader("shader\\OceanWaterSimulation\\AddHeightMap.psh"));

	// 根据要求和系统是否支持MRT来初始化不同的Shader
	if(m_bGenerateNormal && m_bGenerateTangent && d3dcaps.NumSimultaneousRTs > 1)
	{
		V_RETURN(m_PSGenerateNormalandTangent.InitPixelShader("shader\\OceanWaterSimulation\\GenerateNormalandTangent.psh"));
	}
	else
	{
		if(m_bGenerateNormal)
			V_RETURN(m_PSGenerateNormal.InitPixelShader("shader\\OceanWaterSimulation\\GenerateNormal.psh"));
		if(m_bGenerateTangent)
			V_RETURN(m_PSGenerateTangent.InitPixelShader("shader\\OceanWaterSimulation\\GenerateTangent.psh"));
	}


	// 创建深度缓冲
	D3DSURFACE_DESC DescDepth;
	LPDIRECT3DSURFACE9 pOldDepth = NULL;

	if(FAILED(d3ddevice->GetDepthStencilSurface(&pOldDepth)))
		return E_FAIL;
	if(FAILED(pOldDepth->GetDesc(&DescDepth)))
		return E_FAIL;
	SAFE_RELEASE(pOldDepth);

	V_RETURN(d3ddevice->CreateDepthStencilSurface(m_OceanData.iWidth, m_OceanData.iHeight, DescDepth.Format, DescDepth.MultiSampleType, DescDepth.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL));


	// 成功
	m_iCreateAttrib = 1;
	return S_OK;
}








D3DXVECTOR2 KFFTOceanWater::CalcH0(int iX, int iY)
{
	D3DXVECTOR2 Result = D3DXVECTOR2(0, 0);

	// 计算用数据
	float fPh = 0.0f;	// Phillips Spectrum
	D3DXVECTOR2 fGaussRandom = D3DXVECTOR2(1.0f, 0.0f);	// 高斯随机数
	D3DXVECTOR2 VecK = D3DXVECTOR2(0, 0);
	float fL = 0.0f, fk2 = 0.0f;
	float fKDotW2 = 0.0f;
	float fTemp = 0.0f;

	// 先计算规格化K和模k^2
	VecK.x = (float)iX / (float)m_OceanData.iWidth;
	VecK.y = (float)iY / (float)m_OceanData.iHeight;
	fk2 = D3DXVec2Dot(&VecK, &VecK);
	D3DXVec2Normalize(&VecK, &VecK);

	// 计算最大风速下的波长L
	fL = m_OceanData.fWindSpeed * m_OceanData.fWindSpeed / GravityConstant;

	// 计算风向和当前波浪方向的Dot
	fKDotW2 = D3DXVec2Dot(&VecK, &m_OceanData.VecWindDir) * D3DXVec2Dot(&VecK, &m_OceanData.VecWindDir);

	// 计算Ph
	fTemp = -1.0f / (fk2 * fL * fL);	// e的指数
	fPh = expf(fTemp);
	fPh /= (fk2 * fk2);					// 分母，k^4
	fPh *= fKDotW2;

	// 计算随机数，是一个复数
	fGaussRandom = D3DXVECTOR2( (float)gaussrandom(), (float)gaussrandom() );

	// 把前面的计算合成到Ph中
	fPh = sqrtf(fPh / 2.0f);

	// 合成Ph和random，即复数乘法，并写入Result
	Result.x = fPh * fGaussRandom.x;
	Result.y = fPh * fGaussRandom.y;

	// 结束
	return Result;
}

HRESULT KFFTOceanWater::InitH0()
{
	if(!m_OceanData.CheckAvaliable())
		return D3DERR_NOTAVAILABLE;

	SAFE_RELEASE(m_ppTexH0[0]);
	SAFE_RELEASE(m_ppTexH0[1]);


	// 这张贴图在高分辨率下初始化时间需要很久，用FP16不明智
	D3DFORMAT FormatFourChannel = D3DFMT_A32B32G32R32F;

	V_RETURN(d3ddevice->CreateTexture(m_OceanData.iWidth, m_OceanData.iHeight, 1, D3DUSAGE_0, FormatFourChannel, D3DPOOL_MANAGED, &m_ppTexH0[0], NULL));
	V_RETURN(d3ddevice->CreateTexture(m_OceanData.iWidth, m_OceanData.iHeight, 1, D3DUSAGE_0, FormatFourChannel, D3DPOOL_MANAGED, &m_ppTexH0[1], NULL));


	HRESULT hr = S_OK;
	UINT i = 0, iX = 0, iY = 0;
	D3DLOCKED_RECT Rect, Rect_Oppo;
	float16 *p16 = NULL, *p16_Oppo = NULL;
	float *p = NULL, *p_Oppo = NULL;


	// Lock，Ready? Go!
	V_RETURN(m_ppTexH0[0]->LockRect(0, &Rect, NULL, 0));
	p16 = (float16 *)((BYTE *)Rect.pBits);	
	p = (float *)((BYTE *)Rect.pBits);

	V_RETURN(m_ppTexH0[1]->LockRect(0, &Rect_Oppo, NULL, 0));
	p16_Oppo = (float16 *)((BYTE *)Rect.pBits);	
	p_Oppo = (float *)((BYTE *)Rect.pBits);


	D3DXVECTOR2 fH0;	// 用于得到H0的值
	D3DXVECTOR4 fData, fData_Oppo;	// 保存的向量，格式为实虚部+实虚部符号（0或2）


	// 当前迭代次数中，是多少个像素一组
	for(iY = 0; iY < m_OceanData.iHeight; iY++)
	{
		p16 = (float16 *)((BYTE *)Rect.pBits + iY * Rect.Pitch);
		p = (float *)((BYTE *)Rect.pBits + iY * Rect.Pitch);
		p16_Oppo = (float16 *)((BYTE *)Rect_Oppo.pBits + iY * Rect_Oppo.Pitch);
		p_Oppo = (float *)((BYTE *)Rect_Oppo.pBits + iY * Rect_Oppo.Pitch);

		for(iX = 0; iX < m_OceanData.iWidth; iX++)
		{
			// 计算h0(K)，加1是为了将0去掉
			fH0 = CalcH0((int)iX+1, (int)iY+1);
			fData.x = absf(fH0.x);
			fData.y = absf(fH0.y);
			fData.z = fH0.x < 0.0f ? 0.0f : 2.0f;
			fData.w = fH0.y < 0.0f ? 0.0f : 2.0f;

			// 计算h0(-K)
			fH0 = CalcH0(-(int)iX-1, -(int)iY-1);
			fData_Oppo.x = absf(fH0.x);
			fData_Oppo.y = absf(fH0.y);
			fData_Oppo.z = fH0.x < 0.0f ? 0.0f : 2.0f;
			fData_Oppo.w = fH0.y < 0.0f ? 0.0f : 2.0f;


			// 写入贴图
			for(i = 0; i < 4; i++)
			{
				*p++ = fData[i];
				*p_Oppo++ = fData_Oppo[i];
			}// end for Write Texel
		}// end for X
	}

	// 结束
	V_RETURN(m_ppTexH0[0]->UnlockRect(0));
	V_RETURN(m_ppTexH0[1]->UnlockRect(0));

	return S_OK;
}































/****************************基于波动方程的水体模拟**********************************/
/////////外部接口
KWaveEquationWater::KWaveEquationWater()
{
	m_iCreateAttrib = 0;
	m_bGenerateNormal = m_bGenerateTangent = FALSE;
	m_iStride = 0;
	m_fDeltaTime = 0;

	m_pVBQuad = m_pVBQuadUser = m_pVBPoint = NULL;
	
	m_pDeclaration = NULL;
	m_pDepthBuffer = NULL;

	m_pTexPrev = m_pTexNow = m_pTexHeight = NULL;
	m_pTexAreaDamping = m_pTexTemp = NULL;
	m_pTexNormal = m_pTexTangent = NULL;
}

void KWaveEquationWater::Release()
{
	m_iCreateAttrib = 0;
	m_bGenerateNormal = m_bGenerateTangent = FALSE;
	m_iStride = 0;
	m_fDeltaTime = 0;

	SAFE_RELEASE(m_pVBQuad);
	SAFE_RELEASE(m_pVBQuadUser);
	SAFE_RELEASE(m_pVBPoint);

	SAFE_RELEASE(m_pDeclaration);
	SAFE_RELEASE(m_pDepthBuffer);

	SAFE_RELEASE(m_pTexPrev);
	SAFE_RELEASE(m_pTexNow);
	SAFE_RELEASE(m_pTexHeight);
	
	SAFE_RELEASE(m_pTexTemp);
	SAFE_RELEASE(m_pTexAreaDamping);

	SAFE_RELEASE(m_pTexNormal);
	SAFE_RELEASE(m_pTexTangent);

	// Shader
	m_PSClearTexture.Release();
	m_PSCopyTexture.Release();

	m_PSGenerateNormal.Release();
	m_PSGenerateTangent.Release();
	m_PSGenerateNormalandTangent.Release();

	m_PSWaveEquation.Release();
	m_PSWaveEquationWithDampTexture.Release();
	
	m_PSSetPointHeight.Release();
	m_PSSetAreaHeight.Release();
	m_PSAddPointHeight.Release();
	m_PSAddAreaHeight.Release();

		// ABC
	SAFE_RELEASE(m_pTexABCBoundaryToOffset);
	SAFE_RELEASE(m_pTexABCBoundary);
	SAFE_RELEASE(m_pTexABCOffset);
	m_PSObstacleToBoundary.Release();
	m_PSBoundaryToOffset.Release();
	m_PSABC_Bounce.Release();
}






HRESULT KWaveEquationWater::ResetWave() 
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便FFT渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));

	// 清零
	V_RETURN(ClearTexture(m_pTexHeight, &D3DXVECTOR4(0, 0, 0, 0)));
	V_RETURN(ClearTexture(m_pTexPrev, &D3DXVECTOR4(0, 0, 0, 0)));
	V_RETURN(ClearTexture(m_pTexNow, &D3DXVECTOR4(0, 0, 0, 0)));

	V_RETURN(ClearTexture(m_pTexNormal, &D3DXVECTOR4(0.5f, 0.5f, 0.5f, 0)));
	V_RETURN(ClearTexture(m_pTexTangent, &D3DXVECTOR4(0.5f, 0.5f, 0.5f, 0)));

	// 恢复RT和深度缓冲
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);

	return S_OK;
}




HRESULT KWaveEquationWater::SetPointHeight(UINT iPointNum, UINT *pX, UINT *pY, float *pHeight, BOOL bAddHeight )
{
	if(!m_pVBPoint)
		return D3DERR_NOTAVAILABLE;

	if(!iPointNum || !pX || !pY || !pHeight)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	UINT i = 0;


	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便FFT渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// 锁定Point缓冲
	BYTE *pData = NULL;
	float *pWrite = NULL;
	V_RETURN(m_pVBPoint->Lock(0, iPointNum * m_iStride, (void **)&pData, D3DLOCK_DISCARD));


	// 开始处理每个点，构造PointList顶点缓冲
	for(i = 0; i < iPointNum; i++)
	{
		// 超过范围，跳过
		if(pX[i] >= m_WaveData.iWidth || pY[i] >= m_WaveData.iHeight)
			continue;
		
		pWrite = (float *)pData;

		// 更新XYZRHW		
		*pWrite++ = (float)pX[i];
		*pWrite++ = (float)pY[i];
		*pWrite++ = 0.0f;
		*pWrite++ = 1.0f;

		// 第一个纹理坐标
		*pWrite++ = (float)pX[i] / (float)m_WaveData.iWidth;
		*pWrite++ = (float)pY[i] / (float)m_WaveData.iHeight;
		*pWrite++ = 0.0f;
		*pWrite++ = 0.0f;

		// 第二个纹理坐标保存写入的数据x = abs(Height), z = sign(Height)
		*pWrite++ = absf(pHeight[i]);
		pWrite++;
		*pWrite++ = (pHeight[i] > 0.0f ? 2.0f : 0.0f);
		pWrite++;

		// 跳到下一个点
		pData += m_iStride;
	}

	V_RETURN(m_pVBPoint->Unlock());

	// 绘制，写入纹理
	if(bAddHeight)
	{
		V_RETURN(CopyTexture(m_pTexTemp, m_pTexHeight));
		V_RETURN(CommonComputePoint(iPointNum, m_pTexTemp, NULL, NULL, m_pTexHeight, &m_PSAddPointHeight));
	}
	else
	{
		V_RETURN(CommonComputePoint(iPointNum, NULL, NULL, NULL, m_pTexHeight, &m_PSSetPointHeight));
	}



	// 恢复RT和深度缓冲
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);


	// 完成
	return S_OK;
}







HRESULT KWaveEquationWater::SetAreaHeight(UINT iX, UINT iY, D3DXVECTOR2 VecRange, LPDIRECT3DTEXTURE9 pTexArea, float fHeight, BOOL bAddHeight )
{
	if(!m_pVBQuadUser)
		return D3DERR_NOTAVAILABLE;

	if(iX >= m_WaveData.iWidth || iY >= m_WaveData.iHeight || !pTexArea)
		return D3DERR_INVALIDCALL;
	if(VecRange.x <= 0.0f || VecRange.x > 1.0f || VecRange.y <= 0.0f || VecRange.y > 1.0f)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	UINT i = 0;


	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便FFT渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// 锁定Rect缓冲
	BYTE *pData = NULL;
	float *pWrite = NULL;
	V_RETURN(m_pVBQuadUser->Lock(0, 4 * m_iStride, (void **)&pData, D3DLOCK_DISCARD));

	UINT iDeltaWidth = (UINT)(m_WaveData.iWidth * VecRange.x / 2.0f);
	UINT iDeltaHeight = (UINT)(m_WaveData.iHeight * VecRange.y / 2.0f);
	UINT iWidth = iDeltaWidth * 2, iHeight = iDeltaHeight * 2;

	D3DXVECTOR2 TexCoordLeftUp((float)(iX-iDeltaWidth) / (float)m_WaveData.iWidth,		(float)(iY-iDeltaHeight) / (float)m_WaveData.iHeight);
	D3DXVECTOR2 TexCoordLeftDown((float)(iX-iDeltaWidth) / (float)m_WaveData.iWidth,		(float)(iY+iDeltaHeight) / (float)m_WaveData.iHeight);
	D3DXVECTOR2 TexCoordRightUp((float)(iX+iDeltaWidth) / (float)m_WaveData.iWidth,		(float)(iY-iDeltaHeight) / (float)m_WaveData.iHeight);
	D3DXVECTOR2 TexCoordRightDown((float)(iX+iDeltaWidth) / (float)m_WaveData.iWidth,		(float)(iY+iDeltaHeight) / (float)m_WaveData.iHeight);


	VERTEXTYPE QuadVertex[4] = 
	{
		{(float)(iX-iDeltaWidth), (float)(iY+iDeltaHeight),		0, 1,		0,1,0,0,		TexCoordLeftDown.x, TexCoordLeftDown.y,0,0},
		{(float)(iX-iDeltaWidth), (float)(iY-iDeltaHeight),		0, 1,		0,0,0,0,		TexCoordLeftUp.x, TexCoordLeftUp.y,0,0},
		{(float)(iX+iDeltaWidth), (float)(iY+iDeltaHeight),		0, 1,		1,1,0,0,		TexCoordRightDown.x, TexCoordRightDown.y,0,0},
		{(float)(iX+iDeltaWidth), (float)(iY-iDeltaHeight),		0, 1,		1,0,0,0,		TexCoordRightUp.x, TexCoordRightUp.y,0,0}
	};

	// 开始每个矩形，构造Rect顶点缓冲
	memcpy(pData, QuadVertex, 4 * m_iStride);
	V_RETURN(m_pVBQuadUser->Unlock());


	// 绘制，写入纹理
	d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(fHeight, fHeight, fHeight, fHeight), 1);
	if(bAddHeight)
	{
		V_RETURN(CopyTexture(m_pTexTemp, m_pTexHeight));
		V_RETURN(CommonComputeQuad(pTexArea, m_pTexTemp, NULL, m_pTexHeight, NULL, &m_PSAddAreaHeight, TRUE));
	}
	else
	{
		V_RETURN(CommonComputeQuad(pTexArea, NULL, NULL, m_pTexHeight, NULL, &m_PSSetAreaHeight, TRUE));
	}



	// 恢复RT和深度缓冲
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);


	// 完成
	return S_OK;
}





HRESULT KWaveEquationWater::SetObstacleTexture(LPDIRECT3DTEXTURE9 pTexObstacle, LPDIRECT3DTEXTURE9 pTexOffset /* = NULL */)
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// 防止每帧连续设置相同的Obstacle图，导致的性能下降
	static LPDIRECT3DTEXTURE9 s_pTexture = NULL;

	if(!pTexObstacle)
	{
		s_pTexture = NULL;
		m_bABC = FALSE;
		return S_OK;
	}

	// 不支持GPU生成ObstacleTexture，至少需要76条指令，而且又没有手动设置Offset图，失败
	if(!CheckPS2xSupport(76, FALSE) && !pTexOffset)
		return D3DERR_NOTAVAILABLE;


	// 阻挡图分辨率必须和纹理一致！
	D3DSURFACE_DESC Desc;
	V_RETURN(pTexObstacle->GetLevelDesc(0, &Desc));
	if(Desc.Width != m_WaveData.iWidth || Desc.Height != m_WaveData.iHeight)
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
	V_RETURN(CommonComputeQuad(pTexObstacle, NULL, NULL, m_pTexABCBoundary, NULL, &m_PSObstacleToBoundary));

	// 边界图直接转换为ABC偏移坐标图，注意偏移坐标要乘加0.5，所以要清为0.5
	V_RETURN(ClearTexture(m_pTexABCOffset, &D3DXVECTOR4(0.5f, 0.5f, 0.5f, 0.5f)));
	// 硬件支持的话就用GPU来计算，反之就复制手动设定的Offset图
	if(CheckPS2xSupport(76, FALSE))
	{
		V_RETURN(CommonComputeQuad(m_pTexABCBoundary, m_pTexABCBoundaryToOffset, NULL, m_pTexABCOffset, NULL, &m_PSBoundaryToOffset));
	}
	else
	{
		V_RETURN(CopyTexture(m_pTexABCOffset, pTexOffset));
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













HRESULT KWaveEquationWater::WaterSimulation( float fDeltaTime ) 
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	m_fDeltaTime = fDeltaTime;

	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便FFT渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// 开始处理
	UINT i = 0;


	// 波动模拟
	// Verlet积分：先交换指针，上一帧的NowP变成PrevP，上一帧计算好的NewP现在变成NowP（两个输入），上一帧的Prev丢弃，变成新的计算目的地，即本帧的NewP
	LPDIRECT3DTEXTURE9 pTexTemp = NULL, pTexTemp_Sign = NULL;

	pTexTemp = m_pTexPrev;
	m_pTexPrev = m_pTexNow;
	m_pTexNow = m_pTexHeight;
	m_pTexHeight = pTexTemp;

	// 解波动方程
	d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_WaveData.fDampCoef, m_WaveData.fDampCoef, m_WaveData.fDampCoef, m_WaveData.fDampCoef), 1);
	d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(m_WaveData.fWaveSpeed, m_WaveData.fWaveSpeed, m_WaveData.fWaveSpeed, m_WaveData.fWaveSpeed), 1);
	float fCoef_a = m_fDeltaTime * m_fDeltaTime;
	d3ddevice->SetPixelShaderConstantF(7, (float *)&D3DXVECTOR4(fCoef_a, fCoef_a, fCoef_a, fCoef_a), 1);


		// 指定了AreaDampTexture的需要调用不同的Shader
	if(m_pTexAreaDamping)
	{
		V_RETURN(CommonComputeQuad(m_pTexPrev, m_pTexNow, m_pTexAreaDamping, m_pTexHeight, NULL, &m_PSWaveEquationWithDampTexture));
	}
	else
	{
		V_RETURN(CommonComputeQuad(m_pTexPrev, m_pTexNow, NULL, m_pTexHeight, NULL, &m_PSWaveEquation));
	}


	// 如有需要，则更新任意边界
	if(m_bABC)
	{
		// 注意这里要Clear一下，由于下面的Copy操作并不包括基本边界线，在边界处理中会采样到临近点，所以还是要把边界点处的值清为0
		V_RETURN(ClearTexture(m_pTexTemp));

		// 复制到临时贴图中待用
		V_RETURN(CopyTexture(m_pTexTemp, m_pTexHeight));

		// 当前高度图和Offset一起计算为新的高度图
		V_RETURN(CommonComputeQuad(m_pTexTemp, m_pTexABCBoundary, m_pTexABCOffset, m_pTexHeight, NULL, &m_PSABC_Bounce));
	}





	// 生成法/切线图
		// 都需要生成且支持MRT时
	if(m_bGenerateNormal && m_bGenerateTangent && d3dcaps.NumSimultaneousRTs > 1)
	{
		d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_WaveData.WaterSquare.x, m_WaveData.WaterSquare.y, 0, 0), 1);
		V_RETURN(CommonComputeQuad(m_pTexHeight, NULL, NULL, m_pTexNormal, m_pTexTangent, &m_PSGenerateNormalandTangent));
	}
	else
	{
		if(m_bGenerateNormal)
		{
			d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_WaveData.WaterSquare.x, m_WaveData.WaterSquare.y, 0, 0), 1);
			V_RETURN(CommonComputeQuad(m_pTexHeight, NULL, NULL, m_pTexNormal, NULL, &m_PSGenerateNormal));
		}
		if(m_bGenerateTangent)
		{
			d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_WaveData.WaterSquare.x, m_WaveData.WaterSquare.y, 0, 0), 1);
			V_RETURN(CommonComputeQuad(m_pTexHeight, NULL, NULL, m_pTexTangent, NULL, &m_PSGenerateTangent));
		}
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









/****************************初始化**********************************/

HRESULT KWaveEquationWater::Init(WAVEEQUATION_ATTRIBUTE WaveData, BOOL bGenerateNormal /* = FALSE */, BOOL bGenerateTangent /* = FALSE */)
{
	if(m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// 检查波动数据是否合法
	if(!WaveData.CheckAvaliable())
		return D3DERR_INVALIDCALL;
	m_WaveData = WaveData;

	// 检查硬件是否支持生成法线/切线图，只有支持才会使用用户给定的数据。初始化时检查一次，后面就不用检查了
	if(CheckPS2xSupport(32, FALSE, TRUE))
	{
		m_bGenerateNormal = bGenerateNormal;
		m_bGenerateTangent = bGenerateTangent;
	}
	else
	{
		m_bGenerateNormal = FALSE;
		m_bGenerateTangent = FALSE;
	}


	D3DFORMAT FormatFourChannel = D3DFMT_A8R8G8B8;
	// 创建和初始化数据贴图
#ifdef USE_FP16
	FormatFourChannel = D3DFMT_A16B16G16R16F;
#elif defined USE_FP32
	FormatFourChannel = D3DFMT_A32B32G32R32F;
#else
	mymessage("未定义使用浮点纹理，无法初始化波动引擎！");
	return E_FAIL;
#endif

	// Area Damping Texture可选，所以失败也不要返回
	if(FAILED(D3DXCreateTextureFromFileEx(d3ddevice, WaveData.szAreaDampTexture, m_WaveData.iWidth, m_WaveData.iHeight, 0, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_NONE, 0, NULL, NULL, &m_pTexAreaDamping)))
	{
		SAFE_RELEASE(m_pTexAreaDamping);
	}

	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexPrev, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexNow, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTemp, NULL));

	// 注意这里，这三张是要供给外界使用的，无论是用CPU Lock还是用VTF，都只能支持FP32的贴图！
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pTexHeight, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pTexNormal, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pTexTangent, NULL));


	// 任意边界
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexABCOffset, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexABCBoundary, NULL));	

	// 初始化ABC类型对应的偏移点坐标转换图（共13种边界开口类型）。由于Offset只有-1 0 1三种，所以写入的的时候要乘加0.5
	// 由于这两张1D贴图都比较小，为了写入数据的精确起见，这里用FP32纹理
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


	// 初始化Vertex Buffer，一个Point，一个Quad，四个Line
	m_iStride = sizeof(VERTEXTYPE);

	V_RETURN(d3ddevice->CreateVertexBuffer(4 * m_iStride, 0, 0, D3DPOOL_DEFAULT, &m_pVBQuad, NULL));
	V_RETURN(d3ddevice->CreateVertexBuffer(4 * m_iStride, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &m_pVBQuadUser, NULL));

	V_RETURN(d3ddevice->CreateVertexBuffer(m_WaveData.iWidth * m_WaveData.iHeight * m_iStride, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &m_pVBPoint, NULL));

	//	顶点位置：	1	3
	//				0	2

	UINT iWidth = m_WaveData.iWidth, iHeight = m_WaveData.iHeight;
	VOID* pVertexStream = NULL;
	VERTEXTYPE QuadVertex[4] = 
	{
		{0, (float)iHeight,					0, 1,		0,1,0,0,		0,(float)iHeight,0,0},
		{0, 0,								0, 1,		0,0,0,0,		0,0,0,0},
		{(float)iWidth, (float)iHeight,		0, 1,		1,1,0,0,		(float)iWidth,(float)iHeight,0,0},
		{(float)iWidth, 0,					0, 1,		1,0,0,0,		(float)iWidth,0,0,0}
	};
	
	V_RETURN(m_pVBQuad->Lock(0, 4 * m_iStride, &pVertexStream, 0));
	memcpy(pVertexStream, QuadVertex, 4 * m_iStride);
	m_pVBQuad->Unlock();





	// 初始化Shader
		// MISC
	V_RETURN(m_PSCopyTexture.InitPixelShader("shader\\WaveEquation\\CopyTexture.psh"));
	V_RETURN(m_PSClearTexture.InitPixelShader("shader\\WaveEquation\\ClearTexture.psh"));

		// User Inject
	V_RETURN(m_PSSetPointHeight.InitPixelShader("shader\\WaveEquation\\User\\SetPointHeight.psh"));
	V_RETURN(m_PSSetAreaHeight.InitPixelShader("shader\\WaveEquation\\User\\SetAreaHeight.psh"));
	V_RETURN(m_PSAddPointHeight.InitPixelShader("shader\\WaveEquation\\User\\AddPointHeight.psh"));
	V_RETURN(m_PSAddAreaHeight.InitPixelShader("shader\\WaveEquation\\User\\AddAreaHeight.psh"));

		// Wave Simulation
	V_RETURN(m_PSWaveEquation.InitPixelShader("shader\\WaveEquation\\Wave\\SolveWaveEquation.psh"));
	V_RETURN(m_PSWaveEquationWithDampTexture.InitPixelShader("shader\\WaveEquation\\Wave\\SolveWaveEquationWithDampTexture.psh"));

		// ABC
	V_RETURN(m_PSABC_Bounce.InitPixelShader("shader\\WaveEquation\\BC\\ABC_Bounce.psh"));
	V_RETURN(m_PSObstacleToBoundary.InitPixelShader("shader\\WaveEquation\\BC\\ObstacleToBoundary.psh"));

	if(CheckPS2xSupport(76, FALSE))
		V_RETURN(m_PSBoundaryToOffset.InitPixelShader("shader\\WaveEquation\\BC\\BoundaryToOffset.psh"));


		// 附加图：根据要求和系统是否支持MRT来初始化不同的Shader
	if(m_bGenerateNormal && m_bGenerateTangent && d3dcaps.NumSimultaneousRTs > 1)
	{
		V_RETURN(m_PSGenerateNormalandTangent.InitPixelShader("shader\\WaveEquation\\Addon\\GenerateNormalandTangent.psh"));
	}
	else
	{
		if(m_bGenerateNormal)
		{
			V_RETURN(m_PSGenerateNormal.InitPixelShader("shader\\WaveEquation\\Addon\\GenerateNormal.psh"));
		}
		if(m_bGenerateTangent)
		{
			V_RETURN(m_PSGenerateTangent.InitPixelShader("shader\\WaveEquation\\Addon\\GenerateTangent.psh"));
		}
	}


	// 创建深度缓冲
	D3DSURFACE_DESC DescDepth;
	LPDIRECT3DSURFACE9 pOldDepth = NULL;

	if(FAILED(d3ddevice->GetDepthStencilSurface(&pOldDepth)))
		return E_FAIL;
	if(FAILED(pOldDepth->GetDesc(&DescDepth)))
		return E_FAIL;
	SAFE_RELEASE(pOldDepth);

	V_RETURN(d3ddevice->CreateDepthStencilSurface(m_WaveData.iWidth, m_WaveData.iHeight, DescDepth.Format, DescDepth.MultiSampleType, DescDepth.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL));


	// 成功
	m_iCreateAttrib = 1;

	// 初始化波动
	V_RETURN(ResetWave());

	return S_OK;
}






/****************************内部接口--公用**********************************/
HRESULT KWaveEquationWater::CommonComputeQuad(LPDIRECT3DTEXTURE9 pSrcTex1, LPDIRECT3DTEXTURE9 pSrcTex2, LPDIRECT3DTEXTURE9 pSrcTex3, LPDIRECT3DTEXTURE9 pRT1, LPDIRECT3DTEXTURE9 pRT2, PIXELSHADER *pPS, BOOL bUserQuad /* = FALSE */)
{
	if(!m_pVBQuad || !m_pVBQuadUser)
		return D3DERR_NOTAVAILABLE;

	if(!pRT1 || !pPS)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	UINT i = 0;

	// 设置渲染来源和目的
	V_RETURN(SetTexturedRenderTarget(0, pRT1, NULL));
	SetTexturedRenderTarget(1, pRT2, NULL);

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


	// 设置PS及常量寄存器（1/2tap、1tap和TextureDimension，在c0、c1和c2）
	V_RETURN(pPS->SetPixelShader());
	V_RETURN(pPS->SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_WaveData.iWidth, 0.5f/(float)m_WaveData.iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_WaveData.iWidth, 1.0f/(float)m_WaveData.iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(2, &D3DXVECTOR4((float)m_WaveData.iWidth, (float)m_WaveData.iHeight, 0, 0), 1));
	// 一些常数和当前时间间隔
	V_RETURN(pPS->SetConstant(3, &D3DXVECTOR4(0, 0.5f, 1.0f, 2.0f), 1));
	V_RETURN(pPS->SetConstant(4, &D3DXVECTOR4(m_fDeltaTime, m_fDeltaTime, m_fDeltaTime, m_fDeltaTime), 1));


	// 设置Declaration
	V_RETURN(d3ddevice->SetVertexShader(NULL));
	V_RETURN(d3ddevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE4(0) | D3DFVF_TEXCOORDSIZE4(1)));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_pDeclaration));

	// 开始渲染
	if(!bUserQuad)
	{
		V_RETURN(d3ddevice->SetStreamSource(0, m_pVBQuad, 0, m_iStride));
	}
	else
	{
		V_RETURN(d3ddevice->SetStreamSource(0, m_pVBQuadUser, 0, m_iStride));
	}

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// 恢复
	V_RETURN(pPS->RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(1, NULL));

	// 完成
	return S_OK;
}



HRESULT KWaveEquationWater::CommonComputePoint(UINT iPointNum, LPDIRECT3DTEXTURE9 pSrcTex1, LPDIRECT3DTEXTURE9 pSrcTex2, LPDIRECT3DTEXTURE9 pSrcTex3, LPDIRECT3DTEXTURE9 pRT, PIXELSHADER *pPS)
{
	if(!m_pVBPoint)
		return D3DERR_NOTAVAILABLE;

	if(!pRT || !pPS || !iPointNum)
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


	// 设置PS及常量寄存器（1/2tap、1tap和TextureDimension，在c0、c1和c2）
	V_RETURN(pPS->SetPixelShader());
	V_RETURN(pPS->SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_WaveData.iWidth, 0.5f/(float)m_WaveData.iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_WaveData.iWidth, 1.0f/(float)m_WaveData.iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(2, &D3DXVECTOR4((float)m_WaveData.iWidth, (float)m_WaveData.iHeight, 0, 0), 1));
	// 一些常数和当前时间间隔
	V_RETURN(pPS->SetConstant(3, &D3DXVECTOR4(0, 0.5f, 1.0f, 2.0f), 1));
	V_RETURN(pPS->SetConstant(4, &D3DXVECTOR4(m_fDeltaTime, m_fDeltaTime, m_fDeltaTime, m_fDeltaTime), 1));


	// 设置Declaration
	V_RETURN(d3ddevice->SetVertexShader(NULL));
	V_RETURN(d3ddevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE4(0) | D3DFVF_TEXCOORDSIZE4(1)));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_pDeclaration));

	// 开始渲染
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVBPoint, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_POINTLIST, 0, iPointNum));
	V_RETURN(d3ddevice->EndScene());

	// 恢复
	V_RETURN(pPS->RestorePixelShader());

	// 完成
	return S_OK;
}