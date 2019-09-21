/*
  *山谷与山峰d3d实现
  *2018年3月6日
  *@author:xiaohuaxiong
 */
#include "PeaksAndValleys.h"
#include "DirectEngine.h"
#include<math.h>

static float static_get_height(float x, float z)
{
	return 0.3f*(z*sinf(0.1f*x) + x * cosf(0.1f*z));
}
PeaksAndValleys::PeaksAndValleys():
	_meshRow(0)
	,_meshColumn(0)
	,_vertexCount(0)
	,_indexCount(0)
	,_vertexBuffer(nullptr)
	,_indexBuffer(nullptr)
	,_deltaTime(0)
{

}

PeaksAndValleys::~PeaksAndValleys()
{
	_vertexBuffer->Release();
	_indexBuffer->Release();
}

void PeaksAndValleys::init(int row, int column)
{
	initMesh(row, column);
	//Create Effect Technique
	int shaderFlag = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG	)
	shaderFlag |= D3D10_SHADER_DEBUG |D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Device *device = DirectEngine::getInstance()->getDevice();
	ID3D10Blob *errorBlob = nullptr;
	int result = D3DX10CreateEffectFromFile(L"color.fx",
		nullptr,nullptr,
		"fx_4_0",
		shaderFlag,
		0, 
		device,
		nullptr,
		nullptr,
		&_effect,&errorBlob,nullptr
		);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), nullptr, 0);
			errorBlob->Release();
		}
		DXTrace(__FILE__, __LINE__, result, L"D3DX10CreateEffectFromFile",true);
	}
	_effectTech = _effect->GetTechniqueByName("ColorTech");
	_mvpMatrixV = _effect->GetVariableByName("g_MVPMatrix")->AsMatrix();
	//Create Layout
	D3D10_INPUT_ELEMENT_DESC   inputDesc[2];
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
	inputDesc[1].AlignedByteOffset = sizeof(float) * 3;
	inputDesc[1].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	inputDesc[1].InstanceDataStepRate = 0;
	//
	D3D10_PASS_DESC   passDesc;
	_effectTech->GetPassByName("P0")->GetDesc(&passDesc);
	result = device->CreateInputLayout(inputDesc, 2, passDesc.pIAInputSignature,passDesc.IAInputSignatureSize,&_inputLayout);
	assert(result>=0);
	//Matrix
	D3DXMatrixIdentity(&_modelMatrix);
	D3DXMatrixIdentity(&_projMatrix);
	D3DXMatrixIdentity(&_viewMatrix);
	//Create Project Matrix
	auto &winSize = DirectEngine::getInstance()->getWinSize();
	D3DXMatrixPerspectiveFovLH(&_projMatrix,M_PI/4,winSize.width/winSize.height,1.0f,400.0f);
	D3DXVECTOR3   eyePosition(0,80,-120);
	D3DXVECTOR3   targetPosition(0,0,0);
	D3DXVECTOR3   upperVec(0,1,0);
	D3DXMatrixLookAtLH(&_viewMatrix, &eyePosition, &targetPosition, &upperVec);
}

void PeaksAndValleys::initMesh(int row, int column)
{
	_meshRow = row;
	_meshColumn = column;
	//
	_vertexCount = row * column;
	_indexCount = (row - 1)*(column - 1) * 6;
	float   *vertexData = new float[_vertexCount * 7];
	//创建网格的依据时左手坐标系,并且中心点在原点
	float     halfCenterX = (column-1)*0.5f;
	float     halfCenterZ = (row-1) *0.5f;
	int  index = 0;
	for (int i = 0; i < row; ++i)
	{
		float Z = i - halfCenterZ;
		for (int j = 0; j < column; ++j)
		{
			float x = j - halfCenterX;
			float y = static_get_height(x,Z);
			//Vertex
			vertexData[index] = x;
			vertexData[index + 1] = y;
			vertexData[index + 2] = Z;
			//Color
			if (y < -10)
			{
				vertexData[index + 3] = 1.0f;
				vertexData[index + 4] = 0.96f;
				vertexData[index + 5] = 0.62f;
			}
			else if (y < 5)
			{
				vertexData[index + 3] = 0.48f;
				vertexData[index + 4] = 0.77f;
				vertexData[index + 5] = 0.46;
			}
			else if (y < 12)
			{
				vertexData[index + 3] = 0.1f;
				vertexData[index + 4] = 0.48f;
				vertexData[index + 5] = 0.19f;
			}
			else if (y < 20)
			{
				vertexData[index + 3] = 0.45f;
				vertexData[index + 4] = 0.39f;
				vertexData[index + 5] = 0.34f;
			}
			else
			{
				vertexData[index + 3] = 1.0f;
				vertexData[index + 4] = 1.0f;
				vertexData[index + 5] = 1.0f;
			}
			vertexData[index + 6] = 1.0f;
			index += 7;
		}
	}
	//创建顶点缓冲区对象
	D3D10_BUFFER_DESC  bufferDesc;
	bufferDesc.ByteWidth = sizeof(float)*_vertexCount*7;
	bufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	//
	D3D10_SUBRESOURCE_DATA    resourceData;
	resourceData.pSysMem = vertexData;
	//
	ID3D10Device *device = DirectEngine::getInstance()->getDevice();
	int result = device->CreateBuffer(&bufferDesc,&resourceData,&_vertexBuffer);
	assert(result>=0);
	//inde buffer
	short *indexData = (short*)vertexData;
	index = 0;
	for (int i = 0; i < row - 1; ++i)
	{
		for (int j = 0; j < column - 1; ++j)
		{
			indexData[index] = i*column +j;
			indexData[index + 1] = (i + 1)*column + j;
			indexData[index+2] = (i+1)*column +(j+1);

			indexData[index + 3] = (i+1)*column +(j+1);
			indexData[index + 4] = i*column +j+1;
			indexData[index + 5] = i*column+j;

			index += 6;
		}
	}
	//
	D3D10_BUFFER_DESC indexDesc;
	indexDesc.ByteWidth = sizeof(short)*_indexCount;
	indexDesc.Usage = D3D10_USAGE_IMMUTABLE;
	indexDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	indexDesc.CPUAccessFlags = 0;
	indexDesc.MiscFlags = 0;
	//
	D3D10_SUBRESOURCE_DATA indexResource;
	indexResource.pSysMem = indexData;
	result = device->CreateBuffer(&indexDesc,&indexResource,&_indexBuffer);
	assert(result>=0);
	//
	delete[] vertexData;
	vertexData = nullptr;
}

void  PeaksAndValleys::update(float dt)
{
	_deltaTime += dt*0.5f;
	D3DXMatrixRotationY(&_modelMatrix, _deltaTime);
}

void PeaksAndValleys::render()
{
	ID3D10Device *device = DirectEngine::getInstance()->getDevice();
	device->IASetInputLayout(_inputLayout);
	device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DXMATRIX mvpMatrix = _modelMatrix * _viewMatrix * _projMatrix;
	_mvpMatrixV->SetMatrix((float*)&mvpMatrix);

	_effectTech->GetPassByIndex(0)->Apply(0);

	unsigned vertexStride = sizeof(float)*7;
	unsigned offset = 0;

	device->IASetVertexBuffers(0, 1, &_vertexBuffer, &vertexStride, &offset);
	device->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	device->DrawIndexed(_indexCount, 0, 0);
}