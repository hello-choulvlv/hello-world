/*
  *光线跟踪实现
  *2018年4月26日
  *@author:xiaohuaxiong
 */
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#if defined(DEBUG) || defined(_DEBUG)
#include <crtdbg.h>
#endif
#include<stdio.h>
#include<math.h>
#include "engine/Geometry.h"
#include "engine/Camera2.h"
#include "RayTrace.h"
#include "Raster.h"
 //Key Mask
#define KEY_MASK_W 0x1
#define KEY_MASK_S   0x2
#define KEY_MASK_A  0x4
#define KEY_MASK_D  0x8
///////global variable////////////////
const    int   s_window_width = 800;
const    int   s_window_height = 600;
HWND        s_windowHandler = nullptr;
static  LARGE_INTEGER   s_hardwareFrequency;
static LARGE_INTEGER    s_baseHardwareCounter;
static double							  s_lastDelayTime;
static double                           s_lastFrameRecordTime;//记录上次帧数显示的时间
static long                                s_framePass = 0;
static  Camera2                     *s_Camera;
static  Camera                        g_Camera;
static  Raster                          *s_Raster;
static  RayTracer                   *s_RayTracer;
LRESULT CALLBACK  windowMessageProcFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void            onRender();
//初始化应用程序
void    initApp()
{
	QueryPerformanceFrequency(&s_hardwareFrequency);
	QueryPerformanceCounter(&s_baseHardwareCounter);
	s_lastDelayTime = 0;
	//摄像机
	float_3    eyePosition(0,0,10.f);
	float_3    targetPosition(0,0,0);
	float_3    upperVec(0,1,0);
	s_Camera = Camera2::createWithPerspective(45, 1.0f*s_window_width / s_window_height,1.0f,40.0f);// Camera::createCamera(eyePosition, targetPosition, upperVec);
	s_Camera->lookAt(eyePosition, targetPosition);

	g_Camera.Look(vec3(0,0,8.75),vec3(0,0,0),true);
	g_Camera.VPin[0] = 1.0f / (float)s_window_width;
	g_Camera.VPin[5] = 1.0f / (float)s_window_height;

	float tany = tan(45.0f / 360.0f * (float)M_PI), aspect = (float)s_window_width / (float)s_window_height;

	g_Camera.Pin[0] = tany * aspect;
	g_Camera.Pin[5] = tany;
	g_Camera.Pin[10] = 0.0f;
	g_Camera.Pin[14] = -1.0f;

	g_Camera.CalculateRayMatrix();
	//光栅化对象
	s_Raster = Raster::create(s_window_width, s_window_height);
	s_RayTracer = RayTracer::create(s_Camera,&g_Camera);
	//计算RayMatrix
	s_Camera->updateViewportMatrix(s_window_width, s_window_height);
	s_Camera->updateRayProjMatrix(45.0f,1.0f*s_window_width / s_window_height);
	s_Camera->updateRayMatrix();
}

void     destroyApp()
{
	delete s_Camera;
	delete s_Raster;
	s_RayTracer->release();
}
void   showWindow(HWND   handler)
{
	RECT dRect, wRect, cRect;

	GetWindowRect(GetDesktopWindow(), &dRect);
	GetWindowRect(handler, &wRect);
	GetClientRect(handler, &cRect);

	wRect.right += s_window_width - cRect.right;
	wRect.bottom += s_window_height - cRect.bottom;

	wRect.right -= wRect.left;
	wRect.bottom -= wRect.top;

	wRect.left = dRect.right / 2 - wRect.right / 2;
	wRect.top = dRect.bottom / 2 - wRect.bottom / 2;

	MoveWindow(handler, wRect.left, wRect.top, wRect.right, wRect.bottom, FALSE);

	ShowWindow(handler, SW_SHOWNORMAL);
}
//
int WINAPI   WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
//#if defined(DEBUG) || defined(_DEBUG)
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	WNDCLASSEX windowClass;
	memset(&windowClass,0,sizeof(windowClass));
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = windowMessageProcFunc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);
	windowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = "RayTracer";
	//
	if (!RegisterClassEx(&windowClass))
	{
		MessageBox(nullptr, "RegisterClass Failed.", nullptr, 0);
		PostQuitMessage(0);
	}
	RECT   windowFrame = { 0,0,s_window_width,s_window_height };
	AdjustWindowRect(&windowFrame, WS_OVERLAPPEDWINDOW, 0);
	//Create Window
	DWORD Style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	s_windowHandler = CreateWindowEx(WS_EX_APPWINDOW, "RayTracer", "RayTracer", Style, 0, 0,
			s_window_width, s_window_height, NULL, NULL, hInstance, NULL);
	if (!s_windowHandler)
	{
		MessageBox(nullptr, "CreateWindow Failed.", 0, 0);
		PostQuitMessage(0);
	}
	initApp();
	showWindow(s_windowHandler);
	UpdateWindow(s_windowHandler);
	//
	MSG Msg = { 0 };
	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	destroyApp();
	return Msg.lParam;
}

/*
*消息处理
*/
static   int		 s_buttonType;
static   float_2   s_touchPoint;
static   float_2   s_nowPoint;
static   int		s_keyMask;
static   int		s_index_x = 0;
static   int		s_index_y = 0;
LRESULT CALLBACK  windowMessageProcFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	float x, y;
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		s_touchPoint.x = LOWORD(lParam);
		s_touchPoint.y = s_window_height - HIWORD(lParam);
		s_buttonType = WM_LBUTTONDOWN;
		//InvalidateRect(s_windowHandler, NULL, FALSE);
		break;
	case WM_MOUSEMOVE:
		if (s_buttonType == WM_LBUTTONDOWN)
		{
			x = LOWORD(lParam);
			y = s_window_height - HIWORD(lParam);
			float  deltaX = x - s_touchPoint.x;
			float   deltaY = y - s_touchPoint.y;
			//s_Camera->rotate2(-deltaX, deltaY);
			g_Camera.OnMouseMove(-deltaX,deltaY);
			s_touchPoint.x = x;
			s_touchPoint.y = y;

			s_index_x = 0;
			s_index_y = 0;
			InvalidateRect(s_windowHandler, NULL, FALSE);
		}
		else
			s_buttonType = 0;
		break;
	case WM_LBUTTONUP:
		s_buttonType = 0;
		//InvalidateRect(s_windowHandler, NULL, FALSE);
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
		//InvalidateRect(s_windowHandler, NULL, FALSE);
		break;
	case WM_PAINT:
		onRender();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}
//光栅化函数
void    onRender()
{
	//计算帧率
	LARGE_INTEGER    now_counter;
	QueryPerformanceCounter(&now_counter);
	double    delayTime = 1.0 *(now_counter.QuadPart - s_baseHardwareCounter.QuadPart)/s_hardwareFrequency.QuadPart;
	++s_framePass;
	if (delayTime > s_lastDelayTime + 1.0)
	{
		double frame_rate = s_framePass / (delayTime - s_lastDelayTime);
		char buffer[128];
		sprintf(buffer,"%1lf",frame_rate);
		SetWindowText(s_windowHandler,buffer);
		s_framePass = 0;
		s_lastDelayTime = delayTime;
	}
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(s_windowHandler, &ps);
	//对每一个像素渲染
	unsigned char   *color_buffer = s_Raster->_colorBuffer;
	if (!s_index_x && !s_index_y)
		memset(color_buffer, 0, s_window_height*s_window_width * 3);
	//memset(color_buffer,0,s_window_width*s_window_height*3);
	float_3   color;
	float    ndcy = 2.0f *(s_index_y + 0.5f) / s_window_height - 1.0f;
	for (int j = 0; j < s_window_width; ++j)
	{
		//convert ndc
		s_RayTracer->rayTrace(j, s_index_y,color);
		//写入到像素缓冲区对象中
		int  idx = (s_index_y * s_window_width + j) *3;
		color_buffer[idx +2] = min(max(color.x,0.0f),1.0f) * 255;
		color_buffer[idx + 1] = min(max(color.y,0.0f),1.0f) * 255;
		color_buffer[idx] = min(max(color.z,0.0f),1.0f) * 255;
	}
	if (s_index_y < s_window_height - 1)
		++s_index_y;
	s_Raster->swapBuffers(hdc);
	InvalidateRect(s_windowHandler, NULL, FALSE);
	EndPaint(s_windowHandler, &ps);
}