/*
  *镜像反射
  *2018/3/16
  *@author:xiaohuaxiong
 */
#include "Reflect.h"
#include "engine/DirectEngine.h"
#include "engine/Geometry.h"
#include "engine/dxTypes.h"
#include "engine/EventManager.h"
#define KEY_MASK_W 0x1
#define KEY_MASK_S   0x2
#define KEY_MASK_A   0x4
#define KEY_MASK_D  0x8
Reflect::Reflect():
	_vertexBufferBox(nullptr)
	,_indexBufferBox(nullptr)
	,_vertexCountBox(0)
	,_indexCountBox(0)
	,_vertexBufferWall(nullptr)
	,_vertexCountWall(0)
	,_vertexBufferReflect(nullptr)
	,_effect(nullptr)
	,_effectTech(nullptr)
	,_lightInputLayoutBox(nullptr)
	,v_modelMatrix(nullptr)
	,v_viewProjMatrix(nullptr)
	,v_lightDirection(nullptr)
	,v_baseTexture(nullptr)
	,_textureWall(nullptr)
	,_textureReflect(nullptr)
	,_textureMesh(nullptr)
	,_keyListener(nullptr)
	,_keyMask(0)
{
}

Reflect::~Reflect()
{
}

void  Reflect::init()
{
	initBuffer();
	initEffect();
	initInputLayout();
	initTexture();
	//
	initBlendState();
	//
	initRasterizerState();
	//
	initDepthStencilState();
	//
	initMatrix();
	//
	D3DXVECTOR3   lightDirection(1,1,-1);
	D3DXVec3Normalize(&_lightDirection, &lightDirection);
	//
	auto &winSize = DirectEngine::getInstance()->getWinSize();
	_camera = Camera::createPerspective(45, winSize.width / winSize.height, 1.2f, 200.0f);
	//
	D3DXVECTOR3  eyePosition(-5, 2, -8);
	D3DXVECTOR3  targetPosition(0, 0, 0);
	_camera->lookAt(eyePosition, targetPosition);
	//
	_touchListener = TouchEventListener::create(this,touch_pressed_callback(Reflect::onTouchPressed),
																								touch_moved_callback(Reflect::onTouchMoved),
																							touch_released_callback(Reflect::onTouchReleased));
	EventManager::getInstance()->addTouchListener(_touchListener,0);

	_keyListener = KeyEventListener::create(this,key_pressed_callback(Reflect::onKeyPressed),key_released_callback(Reflect::onKeyReleased));
	EventManager::getInstance()->addKeyListener(_keyListener, 0);
}

void   Reflect::initBuffer()
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
	_indexCountBox = sizeof(indexData) / sizeof(short);
	//顶点缓冲区对象
	D3D10_BUFFER_DESC     bufferDesc;
	bufferDesc.ByteWidth = sizeof(vertexData);
	bufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA  resData;
	resData.pSysMem = vertexData;

	int result = _device->CreateBuffer(&bufferDesc, &resData, &_vertexBufferBox);
	assert(result >= 0);

	bufferDesc.ByteWidth = sizeof(indexData);
	bufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	resData.pSysMem = indexData;

	result = _device->CreateBuffer(&bufferDesc, &resData, &_indexBufferBox);
	assert(result >= 0);
	////墙体==>XOY平面
	float wallVertex[] = {
		-1.0f,-1.0f,0.0f,   0.0f,0.0f,-1.0f,  0.0f,1.0f,//0
		-1.0f,1.0f,0.0f,     0.0f,0.0f,-1.0f,  0.0f,0.0f,//1

		1.0f,-1.0f,0.0f,     0.0f,0.0f,-1.0f, 1.0f,1.0f,//2
		1.0f,1.0f,0.0f,      0.0f,0.0f,-1.0f,   1.0f,0.0f,//3
	};
	_vertexCountWall = 4;
	bufferDesc.ByteWidth = sizeof(wallVertex);
	bufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	resData.pSysMem = wallVertex;
	result = _device->CreateBuffer(&bufferDesc, &resData, &_vertexBufferWall);
	assert(result>=0);
	//镜子
	float reflectVertex[] = {
		-0.5f,-0.5f,0.0f,  0.0f,0.0f,-1.0f,   0.0f,1.0f,
		-0.5f, 0.5f, 0.0f, 0.0f,0.0f,-1.0f,    0.0f,0.0f,
		0.5f,-0.5f,0.0f,   0.0f,0.0f,-1.0f,    1.0f,1.0f,
		0.5f,0.5f,0.0f,    0.0f,0.0f,-1.0f,     1.0f,0.0f,
	};
	_vertexCountReflect = 4;
	bufferDesc.ByteWidth = sizeof(reflectVertex);
	bufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	resData.pSysMem = reflectVertex;
	result = _device->CreateBuffer(&bufferDesc, &resData, &_vertexBufferReflect);
	assert(result>=0);
}

void Reflect::initEffect()
{
	unsigned int shaderFlag = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlag |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob  *errorBlob = nullptr;
	int result = D3DX10CreateEffectFromFile(L"fx/Reflect.fx",nullptr,nullptr,"fx_4_0",shaderFlag,0,_device,nullptr,nullptr,&_effect,&errorBlob,nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), nullptr, 0);
			errorBlob->Release();
		}
	}
	_effectTech = _effect->GetTechniqueByName("ReflectTech");
	v_modelMatrix = _effect->GetVariableByName("g_ModelMatrix")->AsMatrix();
	v_viewProjMatrix = _effect->GetVariableByName("g_ViewProjMatrix")->AsMatrix();
	v_lightDirection = _effect->GetVariableByName("g_LightDirection")->AsVector();
	v_baseTexture = _effect->GetVariableByName("g_BaseTexture")->AsShaderResource();
}

void Reflect::initTexture()
{
	int result = D3DX10CreateShaderResourceViewFromFile(_device, L"image/brick01.dds",nullptr,nullptr,&_textureWall,nullptr);
	assert(result>=0);

	result = D3DX10CreateShaderResourceViewFromFile(_device,L"image/ice.dds",nullptr,nullptr,&_textureReflect,nullptr);
	assert(result>=0);

	result = D3DX10CreateShaderResourceViewFromFile(_device,L"image/checkboard.dds",nullptr,nullptr,&_textureMesh,nullptr);
	assert(result>=0);

	result = D3DX10CreateShaderResourceViewFromFile(_device,L"image/WoodCrate02.dds",nullptr,nullptr,&_textureBox,nullptr);
	assert(result>=0);
}

void Reflect::initMatrix()
{
	auto &winSize = DirectEngine::getInstance()->getWinSize();
	D3DXMATRIX  view, proj;
	D3DXMatrixPerspectiveFovLH(&proj, M_PI / 4, winSize.width / winSize.height, 1.2f, 200.0f);

	D3DXVECTOR3  eyePosition(0,2,-8);
	D3DXVECTOR3  targetPosition(0,0,0);
	D3DXVECTOR3  upperVec(0,1,0);

	D3DXMatrixLookAtLH(&view, &eyePosition, &targetPosition,&upperVec);
	_viewProjMatrix = view * proj;
}

void  Reflect::initInputLayout()
{
	//Box Input Layout
	D3D10_INPUT_ELEMENT_DESC    inputDesc[3];
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
	inputDesc[1].AlignedByteOffset = sizeof(float)*3;
	inputDesc[1].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	inputDesc[1].InstanceDataStepRate = 0;
	//Tex Coord
	inputDesc[2].SemanticName = "TEXCOORD";
	inputDesc[2].SemanticIndex = 0;
	inputDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputDesc[2].InputSlot = 0;
	inputDesc[2].AlignedByteOffset = sizeof(float)*6;
	inputDesc[2].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	inputDesc[2].InstanceDataStepRate = 0;
	//
	D3D10_PASS_DESC   passDesc;
	_effectTech->GetPassByIndex(0)->GetDesc(&passDesc);

	int result = _device->CreateInputLayout(inputDesc, 3, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &_lightInputLayoutBox);
	assert(result>=0);
	///////////////////Wall+Reflect///////////////////////
}

void Reflect::update(float dt)
{
	if (_keyMask)
	{
		float speed = 1.0f;
		float dx = 0;
		float dz = 0;
		if (_keyMask & KEY_MASK_W)
			dz += speed *dt;
		if (_keyMask & KEY_MASK_S)
			dz -= speed *dt;
		if (_keyMask & KEY_MASK_A)
			dx -= speed *dt;
		if (_keyMask & KEY_MASK_D)
			dx += speed *dt;
		_camera->translate(dx, dz);
	}
}

void Reflect::render()
{
	//墙
	unsigned stride = sizeof(float)*8;
	unsigned offset = 0;
	_device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	_device->IASetInputLayout(_lightInputLayoutBox);
	_device->IASetVertexBuffers(0, 1, &_vertexBufferWall,&stride,&offset );
	//
	D3DXMATRIX   affineTransform,scaleMatrix,modelMatrix;
	D3DXMatrixTranslation(&affineTransform, 0, 0, 6);
	D3DXMatrixScaling(&scaleMatrix, 6.0f, 6.0f, 1.0f);
	modelMatrix = scaleMatrix * affineTransform;

	v_modelMatrix->SetMatrix((float*)modelMatrix.m);
	v_viewProjMatrix->SetMatrix((float*)&_camera->getViewProjMatrix());
	v_baseTexture->SetResource(_textureWall);
	v_lightDirection->SetFloatVector(&_lightDirection.x);
	//
	_effectTech->GetPassByIndex(0)->Apply(0);
	_device->Draw(_vertexCountWall, 0);
	//箱子
	_device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_device->IASetVertexBuffers(0, 1, &_vertexBufferBox, &stride, &offset);
	_device->IASetIndexBuffer(_indexBufferBox, DXGI_FORMAT_R16_UINT, 0);

	D3DXMatrixTranslation(&_modelMatrix,1,0,-1);
	v_modelMatrix->SetMatrix((float*)&_modelMatrix);
	v_viewProjMatrix->SetMatrix((float*)&_camera->getViewProjMatrix());
	v_lightDirection->SetFloatVector(&_lightDirection.x);
	v_baseTexture->SetResource(_textureBox);
	_effectTech->GetPassByIndex(0)->Apply(0);
	_device->DrawIndexed(_indexCountBox, 0, 0);
	//
	_device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//切换深度模版测试状态
	_device->OMSetDepthStencilState(_depthStencilNormal,0x1);
	_device->IASetVertexBuffers(0, 1, &_vertexBufferReflect, &stride, &offset);
	D3DXMatrixTranslation(&affineTransform, 0, 0, 4.0f);
	D3DXMatrixScaling(&scaleMatrix, 4.0f, 4.0f, 1.0f);
	modelMatrix = scaleMatrix * affineTransform;
	v_modelMatrix->SetMatrix((float*)&modelMatrix);
	v_viewProjMatrix->SetMatrix((float*)&_camera->getViewProjMatrix());
	v_baseTexture->SetResource(_textureReflect);
	v_lightDirection->SetFloatVector(&_lightDirection.x);
	//
	_effectTech->GetPassByIndex(0)->Apply(0);
	_device->Draw(_vertexCountReflect, 0);
	//再次渲染箱子
	_device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_device->OMSetDepthStencilState(_depthStencilReflect, 0x1);
	_device->RSSetState(_rasterizerState);
	float blendFactor[4] = { 0.65f, 0.65f, 0.65f, 1.0f };
	_device->OMSetBlendState(_blendStateReflect, blendFactor,-1);
	//draw
	_device->IASetVertexBuffers(0, 1, &_vertexBufferBox, &stride, &offset);
	_device->IASetIndexBuffer(_indexBufferBox, DXGI_FORMAT_R16_UINT, 0);

	D3DXMATRIX    reflectMatrix,translate;
	D3DXPLANE       plane(0,0,-1,4);
	D3DXVECTOR3 lightDirection;
	CreateReflectMatrix(reflectMatrix, plane);
	D3DXMatrixTranslation(&translate, 1, 0, -1);
	D3DXVec3TransformNormal(&lightDirection, &_lightDirection, &reflectMatrix);
	_modelMatrix = translate * reflectMatrix;

	v_modelMatrix->SetMatrix((float*)&_modelMatrix);
	v_viewProjMatrix->SetMatrix((float*)&_camera->getViewProjMatrix());
	v_lightDirection->SetFloatVector(&lightDirection.x);
	v_baseTexture->SetResource(_textureBox);
	_effectTech->GetPassByIndex(0)->Apply(0);
	_device->DrawIndexed(_indexCountBox, 0, 0);
	//restore
	float blendFactor2[4] = {0,0,0,0};
	_device->OMSetBlendState(nullptr, blendFactor2,-1);
	_device->RSSetState(nullptr);
	_device->OMSetDepthStencilState(nullptr,0);
}

bool Reflect::onTouchPressed(const Vec2 &touchPoint)
{
	_offsetVec2 = touchPoint;
	return true;
}

void Reflect::onTouchMoved(const Vec2 &touchPoint)
{
	float dx = touchPoint.x - _offsetVec2.x;
	float dy = touchPoint.y - _offsetVec2.y;

	_camera->rotate(-dy*0.15f, dx*0.15f);

	_offsetVec2 = touchPoint;
}

void Reflect::onTouchReleased(const Vec2 &touchPoint)
{

}

void Reflect::onKeyPressed(KeyCode keyCode)
{
	if (keyCode == KeyCode_W)
		_keyMask |= KEY_MASK_W;
	else if (keyCode == KeyCode_S)
		_keyMask |= KEY_MASK_S;
	else if (keyCode == KeyCode_A)
		_keyMask |= KEY_MASK_A;
	else if (keyCode == KeyCode_D)
		_keyMask |= KEY_MASK_D;
}

void Reflect::onKeyReleased(KeyCode keyCode)
{
	if (keyCode == KeyCode_W)
		_keyMask &= ~KEY_MASK_W;
	else if (keyCode == KeyCode_S)
		_keyMask &= ~KEY_MASK_S;
	else if (keyCode == KeyCode_A)
		_keyMask &= ~KEY_MASK_A;
	else if (keyCode == KeyCode_D)
		_keyMask &= ~KEY_MASK_D;
}
//光栅化对象
void Reflect::initRasterizerState()
{
	D3D10_RASTERIZER_DESC    rasterDesc;
	memset(&rasterDesc,0,sizeof(rasterDesc));

	rasterDesc.FillMode = D3D10_FILL_SOLID;
	rasterDesc.CullMode = D3D10_CULL_BACK;
	rasterDesc.FrontCounterClockwise = true;//以逆时针为正

	int result = _device->CreateRasterizerState(&rasterDesc, &_rasterizerState);
}
//模板深度测试
void Reflect::initDepthStencilState()
{
	D3D10_DEPTH_STENCIL_DESC   depthStencilDesc;
	//渲染镜子时需要的状态设置
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D10_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	//正面模板操作
	depthStencilDesc.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D10_STENCIL_OP_REPLACE;
	depthStencilDesc.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
	//背面操作
	depthStencilDesc.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D10_STENCIL_OP_REPLACE;
	depthStencilDesc.BackFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
	int result = _device->CreateDepthStencilState(&depthStencilDesc, &_depthStencilNormal);
	assert(result>=0);
	//渲染镜子中的物体
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D10_COMPARISON_ALWAYS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	depthStencilDesc.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D10_COMPARISON_EQUAL;

	depthStencilDesc.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D10_COMPARISON_EQUAL;
	result = _device->CreateDepthStencilState(&depthStencilDesc, &_depthStencilReflect);
}

void Reflect::initBlendState()
{
	D3D10_BLEND_DESC   blendDesc;
	memset(&blendDesc,0,sizeof(blendDesc));

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.BlendEnable[0] = true;
	blendDesc.SrcBlend = D3D10_BLEND_BLEND_FACTOR;
	blendDesc.DestBlend = D3D10_BLEND_INV_BLEND_FACTOR;
	blendDesc.BlendOp = D3D10_BLEND_OP_ADD;
	blendDesc.SrcBlendAlpha = D3D10_BLEND_ONE;
	blendDesc.DestBlendAlpha = D3D10_BLEND_ZERO;
	blendDesc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
	blendDesc.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;

	int result = _device->CreateBlendState(&blendDesc, &_blendStateReflect);
}