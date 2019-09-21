/*
  *2017-8-2
  *@Author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "WaterNormalShader.h"

WaterNormalShader::WaterNormalShader() :
	_glProgram(nullptr)
	,_baseMapLoc(-1)
	,_meshIntervalLoc(-1)
	,_positionLoc(-1)
	,_fragCoordLoc(-1)
{

}

WaterNormalShader::~WaterNormalShader()
{
	_glProgram->release();
	_glProgram = nullptr;
}

WaterNormalShader *WaterNormalShader::create(const char *vsFile, const char *fsFile)
{
	WaterNormalShader *shader = new WaterNormalShader();
	shader->initWithFile(vsFile, fsFile);
	return shader;
}

void WaterNormalShader::initWithFile(const char *vsFile, const char *fsFile)
{
	_glProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_baseMapLoc = _glProgram->getUniformLocation("g_BaseMap");
	_meshIntervalLoc = _glProgram->getUniformLocation("g_MeshInterval");
	_positionLoc = _glProgram->getAttribLocation("a_position");
	_fragCoordLoc = _glProgram->getAttribLocation("a_fragCoord");
}

void WaterNormalShader::setBaseMap(int textureId, int unit)
{
	if (_baseMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0+unit);
		glBindTexture(GL_TEXTURE_2D,textureId);
		glUniform1i(_baseMapLoc,unit);
	}
}

void WaterNormalShader::setMeshInterval(float meshInterval)
{
	if (_meshIntervalLoc >= 0)
	{
		glUniform1f(_meshIntervalLoc,meshInterval);
	}
}

int WaterNormalShader::getPositionLoc()const
{
	return _positionLoc;
}

int WaterNormalShader::getFragCoordLoc()const
{
	return _fragCoordLoc;
}

void WaterNormalShader::perform()const
{
	_glProgram->perform();
}