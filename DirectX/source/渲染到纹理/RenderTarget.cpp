/*
  *渲染到纹理
  *2018/3/19
  *xiaohuaxiong
 */
#include "RenderTarget.h"
#include "engine/DirectEngine.h"

RenderTarget::RenderTarget() :
	_colorShaderView(nullptr)
	,_renderView(nullptr)
	,_depthShaderView(nullptr)
	,_depthView(nullptr)
	,_vertexBuffer(nullptr)
	,_indexBuffer(nullptr)
	,_camera(nullptr)
{
}

RenderTarget::~RenderTarget()
{
	_colorShaderView->Release();
	_renderView->Release();
	_depthShaderView->Release();
	_depthView->Release();
	_vertexBuffer->Release();
	_indexBuffer->Release();
	_camera->release();
	//
	_inputLayout->Release();
	_quadInputLayout->Release();
	_quadBuffer->Release();
	_texture->Release();
	
}

void RenderTarget::init(int w,int h)
{
	_viewPort.TopLeftX = 0;
	_viewPort.TopLeftY = 0;
	_viewPort.Width = w;
	_viewPort.Height = h;
	_viewPort.MinDepth = 0.0f;
	_viewPort.MaxDepth = 1.0f;
	//
	initBuffer();
	initEffect();
	initInputLayout();
	initRenderTargetView();
	//
	auto &winSize = DirectEngine::getInstance()->getWinSize();
	_camera = Camera::createPerspective(60.0f, winSize.width / winSize.height, 1.5f, 200.0f);
	//
	D3DXVECTOR3    eyePosition(-4,2,-4);
	D3DXVECTOR3    targetPosition(0,0,0);

	_camera->lookAt(eyePosition, targetPosition);
	D3DXMatrixTranslation(&_modelMatrix, 0, 0, 1);
	//
	unsigned  count = 1;
	_device->RSGetViewports(&count, &_oldViewPort);
	int result = D3DX10CreateShaderResourceViewFromFile(_device, L"image/WoodCrate02.dds",nullptr,nullptr,&_texture,nullptr);
	assert(result>=0);
}

void RenderTarget::initBuffer()
{
	//顶点数据
	float    vertexData[] = {
		//-Z
		-1,-1,-1,   0,-1,0,  0,1,//0
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
	//
	D3D10_BUFFER_DESC	 bufferDesc;
	bufferDesc.ByteWidth = sizeof(vertexData);
	bufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA  resData;
	resData.pSysMem = vertexData;

	int result = _device->CreateBuffer(&bufferDesc, &resData, &_vertexBuffer);
	assert(result>=0);

	bufferDesc.ByteWidth = sizeof(indexData);
	bufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	resData.pSysMem = indexData;
	result = _device->CreateBuffer(&bufferDesc,&resData,&_indexBuffer);
	assert(result>=0);
	/////////////屏幕四边形/////////////////////////
	float quadData[] = {
		-1,-1,0,   0,1,//0
		-1,1,0,    0,0,//1
		1,-1,0,      1,1,//2
		1,1,0,        1,0,//3
	};
	bufferDesc.ByteWidth = sizeof(quadData);
	bufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	resData.pSysMem = quadData;
	result = _device->CreateBuffer(&bufferDesc,&resData,&_quadBuffer);
	assert(result>=0);
}

void RenderTarget::initEffect()
{
	unsigned shaderFlag = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlag |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob  *errorBlob = nullptr;
	int result = D3DX10CreateEffectFromFile(L"fx/RenderTarget.fx",nullptr,nullptr,"fx_4_0",shaderFlag,0,_device,nullptr,nullptr,&_effect,&errorBlob,nullptr);
	if (result < 0)
	{
		if (errorBlob)
		{
			MessageBoxA(nullptr,(char*)errorBlob->GetBufferPointer(),nullptr,0);
			errorBlob->Release();
		}
	}
	_effectTech = _effect->GetTechniqueByName("ColorTech");
	v_mvpMatrix = _effect->GetVariableByName("g_MVPMatrix")->AsMatrix();
	v_baseTexture = _effect->GetVariableByName("g_BaseTexture")->AsShaderResource();
}

void RenderTarget::initInputLayout()
{
	D3D10_INPUT_ELEMENT_DESC   inputDesc[2];
	//Position
	inputDesc[0].SemanticName = "POSITION";
	inputDesc[0].SemanticIndex = 0;
	inputDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputDesc[0].AlignedByteOffset = 0;
	inputDesc[0].InputSlot = 0;
	inputDesc[0].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	inputDesc[0].InstanceDataStepRate = 0;

	inputDesc[1].SemanticName = "TEXCOORD";
	inputDesc[1].SemanticIndex = 0;
	inputDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputDesc[1].AlignedByteOffset = sizeof(float)*6;
	inputDesc[1].InputSlot = 0;
	inputDesc[1].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	inputDesc[1].InstanceDataStepRate = 0;

	D3D10_PASS_DESC  passDesc;
	_effectTech->GetPassByIndex(0)->GetDesc(&passDesc);
	int result = _device->CreateInputLayout(inputDesc, 2,passDesc.pIAInputSignature,passDesc.IAInputSignatureSize,&_inputLayout);
	assert(result>=0);

	inputDesc[0].SemanticName = "POSITION";
	inputDesc[0].SemanticIndex = 0;
	inputDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputDesc[0].AlignedByteOffset = 0;
	inputDesc[0].InputSlot = 0;
	inputDesc[0].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	inputDesc[0].InstanceDataStepRate = 0;

	inputDesc[1].SemanticName = "TEXCOORD";
	inputDesc[1].SemanticIndex = 0;
	inputDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputDesc[1].AlignedByteOffset = sizeof(float)*3;
	inputDesc[1].InputSlot = 0;
	inputDesc[1].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	inputDesc[1].InstanceDataStepRate = 0;
	result = _device->CreateInputLayout(inputDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &_quadInputLayout);
	assert(result>=0);
}

void RenderTarget::initRenderTargetView()
{
	//颜色
	ID3D10Texture2D      *colorTexture = nullptr;
	D3D10_TEXTURE2D_DESC   textureDesc;
	textureDesc.Width = _viewPort.Width;
	textureDesc.Height = _viewPort.Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D10_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	int result = _device->CreateTexture2D(&textureDesc, nullptr, &colorTexture);
	assert(result>=0);
	//Resource View
	//使用2D纹理,并且使用的纹理切片索引为0
	D3D10_RENDER_TARGET_VIEW_DESC    renderTargetViewDesc;
	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetViewDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	result = _device->CreateRenderTargetView(colorTexture, &renderTargetViewDesc, &_renderView);
	assert(result>=0);
	D3D10_SHADER_RESOURCE_VIEW_DESC   shaderViewDesc;
	shaderViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	shaderViewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	shaderViewDesc.Texture2D.MostDetailedMip = 0;
	shaderViewDesc.Texture2D.MipLevels = 1;
	result = _device->CreateShaderResourceView(colorTexture, &shaderViewDesc, &_colorShaderView);
	assert(result>=0);
	colorTexture->Release();
	/////深度缓冲区对象///////////////
	ID3D10Texture2D                               *depthTexture=nullptr;
	D3D10_TEXTURE2D_DESC  depthTextrureDesc;
	depthTextrureDesc.Width = _viewPort.Width;
	depthTextrureDesc.Height = _viewPort.Height;
	depthTextrureDesc.MipLevels = 1;
	depthTextrureDesc.ArraySize = 1;
	depthTextrureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthTextrureDesc.SampleDesc.Count = 1;
	depthTextrureDesc.SampleDesc.Quality = 0;
	depthTextrureDesc.Usage = D3D10_USAGE_DEFAULT;
	depthTextrureDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE;
	depthTextrureDesc.CPUAccessFlags = 0;
	depthTextrureDesc.MiscFlags = 0;
	//
	result = _device->CreateTexture2D(&depthTextrureDesc, nullptr, &depthTexture);
	assert(result>=0);

	D3D10_DEPTH_STENCIL_VIEW_DESC  depthStencilViewDesc;
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	result = _device->CreateDepthStencilView(depthTexture, &depthStencilViewDesc, &_depthView);
	assert(result>=0);
	//
	D3D10_SHADER_RESOURCE_VIEW_DESC  depthViewDesc;
	depthViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthViewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	depthViewDesc.Texture2D.MipLevels = 1;
	depthViewDesc.Texture2D.MostDetailedMip = 0;
	result = _device->CreateShaderResourceView(depthTexture, &depthViewDesc, &_depthShaderView);
	assert(result>=0);
	//
	depthTexture->Release();
}

void RenderTarget::update(float dt)
{

}

void RenderTarget::render()
{
	ID3D10ShaderResourceView  *nullViews[2] = {nullptr,nullptr};
	unsigned stride = sizeof(float)*8;
	unsigned offset = 0;
	_device->RSSetViewports(1, &_viewPort);
	_device->PSSetShaderResources(0, 1, nullViews);
	_device->OMSetRenderTargets(1, &_renderView, _depthView);
	float color[4] = {0,0,0,0};
	_device->ClearRenderTargetView(_renderView, color);
	_device->ClearDepthStencilView(_depthView, D3D10_CLEAR_DEPTH, 1.0f, 0);
	_device->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	_device->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	_device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_device->IASetInputLayout(_inputLayout);
	//
	v_baseTexture->SetResource(_texture);
	D3DXMATRIX  mvpMatrix = _modelMatrix * _camera->getViewProjMatrix();
	v_mvpMatrix->SetMatrix((float*)mvpMatrix.m);
	//
	_effectTech->GetPassByIndex(0)->Apply(0);
	_device->DrawIndexed(_indexCount, 0, 0);
	//////////////////////恢复视口变换////////////////////////
	_device->RSSetViewports(1, &_oldViewPort);
	//
	stride = sizeof(float)*5;
	offset = 0;
	D3DXMATRIX  identity;
	D3DXMatrixIdentity(&identity);
	//恢复原来的渲染目标/////////////////////////////
	_device->IASetInputLayout(_quadInputLayout);
	_device->PSSetShaderResources(0, 1, nullViews);
	DirectEngine::getInstance()->restorRenderTarget();
	_device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	_device->IASetVertexBuffers(0, 1, &_quadBuffer, &stride,&offset);
	v_mvpMatrix->SetMatrix((float*)identity.m);
	v_baseTexture->SetResource(_colorShaderView);
	_effectTech->GetPassByIndex(0)->Apply(0);
	_device->Draw(4, 0);
}