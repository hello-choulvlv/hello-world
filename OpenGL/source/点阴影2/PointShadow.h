/*
  *点阴影实现
  *@date:2017年12月11日
  *@Author:xiaohuaxiong
 */
#ifndef __POINT_SHADOW_H__
#define __POINT_SHADOW_H__
#include"engine/Object.h"
#include"engine/Geometry.h"
#include"engine/GLProgram.h"
#include"engine/Camera2.h"
#include"engine/GLTexture.h"
#include"engine/ShadowMap.h"
#include "engine/Shape.h"

#include"engine/event/TouchEventListener.h"
#include"engine/event/KeyEventListener.h"

class PointShadow :public glk::Object
{
	glk::GLProgram *_shadowProgram;
	glk::GLProgram *_renderProgram;
	glk::GLProgram *_lightProgram;
	glk::Camera2      *_camera;
	glk::ShadowMap *_shadowMap;
	//封闭的房间
	glk::Skybox			*_roomMesh;
	//球体
	glk::Sphere        *_sphereMesh;
	glk::GLTexture *_texture;
	//立方体盒子点光源
	glk::GLVector2   _offsetVec2;
	//event
	glk::TouchEventListener *_touchListener;
	glk::KeyEventListener     *_keyListener;
	//六个面矩阵
	glk::Mat4            _viewProjMatrix[6];
	//六个视图投影偏置矩阵
	glk::Mat4            _viewProjOffsetMat4[6];
	//光源的位置
	glk::Vec3             _lightPosition;
	//
	int                       _keyMask;
public:
	PointShadow();
	~PointShadow();
	void       initShadow();
	//
	void       update(float deltaTime);
	void       render();
	//阴影
	void       renderShadow();
	//event
	bool      onTouchPressed(const glk::Vec2 &touchPoint);
	void      onTouchMoved(const glk::Vec2  &touchPoint);
	void     onTouchReleased(const glk::Vec2 &touchPoint);
	//
	bool    onKeyPressed(glk::KeyCodeType keyCode);
	void    onKeyReleased(glk::KeyCodeType keyCode);
};
#endif