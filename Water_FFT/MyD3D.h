//此文件是封装D3D的主头文件，所有CPP文件只需要包含它即可，因为它里面包含了所有全局需要的头文件、宏定义和变量
//在MYD3D.CPP中定义的全局变量在此重定义EXTERN
//函数不要在此定义、说明
//其他的头文件只需要INCLUDE自身所需的H文件即可
//只是提示信息的（未创建D3D设备前），用mymessage(信息字串)，致命错误要使用myfail(错误宏)，之后加不加分号都无所谓


/*********************自定义的信息和固定值***************/
#define MYD3D_MODULETEXTURENUM 8	//物体最大可创建纹理总数
#define D3DUSAGE_0 0

#define MYD3D_FILEERROR 1			//文件读取错误
#define MYD3D_OK 0

#define DIRECTINPUT_VERSION	0x0800	//去掉烦人的编译提示



/*********************公共头文件********************/
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

/*********************公共全局变量*****************/
#define FPSUPDATETIME 1000                 //帧数更新时间，单位毫秒
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

extern BOOL WindowMode, ChangeDisplayModeSign;  ////切换显示模式
extern UINT DialogSign;        //弹出对话框标记
extern BOOL BeforeDlgChangeModeSign;


extern D3DXMATRIX myview, myproj;
extern CAMERACHANGE CameraChange;
extern int effectenable, AltEnterSign;
extern float curfps;
extern LARGE_INTEGER PerformanceFrequency;

// 摄像机初始位置和方向，实体在test.cpp中
extern D3DXVECTOR3 g_PtCameraInitPos, g_VecCameraInitDir;



/**********************公共函数*********************/
#define SAFE_RELEASE(p) {if(p) p->Release(); p=NULL;}
#define SAFE_DELETE(p) {if(p) delete p; p=NULL;}
#define SAFE_DELETE_ARRAY(p) {if(p) delete[] p; p=NULL;}

#define V_RETURN(express) {if(FAILED(express)) return E_FAIL;}
#define V_RETURN_MessageBox(express, message) {if(FAILED(express)) {MessageBox(hWnd, message, "Fatal Error", MB_OK); return E_FAIL;}}
#define HR_RETURN(express) {if(hr = FAILED(express)) return hr;}

#define angtoarg(angle) angle*D3DX_PI/180  //角度变弧度
#define argtoang(arg) angle*180/D3DX_PI   //弧度变角度

#define pointtovector(ax,ay,az,bx,by,bz,vx,vy,vz) {vx=bx-ax;vy=by-ay;vz=bz-az;}  //两点确定的直线转换为方向向量

#define ftodw(var) *(DWORD*)&var      //float to dword指针，很多TSS或RS参数中用


#define mymessage(LPSTR)	MessageBox(hWnd,LPSTR,"Fatal Error",MB_OK);

#define SAFE_RELEASE(p) {if(p) p->Release(); p=NULL;}

//将指定DEVICE的第i层纹理用OP方法，把ARG1和ARG2混合
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
	#define MYERR_NOTEXTUREFILE "读取纹理错误！"
	#define MYERR_NOMESHFILE     "读取模型错误！"
	#define MYERR_CREATEVERTEXBUFFER "初始化顶点缓冲错！"
	#define MYERR_CREATECUBEMAP     "初始化立方纹理错！"
	#define MYERR_CREATERENDERTOTEXTURE "初始化渲染到纹理错！"
	#define MYERR_CREATEBUMPMAP     "初始化凹凸贴图错！"
	#define MYERR_CREATENORMALMAP   "初始化法向量贴图错！"
	#define MYERR_CREATEVERTEXSHADER "初始化顶点着色器出错！"
	#define MYERR_CREATEPIXELSHADER "初始化像素着色器出错！"
	#define MYERR_CREATEATTENUATIONMAP "初始化衰减纹理错！"
	#define MYERR_CREATEANISOTROPYMAP "初始化各向异性光照纹理错！"
	#define MYERR_CREATEANISODIRMAP "初始化各向异性光照方向纹理错！"
	#define MYERR_CREATESPECULARMAP "初始化高光强度贴图错！"
	#define MYERR_CREATEFONT		"创建字体错误！"
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
