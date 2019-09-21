/*
  *Caustic
  *@2017-8-23
  *@Author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "CausticShader.h"

CausticShader::CausticShader() :
	_glProgram(nullptr)
	, _waterHeightMapLoc(-1)
	, _waterNormalMapLoc(-1)
	, _modeMatrixLoc(-1)
	, _groundHeightLoc(-1)
	, _waterHeightLoc(-1)
	, _lightDirectionLoc(-1)
	, _resolutionLoc(-1)
	, _groundMapLoc(-1)
{

}

CausticShader::~CausticShader()
{
	_glProgram->release();
	_glProgram = nullptr;
}

CausticShader		*CausticShader::create(const char *vsFile, const char *fsFile)
{
	CausticShader *shader = new CausticShader();
	shader->init(vsFile, fsFile);
	return shader;
}

void   CausticShader::init(const char *vsFile, const char *fsFile)
{
	_glProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_modeMatrixLoc = _glProgram->getUniformLocation("g_ModelMatrix");
	_waterHeightMapLoc = _glProgram->getUniformLocation("g_WaterHeightMap");
	_waterNormalMapLoc = _glProgram->getUniformLocation("g_WaterNormalMap");
	_groundHeightLoc = _glProgram->getUniformLocation("g_GroundHeight");
	_waterHeightLoc = _glProgram->getUniformLocation("g_WaterHeight");
	_lightDirectionLoc = _glProgram->getUniformLocation("g_LightDirection");
	_resolutionLoc = _glProgram->getUniformLocation("g_Resolution");
	_groundMapLoc = _glProgram->getUniformLocation("g_GroundMap");
}

void   CausticShader::setModelMatrix(const glk::Matrix &modelMatrix)
{
	if (_modeMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_modeMatrixLoc, 1, GL_FALSE, modelMatrix.pointer());
	}
}

void   CausticShader::setWaterHeightMap(int textureId, int unit)
{
	if (_waterHeightMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0+unit);
		glBindTexture(GL_TEXTURE_2D,textureId);
		glUniform1i(_waterHeightMapLoc,unit);
	}
}

void  CausticShader::setWaterNormalMap(int textureId, int unit)
{
	if (_waterNormalMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D,textureId);
		glUniform1i(_waterNormalMapLoc,unit);
	}
}

void  CausticShader::setGroundHeight(float groundHeight)
{
	if (_groundHeightLoc >= 0)
	{
		glUniform1f(_groundHeightLoc,groundHeight);
	}
}

void CausticShader::setGroundMap(int textureId, int unit)
{
	if (_groundMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D,textureId);
		glUniform1i(_groundMapLoc,unit);
	}
}

void  CausticShader::setLightDirection(const glk::GLVector3 &lightDirection)
{
	if (_lightDirectionLoc >= 0)
	{
		glUniform3fv(_lightDirectionLoc, 1, &lightDirection.x);
	}
}

void CausticShader::setResolution(const glk::GLVector2 &resolution)
{
	if (_resolutionLoc >= 0)
	{
		glUniform2f(_resolutionLoc,resolution.x,resolution.y);
	}
}

void  CausticShader::setWaterHeight(float waterHeight)
{
	if (_waterHeightLoc >= 0)
	{
		glUniform1f(_waterHeightLoc,waterHeight);
	}
}

void CausticShader::perform()const
{
	_glProgram->perform();
}