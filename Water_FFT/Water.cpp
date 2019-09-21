#include "myd3d.h"
#include "Water.h"







/************************Ocean Water Simulation*************************/
/////////�ⲿ�ӿ�
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

	// ������H
	V_RETURN(GenerateH(fTime));

	// ����IFFT���õ����˸߶�ͼ
	V_RETURN(m_IFFT.IFFT(m_pTexH));

	// ��Դͼ�ϵ���ָ���ĺ��˸߶�ͼ��m_pTexHeight�����û�е��ӵģ���ֱ�ӽ�Դ���Ƶ�m_pTexHeight
	if(pTexAddonHeightMap)
	{
		V_RETURN(AddHeightMap(pTexAddonHeightMap));
	}
	else
	{
		V_RETURN(m_IFFT.GetResultData(m_pTexHeight, FALSE));
	}

	// ���ɷ���ͼ
	// ����Ҫ������֧��MRTʱ
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

	// ���
	m_iCreateAttrib = 2;
	return S_OK;
}





HRESULT KFFTOceanWater::AddHeightMap(LPDIRECT3DTEXTURE9 pTexAddonHeightMap)
{
	if(!m_iCreateAttrib || !m_IFFT.GetResultTexture() || !m_pTexHeight || !pTexAddonHeightMap)
		return D3DERR_NOTAVAILABLE;

	// ������ͼ��Ч�Լ�飬������4ͨ������һ���Ǹ��㣩���������ڴ�����
	D3DSURFACE_DESC Desc;
	V_RETURN(pTexAddonHeightMap->GetLevelDesc(0, &Desc));
	if(Desc.Pool == D3DPOOL_SYSTEMMEM)
		return D3DERR_WRONGTEXTUREFORMAT;
	if(Desc.Format != D3DFMT_A8R8G8B8 && Desc.Format != D3DFMT_A16B16G16R16 && Desc.Format != D3DFMT_A16B16G16R16F && Desc.Format != D3DFMT_A32B32G32R32F)
		return D3DERR_WRONGTEXTUREFORMAT;


	HRESULT hr = S_OK;
	UINT i = 0;

	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա���Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// ������Ⱦ��Դ��Ŀ��
	V_RETURN(SetTexturedRenderTarget(0, m_pTexHeight, NULL));
	V_RETURN(d3ddevice->SetTexture(0, m_IFFT.GetResultTexture()));
	V_RETURN(d3ddevice->SetTexture(1, pTexAddonHeightMap));

	// ������Ⱦ����
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


	// ����Shader
	V_RETURN(m_PSAddHeightMap.SetPixelShader());
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));

	// ���ó����Ĵ���
	// 0.5 tap
	V_RETURN(m_PSAddHeightMap.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_OceanData.iWidth, 0.5f/(float)m_OceanData.iHeight, 0, 0), 1));

	// ��ʼ��Ⱦ
	V_RETURN(d3ddevice->SetFVF(0));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// �ָ�Pixel Shader��RT����Ȼ���
	V_RETURN(m_PSAddHeightMap.RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	// ���
	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);
	return S_OK;
}



/********************���ɷ�������ͼ���ڲ�����*********************/

HRESULT KFFTOceanWater::GenerateTangent()
{
	if(!m_iCreateAttrib || !m_pTexHeight)
		return D3DERR_NOTAVAILABLE;

	HRESULT hr = S_OK;
	UINT i = 0;

	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա���Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// ������Ⱦ��Դ��Ŀ��
	V_RETURN(SetTexturedRenderTarget(0, m_pTexTangent, NULL));
	V_RETURN(d3ddevice->SetTexture(0, m_pTexHeight));

	// ������Ⱦ����
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	// ����Shader
	V_RETURN(m_PSGenerateTangent.SetPixelShader());
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));

	// ���ó����Ĵ���
	// 0.5 tap
	V_RETURN(m_PSGenerateTangent.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_OceanData.iWidth, 0.5f/(float)m_OceanData.iHeight, 0, 0), 1));
	// 1 tap
	V_RETURN(m_PSGenerateTangent.SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_OceanData.iWidth, 1.0f/(float)m_OceanData.iHeight, 0, 0), 1));
	// ��ˮ���
	V_RETURN(m_PSGenerateTangent.SetConstant(2, &D3DXVECTOR4(m_OceanData.WaterSquare.x, m_OceanData.WaterSquare.y, 0, 0), 1));
	// �߶�����
	V_RETURN(m_PSGenerateTangent.SetConstant(3, &D3DXVECTOR4(m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale), 1));



	// ��ʼ��Ⱦ
	V_RETURN(d3ddevice->SetFVF(0));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// �ָ�Pixel Shader��RT����Ȼ���
	V_RETURN(m_PSGenerateTangent.RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	// ���
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

	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա���Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// ������Ⱦ��Դ��Ŀ��
	V_RETURN(SetTexturedRenderTarget(0, m_pTexNormal, NULL));
	V_RETURN(d3ddevice->SetTexture(0, m_pTexHeight));

	// ������Ⱦ����
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	// ����Shader
	V_RETURN(m_PSGenerateNormal.SetPixelShader());
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));

	// ���ó����Ĵ���
	// 0.5 tap
	V_RETURN(m_PSGenerateNormal.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_OceanData.iWidth, 0.5f/(float)m_OceanData.iHeight, 0, 0), 1));
	// 1 tap
	V_RETURN(m_PSGenerateNormal.SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_OceanData.iWidth, 1.0f/(float)m_OceanData.iHeight, 0, 0), 1));
	// ��ˮ���
	V_RETURN(m_PSGenerateNormal.SetConstant(2, &D3DXVECTOR4(m_OceanData.WaterSquare.x, m_OceanData.WaterSquare.y, 0, 0), 1));
	// �߶�����
	V_RETURN(m_PSGenerateNormal.SetConstant(3, &D3DXVECTOR4(m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale), 1));



	// ��ʼ��Ⱦ
	V_RETURN(d3ddevice->SetFVF(0));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// �ָ�Pixel Shader��RT����Ȼ���
	V_RETURN(m_PSGenerateNormal.RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	// ���
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

	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա���Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer[2] = {NULL, NULL}, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer[0]));
	d3ddevice->GetRenderTarget(1, &pOldBackBuffer[1]);	// û��MRT�Ļ����Զ�����D3DERR_NOTFOUND�ģ������Ǵ���
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// ������Ⱦ��Դ��Ŀ�ģ�MRT��
	V_RETURN(SetTexturedRenderTarget(0, m_pTexNormal, NULL));
	V_RETURN(SetTexturedRenderTarget(1, m_pTexTangent, NULL));
	V_RETURN(d3ddevice->SetTexture(0, m_pTexHeight));

	// ������Ⱦ����
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	// ����Shader
	V_RETURN(m_PSGenerateNormalandTangent.SetPixelShader());
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));

	// ���ó����Ĵ���
	// 0.5 tap
	V_RETURN(m_PSGenerateNormalandTangent.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_OceanData.iWidth, 0.5f/(float)m_OceanData.iHeight, 0, 0), 1));
	// 1 tap
	V_RETURN(m_PSGenerateNormalandTangent.SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_OceanData.iWidth, 1.0f/(float)m_OceanData.iHeight, 0, 0), 1));
	// ��ˮ���
	V_RETURN(m_PSGenerateNormalandTangent.SetConstant(2, &D3DXVECTOR4(m_OceanData.WaterSquare.x, m_OceanData.WaterSquare.y, 0, 0), 1));
	// �߶�����
	V_RETURN(m_PSGenerateNormalandTangent.SetConstant(3, &D3DXVECTOR4(m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale, m_OceanData.fWaveHeightScale), 1));



	// ��ʼ��Ⱦ
	V_RETURN(d3ddevice->SetFVF(0));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// �ָ�Pixel Shader��RT����Ȼ���
	V_RETURN(m_PSGenerateNormalandTangent.RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer[0]));
	V_RETURN(d3ddevice->SetRenderTarget(1, pOldBackBuffer[1]));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	// ���
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

	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա���Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// ������Ⱦ��Դ��Ŀ��
	V_RETURN(SetTexturedRenderTarget(0, m_pTexH, NULL));
	V_RETURN(d3ddevice->SetTexture(0, m_ppTexH0[0]));
	V_RETURN(d3ddevice->SetTexture(1, m_ppTexH0[1]));

	// ������Ⱦ����
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

	// ����Shader
	V_RETURN(m_PSGenerateH.SetPixelShader());
	V_RETURN(d3ddevice->SetVertexShader(m_VSDrawQuad.Handle));

	// ���ó����Ĵ���
	// 0.5 tap
	V_RETURN(m_PSGenerateH.SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_OceanData.iWidth, 0.5f/(float)m_OceanData.iHeight, 0, 0), 1));
	// Current Time
	V_RETURN(m_PSGenerateH.SetConstant(1, &D3DXVECTOR4(fTime, fTime, fTime, fTime), 1));
	// D3D SinCos Constant
	V_RETURN(m_PSGenerateH.SetConstant(30, &D3DXVECTOR4(D3DSINCOSCONST1), 1));
	V_RETURN(m_PSGenerateH.SetConstant(31, &D3DXVECTOR4(D3DSINCOSCONST2), 1));


	// ��ʼ��Ⱦ
	V_RETURN(d3ddevice->SetFVF(0));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_VSDrawQuad.Declaration));
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());

	// �ָ�Pixel Shader��RT����Ȼ���
	V_RETURN(m_PSGenerateH.RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	// ���
	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);
	return S_OK;
}





/****************************��ʼ��**********************************/

HRESULT KFFTOceanWater::Init(OCEANWATER_ATTRIBUTE OceanData, BOOL bGenerateNormal /* = TRUE */, BOOL bGenerateTangent /* = TRUE */)
{
	if(m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// ��麣�������Ƿ�Ϸ�
	if(!OceanData.CheckAvaliable())
		return D3DERR_INVALIDCALL;
	m_OceanData = OceanData;

	// ���ü���Ƿ�2���ݣ�����Ƿ�����ʼ��IFFT��ʧ��
	V_RETURN(m_IFFT.Init(m_OceanData.iWidth, m_OceanData.iHeight));

	m_bGenerateNormal = bGenerateNormal;
	m_bGenerateTangent = bGenerateTangent;


	D3DFORMAT FormatFourChannel = D3DFMT_A8R8G8B8;
	// �����ͳ�ʼ��������ͼ
#ifdef USE_FP16
	FormatFourChannel = D3DFMT_A16B16G16R16F;
#elif defined USE_FP32
	FormatFourChannel = D3DFMT_A32B32G32R32F;
#else
	mymessage("δ����ʹ�ø��������޷���ʼ���������棡");
	return E_FAIL;
#endif

	V_RETURN(d3ddevice->CreateTexture(m_OceanData.iWidth, m_OceanData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexH, NULL));

	// ע�������������Ҫ�������ʹ�õģ���������CPU Lock������VTF����ֻ��֧��FP32����ͼ��
	V_RETURN(d3ddevice->CreateTexture(m_OceanData.iWidth, m_OceanData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pTexHeight, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_OceanData.iWidth, m_OceanData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pTexNormal, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_OceanData.iWidth, m_OceanData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pTexTangent, NULL));

	V_RETURN(InitH0());


	// ��ʼ��Quad
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

	// ��ʼ��Shader
	D3DVERTEXELEMENT9 Dclr[] = {
		{0,0,D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0,12,D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};

	V_RETURN(m_VSDrawQuad.InitVertexShader("shader\\FFT\\Quad.vsh", Dclr));
	V_RETURN(m_PSGenerateH.InitPixelShader("shader\\OceanWaterSimulation\\GenerateH.psh"));
	V_RETURN(m_PSAddHeightMap.InitPixelShader("shader\\OceanWaterSimulation\\AddHeightMap.psh"));

	// ����Ҫ���ϵͳ�Ƿ�֧��MRT����ʼ����ͬ��Shader
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


	// ������Ȼ���
	D3DSURFACE_DESC DescDepth;
	LPDIRECT3DSURFACE9 pOldDepth = NULL;

	if(FAILED(d3ddevice->GetDepthStencilSurface(&pOldDepth)))
		return E_FAIL;
	if(FAILED(pOldDepth->GetDesc(&DescDepth)))
		return E_FAIL;
	SAFE_RELEASE(pOldDepth);

	V_RETURN(d3ddevice->CreateDepthStencilSurface(m_OceanData.iWidth, m_OceanData.iHeight, DescDepth.Format, DescDepth.MultiSampleType, DescDepth.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL));


	// �ɹ�
	m_iCreateAttrib = 1;
	return S_OK;
}








D3DXVECTOR2 KFFTOceanWater::CalcH0(int iX, int iY)
{
	D3DXVECTOR2 Result = D3DXVECTOR2(0, 0);

	// ����������
	float fPh = 0.0f;	// Phillips Spectrum
	D3DXVECTOR2 fGaussRandom = D3DXVECTOR2(1.0f, 0.0f);	// ��˹�����
	D3DXVECTOR2 VecK = D3DXVECTOR2(0, 0);
	float fL = 0.0f, fk2 = 0.0f;
	float fKDotW2 = 0.0f;
	float fTemp = 0.0f;

	// �ȼ�����K��ģk^2
	VecK.x = (float)iX / (float)m_OceanData.iWidth;
	VecK.y = (float)iY / (float)m_OceanData.iHeight;
	fk2 = D3DXVec2Dot(&VecK, &VecK);
	D3DXVec2Normalize(&VecK, &VecK);

	// �����������µĲ���L
	fL = m_OceanData.fWindSpeed * m_OceanData.fWindSpeed / GravityConstant;

	// �������͵�ǰ���˷����Dot
	fKDotW2 = D3DXVec2Dot(&VecK, &m_OceanData.VecWindDir) * D3DXVec2Dot(&VecK, &m_OceanData.VecWindDir);

	// ����Ph
	fTemp = -1.0f / (fk2 * fL * fL);	// e��ָ��
	fPh = expf(fTemp);
	fPh /= (fk2 * fk2);					// ��ĸ��k^4
	fPh *= fKDotW2;

	// �������������һ������
	fGaussRandom = D3DXVECTOR2( (float)gaussrandom(), (float)gaussrandom() );

	// ��ǰ��ļ���ϳɵ�Ph��
	fPh = sqrtf(fPh / 2.0f);

	// �ϳ�Ph��random���������˷�����д��Result
	Result.x = fPh * fGaussRandom.x;
	Result.y = fPh * fGaussRandom.y;

	// ����
	return Result;
}

HRESULT KFFTOceanWater::InitH0()
{
	if(!m_OceanData.CheckAvaliable())
		return D3DERR_NOTAVAILABLE;

	SAFE_RELEASE(m_ppTexH0[0]);
	SAFE_RELEASE(m_ppTexH0[1]);


	// ������ͼ�ڸ߷ֱ����³�ʼ��ʱ����Ҫ�ܾã���FP16������
	D3DFORMAT FormatFourChannel = D3DFMT_A32B32G32R32F;

	V_RETURN(d3ddevice->CreateTexture(m_OceanData.iWidth, m_OceanData.iHeight, 1, D3DUSAGE_0, FormatFourChannel, D3DPOOL_MANAGED, &m_ppTexH0[0], NULL));
	V_RETURN(d3ddevice->CreateTexture(m_OceanData.iWidth, m_OceanData.iHeight, 1, D3DUSAGE_0, FormatFourChannel, D3DPOOL_MANAGED, &m_ppTexH0[1], NULL));


	HRESULT hr = S_OK;
	UINT i = 0, iX = 0, iY = 0;
	D3DLOCKED_RECT Rect, Rect_Oppo;
	float16 *p16 = NULL, *p16_Oppo = NULL;
	float *p = NULL, *p_Oppo = NULL;


	// Lock��Ready? Go!
	V_RETURN(m_ppTexH0[0]->LockRect(0, &Rect, NULL, 0));
	p16 = (float16 *)((BYTE *)Rect.pBits);	
	p = (float *)((BYTE *)Rect.pBits);

	V_RETURN(m_ppTexH0[1]->LockRect(0, &Rect_Oppo, NULL, 0));
	p16_Oppo = (float16 *)((BYTE *)Rect.pBits);	
	p_Oppo = (float *)((BYTE *)Rect.pBits);


	D3DXVECTOR2 fH0;	// ���ڵõ�H0��ֵ
	D3DXVECTOR4 fData, fData_Oppo;	// �������������ʽΪʵ�鲿+ʵ�鲿���ţ�0��2��


	// ��ǰ���������У��Ƕ��ٸ�����һ��
	for(iY = 0; iY < m_OceanData.iHeight; iY++)
	{
		p16 = (float16 *)((BYTE *)Rect.pBits + iY * Rect.Pitch);
		p = (float *)((BYTE *)Rect.pBits + iY * Rect.Pitch);
		p16_Oppo = (float16 *)((BYTE *)Rect_Oppo.pBits + iY * Rect_Oppo.Pitch);
		p_Oppo = (float *)((BYTE *)Rect_Oppo.pBits + iY * Rect_Oppo.Pitch);

		for(iX = 0; iX < m_OceanData.iWidth; iX++)
		{
			// ����h0(K)����1��Ϊ�˽�0ȥ��
			fH0 = CalcH0((int)iX+1, (int)iY+1);
			fData.x = absf(fH0.x);
			fData.y = absf(fH0.y);
			fData.z = fH0.x < 0.0f ? 0.0f : 2.0f;
			fData.w = fH0.y < 0.0f ? 0.0f : 2.0f;

			// ����h0(-K)
			fH0 = CalcH0(-(int)iX-1, -(int)iY-1);
			fData_Oppo.x = absf(fH0.x);
			fData_Oppo.y = absf(fH0.y);
			fData_Oppo.z = fH0.x < 0.0f ? 0.0f : 2.0f;
			fData_Oppo.w = fH0.y < 0.0f ? 0.0f : 2.0f;


			// д����ͼ
			for(i = 0; i < 4; i++)
			{
				*p++ = fData[i];
				*p_Oppo++ = fData_Oppo[i];
			}// end for Write Texel
		}// end for X
	}

	// ����
	V_RETURN(m_ppTexH0[0]->UnlockRect(0));
	V_RETURN(m_ppTexH0[1]->UnlockRect(0));

	return S_OK;
}































/****************************���ڲ������̵�ˮ��ģ��**********************************/
/////////�ⲿ�ӿ�
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

	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա�FFT��Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));

	// ����
	V_RETURN(ClearTexture(m_pTexHeight, &D3DXVECTOR4(0, 0, 0, 0)));
	V_RETURN(ClearTexture(m_pTexPrev, &D3DXVECTOR4(0, 0, 0, 0)));
	V_RETURN(ClearTexture(m_pTexNow, &D3DXVECTOR4(0, 0, 0, 0)));

	V_RETURN(ClearTexture(m_pTexNormal, &D3DXVECTOR4(0.5f, 0.5f, 0.5f, 0)));
	V_RETURN(ClearTexture(m_pTexTangent, &D3DXVECTOR4(0.5f, 0.5f, 0.5f, 0)));

	// �ָ�RT����Ȼ���
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


	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա�FFT��Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// ����Point����
	BYTE *pData = NULL;
	float *pWrite = NULL;
	V_RETURN(m_pVBPoint->Lock(0, iPointNum * m_iStride, (void **)&pData, D3DLOCK_DISCARD));


	// ��ʼ����ÿ���㣬����PointList���㻺��
	for(i = 0; i < iPointNum; i++)
	{
		// ������Χ������
		if(pX[i] >= m_WaveData.iWidth || pY[i] >= m_WaveData.iHeight)
			continue;
		
		pWrite = (float *)pData;

		// ����XYZRHW		
		*pWrite++ = (float)pX[i];
		*pWrite++ = (float)pY[i];
		*pWrite++ = 0.0f;
		*pWrite++ = 1.0f;

		// ��һ����������
		*pWrite++ = (float)pX[i] / (float)m_WaveData.iWidth;
		*pWrite++ = (float)pY[i] / (float)m_WaveData.iHeight;
		*pWrite++ = 0.0f;
		*pWrite++ = 0.0f;

		// �ڶ����������걣��д�������x = abs(Height), z = sign(Height)
		*pWrite++ = absf(pHeight[i]);
		pWrite++;
		*pWrite++ = (pHeight[i] > 0.0f ? 2.0f : 0.0f);
		pWrite++;

		// ������һ����
		pData += m_iStride;
	}

	V_RETURN(m_pVBPoint->Unlock());

	// ���ƣ�д������
	if(bAddHeight)
	{
		V_RETURN(CopyTexture(m_pTexTemp, m_pTexHeight));
		V_RETURN(CommonComputePoint(iPointNum, m_pTexTemp, NULL, NULL, m_pTexHeight, &m_PSAddPointHeight));
	}
	else
	{
		V_RETURN(CommonComputePoint(iPointNum, NULL, NULL, NULL, m_pTexHeight, &m_PSSetPointHeight));
	}



	// �ָ�RT����Ȼ���
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);


	// ���
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


	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա�FFT��Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// ����Rect����
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

	// ��ʼÿ�����Σ�����Rect���㻺��
	memcpy(pData, QuadVertex, 4 * m_iStride);
	V_RETURN(m_pVBQuadUser->Unlock());


	// ���ƣ�д������
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



	// �ָ�RT����Ȼ���
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);


	// ���
	return S_OK;
}





HRESULT KWaveEquationWater::SetObstacleTexture(LPDIRECT3DTEXTURE9 pTexObstacle, LPDIRECT3DTEXTURE9 pTexOffset /* = NULL */)
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// ��ֹÿ֡����������ͬ��Obstacleͼ�����µ������½�
	static LPDIRECT3DTEXTURE9 s_pTexture = NULL;

	if(!pTexObstacle)
	{
		s_pTexture = NULL;
		m_bABC = FALSE;
		return S_OK;
	}

	// ��֧��GPU����ObstacleTexture��������Ҫ76��ָ�������û���ֶ�����Offsetͼ��ʧ��
	if(!CheckPS2xSupport(76, FALSE) && !pTexOffset)
		return D3DERR_NOTAVAILABLE;


	// �赲ͼ�ֱ��ʱ��������һ�£�
	D3DSURFACE_DESC Desc;
	V_RETURN(pTexObstacle->GetLevelDesc(0, &Desc));
	if(Desc.Width != m_WaveData.iWidth || Desc.Height != m_WaveData.iHeight)
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
	V_RETURN(CommonComputeQuad(pTexObstacle, NULL, NULL, m_pTexABCBoundary, NULL, &m_PSObstacleToBoundary));

	// �߽�ͼֱ��ת��ΪABCƫ������ͼ��ע��ƫ������Ҫ�˼�0.5������Ҫ��Ϊ0.5
	V_RETURN(ClearTexture(m_pTexABCOffset, &D3DXVECTOR4(0.5f, 0.5f, 0.5f, 0.5f)));
	// Ӳ��֧�ֵĻ�����GPU�����㣬��֮�͸����ֶ��趨��Offsetͼ
	if(CheckPS2xSupport(76, FALSE))
	{
		V_RETURN(CommonComputeQuad(m_pTexABCBoundary, m_pTexABCBoundaryToOffset, NULL, m_pTexABCOffset, NULL, &m_PSBoundaryToOffset));
	}
	else
	{
		V_RETURN(CopyTexture(m_pTexABCOffset, pTexOffset));
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













HRESULT KWaveEquationWater::WaterSimulation( float fDeltaTime ) 
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	m_fDeltaTime = fDeltaTime;

	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա�FFT��Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));


	// ��ʼ����
	UINT i = 0;


	// ����ģ��
	// Verlet���֣��Ƚ���ָ�룬��һ֡��NowP���PrevP����һ֡����õ�NewP���ڱ��NowP���������룩����һ֡��Prev����������µļ���Ŀ�ĵأ�����֡��NewP
	LPDIRECT3DTEXTURE9 pTexTemp = NULL, pTexTemp_Sign = NULL;

	pTexTemp = m_pTexPrev;
	m_pTexPrev = m_pTexNow;
	m_pTexNow = m_pTexHeight;
	m_pTexHeight = pTexTemp;

	// �Ⲩ������
	d3ddevice->SetPixelShaderConstantF(5, (float *)&D3DXVECTOR4(m_WaveData.fDampCoef, m_WaveData.fDampCoef, m_WaveData.fDampCoef, m_WaveData.fDampCoef), 1);
	d3ddevice->SetPixelShaderConstantF(6, (float *)&D3DXVECTOR4(m_WaveData.fWaveSpeed, m_WaveData.fWaveSpeed, m_WaveData.fWaveSpeed, m_WaveData.fWaveSpeed), 1);
	float fCoef_a = m_fDeltaTime * m_fDeltaTime;
	d3ddevice->SetPixelShaderConstantF(7, (float *)&D3DXVECTOR4(fCoef_a, fCoef_a, fCoef_a, fCoef_a), 1);


		// ָ����AreaDampTexture����Ҫ���ò�ͬ��Shader
	if(m_pTexAreaDamping)
	{
		V_RETURN(CommonComputeQuad(m_pTexPrev, m_pTexNow, m_pTexAreaDamping, m_pTexHeight, NULL, &m_PSWaveEquationWithDampTexture));
	}
	else
	{
		V_RETURN(CommonComputeQuad(m_pTexPrev, m_pTexNow, NULL, m_pTexHeight, NULL, &m_PSWaveEquation));
	}


	// ������Ҫ�����������߽�
	if(m_bABC)
	{
		// ע������ҪClearһ�£����������Copy�����������������߽��ߣ��ڱ߽紦���л�������ٽ��㣬���Ի���Ҫ�ѱ߽�㴦��ֵ��Ϊ0
		V_RETURN(ClearTexture(m_pTexTemp));

		// ���Ƶ���ʱ��ͼ�д���
		V_RETURN(CopyTexture(m_pTexTemp, m_pTexHeight));

		// ��ǰ�߶�ͼ��Offsetһ�����Ϊ�µĸ߶�ͼ
		V_RETURN(CommonComputeQuad(m_pTexTemp, m_pTexABCBoundary, m_pTexABCOffset, m_pTexHeight, NULL, &m_PSABC_Bounce));
	}





	// ���ɷ�/����ͼ
		// ����Ҫ������֧��MRTʱ
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


	// �ָ�RT����Ȼ���
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);



	// ���в������
	m_iCreateAttrib = 2;
	return S_OK;
}









/****************************��ʼ��**********************************/

HRESULT KWaveEquationWater::Init(WAVEEQUATION_ATTRIBUTE WaveData, BOOL bGenerateNormal /* = FALSE */, BOOL bGenerateTangent /* = FALSE */)
{
	if(m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// ��鲨�������Ƿ�Ϸ�
	if(!WaveData.CheckAvaliable())
		return D3DERR_INVALIDCALL;
	m_WaveData = WaveData;

	// ���Ӳ���Ƿ�֧�����ɷ���/����ͼ��ֻ��֧�ֲŻ�ʹ���û����������ݡ���ʼ��ʱ���һ�Σ�����Ͳ��ü����
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
	// �����ͳ�ʼ��������ͼ
#ifdef USE_FP16
	FormatFourChannel = D3DFMT_A16B16G16R16F;
#elif defined USE_FP32
	FormatFourChannel = D3DFMT_A32B32G32R32F;
#else
	mymessage("δ����ʹ�ø��������޷���ʼ���������棡");
	return E_FAIL;
#endif

	// Area Damping Texture��ѡ������ʧ��Ҳ��Ҫ����
	if(FAILED(D3DXCreateTextureFromFileEx(d3ddevice, WaveData.szAreaDampTexture, m_WaveData.iWidth, m_WaveData.iHeight, 0, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_NONE, 0, NULL, NULL, &m_pTexAreaDamping)))
	{
		SAFE_RELEASE(m_pTexAreaDamping);
	}

	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexPrev, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexNow, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexTemp, NULL));

	// ע�������������Ҫ�������ʹ�õģ���������CPU Lock������VTF����ֻ��֧��FP32����ͼ��
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pTexHeight, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pTexNormal, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pTexTangent, NULL));


	// ����߽�
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexABCOffset, NULL));
	V_RETURN(d3ddevice->CreateTexture(m_WaveData.iWidth, m_WaveData.iHeight, 1, D3DUSAGE_RENDERTARGET, FormatFourChannel, D3DPOOL_DEFAULT, &m_pTexABCBoundary, NULL));	

	// ��ʼ��ABC���Ͷ�Ӧ��ƫ�Ƶ�����ת��ͼ����13�ֱ߽翪�����ͣ�������Offsetֻ��-1 0 1���֣�����д��ĵ�ʱ��Ҫ�˼�0.5
	// ����������1D��ͼ���Ƚ�С��Ϊ��д�����ݵľ�ȷ�����������FP32����
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


	// ��ʼ��Vertex Buffer��һ��Point��һ��Quad���ĸ�Line
	m_iStride = sizeof(VERTEXTYPE);

	V_RETURN(d3ddevice->CreateVertexBuffer(4 * m_iStride, 0, 0, D3DPOOL_DEFAULT, &m_pVBQuad, NULL));
	V_RETURN(d3ddevice->CreateVertexBuffer(4 * m_iStride, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &m_pVBQuadUser, NULL));

	V_RETURN(d3ddevice->CreateVertexBuffer(m_WaveData.iWidth * m_WaveData.iHeight * m_iStride, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &m_pVBPoint, NULL));

	//	����λ�ã�	1	3
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





	// ��ʼ��Shader
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


		// ����ͼ������Ҫ���ϵͳ�Ƿ�֧��MRT����ʼ����ͬ��Shader
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


	// ������Ȼ���
	D3DSURFACE_DESC DescDepth;
	LPDIRECT3DSURFACE9 pOldDepth = NULL;

	if(FAILED(d3ddevice->GetDepthStencilSurface(&pOldDepth)))
		return E_FAIL;
	if(FAILED(pOldDepth->GetDesc(&DescDepth)))
		return E_FAIL;
	SAFE_RELEASE(pOldDepth);

	V_RETURN(d3ddevice->CreateDepthStencilSurface(m_WaveData.iWidth, m_WaveData.iHeight, DescDepth.Format, DescDepth.MultiSampleType, DescDepth.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL));


	// �ɹ�
	m_iCreateAttrib = 1;

	// ��ʼ������
	V_RETURN(ResetWave());

	return S_OK;
}






/****************************�ڲ��ӿ�--����**********************************/
HRESULT KWaveEquationWater::CommonComputeQuad(LPDIRECT3DTEXTURE9 pSrcTex1, LPDIRECT3DTEXTURE9 pSrcTex2, LPDIRECT3DTEXTURE9 pSrcTex3, LPDIRECT3DTEXTURE9 pRT1, LPDIRECT3DTEXTURE9 pRT2, PIXELSHADER *pPS, BOOL bUserQuad /* = FALSE */)
{
	if(!m_pVBQuad || !m_pVBQuadUser)
		return D3DERR_NOTAVAILABLE;

	if(!pRT1 || !pPS)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	UINT i = 0;

	// ������Ⱦ��Դ��Ŀ��
	V_RETURN(SetTexturedRenderTarget(0, pRT1, NULL));
	SetTexturedRenderTarget(1, pRT2, NULL);

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


	// ����PS�������Ĵ�����1/2tap��1tap��TextureDimension����c0��c1��c2��
	V_RETURN(pPS->SetPixelShader());
	V_RETURN(pPS->SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_WaveData.iWidth, 0.5f/(float)m_WaveData.iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_WaveData.iWidth, 1.0f/(float)m_WaveData.iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(2, &D3DXVECTOR4((float)m_WaveData.iWidth, (float)m_WaveData.iHeight, 0, 0), 1));
	// һЩ�����͵�ǰʱ����
	V_RETURN(pPS->SetConstant(3, &D3DXVECTOR4(0, 0.5f, 1.0f, 2.0f), 1));
	V_RETURN(pPS->SetConstant(4, &D3DXVECTOR4(m_fDeltaTime, m_fDeltaTime, m_fDeltaTime, m_fDeltaTime), 1));


	// ����Declaration
	V_RETURN(d3ddevice->SetVertexShader(NULL));
	V_RETURN(d3ddevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE4(0) | D3DFVF_TEXCOORDSIZE4(1)));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_pDeclaration));

	// ��ʼ��Ⱦ
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

	// �ָ�
	V_RETURN(pPS->RestorePixelShader());
	V_RETURN(d3ddevice->SetRenderTarget(1, NULL));

	// ���
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


	// ����PS�������Ĵ�����1/2tap��1tap��TextureDimension����c0��c1��c2��
	V_RETURN(pPS->SetPixelShader());
	V_RETURN(pPS->SetConstant(0, &D3DXVECTOR4(0.5f/(float)m_WaveData.iWidth, 0.5f/(float)m_WaveData.iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(1, &D3DXVECTOR4(1.0f/(float)m_WaveData.iWidth, 1.0f/(float)m_WaveData.iHeight, 0, 0), 1));
	V_RETURN(pPS->SetConstant(2, &D3DXVECTOR4((float)m_WaveData.iWidth, (float)m_WaveData.iHeight, 0, 0), 1));
	// һЩ�����͵�ǰʱ����
	V_RETURN(pPS->SetConstant(3, &D3DXVECTOR4(0, 0.5f, 1.0f, 2.0f), 1));
	V_RETURN(pPS->SetConstant(4, &D3DXVECTOR4(m_fDeltaTime, m_fDeltaTime, m_fDeltaTime, m_fDeltaTime), 1));


	// ����Declaration
	V_RETURN(d3ddevice->SetVertexShader(NULL));
	V_RETURN(d3ddevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE4(0) | D3DFVF_TEXCOORDSIZE4(1)));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_pDeclaration));

	// ��ʼ��Ⱦ
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVBPoint, 0, m_iStride));

	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_POINTLIST, 0, iPointNum));
	V_RETURN(d3ddevice->EndScene());

	// �ָ�
	V_RETURN(pPS->RestorePixelShader());

	// ���
	return S_OK;
}