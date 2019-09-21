/*
  *水渲染,DirectX实现
  *@2018年3月10日
  *@author:xiaohuaxiong
 */
#ifndef __WATER_MESH_H__
#define __WATER_MESH_H__
#include"Scene.h"
#include<d3d10.h>
#include<d3dx10.h>
class WaterMesh :public Scene
{
	//顶点缓冲区对象
	ID3D10Buffer    *_vertexBuffer;
	ID3D10Buffer    *_indexBuffer;
	ID3D10Device   *_device;
	//网格的行列数目
	int                          _row, _column;
	int                          _vertexCount, _indexCount;
	//网格的扰动系数
	float                       _K1, _K2, _K3;
	//网格的数据对象
	float                      *_prevMesh, *_currentMesh;
	//Effect
	ID3D10Effect                             *_effect;
	ID3D10EffectTechnique          *_effectTech;
	//矩阵变量
	ID3D10EffectMatrixVariable  *v_mvpMatrix;
	//颜色
	ID3D10EffectVectorVariable   *v_color;
	//光栅化状态对象
	ID3D10RasterizerState             *_rasterizerState;
	//InputLayout
	ID3D10InputLayout                  *_inputLayout;
	//MVP矩阵
	D3DXMATRIX                             _modelMatrix, _viewMatrix, _projMatrix,_mvpMatrix;
	//
	D3DXCOLOR                               _color;
	//
	float                                                _deltaTime,_dtInterval,_rainTime;
public:
	WaterMesh();
	void              init(int row,int column, float dx, float dt, float speed,float damping);
	//初始化网格对象
	void              initMesh(int row,int column);
	//layout
	void             initLayout();
	//effect
	void             initEffect();
	//
	void            initRasterizerState();
	//draw/update
	virtual void render();
	//
	virtual void update(float dt);
	//对网格进行扰动
	void              disturb(int x,int y,float magnitude);
};
#endif