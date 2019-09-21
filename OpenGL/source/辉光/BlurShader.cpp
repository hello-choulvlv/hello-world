/*
  *纹理图像模糊实现
  *2017-07-14
  *@Author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "BlurShader.h"
BlurShader::BlurShader():
_glProgram(nullptr)
,_mvpMatrixLoc(-1)
,_baseMapLoc(-1)
,_directionLoc(-1)
,_stepWidthLoc(-1)
{

}

BlurShader::~BlurShader()
{
	_glProgram->release();
	_glProgram = nullptr;
}

void   BlurShader::initWithFile(const char *vsFile, const char *fsFile)
{
	_glProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_mvpMatrixLoc = _glProgram->getUniformLocation("g_MVPMatrix");
	_baseMapLoc = _glProgram->getUniformLocation("g_BaseMap");
	_directionLoc = _glProgram->getUniformLocation("g_Direction");
	_stepWidthLoc = _glProgram->getUniformLocation("g_Width");

	_positionLoc = _glProgram->getAttribLocation("a_position");
	_fragCoordLoc = _glProgram->getAttribLocation("a_fragCoord");
}

BlurShader *BlurShader::create(const char *vsFile, const char *fsFile)
{
	BlurShader *shader = new BlurShader();
	shader->initWithFile(vsFile, fsFile);
	return shader;
}

int BlurShader::getPositionLoc()const
{
	return _positionLoc;
}

int  BlurShader::getFragCoord()const
{
	return _fragCoordLoc;
}

void BlurShader::setMVPMatrix(const glk::Matrix &mvpMatrix)
{
	if (_mvpMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_mvpMatrixLoc, 1, GL_FALSE, mvpMatrix.pointer());
	}
}

void  BlurShader::setBaseMap(int textureId, int unit)
{
	if (_baseMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glUniform1i(_baseMapLoc,unit);
	}
}

void BlurShader::setDirection(const glk::GLVector2 &direction)
{
	if (_directionLoc >= 0)
	{
		glUniform2f(_directionLoc, direction.x, direction.y);
	}
}

void BlurShader::setStepWidth(float width)
{
	if (_stepWidthLoc >= 0)
	{
		glUniform1f(_stepWidthLoc,width);
	}
}

void BlurShader::perform()const
{
	_glProgram->perform();
}

