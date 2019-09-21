/*
  *2017-8-2
  *@Author:xiaohuaxiong
*/
#include "GL/glew.h"
#include "WaterHeightShader.h"

WaterHeightShader::WaterHeightShader() :
	_glProgram(nullptr)
	,_baseMapLoc(-1)
	,_waterParamLoc(-1)
	,_meshSizeLoc(-1)
	,_positionLoc(-1)
	,_fragCoordLoc(-1)
{

}

WaterHeightShader::~WaterHeightShader()
{
	_glProgram->release();
}

void WaterHeightShader::initWithFile(const char *vsFile, const char *fsFile)
{
	_glProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_baseMapLoc = _glProgram->getUniformLocation("g_BaseMap");
	_waterParamLoc = _glProgram->getUniformLocation("g_WaterParam");
	_meshSizeLoc = _glProgram->getUniformLocation("g_MeshSize");
	_positionLoc = _glProgram->getAttribLocation("a_position");
	_fragCoordLoc = _glProgram->getAttribLocation("a_fragCoord");
}

WaterHeightShader *WaterHeightShader::create(const char *vsFile, const char *fsFile)
{
	WaterHeightShader *shader = new WaterHeightShader();
	shader->initWithFile(vsFile, fsFile);
	return shader;
}

void WaterHeightShader::setBaseMap(int textureId, int unit)
{
	if (_baseMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0+unit);
		glBindTexture(GL_TEXTURE_2D,textureId);
		glUniform1i(_baseMapLoc,unit);
	}
}

void WaterHeightShader::setWaterParam(const glk::GLVector4 &waterParam)
{
	if (_waterParamLoc >= 0)
	{
		glUniform4fv(_waterParamLoc, 1, &waterParam.x);
	}
}

void WaterHeightShader::setMeshSize(glk::GLVector2 &meshSize)
{
	if (_meshSizeLoc >= 0)
	{
		glUniform2fv(_meshSizeLoc,1,&meshSize.x);
	}
}

void WaterHeightShader::perform()const
{
	_glProgram->perform();
}

int WaterHeightShader::getPositionLoc()
{
	return _positionLoc;
}

int WaterHeightShader::getFragCoordLoc()
{
	return _fragCoordLoc;
}