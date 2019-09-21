/*
  *DirectX-����ʱ������ȹ���
  *@author:xiaohuaxiong
  *@2018��2��28��
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
	//Ӧ�ó��������
	HINSTANCE                          _appInstance;
	//���ڵ�����
	HWND                                     _windowHandler;
	//����������һ��DirectX���������������
	ID3D10Device                       *_d3dDevice;
	//������
	IDXGISwapChain                 *_swapChain;
	ID3D10Texture2D                *_depthStencilTexture;
	ID3D10RenderTargetView  *_renderTargetView;
	ID3D10DepthStencilView    *_depthStencilView;
	ID3DX10Font                         *_font;
	D3DXCOLOR                           _clearColor;
	//ʹ�õ������豸������
	D3D10_DRIVER_TYPE       _driverType;
	//���ڵĴ�С
	int                                              _windowWidth, _windowHeight;
	Size                                            _winSize;
	std::vector<Scene*>             _sceneVector;
private:
	DirectEngine();
	DirectEngine(const DirectEngine &);
	//�˺�������main.cpp�е�WinMain�����б�����
	void                init(HINSTANCE hInstance,HWND windowInstance,int windowWidth,int windowHeight);
public:
	~DirectEngine();
	static DirectEngine *getInstance();
	//��ȡӦ�ó��������
	HINSTANCE            getAppInstance()const { return _appInstance; };
	//��ȡ���ڵ�����
	HWND						getWindowHandler()const { return _windowHandler; };
	//��ȡ�豸
	ID3D10Device        *getDevice()const { return _d3dDevice; };
	//����˫����
	void                           swapBuffers();
	//�ָ�ԭ����Ĭ����Ⱦ����
	void                           restorRenderTarget();
	//�������������
	void                           clear();
	//���ó��������ڻص�����
	void                          update(float dt);
	void                          render();
	//���ٴ������еĶ���
	void                          destroy();
	//
	void                          addScene(Scene *scene);
	//
	const Size& getWinSize()const { return _winSize; };
};
#endif