/*
  *天空盒
  *2017年12月10日17:22:49
  *@author:xiaohuaxiong
*/
#ifndef __SKYBOX_H__
#define __SKYBOX_H__
#include"engine/Geometry.h"
#include"engine/GLProgram.h"
#include"engine/Camera2.h"
#include "engine/Camera.h"
#include"engine/GLTexture.h"

#include"engine/event/TouchEventListener.h"
#include"engine/event/KeyEventListener.h"

class Skybox :public glk::Object
{
	glk::GLProgram	*_skyboxProgram;
	glk::GLVector2      _offsetVec;
	glk::Camera2			*_camera;
	glk::Camera          *_camera2;
	glk::GLCubeMap  *_cubeMap;
	unsigned                  _skyboxVertexId;
	unsigned                  _skyboxVertexArrayId;

	glk::TouchEventListener *_touchListener;
	glk::KeyEventListener     *_keyListener;
public:
	Skybox();
	~Skybox();
	void      initSkybox();

	bool      onTouchBegan(const glk::GLVector2 &touchPoint);
	void      onTouchMoved(const glk::GLVector2 &touchPoint);
	void      onTouchEnded(const glk::GLVector2 &touchPoint);

	bool      onKeyPressed(glk::KeyCodeType keyCode);
	void      onKeyReleased(glk::KeyCodeType keyCode);

	void      update(float deltaTime);
	void      render();
};
#endif