/*
  *山峰与山谷
  *2018年3月6日
  *@author:xiaohuaxiong
 */
#ifndef __PEAKS_AND_VALLEYS_H__
#define __PEAKS_AND_VALLEYS_H__
#include "Scene.h"
#include "d3d10.h"
#include "d3dx10.h"

class PeaksAndValleys :public Scene
{
private:
	int  _meshRow,_meshColumn;
	int  _vertexCount, _indexCount;
	ID3D10Buffer   *_vertexBuffer, *_indexBuffer;
	//
	ID3D10InputLayout       *_inputLayout;
	ID3D10Effect                   *_effect;
	ID3D10EffectTechnique *_effectTech;
	ID3D10EffectMatrixVariable *_mvpMatrixV;
	//
	D3DXMATRIX   _viewMatrix, _projMatrix, _modelMatrix;
	float   _deltaTime;
public:
	PeaksAndValleys();
	~PeaksAndValleys();
	void    init(int row,int column);
	//
	void   initMesh(int row,int column);
	//
	virtual void update(float dt);
	virtual void render();
};

#endif