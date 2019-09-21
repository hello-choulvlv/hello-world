/*
  *海平面仿真,需要使用到FFT变换
  *2018年2月2日
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
//定义网格的尺寸
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
	//关于程序中需要使用到的数据
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
	//计算海平面需要用到的数据结构
	//波数
	glk::Vec4      _lambad[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	//基本高度场
	Complex       _basicHeightField[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	//第二阶段生成的fft高度场
	Complex	   _heightField[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	//偏移场/X+Y
	Complex       _offsetXField[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	Complex       _offsetYField[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	//海波的法线
	glk::Vec3	  _normals[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	//海波的定点位置
	glk::Vec3     _positions[_CHOPPY_MESH_SIZE_][_CHOPPY_MESH_SIZE_];
	//索引缓冲区对象
	unsigned     _indexBuffer;
	//顶点纹理坐标缓冲区对象
	unsigned     _fragCoordBuffer;
	//法线纹理
	unsigned     _normalTexture;
	//时间
	float             _time;
	//Key Mask
	int                _keyMask;
public:
	ChoppySimulate();
	~ChoppySimulate();
	void    initChoppySimulate();
	//初始化海浪波数
	void   initHeightField();
	//初始化顶点缓冲区对象
	void   initBuffer();
	//
	void   update(float dt);
	//偏移场变化
	void   updateOffsetField();
	//计算法线
	void   updateNormals();
	//
	void   render();
	//天空盒
	void  renderSkybox();
	//海浪
	void  renderChoppy();
	//更新海浪高度场
	void  updateHeightField();
	//更新偏移场
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