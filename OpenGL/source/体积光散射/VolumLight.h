/*
  *�����ɢ��
  *@author:xiaohuaxiong
  *@date:2018��11��6��
 */
#ifndef __VOLUM_LIGHT_H__
#define __VOLUM_LIGHT_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"
#include "engine/Shape.h"
#include "engine/Camera2.h"
#include "engine/RenderTexture.h"
#include "engine/GLTexture.h"

#include "engine/event/TouchEventListener.h"
#include "engine/event/KeyEventListener.h"
class VolumLight :public glk::Object
{
	glk::Camera2            *_camera;
	glk::GLTexture         *_lightTexture;
	glk::RenderTexture *_offlineRenderTexture;
	glk::RenderTexture *_craftRenderTexture;
	glk::RenderTexture *_downSizeRenderTexture;
	glk::RenderTexture *_fuzzyRenderTexture1;
	glk::RenderTexture *_fuzzyRenderTexture2;
	//Shader
	glk::GLProgram    *_materialShader;
	glk::GLProgram    *_cutoutShader;
	glk::GLProgram    *_rayExtruderShader;
	glk::GLProgram    *_fuzzyShader;
	glk::GLProgram   *_downSizeShader;
	glk::GLProgram   *_combineShader;
	//Geometry
	glk::Mesh              *_meshShape;
	glk::Pyramid        *_pyramidShape;
	glk::Sphere           *_sphereShape;
	glk::Cube             *_cubeShape;
	//Geometry Matrix
	glk::Matrix            _meshModelMatrix1;
	glk::Matrix3          _meshNormalMatrix1;
	glk::Matrix            _meshModelMatrix2;
	glk::Matrix3          _meshNormalMatrix2;
	glk::Matrix            _pyramidModelMatrix;
	glk::Matrix3          _pyramidNormalMatrix;
	glk::Matrix            _sphereModelMatrix;
	glk::Matrix3          _sphereNormalMatrix;
	glk::Matrix           _cubeModelMatrix1;
	glk::Matrix           _cubeModelMatrix2;
	glk::Matrix3         _cubeNormalMatrix1;
	glk::Matrix3         _cubeNormalMatrix2;
	//ģ����Ȩ��
	float  _fuzzyWeights[6];
	float  _fuzzyWeightInv;
	float  _rayOpacity;
	float  _rayRadius;
	//ȫ�ֱ���
	glk::GLVector3      _lightPosition;
	glk::GLVector3      _lightColor;
	glk::GLVector3      _lightAmbientColor;
	glk::GLVector4      _rayColor;
	glk::GLVector4      _occluderParams;
	glk::GLVector4      _nearQFarRatio;
	glk::GLVector2      _pixelStepVec2;
	//glk::GLVector2      _lightFadeoutParam;//���յ�˥��ϵ��
	//
	//����Shader��uniform������λ��
	int                          _materialModelMatrixLoc;
	int                          _materialNormalMatrixLoc;
	int                          _materialViewProjMatrixLoc;
	int                          _materialColorLoc;
	int                          _materialLightPositionLoc;
	int                          _materialLightColorLoc;
	int                          _materialAmbientColorLoc;
	//Cutout Shader Uniform location
	int                          _cutoutDepthTextureLoc;
	int                          _cutoutColorTextureLoc;
	int                          _cutoutOccluderParamLoc;
	int                          _cutoutNearQFarLoc;
	int                          _cutoutLightPositionScreenLoc;
	//Ray Extruder shader uniform location
	int                          _rayExtruderRayTextureLoc;
	int                          _rayExtruderRayLengthVec2Loc;
	int                          _rayExtruderLightPositionScreenLoc;
	//down size shader uniform location
	int                          _downSizeBaseTextureLoc;
	//fuzzy shader uniform location
	int                          _fuzzyBaseTextureLoc;
	int                          _fuzzyPixelStepLoc;
	int                          _fuzzyWeightsLoc;
	//combine shader uniform location
	int                          _combineBaseTextureLoc;
	int                          _combineRayTextureLoc;
	int                          _combineRayColorLoc;
	int                          _combineRayOpacityLoc;

	glk::TouchEventListener  *_touchListener;
	glk::KeyEventListener       *_keyListener;
	glk::GLVector2                      _touchPoint;
	int                                             _keyCodeMask;
public:
	VolumLight();
	~VolumLight();
	void   init();

	void   initRenderTexture();
	void    initShader();
	//load geometry
	void    initGeometry();
	/*
	  *��Ⱦ������
	 */
	void    renderGeometry();
	/*
	  *��ȡ�����������Ϣ
	 */
	void   cutoutDepthVolum();
	/*
	  *��С���ɵĹ���ͼ�ĳߴ�
	 */
	void   downSizeLightVolum();
	/*
	  *ģ������
	 */
	void   fuzzyTexture(unsigned textureId);
	/*
	  *��ѹ���ɵĹ���ͼ
	 */
	void  extrudeLightVolum();
	/*
	  *�ϳ����յ�ͼ��
	 */
	void  combineLightVolum();
	/*
	  *debug texture
	 */
	void  debugTexture(unsigned textureId);
	/*
	  *��Ⱦ
	 */
	void  render();
	/*
	  *update
	 */
	void  update(float t,float s);
	/*
	  *touch callback
	 */
	bool  onTouchBegan(const glk::GLVector2 &touchPoint);
	void  onTouchMoved(const glk::GLVector2 &touchPoint);
	void  onTouchEnded(const glk::GLVector2 &touchPoint);
	/*
	  *key callback
	 */
	bool  onKeyPressed(glk::KeyCodeType  keyCode);
	void  onKeyReleased(glk::KeyCodeType keyCode);
};
#endif