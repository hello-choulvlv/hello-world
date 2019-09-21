/*
  *�����ﳡ����Ⱦ
  *2017��12��5��
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
	//�������������ﳡ���Ķ��㻺��������
	unsigned                                _buildingVertexId;
	int                                          _vertexCount;
	glk::GLProgram                   *_glProgram;
	//��������
	int                                          _keyMask;
	//��Դ��λ��
	glk::GLVector3                    _lightPosition;
public:
	SceneBuilding();
	~SceneBuilding();
	/*
	  *���ļ��м���ģ������
	 */
	void            loadSceneBuilding(const char *filename);
	/*
	  *��Ⱦ
	 */
	void            render();
	/*
	  *ʵʱˢ��
	 */
	void           update(float deltaTime);
	/*
	  *����¼�
	 */
	bool           onTouchPressed(const glk::GLVector2 &touchPoint);
	void           onTouchMoved(const  glk::GLVector2  &touchPoint);
	void           onTouchReleased(const glk::GLVector2 &touchPoint);
	/*
	  *�����¼�
	 */
	void          onKeyRelease(glk::KeyCodeType keyCode);
	bool          onKeyPressed(glk::KeyCodeType keyCode);
};
#endif