/*
  *立方体渲染实现
  *@2018年3月2日
  *@author:xiaohuaxiong
 */
#include "Cube.h"
#include "DirectEngine.h"
#include<d3d10.h>
#include<d3dx10.h>
#include<dxerr.h>
#include<math.h>

Cube::Cube():
	_vertexBuffer(nullptr)
	,_indexBuffer(nullptr)
	,_vertexCount(0)
	,_indexCount(0)
{
}

Cube::~Cube()
{
	_vertexBuffer->Release();
	_indexBuffer->Release();
}

void Cube::init()
{
	ID3D10Device *d3dDevice = DirectEngine::getInstance()->getDevice();
	//左手坐标系
	float   vertexData[] = {
		-0.5f,-0.5f,-0.5f,      1.0f,1.0f,1.0f,1.0f,//0
		-0.5f,0.5f,-0.5f,        1.0f,0.0f,0.0f,1.0f,//1
		0.5f,0.5f,-0.5f,          0,1,0,1,//2
		0.5f,-0.5f,-0.5f,         0,0,1,1,//3
		-0.5f,-0.5f,0.5f,         0,0,0,1,//4
		-0.5f,0.5f,0.5f,           1.0f,1.0f,0.0f,1.0f,//5
		0.5f,0.5f,0.5f,             1.0f,0.0f,1.0f,1.0f,//6
		0.5f,-0.5f,0.5f,           0.0f,1.0f,1.0f,1.0f,//7
	};
	struct Vertex
	{
		D3DXVECTOR3 pos;
		D3DXCOLOR   color;
	};
	const D3DXCOLOR WHITE(1.0f, 1.0f, 1.0f, 1.0f);
	const D3DXCOLOR BLACK(0.0f, 0.0f, 0.0f, 1.0f);
	const D3DXCOLOR RED(1.0f, 0.0f, 0.0f, 1.0f);
	const D3DXCOLOR GREEN(0.0f, 1.0f, 0.0f, 1.0f);
	const D3DXCOLOR BLUE(0.0f, 0.0f, 1.0f, 1.0f);
	const D3DXCOLOR YELLOW(1.0f, 1.0f, 0.0f, 1.0f);
	const D3DXCOLOR CYAN(0.0f, 1.0f, 1.0f, 1.0f);
	const D3DXCOLOR MAGENTA(1.0f, 0.0f, 1.0f, 1.0f);

	const D3DXCOLOR BEACH_SAND(1.0f, 0.96f, 0.62f, 1.0f);
	const D3DXCOLOR LIGHT_YELLOW_GREEN(0.48f, 0.77f, 0.46f, 1.0f);
	const D3DXCOLOR DARK_YELLOW_GREEN(0.1f, 0.48f, 0.19f, 1.0f);
	const D3DXCOLOR DARKBROWN(0.45f, 0.39f, 0.34f, 1.0f);
	Vertex vertices[] =
	{
		{ D3DXVECTOR3(-1.0f, -1.0f, -1.0f), WHITE },//0
		{ D3DXVECTOR3(-1.0f, +1.0f, -1.0f), BLACK },//1
		{ D3DXVECTOR3(+1.0f, +1.0f, -1.0f), RED },//2
		{ D3DXVECTOR3(+1.0f, -1.0f, -1.0f), GREEN },//3
		{ D3DXVECTOR3(-1.0f, -1.0f, +1.0f), BLUE },//4
		{ D3DXVECTOR3(-1.0f, +1.0f, +1.0f), YELLOW },//5
		{ D3DXVECTOR3(+1.0f, +1.0f, +1.0f), CYAN },//6
		{ D3DXVECTOR3(+1.0f, -1.0f, +1.0f), MAGENTA },//7
	};
	unsigned   indexData[] = {
		0,1,2,
		0,2,3,

		3,2,6,
		3,6,7,

		7,6,5,
		7,5,4,

		4,5,1,
		4,1,0,

		4,0,3,
		4,3,7,

		1,5,6,
		1,6,2,
	};

	_vertexCount = sizeof(vertexData)/(sizeof(float)*7);
	_indexCount = sizeof(indexData) / sizeof(int);
	//
	D3D10_BUFFER_DESC   bufferDesc;
	bufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA  bufferData;
	bufferData.pSysMem = vertexData;

	long result = d3dDevice->CreateBuffer(&bufferDesc, &bufferData, &_vertexBuffer);
	assert(result >= 0);

	D3D10_BUFFER_DESC   bufferDesc2;
	bufferDesc2.Usage = D3D10_USAGE_IMMUTABLE;
	bufferDesc2.ByteWidth = sizeof(indexData);
	bufferDesc2.BindFlags = D3D10_BIND_INDEX_BUFFER;
	bufferDesc2.CPUAccessFlags = 0;
	bufferDesc2.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA bufferData2;
	bufferData2.pSysMem = indexData;

	result = d3dDevice->CreateBuffer(&bufferDesc2,&bufferData2,&_indexBuffer);
	assert(result>=0);
	//Shader
	int shaderFlag = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlag |= D3D10_SHADER_DEBUG|D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob   *errorBlob=nullptr;
	result = D3DX10CreateEffectFromFile(L"color.fx",nullptr,nullptr,"fx_4_0",shaderFlag,
		0,d3dDevice,nullptr,nullptr,&_fxEffect,&errorBlob,nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr,(char*)errorBlob->GetBufferPointer(),nullptr,0);
			errorBlob->Release();
		}
		DXTrace(__FILE__, __LINE__,result,L"D3D10CreateEffectFromeFile",true);
	}
	_effectTechnique = _fxEffect->GetTechniqueByName("ColorTech");
	_mvpMatrixV = _fxEffect->GetVariableByName("gWVP")->AsMatrix();
	//Vertex Layout
	D3D10_INPUT_ELEMENT_DESC inputDesc[2];
	inputDesc[0].SemanticName = "POSITION";
	inputDesc[0].SemanticIndex = 0;
	inputDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputDesc[0].InputSlot = 0;
	inputDesc[0].AlignedByteOffset = 0;
	inputDesc[0].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	inputDesc[0].InstanceDataStepRate = 0;

	inputDesc[1].SemanticName = "COLOR";
	inputDesc[1].SemanticIndex = 0;
	inputDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputDesc[1].InputSlot = 0;
	inputDesc[1].AlignedByteOffset = sizeof(float)*3;
	inputDesc[1].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	inputDesc[1].InstanceDataStepRate = 0;

	D3D10_PASS_DESC passDesc;
	_effectTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	result = d3dDevice->CreateInputLayout(inputDesc, 2,
		passDesc.pIAInputSignature, passDesc.IAInputSignatureSize,&_vertexLayout);
	//
	if (result < 0)
	{
		DXTrace(__FILE__, __LINE__, result, L"d3dDevice->CreateInputLayout",true);
	}
	D3DXMatrixIdentity(&_modelMatrix);
	D3DXMatrixIdentity(&_projMatrix);
	auto &winSize = DirectEngine::getInstance()->getWinSize();
	D3DXMatrixPerspectiveFovLH(&_projMatrix,M_PI/4,winSize.x/winSize.y,0.5f,10.0f);

	D3DXVECTOR3 pos(2, 2, -2);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&_viewMatrix, &pos, &target, &up);

	_mvpMatrix = _viewMatrix * _projMatrix;

	_box.init(d3dDevice, 0.5f);
}

void   Cube::update(float dt)
{

}

void Cube::render()
{
	ID3D10Device *d3dDevice = DirectEngine::getInstance()->getDevice();
	//d3dDevice->OMSetDepthStencilState(nullptr, 0);
	//float blendFactor[4] = { 0,0,0,0 };
	//d3dDevice->OMSetBlendState(nullptr, blendFactor, -1);
	d3dDevice->IASetInputLayout(_vertexLayout);
	d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//
	int result = _mvpMatrixV->SetMatrix((float*)&_mvpMatrix);
	//
	D3D10_TECHNIQUE_DESC techDesc;
	_effectTechnique->GetDesc(&techDesc);
	_effectTechnique->GetPassByIndex(0)->Apply(0);

	////输入汇编阶段
	unsigned   offset = 0;
	unsigned   stride = sizeof(float) * 7;
	d3dDevice->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	d3dDevice->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	d3dDevice->DrawIndexed(_indexCount, 0, 0);
	//_box.draw();
}