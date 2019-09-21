/*
  *2018/3/21
  *火焰粒子系统
  *author:xiaohuaxiong
 */
#include "Particles.h"
#include "engine/DirectEngine.h"
#define PARTICLE_COUNT  500
struct ParticleVertex
{
	D3DXVECTOR3  position;
	D3DXVECTOR3  velocity;
	D3DXVECTOR2  size;
	float                        age;
	unsigned               type;
};
Particles::Particles():
	_flareResourceView(nullptr)
	,_randomResourceView(nullptr)
	,_bufferPrev(nullptr)
	,_bufferNext(nullptr)
	,_effect(nullptr)
	,_effectStreamTech(nullptr)
	,_effectColorTech(nullptr)
	,_inputLayoutStream(nullptr)
	,_inputLayoutColor(nullptr)
	,v_viewProjMatrix(nullptr)
	,v_eyePosition(nullptr)
	,v_emmiterPosition(nullptr)
	,v_totalTime(nullptr)
	,v_timeInterval(nullptr)
	,_camera(nullptr)
	,_isFirstRun(true)
	,_delayTime(0)
	,_timeInterval(0)
	,_emmiterPosition(0,0,0)
{
}

Particles::~Particles()
{
	_flareResourceView->Release();
	_randomResourceView->Release();
	_effect->Release();
	_inputLayoutStream->Release();
	_inputLayoutColor->Release();
	//
	_camera->release();
}

void    Particles::init()
{
	initStreamBuffer();
	initEffect();
	initInputLayout();
	initTexture();
	//
	auto &winSize = DirectEngine::getInstance()->getWinSize();
	_camera = Camera::createPerspective(60.0f, winSize.width/winSize.height,1.2f,100.0f);
	D3DXVECTOR3   eyePosition(0.0f, 1.8f, -10.0f);
	_camera->lookAt(eyePosition, _emmiterPosition);
}

void Particles::initStreamBuffer()
{
	//创建顶点缓冲区对象
	D3D10_BUFFER_DESC   bufferDesc;
	bufferDesc.ByteWidth = sizeof(ParticleVertex) * PARTICLE_COUNT;
	bufferDesc.Usage = D3D10_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D10_BIND_STREAM_OUTPUT | D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA  resData;

	ParticleVertex *data = new ParticleVertex[PARTICLE_COUNT];
	for (int k = 0; k < 50; ++k)
	{
		data[k].type = 0;
		data[k].age = 0;
	}
	resData.pSysMem = data;
	int result = _device->CreateBuffer(&bufferDesc, &resData, &_bufferPrev);
	assert(result>=0);

	result = _device->CreateBuffer(&bufferDesc,&resData,&_bufferNext);
	assert(result>=0);
	delete[] data;
}

void Particles::initEffect()
{
	unsigned shaderFlag = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlag |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob *errorBlob = nullptr;
	int result = D3DX10CreateEffectFromFile(L"fx/Particles.fx", nullptr, nullptr, "fx_4_0", shaderFlag, 0, _device, nullptr, nullptr, &_effect, &errorBlob, nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr,(char*)errorBlob->GetBufferPointer(),nullptr,0);
			errorBlob->Release();
		}
	}
	//
	_effectStreamTech = _effect->GetTechniqueByName("StreamTech");
	_effectColorTech = _effect->GetTechniqueByName("ColorTech");
	v_viewProjMatrix = _effect->GetVariableByName("g_ViewProjMatrix")->AsMatrix();
	v_eyePosition = _effect->GetVariableByName("g_EyePosition")->AsVector();
	v_emmiterPosition = _effect->GetVariableByName("g_EmitterPosition")->AsVector();
	v_totalTime = _effect->GetVariableByName("g_TotalTime")->AsScalar();
	v_timeInterval = _effect->GetVariableByName("g_TimeInterval")->AsScalar();
	v_baseTexture = _effect->GetVariableByName("g_BaseTexture")->AsShaderResource();
	v_randomTexture = _effect->GetVariableByName("g_RandomTexture")->AsShaderResource();
}

void Particles::initInputLayout()
{
	D3D10_INPUT_ELEMENT_DESC  inputDesc[5] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D10_INPUT_PER_VERTEX_DATA,0},//0
		{"VELOCITY",0,DXGI_FORMAT_R32G32B32_FLOAT,0,sizeof(float)*3,D3D10_INPUT_PER_VERTEX_DATA,0},//1
		{"SIZE",0,DXGI_FORMAT_R32G32_FLOAT,0,sizeof(float)*6,D3D10_INPUT_PER_VERTEX_DATA,0},//2
		{"AGE",0,DXGI_FORMAT_R32_FLOAT,0,sizeof(float)*8,D3D10_INPUT_PER_VERTEX_DATA,0},//3
		{"TYPE",0,DXGI_FORMAT_R32_UINT,0,sizeof(float)*9,D3D10_INPUT_PER_VERTEX_DATA,0},//4
	};
	D3D10_PASS_DESC passDesc;
	_effectStreamTech->GetPassByIndex(0)->GetDesc(&passDesc);
	int result = _device->CreateInputLayout(inputDesc,5,passDesc.pIAInputSignature,passDesc.IAInputSignatureSize,&_inputLayoutStream);
	assert(result>=0);

	_effectColorTech->GetPassByIndex(0)->GetDesc(&passDesc);
	result = _device->CreateInputLayout(inputDesc, 5, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &_inputLayoutColor);
	assert(result>=0);
}

void  Particles::initTexture()
{
	//
	int result = D3DX10CreateShaderResourceViewFromFile(_device, L"image/flare0.dds", nullptr, nullptr, &_flareResourceView, nullptr);
	assert(result>=0);
	//random texture 1d
	D3DXVECTOR4     randomVec3[1024];
	for (int k = 0; k < 1024; ++k)
	{
		randomVec3[k].x = 2.0f * rand() / RAND_MAX - 1.0f;
		randomVec3[k].y = 2.0f * rand() / RAND_MAX - 1.0f;
		randomVec3[k].z = 2.0f * rand() / RAND_MAX - 1.0f;
		randomVec3[k].w = 2.0f* rand() / RAND_MAX - 1.0f;
	}
	//Create Texture
	D3D10_TEXTURE1D_DESC  textureDesc;
	textureDesc.Width = 1024;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.Usage = D3D10_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA resData;
	resData.pSysMem = randomVec3;
	//The distance (in bytes) from the beginning of one line of a texture to the next line. System-memory pitch is used only for 2D and 3D texture data as it is has no meaning for the other resource types.
	resData.SysMemPitch = sizeof(randomVec3);
	//The distance (in bytes) from the beginning of one depth level to the next. System-memory-slice pitch is only used for 3D texture data as it has no meaning for the other resource types.
	resData.SysMemSlicePitch = sizeof(randomVec3);
	// 
	ID3D10Texture1D *randomTexture;
	result = _device->CreateTexture1D(&textureDesc, &resData, &randomTexture);
	assert(result>=0);
	//create shader resource view
	D3D10_SHADER_RESOURCE_VIEW_DESC  shaderResourceViewDesc;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE1D;
	shaderResourceViewDesc.Texture1D.MipLevels = 1;
	shaderResourceViewDesc.Texture1D.MostDetailedMip = 0;
	result = _device->CreateShaderResourceView(randomTexture, &shaderResourceViewDesc, &_randomResourceView);
	assert(result>=0);
	randomTexture->Release();
}

void  Particles::update(float dt)
{
	_delayTime += dt;
	_timeInterval = dt;
}

void Particles::render()
{
	//_device->OMSetDepthStencilState(0, 0);
	//float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//_device->OMSetBlendState(0, blendFactor, 0xffffffff);
	//图元
	unsigned stride = sizeof(ParticleVertex);
	unsigned offset = 0;
	_device->IASetInputLayout(_inputLayoutStream);
	_device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
	_device->IASetVertexBuffers(0, 1, &_bufferPrev, &stride, &offset);
	_device->SOSetTargets(1, &_bufferNext, &offset);
	//
	v_viewProjMatrix->SetMatrix((float*)_camera->getViewProjMatrix().m);
	v_emmiterPosition->SetFloatVector(&_emmiterPosition.x);
	D3DXVECTOR3 eyePosition = _camera->getEyePosition();
	v_eyePosition->SetFloatVector(&eyePosition.x);
	v_totalTime->SetFloat(_delayTime);
	v_timeInterval->SetFloat(_timeInterval);
	v_baseTexture->SetResource(_flareResourceView);
	v_randomTexture->SetResource(_randomResourceView);
	//
	_effectStreamTech->GetPassByIndex(0)->Apply(0);
	//
	if (_isFirstRun)
	{
		_device->Draw(1, 0);
		_isFirstRun = 0;
	}
	else
		_device->DrawAuto();
	ID3D10Buffer *emptyBuffer = nullptr;
	_device->SOSetTargets(1, &emptyBuffer, &offset);
	//draw
	_device->IASetVertexBuffers(0, 1, &_bufferNext, &stride, &offset);
	_device->IASetInputLayout(_inputLayoutColor);
	_effectColorTech->GetPassByIndex(0)->Apply(0);
	_device->DrawAuto();
	//_device->OMSetBlendState(0, blendFactor, 0xffffffff); // restore default
	//Swap
	emptyBuffer = _bufferNext;
	_bufferNext = _bufferPrev;
	_bufferPrev = emptyBuffer;
}