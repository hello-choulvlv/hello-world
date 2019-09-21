/*
  *天空盒实现DirectX11
  *2018年3月31日
  *@author:xiaohuaxiong
 */
#ifndef __SKYBOX_H__
#define __SKYBOX_H__
#include "common.h"
#include "Camera.h"
#include<d3dx11.h>
#include<d3dx10math.h>
class Skybox
{
	ID3D11Device   *_device;
	ID3D11DeviceContext   *_deviceContext;
	ID3D11VertexShader  *_vertexShader;
	ID3D11PixelShader     *_pixelShader;
	ID3D11Buffer               *_vertexBuffer,*_cbPerFrameBuffer;
	ID3D11InputLayout    *_inputLayout;
	//
	//ID3D11Texture2D       *_skyboxCubeTexture;
	ID3D11ShaderResourceView *_skyboxCubeView;
	ID3D11SamplerState   *_samplerState;
	ID3D11DepthStencilState  *_depthStencilState;
	//
	D3DXMATRIX               _mvpMatrix;
public:
	Skybox();
	~Skybox();
	void                     initSkybox(ID3D11Device *device,ID3D11DeviceContext *deviceContext);
	static     Skybox *create(ID3D11Device *device, ID3D11DeviceContext *deviceContext);
	//
	void                     onUpdate(float  deltaTime,Camera &camera);
	//
	void                     onRender(Camera &camera);
};
#endif