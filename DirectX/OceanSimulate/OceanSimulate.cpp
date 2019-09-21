/*
  *海平面实现
  *2018年4月3日
  *@author:xiaohuaxiong
 */
#include "OceanSimulate.h"
#include<assert.h>
//#include "other/utilities.h"
#include<assert.h>
using namespace concurrency;
using namespace concurrency::graphics;
using namespace std;
//#define   USE_OTHER_OCEAN
OceanSimulate::OceanSimulate(OceanParam &param, ID3D11Device *device, ID3D11DeviceContext *deviceContext) :
	_device(device),
	_deviceContext(deviceContext),
	_acceleratorView(concurrency::direct3d::create_accelerator_view(reinterpret_cast<IUnknown*>(device))),
	_array_view_float2_h0(nullptr),
	_array_view_float_omega(nullptr),
	_array_view_float2_ht(nullptr),
	_array_view_float2_dxyz(nullptr),
	_fft_512x512_plan(3,_acceleratorView)
{
	//空间网格划分,以及初始高度场/角度场的生成
	_oceanParam = param;
	int    map_dim = param.map_dim;
	vector<float_2>   h0_vector((map_dim+1)*(map_dim+1));
	vector<float>        omega_vector((map_dim+1)*(map_dim+1));
	vector<float_2>   zero_data(3*map_dim*map_dim);
	memset(zero_data.data(),0,sizeof(float_2)*3*map_dim*map_dim);

	init_height_map(param, h0_vector, omega_vector);

	init_buffer(param, h0_vector,omega_vector, zero_data);
	//初始化DirectX 11引擎
	init_directx11(param, 3 * map_dim*map_dim, sizeof(float_2));
}

OceanSimulate::~OceanSimulate()
{
	_displacementTexture->Release();
	_displacementShaderView->Release();
	_displacementRenderView->Release();
	_gradientTexture->Release();
	_gradientShaderView->Release();
	_gradientRenderView->Release();
	_samplerStatePoint->Release();

	delete _array_view_float2_h0;
	delete _array_view_float_omega;
	delete _array_view_float2_ht;
	delete _array_view_float2_dxyz;

	_buffer_float2_dxyz->Release();
	_shaderViewDxyz->Release();
	_unorderViewDxyz->Release();
	_quad_buffer->Release();
	_quad_vertex_shader->Release();
	_displace_pixel_shader->Release();
	_gradient_pixel_shader->Release();
	_quad_layout->Release();
	_immutable_buffer->Release();
}

OceanSimulate *OceanSimulate::create(OceanParam &param, ID3D11Device *device, ID3D11DeviceContext *deviceContext)
{
	OceanSimulate *ocean = new OceanSimulate(param,device,deviceContext);
	//ocean->empty();
	return ocean;
}

void  OceanSimulate::init_buffer(const OceanParam &param, std::vector<concurrency::graphics::float_2> &ho_data, std::vector<float> &omega_data, std::vector<concurrency::graphics::float_2> &zero_data)
{
	int map_dim = param.map_dim;
	int buffer_size = (map_dim + 1)*(map_dim+1);
	int dxyz_buffer_size = 3 * map_dim*map_dim;
	_array_view_float2_h0 = new array_view<const float_2>(concurrency::array<float_2>(buffer_size,ho_data.begin(),_acceleratorView));

	_array_view_float_omega = new array_view<const float>(concurrency::array<float>(buffer_size,omega_data.begin(),_acceleratorView));

	_array_view_float2_ht = new array_view<float_2>(concurrency::array<float_2>(dxyz_buffer_size,zero_data.begin(),_acceleratorView));

	concurrency::array<float_2>    temp_array_buffer(dxyz_buffer_size,zero_data.begin(),_acceleratorView);
	//random access view 
	_array_view_float2_dxyz = new array_view<float_2>(temp_array_buffer);
	_buffer_float2_dxyz = reinterpret_cast<ID3D11Buffer*>(concurrency::direct3d::get_buffer(temp_array_buffer));
}
#ifndef USE_OTHER_OCEAN
void OceanSimulate::init_directx11(const OceanParam &param, int total_size, int sizeof_float2)
{
	int map_dim = param.map_dim;
	createUnorderAccessView(_buffer_float2_dxyz,3* map_dim*map_dim*sizeof_float2,
	&_unorderViewDxyz,&_shaderViewDxyz);
	//createUAV(_device, _buffer_float2_dxyz, 3 * map_dim*map_dim*sizeof(float_2), sizeof(float),
	//	&_unorderViewDxyz,&_shaderViewDxyz);
	//位移
	createTextureAndViews2(map_dim, map_dim, DXGI_FORMAT_R32G32B32A32_FLOAT, 
							&_displacementTexture, &_displacementShaderView, &_displacementRenderView);
	//createTextureAndViews(_device, map_dim, map_dim, DXGI_FORMAT_R32G32B32A32_FLOAT, 
	//	&_displacementTexture,&_displacementShaderView,&_displacementRenderView);
	//梯度
	createTextureAndViews2(map_dim, map_dim, DXGI_FORMAT_R16G16B16A16_FLOAT, 
	&_gradientTexture, &_gradientShaderView, &_gradientRenderView);
	//createTextureAndViews(_device, map_dim, map_dim, DXGI_FORMAT_R16G16B16A16_FLOAT, 
	//	&_gradientTexture, &_gradientShaderView, &_gradientRenderView);
	//Sampler
	D3D11_SAMPLER_DESC   samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 1;
	samplerDesc.BorderColor[1] = 1;
	samplerDesc.BorderColor[2] = 1;
	samplerDesc.BorderColor[3] = 1;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	int result = _device->CreateSamplerState(&samplerDesc, &_samplerStatePoint);
	assert(result>=0);
	//create shader
	ID3D10Blob   *errorBlob = nullptr;
	ID3D10Blob   *quadBlob = nullptr;
	ID3D10Blob   *displacePixelBlob = nullptr;
	ID3D10Blob   *gradientPixelBlob = nullptr;
	uint shaderFlag = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(_DEBUG) || defined(DEBUG)
	shaderFlag |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	result = D3DX11CompileFromFile(L"shader/ocean_simulate.hlsl",nullptr,nullptr,"Quad_VS","vs_5_0", shaderFlag,0,
		nullptr,&quadBlob,&errorBlob,nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr,(char*)errorBlob->GetBufferPointer(),nullptr,0);
			errorBlob->Release();
			errorBlob = nullptr;
		}
	}
	result = D3DX11CompileFromFile(L"shader/ocean_simulate.hlsl",nullptr,nullptr,"Gen_Displacement_FS","ps_5_0",
		shaderFlag,0,nullptr,&displacePixelBlob,&errorBlob,nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr,static_cast<char*>(errorBlob->GetBufferPointer()),nullptr,0);
			errorBlob->Release();
			errorBlob->Release();
			errorBlob = nullptr;
		}
	}
	result = D3DX11CompileFromFile(L"shader/ocean_simulate.hlsl",nullptr,nullptr,"Gen_Gradient_FS","ps_5_0",
		shaderFlag,0,nullptr,&gradientPixelBlob,&errorBlob,nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr,static_cast<char*>(errorBlob->GetBufferPointer()),nullptr,0);
			errorBlob->Release();
			errorBlob = nullptr;
		}
	}
	//create shader
	result = _device->CreateVertexShader(quadBlob->GetBufferPointer(), quadBlob->GetBufferSize(), nullptr, &_quad_vertex_shader);
	assert(result>=0);
	result = _device->CreatePixelShader(displacePixelBlob->GetBufferPointer(), displacePixelBlob->GetBufferSize(), nullptr, &_displace_pixel_shader);
	assert(result>=0);
	result = _device->CreatePixelShader(gradientPixelBlob->GetBufferPointer(), gradientPixelBlob->GetBufferSize(), nullptr, &_gradient_pixel_shader);
	assert(result>=0);
	//释放掉像素Blob shader
	displacePixelBlob->Release();
	gradientPixelBlob->Release();
	//
	D3D11_INPUT_ELEMENT_DESC   inputLayoutDesc;
	inputLayoutDesc.SemanticName = "POSITION";
	inputLayoutDesc.SemanticIndex = 0;
	inputLayoutDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputLayoutDesc.InputSlot = 0;
	inputLayoutDesc.AlignedByteOffset = 0;
	inputLayoutDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayoutDesc.InstanceDataStepRate = 0;
	result = _device->CreateInputLayout(&inputLayoutDesc, 1, quadBlob->GetBufferPointer(), quadBlob->GetBufferSize(), &_quad_layout);
	assert(result>=0);
	quadBlob->Release();
	//////////////Create-Buffer-//////////////////
	float vertex_data[] = {
		-1.0f,-1.0f,0.0f,
		-1.0f,1.0f,0.0f,
		1.0f,-1.0f,0.0f,
		1.0f,1.0f,0.0f
	};
	D3D11_BUFFER_DESC   bufferDesc;
	bufferDesc.ByteWidth = sizeof(vertex_data);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA  resData;
	resData.pSysMem = vertex_data;
	result = _device->CreateBuffer(&bufferDesc, &resData, &_quad_buffer);
	assert(result>=0);
	//immutable buffer
	_immutable.actual_dim = param.map_dim;
	_immutable.output_width = param.map_dim;
	_immutable.output_height = param.map_dim;
	_immutable.dx_offset = param.map_dim * param.map_dim;
	_immutable.dy_offset = 2 * param.map_dim * param.map_dim;
	_immutable.pixel_width = 1.0f / param.map_dim;
	_immutable.pixel_height = 1.0f / param.map_dim;
	_immutable.choppy_scale = param.choppy_scale;

	bufferDesc.ByteWidth = __align16(sizeof(_immutable));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	resData.pSysMem = &_immutable;
	result = _device->CreateBuffer(&bufferDesc, &resData, &_immutable_buffer);
	assert(result>=0);

	_spectrumConstant.actual_dim = param.map_dim;
	_spectrumConstant.input_width = param.map_dim + 1;
	_spectrumConstant.output_width = param.map_dim;
	_spectrumConstant.dx_offset = param.map_dim*param.map_dim;
	_spectrumConstant.dy_offset = 2 * param.map_dim*param.map_dim;
}

void OceanSimulate::init_height_map(OceanParam &param, std::vector<concurrency::graphics::float_2> &h0_float_2, std::vector<float> &omega_float)
{
	srand(0x7);
	const int map_dim = param.map_dim;
	const float half_dim = map_dim / 2.0f;
	float a = param.amplitude * 1e-7f;	// It is too small. We must scale it for editing.
	D3DXVECTOR2   K;
	//纵向
	for (int i = 0; i < map_dim + 1; ++i)
	{
		K.y = 2.0f * M_PI *(i - half_dim)/ param.patch_length;
		//横向
		for (int j = 0; j < map_dim + 1; ++j)
		{
			K.x = 2.0f * M_PI *(j-half_dim)/ param.patch_length;
			float phillipsValue = (K.x == 0 && K.y == 0) ?0:phillips(K,param.wind_dir,param.wind_speed,a,param.wind_dependency);
			phillipsValue = sqrtf(phillipsValue);
			h0_float_2[i*(map_dim + 1) + j].x = phillipsValue * HALF_SQRT_2 * gauss_random();
			h0_float_2[i*(map_dim + 1) + j].y = phillipsValue * HALF_SQRT_2 * gauss_random();

			omega_float[i*(map_dim + 1) + j] = sqrtf(GRAV_ACCEL*sqrtf(K.x*K.x+K.y*K.y));
		}
	}
}
#endif
void update_spectrum(int dim_x,int dim_y,
				accelerator_view  &acceratorView,array_view<const float_2> h0_float2_view,
				array_view<const float> omega_float_view,array_view<float_2> output_float2_view,
	SpectrumConstant  param,float time)
{
	output_float2_view.discard_data();
	concurrency::extent<2>  update_extent(dim_y,dim_x);
	parallel_for_each(acceratorView, update_extent, [=](index<2> idx)restrict(amp) {
		//计算空间的三段域
		index<1>  input_index = index<1>(idx[0] * param.input_width + idx[1]);
		index<1>  input_m_index = index<1>((param.actual_dim - idx[0]) * param.input_width +(param.actual_dim - idx[1]));

		float_2  h0_k = h0_float2_view[input_index];
		float_2  h0_km = h0_float2_view[input_m_index];
		//与相关的复数根进行乘法
		float  sin_v, cos_v;
		fast_math::sincos(omega_float_view[input_index]*time,&sin_v,&cos_v);
		//将h0_k * exp(i*ot) + conjugation(h0_km) * exp(-i*ot)展开
		float_2  ht_k;
		ht_k.x = (h0_k.x + h0_km.x)*cos_v -(h0_k.y +h0_km.y)*sin_v;
		ht_k.y = (h0_k.x - h0_km.x)*sin_v + (h0_k.y - h0_km.y)*cos_v;
		//计算dx/dy
		float_2  ht_dx, ht_dy;
		float       kx = idx[1] - 0.5f * param.actual_dim;
		float       ky = idx[0] - 0.5f*param.actual_dim;
		float       rsqrtk = 0.0f;
		float       pow2_k = kx*kx + ky*ky;
		if (pow2_k > 1e-12)
			rsqrtk = fast_math::rsqrt(pow2_k);
		kx *= rsqrtk;
		ky *= rsqrtk;

		ht_dx.x = ht_k.y * kx;
		ht_dx.y = -ht_k.x * kx;

		ht_dy.x = ht_k.y * ky;
		ht_dy.y = -ht_k.x *ky;

		index<1> output_index = index<1>(idx[0] * param.output_width + idx[1]);
		output_float2_view[output_index] = ht_k;
		output_float2_view[output_index + param.dx_offset] = ht_dx;
		output_float2_view[output_index + param.dy_offset] = ht_dy;
	});
}

void OceanSimulate::update_displacement_map(float dt,float time)
{
	//C++ AMP 进行FFT变换
	update_spectrum(_oceanParam.map_dim, _oceanParam.map_dim, _acceleratorView, 
		*_array_view_float2_h0, *_array_view_float_omega, 
		*_array_view_float2_ht, _spectrumConstant, time*0.8f);
	//进行下一步的逆变换
	_fft_512x512_plan.fft_512x512_c2c_amp(*_array_view_float2_dxyz, *_array_view_float2_ht);
	//获取光栅化视图
	ID3D11RenderTargetView  *old_render_target_view = nullptr;
	ID3D11DepthStencilView    *old_depth_stencil_view = nullptr;
	_deviceContext->OMGetRenderTargets(1, &old_render_target_view, &old_depth_stencil_view);
	//设置视口
	D3D11_VIEWPORT  old_viewport;
	unsigned                       number_of_viewport = 1;
	_deviceContext->RSGetViewports(&number_of_viewport, &old_viewport);
	D3D11_VIEWPORT  new_viewport = { 0,0,_oceanParam.map_dim,_oceanParam.map_dim,0.0f,1.0f };
	_deviceContext->RSSetViewports(1, &new_viewport);
	_deviceContext->OMSetRenderTargets(1, &_displacementRenderView, nullptr);
	//设置顶点/像素shader
	_deviceContext->VSSetShader(_quad_vertex_shader, nullptr, 0);
	_deviceContext->PSSetShader(_displace_pixel_shader, nullptr, 0);
	//设置全局缓冲区对象
	//_deviceContext->VSSetConstantBuffers(0, 1, &_immutable_buffer);
	_deviceContext->PSSetConstantBuffers(0, 1, &_immutable_buffer);
	//设置ByteAccressBuffer对象
	_deviceContext->PSSetShaderResources(0, 1, &_shaderViewDxyz);
	//Input Assembler
	_deviceContext->IASetInputLayout(_quad_layout);
	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	unsigned stride = sizeof(float)*3;
	unsigned offset = 0;
	_deviceContext->IASetVertexBuffers(0, 1, &_quad_buffer,&stride,&offset);
	_deviceContext->Draw(4, 0);
	//
	ID3D11ShaderResourceView *emptyShaderView = nullptr;
	_deviceContext->PSSetShaderResources(0, 1,&emptyShaderView);
	///////////////////////gradient/////////////////////////////////
	_deviceContext->OMSetRenderTargets(1, &_gradientRenderView, nullptr);
	_deviceContext->VSSetShader(_quad_vertex_shader, nullptr, 0);
	_deviceContext->PSSetShader(_gradient_pixel_shader,nullptr,0);
	_deviceContext->PSSetShaderResources(0, 1, &_displacementShaderView);
	_deviceContext->PSSetSamplers(0, 1, &_samplerStatePoint);
	_deviceContext->PSSetConstantBuffers(0, 1, &_immutable_buffer);
	_deviceContext->Draw(4, 0);
	//////////////////restore////////////////////////
	ID3D11Buffer *empty_buffer = nullptr;
	_deviceContext->PSSetConstantBuffers(0, 1, &empty_buffer);
	_deviceContext->RSSetViewports(number_of_viewport, &old_viewport);
	_deviceContext->OMSetRenderTargets(1, &old_render_target_view,old_depth_stencil_view);
}
#ifdef USE_OTHER_OCEAN
void OceanSimulate::init_directx11(const OceanParam &param, int total_size, int sizeof_float2)
{
	createUAV(_device, _buffer_float2_dxyz, 3 * param.map_dim *param.map_dim* sizeof_float2, sizeof(float),
		&_unorderViewDxyz, &_shaderViewDxyz);

	// D3D11 Textures
	createTextureAndViews(_device, param.map_dim, param.map_dim, DXGI_FORMAT_R32G32B32A32_FLOAT, 
		&_displacementTexture, &_displacementShaderView, &_displacementRenderView);
	createTextureAndViews(_device, param.map_dim, param.map_dim, DXGI_FORMAT_R16G16B16A16_FLOAT,
		&_gradientTexture, &_gradientShaderView, &_gradientRenderView);

	// Samplers
	D3D11_SAMPLER_DESC sam_desc;
	sam_desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.MipLODBias = 0;
	sam_desc.MaxAnisotropy = 1;
	sam_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sam_desc.BorderColor[0] = 1.0f;
	sam_desc.BorderColor[1] = 1.0f;
	sam_desc.BorderColor[2] = 1.0f;
	sam_desc.BorderColor[3] = 1.0f;
	sam_desc.MinLOD = -FLT_MAX;
	sam_desc.MaxLOD = FLT_MAX;
	_device->CreateSamplerState(&sam_desc, &_samplerStatePoint);
	assert(_samplerStatePoint);

	// Vertex & pixel shaders
	ID3DBlob* pBlobQuadVS = nullptr;
	ID3DBlob* pBlobUpdateDisplacementPS = nullptr;
	ID3DBlob* pBlobGenGradientFoldingPS = nullptr;

	//tstring filePath = GetFilePath::GetFilePath(_T("ocean_simulator_vs_ps.hlsl"));
	unsigned shaderFlag = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(_DEBUG) || defined(DEBUG)
	shaderFlag |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob  *errorBlob = nullptr;
	int result = D3DX11CompileFromFile(L"shader/ocean_simulate.hlsl", nullptr, nullptr, "Quad_VS", "vs_4_0", shaderFlag, 0, nullptr, &pBlobQuadVS, &errorBlob, nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr, static_cast<char*>(errorBlob->GetBufferPointer()), nullptr, 0);
			errorBlob->Release();
			errorBlob = nullptr;
		}
	}
	assert(result >= 0);
	result = D3DX11CompileFromFile(L"shader/ocean_simulate.hlsl", nullptr, nullptr, "Gen_Displacement_FS", "ps_4_0", shaderFlag, 0, nullptr, &pBlobUpdateDisplacementPS, &errorBlob, nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr, static_cast<char*>(errorBlob->GetBufferPointer()), nullptr, 0);
			errorBlob->Release();
			errorBlob = nullptr;
		}
	}
	assert(result >= 0);
	result = D3DX11CompileFromFile(L"shader/ocean_simulate.hlsl", nullptr, nullptr, "Gen_Gradient_FS", "ps_4_0", shaderFlag, 0, nullptr, &pBlobGenGradientFoldingPS, &errorBlob, nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr, static_cast<char*>(errorBlob->GetBufferPointer()), nullptr, 0);
			errorBlob->Release();
			errorBlob = nullptr;
		}
	}
	assert(result >= 0);
	assert(pBlobQuadVS);
	assert(pBlobUpdateDisplacementPS);
	assert(pBlobGenGradientFoldingPS);

	_device->CreateVertexShader(pBlobQuadVS->GetBufferPointer(), pBlobQuadVS->GetBufferSize(), nullptr, &_quad_vertex_shader);
	_device->CreatePixelShader(pBlobUpdateDisplacementPS->GetBufferPointer(), pBlobUpdateDisplacementPS->GetBufferSize(), nullptr, &_displace_pixel_shader);
	_device->CreatePixelShader(pBlobGenGradientFoldingPS->GetBufferPointer(), pBlobGenGradientFoldingPS->GetBufferSize(), nullptr, &_gradient_pixel_shader);
	assert(_quad_vertex_shader);
	assert(_displace_pixel_shader);
	assert(_gradient_pixel_shader);
	pBlobUpdateDisplacementPS->Release();
	pBlobGenGradientFoldingPS->Release();

	// Input layout
	D3D11_INPUT_ELEMENT_DESC quad_layout_desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	_device->CreateInputLayout(quad_layout_desc, 1, pBlobQuadVS->GetBufferPointer(), pBlobQuadVS->GetBufferSize(), &_quad_layout);
	assert(_quad_layout);

	pBlobQuadVS->Release();

	// Quad vertex buffer
	D3D11_BUFFER_DESC vb_desc;
	vb_desc.ByteWidth = 4 * sizeof(D3DXVECTOR3);
	vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vb_desc.CPUAccessFlags = 0;
	vb_desc.MiscFlags = 0;

	float quad_verts[] =
	{
		-1, -1, 0,
		-1,  1, 0,
		1, -1, 0,
		1,  1, 0,
	};
	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = &quad_verts[0];
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	_device->CreateBuffer(&vb_desc, &init_data, &_quad_buffer);
	assert(_quad_buffer);

	// Constant buffers
	UINT actual_dim = param.map_dim;
	UINT input_width = actual_dim + 1;

	_immutable.actual_dim = actual_dim;
	_immutable.output_width = actual_dim;
	_immutable.output_height = actual_dim;
	_immutable.dx_offset = actual_dim * actual_dim;
	_immutable.dy_offset = 2 * actual_dim*actual_dim;
	_immutable.pixel_width = 1.0f / actual_dim;
	_immutable.pixel_height = 1.0f / actual_dim;
	_immutable.choppy_scale = param.choppy_scale;

	D3D11_SUBRESOURCE_DATA init_cb0 = { &_immutable, 0, 0 };

	D3D11_BUFFER_DESC cb_desc;
	cb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_desc.CPUAccessFlags = 0;
	cb_desc.MiscFlags = 0;
	cb_desc.ByteWidth = __align16(sizeof(_immutable));

	_device->CreateBuffer(&cb_desc, &init_cb0, &_immutable_buffer);
	assert(_immutable_buffer);

	_spectrumConstant.actual_dim = param.map_dim;
	_spectrumConstant.input_width = param.map_dim + 1;
	_spectrumConstant.output_width = param.map_dim;
	_spectrumConstant.dx_offset = param.map_dim*param.map_dim;
	_spectrumConstant.dy_offset = 2 * param.map_dim*param.map_dim;
}

void OceanSimulate::init_height_map(OceanParam& params, vector<float_2> &out_h0, vector<float> &out_omega)
{
	int i, j;
	D3DXVECTOR2 K, Kn;

	D3DXVECTOR2 wind_dir;
	D3DXVec2Normalize(&wind_dir, &params.wind_dir);
	float a = params.amplitude * 1e-7f;	// It is too small. We must scale it for editing.
	float v = params.wind_speed;
	float dir_depend = params.wind_dependency;

	int height_map_dim = params.map_dim;
	float patch_length = params.patch_length;

	// initialize random generator.
	srand(0x7);
	float halfHeight = height_map_dim / 2.0f;
	/*
	*生成波频谱,生成的算法基于统计模型
	*/
	for (i = 0; i <= height_map_dim; i++)
	{
		// K is wave-vector, range [-|DX/W, |DX/W], [-|DY/H, |DY/H]
		K.y = 2 * D3DX_PI*(i - halfHeight) / patch_length;

		for (j = 0; j <= height_map_dim; j++)
		{
			K.x = 2 * D3DX_PI *(j - halfHeight) / patch_length;

			float phil = (K.x == 0 && K.y == 0) ? 0 : sqrtf(phillips(K, wind_dir, v, a, dir_depend));

			out_h0[i * (height_map_dim + 1) + j].x = float(phil * gauss_random() * 0.7071068f);
			out_h0[i * (height_map_dim + 1) + j].y = float(phil * gauss_random() * 0.7071068f);

			// The angular frequency is following the dispersion relation:
			//            out_omega^2 = g*k
			// So the equation of Gerstner wave is:
			//            x = x0 - K/k * A * sin(dot(K, x0) - sqrt(g * k) * t), x is a 2D vector.
			//            z = A * cos(dot(K, x0) - sqrt(g * k) * t)
			// Gerstner wave shows that a point on a simple sinusoid wave is doing a uniform circular
			// motion with the center at (x0, y0, z0), radius at A, and the circular plane is parallel
			// to vector K.
			out_omega[i * (height_map_dim + 1) + j] = sqrtf(GRAV_ACCEL * sqrtf(K.x * K.x + K.y * K.y));
		}
	}
}
#endif