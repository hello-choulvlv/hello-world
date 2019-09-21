/*
  *生成辉光的Shader
  *2017-07-17
  *@Author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "HaloShader.h"

HaloShader::HaloShader():
_glProgram(nullptr)
,_mvpMatrixLoc(-1)
,_highTextureLoc(-1)
,_lowTextureLoc(-1)
,_baseTextureLoc(-1)
,_dispersalLoc(-1)
,_haloWidthLoc(-1)
,_intensityLoc(-1)
,_sunProjPositionLoc(-1)
,_distortionLoc(-1)
,_positionLoc(-1)
,_fragCoordLoc(-1)
{

}

HaloShader::~HaloShader()
{
	_glProgram->release();
	_glProgram = nullptr;
}

HaloShader *HaloShader::create(const char *vsFile, const char *fsFile)
{
	HaloShader *shader = new HaloShader();
	shader->initWithFile(vsFile, fsFile);
	return shader;
}

void HaloShader::initWithFile(const char *vsFile, const char *fsFile)
{
	_glProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_mvpMatrixLoc = _glProgram->getUniformLocation("g_MVPMatrix");
	_highTextureLoc = _glProgram->getUniformLocation("g_HighSunTexture");
	_lowTextureLoc = _glProgram->getUniformLocation("g_LowSunTexture");
	_baseTextureLoc = _glProgram->getUniformLocation("g_BaseMap");
	_dispersalLoc = _glProgram->getUniformLocation("g_Dispersal");
	_haloWidthLoc = _glProgram->getUniformLocation("g_HaloWidth");
	_intensityLoc = _glProgram->getUniformLocation("g_Intensity");
	_sunProjPositionLoc = _glProgram->getUniformLocation("g_SunProjPosition");
	_distortionLoc = _glProgram->getUniformLocation("g_Distortion");
	_positionLoc = _glProgram->getAttribLocation("a_position");
	_fragCoordLoc = _glProgram->getAttribLocation("a_fragCoord");
}

int HaloShader::getPositionLoc()const
{
	return _positionLoc;
}

int HaloShader::getFragCoordLoc()const
{
	return _fragCoordLoc;
}

void HaloShader::setMVPMatrix(const glk::Matrix &mvp)
{
	if (_mvpMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_mvpMatrixLoc, 1, GL_FALSE, mvp.pointer());
	}
}

void HaloShader::setHighTexture(int textureId, int unit)
{
	if (_highTextureLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glUniform1i(_highTextureLoc,unit);
	}
}

void HaloShader::setLowTexture(int textureId, int unit)
{
	if (_lowTextureLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glUniform1i(_lowTextureLoc,unit);
	}
}

void HaloShader::setBaseTexture(int textureId, int unit)
{
	if (_baseTextureLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glUniform1i(_baseTextureLoc,unit);
	}
}

void HaloShader::setDispersal(float dispersal)
{
	if (_dispersalLoc >= 0)
	{
		glUniform1f(_dispersalLoc, dispersal);
	}
}

void HaloShader::setHaloWidth(float haloWidth)
{
	if (_haloWidthLoc >= 0)
	{
		glUniform1f(_haloWidthLoc, haloWidth);
	}
}

void HaloShader::setHaloIntensity(float intensity)
{
	if (_intensityLoc >= 0)
	{
		glUniform1f(_intensityLoc,intensity);
	}
}

void HaloShader::setSunProjPosition(const glk::GLVector2 &projPosition)
{
	if (_sunProjPositionLoc >= 0)
	{
		glUniform2f(_sunProjPositionLoc, projPosition.x, projPosition.y);
	}
}

void HaloShader::setDistortion(const glk::GLVector3 &distortion)
{
	if (_distortionLoc >= 0)
	{
		glUniform3fv(_distortionLoc, 1, &distortion.x);
	}
}

void HaloShader::perform()const
{
	_glProgram->perform();
}