/*
  *神光Shader实现
  *2017-06-24
  *@Author:xiaohuaxiong
 */
#include"GL/glew.h"
#include "RayShader.h"

RayShader::RayShader()
{
	_glProgram = nullptr;
	_modelMatrixLoc = -1;
	_viewProjMatrixLoc = -1;
	_normalMatrixLoc = -1;
	_eyePositionLoc = -1;
	_colorLoc = -1;
}

RayShader::~RayShader()
{
	_glProgram->release();
	_glProgram = nullptr;
}

RayShader *RayShader::createRayShader(const char *vsFile, const char *fsFile)
{
	RayShader * ray = new RayShader();
	ray->initWithFileName(vsFile, fsFile);
	return ray;
}
void RayShader::initWithFileName(const char *vsFile, const char *fsFile)
{
	_glProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_modelMatrixLoc = _glProgram->getUniformLocation("g_ModelMatrix");
	_viewProjMatrixLoc = _glProgram->getUniformLocation("g_ViewProjMatrix");
	_normalMatrixLoc = _glProgram->getUniformLocation("g_NormalMatrix");
	_eyePositionLoc = _glProgram->getUniformLocation("g_EyePosition");
	_colorLoc = _glProgram->getUniformLocation("g_Color");
	_positionLoc = _glProgram->getAttribLocation("a_position");
	_normalLoc = _glProgram->getAttribLocation("a_normal");
}

void RayShader::setModelMatrix(const glk::Matrix &modelMatrix)
{
	if (_modelMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_modelMatrixLoc, 1, GL_FALSE, modelMatrix.pointer());
	}
}

void RayShader::setViewProjMatrix(const glk::Matrix &viewProjMatrix)
{
	if (_viewProjMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_viewProjMatrixLoc, 1, GL_FALSE, viewProjMatrix.pointer());
	}
}

void RayShader::setNormalMatrix(const glk::Matrix3 &normalMatrix)
{
	if (_normalMatrixLoc >= 0)
	{
		glUniformMatrix3fv(_normalMatrixLoc, 1, GL_FALSE, normalMatrix.pointer());
	}
}

void RayShader::setEyePosition(const glk::GLVector3 &eyePosition)
{
	if (_eyePositionLoc >= 0)
	{
		glUniform3fv(_eyePositionLoc, 1, &eyePosition.x);
	}
}

void RayShader::setColor(const glk::GLVector4 &color)
{
	if (_colorLoc >= 0)
	{
		glUniform4fv(_colorLoc, 1, &color.x);
	}
}

void RayShader::perform()const
{
	_glProgram->perform();
}

int RayShader::getPositionLoc()const
{
	return _positionLoc;
}
int RayShader::getNormalLoc()const
{
	return _normalLoc;
}