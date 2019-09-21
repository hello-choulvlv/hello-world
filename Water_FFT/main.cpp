#include "res\resource.h"
#include "myd3d.h"
#include "UserControl.h"


HRESULT InitD3D(HINSTANCE hInstance);
//该函数返回自定义错误信息，0表示成功
UINT InitMyScene();
void ControlProc();
//因为该函数要调用InitMyScene，所以返回值跟它相同，若在显示模式切换时出错则可显示出错码
UINT DisplayMyScene();
void ReleaseD3D();


//窗口的位置和大小
UINT WindowX = 0, WindowY = 0;
UINT WindowWidth = GetSystemMetrics(SM_CXSCREEN), WindowHeight = GetSystemMetrics(SM_CYSCREEN);

BOOL CALLBACK GeneralDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		EndDialog(hDlg, 0);
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK ChangeDisplayModeDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hModeList = GetDlgItem(hDlg, IDC_MODELIST);
	HWND hWindowModeCheck = GetDlgItem(hDlg, IDC_WINDOWMODE);
	D3DDISPLAYMODE ModeInfo, *pModeInfo, ModeExist, *pModeExist;
	UINT BufferCount[2], AllBufferCount=0, ModeCount=0, ModeExistCount=0, i, j, k;
	UINT ColorBits, CurrentIndex=65535;
	BOOL SkipSign = FALSE;
	char pListInfo[100];
	// 要是新加入支持的色彩模式（必须是颜色位数不同），只需要加入该结构，然后将循环中的K改成3就行了
	D3DFORMAT SupportColorFormat[2]={D3DFMT_R5G6B5, D3DFMT_X8R8G8B8};

			//得到枚举信息，只支持400*300以上，X8R8G8B8和R5G6B5，还有D24S8和D16，过滤掉所有的刷新率
			for(k=0; k<2; k++)
			{
				BufferCount[k] = d3d->GetAdapterModeCount(D3DADAPTER_DEFAULT, SupportColorFormat[k]);
				AllBufferCount += BufferCount[k];
			}

			pModeInfo = new D3DDISPLAYMODE[AllBufferCount];
			pModeExist = new D3DDISPLAYMODE[AllBufferCount];
		for(k=0; k<2; k++)
			for(i=0; i<BufferCount[k]; i++)
			{
				if(FAILED(d3d->EnumAdapterModes(D3DADAPTER_DEFAULT, SupportColorFormat[k], i, pModeInfo + ModeCount)))
				{
					EndDialog(hDlg, 0);
					return TRUE;
				}

				ModeInfo = *(pModeInfo + ModeCount);

				//得到当前显示模式在枚举（忽略刷新率）中的序号
				if(ModeInfo.Width==d3ddm.Width && ModeInfo.Height==d3ddm.Height && ModeInfo.Format==d3ddm.Format && CurrentIndex==65535)
					CurrentIndex = ModeCount;
		

				SkipSign = FALSE;
				//去掉400*300以下的模式，分辨率太低时使用d3dim和pure hal会出现问题
				if(ModeInfo.Width <400 || ModeInfo.Height < 300)
					SkipSign = TRUE;
				//加这个是为了去掉同一个显示模式，不同刷新率的选项
				for(j=0; j<ModeExistCount; j++)
				{
					ModeExist = *(pModeExist + j);
					if(ModeExist.Width == ModeInfo.Width && ModeExist.Height == ModeInfo.Height && ModeInfo.Format == ModeExist.Format)
					{
						SkipSign = TRUE;
						break;
					}
				}
				if(SkipSign == FALSE)
				{
					(*(pModeExist + ModeExistCount)).Width = ModeInfo.Width;
					(*(pModeExist + ModeExistCount)).Height = ModeInfo.Height;
					(*(pModeExist + ModeExistCount)).Format = ModeInfo.Format;
					ModeExistCount++;
				
					ModeCount++;				
				}
			}


	switch(msg)
	{
		case WM_INITDIALOG:
			//初始化列表
			for(i=0; i<ModeCount; i++)
			{
				ModeInfo = *(pModeInfo+i);
				//翻译颜色值
				if(ModeInfo.Format==D3DFMT_R5G6B5) ColorBits = 16;
				else if(ModeInfo.Format==D3DFMT_X8R8G8B8) ColorBits = 32;
				else ColorBits = 0;
				sprintf(pListInfo, "%d*%d, %dbits", ModeInfo.Width, ModeInfo.Height, ColorBits);
				SendMessage(hModeList, CB_ADDSTRING, 0, (LPARAM)pListInfo);
			}

			//将当前设置设为默认选项
			SendMessage(hModeList, CB_SETCURSEL, (WPARAM)CurrentIndex, 0);
			delete[] pModeInfo;
			delete[] pModeExist;
			return TRUE;

		case WM_COMMAND:
			if(LOWORD(wParam) == IDOK)
			{
				//处理check box，置窗口下的分辨率模式
				if(SendMessage(hWindowModeCheck, BM_GETCHECK, 0, 0) == TRUE)
				{
					WindowMode = TRUE;
					CurrentIndex = SendMessage(hModeList, CB_GETCURSEL, 0, 0);
					ModeInfo = *(pModeInfo + CurrentIndex);
					WindowWidth = (ModeInfo.Width>DesktopDisplayMode.Width)?DesktopDisplayMode.Width:ModeInfo.Width;
					WindowHeight = (ModeInfo.Height>DesktopDisplayMode.Height)?DesktopDisplayMode.Height:ModeInfo.Height;
					WindowX = WindowY = 0;
					d3ddm.Format = ModeInfo.Format;
				}
				else
				{
					WindowMode = FALSE;
					//处理combo box，窗口模式时，对于全屏模式的修改都无效
					CurrentIndex = SendMessage(hModeList, CB_GETCURSEL, 0, 0);
					ModeInfo = *(pModeInfo + CurrentIndex);
					d3ddm.Width = ModeInfo.Width;
					d3ddm.Height = ModeInfo.Height;
					d3ddm.Format = ModeInfo.Format;
				}
				EndDialog(hDlg, 1);
			}

			if(LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, 0);
			}
			
			delete[] pModeInfo;
			delete[] pModeExist;
			return TRUE;
	}

	delete[] pModeInfo;
	delete[] pModeExist;
	return FALSE;
}



LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bLastWindowMode = WindowMode;
	RECT WindowRect;

//为了保证帧数的稳定性，暂时去掉	Sleep(1);

	switch(msg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_PAINT:
			return 0;

		//改变窗口位置和大小就要重置
		case WM_SIZE:
			if(WindowMode == FALSE) return 0; //非窗口模式就不改变，防止切换到全屏时无法保持原来的窗口分辨率
			GetWindowRect(hWnd, &WindowRect);

			// 如果只是因为用户控制导致产生该消息，事实上模式和分辨率都没变，那么就不要处理（比如以窗口模式启动时会产生一次Size消息，或者点击边框想改变窗口大小其实又没改变，或者在窗口已经是最大的情况下点最大化按钮）
			if(WindowMode == bLastWindowMode && (WindowRect.right-WindowRect.left) == WindowWidth && (WindowRect.bottom-WindowRect.top) == WindowHeight)
				return 0;
			// 最小化的时候忽略
			if(WindowRect.right < 0 || WindowRect.left < 0 || WindowRect.bottom < 0 || WindowRect.top < 0)
				return 0;

			bLastWindowMode = WindowMode;

			WindowWidth = WindowRect.right - WindowRect.left;
			WindowHeight = WindowRect.bottom - WindowRect.top;
			if(WindowMode)
			{
				ChangeDisplayModeSign = TRUE; //动态改变窗口大小时，要动态Reset
				if(WindowWidth < 400)      //禁止分辨率小于400*300
					WindowWidth = 400;
				if(WindowHeight < 300)
					WindowHeight = 300;
			}
			return 0;
		case WM_MOVE:
			if(WindowMode == FALSE) return 0; //非窗口模式就不改变，防止切换到全屏时无法保持原来的窗口分辨率
			GetWindowRect(hWnd, &WindowRect);
			WindowX = WindowRect.left;
			WindowY = WindowRect.top;
			return 0;
		//拦截除了退出以外所有的键盘消息，改由USERCONTROL处理
		case WM_KEYDOWN:
			//if(wParam == VK_ESCAPE) PostMessage(hWnd,WM_CLOSE,0,0);
			return 0;
		case WM_KEYUP:
			return 0;
		case WM_SYSKEYDOWN:
			return 0;
		case WM_SYSKEYUP:
			return 0;
		case WM_CHAR:
			return 0;


			// 鼠标控制
		case WM_MOUSEMOVE:
			g_MouseControl.SetPosition(LOWORD(lParam), HIWORD(lParam));
			break;
		
		case WM_LBUTTONDOWN:
			g_MouseControl.SetKeyAttribute(WM_LBUTTONDOWN, wParam);
			break;
		case WM_RBUTTONDOWN:
			g_MouseControl.SetKeyAttribute(WM_RBUTTONDOWN, wParam);
			break;
		case WM_MBUTTONDOWN:
			g_MouseControl.SetKeyAttribute(WM_MBUTTONDOWN, wParam);
			break;

		case WM_LBUTTONUP:
			g_MouseControl.SetKeyAttribute(WM_LBUTTONUP, wParam);
			break;
		case WM_RBUTTONUP:
			g_MouseControl.SetKeyAttribute(WM_RBUTTONUP, wParam);
			break;
		case WM_MBUTTONUP:
			g_MouseControl.SetKeyAttribute(WM_MBUTTONUP, wParam);
			break;

		case WM_LBUTTONDBLCLK:
			g_MouseControl.SetKeyAttribute(WM_LBUTTONDBLCLK, wParam);
			break;
		case WM_RBUTTONDBLCLK:
			g_MouseControl.SetKeyAttribute(WM_RBUTTONDBLCLK, wParam);
			break;
		case WM_MBUTTONDBLCLK:
			g_MouseControl.SetKeyAttribute(WM_MBUTTONDBLCLK, wParam);
			break;
		case 0x020A:	// WM_MOUSEWHEEL
			g_MouseControl.SetMidAxis(HIWORD(wParam));
			break;
	}

 
	return DefWindowProc(hWnd, msg, wParam, lParam);
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	WNDCLASSEX wc={sizeof(WNDCLASSEX), CS_CLASSDC | CS_DBLCLKS, MsgProc, 0L, 0L, GetModuleHandle(NULL), LoadIcon(hInstance, (LPCSTR)IDI_3DFX), LoadCursor(NULL, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH), NULL, "My D3D Application", NULL};
	RegisterClassEx(&wc);
	hWnd=CreateWindow("My D3D Application", "My Effect", WS_OVERLAPPEDWINDOW, WindowX, WindowY, WindowWidth, WindowHeight, GetDesktopWindow(), NULL, wc.hInstance, NULL);
	//SetWindowLong( hWnd, GWL_STYLE, WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX);

	char pInfo[500];
	UINT InfoID = 0;

	if(SUCCEEDED(InitD3D(hInstance)))
	{
		InfoID = InitMyScene();

		ShowWindow(hWnd,SW_SHOWDEFAULT);
		UpdateWindow(hWnd);

		MSG msg;
		while(GetMessage(&msg,NULL,0,0))
		{
			TranslateMessage(&msg);	
			DispatchMessage(&msg);	

#ifndef USE_DEBUG       //非DEBUG的时候有错误不能直接弹出对话框，要根据函数返回值退出后再提示，另外要用RELEASE版编译后运行才不会在退出显示MESSAGEBOX时出错
			if(InfoID != 0) 
			{
				SendMessage(hWnd, WM_CLOSE, 0, 0);
				continue;
			}
#endif

			if(msg.message==WM_CLOSE) break;
			ControlProc();

			//显示出错，可能是重置设备或切换分辨率的问题
			InfoID = DisplayMyScene();
#ifndef USE_DEBUG       //非DEBUG的时候有错误不能直接弹出对话框，要根据函数返回值退出后再提示，另外要用RELEASE版编译后运行才不会在退出显示MESSAGEBOX时出错
			if(InfoID != 0) 
			{
				SendMessage(hWnd, WM_CLOSE, 0, 0);
				continue;
			}
#endif
		}
	}

	ReleaseD3D();
	UnregisterClass("My D3D Application", wc.hInstance);
	LoadString(hInstance, InfoID, pInfo, 500);
	// 如果直接返回E_FAIL，就不要提示了（表示该错误类型内部字串中是没有的），直接退出即可
	if(InfoID != 0 && InfoID != E_FAIL)
		MessageBox(NULL, pInfo, "Fatal Error", MB_OK);
	return(0);
}





















