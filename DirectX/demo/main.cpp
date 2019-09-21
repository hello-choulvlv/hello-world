/*
  *程序的入口
  *2018年2月28日
  *@Author:xiaohuaxiong
 */
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#if defined(DEBUG) || defined(_DEBUG)
#include <crtdbg.h>
#endif
#include<d3dx10.h>
#include<unordered_map>
#include "engine/DirectEngine.h"
#include "engine/EventManager.h"
#include "AppEntry.h"
//定时器的ID
#define _TIMER_UPDATE_CALL_BACK_ID_   0x7CFC8E97
#define NONE_BUTTON 0x0
#define LEFT_BUTTON  0x1
#define RIGHT_BUTTON  0x2
#define MIDDLE_BUTTON 0x3
void CALLBACK  onTimerCallFunc(HWND  handler, UINT  param1, UINT  param2, DWORD   param3);
//消息处理函数
LRESULT CALLBACK  windowMessageProcFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static const int window_width = 1334;
static const int window_height = 750;
//时间计时器
static int    static_base_counter = 0;
static int    static_last_counter=0;
//键盘字符转查找表
static std::unordered_map<int,KeyCode>    s_keyTranslatorMap;
//记录上次时那个鼠标按键按下了
int              static_lastMouseButton = -1;
//翻译
void   translateKey()
{
	//字符
	for (int i = 0; i < 26; ++i)
	{
		s_keyTranslatorMap['A' + i] = (KeyCode)(KeyCode_A + i);
		s_keyTranslatorMap['a' + i] = (KeyCode)(KeyCode_A + i);
	}
	//数字
	for (int i = 0; i < 10; ++i)
	{
		s_keyTranslatorMap['0' + i] = (KeyCode)(KeyCode_0 + i);
	}
	//
	s_keyTranslatorMap[VK_SPACE] = KeyCode_SPACE;
	s_keyTranslatorMap[VK_TAB] = KeyCode_TAB;
	s_keyTranslatorMap[VK_SHIFT] = KeyCode_SHIFT;
	s_keyTranslatorMap[VK_CONTROL] = KeyCode_CTRL;
	s_keyTranslatorMap[VK_LMENU] = KeyCode_ALT;
	s_keyTranslatorMap[VK_RMENU]= KeyCode_ALT;
	//
	s_keyTranslatorMap[VK_LEFT] = KeyCode_LEFT;
	s_keyTranslatorMap[VK_UP] = KeyCode_UP;
	s_keyTranslatorMap[VK_RIGHT] = KeyCode_RIGHT;
	s_keyTranslatorMap[VK_DOWN] = KeyCode_DOWN;
	//
	for(int i=0;i<12;++i)
		s_keyTranslatorMap[VK_F1+i] = (KeyCode)(KeyCode_F1+i);
}

int WINAPI WinMain( HINSTANCE hInstance,  HINSTANCE hPrevInstance,  PSTR lpCmdLine,  int nShowCmd)
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
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
		MessageBox(nullptr, L"RegisterClass Failed.",nullptr,0);
		PostQuitMessage(0);
	}
	RECT   windowFrame = {0,0,window_width,window_height};
	AdjustWindowRect(&windowFrame,WS_OVERLAPPEDWINDOW,0 );
	//Create Window
	HWND   windowHandler = CreateWindow(L"DirectX_3D_class_engine",L"Learn-DirectX",
									WS_OVERLAPPEDWINDOW &~(WS_THICKFRAME| WS_MAXIMIZEBOX| WS_MINIMIZEBOX),CW_USEDEFAULT,CW_USEDEFAULT,
									window_width,window_height,0,0, hInstance,nullptr);
	if (!windowHandler)
	{
		MessageBox(nullptr,L"CreateWindow Failed.",0,0);
		PostQuitMessage(0);
	}
	//
	ShowWindow(windowHandler, SW_NORMAL);
	UpdateWindow(windowHandler);
	//翻译按键
	translateKey();
	//初始化DirectEngine对象
	DirectEngine  *engine = DirectEngine::getInstance();
	engine->init(hInstance, windowHandler, window_width, window_height);
	//初始化应用程序状态
	AppEntry appEntry;
	appEntry.onCreateComplete();
	//设置计时器
	static_base_counter = GetTickCount();
	static_last_counter = 0;
	//设置回掉函数
	SetTimer(windowHandler, _TIMER_UPDATE_CALL_BACK_ID_,0, &onTimerCallFunc);
	//
	MSG  windowMessage = {0};
	while (windowMessage.message != WM_QUIT)
	{
		if (PeekMessage(&windowMessage, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&windowMessage);
			DispatchMessage(&windowMessage);
		}
		//engine->render();
	}
	appEntry.onDestroy();
	//释放内存
	delete engine;
	engine = nullptr;
	return windowMessage.lParam;
}
LRESULT CALLBACK  windowMessageProcFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	std::unordered_map<int, KeyCode>::iterator it;
	int x, y;
	switch(msg)
	{
	case WM_KEYDOWN:
		 it = s_keyTranslatorMap.find(wParam);
		 if (it != s_keyTranslatorMap.end())
			 EventManager::getInstance()->dispatchKeyEvent(KeyState_Pressed, it->second);
		break;
	case WM_KEYUP:
		it = s_keyTranslatorMap.find(wParam);
		if (it != s_keyTranslatorMap.end())
			EventManager::getInstance()->dispatchKeyEvent(KeyState_Released, it->second);
		break;
	case WM_LBUTTONDOWN:
		x = LOWORD(lParam);
		y = window_height -  HIWORD(lParam); 
		EventManager::getInstance()->dispatchTouchEvent(TouchState_Pressed, Vec2(x,y));
		static_lastMouseButton = LEFT_BUTTON;
		break;
	case WM_MOUSEMOVE:
		if (static_lastMouseButton == LEFT_BUTTON)
		{
			x = LOWORD(lParam);
			y = window_height - HIWORD(lParam);
			EventManager::getInstance()->dispatchTouchEvent(TouchState_Moved, Vec2(x, y));
		}
		break;
	case WM_LBUTTONUP:
		x = LOWORD(lParam);
		y = window_height - HIWORD(lParam);
		EventManager::getInstance()->dispatchTouchEvent(TouchState_Released, Vec2(x,y));
		static_lastMouseButton = NONE_BUTTON;
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		static_lastMouseButton = RIGHT_BUTTON;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
//HWND, UINT, UINT_PTR, DWORD
void CALLBACK  onTimerCallFunc(HWND  handler,UINT  param1,UINT  param2,DWORD   param3)
{
	DirectEngine *engine = DirectEngine::getInstance();
	engine->clear();
	//
	int  now_tick_count = GetTickCount() - static_base_counter;
	engine->update((now_tick_count - static_last_counter)/1000.0f);
	//
	engine->render();
	static_last_counter = now_tick_count;
	//
	engine->swapBuffers();
	SetTimer(engine->getWindowHandler(), _TIMER_UPDATE_CALL_BACK_ID_, 5, onTimerCallFunc);
}