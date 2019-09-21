/*
 */
#include "GL/glew.h"
#include "RenderShader.h"

RenderShader::RenderShader():
_mvpMatrixLoc(-1)
,_colorLoc(-1)
,_positionLoc(-1)
{

}

RenderShader::~RenderShader()
{
	_glProgram->release();
}

RenderShader *RenderShader::create(const char *vsFile, const char *fsFile)
{
	RenderShader *render = new RenderShader();
	render->initWithFile(vsFile, fsFile);
	return render;
}

void RenderShader::initWithFile(const char *vsFile, const char *fsFile)
{
	_glProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_mvpMatrixLoc = _glProgram->getUniformLocation("g_MVPMatrix");
	_colorLoc = _glProgram->getUniformLocation("g_Color");
	_positionLoc = _glProgram->getAttribLocation("a_position");
}

void RenderShader::setMVPMatrix(const glk::Matrix &mvpMatrix)
{
	if (_mvpMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_mvpMatrixLoc, 1, GL_FALSE, mvpMatrix.pointer());
	}
}

void RenderShader::setColor(const glk::GLVector4 &color)
{
	if (_colorLoc >= 0)
	{
		glUniform4fv(_colorLoc, 1, &color.x);
	}
}

int RenderShader::getPositionLoc()const
{
	return _positionLoc;
}

void RenderShader::perform()const
{
	_glProgram->perform();
}