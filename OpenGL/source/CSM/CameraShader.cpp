/*
  *场景渲染Shader实现
  *2017-4-10
  *@author:xiaohuaxiong
 */
//是否调试这个Shader
#define   __DEBUG_CAMERA_SHADER_
#include"CameraShader.h"
#include<GL/glew.h>
#include<assert.h>
#include<string.h>
CameraShader::CameraShader()
{
	_modelMatrixLoc = 0;
	_viewProjMatrixLoc = 0;
	_normalMatrixLoc = 0;
	_shadowMapArrayLoc = 0;
	_baseMapLoc = 0;
	_lightVPSBMatrixLoc = 0;
	_normalSegmentsLoc = 0;
	_lightDirectionLoc = 0;
	_eyePositionLoc = 0;
}

CameraShader::~CameraShader()
{
	_renderProgram->release();
}

bool    CameraShader::loadShaderSource(const char *vsFile, const char *fsFile)
{
	_renderProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_modelMatrixLoc = _renderProgram->getUniformLocation("u_modelMatrix");
	_viewProjMatrixLoc = _renderProgram->getUniformLocation("u_viewProjMatrix");
	_normalMatrixLoc= _renderProgram->getUniformLocation("u_normalMatrix");

	_shadowMapArrayLoc = _renderProgram->getUniformLocation("u_shadowMapArray");
	_baseMapLoc = _renderProgram->getUniformLocation("u_baseMap");
	_lightVPSBMatrixLoc = _renderProgram->getUniformLocation("u_lightVPSBMatrix");
	_normalSegmentsLoc = _renderProgram->getUniformLocation("u_normalSegments");
	_lightDirectionLoc = _renderProgram->getUniformLocation("u_lightDirection");
	_eyePositionLoc = _renderProgram->getUniformLocation("u_eyePosition");
	return true;
}

CameraShader   *CameraShader::createCameraShader(const char *vsFile, const char *fsFile)
{
	CameraShader  *shader = new CameraShader();
	shader->loadShaderSource(vsFile, fsFile);
	return shader;
}

void CameraShader::setModelMatrix(const glk::Matrix &modelMatrix)
{
#ifdef __DEBUG_CAMERA_SHADER_
	int    _defaultProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &_defaultProgram);
	assert(_defaultProgram == _renderProgram->getProgram());
#endif
	if (_modelMatrixLoc >= 0)
	{
		_modelMatrix = modelMatrix;
		glUniformMatrix4fv(_modelMatrixLoc, 1, GL_FALSE, modelMatrix.pointer());
	}
}

void CameraShader::setViewProjMatrix(const glk::Matrix &viewProjMatrix)
{
#ifdef __DEBUG_CAMERA_SHADER_
	int    _defaultProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &_defaultProgram);
	assert(_defaultProgram == _renderProgram->getProgram());
#endif
	if (_viewProjMatrixLoc >= 0)
	{
		_viewProjMatrix = viewProjMatrix;
		glUniformMatrix4fv(_viewProjMatrixLoc, 1, GL_FALSE, viewProjMatrix.pointer());
	}
}

void CameraShader::setNormalMatrix(const glk::Matrix3 &normalMatrix)
{
#ifdef __DEBUG_CAMERA_SHADER_
	int    _defaultProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &_defaultProgram);
	assert(_defaultProgram == _renderProgram->getProgram());
#endif
	if (_normalMatrixLoc >= 0)
	{
		memcpy(&_normalMatrix, &normalMatrix, sizeof(glk::Matrix3));
		glUniformMatrix3fv(_normalMatrixLoc, 1, GL_FALSE, normalMatrix.pointer());
	}
}

void CameraShader::setShadowMapArray(const unsigned shadowMapId,const int textureUnit)
{
#ifdef __DEBUG_CAMERA_SHADER_
	int    _defaultProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &_defaultProgram);
	assert(_defaultProgram == _renderProgram->getProgram());
#endif
	if (_shadowMapArrayLoc >= 0)
	{
		_shadowMapArray = shadowMapId;
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D_ARRAY, _shadowMapArray);
		glUniform1i(_shadowMapArrayLoc, textureUnit);
	}
}

void CameraShader::setBaseMap(const unsigned baseMapId,const int textureUnit)
{
#ifdef __DEBUG_CAMERA_SHADER_
	int    _defaultProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &_defaultProgram);
	assert(_defaultProgram == _renderProgram->getProgram());
#endif
	if (_baseMapLoc >= 0)
	{
		_baseMap = baseMapId;
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, baseMapId);
		glUniform1i(_baseMapLoc, textureUnit);
	}
}

void  CameraShader::setLightVPSBMatrix(const glk::Matrix vpsbMatrixArray[4])
{
#ifdef __DEBUG_CAMERA_SHADER_
	int    _defaultProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &_defaultProgram);
	assert(_defaultProgram == _renderProgram->getProgram());
#endif
	if (_lightVPSBMatrixLoc)
	{
		memcpy(_lightVPSBMatrix, vpsbMatrixArray, 4 * sizeof(glk::Matrix));
		glUniformMatrix4fv(_lightVPSBMatrixLoc, 4, GL_FALSE, vpsbMatrixArray[0].pointer());
	}
}

void CameraShader::setNormalSegments(const glk::GLVector4 &normalSegments)
{
#ifdef __DEBUG_CAMERA_SHADER_
	int    _defaultProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &_defaultProgram);
	assert(_defaultProgram == _renderProgram->getProgram());
#endif
	if (_normalSegmentsLoc >= 0)
	{
		_normalSegments = normalSegments;
		glUniform4fv(_normalSegmentsLoc, 1, &normalSegments.x);
	}
}

void CameraShader::setLightDirection(const glk::GLVector3 &lightDirection)
{
#ifdef __DEBUG_CAMERA_SHADER_
	int    _defaultProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &_defaultProgram);
	assert(_defaultProgram == _renderProgram->getProgram());
#endif
	if (_lightDirectionLoc >= 0)
	{
		_lightDirection = lightDirection;
		glUniform3fv(_lightDirectionLoc, 1, &lightDirection.x);
	}
}

void CameraShader::setEyePosition(const glk::GLVector3 &eyePosition)
{
#ifdef __DEBUG_CAMERA_SHADER_
	int    _defaultProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &_defaultProgram);
	assert(_defaultProgram == _renderProgram->getProgram());
#endif
	if (_eyePositionLoc >= 0)
	{
		_eyePosition = eyePosition;
		glUniform3fv(_eyePositionLoc, 1, &eyePosition.x);
	}
}

void CameraShader::perform()
{
	_renderProgram->perform();
}