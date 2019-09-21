/*
  *公共函数库
  *@2018年4月3日
  *@author:xiaoxiong
 */
#include"common.h"
#include<d3dx11.h>
#include<assert.h>

void createUnorderAccessView(ID3D11Buffer *buffer, int totalSize, ID3D11UnorderedAccessView **unorderAccessView, ID3D11ShaderResourceView **shaderResourceView)
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC  uavDesc;
	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = totalSize / 4;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
	int result = s_device->CreateUnorderedAccessView(buffer, &uavDesc, unorderAccessView);
	assert(result>=0);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderViewDesc;
	shaderViewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	shaderViewDesc.BufferEx.FirstElement = 0;
	shaderViewDesc.BufferEx.NumElements = totalSize / 4;
	shaderViewDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
	result = s_device->CreateShaderResourceView(buffer, &shaderViewDesc, shaderResourceView);
	assert(result>=0);
}

void  createTextureAndViews2(int width, int height, DXGI_FORMAT format, ID3D11Texture2D **outTexture, ID3D11ShaderResourceView **outResourceView, ID3D11RenderTargetView **outRenderTargetView)
{
	D3D11_TEXTURE2D_DESC   textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	int result = s_device->CreateTexture2D(&textureDesc, nullptr, outTexture);
	assert(result>=0);
	(*outTexture)->GetDesc(&textureDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderViewDesc;
	shaderViewDesc.Format = format;
	shaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	shaderViewDesc.Texture2D.MostDetailedMip = 0;
	result = s_device->CreateShaderResourceView(*outTexture, &shaderViewDesc, outResourceView);
	assert(result>=0);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	result = s_device->CreateRenderTargetView(*outTexture, &renderTargetViewDesc, outRenderTargetView);
	assert(result>=0);
}

// Generating gaussian random number with mean 0 and standard deviation 1.
float gauss_random()
{
	float u1 = rand() / (float)RAND_MAX;
	float u2 = rand() / (float)RAND_MAX;
	if (u1 < 1e-6f)
		u1 = 1e-6f;
	return sqrtf(-2 * logf(u1)) * cosf(2 * D3DX_PI * u2);
}

// phillips Spectrum
// K: normalized wave vector, W: wind direction, v: wind velocity, a: amplitude constant
float phillips(const D3DXVECTOR2 &K, const D3DXVECTOR2 &W, float v, float a, float dir_depend)
{
	// largest possible wave from constant wind of velocity v
	float l = v * v / GRAV_ACCEL;
	// damp out waves with very small length w << l
	float w = l / 1000;

	float Ksqr = K.x * K.x + K.y * K.y;
	float Kcos = K.x * W.x + K.y * W.y;
	float phillips = a * expf(-1 / (l * l * Ksqr)) / (Ksqr * Ksqr * Ksqr) * (Kcos * Kcos);

	// filter out waves moving opposite to wind
	if (Kcos < 0)
		phillips *= dir_depend;

	// damp out waves with very small length w << l
	return phillips * expf(-Ksqr * w * w);
}