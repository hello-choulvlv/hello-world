/*
  *Texture DirectX实现
  *@2018年3月13日
  *@author:xiaohuaxiong
 */
#include "Texture.h"
#include "DirectEngine.h"
Texture::Texture() :
	_device(nullptr)
	, _textureView(nullptr)
	, _specularView(nullptr)
	, _vertexBuffer(nullptr)
	, _indexBuffer(nullptr)
	, _vertexCount(0)
	, _indexCount(0)
	, _lightEffect(nullptr)
	, _lightEffectTech(nullptr)
	, v_modelMatrix(nullptr)
	, v_viewProjMatrix(nullptr)
	, v_lightDirection(nullptr)
	,v_texture(nullptr)
	,v_specular(nullptr)
	,_lightInputLayout(nullptr)
	,_deltaTime(0)
	, _lightDirection(1,4,0)
{
	
}

void Texture::init()
{
	_device = DirectEngine::getInstance()->getDevice();
	//
	initBuffer();
	//
	initEffect();
	//
	initLayout();
	//
	initTexture();
	//
	D3DXMatrixIdentity(&_viewProjMatrix);
	D3DXMatrixIdentity(&_modelMatrix);
	auto &winSize = DirectEngine::getInstance()->getWinSize();
	D3DXMATRIX   viewMatrix, projMatrix;
	D3DXMatrixIdentity(&viewMatrix);
	D3DXMatrixIdentity(&projMatrix);
	D3DXMatrixPerspectiveFovLH(&projMatrix, M_PI / 4, winSize.width / winSize.height, 1.0f, 200.0f);
	//
	D3DXVECTOR3   eyePosition(0,0,-6);
	D3DXVECTOR3   targetPosition(0,0,0);
	D3DXVECTOR3   upperVector(0,1,0);
	D3DXMatrixLookAtLH(&viewMatrix,&eyePosition,&targetPosition ,&upperVector);

	_viewProjMatrix = viewMatrix * projMatrix;
	D3DXVec3Normalize(&_lightDirection, &_lightDirection);
}

void Texture::initBuffer()
{
	//顶点数据
	float    vertexData[] = {
		//-Z
		-1,-1,-1, 0,-1,0,  0,1,//0
		-1,1,-1,    0,-1,0, 0,0,//1
		1,1,-1,     0,-1,0, 1,0,//2
		1, -1,-1,  0,-1,0,  1,1,//3
		//+X
		1,1,-1, 1,0,0, 0,0,//4,
		1,1,1,   1,0,0, 1,0,//5
		1,-1,1,  1,0,0, 1,1,//6
		1, -1, -1, 1,0,0, 0, 1,//7
		//+Z
		1,1,1,  0,0,1,  0,0,//8
		-1,1,1, 0,0,1,  1,0,//9
		-1,-1,1, 0,0,1, 1,1,//10
		1,-1,1, 0,0,1,   0,1,//11
		//-X
		-1,1,1, -1,0,0,  0,0,//12
		-1,1,-1, -1,0,0, 1,0,//13
		-1,-1,-1, -1,0,0, 1,1,//14
		-1,-1,1, -1,0,0,  0,1,//15
		//+Y
		-1,1,1,  0,1,0,  0,0,//16
		1,1,1,    0,1,0, 1,0,//17
		1,1,-1,   0,1,0, 1,1,//18
		-1,1,-1,  0,1,0, 0,1,//19
		//-Y
		-1,-1,-1, 0,-1,0, 0,0,//20
		1,-1,-1,   0,-1,0, 1,0,//21
		1,-1,1,     0,-1,0, 1,1,//22
		-1,-1,1,  0,-1,0,   0,1,//23
	};
	//index
	short   indexData[] = {
		0,1,2,2,3,0, // -Z
		4,5,6,6,7,4,//+X
		8,9,10,10,11,8,//+Z
		12,13,14,14,15,12,//-X
		16,17,18,18,19,16,//+Y
		20,21,22,22,23,20,//-Y
	};
	_indexCount = sizeof(indexData)/sizeof(short);
	//顶点缓冲区对象
	D3D10_BUFFER_DESC     bufferDesc;
	bufferDesc.ByteWidth = sizeof(vertexData);
	bufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA  resData;
	resData.pSysMem = vertexData;

	int result = _device->CreateBuffer(&bufferDesc,&resData,&_vertexBuffer);
	assert(result>=0);

	bufferDesc.ByteWidth = sizeof(indexData);
	bufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	resData.pSysMem = indexData;

	result = _device->CreateBuffer(&bufferDesc, &resData, &_indexBuffer);
	assert(result>=0);
}

void Texture::initEffect()
{
	unsigned shaderFlag = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlag |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob   *errorBlob = nullptr;
	int result = D3DX10CreateEffectFromFile(L"LightTexture.fx",nullptr,nullptr,"fx_4_0",shaderFlag,0,
		_device,nullptr,nullptr,&_lightEffect,&errorBlob,nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr,(char*)errorBlob->GetBufferPointer(),nullptr,0);
			errorBlob->Release();
		}
	}
	_lightEffectTech = _lightEffect->GetTechniqueByName("LightTech");
	v_modelMatrix = _lightEffect->GetVariableByName("g_ModelMatrix")->AsMatrix();
	v_viewProjMatrix = _lightEffect->GetVariableByName("g_ViewProjMatrix")->AsMatrix();
	v_normalMatrix = _lightEffect->GetVariableByName("g_NormalMatrix")->AsMatrix();
	v_lightDirection = _lightEffect->GetVariableByName("g_LightDirection")->AsVector();
	v_texture = _lightEffect->GetVariableByName("g_DiffuseMap")->AsShaderResource();
	//v_specular = _lightEffect->GetVariableByName("g_SpecularMap")->AsShaderResource();
}

void Texture::initLayout()
{
	D3D10_INPUT_ELEMENT_DESC  inputDesc[3];
	//Position
	inputDesc[0].SemanticName = "POSITION";
	inputDesc[0].SemanticIndex = 0;
	inputDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputDesc[0].InputSlot = 0;
	inputDesc[0].AlignedByteOffset = 0;
	inputDesc[0].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	inputDesc[0].InstanceDataStepRate = 0;
	//Normal
	inputDesc[1].SemanticName = "NORMAL";
	inputDesc[1].SemanticIndex = 0;
	inputDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputDesc[1].InputSlot = 0;
	inputDesc[1].AlignedByteOffset = sizeof(float) * 3;
	inputDesc[1].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	inputDesc[1].InstanceDataStepRate = 0;
	//FragCoord
	inputDesc[2].SemanticName = "TEXCOORD";
	inputDesc[2].SemanticIndex = 0;
	inputDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputDesc[2].InputSlot = 0;
	inputDesc[2].AlignedByteOffset = sizeof(float)*6;
	inputDesc[2].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	inputDesc[2].InstanceDataStepRate = 0;
	//
	D3D10_PASS_DESC  passDesc;
	_lightEffectTech->GetPassByIndex(0)->GetDesc(&passDesc);
	int result = _device->CreateInputLayout(inputDesc, 3, passDesc.pIAInputSignature,passDesc.IAInputSignatureSize,&_lightInputLayout);
	assert(result>=0);
}

void Texture::initTexture()
{
	int result = D3DX10CreateShaderResourceViewFromFile(_device, L"WoodCrate01.dds",nullptr,nullptr,&_textureView,nullptr);
	assert(result>=0);

	result = D3DX10CreateShaderResourceViewFromFile(_device,L"WoodCrate02.dds",nullptr,nullptr,&_specularView,nullptr);
	assert(result>=0);
}

void Texture::update(float dt)
{
	_deltaTime += dt;
	D3DXVECTOR3   axis(1,4,1);
	D3DXVec3Normalize(&axis, &axis);
	//D3DXMatrixIdentity(&_modelMatrix);
	D3DXMatrixRotationAxis(&_modelMatrix, &axis,_deltaTime);
}

void Texture::render()
{
	_device->IASetInputLayout(_lightInputLayout);
	_device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//
	unsigned stride = sizeof(float)*8;
	unsigned offset = 0;
	_device->IASetVertexBuffers(0,1,&_vertexBuffer,&stride,&offset);
	_device->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	v_modelMatrix->SetMatrix((float*)&_modelMatrix);
	v_viewProjMatrix->SetMatrix((float*)&_viewProjMatrix);
	v_normalMatrix->SetMatrix((float*)&_modelMatrix);
	v_texture->SetResource(_textureView);
	v_lightDirection->SetFloatVector(&_lightDirection.x);

	_lightEffectTech->GetPassByIndex(0)->Apply(0);

	_device->DrawIndexed(_indexCount,0,0);
}