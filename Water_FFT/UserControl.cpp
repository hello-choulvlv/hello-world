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
	// �������ݣ�dwData��ʾ״̬�����»���̧�𣩣�dwOfs��ʾ���ݣ�������ɨ���룬��ͬ��DIK_XXX��
	#define DIBUF_KEYDOWN(buffer, element, key) ( ((buffer[element].dwData & 0x80) ? 1 : 0 ) & (buffer[element].dwOfs == key) )
	#define DIBUF_KEYUP(buffer, element, key) ( ((buffer[element].dwData & 0x80) ? 0 : 1 ) & (buffer[element].dwOfs == key) )
#endif


#ifdef USE_IMMEDIATE_MOUSE_INPUT
#define DIIM_MOUSEDOWN(buffer, key) ( (buffer.rgbButtons[key] & 0x80) ? 1 : 0 )
#define DIIM_MOUSEUP(buffer, key) ( (buffer.rgbButtons[key] & 0x80) ? 0 : 1 )
#endif
#ifdef USE_BUFFERED_MOUSE_INPUT
// �������ݣ�dwData��ʾ���ݣ����»���̧����ƶ��ľ��룩��dwOfs��ʾ���ͣ��ĸ��������ᣩ
#define DIBUF_MOUSEDOWN(buffer, element, key) ( ((buffer[element].dwData & 0x80) ? 1 : 0 ) & (buffer[element].dwOfs == key) )
#define DIBUF_MOUSEUP(buffer, element, key) ( ((buffer[element].dwData & 0x80) ? 0 : 1 ) & (buffer[element].dwOfs == key) )
#endif



int effectenable=0xffffffff;	//0��ʾ��ֹ����0��ʾ����ע�ⲻ��1
UINT g_iCurrentEffectNo = 0;		// 0��ʾ��һ�����͵���Ч�����ӻ�������ۼӻ�ݼ���ʾ��ͬ����Ч
UINT DialogSign = 0;    ////����ͨ�öԻ���ʱ����Ҫ���ݸ�ֵ��ȷ�������Ի����ID��Ϊ0��ʾ�޵����Ի���
BOOL BeforeDlgChangeModeSign = FALSE;  //����ͨ�öԻ����ڵõ�������Ϣ�������ȸı���ʾģʽ����ʱ�������Ի���


//��ԭ���������x,y,z���ƶ�����Ϊchangeway������Y-AxisTranslation��MoveStep,RiseStep��ֵ���ж���ײ
void JudgeMovement(unsigned short int changeway, float x, float y, float z)
{
	//�ڴ������ƶ������ϰ���߽�ĵط���EYE���꣬����changeway���෴��������������ǰ����1������ǰ��0.3�ĵط����ϰ���������Ҫ����1-0.3
}




//�˺������û����ƺ�������WINMAIN����ѭ���е��ã��޸����е�VK_��������/���/��������ʵ�ֶ�Ӧ�Ĺ��ܣ�Ҳ�ɸ���DINPUT
void ControlProc()
{
	HRESULT hr = S_OK;

	//����ѡ����ʾģʽ�Ի���ǰ����Ҫ����ΪWindowģʽ������Ҫ����ԭ����ģʽ״ֵ̬��ͬʱ��Ҫ����һ��״̬
	static BOOL OldWindowMode;
	static UINT presssign[10]={0,0,0,0,0,0,0,0,0,0};      //0 ready  1 PRESSED
	//��ͼ
	LPDIRECT3DSURFACE9 RenderTarget;
	//����δ�ƶ�ǰ�����������
	float x=CameraChange.Eye.x, y=CameraChange.Eye.y, z=CameraChange.Eye.z;


	//�ƶ����Ʊ��
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

	//////////////////////////////////////////////////////////////////////////����
#ifdef USE_IMMEDIATE_KEYBOARD_INPUT
	// ��ȡDINPUT���루�������ݣ�
	char     IM_buffer[256] = "";
	if(FAILED(g_pDIKey->GetDeviceState(sizeof(IM_buffer),(LPVOID)&IM_buffer)))
	{
		// �豸��ʧ
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

	DWORD dwElements = SAMPLE_BUFFER_SIZE;	// ������

	// ����������
#ifdef USE_BUFFERED_KEYBOARD_INPUT
	DIDEVICEOBJECTDATA BUF_buffer[ SAMPLE_BUFFER_SIZE ];  // Receives buffered data 
	ZeroMemory(BUF_buffer, sizeof(DIDEVICEOBJECTDATA) * SAMPLE_BUFFER_SIZE);
	if(FAILED(hr = g_pDIKey->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), BUF_buffer, &dwElements, 0 )))
	{
		// �豸��ʧ
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



	//////////////////////////////////////////////////////////////////////////���
#ifdef USE_IMMEDIATE_MOUSE_INPUT
	DIMOUSESTATE IM_MouseState;
	ZeroMemory( &IM_MouseState, sizeof(DIMOUSESTATE) );
	if(FAILED(hr = g_pDIMouse->GetDeviceState( sizeof(DIMOUSESTATE), &IM_MouseState )))
	{
		// �豸��ʧ
		g_pDIMouse->Acquire();
		while(g_pDIMouse->Acquire() == DIERR_INPUTLOST)
			g_pDIMouse->Acquire();
		hr = g_pDIMouse->GetDeviceState( sizeof(DIMOUSESTATE), &IM_MouseState );
	}

	sLeftMouseKeyDown |= DIIM_MOUSEDOWN(IM_MouseState, 0);
	sRightMouseKeyDown |= DIIM_MOUSEDOWN(IM_MouseState, 1);
	sMidMouseKeyDown |= DIIM_MOUSEDOWN(IM_MouseState, 2);
	iMouseMoveX += IM_MouseState.lX;	// ��Щֵ���Ǳ�ʾ�ƶ�ƫ�ƣ�����Ϊ������
	iMouseMoveY += IM_MouseState.lY;
	iMouseMoveZ += IM_MouseState.lZ;
#endif

#ifdef USE_BUFFERED_MOUSE_INPUT
	DIDEVICEOBJECTDATA BUF_MouseState[ SAMPLE_BUFFER_SIZE ];
	ZeroMemory(BUF_MouseState, sizeof(DIDEVICEOBJECTDATA) * SAMPLE_BUFFER_SIZE);
	if(FAILED(hr = g_pDIMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), BUF_MouseState, &dwElements, 0 )))
	{
		// �豸��ʧ
		g_pDIMouse->Acquire();
		while(g_pDIMouse->Acquire() == DIERR_INPUTLOST)
			g_pDIMouse->Acquire();
		hr = g_pDIMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), BUF_MouseState, &dwElements, 0 );
	}

	sLeftMouseKeyDown |= DIBUF_MOUSEDOWN(BUF_MouseState, 0, DIMOFS_BUTTON0);
	sRightMouseKeyDown |= DIBUF_MOUSEDOWN(BUF_MouseState, 0, DIMOFS_BUTTON1);
	sMidMouseKeyDown |= DIBUF_MOUSEDOWN(BUF_MouseState, 0, DIMOFS_BUTTON2);
	
	if(BUF_MouseState[0].dwOfs == DIMOFS_X)
		iMouseMoveX += BUF_MouseState[0].dwData;	// ��Щֵ���Ǳ�ʾ�ƶ�ƫ�ƣ�����Ϊ������
	if(BUF_MouseState[0].dwOfs == DIMOFS_Y)
		iMouseMoveY += BUF_MouseState[0].dwData;	// ��Щֵ���Ǳ�ʾ�ƶ�ƫ�ƣ�����Ϊ������
	if(BUF_MouseState[0].dwOfs == DIMOFS_Z)
		iMouseMoveZ += BUF_MouseState[0].dwData;	// ��Щֵ���Ǳ�ʾ�ƶ�ƫ�ƣ�����Ϊ������

	for(UINT i = 1; i < SAMPLE_BUFFER_SIZE; i++)
	{
		sLeftMouseKeyDown |= DIBUF_MOUSEDOWN(BUF_MouseState, i, DIMOFS_BUTTON0);
		sRightMouseKeyDown |= DIBUF_MOUSEDOWN(BUF_MouseState, i, DIMOFS_BUTTON1);
		sMidMouseKeyDown |= DIBUF_MOUSEDOWN(BUF_MouseState, i, DIMOFS_BUTTON2);
		if(BUF_MouseState[i].dwOfs == DIMOFS_X)
			iMouseMoveX += BUF_MouseState[i].dwData;	// ��Щֵ���Ǳ�ʾ�ƶ�ƫ�ƣ�����Ϊ������
		if(BUF_MouseState[i].dwOfs == DIMOFS_Y)
			iMouseMoveY += BUF_MouseState[i].dwData;	// ��Щֵ���Ǳ�ʾ�ƶ�ƫ�ƣ�����Ϊ������
		if(BUF_MouseState[i].dwOfs == DIMOFS_Z)
			iMouseMoveZ += BUF_MouseState[i].dwData;	// ��Щֵ���Ǳ�ʾ�ƶ�ƫ�ƣ�����Ϊ������
	}

#endif

	// ����WINDOWS��Ϣ����������ƶ���Χ�ж�
	iMouseMoveX = g_MouseControl.GetMouseMove().x;
	iMouseMoveY = g_MouseControl.GetMouseMove().y;
	

	// ���ݰ������в���
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

	// ��ESCAPE���˳�
	if(sEscape) {PostMessage(hWnd,WM_CLOSE,0,0); return;}
	

	// ���û���ƶ������£��ʹ���NOCHANGE�����ڼ���MOVE��IDLE״̬��HDR�����Զ���Ӧ��
	if(!sForward && !sTurnDown && !sLeftward && !sRightward && !sUp && !sDown && !sTurnLeft && !sTurnRight && !sTurnUp && !sTurnDown)
		CameraChange.ChangeCamera(CAM_NOCHANGE);




	// ����ʱ��ϵͳ���ƣ�֮����Ҫ��presssign��Ϊ���ð����ٷſ�֮�����Ч
	UINT iPressType = 0;
	if(KEYDOWN(VK_RETURN)) presssign[iPressType]=1;     //�л���Ч
	if(KEYUP(VK_RETURN) && presssign[iPressType])
	{
		presssign[iPressType] = 0;
		effectenable = !effectenable;	
	}
	iPressType++;


	// ������Ч�ţ�187��������Ⱥţ�����̣�
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


	// ��С��Ч�ţ�����С��0��189����������ţ�����̣�
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

	
	if(KEYDOWN(VK_F10)) presssign[iPressType]=1;           //�л�ȫ������
	if(KEYUP(VK_F10) && presssign[iPressType])
	{
		presssign[iPressType] = 0;
		WindowMode = !WindowMode;
		ChangeDisplayModeSign = TRUE;
	}
	iPressType++;


	if(KEYDOWN(VK_F11)) presssign[iPressType]=1;           //��ͼ
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

	








	
	//����ͨ�öԻ�����д���еİ�����Ӧ��ID
	if(KEYDOWN(VK_F2)) DialogSign = IDD_CHANGEMODEDLG;
	if(KEYDOWN(VK_F1) || KEYDOWN(VK_F1)) DialogSign = IDD_HELPDLG;
	if(KEYDOWN(VK_F3) || KEYDOWN(VK_F3)) DialogSign = IDD_ABOUTDLG;
	
	if(DialogSign && BeforeDlgChangeModeSign == FALSE)
	{
		//���л�������ģʽ
		OldWindowMode = WindowMode;
		WindowMode = TRUE;
		ChangeDisplayModeSign = TRUE;
		BeforeDlgChangeModeSign = TRUE;
		return;
	}
	
	if(BeforeDlgChangeModeSign)
	{
		//����һ���ڵ����Ի���ǰ�л��������ˣ���ô��һ���͵������ڣ��ָ�ԭ����ģʽ���������豸
		ChangeDisplayModeSign = TRUE;
		WindowMode = OldWindowMode;
		BeforeDlgChangeModeSign = FALSE;

		//ͨ�öԻ�������������Ĳ��������if�иı�
		DLGPROC ProcFunc = GeneralDlgProc;
		if (DialogSign == IDD_CHANGEMODEDLG)
			ProcFunc = ChangeDisplayModeDlgProc;

		DialogBox((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE), (char *)DialogSign, hWnd, ProcFunc);
		DialogSign = 0;
	}
	

	// ÿ֡��Ҫ���У�������һ֡��Ⱦ��ʱ�䣬�ŵ���Ⱦǰ
	CameraChange.SetFrameTime();
	// ÿ֡��Ҫ�������״̬����ǰ���ж�����굱ǰ״̬������Щ״̬��˲ʱ�ģ�ǰ��Ҫ����Щ˲ʱ���ݣ����Բ��ܸı䣬��ô���������˲ʱ���ݸĵ�
	g_MouseControl.Breath();
}
