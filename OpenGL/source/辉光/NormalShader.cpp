/*
  *NormalShader.h
 */
#include "GL/glew.h"
#include "NormalShader.h"

NormalShader::NormalShader():
_glProgram(nullptr)
,_mvpMatrixLoc(-1)
,_baseMapLoc(-1)
,_positionLoc(-1)
,_fragCoordLoc(-1)
{

}

NormalShader::~NormalShader()
{
	_glProgram->release();
}

NormalShader *NormalShader::create(const char *vsFIle, const char *fsFile)
{
	NormalShader *shader = new NormalShader();
	shader->initWithFile(vsFIle, fsFile);
	return shader;
}

void NormalShader::initWithFile(const char *vsFile, const char *fsFile)
{
	_glProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_mvpMatrixLoc = _glProgram->getUniformLocation("g_MVPMatrix");
	_baseMapLoc = _glProgram->getUniformLocation("g_BaseMap");
	_positionLoc = _glProgram->getAttribLocation("a_position");
	_fragCoordLoc = _glProgram->getAttribLocation("a_fragCoord");
}

void NormalShader::setMVPMatrix(const glk::Matrix &mvpMatrix)
{
	if (_mvpMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_mvpMatrixLoc, 1, GL_FALSE, mvpMatrix.pointer());
	}
}

void NormalShader::setBaseMap(int textureId, int unit)
{
	if (_baseMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glUniform1i(_baseMapLoc,unit);
	}
}

int NormalShader::getPositionLoc()const
{
	return _positionLoc;
}

int NormalShader::getFragCoordLoc()const
{
	return _fragCoordLoc;
}

void NormalShader::perform()const
{
	_glProgram->perform();
}