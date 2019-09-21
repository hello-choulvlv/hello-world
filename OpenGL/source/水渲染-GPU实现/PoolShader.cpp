/*
  *2017-8-3
  *@Author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "PoolShader.h"

PoolShader::PoolShader() :
	_glProgram(nullptr)
	,_mVPMatrixLoc(-1)
	,_texCubeMapLoc(-1)
	,_positionLoc(-1)
	,_fragCoordLoc(-1)
{

}

PoolShader::~PoolShader()
{
	_glProgram->release();
	_glProgram = nullptr;
}

PoolShader *PoolShader::create(const char *vsFile, const char *fsFile)
{
	PoolShader *shader = new PoolShader();
	shader->initWithFile(vsFile, fsFile);
	return shader;
}

void		PoolShader::initWithFile(const char *vsFile, const char *fsFile)
{
	_glProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_mVPMatrixLoc = _glProgram->getUniformLocation("g_MVPMatrix");
	_texCubeMapLoc = _glProgram->getUniformLocation("g_TexCubeMap");
	_photonCubeMapLoc = _glProgram->getUniformLocation("g_PhotonCubeMap");
	_kernelLoc = _glProgram->getUniformLocation("g_Kernel");
	_positionLoc = _glProgram->getAttribLocation("a_position");
	_fragCoordLoc = _glProgram->getAttribLocation("a_fragCoord");
}

void		PoolShader::setMVPMatrix(const glk::Matrix &mVPMatrix)
{
	if (_mVPMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_mVPMatrixLoc, 1, GL_FALSE, mVPMatrix.pointer());
	}
}

void		PoolShader::setTexCubeMap(int texCubeMapId, int unit)
{
	if (_texCubeMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0+unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP,texCubeMapId);
		glUniform1i(_texCubeMapLoc,unit);
	}
}

void PoolShader::setCausticCubeMap(int texCubeMapId, int unit)
{
	if (_photonCubeMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP,texCubeMapId);
		glUniform1i(_photonCubeMapLoc, unit);
	}
}

void PoolShader::setKernel(const glk::GLVector3 *kernel, int size)
{
	if (_kernelLoc >= 0)
	{
		glUniform3fv(_kernelLoc,size,&kernel->x);
	}
}

void		PoolShader::perform()
{
	_glProgram->perform();
}

int		PoolShader::getPositionLoc()
{
	return _positionLoc;
}

int		PoolShader::getFragCoordLoc()
{
	return _fragCoordLoc;
}