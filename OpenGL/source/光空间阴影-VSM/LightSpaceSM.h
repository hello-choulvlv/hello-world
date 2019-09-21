/*
  *��ռ�͸����Ӱ
  *@author:xiaohuaxiong
  *date:2018��8��7��
 */
#ifndef __LIGHT_SPACE_SM_H__
#define __LIGHT_SPACE_SM_H__
#include "engine/GLProgram.h"
#include "engine/Geometry.h"
#include "engine/ShadowMap.h"
#include "engine/Camera2.h"
#include "engine/Shape.h"

#include "engine/event/TouchEventListener.h"
#include "engine/event/KeyEventListener.h"

class LightSpaceSM :public glk::Object
{
	glk::ShadowMap    *_shadowMap;//������Ӱ����
	glk::GLProgram     *_shadowProgram;
	glk::Camera2          *_camera;
	//������ģ�Ͷ���
	glk::Pyramid          *_pyramidMesh;
	glk::Mesh               *_groundMesh;
	glk::Sphere            *_sphereMesh;
	//��Χ��
	//glk::AABB                _pyramidAABB;
	//glk::AABB                _groundAABB;
	//glk::AABB                _sphereAABB;

	//model matrix
	glk::Matrix              _pyramidModelMatrix;
	glk::Matrix3            _pyramidNormalMatrix;
	glk::Matrix              _groundModelMatrix;
	glk::Matrix3            _groundNormalMatrix;
	glk::Matrix              _sphereModelMatrix;
	glk::Matrix3            _sphereNormalMatrix;

	glk::Matrix                _lightViewMatrix;
	glk::Matrix                _lightProjMatrix;
	glk::Matrix                _lightViewProjMatrix;
	glk::GLVector3        _lightDirection;
	glk::GLVector4        _lightColor;
	glk::GLVector3        _lightAmbientColor;

	int                               _shadowMVPMatrixLoc;

	glk::GLProgram      *_lightProgram;
	int                               _lightModelMatrixLoc;
	int                               _lightViewProjMatrixLoc;
	int                               _lightNormalMatrixLoc;
	int                               _lightShadowMapLoc;
	int                               _lightLightDirectionLoc;
	int                               _lightLightViewProjMatrixLoc;
	int                               _lightAmbientColorLoc;
	int                               _lightColorLoc;

	glk::GLProgram     *_debugProgram;
	glk::Matrix               _debugMVPMatrix;
	int                              _debugMVPMatrixLoc;
	int                              _debugBaseMapLoc;

	glk::TouchEventListener *_touchListener;
	glk::KeyEventListener      *_keyListener;

	int                               _keyMask;
	glk::GLVector2        _touchOffset;

	int                               _viewMode;//�ӽ��л�
	int                               _showDebug;//�Ƿ������½���ʾ�������
	int                               _analysisScene;//�Ƿ����ó�������
public:
	LightSpaceSM();
	~LightSpaceSM();
	static    LightSpaceSM *create();
	bool      init();
	//bool      initLightMatrix();

	void      updateLightSpaceFrustum();

	void      render();
	void      debugRender();
	void      update(float deltaTime,float delayTime);

	bool      onTouchBegan(const glk::GLVector2 &touchPoint);
	void      onTouchMoved(const glk::GLVector2 &touchPoint);
	void      onTouchEnded(const glk::GLVector2 &touchPoint);

	bool      onKeyPressed(glk::KeyCodeType keyCode);
	void      onKeyReleased(glk::KeyCodeType keyCode);
};

#endif