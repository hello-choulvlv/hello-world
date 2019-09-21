/*
  *箱子/立方体
  *@author:xiaohuaxiong
  *2018年3月1日
 */
#ifndef __CUBE_H__
#define __CUBE_H__
#include "Scene.h"
#include<d3d10.h>
#include<d3dx10.h>
#include"Box.h"
class Cube :public Scene
{
	//顶点缓冲区对象
	ID3D10Buffer    *_vertexBuffer;
	ID3D10Buffer    *_indexBuffer;
	//顶点数目,索引数目
	int                           _vertexCount, _indexCount;
	ID3D10Effect       *_fxEffect;
	ID3D10EffectTechnique  *_effectTechnique;
	ID3D10EffectMatrixVariable  *_mvpMatrixV;
	ID3D10InputLayout                 *_vertexLayout;
	//Matrix
	D3DXMATRIX                         _projMatrix;
	D3DXMATRIX                         _viewMatrix;
	D3DXMATRIX                         _modelMatrix;
	D3DXMATRIX                         _mvpMatrix;
	Box                                              _box;
public:
	Cube();
	~Cube();
	void   init();
	virtual void update(float dt);
	virtual void render();
};
#endif