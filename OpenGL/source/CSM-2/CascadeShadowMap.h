/*
  *层叠阴影实现
  *Cascade Shadow Map
  *@date:2017-4-8
  *@author:xiaohuaxiong
  *@Version:geometry使用传统的几何着色器实现
 */
#ifndef __CASCADE_SHADOW_MAP_H__
#define __CASCADE_SHADOW_MAP_H__
#include<engine/Object.h>
#include<engine/Shape.h>
#include<engine/GLTexture.h>
#include<engine/ShadowMap.h>
#include<engine/event/TouchEventListener.h>
#include "engine/event/KeyEventListener.h"
#include"LightShader.h"
#include"CameraShader.h"
class  CascadeShadowMap :public glk::Object
{
private:
	//视口
	glk::GLVector4		_viewport[4];
	//光空间视图投影缩放偏移矩阵
	glk::Matrix            _lightVPSBMatrix[4];
	//投影矩阵
	glk::Matrix            _projMatrix;
	//光源视图矩阵
	glk::Matrix             _lightViewMatrix;
	//光源的投影矩阵
	glk::Matrix             _lightProjMatrix;
	glk::Matrix             _lightViewProjMatrix;
	//单独的裁剪矩阵
	glk::Matrix             _cropMatrix[4];
	//场景的渲染所需要的纹理采样矩阵
	glk::Matrix             _cropTextureMatrix[4];
	//光源视图投影矩阵
	//glk::Matrix             _lightViewProjMatrix;
	//光源的方向
	glk::GLVector3       _lightDirection;
	//眼睛的坐标
	glk::GLVector3        _eyePosition;
	//摄像机视角下的视图矩阵
	glk::Matrix             _cameraViewMatrix;
	//摄像机视角下的视图投影矩阵
	glk::Matrix             _cameraViewPorjMatrix;
	//规范化的远平面分割
	glk::GLVector4		_normalSegments;
	//分割的远平面(非规范化)
	float                        _farSegemtns[5];
	//远平面,近平面距离
	float                        _nearZ, _farZ;
	//Shader
	LightShader		   *_lightShader;
	CameraShader      *_cameraShader;
	//平面网格对象
	glk::Mesh               *_groundMesh;
	//三维球体
	glk::Sphere            *_sphere;
	//纹理
	glk::GLTexture      *_groundTexture;
	glk::GLTexture      *_sphereTexture;
	//CSM阴影贴图阵列
	glk::ShadowMap   *_csmShadowArray;
	//触屏事件
	glk::TouchEventListener    *_touchEvent;
	//键盘事件
	glk::KeyEventListener         *_keyEvent;
	//与视图矩阵相关的数据
	glk::GLVector2         _offset;
	glk::GLVector3         _pitchYawRoll;
	glk::GLVector3         _rotateVector;
	glk::GLVector3         _translateVec;
	glk::GLVector3         _velocityVec;
	glk::GLVector3         _lastVelocityVec;
private:
	CascadeShadowMap(CascadeShadowMap &);
	CascadeShadowMap();
	void         initCascadeShadowMap();
public:
	~CascadeShadowMap();
	static CascadeShadowMap *createCascadeShadowMap();
	/*
	  *初始化摄像机
	 */
	void           initCamera(const glk::GLVector3 &eyePosition,const glk::GLVector3 &viewPosition,const glk::GLVector3 &upVector);
	/*
	  *初始化光源
	 */
	void           initLight(const glk::GLVector3 &lightPosition,const glk::GLVector3 &lightViewPosition,const glk::GLVector3 &upVector);
	/*
	  *计算视锥体分割平面
	*/
	void           updateFrustumSegment();
	/*
	  *更新与光源相关的远平面分割,以及被分割的视锥体
	 */
	void           updateLightViewFrustum();
	/*
	  *计算光源视角下的由给定远近平面决定的包围盒的大小
	 */
	void           frustumBoudingboxInLightSpaceView(const float nearZ,const float farZ,glk::GLVector4 &boxMin,glk::GLVector4 &boxMax);
	/*
	  *计算给定的包围盒决定的裁剪矩阵
	 */
	void           buildCropMatrix(const glk::GLVector3 &maxCorner,const glk::GLVector3 &minCorner,glk::Matrix &cropMatrix);
	/*
	  *在给定的光源的视图矩阵之下的包围盒求取光源的裁剪矩阵
	 */
	void           calculateCropMatrix(const glk::GLVector3 &maxCorner, const glk::GLVector3 &minCorner, glk::Matrix &cropMatrix);
	/*
	  *渲染光源视角下的场景
	 */
	void           renderLightView( );
	/*
	  *渲染摄像机视角下的场景
	 */
	void           renderCameraView();
	/*
	  *更新与CSM相关的数据
	 */
	void           update(const float deltaTime);

	/*
	  *触屏事件
	 */
	bool           onTouchBegan(const glk::GLVector2 *touchPoint);
	void           onTouchMoved(const glk::GLVector2 *touchPoint);
	void           onTouchEnded(const glk::GLVector2  *touchPoint);
	/*
	  *键盘事件
	  */
	bool          onKeyPressed(const glk::KeyCodeType keyCode);
	void          onKeyReleased(const glk::KeyCodeType keyCode);
};

#endif