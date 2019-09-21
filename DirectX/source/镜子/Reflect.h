/*
  *空间镜像实现/模板缓冲区
  *2018/3/16
  *@author:xiaohuaxiong
 */
#ifndef __REFLECT_H__
#define __REFLECT_H__
#include "engine/Scene.h"
#include "engine/Camera.h"
#include "engine/MouseEventListener.h"
#include "engine/KeyEventListener.h"
#include<d3d10.h>
#include<d3dx10.h>
class Reflect : public Scene
{
	//顶点缓冲区对象/箱子
	ID3D10Buffer     *_vertexBufferBox;
	ID3D10Buffer     *_indexBufferBox;
	int                          _vertexCountBox, _indexCountBox;
	//墙体
	ID3D10Buffer      *_vertexBufferWall;
	int                            _vertexCountWall;
	//镜子
	ID3D10Buffer     *_vertexBufferReflect;
	int                           _vertexCountReflect;
	//
	ID3D10Effect      *_effect;
	ID3D10EffectTechnique          *_effectTech;
	ID3D10InputLayout                 *_lightInputLayoutBox;
	ID3D10InputLayout                 *_lightInputLayoutWall;
	ID3D10EffectMatrixVariable  *v_modelMatrix;
	ID3D10EffectMatrixVariable  *v_viewProjMatrix;
	ID3D10EffectVectorVariable   *v_lightDirection;
	ID3D10EffectShaderResourceVariable *v_baseTexture;
	//纹理
	ID3D10ShaderResourceView      *_textureWall,*_textureReflect,*_textureMesh,*_textureBox;
	//光栅化对象
	ID3D10RasterizerState                *_rasterizerState;
	//深度模版测试对象
	ID3D10DepthStencilState          *_depthStencilNormal,*_depthStencilReflect;
	//颜色混溶对象
	ID3D10BlendState                       *_blendStateReflect;
	//
	D3DXMATRIX       _modelMatrix;
	D3DXMATRIX       _viewProjMatrix;
	D3DXPLANE          _plane;
	D3DXVECTOR3	   _lightDirection;
	//
	Camera                    *_camera;
	TouchEventListener  *_touchListener;
	Vec2                            _offsetVec2;
	KeyEventListener    *_keyListener;
	int                                 _keyMask;
public:
	Reflect();
	~Reflect();

	void        init();
	//初始化顶点缓冲区对象
	void        initBuffer();
	//effect
	void        initEffect();
	//纹理
	void        initTexture();
	//矩阵
	void        initMatrix();
	//
	void        initInputLayout();
	//光栅化对象
	void        initRasterizerState();
	//深度模版测试对象
	void        initDepthStencilState();
	//颜色混溶对象
	void        initBlendState();
	//update
	void        update(float dt);
	//render
	void       render();
	//event//
	bool       onTouchPressed(const Vec2 &touchPoint);
	void       onTouchMoved(const Vec2 &touchPoint);
	void       onTouchReleased(const Vec2 &touchPoint);
	//
	void       onKeyPressed(KeyCode keyCode);
	void       onKeyReleased(KeyCode keyCode);
};

#endif