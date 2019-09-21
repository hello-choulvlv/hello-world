/*
  *DirectX Texture
  *2018Äê3ÔÂ13ÈÕ
  *@author:xiaohuaxiong
 */
#ifndef __TEXTURE_H__
#define __TEXTURE_H__
#include "Scene.h"
#include <d3d10.h>
#include <d3dx10.h>
class Texture :public Scene
{
	ID3D10Device    *_device;
	//ResourceView
	ID3D10ShaderResourceView    *_textureView;
	ID3D10ShaderResourceView    *_specularView;
	//buffer
	ID3D10Buffer                               *_vertexBuffer;
	ID3D10Buffer                               *_indexBuffer;
	int                                                    _vertexCount, _indexCount;
	//effect
	ID3D10Effect                                *_lightEffect;
	ID3D10EffectTechnique             *_lightEffectTech;
	ID3D10EffectMatrixVariable    *v_modelMatrix;
	ID3D10EffectMatrixVariable    *v_viewProjMatrix;
	ID3D10EffectMatrixVariable    *v_normalMatrix;
	ID3D10EffectVectorVariable    *v_lightDirection;
	ID3D10EffectShaderResourceVariable   *v_texture;
	ID3D10EffectShaderResourceVariable   *v_specular;
	//layout
	ID3D10InputLayout                     *_lightInputLayout;
	//
	D3DXMATRIX                               _modelMatrix,_viewProjMatrix;
	D3DXVECTOR3                            _lightDirection;
	//
	float                                                   _deltaTime;
public:
	Texture();
	void        init();
	//
	void        initEffect();
	//
	void        initLayout();
	//
	void       initTexture();
	//
	void      initBuffer();
	//
	virtual   void update(float dt);
	//
	virtual  void  render();
};
#endif