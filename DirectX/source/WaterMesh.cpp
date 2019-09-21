/*
  *水网格实现
  *2018年3月10日
  *2018/3/10
 */
#include "WaterMesh.h"
#include "DirectEngine.h"
#include<stdlib.h>
WaterMesh::WaterMesh():
	_vertexBuffer(nullptr)
	,_indexBuffer(nullptr)
	,_device(nullptr)
	,_row(129)
	,_column(129)
	,_K1(0.25f)
	,_K2(0.25f)
	,_K3(0.25f)
	,_prevMesh(nullptr)
	,_currentMesh(nullptr)
	,_effect(nullptr)
	,_effectTech(nullptr)
	,v_mvpMatrix(nullptr)
	,v_color(nullptr)
	,_rasterizerState(nullptr)
	,_inputLayout(nullptr)
	,_color(1.0f,1.0f,0.0f,1.0f)
	,_deltaTime(0)
	, _rainTime(0)
{

}

void  WaterMesh::init(int row, int column,float dx,float dt,float speed,float damping)
{
	//计算网格的扰动系数,具体的原理请参<3D游戏与计算机图形学中的数学方法>第15章 流体仿真
	float  d = damping * dt + 2.0f;
	float  e = speed * speed * dt * dt/(dx*dx);
	//
	_K1 = (damping * dt - 2.0f) / d;
	_K2 = (4.0f - 8.0f*e)/d;
	_K3 = 2.0f * e/d;
	_dtInterval = dt;
	//
	_device = DirectEngine::getInstance()->getDevice();
	//
	initMesh(row, column);
	//
	initEffect();
	//
	initLayout();
	//
	initRasterizerState();
	//
	auto &winSize = DirectEngine::getInstance()->getWinSize();
	D3DXMatrixPerspectiveFovLH(&_projMatrix, M_PI / 4.0f, winSize.width / winSize.height, 2.0f, 500.0f);
	D3DXVECTOR3   eyePosition(0,56,-80);
	D3DXVECTOR3   targetPosition(0,0,0);
	D3DXVECTOR3   upperVec3(0,1,0);
	D3DXMatrixLookAtLH(&_viewMatrix, &eyePosition, &targetPosition, &upperVec3);
	D3DXMatrixIdentity(&_modelMatrix);
	_mvpMatrix = _modelMatrix * _viewMatrix * _projMatrix;
}

void WaterMesh::initMesh(int row, int column)
{
	_row = row;
	_column = column;
	//
	_vertexCount = row*column;
	_prevMesh = new float[_vertexCount * 3];
	_currentMesh = new float[_vertexCount *3];
	int index = 0;
	float  halfZ = (row - 1) / 2.0f;
	float  halfX = (column - 1) / 2.0f;
	for (int i = 0; i < row; ++i)
	{
		float    z = i - halfZ;
		for (int j = 0; j < column; ++j)
		{
			float x = j - halfX;

			_prevMesh[index] = x;
			_prevMesh[index + 1] = 0;
			_prevMesh[index + 2] = z;

			_currentMesh[index] = x;
			_currentMesh[index + 1] = 0;
			_currentMesh[index + 2] = z;

			index += 3;
		}
	}
	//建立顶点缓冲区对象
	D3D10_BUFFER_DESC  bufferDesc;
	bufferDesc.ByteWidth = sizeof(float)*_vertexCount * 3;
	bufferDesc.Usage = D3D10_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA  resData;
	resData.pSysMem = _prevMesh;

	int result = _device->CreateBuffer(&bufferDesc, &resData, &_vertexBuffer);
	assert(result>=0);
	//索引
	_indexCount = (row - 1)*(column-1)*6;
	int     *indexData = new int[_indexCount];
	index = 0;
	for (int i = 0; i < row - 1; ++i)
	{
		for (int j = 0; j < column - 1; ++j)
		{
			indexData[index] = i * column + j;
			indexData[index + 1] = (i+1) * column + j;
			indexData[index + 2] = (i+1)*column + j+1;

			indexData[index + 3] = (i + 1)*column + j + 1;
			indexData[index + 4] = i*column +j+1;
			indexData[index + 5] = i*column + j;

			index += 6;
		}
	}
	bufferDesc.ByteWidth = sizeof(int)*_indexCount;
	bufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	resData.pSysMem = indexData;

	result = _device->CreateBuffer(&bufferDesc, &resData, &_indexBuffer);
	assert(result>=0);
	delete[] indexData;
	indexData = nullptr;
}

void WaterMesh::initEffect()
{
	int shaderFlag = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlag |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob  *errorBlob = nullptr;
	int result = D3DX10CreateEffectFromFile(L"ColorUniform.fx", nullptr, nullptr,
		"fx_4_0", shaderFlag, 0, _device, nullptr, nullptr, &_effect, &errorBlob, nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), nullptr, 0);
			errorBlob->Release();
		}
	}
	//effect technique
	_effectTech = _effect->GetTechniqueByName("ColorTech");
	v_mvpMatrix = _effect->GetVariableByName("g_MVPMatrix")->AsMatrix();
	v_color = _effect->GetVariableByName("g_Color")->AsVector();
}

void WaterMesh::initLayout()
{
	D3D10_INPUT_ELEMENT_DESC vertexDesc;
	vertexDesc.SemanticName = "POSITION";
	vertexDesc.SemanticIndex = 0;
	vertexDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc.InputSlot = 0;
	vertexDesc.AlignedByteOffset = 0;
	vertexDesc.InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	vertexDesc.InstanceDataStepRate = 0;

	D3D10_PASS_DESC  passDesc;
	_effectTech->GetPassByName("P0")->GetDesc(&passDesc);

	int result = _device->CreateInputLayout(&vertexDesc, 1, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize,&_inputLayout);
	assert(result>=0);
}

void WaterMesh::initRasterizerState()
{
	D3D10_RASTERIZER_DESC  rasterDesc;
	memset(&rasterDesc,0,sizeof(D3D10_RASTERIZER_DESC));
	rasterDesc.FillMode = D3D10_FILL_WIREFRAME;
	rasterDesc.CullMode = D3D10_CULL_BACK;

	int result = _device->CreateRasterizerState(&rasterDesc, &_rasterizerState);
	assert(result>=0);
}

void WaterMesh::update(float dt)
{
	_deltaTime += dt;
	if (_deltaTime >= _dtInterval)
	{
		_deltaTime = 0;
		//计算顶点的高度场
		for (int i = 1; i < _row - 1; ++i)
		{
			int baseIndex = (i * _column) * 3;
			for (int j = 1; j < _column - 1; ++j)
			{
				int secondaryIndex = baseIndex + j * 3 + 1;
				_prevMesh[secondaryIndex] = _K1 * _prevMesh[secondaryIndex] + _K2 *_currentMesh[secondaryIndex]
					+ _K3 *(_currentMesh[secondaryIndex + 3] + _currentMesh[secondaryIndex + _column * 3]
						+ _currentMesh[secondaryIndex - 3] + _currentMesh[secondaryIndex - _column* 3]);
			}
		}
		//交换指针
		float *swapMesh = _prevMesh;
		_prevMesh = _currentMesh;
		_currentMesh = swapMesh;
		//重新映射顶点缓冲区对象
		float *vertexPointer = nullptr;
		_vertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&vertexPointer);
		memcpy(vertexPointer, _currentMesh, sizeof(float) * 3 * _vertexCount);
		_vertexBuffer->Unmap();
	}
	_rainTime += dt;
	if (_rainTime > 0.25f)
	{
		_rainTime = 0.0f;
		//disturb
		int x = rand() % (_column - 3) + 1;
		int y = rand() % (_row - 3) + 1;
		float    magnitude = 3.0f*rand() / RAND_MAX - 1.5f;
		disturb(x, y, magnitude);
	}
}

void WaterMesh::render()
{
	_device->IASetInputLayout(_inputLayout);
	_device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_device->RSSetState(_rasterizerState);

	D3D10_TECHNIQUE_DESC  techDesc;
	_effectTech->GetDesc(&techDesc);

	//设置全局变量的数值
	v_mvpMatrix->SetMatrix((float*)&_mvpMatrix);
	v_color->SetFloatVector((float*)&_color);

	_effectTech->GetPassByIndex(0)->Apply(0);

	unsigned stride = sizeof(float) * 3;
	unsigned offset = 0;

	_device->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	_device->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	_device->DrawIndexed(_indexCount, 0, 0);
}

void WaterMesh::disturb(int x, int y,float magnitude)
{
	assert(y > 0 && y < _row - 1 && x>0 && x<_column-1);

	int baseIndex = (y * _column + x) * 3 +1;
	float half = magnitude *0.5f;

	_currentMesh[baseIndex] += magnitude;
	_currentMesh[baseIndex - 3] = half;
	_currentMesh[baseIndex + 3] = half;
	_currentMesh[baseIndex + _column * 3] = half;
	_currentMesh[baseIndex - _column * 3] = half;
}