/*
  *����Ӱʵ��
  *@date:2017��12��11��
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
	//��յķ���
	glk::Skybox			*_roomMesh;
	//����
	glk::Sphere        *_sphereMesh;
	glk::GLTexture *_texture;
	//��������ӵ��Դ
	glk::GLVector2   _offsetVec2;
	//event
	glk::TouchEventListener *_touchListener;
	glk::KeyEventListener     *_keyListener;
	//���������
	glk::Mat4            _viewProjMatrix[6];
	//������ͼͶӰƫ�þ���
	glk::Mat4            _viewProjOffsetMat4[6];
	//��Դ��λ��
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
	//��Ӱ
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