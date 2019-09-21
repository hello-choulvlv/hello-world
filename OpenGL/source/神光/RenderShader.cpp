/*
  *RenderShader µœ÷
  *2017-06-28
  */
#include<GL/glew.h>
#include "RenderShader.h"
#include<assert.h>
RenderShader::RenderShader()
{
	_glProgram = nullptr;
	_modelViewProjMatrixLoc = -1;
	_baseMapLoc = -1;
	_positionLoc = -1;
	_fragCoordLoc = -1;
}

RenderShader::~RenderShader()
{
	_glProgram->release();
}

RenderShader *RenderShader::createRenderShader(const char *vsFile, const char *fsFile)
{
	RenderShader *render = new RenderShader();
	render->initWithFile(vsFile, fsFile);
	return render;
}

void RenderShader::initWithFile(const char *vsFile, const char *fsFile)
{
	_glProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_modelViewProjMatrixLoc = _glProgram->getUniformLocation("g_ModelViewProjMatrix");
	_baseMapLoc = _glProgram->getUniformLocation("g_BaseMap");
	_positionLoc = _glProgram->getAttribLocation("a_position");
	_fragCoordLoc = _glProgram->getAttribLocation("a_fragCoord");
}

void RenderShader::setMVPMatrix(const glk::Matrix &mvp)
{
	if (_modelViewProjMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_modelViewProjMatrixLoc, 1, GL_FALSE,mvp.pointer());
	}
}

void RenderShader::setBaseMap(int texture, int unit)
{
	assert(texture);
	if (_baseMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(_baseMapLoc,unit);
	}
}

int RenderShader::getPositionLoc()const
{
	return _positionLoc;
}

int RenderShader::getFragCoordLoc()const
{
	return _fragCoordLoc;
}

void RenderShader::perform()const
{
	_glProgram->perform();
}