/*
  *建筑物场景渲染
  *2017年12月5日
  *@Author:xiaohuaxiong
 */
#ifndef __SCENE_BUILDING_H__
#define __SCENE_BUILDING_H__
#include "engine/GLTexture.h"
#include "engine/Camera2.h"
#include<engine/GLProgram.h>

#include"engine/event/TouchEventListener.h"
#include"engine/event/KeyEventListener.h"
#include "CollisionDetector.h"

class SceneBuilding:public glk::Object
{
	glk::Camera2							*_camera;
	glk::GLTexture						*_texture;
	CollisionDetector					_collisionDetector;
	glk::GLVector2						_offsetVec;
	glk::TouchEventListener      *_touchListener;
	glk::KeyEventListener          *_keyListener;
	//关于整个建筑物场景的顶点缓冲区对象
	unsigned                                _buildingVertexId;
	int                                          _vertexCount;
	glk::GLProgram                   *_glProgram;
	//键盘掩码
	int                                          _keyMask;
	//光源的位置
	glk::GLVector3                    _lightPosition;
public:
	SceneBuilding();
	~SceneBuilding();
	/*
	  *从文件中加载模型数据
	 */
	void            loadSceneBuilding(const char *filename);
	/*
	  *渲染
	 */
	void            render();
	/*
	  *实时刷新
	 */
	void           update(float deltaTime);
	/*
	  *鼠标事件
	 */
	bool           onTouchPressed(const glk::GLVector2 &touchPoint);
	void           onTouchMoved(const  glk::GLVector2  &touchPoint);
	void           onTouchReleased(const glk::GLVector2 &touchPoint);
	/*
	  *键盘事件
	 */
	void          onKeyRelease(glk::KeyCodeType keyCode);
	bool          onKeyPressed(glk::KeyCodeType keyCode);
};
#endif