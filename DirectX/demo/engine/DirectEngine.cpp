/*
  *DirectX执行引擎
  *@Author:xiaohuaxiong
  *@2018年2月28日
 */
#include "DirectEngine.h"
static DirectEngine   *static_direct_engine_instance = nullptr;
DirectEngine::DirectEngine() :
	_appInstance(nullptr)
	, _windowHandler(nullptr)
	, _d3dDevice(nullptr)
	, _swapChain(nullptr)
	, _depthStencilTexture(nullptr)
	, _renderTargetView(nullptr)
	, _depthStencilView(nullptr)
	, _font(nullptr)
	, _clearColor(0.0f, 0.0f, 0.0f, 0.0f)
	,_driverType(D3D10_DRIVER_TYPE_HARDWARE)
{

}

DirectEngine::~DirectEngine()
{
	_font->Release();
	_depthStencilView->Release();
	_renderTargetView->Release();
	_depthStencilTexture->Release();
	_swapChain->Release();
	_d3dDevice->Release();
	static_direct_engine_instance = nullptr;
}

DirectEngine *DirectEngine::getInstance()
{
	if (!static_direct_engine_instance)
	{
		static_direct_engine_instance = new DirectEngine();
	}
	return static_direct_engine_instance;
}

void DirectEngine::init(HINSTANCE hInstance, HWND windowHandler, int windowWidth, int windowHeight)
{
	_windowWidth = windowWidth;
	_windowHeight = windowHeight;
	_winSize.width = windowWidth;
	_winSize.height = windowHeight;
	//
	_appInstance = hInstance;
	_windowHandler = windowHandler;
	DXGI_SWAP_CHAIN_DESC    swapChainDesc;
	swapChainDesc.BufferDesc.Width = windowWidth;
	swapChainDesc.BufferDesc.Height = windowHeight;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;

	swapChainDesc.OutputWindow = windowHandler;
	swapChainDesc.Windowed = true;

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	//
	UINT createFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createFlags = D3D10_CREATE_DEVICE_DEBUG;
#endif
	int result = D3D10CreateDeviceAndSwapChain(nullptr, 
		_driverType, 
		nullptr,//soft device
		createFlags,
		D3D10_SDK_VERSION,
		&swapChainDesc,
		&_swapChain,
		&_d3dDevice
		);
	if (result < 0)
	{
		DXTrace(__FILE__, (DWORD)__LINE__, result, L"DirectEngine::init", true);
		return;
	}
	//Create Render Target View
	_swapChain->ResizeBuffers(1, windowWidth, windowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	ID3D10Texture2D *backTexture = nullptr;
	_swapChain->GetBuffer(0,__uuidof(ID3D10Texture2D),reinterpret_cast<void**>(&backTexture));
	_d3dDevice->CreateRenderTargetView(backTexture, nullptr , &_renderTargetView);
	backTexture->Release();
	//Create Depth Stencil View
	D3D10_TEXTURE2D_DESC  depthStencilDesc;
	depthStencilDesc.Width = windowWidth;
	depthStencilDesc.Height = windowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D10_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilTexture);
	_d3dDevice->CreateDepthStencilView(_depthStencilTexture,nullptr,&_depthStencilView);
	//
	_d3dDevice->OMSetRenderTargets(1, &_renderTargetView, _depthStencilView);
	//Viewport
	D3D10_VIEWPORT  viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = windowWidth;
	viewport.Height = windowHeight;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;
	_d3dDevice->RSSetViewports(1, &viewport);
	//Create Font
	D3DX10_FONT_DESC fontDesc;
	fontDesc.Height = 24;
	fontDesc.Width = 0;
	fontDesc.Weight = 0;
	fontDesc.MipLevels = 1;
	fontDesc.Italic = false;
	fontDesc.CharSet = DEFAULT_CHARSET;
	fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
	fontDesc.Quality = DEFAULT_QUALITY;
	fontDesc.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	wcscpy(fontDesc.FaceName, L"Times New Roman");

	D3DX10CreateFontIndirect(_d3dDevice, &fontDesc, &_font);
}

void DirectEngine::swapBuffers()
{
	_swapChain->Present(0,0);
}

void DirectEngine::restorRenderTarget()
{
	_d3dDevice->OMSetRenderTargets(1, &_renderTargetView, _depthStencilView);
}

void DirectEngine::clear()
{
	_d3dDevice->ClearRenderTargetView(_renderTargetView, _clearColor);
	_d3dDevice->ClearDepthStencilView(_depthStencilView, D3D10_CLEAR_DEPTH|D3D10_CLEAR_STENCIL,1.0f,0);
}

void DirectEngine::update(float dt)
{
	for (auto it = _sceneVector.begin(); it != _sceneVector.end(); ++it)
		(*it)->update(dt);
}

void DirectEngine::render()
{
	for (auto it = _sceneVector.begin(); it != _sceneVector.end(); ++it)
		(*it)->render();
}

void DirectEngine::destroy()
{
	for (auto it = _sceneVector.begin(); it != _sceneVector.end(); ++it)
		(*it)->release();
	_sceneVector.clear();
}

void DirectEngine::addScene(Scene *scene)
{
	scene->retain();
	_sceneVector.push_back(scene);
}