/*
  *DirectX-运行时引擎调度管理
  *@author:xiaohuaxiong
  *@2018年2月28日
 */
#ifndef __DIRECT_ENGINE_H__
#define __DIRECT_ENGINE_H__
#include<d3d10.h>
#include<d3dx10.h>
#include<dxerr.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include<windows.h>
#include<vector>
#include "Scene.h"
#include "Geometry.h"
class DirectEngine
{
	friend int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nShowCmd);
	//应用程序的引用
	HINSTANCE                          _appInstance;
	//窗口的引用
	HWND                                     _windowHandler;
	//以下是运行一个DirectX程序所必需的数据
	ID3D10Device                       *_d3dDevice;
	//交换链
	IDXGISwapChain                 *_swapChain;
	ID3D10Texture2D                *_depthStencilTexture;
	ID3D10RenderTargetView  *_renderTargetView;
	ID3D10DepthStencilView    *_depthStencilView;
	ID3DX10Font                         *_font;
	D3DXCOLOR                           _clearColor;
	//使用的驱动设备的类型
	D3D10_DRIVER_TYPE       _driverType;
	//窗口的大小
	int                                              _windowWidth, _windowHeight;
	Size                                            _winSize;
	std::vector<Scene*>             _sceneVector;
private:
	DirectEngine();
	DirectEngine(const DirectEngine &);
	//此函数会在main.cpp中的WinMain函数中被调用
	void                init(HINSTANCE hInstance,HWND windowInstance,int windowWidth,int windowHeight);
public:
	~DirectEngine();
	static DirectEngine *getInstance();
	//获取应用程序的引用
	HINSTANCE            getAppInstance()const { return _appInstance; };
	//获取窗口的引用
	HWND						getWindowHandler()const { return _windowHandler; };
	//获取设备
	ID3D10Device        *getDevice()const { return _d3dDevice; };
	//交换双缓冲
	void                           swapBuffers();
	//恢复原来的默认渲染对象
	void                           restorRenderTarget();
	//清除缓冲区对象
	void                           clear();
	//调用场景的周期回掉函数
	void                          update(float dt);
	void                          render();
	//销毁从外界持有的对象
	void                          destroy();
	//
	void                          addScene(Scene *scene);
	//
	const Size& getWinSize()const { return _winSize; };
};
#endif