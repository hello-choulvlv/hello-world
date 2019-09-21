//���ļ��Ƿ�װD3D����ͷ�ļ�������CPP�ļ�ֻ��Ҫ���������ɣ���Ϊ���������������ȫ����Ҫ��ͷ�ļ����궨��ͱ���
//��MYD3D.CPP�ж����ȫ�ֱ����ڴ��ض���EXTERN
//������Ҫ�ڴ˶��塢˵��
//������ͷ�ļ�ֻ��ҪINCLUDE���������H�ļ�����
//ֻ����ʾ��Ϣ�ģ�δ����D3D�豸ǰ������mymessage(��Ϣ�ִ�)����������Ҫʹ��myfail(�����)��֮��Ӳ��ӷֺŶ�����ν


/*********************�Զ������Ϣ�͹̶�ֵ***************/
#define MYD3D_MODULETEXTURENUM 8	//�������ɴ�����������
#define D3DUSAGE_0 0

#define MYD3D_FILEERROR 1			//�ļ���ȡ����
#define MYD3D_OK 0

#define DIRECTINPUT_VERSION	0x0800	//ȥ�����˵ı�����ʾ



/*********************����ͷ�ļ�********************/
#include <math.h>
#include <stdio.h>
#include "include/d3d9.h"
#include "include/d3dx9math.h"
#include "include/dxfile.h"
#include "include/rmxfguid.h"
#include "include/dinput.h"
#include "include/dsound.h"
#include "base.h"
#include "Color.h"
#include "CameraChange.h"
#include "test.h"

/*********************����ȫ�ֱ���*****************/
#define FPSUPDATETIME 1000                 //֡������ʱ�䣬��λ����
extern char convstr[20];

extern HWND hWnd;
extern LPDIRECT3D9 d3d;
extern LPDIRECT3DDEVICE9 d3ddevice;
extern LPDIRECTINPUT8 g_pDI;
extern LPDIRECTINPUTDEVICE8 g_pDIKey;
extern LPDIRECTINPUTDEVICE8 g_pDIMouse;
extern D3DCAPS9 d3dcaps;

extern D3DDISPLAYMODE d3ddm, DesktopDisplayMode;
extern D3DPRESENT_PARAMETERS d3dpp;
extern D3DFORMAT TextureFormat;
extern UINT WindowX, WindowY, WindowWidth, WindowHeight;

extern BOOL WindowMode, ChangeDisplayModeSign;  ////�л���ʾģʽ
extern UINT DialogSign;        //�����Ի�����
extern BOOL BeforeDlgChangeModeSign;


extern D3DXMATRIX myview, myproj;
extern CAMERACHANGE CameraChange;
extern int effectenable, AltEnterSign;
extern float curfps;
extern LARGE_INTEGER PerformanceFrequency;

// �������ʼλ�úͷ���ʵ����test.cpp��
extern D3DXVECTOR3 g_PtCameraInitPos, g_VecCameraInitDir;



/**********************��������*********************/
#define SAFE_RELEASE(p) {if(p) p->Release(); p=NULL;}
#define SAFE_DELETE(p) {if(p) delete p; p=NULL;}
#define SAFE_DELETE_ARRAY(p) {if(p) delete[] p; p=NULL;}

#define V_RETURN(express) {if(FAILED(express)) return E_FAIL;}
#define V_RETURN_MessageBox(express, message) {if(FAILED(express)) {MessageBox(hWnd, message, "Fatal Error", MB_OK); return E_FAIL;}}
#define HR_RETURN(express) {if(hr = FAILED(express)) return hr;}

#define angtoarg(angle) angle*D3DX_PI/180  //�Ƕȱ仡��
#define argtoang(arg) angle*180/D3DX_PI   //���ȱ�Ƕ�

#define pointtovector(ax,ay,az,bx,by,bz,vx,vy,vz) {vx=bx-ax;vy=by-ay;vz=bz-az;}  //����ȷ����ֱ��ת��Ϊ��������

#define ftodw(var) *(DWORD*)&var      //float to dwordָ�룬�ܶ�TSS��RS��������


#define mymessage(LPSTR)	MessageBox(hWnd,LPSTR,"Fatal Error",MB_OK);

#define SAFE_RELEASE(p) {if(p) p->Release(); p=NULL;}

//��ָ��DEVICE�ĵ�i��������OP��������ARG1��ARG2���
#define SetTextureColorMix(i, op, arg1, arg2);       \
	d3ddevice->SetTextureStageState(i, D3DTSS_COLOROP, op); \
	d3ddevice->SetTextureStageState(i, D3DTSS_COLORARG1, arg1); \
	d3ddevice->SetTextureStageState(i, D3DTSS_COLORARG2, arg2); \
	
#define SetTextureAlphaMix(i, op, arg1, arg2);       \
	d3ddevice->SetTextureStageState(i, D3DTSS_ALPHAOP, op); \
	d3ddevice->SetTextureStageState(i, D3DTSS_ALPHAARG1, arg1); \
	d3ddevice->SetTextureStageState(i, D3DTSS_ALPHAARG2, arg2); \
	
#ifdef USE_DEBUG
	#undef MYERR_NOTEXTUREFILE
	#undef MYERR_NOMESHFILE
	#undef MYERR_CREATEVERTEXBUFFER
	#undef MYERR_CREATECUBEMAP
	#undef MYERR_CREATERENDERTOTEXTURE
	#undef MYERR_CREATEBUMPMAP
	#undef MYERR_CREATENORMALMAP
	#undef MYERR_CREATEVERTEXSHADER
	#undef MYERR_CREATEPIXELSHADER
	#undef MYERR_CREATEATTENUATIONMAP
	#undef MYERR_CREATEANISOTROPYMAP
	#undef MYERR_CREATEANISODIRMAP
	#undef MYERR_CREATESPECULARMAP
	#define MYERR_NOTEXTUREFILE "��ȡ�������"
	#define MYERR_NOMESHFILE     "��ȡģ�ʹ���"
	#define MYERR_CREATEVERTEXBUFFER "��ʼ�����㻺���"
	#define MYERR_CREATECUBEMAP     "��ʼ�����������"
	#define MYERR_CREATERENDERTOTEXTURE "��ʼ����Ⱦ�������"
	#define MYERR_CREATEBUMPMAP     "��ʼ����͹��ͼ��"
	#define MYERR_CREATENORMALMAP   "��ʼ����������ͼ��"
	#define MYERR_CREATEVERTEXSHADER "��ʼ��������ɫ������"
	#define MYERR_CREATEPIXELSHADER "��ʼ��������ɫ������"
	#define MYERR_CREATEATTENUATIONMAP "��ʼ��˥�������"
	#define MYERR_CREATEANISOTROPYMAP "��ʼ���������Թ��������"
	#define MYERR_CREATEANISODIRMAP "��ʼ���������Թ��շ��������"
	#define MYERR_CREATESPECULARMAP "��ʼ���߹�ǿ����ͼ��"
	#define MYERR_CREATEFONT		"�����������"
	#define myfail(INFOSTR)	MessageBox(hWnd, INFOSTR, "Fatal Error", MB_OK);
#else
	#define MYERR_NOTEXTUREFILE             1
	#define MYERR_NOMESHFILE                2
	#define MYERR_CREATEVERTEXBUFFER        3
	#define MYERR_CREATECUBEMAP             4
	#define MYERR_CREATERENDERTOTEXTURE     5
	#define MYERR_CREATEBUMPMAP             6
	#define MYERR_CREATENORMALMAP           7
	#define MYERR_CREATEVERTEXSHADER        8
	#define MYERR_CREATEPIXELSHADER         9
	#define MYERR_CREATEATTENUATIONMAP      10
	#define MYERR_CREATEANISOTROPYMAP       11
	#define MYERR_CREATEANISODIRMAP         12
	#define MYERR_CREATESPECULARMAP         13
	#define MYERR_CREATEFONT				14
	#define myfail(FailID) return FailID;
#endif
