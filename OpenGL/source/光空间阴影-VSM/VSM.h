/*
  *VSM场景实现
  *2018年8月3日
  *@author:xiaohuaxiong
 */
#include "engine/GLProgram.h"
#include "engine/Geometry.h"
#include "engine/Shape.h"
#include "engine/Camera2.h"
#include "engine/event/TouchEventListener.h"
#include "engine/event/KeyEventListener.h"
//#define USE_LIGHT_SPACE_SM //是否启用光空间透视阴影
#ifndef USE_LIGHT_SPACE_SM
#include "ShadowMapVSM.h"
#endif
class VSM : public glk::Object
{
#ifndef USE_LIGHT_SPACE_SM
	ShadowMapVSM     *_shadowVSM;
#endif
	glk::Camera2            *_camera;
	//场景Shader
	glk::GLProgram    *_lightProgram;
	//几何体模型对象
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
	//光线数据
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
	//光源矩阵
	glk::Matrix                _lightViewProjMatrix;
	glk::Matrix                _lightViewMatrix;
	glk::Matrix                _lightProjMatrix;

	//Shadow Shader
	glk::GLProgram    *_shadowProgram;
	int                               _shadowViewProjMatrixLoc;
	int                               _shadowModelMatrixLoc;
	//int                               _shadowLightPositionLoc;
	int                               _shadowLightViewMatrixLoc;

	int                              _analysisScene;//是否开启场景分析
	int                              _changeViewMode;//是否切换视角
public:
	VSM();
	~VSM();

	void         init();
	//初始化光源矩阵
	void         updateLightMatrix();

	void         update(float deltaTime,float delayTime);
	void         render();

	bool         onTouchBegan(const glk::GLVector2 &touchPoint);
	void         onTouchMoved(const glk::GLVector2 &touchPoint);
	void         onTouchEnded(const glk::GLVector2 &touchPoint);

	bool        onKeyPressed(glk::KeyCodeType keyCode);
	void        onKeyReleased(glk::KeyCodeType keyCode);
};
