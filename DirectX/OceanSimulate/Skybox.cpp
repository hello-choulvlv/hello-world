/*
  *天空盒实现
  *2018年3月31日
  *@author:xiaohuaxiong
 */
#include "Skybox.h"
#include<d3dx11.h>
#include<assert.h>
Skybox::Skybox():
	_device(nullptr),
	_deviceContext(nullptr),
	_vertexShader(nullptr),
	_pixelShader(nullptr),
	_vertexBuffer(nullptr),
	_cbPerFrameBuffer(nullptr),
	_inputLayout(nullptr),
	_skyboxCubeView(nullptr),
	_samplerState(nullptr),
	_depthStencilState(nullptr)
{
}

Skybox::~Skybox()
{
	_vertexShader->Release();
	_pixelShader->Release();
	_vertexBuffer->Release();
	_cbPerFrameBuffer->Release();
	_inputLayout->Release();
	//
	_skyboxCubeView->Release();
	_samplerState->Release();
	_depthStencilState->Release();
}

Skybox *Skybox::create(ID3D11Device *device, ID3D11DeviceContext *deviceContext)
{
	Skybox *skybox = new Skybox();
	skybox->initSkybox(device, deviceContext);
	return skybox;
}

void   Skybox::initSkybox(ID3D11Device *device, ID3D11DeviceContext *deviceContext)
{
	_device = device;
	_deviceContext = deviceContext;
	//
	float      vertex_data[] = {
		-1.0f,-1.0f, 1.0f, //lb
		-1.0f,1.0f, 1.0f,   //lt
		1.0f,-1.0f, 1.0f,   //rb
		1.0f,1.0f,  1.0f,    //rt
	};
	D3D11_BUFFER_DESC  bufferDesc;
	bufferDesc.ByteWidth = sizeof(vertex_data);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA   dataDesc;
	dataDesc.pSysMem = vertex_data;
	dataDesc.SysMemPitch = 0;
	dataDesc.SysMemSlicePitch = 0;
	int result = _device->CreateBuffer(&bufferDesc, &dataDesc, &_vertexBuffer);
	assert(result>=0);
	//Shader
	ID3D10Blob    *errorBlob = nullptr;
	ID3D10Blob    *vertexBlob = nullptr;
	ID3D10Blob    *pixelBlob = nullptr;
	unsigned compileFlag = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	compileFlag |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	result = D3DX11CompileFromFile(L"shader/skybox.hlsl", nullptr, nullptr, "vs_main","vs_5_0", 
		compileFlag, 0, nullptr, &vertexBlob, &errorBlob, nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), nullptr, 0);
			errorBlob->Release();
			errorBlob = nullptr;
		}
	}
	result = D3DX11CompileFromFile(L"shader/skybox.hlsl",nullptr,nullptr,"ps_main","ps_5_0",
		compileFlag,0,nullptr,&pixelBlob,&errorBlob,nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), nullptr, 0);
			errorBlob->Release();
			errorBlob = nullptr;
		}
	}
	//shader
	result = _device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &_vertexShader);
	assert(result>=0);
	result = _device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &_pixelShader);
	assert(result>=0);
	//
	D3D11_INPUT_ELEMENT_DESC  inputDesc[2];
	inputDesc[0].SemanticName = "POSITION";
	inputDesc[0].SemanticIndex = 0;
	inputDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputDesc[0].InputSlot = 0;
	inputDesc[0].AlignedByteOffset = 0;
	inputDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputDesc[0].InstanceDataStepRate = 0;

	//inputDesc[1].SemanticName = "COLOR";
	//inputDesc[1].SemanticIndex = 0;
	//inputDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//inputDesc[1].InputSlot = 0;
	//inputDesc[1].AlignedByteOffset = sizeof(float) * 3;
	//inputDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	//inputDesc[1].InstanceDataStepRate = 0;

	result = _device->CreateInputLayout(inputDesc, 1, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &_inputLayout);
	vertexBlob->Release();
	pixelBlob->Release();
	assert(result>=0);
	//Shader resource
	result = D3DX11CreateShaderResourceViewFromFile(device, L"media/sky_cube.dds",nullptr,nullptr,&_skyboxCubeView,nullptr);
	assert(result>=0);
	//Sampler State
	D3D11_SAMPLER_DESC  samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 0;
	result = _device->CreateSamplerState(&samplerDesc, &_samplerState);
	assert(result>=0);
	/////////////全局缓冲区对象
	bufferDesc.ByteWidth = sizeof(float)*16;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	result = _device->CreateBuffer(&bufferDesc, nullptr, &_cbPerFrameBuffer);
	assert(result>=0);
	//depth stencil state /////////
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	memset(&depthStencilDesc,0,sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = false;
	result = _device->CreateDepthStencilState(&depthStencilDesc, &_depthStencilState);
	assert(result>=0);
}

void Skybox::onUpdate(float deltaTime,Camera &camera)
{
	D3DXMATRIX matView = s_reflect_matrix * camera.getViewMatrix(),matTranspose;
	_mvpMatrix = matView * camera.getProjMatrix();
	D3DXMatrixInverse(&matView, nullptr, &_mvpMatrix);
	D3DXMatrixTranspose(&matTranspose, &matView);

	D3D11_MAPPED_SUBRESOURCE   resMap;
	_deviceContext->Map(_cbPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD,0,&resMap);
	memcpy(resMap.pData,&matTranspose,sizeof(matTranspose));
	_deviceContext->Unmap(_cbPerFrameBuffer,0);
}

void Skybox::onRender(Camera &camera)
{
	unsigned stride = sizeof(float)*3;
	unsigned offset = 0;
	_deviceContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	_deviceContext->IASetInputLayout(_inputLayout);
	_deviceContext->VSSetShader(_vertexShader, nullptr, 0);
	_deviceContext->PSSetShader(_pixelShader, nullptr, 0);
	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//
	_deviceContext->VSSetConstantBuffers(0, 1, &_cbPerFrameBuffer);
	_deviceContext->PSSetSamplers(0, 1, &_samplerState);
	_deviceContext->PSSetShaderResources(0, 1, &_skyboxCubeView);
	//
	unsigned depthStencilRef = 0;
	ID3D11DepthStencilState  *lastDepthStencilState = nullptr;
	_deviceContext->OMGetDepthStencilState(&lastDepthStencilState,&depthStencilRef);
	_deviceContext->OMSetDepthStencilState(_depthStencilState, 0x1);
	//
	_deviceContext->Draw(4, 0);
	_deviceContext->OMSetDepthStencilState(lastDepthStencilState, depthStencilRef);
}