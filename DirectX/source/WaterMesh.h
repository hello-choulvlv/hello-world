/*
  *ˮ��Ⱦ,DirectXʵ��
  *@2018��3��10��
  *@author:xiaohuaxiong
 */
#ifndef __WATER_MESH_H__
#define __WATER_MESH_H__
#include"Scene.h"
#include<d3d10.h>
#include<d3dx10.h>
class WaterMesh :public Scene
{
	//���㻺��������
	ID3D10Buffer    *_vertexBuffer;
	ID3D10Buffer    *_indexBuffer;
	ID3D10Device   *_device;
	//�����������Ŀ
	int                          _row, _column;
	int                          _vertexCount, _indexCount;
	//������Ŷ�ϵ��
	float                       _K1, _K2, _K3;
	//��������ݶ���
	float                      *_prevMesh, *_currentMesh;
	//Effect
	ID3D10Effect                             *_effect;
	ID3D10EffectTechnique          *_effectTech;
	//�������
	ID3D10EffectMatrixVariable  *v_mvpMatrix;
	//��ɫ
	ID3D10EffectVectorVariable   *v_color;
	//��դ��״̬����
	ID3D10RasterizerState             *_rasterizerState;
	//InputLayout
	ID3D10InputLayout                  *_inputLayout;
	//MVP����
	D3DXMATRIX                             _modelMatrix, _viewMatrix, _projMatrix,_mvpMatrix;
	//
	D3DXCOLOR                               _color;
	//
	float                                                _deltaTime,_dtInterval,_rainTime;
public:
	WaterMesh();
	void              init(int row,int column, float dx, float dt, float speed,float damping);
	//��ʼ���������
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
	//����������Ŷ�
	void              disturb(int x,int y,float magnitude);
};
#endif