/*
  *µØĞÎµÄäÖÈ¾Shader
  *2017-6-8
  *@Author:xiaohuaxiong
 */
#include <GL/glew.h>
#include "TerrainShader.h"

TerrainShader::TerrainShader()
{
	_modelMatrixLoc = -1;
	_viewProjMatrixLoc = -1;
	_normalMatrixLoc = -1;
	_eyePositionLoc = -1;
	_colorLoc = -1;
	_lightColorLoc = -1;
	_lightDirectionLoc = -1;
	_terrainProgram = nullptr;
}

TerrainShader::~TerrainShader()
{
	_terrainProgram->release();
	_terrainProgram = nullptr;
}

void TerrainShader::initWithFile(const char *vsFile, const char *fsFile)
{
	_terrainProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_modelMatrixLoc = _terrainProgram->getUniformLocation("g_modelMatrix");
	_viewProjMatrixLoc = _terrainProgram->getUniformLocation("g_viewProjMatrix");
	_normalMatrixLoc = _terrainProgram->getUniformLocation("g_normalMatrix");
	_eyePositionLoc = _terrainProgram->getUniformLocation("g_eyePosition");
	_colorLoc = _terrainProgram->getUniformLocation("g_color");
	_lightColorLoc = _terrainProgram->getUniformLocation("g_lightColor");
	_lightDirectionLoc = _terrainProgram->getUniformLocation("g_lightDirection");
	_positionLoc = _terrainProgram->getAttribLocation("a_position");
	_normalLoc = _terrainProgram->getAttribLocation("a_normal");
}

TerrainShader  *TerrainShader::createTerrainShader(const char *vsFile, const char *fsFile)
{
	TerrainShader *shader = new TerrainShader();
	shader->initWithFile(vsFile, fsFile);
	return shader;
}

void TerrainShader::setModelMatrix(const glk::Matrix &modelMatrix)
{
	if (_modelMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_modelMatrixLoc, 1, GL_FALSE, modelMatrix.pointer());
	}
}

void TerrainShader::setViewProjMatrix(const glk::Matrix &viewProjMatrix)
{
	if (_viewProjMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_viewProjMatrixLoc, 1, GL_FALSE, viewProjMatrix.pointer());
	}
}

void TerrainShader::setNormalMatrix(const glk::Matrix3 &normalMatrix)
{
	if (_normalMatrixLoc >= 0)
	{
		glUniformMatrix3fv(_normalMatrixLoc, 1, GL_FALSE, normalMatrix.pointer());
	}
}

void TerrainShader::setEyePosition(const glk::GLVector3 &eyePosition)
{
	if (_eyePositionLoc >= 0)
	{
		glUniform3fv(_eyePositionLoc, 1, &eyePosition.x);
	}
}

void TerrainShader::setTerrainColor(const glk::GLVector4 &color)
{
	if (_colorLoc >= 0)
	{
		glUniform4fv(_colorLoc, 1, &color.x);
	}
}

void  TerrainShader::setLightColor(const glk::GLVector4 &lightColor)
{
	if (_lightColorLoc >= 0)
	{
		glUniform4fv(_lightColorLoc, 1, &lightColor.x);
	}
}

void TerrainShader::setLightDirection(const glk::GLVector3 &lightDirection)
{
	if (_lightDirectionLoc >= 0)
	{
		glUniform3fv(_lightDirectionLoc, 1,&lightDirection.x);
	}
}

int   TerrainShader::getPositionLoc()const
{
	return _positionLoc;
}

int TerrainShader::getNormalLoc()const
{
	return _normalLoc;
}

void  TerrainShader::perform()const
{
	_terrainProgram->perform();
}