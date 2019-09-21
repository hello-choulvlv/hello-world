/*
  *≥°æ∞‰÷»æShader
  *@date:2017-4-8
  *@Author:xiaohuaxiong
 */
#ifndef __CAMERA_SHADER_H__
#define __CAMERA_SHADER_H__
//#include<engine/Object.h>
#include<engine/Geometry.h>
#include<engine/GLProgram.h>

class CameraShader
{
private:
	glk::GLProgram      *_renderProgram;
	//location of uniforms
	int                              _modelMatrixLoc;
	int                              _viewProjMatrixLoc;
	int                              _normalMatrixLoc;
	int                              _cropMatrixLoc;
	int                              _shadowMapArrayLoc;
	int                              _baseMapLoc;
	//int                              _lightVPSBMatrixLoc;
	int                              _normalSegmentsLoc;
	int                              _lightDirectionLoc;
	int                              _eyePositionLoc;
//uniform variable value
	glk::Matrix               _modelMatrix;
	glk::Matrix               _viewProjMatrix;
	glk::Matrix3             _normalMatrix;
	unsigned                  _shadowMapArray;
	unsigned                  _baseMap;
	//glk::Matrix              _lightVPSBMatrix[4];
	glk::Matrix             _cropMatrix[4];
	glk::GLVector4		 _normalSegments;
	glk::GLVector3       _lightDirection;
	glk::GLVector3       _eyePosition;
private:
	CameraShader();
	bool       loadShaderSource(const char *vsFile,const char *fsFile);
public:
	~CameraShader();
	//
	static CameraShader *createCameraShader(const char *vsFile,const char *fsFile);
	/*
	  *
	 */
	void       setModelMatrix(const glk::Matrix &modelMatrix);

	/*
	 */
	void      setViewProjMatrix(const glk::Matrix &viewProjMatrix);

	//
	void      setNormalMatrix(const glk::Matrix3 &normalMatrix);

	//
	void      setCropMatrix(const glk::Matrix cropMatrix[4],const int size);
	//
	void      setShadowMapArray(const unsigned shadowMapId,const int textureUnit);
	//
	void      setBaseMap(const unsigned baseMapId,const int textureUnit);
	//
	//void      setLightVPSBMatrix(const glk::Matrix vpsbMatrixArray[4]);
	//
	void      setNormalSegments(const glk::GLVector4 &normalSegments);
	//
	void      setLightDirection(const glk::GLVector3 &lightDirection);
	//
	void      setEyePosition(const glk::GLVector3 &eyePosition);
	//bind and set uniform values
	void      perform();
};

#endif