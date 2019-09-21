/*
  *Ë®äÖÈ¾×ÅÉ«
  *2017-7-26
 */
#include "GL/glew.h"
#include "RenderShader.h"

RenderShader::RenderShader() :
	_glProgram(nullptr)
	,_mvpMatrixLoc(-1)
	,_texCubeMapLoc(-1)
	,_waterColorLoc(-1)
	,_cameraPositionLoc(-1)
	,_freshnelParamLoc(-1)
{

}

RenderShader::~RenderShader()
{
	_glProgram->release();
	_glProgram = nullptr;
}

void RenderShader::initWithFile(const char *vsFile,const char *fsFile)
{
	_glProgram = glk::GLProgram::createWithFile(vsFile, fsFile);
	_mvpMatrixLoc = _glProgram->getUniformLocation("g_ViewProjMatrix");
	_modelMatrixLoc = _glProgram->getUniformLocation("g_ModelMatrix");
	_texCubeMapLoc = _glProgram->getUniformLocation("g_TexCubMap");
	_cameraPositionLoc = _glProgram->getUniformLocation("g_CameraPosition");
	_freshnelParamLoc = _glProgram->getUniformLocation("g_FreshnelParam");
	_waterColorLoc = _glProgram->getUniformLocation("g_WaterColor");

	_positionLoc = _glProgram->getAttribLocation("a_position");
	_normalLoc = _glProgram->getAttribLocation("a_normal");
}

RenderShader *RenderShader::create(const char *vsFile, const char *fsFile)
{
	RenderShader *shader = new RenderShader();
	shader->initWithFile(vsFile, fsFile);
	return shader;
}

void RenderShader::setViewProjMatrix(const glk::Matrix &mvpMatrix)const
{
	if (_mvpMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_mvpMatrixLoc, 1, GL_FALSE, mvpMatrix.pointer());
	}
}

void RenderShader::setModelMatrix(const glk::Matrix &modelMatrix)const
{
	if (_modelMatrixLoc >= 0)
	{
		glUniformMatrix4fv(_modelMatrixLoc, 1, GL_FALSE, modelMatrix.pointer());
	}
}

void RenderShader::setFreshnelPatram(const glk::GLVector3 &freshnelParam)const
{
	if (_freshnelParamLoc >= 0)
	{
		glUniform3fv(_freshnelParamLoc, 1, &freshnelParam.x);
	}
}

void RenderShader::setCameraPosition(const glk::GLVector3 &cameraPosition)const
{
	if (_cameraPositionLoc >= 0)
	{
		glUniform3fv(_cameraPositionLoc, 1, &cameraPosition.x);
	}
}

void RenderShader::setTexCubeMap(int textureId, int unit)const
{
	if (_texCubeMapLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
		glUniform1i(_texCubeMapLoc,unit);
	}
}

void RenderShader::setWaterColor(const glk::GLVector4 &waterColor)const
{
	if (_waterColorLoc >= 0)
	{
		glUniform4fv(_waterColorLoc, 1, &waterColor.x);
	}
}

void RenderShader::perform()const
{
	_glProgram->perform();
}

int RenderShader::getPositionLoc()const
{
	return _positionLoc;
}

int RenderShader::getNormalLoc()const
{
	return _normalLoc;
}