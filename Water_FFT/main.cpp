#include "res\resource.h"
#include "myd3d.h"
#include "UserControl.h"


HRESULT InitD3D(HINSTANCE hInstance);
//�ú��������Զ��������Ϣ��0��ʾ�ɹ�
UINT InitMyScene();
void ControlProc();
//��Ϊ�ú���Ҫ����InitMyScene�����Է���ֵ������ͬ��������ʾģʽ�л�ʱ���������ʾ������
UINT DisplayMyScene();
void ReleaseD3D();


//���ڵ�λ�úʹ�С
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
	// Ҫ���¼���֧�ֵ�ɫ��ģʽ����������ɫλ����ͬ����ֻ��Ҫ����ýṹ��Ȼ��ѭ���е�K�ĳ�3������
	D3DFORMAT SupportColorFormat[2]={D3DFMT_R5G6B5, D3DFMT_X8R8G8B8};

			//�õ�ö����Ϣ��ֻ֧��400*300���ϣ�X8R8G8B8��R5G6B5������D24S8��D16�����˵����е�ˢ����
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

				//�õ���ǰ��ʾģʽ��ö�٣�����ˢ���ʣ��е����
				if(ModeInfo.Width==d3ddm.Width && ModeInfo.Height==d3ddm.Height && ModeInfo.Format==d3ddm.Format && CurrentIndex==65535)
					CurrentIndex = ModeCount;
		

				SkipSign = FALSE;
				//ȥ��400*300���µ�ģʽ���ֱ���̫��ʱʹ��d3dim��pure hal���������
				if(ModeInfo.Width <400 || ModeInfo.Height < 300)
					SkipSign = TRUE;
				//�������Ϊ��ȥ��ͬһ����ʾģʽ����ͬˢ���ʵ�ѡ��
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
			//��ʼ���б�
			for(i=0; i<ModeCount; i++)
			{
				ModeInfo = *(pModeInfo+i);
				//������ɫֵ
				if(ModeInfo.Format==D3DFMT_R5G6B5) ColorBits = 16;
				else if(ModeInfo.Format==D3DFMT_X8R8G8B8) ColorBits = 32;
				else ColorBits = 0;
				sprintf(pListInfo, "%d*%d, %dbits", ModeInfo.Width, ModeInfo.Height, ColorBits);
				SendMessage(hModeList, CB_ADDSTRING, 0, (LPARAM)pListInfo);
			}

			//����ǰ������ΪĬ��ѡ��
			SendMessage(hModeList, CB_SETCURSEL, (WPARAM)CurrentIndex, 0);
			delete[] pModeInfo;
			delete[] pModeExist;
			return TRUE;

		case WM_COMMAND:
			if(LOWORD(wParam) == IDOK)
			{
				//����check box���ô����µķֱ���ģʽ
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
					//����combo box������ģʽʱ������ȫ��ģʽ���޸Ķ���Ч
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

//Ϊ�˱�֤֡�����ȶ��ԣ���ʱȥ��	Sleep(1);

	switch(msg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_PAINT:
			return 0;

		//�ı䴰��λ�úʹ�С��Ҫ����
		case WM_SIZE:
			if(WindowMode == FALSE) return 0; //�Ǵ���ģʽ�Ͳ��ı䣬��ֹ�л���ȫ��ʱ�޷�����ԭ���Ĵ��ڷֱ���
			GetWindowRect(hWnd, &WindowRect);

			// ���ֻ����Ϊ�û����Ƶ��²�������Ϣ����ʵ��ģʽ�ͷֱ��ʶ�û�䣬��ô�Ͳ�Ҫ���������Դ���ģʽ����ʱ�����һ��Size��Ϣ�����ߵ���߿���ı䴰�ڴ�С��ʵ��û�ı䣬�����ڴ����Ѿ�����������µ���󻯰�ť��
			if(WindowMode == bLastWindowMode && (WindowRect.right-WindowRect.left) == WindowWidth && (WindowRect.bottom-WindowRect.top) == WindowHeight)
				return 0;
			// ��С����ʱ�����
			if(WindowRect.right < 0 || WindowRect.left < 0 || WindowRect.bottom < 0 || WindowRect.top < 0)
				return 0;

			bLastWindowMode = WindowMode;

			WindowWidth = WindowRect.right - WindowRect.left;
			WindowHeight = WindowRect.bottom - WindowRect.top;
			if(WindowMode)
			{
				ChangeDisplayModeSign = TRUE; //��̬�ı䴰�ڴ�Сʱ��Ҫ��̬Reset
				if(WindowWidth < 400)      //��ֹ�ֱ���С��400*300
					WindowWidth = 400;
				if(WindowHeight < 300)
					WindowHeight = 300;
			}
			return 0;
		case WM_MOVE:
			if(WindowMode == FALSE) return 0; //�Ǵ���ģʽ�Ͳ��ı䣬��ֹ�л���ȫ��ʱ�޷�����ԭ���Ĵ��ڷֱ���
			GetWindowRect(hWnd, &WindowRect);
			WindowX = WindowRect.left;
			WindowY = WindowRect.top;
			return 0;
		//���س����˳��������еļ�����Ϣ������USERCONTROL����
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


			// ������
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

#ifndef USE_DEBUG       //��DEBUG��ʱ���д�����ֱ�ӵ����Ի���Ҫ���ݺ�������ֵ�˳�������ʾ������Ҫ��RELEASE���������вŲ������˳���ʾMESSAGEBOXʱ����
			if(InfoID != 0) 
			{
				SendMessage(hWnd, WM_CLOSE, 0, 0);
				continue;
			}
#endif

			if(msg.message==WM_CLOSE) break;
			ControlProc();

			//��ʾ���������������豸���л��ֱ��ʵ�����
			InfoID = DisplayMyScene();
#ifndef USE_DEBUG       //��DEBUG��ʱ���д�����ֱ�ӵ����Ի���Ҫ���ݺ�������ֵ�˳�������ʾ������Ҫ��RELEASE���������вŲ������˳���ʾMESSAGEBOXʱ����
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
	// ���ֱ�ӷ���E_FAIL���Ͳ�Ҫ��ʾ�ˣ���ʾ�ô��������ڲ��ִ�����û�еģ���ֱ���˳�����
	if(InfoID != 0 && InfoID != E_FAIL)
		MessageBox(NULL, pInfo, "Fatal Error", MB_OK);
	return(0);
}





















