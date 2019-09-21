#include "res\\resource.h"
#include "myd3d.h"
#include "UserControl.h"

KMouseControl g_MouseControl;

HRESULT ResetDevice();
BOOL CALLBACK ChangeDisplayModeDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK GeneralDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define KEYDOWN(vk_code)  ((GetAsyncKeyState(vk_code) & 0x8000)? 1 : 0)
#define KEYUP(vk_code)    ((GetAsyncKeyState(vk_code) & 0x8000)? 0 : 1)
#define KEYALTDOWN        ((GetAsyncKeyState(VK_LMENU) & 0x8000 || GetAsyncKeyState(VK_RMENU) & 0x8000)? 1 : 0)
#define VK_A 0x41
#define VK_S 0x53
#define VK_X 0x58
#define VK_Z 0x5a


#ifdef USE_IMMEDIATE_KEYBOARD_INPUT
	#define DIIM_KEYDOWN(buffer, key) ( (buffer[key] & 0x80) ? 1 : 0 )
	#define DIIM_KEYUP(buffer, key) ( (buffer[key] & 0x80) ? 0 : 1 )
#endif
#ifdef USE_BUFFERED_KEYBOARD_INPUT
	// 缓冲数据，dwData表示状态（按下还是抬起），dwOfs表示数据（按键的扫描码，等同于DIK_XXX）
	#define DIBUF_KEYDOWN(buffer, element, key) ( ((buffer[element].dwData & 0x80) ? 1 : 0 ) & (buffer[element].dwOfs == key) )
	#define DIBUF_KEYUP(buffer, element, key) ( ((buffer[element].dwData & 0x80) ? 0 : 1 ) & (buffer[element].dwOfs == key) )
#endif


#ifdef USE_IMMEDIATE_MOUSE_INPUT
#define DIIM_MOUSEDOWN(buffer, key) ( (buffer.rgbButtons[key] & 0x80) ? 1 : 0 )
#define DIIM_MOUSEUP(buffer, key) ( (buffer.rgbButtons[key] & 0x80) ? 0 : 1 )
#endif
#ifdef USE_BUFFERED_MOUSE_INPUT
// 缓冲数据，dwData表示数据（按下还是抬起或移动的距离），dwOfs表示类型（哪个按键、轴）
#define DIBUF_MOUSEDOWN(buffer, element, key) ( ((buffer[element].dwData & 0x80) ? 1 : 0 ) & (buffer[element].dwOfs == key) )
#define DIBUF_MOUSEUP(buffer, element, key) ( ((buffer[element].dwData & 0x80) ? 0 : 1 ) & (buffer[element].dwOfs == key) )
#endif



int effectenable=0xffffffff;	//0表示禁止，非0表示允许，注意不是1
UINT g_iCurrentEffectNo = 0;		// 0表示第一种类型的特效，按加或减依次累加或递减表示不同的特效
UINT DialogSign = 0;    ////处理通用对话框时，需要根据该值来确定弹出对话框的ID，为0表示无弹出对话框
BOOL BeforeDlgChangeModeSign = FALSE;  //处理通用对话框，在得到按键信息后，马上先改变显示模式（暂时不弹出对话框）


//从原来的摄像机x,y,z，移动方向为changeway，根据Y-AxisTranslation和MoveStep,RiseStep的值来判断碰撞
void JudgeMovement(unsigned short int changeway, float x, float y, float z)
{
	//在此修正移动到有障碍或边界的地方的EYE坐标，按照changeway的相反方向来修正，如前进了1，可在前方0.3的地方有障碍，则现在要后退1-0.3
}




//此函数即用户控制函数，在WINMAIN函数循环中调用，修改其中的VK_（即键盘/鼠标/……）来实现对应的功能，也可改用DINPUT
void ControlProc()
{
	HRESULT hr = S_OK;

	//弹出选择显示模式对话框前，需要先置为Window模式，所以要保存原来的模式状态值，同时还要给定一个状态
	static BOOL OldWindowMode;
	static UINT presssign[10]={0,0,0,0,0,0,0,0,0,0};      //0 ready  1 PRESSED
	//截图
	LPDIRECT3DSURFACE9 RenderTarget;
	//保存未移动前的摄像机坐标
	float x=CameraChange.Eye.x, y=CameraChange.Eye.y, z=CameraChange.Eye.z;


	//移动控制标记
	SHORT sForward = 0, sBackward = 0, sLeftward = 0, sRightward = 0, sUp = 0, sDown = 0;
	SHORT sTurnLeft = 0, sTurnRight = 0, sTurnUp = 0, sTurnDown = 0, sSliceLeft = 0, sSliceRight = 0;
	SHORT sResetCamera = 0, sEscape = 0;
	SHORT sLeftMouseKeyDown = 0, sRightMouseKeyDown = 0, sMidMouseKeyDown = 0;
	LONG iMouseMoveX = 0, iMouseMoveY = 0, iMouseMoveZ = 0;

	//////////////////////////////////////////////////////////////////////////Win32 : AsyncKey
#ifndef USE_BUFFERED_KEYBOARD_INPUT
#ifndef USE_BUFFERED_MOUSE_INPUT
	sForward |= KEYDOWN(VK_UP) || KEYDOWN('w') || KEYDOWN('W');
	sBackward |= KEYDOWN(VK_DOWN) || KEYDOWN('s') || KEYDOWN('S');
	sLeftward |= KEYDOWN('a') || KEYDOWN('A');
	sRightward |= KEYDOWN('d') || KEYDOWN('D');
	sUp |= KEYDOWN(VK_LCONTROL) || KEYDOWN(VK_SPACE);
	sDown |= KEYDOWN(VK_RCONTROL);
	sTurnLeft |= KEYDOWN(VK_LEFT);
	sTurnRight |= KEYDOWN(VK_RIGHT);
	sTurnUp |= KEYDOWN(VK_HOME);
	sTurnDown |= KEYDOWN(VK_END);
	sSliceLeft |= KEYDOWN(VK_LSHIFT);
	sSliceRight |= KEYDOWN(VK_RSHIFT);
	sResetCamera |= KEYDOWN('r') || KEYDOWN('R');
	sEscape |= KEYDOWN(VK_ESCAPE);
#endif
#endif

	//////////////////////////////////////////////////////////////////////////键盘
#ifdef USE_IMMEDIATE_KEYBOARD_INPUT
	// 获取DINPUT输入（立即数据）
	char     IM_buffer[256] = "";
	if(FAILED(g_pDIKey->GetDeviceState(sizeof(IM_buffer),(LPVOID)&IM_buffer)))
	{
		// 设备丢失
		g_pDIKey->Acquire();
		while(g_pDIKey->Acquire() == DIERR_INPUTLOST)
			g_pDIKey->Acquire();
		g_pDIKey->GetDeviceState(sizeof(IM_buffer),(LPVOID)&IM_buffer);
	}

	sForward |= DIIM_KEYDOWN(IM_buffer, DIK_UP) || DIIM_KEYDOWN(IM_buffer, DIK_W);
	sBackward |= DIIM_KEYDOWN(IM_buffer, DIK_DOWN) || DIIM_KEYDOWN(IM_buffer, DIK_S);
	sLeftward |= DIIM_KEYDOWN(IM_buffer, DIK_A);
	sRightward |= DIIM_KEYDOWN(IM_buffer, DIK_D);
	sUp |= DIIM_KEYDOWN(IM_buffer, DIK_LCONTROL) || DIIM_KEYDOWN(IM_buffer, DIK_SPACE);
	sDown |= DIIM_KEYDOWN(IM_buffer, DIK_RCONTROL);
	sTurnLeft |= DIIM_KEYDOWN(IM_buffer, DIK_LEFT);
	sTurnRight |= DIIM_KEYDOWN(IM_buffer, DIK_RIGHT);
	sTurnUp |= DIIM_KEYDOWN(IM_buffer, DIK_HOME);
	sTurnDown |= DIIM_KEYDOWN(IM_buffer, DIK_END);
	sSliceLeft |= DIIM_KEYDOWN(IM_buffer, DIK_LSHIFT);
	sSliceRight |= DIIM_KEYDOWN(IM_buffer, DIK_RSHIFT);
	sResetCamera |= DIIM_KEYDOWN(IM_buffer, DIK_R);
	sEscape |= DIIM_KEYDOWN(IM_buffer, DIK_ESCAPE);
#endif

	DWORD dwElements = SAMPLE_BUFFER_SIZE;	// 采样数

	// 缓冲区数据
#ifdef USE_BUFFERED_KEYBOARD_INPUT
	DIDEVICEOBJECTDATA BUF_buffer[ SAMPLE_BUFFER_SIZE ];  // Receives buffered data 
	ZeroMemory(BUF_buffer, sizeof(DIDEVICEOBJECTDATA) * SAMPLE_BUFFER_SIZE);
	if(FAILED(hr = g_pDIKey->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), BUF_buffer, &dwElements, 0 )))
	{
		// 设备丢失
		g_pDIKey->Acquire();
		while(g_pDIKey->Acquire() == DIERR_INPUTLOST)
			g_pDIKey->Acquire();
		hr = g_pDIKey->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), BUF_buffer, &dwElements, 0 );
	}
    
	sForward |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_UP) || DIBUF_KEYDOWN(BUF_buffer, 0, DIK_W);
	sBackward |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_DOWN) || DIBUF_KEYDOWN(BUF_buffer, 0, DIK_S);
	sLeftward |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_A);
	sRightward |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_D);
	sUp |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_LCONTROL) || DIBUF_KEYDOWN(BUF_buffer, 0, DIK_SPACE);
	sDown |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_RCONTROL);
	sTurnLeft |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_LEFT);
	sTurnRight |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_RIGHT);
	sTurnUp |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_HOME);
	sTurnDown |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_END);
	sSliceLeft |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_LSHIFT);
	sSliceRight |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_RSHIFT);
	sResetCamera |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_R);
	sEscape |= DIBUF_KEYDOWN(BUF_buffer, 0, DIK_ESCAPE);

	for(UINT i = 1; i < SAMPLE_BUFFER_SIZE; i++)
	{
		sForward |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_UP) || DIBUF_KEYDOWN(BUF_buffer, i, DIK_W);
		sBackward |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_DOWN) || DIBUF_KEYDOWN(BUF_buffer, i, DIK_S);
		sLeftward |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_A);
		sRightward |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_D);
		sUp |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_LCONTROL) || DIBUF_KEYDOWN(BUF_buffer, i, DIK_SPACE);
		sDown |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_RCONTROL);
		sTurnLeft |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_LEFT);
		sTurnRight |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_RIGHT);
		sTurnUp |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_HOME);
		sTurnDown |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_END);
		sSliceLeft |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_LSHIFT);
		sSliceRight |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_RSHIFT);
		sResetCamera |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_R);
		sEscape |= DIBUF_KEYDOWN(BUF_buffer, i, DIK_ESCAPE);
	}

#endif



	//////////////////////////////////////////////////////////////////////////鼠标
#ifdef USE_IMMEDIATE_MOUSE_INPUT
	DIMOUSESTATE IM_MouseState;
	ZeroMemory( &IM_MouseState, sizeof(DIMOUSESTATE) );
	if(FAILED(hr = g_pDIMouse->GetDeviceState( sizeof(DIMOUSESTATE), &IM_MouseState )))
	{
		// 设备丢失
		g_pDIMouse->Acquire();
		while(g_pDIMouse->Acquire() == DIERR_INPUTLOST)
			g_pDIMouse->Acquire();
		hr = g_pDIMouse->GetDeviceState( sizeof(DIMOUSESTATE), &IM_MouseState );
	}

	sLeftMouseKeyDown |= DIIM_MOUSEDOWN(IM_MouseState, 0);
	sRightMouseKeyDown |= DIIM_MOUSEDOWN(IM_MouseState, 1);
	sMidMouseKeyDown |= DIIM_MOUSEDOWN(IM_MouseState, 2);
	iMouseMoveX += IM_MouseState.lX;	// 这些值都是表示移动偏移，可能为负数！
	iMouseMoveY += IM_MouseState.lY;
	iMouseMoveZ += IM_MouseState.lZ;
#endif

#ifdef USE_BUFFERED_MOUSE_INPUT
	DIDEVICEOBJECTDATA BUF_MouseState[ SAMPLE_BUFFER_SIZE ];
	ZeroMemory(BUF_MouseState, sizeof(DIDEVICEOBJECTDATA) * SAMPLE_BUFFER_SIZE);
	if(FAILED(hr = g_pDIMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), BUF_MouseState, &dwElements, 0 )))
	{
		// 设备丢失
		g_pDIMouse->Acquire();
		while(g_pDIMouse->Acquire() == DIERR_INPUTLOST)
			g_pDIMouse->Acquire();
		hr = g_pDIMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), BUF_MouseState, &dwElements, 0 );
	}

	sLeftMouseKeyDown |= DIBUF_MOUSEDOWN(BUF_MouseState, 0, DIMOFS_BUTTON0);
	sRightMouseKeyDown |= DIBUF_MOUSEDOWN(BUF_MouseState, 0, DIMOFS_BUTTON1);
	sMidMouseKeyDown |= DIBUF_MOUSEDOWN(BUF_MouseState, 0, DIMOFS_BUTTON2);
	
	if(BUF_MouseState[0].dwOfs == DIMOFS_X)
		iMouseMoveX += BUF_MouseState[0].dwData;	// 这些值都是表示移动偏移，可能为负数！
	if(BUF_MouseState[0].dwOfs == DIMOFS_Y)
		iMouseMoveY += BUF_MouseState[0].dwData;	// 这些值都是表示移动偏移，可能为负数！
	if(BUF_MouseState[0].dwOfs == DIMOFS_Z)
		iMouseMoveZ += BUF_MouseState[0].dwData;	// 这些值都是表示移动偏移，可能为负数！

	for(UINT i = 1; i < SAMPLE_BUFFER_SIZE; i++)
	{
		sLeftMouseKeyDown |= DIBUF_MOUSEDOWN(BUF_MouseState, i, DIMOFS_BUTTON0);
		sRightMouseKeyDown |= DIBUF_MOUSEDOWN(BUF_MouseState, i, DIMOFS_BUTTON1);
		sMidMouseKeyDown |= DIBUF_MOUSEDOWN(BUF_MouseState, i, DIMOFS_BUTTON2);
		if(BUF_MouseState[i].dwOfs == DIMOFS_X)
			iMouseMoveX += BUF_MouseState[i].dwData;	// 这些值都是表示移动偏移，可能为负数！
		if(BUF_MouseState[i].dwOfs == DIMOFS_Y)
			iMouseMoveY += BUF_MouseState[i].dwData;	// 这些值都是表示移动偏移，可能为负数！
		if(BUF_MouseState[i].dwOfs == DIMOFS_Z)
			iMouseMoveZ += BUF_MouseState[i].dwData;	// 这些值都是表示移动偏移，可能为负数！
	}

#endif

	// 根据WINDOWS消息来进行鼠标移动范围判断
	iMouseMoveX = g_MouseControl.GetMouseMove().x;
	iMouseMoveY = g_MouseControl.GetMouseMove().y;
	

	// 根据按键进行操作
	if(sForward) {CameraChange.ChangeCamera(CAM_GOFORWARD);JudgeMovement(CAM_GOFORWARD,x,y,z);}
	if(sBackward) {CameraChange.ChangeCamera(CAM_GOBACKWARD);JudgeMovement(CAM_GOBACKWARD,x,y,z);}
	if(sLeftward) {CameraChange.ChangeCamera(CAM_GOLEFT);JudgeMovement(CAM_GOLEFT,x,y,z);}
	if(sRightward) {CameraChange.ChangeCamera(CAM_GORIGHT);JudgeMovement(CAM_GORIGHT,x,y,z);}
	if(sUp) {CameraChange.ChangeCamera(CAM_GOUP);JudgeMovement(CAM_GOUP,x,y,z);}
	if(sDown) {CameraChange.ChangeCamera(CAM_GODOWN);JudgeMovement(CAM_GODOWN,x,y,z);}
	if(sTurnLeft) CameraChange.ChangeCamera(CAM_TURNLEFT);
	if(sTurnRight) CameraChange.ChangeCamera(CAM_TURNRIGHT);
	if(sTurnUp) CameraChange.ChangeCamera(CAM_TURNUP);
	if(sTurnDown) CameraChange.ChangeCamera(CAM_TURNDOWN);
	if(sSliceLeft) CameraChange.ChangeCamera(CAM_SLICELEFT);
	if(sSliceRight) CameraChange.ChangeCamera(CAM_SLICERIGHT);
	if(sResetCamera) CameraChange.ResetCamera();

	if(iMouseMoveX < 0 && BASE_MOUSEMOVE_DISTANCE)
	{
		float fMoveCoef = -(float)iMouseMoveX / (float)BASE_MOUSEMOVE_DISTANCE;
		CameraChange.ChangeCamera(CAM_TURNLEFT, fMoveCoef);
	}
	if(iMouseMoveX > 0 && BASE_MOUSEMOVE_DISTANCE)
	{
		float fMoveCoef = (float)iMouseMoveX / (float)BASE_MOUSEMOVE_DISTANCE;
		CameraChange.ChangeCamera(CAM_TURNRIGHT, fMoveCoef);
	}
	if(iMouseMoveY < 0 && BASE_MOUSEMOVE_DISTANCE)
	{
		float fMoveCoef = -(float)iMouseMoveY / (float)BASE_MOUSEMOVE_DISTANCE;
		CameraChange.ChangeCamera(CAM_TURNUP, fMoveCoef);
	}
	if(iMouseMoveY > 0 && BASE_MOUSEMOVE_DISTANCE)
	{
		float fMoveCoef = (float)iMouseMoveY / (float)BASE_MOUSEMOVE_DISTANCE;
		CameraChange.ChangeCamera(CAM_TURNDOWN, fMoveCoef);
	}

	// 按ESCAPE就退出
	if(sEscape) {PostMessage(hWnd,WM_CLOSE,0,0); return;}
	

	// 如果没有移动键按下，就传入NOCHANGE，用于计算MOVE或IDLE状态（HDR人眼自动适应）
	if(!sForward && !sTurnDown && !sLeftward && !sRightward && !sUp && !sDown && !sTurnLeft && !sTurnRight && !sTurnUp && !sTurnDown)
		CameraChange.ChangeCamera(CAM_NOCHANGE);




	// 运行时的系统控制，之所以要用presssign是为了让按下再放开之后才生效
	UINT iPressType = 0;
	if(KEYDOWN(VK_RETURN)) presssign[iPressType]=1;     //切换特效
	if(KEYUP(VK_RETURN) && presssign[iPressType])
	{
		presssign[iPressType] = 0;
		effectenable = !effectenable;	
	}
	iPressType++;


	// 增加特效号，187是虚拟键等号（大键盘）
	if(KEYDOWN(VK_ADD)) presssign[iPressType] = 1;
	if(KEYUP(VK_ADD) && presssign[iPressType])
	{
		presssign[iPressType] = 0;
		g_iCurrentEffectNo ++;
	}
	iPressType++;
	if(KEYDOWN(187)) presssign[iPressType] = 1;
	if(KEYUP(187) && presssign[iPressType])
	{
		presssign[iPressType] = 0;
		g_iCurrentEffectNo ++;
	}
	iPressType++;


	// 减小特效号，不能小于0。189是虚拟键减号（大键盘）
	if(KEYDOWN(VK_SUBTRACT)) presssign[iPressType] = 1;
	if(KEYUP(VK_SUBTRACT) && presssign[iPressType])
	{
		presssign[iPressType] = 0;
		if(g_iCurrentEffectNo > 0)
			g_iCurrentEffectNo --;
	}
	iPressType++;
	if(KEYDOWN(189)) presssign[iPressType] = 1;
	if(KEYUP(189) && presssign[iPressType])
	{
		presssign[iPressType] = 0;
		if(g_iCurrentEffectNo > 0)
			g_iCurrentEffectNo --;
	}
	iPressType++;

	
	if(KEYDOWN(VK_F10)) presssign[iPressType]=1;           //切换全屏窗口
	if(KEYUP(VK_F10) && presssign[iPressType])
	{
		presssign[iPressType] = 0;
		WindowMode = !WindowMode;
		ChangeDisplayModeSign = TRUE;
	}
	iPressType++;


	if(KEYDOWN(VK_F11)) presssign[iPressType]=1;           //截图
	if(KEYUP(VK_F11) && presssign[iPressType])
	{
		presssign[iPressType] = 0;
		if(FAILED(d3ddevice->GetRenderTarget(0, &RenderTarget))) 
			return;
		if(FAILED(D3DXSaveSurfaceToFile("ScrShot.bmp", D3DXIFF_BMP, RenderTarget, NULL, NULL))) 
			return;
		RenderTarget->Release();
	}
	iPressType++;

	








	
	//处理通用对话框，先写所有的按键对应的ID
	if(KEYDOWN(VK_F2)) DialogSign = IDD_CHANGEMODEDLG;
	if(KEYDOWN(VK_F1) || KEYDOWN(VK_F1)) DialogSign = IDD_HELPDLG;
	if(KEYDOWN(VK_F3) || KEYDOWN(VK_F3)) DialogSign = IDD_ABOUTDLG;
	
	if(DialogSign && BeforeDlgChangeModeSign == FALSE)
	{
		//先切换到窗口模式
		OldWindowMode = WindowMode;
		WindowMode = TRUE;
		ChangeDisplayModeSign = TRUE;
		BeforeDlgChangeModeSign = TRUE;
		return;
	}
	
	if(BeforeDlgChangeModeSign)
	{
		//若上一步在弹出对话框前切换到窗口了，那么这一步就弹出窗口，恢复原来的模式，并重置设备
		ChangeDisplayModeSign = TRUE;
		WindowMode = OldWindowMode;
		BeforeDlgChangeModeSign = FALSE;

		//通用对话框处理函数，特殊的才在下面的if中改变
		DLGPROC ProcFunc = GeneralDlgProc;
		if (DialogSign == IDD_CHANGEMODEDLG)
			ProcFunc = ChangeDisplayModeDlgProc;

		DialogBox((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE), (char *)DialogSign, hWnd, ProcFunc);
		DialogSign = 0;
	}
	

	// 每帧都要进行，设置上一帧渲染的时间，放到渲染前
	CameraChange.SetFrameTime();
	// 每帧都要重置鼠标状态，在前面判断了鼠标当前状态，但有些状态是瞬时的，前面要用这些瞬时数据，所以不能改变，那么就在这里把瞬时数据改掉
	g_MouseControl.Breath();
}
