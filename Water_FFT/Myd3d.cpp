#include "myd3d.h"
void ReleaseMyScene();
UINT InitMyScene();


//ȫ�ֱ����ڴ˶���
LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3ddevice = NULL;

LPDIRECTINPUT8 g_pDI = NULL;
LPDIRECTINPUTDEVICE8 g_pDIKey = NULL;
LPDIRECTINPUTDEVICE8 g_pDIMouse = NULL;

D3DCAPS9 d3dcaps;
D3DDISPLAYMODE DesktopDisplayMode, d3ddm;
D3DFORMAT TextureFormat;
BOOL WindowMode = TRUE, ChangeDisplayModeSign = FALSE;
D3DPRESENT_PARAMETERS d3dpp;
HWND hWnd = NULL;

D3DXMATRIX myview, myproj;  //͸�Ӿ���͹۲����
float curfps = 0;               //��ǰ֡��
LARGE_INTEGER PerformanceFrequency;  //��ʱ��Ƶ�ʣ��ڲ�ʹ�ã�
CAMERACHANGE CameraChange;




//��ʼ��D3D�ӿ�
HRESULT DetectDeviceCaps();

HRESULT InitD3D(HINSTANCE hInstance)
{
	// �ȴ���DInput��DSound
	if(FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pDI, NULL)))
		return E_FAIL;
	if(FAILED(g_pDI->CreateDevice(GUID_SysKeyboard, &g_pDIKey, NULL)))
		return E_FAIL;
	if(FAILED(g_pDI->CreateDevice(GUID_SysMouse, &g_pDIMouse, NULL)))
		return E_FAIL;
		// ���ò���
	if(FAILED(g_pDIKey->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
		return E_FAIL;
	if(FAILED(g_pDIKey->SetDataFormat(&c_dfDIKeyboard)))
		return E_FAIL;
	if(FAILED(g_pDIMouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
		return E_FAIL;
	if(FAILED(g_pDIMouse->SetDataFormat(&c_dfDIMouse)))
		return E_FAIL;
		
		// �������룬�������ݸ�ʽ������
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj        = 0;
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData            = SAMPLE_BUFFER_SIZE; // Arbitary buffer size

#ifdef USE_BUFFERED_KEYBOARD_INPUT
	if( FAILED( g_pDIKey->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
		return E_FAIL;
#endif
#ifdef USE_BUFFERED_MOUSE_INPUT
	if( FAILED( g_pDIMouse->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
		return E_FAIL;
#endif

		// ��ʼ
	g_pDIKey->Acquire();
	g_pDIMouse->Acquire();



	// ��ʼ��D3D
	if(NULL==(d3d=Direct3DCreate9(D3D_SDK_VERSION)))
		  return E_FAIL;

	//��ѯ��ǰ�Կ�ģʽ��Desktop��Ŀǰ�������ã�d3ddm���ǵ�ǰʹ��ȫ��ģʽ�����ã���ʹ�Ǵ���ģʽ����ɫ��ֵҲ�ܺ͵�ǰ��һ��
	if(FAILED(d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &DesktopDisplayMode)))
		  return E_FAIL;
	if(FAILED(d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
		return E_FAIL;

	//ȫ���Ļ�Ĭ����32λɫ
	if(!WindowMode) d3ddm.Format = D3DFMT_X8R8G8B8;

	//��ѯ�豸�����������ö��㴦����ģʽ
	DWORD UserVertexShaderProcessing;

	d3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dcaps);

	if(FAILED(DetectDeviceCaps())) return E_FAIL;

	if(d3dcaps.VertexShaderVersion < D3DVS_VERSION(1, 0))
		UserVertexShaderProcessing=D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		UserVertexShaderProcessing=D3DCREATE_SOFTWARE_VERTEXPROCESSING;

//����D3D�豸
	ZeroMemory(&d3dpp,sizeof(d3dpp));

	d3dpp.Windowed = WindowMode;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferWidth = d3ddm.Width;
	d3dpp.BackBufferHeight = d3ddm.Height;
	d3dpp.EnableAutoDepthStencil = TRUE;
	
	if(WindowMode)
	{
		d3ddm.Format = DesktopDisplayMode.Format;
		d3dpp.BackBufferWidth = WindowWidth;
		d3dpp.BackBufferHeight = WindowHeight;
	}

	d3dpp.BackBufferFormat = d3ddm.Format;

	if(d3ddm.Format == D3DFMT_X8R8G8B8)
	{
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
		TextureFormat = D3DFMT_A8R8G8B8;
	}
	else
	{
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		TextureFormat = D3DFMT_A4R4G4B4;
	}


	// ��ǿ����Ӳ�����㷽ʽ�����豸��Ϊ�˲�©��MX440���ֲ�֧��VS��֧��T&L���Կ���XIXI��
	UserVertexShaderProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	HRESULT hr = d3d->CreateDevice( D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL, hWnd, UserVertexShaderProcessing, &d3dpp, &d3ddevice);
	// ���ִ����ʾ���㴦��ʽ����Ӳ��֧�֣�ǿ���������ʽ�����豸��Ϊ�˼��ݲ�֧��T&L��DX7�Կ�������ĳЩ�����Կ���
	if(hr == D3DERR_INVALIDCALL)
	{
		mymessage("���棺�Կ���֧��Ӳ�����㴦��ǿ���������ʽ�����豸�����ܻἱ���½�������");
		UserVertexShaderProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		hr = d3d->CreateDevice( D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL, hWnd, UserVertexShaderProcessing, &d3dpp, &d3ddevice);
	}
	// ȫ��ʧ�ܣ�������ɣ�
	if(FAILED(hr))
	{
		char szErrorInfo[512] = "";
		sprintf(szErrorInfo, "����D3D�豸ʧ�ܣ�����ϵͳ��Ӳ�����ã������Կ����������DirectX�Ƿ�װ��ȷ�������%x", hr);
		mymessage(szErrorInfo);
		return E_FAIL;
	}

	// Shader�汾���
	char pVSVersion[7] = "", pPSVersion[7] = "";
	char pNeedVSVersion[7] = USE_VSVERSION, pNeedPSVersion[7] = USE_PSVERSION;
	char szErrorInfo[100] = "";

#ifdef USE_PIXELSHADER
	// �ж�PS�汾
	memcpy(pPSVersion, D3DXGetPixelShaderProfile(d3ddevice), 7);
	// 2.x��'b'��Ȼ��ĸ��ֵ����'a'��������֧�ֶ���������С��a�ģ���������ǿ�ư�����ΪС��'a'���Ա����Ƚ�
	if(pPSVersion[3] == '2' && pPSVersion[5] == 'b')
		pPSVersion[5] = 'a'-1;
	if(pNeedPSVersion[3] == '2' && pNeedPSVersion[5] == 'b')
		pNeedPSVersion[5] = 'a'-1;
	if(pPSVersion[3]<pNeedPSVersion[3] || pPSVersion[3]==pNeedPSVersion[3]&&pPSVersion[5]<pNeedPSVersion[5])
	{
		sprintf(szErrorInfo, "��֧��Pixel Shader %c.%c��", pNeedPSVersion[3], pNeedPSVersion[5]);
		mymessage(szErrorInfo);	return E_FAIL;
	}
	
	// �ж�PS2.0x�������Ƿ���ϳ������С����
	if(!strcmp(USE_PSVERSION, "ps_2_a") || !strcmp(USE_PSVERSION, "ps_2_b") || !strcmp(USE_PSVERSION, "ps_2_x"))
	{
		BOOL bFlowControl = FALSE;
#ifdef PS2x_USE_FLOWCONTROL
		bFlowControl = TRUE;
#endif

		if(!CheckPS2xSupport(PS2x_USE_MAXNUM_TEMP, bFlowControl))
		{
			sprintf(szErrorInfo, "������֧��Pixel Shader 2.x���޷�ִ�г���");
			mymessage(szErrorInfo);	return E_FAIL;
		}
	}


#endif USE_PIXELSHADER

#ifdef USE_VERTEXSHADER
	BOOL bSoftwareVertexProcessing = FALSE;
	// ��ȫ��֧�ֵģ�Ҫ�����汾��⣬����D3DXGetVertexShaderProfile�᷵�ؿ�ָ����ɷǷ�����
	if((d3dcaps.VertexShaderVersion&0xffff) == 0) 
	{
		bSoftwareVertexProcessing = TRUE;
	}
	else
	{
		// �ж�VS�汾
		memcpy(pVSVersion, D3DXGetVertexShaderProfile(d3ddevice), 7);
		if(pVSVersion[3]<pNeedVSVersion[3] || pVSVersion[3]==pNeedVSVersion[3]&&pVSVersion[5]<pNeedVSVersion[5])
		{
			bSoftwareVertexProcessing = TRUE;
		}
		// �ж�VS2.0x�������Ƿ���ϳ������С����
		if(!strcmp(USE_VSVERSION, "vs_2_a") || !strcmp(USE_VSVERSION, "vs_2_b") || !strcmp(USE_VSVERSION, "vs_2_x"))
		{
#ifdef VS2a_USE_MAXNUM_TEMP
			if(d3dcaps.VS20Caps.NumTemps < VS2x_USE_MAXNUM_TEMP)
			{
				bSoftwareVertexProcessing = TRUE;
			}
#endif
		}
	}

	// ����ղ�����Ӳ�����㴴���豸�ģ�����������Ҫǿ������������Ӳ����֧�ֵ�VS����ô�ͷ��豸�����´���
	if(bSoftwareVertexProcessing && UserVertexShaderProcessing == D3DCREATE_HARDWARE_VERTEXPROCESSING)
	{
		// �����֧��ָ���汾��Vertex Shader����ǿ�ƴ������㴦���豸
		sprintf(szErrorInfo, "���棺�Կ���֧��Vertex Shader %c.%c��ǿ���������ʽ�����豸�����ܻἱ���½�������", pNeedVSVersion[3], pNeedVSVersion[5]);
		mymessage(szErrorInfo);
		SAFE_RELEASE(d3ddevice);
		hr = d3d->CreateDevice( D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddevice);
		if(FAILED(hr))
		{
			char szErrorInfo[512] = "";
			sprintf(szErrorInfo, "����D3D�豸ʧ�ܣ�����ϵͳ��Ӳ�����ã������Կ����������DirectX�Ƿ�װ��ȷ�������%x", hr);
			mymessage(szErrorInfo);
			return E_FAIL;
		}
	}

#endif



	//��ʼ���������֮���Է��������Ϊ���豸��ʧ�󲻻������õ���ʼλ��
	D3DXVECTOR3 PtLookAt = g_PtCameraInitPos + g_VecCameraInitDir;
	CameraChange.InitCamera(g_PtCameraInitPos.x, g_PtCameraInitPos.y, g_PtCameraInitPos.z,  PtLookAt.x, PtLookAt.y, PtLookAt.z,  0.0f,1.0f,0.0f, 7,7,90);

	//�õ�Ӳ����ȷ��ʱ����Ƶ��
	QueryPerformanceFrequency(&PerformanceFrequency);

	return S_OK;
}


//��ѯ�õ������ԣ�����֧�ֵģ���RESULT��EFAIL��ֻ����ʾ�ľͲ������ˡ�
HRESULT DetectDeviceCaps()
{
	HRESULT Result=S_OK;
	//����֧�ֵ�����
	if(d3dcaps.MaxTextureBlendStages<4) {mymessage("֧�ֵ������ϲ�̫�٣�"); Result=E_FAIL;}
	if(d3dcaps.MaxSimultaneousTextures<2) {mymessage("֧�ֵ�������ˮ��̫�٣�");Result=E_FAIL;}
	if(DesktopDisplayMode.Format == D3DFMT_R5G6B5) mymessage("���齫������ʾģʽ����Ϊ32λ���ɫ���������л�������ģʽʱ���ܻ�������⣡")

//��������
#ifdef USE_CUBEMAPENVIRONMENT
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, d3ddm.Format)))	{mymessage("��֧��RENDERTARGET�����ʽ"); Result=E_FAIL;}
	if((d3dcaps.TextureCaps&D3DPTEXTURECAPS_CUBEMAP)==0) {mymessage("��֧����������"); Result=E_FAIL;}
#endif USE_CUBEMAPENVIRONMENT
#ifdef USE_CUBEMAP
	if((d3dcaps.TextureCaps&D3DPTEXTURECAPS_CUBEMAP)==0) {mymessage("��֧����������"); Result=E_FAIL;}
#endif USE_CUBEMAP

//��Ⱦ������
#ifdef USE_RENDERTOTEXTURE
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, d3ddm.Format)))	{mymessage("��֧��RENDERTARGET�����ʽ"); Result=E_FAIL;}
#endif USE_RENDERTOTEXTURE

//���ع���
#ifdef USE_DOTPRODUCT3
	if((d3dcaps.TextureOpCaps&D3DTEXOPCAPS_DOTPRODUCT3)==0) {mymessage("��֧��DOT3�ڻ������ϣ�"); Result=E_FAIL;}
	if((d3dcaps.SrcBlendCaps&D3DPBLENDCAPS_DESTCOLOR)==0) {mymessage("��֧��ԴCOLOR���������ϣ�"); Result=E_FAIL;}
	if((d3dcaps.DestBlendCaps&D3DPBLENDCAPS_SRCCOLOR)==0) {mymessage("��֧��Ŀ��COLOR���������ϣ�"); Result=E_FAIL;}
#endif USE_DOTPRODUCT3

	//�Ǳ���֧�ֵ����ԣ�������Ҫ������ʾ�Ի��򣬺ܿ�����XP��������

//PixelShader��Vertex Shader�Ļ�ǿ����������㴦�������
#ifdef USE_PIXELSHADER
	// һ��ҪԤ���������жϣ����Ӳ����֧��shader����GetShaderProfile���᷵�ؿ�ָ�룬��ɺ���shader�汾���Ƿ�����
	if((d3dcaps.PixelShaderVersion&0xffff) == 0) {mymessage("��֧��Pixel Shader��"); Result=E_FAIL;}
#endif USE_PIXELSHADER

//��͹����ӳ��
#ifdef USE_EMBM
	if((d3dcaps.TextureOpCaps&(D3DTEXOPCAPS_BUMPENVMAP|D3DTEXOPCAPS_BUMPENVMAPLUMINANCE))==0) {mymessage("ע�⣺�Կ���֧�ְ�͹����ӳ�䣬���������ͨ����ӳ�䣬�������ή�ͻ���������");}
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_0, D3DRTYPE_TEXTURE, D3DFMT_V8U8))) {mymessage("��֧�ְ�͹����ӳ��V8U8��ʽ"); Result=E_FAIL;}
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_0, D3DRTYPE_TEXTURE, D3DFMT_X8L8V8U8))) {mymessage("��֧�ְ�͹����ӳ��X8L8V8U8��ʽ"); Result=E_FAIL;}
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_0, D3DRTYPE_TEXTURE, D3DFMT_L6V5U5))) {mymessage("��֧�ְ�͹����ӳ��L6V6U6��ʽ"); Result=E_FAIL;}
#endif USE_EMBM


// ��������
#ifdef USE_FP16 
	bool bHDRFP16 = true;
	// ��������һ�㶼����ΪRTʹ�õģ�����MRT��HDR
	// GF6/7��֧��R16F��ֻ֧��G16R16F����������Ͳ����R16F�ˣ���ΪGF6/7����֧��
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_G16R16F)))
		bHDRFP16 = false;
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F)))
		bHDRFP16 = false;
	if(!bHDRFP16)
	{
		mymessage("��֧��FP16�뾫�ȸ�������HDR����");
		Result = E_FAIL;
	}
#endif
	
#ifdef USE_FP32
	bool bHDRFP32 = true;
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_R32F)))
		bHDRFP32 = false;
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A32B32G32R32F)))
		bHDRFP32 = false;
	if(!bHDRFP32)
	{
		mymessage("��֧��FP32ȫ���ȸ�������HDR����");
		Result = E_FAIL;
	}
#endif



// ��������ȡ��
#ifdef USE_VTF
	bool bVTF = true;
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE, D3DFMT_R32F)))
		bVTF = false;
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE, D3DFMT_A32B32G32R32F)))
		bVTF = false;
	if(!bVTF)
	{
		mymessage("��֧�ֶ�������ȡ����VTF����");
		Result = E_FAIL;
	}
#endif


	//����
	if(d3dcaps.NumSimultaneousRTs < USE_MAXMRTNUM)
	{
		char szErrorInfo[50];
		int iNum = USE_MAXMRTNUM;
		sprintf(szErrorInfo, "Ӳ����֧��%d��MRT��", iNum);
		mymessage(szErrorInfo);
		Result = E_FAIL;
	}

#ifdef USE_DYNAMICTEXTURE
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_DYNAMIC, D3DRTYPE_TEXTURE, d3ddm.Format)))	{mymessage("��֧�ֶ�̬�����ʽ"); Result=E_FAIL;}
#endif USE_DYNAMICTEXTURE

	return Result;
}


//�ͷ�������Դ
void ReleaseD3D()
{
	//�������ͷ����е������ģ��
	ReleaseMyScene();
	//�ͷ�D3D��D3D�豸
	if(d3ddevice!=NULL)
		d3ddevice->Release();
	if(d3d!=NULL)
		d3d->Release();

	// �ͷ�DirectInput�ӿ�
	if(g_pDIKey)
		g_pDIKey->Unacquire();
	if(g_pDIMouse)
		g_pDIMouse->Unacquire();
	SAFE_RELEASE(g_pDIKey);
	SAFE_RELEASE(g_pDIMouse);
	SAFE_RELEASE(g_pDI);

	// �ͷ�DirectSound�ӿ�

	return;
}

//��ΪҪ����InitMyScene��������UINT������ֵ������0��ʾ����
UINT ResetDevice()
{
	// ��Ȼֻ��Ҫ�õ���֮ǰReleaseһ�μ��ɣ����ò���������ʱ�����඼�б�����ʩ���ظ�ReleaseҲ��������⣨�Զ����أ�
	ReleaseMyScene();
	
	d3dpp.Windowed = WindowMode;
	d3dpp.BackBufferWidth = d3ddm.Width;
	d3dpp.BackBufferHeight = d3ddm.Height;
	//��Ϊ����ģʽ����ȡ�������ɫ����ȡ��ǰ���ڵĴ�С��λ�ã�������
	if(WindowMode)
	{
		d3ddm.Format = DesktopDisplayMode.Format;
		d3dpp.BackBufferWidth = WindowWidth;
		d3dpp.BackBufferHeight = WindowHeight;
		MoveWindow(hWnd, WindowX, WindowY, WindowWidth, WindowHeight, TRUE);
	}

	d3dpp.BackBufferFormat = d3ddm.Format;
	//������ʾģʽ��ȷ��������������ģ�建������ظ�ʽ
	if(d3ddm.Format == D3DFMT_X8R8G8B8)
	{
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
		TextureFormat = D3DFMT_A8R8G8B8;
	}
	else
	{
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		TextureFormat = D3DFMT_A4R4G4B4;
	}

	UINT hr = FAILED(d3ddevice->Reset(&d3dpp))?1:0;
	
	if(hr) return hr;

	// �����Ի���֮ǰҲҪ���øú�����������ֻ��Ҫ�õ���֮��Initһ�μ��ɣ����ⵯ��֮ǰҲInit��û��Ҫ����ʱ
	if(!DialogSign)
	{
		hr = InitMyScene();
	}

	CameraChange.ViewTransform();

	return hr;
}