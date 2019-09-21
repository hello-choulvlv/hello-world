/*
  *渲染到纹理
  *@author:xiaohuaxiong
  *@2018年3月19日
 */
#ifndef __RENDER_TARGET_H__
#define __RENDER_TARGET_H__
#include "engine/Scene.h"
#include "engine/Camera.h"
#include "d3d10.h"
#include "d3dx10.h"
class RenderTarget : public Scene
{
	//Shader View Resource
	D3D10_VIEWPORT                    _viewPort,_oldViewPort;
	D3DXCOLOR                                _clearColor;
	ID3D10ShaderResourceView    *_colorShaderView;
	ID3D10RenderTargetView         *_renderView;
	//
	ID3D10ShaderResourceView    *_depthShaderView;
	ID3D10DepthStencilView          *_depthView;
	ID3D10ShaderResourceView    *_texture;
	//
	ID3D10Buffer                               *_vertexBuffer, *_indexBuffer;
	int                                                     _indexCount;
	ID3D10Buffer                               *_quadBuffer;
	//
	ID3D10Effect                                *_effect;
	ID3D10EffectTechnique             *_effectTech;
	ID3D10InputLayout                    *_inputLayout,*_quadInputLayout;
	ID3D10EffectMatrixVariable    *v_mvpMatrix;
	ID3D10EffectShaderResourceVariable  *v_baseTexture;
	//
	D3DXMATRIX                             _modelMatrix;
	Camera                                          *_camera;
public:
	RenderTarget();
	~RenderTarget();
	void             init(int w,int h);
	void             initBuffer();
	void             initEffect();
	void             initInputLayout();
	void             initRenderTargetView();
	//
	void             update(float dt);
	void             render();
};
#endif