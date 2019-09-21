/*
  *��ƽ�����,��Ҫʹ�õ�FFT�任
  *2018��2��2��
  *@Author:xiaohuaxiong
 */
#ifndef __CHOPPY_SIMULATE_H__
#define __CHOPPY_SIMULATE_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/GLProgram.h"
#include "engine/Camera2.h"
#include "engine/GLTexture.h"
#include "engine/FFT.h"

#include "engine/event/TouchEventListener.h"
#include "engine/event/KeyEventListener.h"
//��������ĳߴ�
#define _CHOPPY_MESH_SIZE_   128
//
class ChoppySimulate :public glk::Object
{
	glk::GLProgram  *_choppyProgram;
	glk::GLProgram  *_skyboxProgram;
	glk::Camera2       *_camera;
	glk::GLCubeMap *_normalCubeMap;
	glk::GLCubeMap *_envCubeMap;
	glk::GLCubeMap *_bottomCubeMap;
	glk::GLTexture     *_skyboxTextures[6];
	//���ڳ�������Ҫʹ�õ�������
	glk::Vec2               _touchOffset;
	glk::Vec4               _choopyColor;
	glk::Mat4              _transform;
	glk::Mat3              _normalMatrix;
	glk::Vec3              _freshnelParam;
	glk::Vec3              _lightPosition;
	glk::Mat4              _skyboxTransform[6];
	//event
	glk::TouchEventListener *_touchListener;
	glk::KeyEventListener     *_keyListener;
	//���㺣ƽ����Ҫ�õ������ݽṹ
	//����
	glk::Vec4      _lambad[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	//�����߶ȳ�
	Complex       _basicHeightField[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	//�ڶ��׶����ɵ�fft�߶ȳ�
	Complex	   _heightField[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	//ƫ�Ƴ�/X+Y
	Complex       _offsetXField[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	Complex       _offsetYField[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	//�����ķ���
	glk::Vec3	  _normals[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	//�����Ķ���λ��
	glk::Vec3     _positions[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	//��������������
	unsigned     _indexBuffer;
	//�����������껺��������
	unsigned     _fragCoordBuffer;
	//��������
	unsigned     _normalTexture;
	//ʱ��
	float             _time;
	//Key Mask
	int                _keyMask;
public:
	ChoppySimulate();
	~ChoppySimulate();
	void    initChoppySimulate();
	//��ʼ�����˲���
	void   initHeightField();
	//��ʼ�����㻺��������
	void   initBuffer();
	//
	void   update(float dt);
	//ƫ�Ƴ��仯
	void   updateOffsetField();
	//���㷨��
	void   updateNormals();
	//
	void   render();
	//��պ�
	void  renderSkybox();
	//����
	void  renderChoppy();
	//���º��˸߶ȳ�
	void  updateHeightField();
	//����ƫ�Ƴ�
	void  updateOffset();
	//Event
	bool onTouchBegan(const glk::Vec2 &touchPoint);
	void onTouchMoved(const glk::Vec2 &touchPoint);
	void onTouchReleased(const glk::Vec2 &touchPoint);
	//Key Event
	bool onKeyPressed(glk::KeyCodeType keyCode);
	void onKeyReleased(glk::KeyCodeType keyCode);

};

#endif