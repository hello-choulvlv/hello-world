/*
  *海平面仿真的入口
  *2018年3月31日
  *@author:xiaohuaxiong
 */
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include<d3d11.h>
#include<d3dx11.h>
#if defined(DEBUG) || defined(_DEBUG)
#include <crtdbg.h>
#endif
#include<assert.h>
#include<stdio.h>
#include "Camera.h"
//////////////////////////////////////////////////////////////////////////
//#define USE_OTHER_SIMULATE
#include "Skybox.h"
#ifndef USE_OTHER_SIMULATE
#include "OceanSimulate.h"
#else
#include "other/ocean_simulator.h"
#endif
#include "render.h"
//////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK  windowMessageProcFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
//初始化DirectX11引擎
void           initializeDirectX11(HWND windHandler);
//初始化App
void           initApp();
//销毁对象
void          destroyDirectX11();
//每帧渲染前调用,应用程序不应尝试在此函数,或者此函数的调用函数中进行渲染
void          onUpdate();
//渲染
void          onRender();
////////////////////////////////////////////////////
const  int    s_window_width = 1280;
const   int    s_window_height = 720;
static  LARGE_INTEGER   s_hardwareFrequency;
static LARGE_INTEGER    s_baseHardwareCounter;
static double							  s_lastDelayTime;
static double                           s_lastFrameRecordTime;//记录上次帧数显示的时间
static long                                s_framePass = 0;
static unsigned                       s_keyMask = 0;
D3DXMATRIX                      s_reflect_matrix;
//DirectX
ID3D11Device							*s_device = nullptr;
ID3D11DeviceContext			*s_deviceContext = nullptr;
IDXGISwapChain                   *s_swapChain=nullptr;
ID3D11RenderTargetView    *s_renderTargetView = nullptr;
ID3D11DepthStencilView      *s_depthStencilView = nullptr;
ID3D11DepthStencilState      *s_depthStencilState=nullptr;
ID3D11RasterizerState           *s_rasterizerState = nullptr;
///////////////////////////////////////////////////////
HWND                                        s_windHandler = nullptr;
Skybox                                        *s_skybox = nullptr;
#ifndef USE_OTHER_SIMULATE
OceanSimulate                         *s_oceanSimulate = nullptr;
#else
ocean_simulator                      *s_oceanSimulate = nullptr;
#endif
/////////////////////////Camera///////////////////////
Camera                                       s_Camera;
///////////////////////////////////////////////////

int WINAPI   WinMain( HINSTANCE hInstance,  HINSTANCE hPrevInstance,  LPSTR lpCmdLine,  int nShowCmd)
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	WNDCLASS  windowClass;
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = windowMessageProcFunc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = L"DirectX_3D_class_engine";
	//
	if (!RegisterClass(&windowClass))
	{
		MessageBox(nullptr, L"RegisterClass Failed.", nullptr, 0);
		PostQuitMessage(0);
	}
	RECT   windowFrame = { 0,0,s_window_width,s_window_height };
	AdjustWindowRect(&windowFrame, WS_OVERLAPPEDWINDOW, 0);
	//Create Window
	s_windHandler = CreateWindow(L"DirectX_3D_class_engine", L"Learn-DirectX",
		WS_OVERLAPPEDWINDOW &~(WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT,
		s_window_width, s_window_height, 0, 0, hInstance, nullptr);
	if (!s_windHandler)
	{
		MessageBox(nullptr, L"CreateWindow Failed.", 0, 0);
		PostQuitMessage(0);
	}
	//
	ShowWindow(s_windHandler, SW_NORMAL);
	UpdateWindow(s_windHandler);
	//Create Swap Chain
	initializeDirectX11(s_windHandler);
	initApp();
	//
	MSG  windowMessage = { 0 };
	while (windowMessage.message != WM_QUIT)
	{
		if (PeekMessage(&windowMessage, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&windowMessage);
			DispatchMessage(&windowMessage);
		}
		else
		{
			onUpdate();
			//
			onRender();
		}
	}
	destroyDirectX11();
	destroyRenderResource();
	return windowMessage.lParam;
}
/*
  *消息处理
 */
static   int    s_buttonType;
static   D3DXVECTOR2    s_touchPoint;
static   D3DXVECTOR2    s_nowPoint;
LRESULT CALLBACK  windowMessageProcFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	float x, y;
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		s_touchPoint.x = LOWORD(lParam);
		s_touchPoint.y = s_window_height - HIWORD(lParam);
		s_buttonType = WM_LBUTTONDOWN;
		break;
	case WM_MOUSEMOVE:
		if (s_buttonType == WM_LBUTTONDOWN)
		{
			x = LOWORD(lParam);
			y = s_window_height - HIWORD(lParam);
			float  deltaX = x - s_touchPoint.x;
			float   deltaY = y - s_touchPoint.y;
			s_Camera.rotate(deltaY*0.3f, -deltaX*0.3f);
			s_touchPoint.x = x;
			s_touchPoint.y = y;
		}
		else
			s_buttonType = 0;
		break;
	case WM_LBUTTONUP:
		s_buttonType = 0;
		break;
	case WM_KEYDOWN:
		if (wParam == 'W' || wParam == 'w')
			s_keyMask |= KEY_MASK_W;
		else if (wParam == 'S' || wParam == 's')
			s_keyMask |= KEY_MASK_S;
		else if (wParam == 'A' || wParam == 'a')
			s_keyMask |= KEY_MASK_A;
		else if (wParam == 'D' || wParam == 'd')
			s_keyMask |= KEY_MASK_D;
		break;
	case WM_KEYUP:
		if (wParam == 'W' || wParam == 'w')
			s_keyMask &= ~KEY_MASK_W;
		else if (wParam == 'S' || wParam == 's')
			s_keyMask &= ~KEY_MASK_S;
		else if (wParam == 'A' || wParam == 'a')
			s_keyMask &= ~KEY_MASK_A;
		else if (wParam == 'D' || wParam == 'd')
			s_keyMask &= ~KEY_MASK_D;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
	}
	 return DefWindowProc(hwnd, msg, wParam, lParam);
}

void initializeDirectX11(HWND   windHandler)
{
	//Create Device and Device Context
	DXGI_SWAP_CHAIN_DESC    swapDesc;
	memset(&swapDesc,0,sizeof(DXGI_SWAP_CHAIN_DESC));
	//
	swapDesc.BufferDesc.Width = s_window_width;
	swapDesc.BufferDesc.Height = s_window_height;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.BufferCount = 2;
	swapDesc.OutputWindow = windHandler;
	swapDesc.Windowed = true;

	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Flags = 0;
	//创建交换链
	D3D_FEATURE_LEVEL    featureLevel = D3D_FEATURE_LEVEL_11_0;
	int result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,&featureLevel,1,
		D3D11_SDK_VERSION,&swapDesc,&s_swapChain,&s_device,nullptr,&s_deviceContext);
	assert(result>=0);
	//获取后台缓冲区对象
	ID3D11Texture2D    *backTexture = nullptr;
	result = s_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),(void**)&backTexture);
	assert(result>=0);
	//创建渲染目标
	result = s_device->CreateRenderTargetView(backTexture, nullptr, &s_renderTargetView);
	assert(result>=0);
	backTexture->Release();
	//创建深度模板缓存
	D3D11_TEXTURE2D_DESC   depthStencilTexDesc;
	depthStencilTexDesc.Width = s_window_width;
	depthStencilTexDesc.Height = s_window_height;
	depthStencilTexDesc.MipLevels = 1;
	depthStencilTexDesc.ArraySize = 1;
	depthStencilTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilTexDesc.SampleDesc.Count = 1;
	depthStencilTexDesc.SampleDesc.Quality = 0;
	depthStencilTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilTexDesc.CPUAccessFlags = 0;
	depthStencilTexDesc.MiscFlags = 0;

	ID3D11Texture2D *depthStencilTexture = nullptr;
	result = s_device->CreateTexture2D(&depthStencilTexDesc, nullptr, &depthStencilTexture);
	assert(result>=0);
	
	D3D11_DEPTH_STENCIL_DESC  depthStencilDesc;
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;

	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	result = s_device->CreateDepthStencilState(&depthStencilDesc, &s_depthStencilState);
	assert(result>=0);
	//设置输出混溶阶段的深度模板状态
	s_deviceContext->OMSetDepthStencilState(s_depthStencilState, 0x1);
	//深度模板视图
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	result = s_device->CreateDepthStencilView(depthStencilTexture, &depthStencilViewDesc, &s_depthStencilView);
	assert(result>=0);
	depthStencilTexture->Release();
	//设置渲染目标
	s_deviceContext->OMSetRenderTargets(1, &s_renderTargetView, s_depthStencilView);
	//设置光栅化对象
	D3D11_RASTERIZER_DESC  rasterDesc;
	memset(&rasterDesc,0,sizeof(rasterDesc));
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0;
	rasterDesc.SlopeScaledDepthBias = 0;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.AntialiasedLineEnable = false;
	result = s_device->CreateRasterizerState(&rasterDesc, &s_rasterizerState);
	assert(result>=0);
	s_deviceContext->RSSetState(s_rasterizerState);
	//设置视口
	D3D11_VIEWPORT  viewPort;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = s_window_width;
	viewPort.Height = s_window_height;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;
	s_deviceContext->RSSetViewports(1, &viewPort);
}

void   destroyDirectX11()
{
	delete s_skybox;
	delete s_oceanSimulate;
	//
	s_renderTargetView->Release();
	s_depthStencilView->Release();
	s_depthStencilState->Release();
	s_rasterizerState->Release();
	s_swapChain->Release();
	s_deviceContext->Release();
	s_device->Release();
}

void    initApp()
{
	//设置硬件的计时频率
	QueryPerformanceFrequency(&s_hardwareFrequency);
	QueryPerformanceCounter(&s_baseHardwareCounter);
	s_lastDelayTime = 0;
	///////////////matrix////////////////////////
	s_reflect_matrix = D3DXMATRIX(1, 0, 0, 0,0, 0, 1, 0,0, 1, 0, 0,0, 0, 0, 1);
	s_Camera.initPerspective(45.0f, 1.0f*s_window_width / s_window_height, 200.0f, 400000);
	//D3DXVECTOR3 vecEye(0.24f, 854.291f, -0.99f);
	//D3DXVECTOR3 vecAt(0.91f, 854.113f, -1.71f);
	D3DXVECTOR3 vecEye(1562.24f, 854.291f, -1224.99f);
	D3DXVECTOR3 vecAt(1562.91f, 854.113f, -1225.71f);
	s_Camera.lookAt(vecEye, vecAt);
#ifndef USE_OTHER_SIMULATE
	OceanParam   oceanParam;
	s_oceanSimulate = new OceanSimulate(oceanParam,s_device,s_deviceContext);
#else
	ocean_parameter   param;
	s_oceanSimulate = new ocean_simulator(param,s_device);
#endif
	s_skybox = Skybox::create(s_device,s_deviceContext);
	/////////////init render resource//////
	initRenderResource();
}

void    onUpdate()
{
	LARGE_INTEGER   nowHardwareCounter;
	QueryPerformanceCounter(&nowHardwareCounter);
	double   nowTime = 1.0 * (nowHardwareCounter.QuadPart - s_baseHardwareCounter.QuadPart)/s_hardwareFrequency.QuadPart;
	if (nowTime < 0)
		nowTime = 0;
	s_framePass += 1;
	double   deltaTime = nowTime - s_lastDelayTime;
	if (nowTime >= s_lastFrameRecordTime+1)
	{
		float  frame_rate = s_framePass / (nowTime- s_lastFrameRecordTime);
		s_framePass = 0;
		s_lastFrameRecordTime = nowTime;
		char buffer[128];
		sprintf(buffer,"Frame rate: %.1f",frame_rate);
		SetWindowTextA(s_windHandler,buffer);
	}
	//检测是否需要修改摄像机
	if (s_keyMask)
	{
		float speed = 200.0f;
		float dx=0, dz=0;
		if (s_keyMask & KEY_MASK_W)
			dz += speed * deltaTime;
		else if (s_keyMask & KEY_MASK_S)
			dz -= speed * deltaTime;
		else if (s_keyMask & KEY_MASK_A)
			dx -= speed * deltaTime;
		else if (s_keyMask & KEY_MASK_D)
			dx += speed * deltaTime;
		s_Camera.translate(dx, dz);
	}
	/////////////////////////////////////////////////////////////////////////////
	s_skybox->onUpdate(deltaTime,s_Camera);
	//////////////////////////////////////////////////////////////////////////
#ifndef USE_OTHER_SIMULATE
	s_oceanSimulate->update_displacement_map(deltaTime, nowTime);
#else
	s_oceanSimulate->update_displacement_map(nowTime);
#endif
	s_lastDelayTime = nowTime;
}

void onRender()
{
	float color[4] = {0,0,0,0};
	s_deviceContext->ClearRenderTargetView(s_renderTargetView, color);
	s_deviceContext->ClearDepthStencilView(s_depthStencilView, D3D10_CLEAR_DEPTH + D3D10_CLEAR_STENCIL, 1.0f, 0);
	s_skybox->onRender(s_Camera);
#ifndef USE_OTHER_SIMULATE
	renderOceanMeshShader(s_Camera, s_oceanSimulate->get_displacement_shader_view(),s_oceanSimulate->get_gradient_shader_view(), s_lastDelayTime);
#else
	renderOceanMeshShader(s_Camera, s_oceanSimulate->get_direct3d_displacement_map(), s_oceanSimulate->get_direct3d_gradient_map(),s_lastDelayTime);
#endif
	s_swapChain->Present(0, 0);
}