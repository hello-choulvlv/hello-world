/*
  *VSM����ʵ��
  *2018��8��3��
  *@author:xiaohuaxiong
 */
#include "engine/GLProgram.h"
#include "engine/Geometry.h"
#include "engine/Shape.h"
#include "engine/Camera2.h"
#include "engine/event/TouchEventListener.h"
#include "engine/event/KeyEventListener.h"
//#define USE_LIGHT_SPACE_SM //�Ƿ����ù�ռ�͸����Ӱ
#ifndef USE_LIGHT_SPACE_SM
#include "ShadowMapVSM.h"
#endif
class VSM : public glk::Object
{
#ifndef USE_LIGHT_SPACE_SM
	ShadowMapVSM     *_shadowVSM;
#endif
	glk::Camera2            *_camera;
	//����Shader
	glk::GLProgram    *_lightProgram;
	//������ģ�Ͷ���
	glk::Pyramid          *_pyramidMesh;
	glk::Mesh               *_groundMesh;
	glk::Sphere            *_sphereMesh;
	//matrix
	glk::Matrix              _pyramidModelMatrix;
	glk::Matrix3            _pyramidNormalMatrix;
	glk::Matrix              _groundModelMatrix;
	glk::Matrix3            _groundNormalMatrix;
	glk::Matrix              _sphereModelMatrix;
	glk::Matrix3            _sphereNormalMatrix;
	//��������
	glk::GLVector3       _lightDirection;
	glk::GLVector3       _lightPosition;
	glk::GLVector3       _lightAmbientColor;
	glk::GLVector4       _lightColor;
	float                           _lightBleeding;

	int                              _viewProjMatrixLoc;
	int                              _modelMatrixLoc;
	int                              _normalMatrixLoc;
	int                              _shadowMapLoc;
	//int                              _lightMVMatrixLoc;
	int                              _lightProjMatrixLoc;
	int                              _lightViewMatrixLoc;
	int                              _lightDirectionLoc;
	int                              _lightAmbientColorLoc;
	int                              _lightColorLoc;
	int                              _lightBleedingLoc;

	glk::TouchEventListener *_touchListener;
	glk::KeyEventListener      *_keyListener;
	glk::GLVector2               _touchPoint;
	int                                     _keyMask;
	//��Դ����
	glk::Matrix                _lightViewProjMatrix;
	glk::Matrix                _lightViewMatrix;
	glk::Matrix                _lightProjMatrix;

	//Shadow Shader
	glk::GLProgram    *_shadowProgram;
	int                               _shadowViewProjMatrixLoc;
	int                               _shadowModelMatrixLoc;
	//int                               _shadowLightPositionLoc;
	int                               _shadowLightViewMatrixLoc;

	int                              _analysisScene;//�Ƿ�����������
	int                              _changeViewMode;//�Ƿ��л��ӽ�
public:
	VSM();
	~VSM();

	void         init();
	//��ʼ����Դ����
	void         updateLightMatrix();

	void         update(float deltaTime,float delayTime);
	void         render();

	bool         onTouchBegan(const glk::GLVector2 &touchPoint);
	void         onTouchMoved(const glk::GLVector2 &touchPoint);
	void         onTouchEnded(const glk::GLVector2 &touchPoint);

	bool        onKeyPressed(glk::KeyCodeType keyCode);
	void        onKeyReleased(glk::KeyCodeType keyCode);
};
