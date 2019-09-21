/*
  *水渲染GPU实现-CPP外围封装
  *2017-8-3
  *@Author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "WaterShader.h"

WaterShader::WaterShader() :
	_glProgram(nullptr)
	,_modelMatrixLoc(-1)
	,_viewProjMatrixLoc(-1)
	,_normalMatrixLoc(-1)
	,_heightMapLoc(-1)
	,_normalMapLoc(-1)
	,_texCubeMapLoc(-1)
	,_cubeMapNormalLoc(-1)
	,_cameraPositionLoc(-1)
	,_lightPositionLoc(-1)
	,_halfCubeHeightLoc(-1)
	,_positionLoc(-1)
	,_fragCoordLoc(-1)
{

}

WaterShader::~WaterShader()
{
	_glProgram->release();
	_glProgram = nullptr;
}

WaterShader *WaterShader::create(const char *vsFile, const char *fsFile)
{
	WaterShader *shader = new WaterShader();
	shader->initWithFile(vsFile, fsFile);
	return shader;
}

void WaterShader::initWithFile(const char *vsFile, const char *fsFile)
{
	_glProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_modelMatrixLoc = _glProgram->getUniformLocation("g_ModelMatrix");
	_viewProjMatrixLoc = _glProgram->getUniformLocation("g_ViewProjMatrix");
	_normalMatrixLoc = _glProgram->getUniformLocation("g_NormalMatrix");
	_heightMapLoc = _glProgram->getUniformLocation("g_HeightMap");
	_normalMapLoc = _glProgram->getUniformLocation("g_NormalMap");
	_texCubeMapLoc = _glProgram->getUniformLocation("g_TexCubeMap");
	_cubeMapNormalLoc = _glProgram->getUniformLocation("g_CubeMapNormal");
	_cameraPositionLoc = _glProgram->getUniformLocation("g_CameraPosition");
	_lightPositionLoc = _glProgram->getUniformLocation("g_LightPosition");
	_halfCubeHeightLoc = _glProgram->getUniformLocation("g_HalfCubeHeight");
	_waterHeightLoc = _glProgram->getUniformLocation("g_WaterHeight");

	_positionLoc = _glProgram->getAttribLocation("a_position");
	_fragCoordLoc = _glProgram->getAttribLocation("a_fragCoord");
}

void		WaterShader::setModelMatrix(const glk::Matrix &modelMatrix)
{
	if (_modelMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_modelMatrixLoc, 1, GL_FALSE, modelMatrix.pointer());
	}
}

void		WaterShader::setViewProjMatrix(const glk::Matrix &viewProjMatrix)
{
	if (_viewProjMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_viewProjMatrixLoc, 1, GL_FALSE, viewProjMatrix.pointer());
	}
}

//void		WaterShader::setNormalMatrix(const glk::Matrix3 &normalMatrix)
//{
//	if (_normalMatrixLoc >= 0)
//	{
//		glUniformMatrix3fv(_normalMatrixLoc, 1, GL_FALSE, normalMatrix.pointer());
//	}
//}

void		WaterShader::setHeightMap(int heightMapId, int unit)
{
	if (_heightMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D,heightMapId);
		glUniform1i(_heightMapLoc,unit);
	}
}

void		WaterShader::setNormalMap(int normalMapId, int unit)
{
	if (_normalMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, normalMapId);
		glUniform1i(_normalMapLoc, unit);
	}
}

void		WaterShader::setTexCubeMap(int texCubeMapId, int unit)
{
	if (_texCubeMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0+unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP,texCubeMapId);
		glUniform1i(_texCubeMapLoc,unit);
	}
}

void		WaterShader::setCubeMapNormal(const glk::GLVector3 *cubeMapNormal, int size)
{
	if (_cubeMapNormalLoc >= 0)
	{
		glUniform3fv(_cubeMapNormalLoc, size, &cubeMapNormal->x);
	}
}

void		WaterShader::setCameraPosition(const glk::GLVector3 &cameraPosition)
{
	if (_cameraPositionLoc >= 0)
	{
		glUniform3fv(_cameraPositionLoc,1,&cameraPosition.x);
	}
}

void		WaterShader::setLightDirection(const glk::GLVector3 &lightDirection)
{
	if (_lightPositionLoc >= 0)
	{
		glUniform3fv(_lightPositionLoc, 1, &lightDirection.x);
	}
}

void		WaterShader::setHalfCubeHeight(float halfCubeHeight)
{
	if (_halfCubeHeightLoc >= 0)
	{
		glUniform1f(_halfCubeHeightLoc,halfCubeHeight);
	}
}

void   WaterShader::setWaterHeight(float waterHeight)
{
	if (_waterHeightLoc >= 0)
	{
		glUniform1f(_waterHeightLoc, waterHeight);
	}
}

void		WaterShader::perform()
{
	_glProgram->perform();
}

int		WaterShader::getPositionLoc()
{
	return _positionLoc;
}

int		WaterShader::getFragCoordLoc()
{
	return _fragCoordLoc;
}