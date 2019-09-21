/*
  *LightShader,光视图Shader
  *@Author:xiaohuaxiong
  *@date:2017-4-8
 */
//是否开启调试着色器,使用这个宏之后，每一步glUniform*操作都会检查当前使用的Shader
#define __DEBUG_SHADOW_PROGRAM_
#include"LightShader.h"
#include<GL/glew.h>
#include<assert.h>
#include<string.h>
LightShader::LightShader()
{
	_lightProgram = NULL;
	_mvpMatrixLoc = 0;
	_cropMatrixLoc = 0;
}

LightShader::~LightShader()
{
	_lightProgram->release();
}
LightShader *LightShader::createShaderWithSource(const char *vsFile, const char *gsFile, const char *fsFile)
{
	LightShader *shader = new LightShader();
	shader->loadShaderSource(vsFile, gsFile, fsFile);
	return shader;
}

void LightShader::loadShaderSource(const char *vsFile, const char *gsFile, const char *fsFile)
{
	_lightProgram = glk::GLProgram::createWithFile(vsFile, gsFile, fsFile);
	_lightProgram->retain();
	_mvpMatrixLoc = _lightProgram->getUniformLocation("g_MVPMatrix");
	_cropMatrixLoc = _lightProgram->getUniformLocation("g_CropMatrix");
}

//void  LightShader::setShadowMapSize(const glk::Size &shadowMapSize)
//{
//#ifdef __DEBUG_SHADOW_PROGRAM_
//	int _defaultProgram;
//	glGetIntegerv(GL_CURRENT_PROGRAM, &_defaultProgram);
//	assert(_defaultProgram && _defaultProgram== _lightProgram->getProgram());
//#endif
//	if (_shadowMapSizeLoc >= 0)
//	{
//		_shadowMapSize = shadowMapSize;
//		glUniform2f(_shadowMapSizeLoc, shadowMapSize.width, shadowMapSize.height);
//	}
//}

void  LightShader::setMVPMatrix(glk::Matrix &mvpMatrix)
{
#ifdef __DEBUG_SHADOW_PROGRAM_
	int _defaultProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &_defaultProgram);
	assert(_defaultProgram && _defaultProgram == _lightProgram->getProgram());
#endif
	if (_mvpMatrixLoc >= 0)
	{
		_mvpMatrix = mvpMatrix;
		glUniformMatrix4fv(_mvpMatrixLoc, 1, GL_FALSE, _mvpMatrix.pointer());
	}
}
void LightShader::setCropMatrix(const glk::Matrix cropMatrix[4],const int size)
{
#ifdef __DEBUG_SHADOW_PROGRAM_
	int _defaultProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &_defaultProgram);
	assert(_defaultProgram && _defaultProgram == _lightProgram->getProgram());
#endif
	if (_cropMatrixLoc >= 0)
	{
		memcpy(&_cropMatrix, cropMatrix, sizeof(glk::Matrix) * size);
		glUniformMatrix4fv(_cropMatrixLoc, size, GL_FALSE, cropMatrix->pointer());
	}
}
//void LightShader::setViewports(const glk::GLVector4 *viewport, const int size)
//{
//#ifdef __DEBUG_SHADOW_PROGRAM_
//	int _defaultProgram;
//	glGetIntegerv(GL_CURRENT_PROGRAM, &_defaultProgram);
//	assert(_defaultProgram && _defaultProgram == _lightProgram->getProgram());
//#endif
//	if (_viewportArrayLoc >= 0)
//	{
//		for (int j = 0; j < size; ++j)
//			_viewportArray[j] = viewport[j];
//		glUniform4fv(_viewportArrayLoc, size, (const float *)viewport);
//	}
//}

void  LightShader::perform()
{
	_lightProgram->perform();
}

