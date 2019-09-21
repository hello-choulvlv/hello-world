/*
  *Í¼ÏñÄ£ºý´¦Àí
  *2017-8-23
  *@Author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "FuzzyShader.h"
__US_GLK__;
FuzzyShader::FuzzyShader() :
	_glProgram(nullptr)
	,_baseMapLoc(-1)
	,_pixelStepLoc(-1)
{

}

FuzzyShader::~FuzzyShader()
{
	_glProgram->release();
	_glProgram = nullptr;
}

FuzzyShader  *FuzzyShader::create(const char *vsFile, const char *fsFile)
{
	FuzzyShader *shader = new FuzzyShader();
	shader->init(vsFile, fsFile);
	return shader;
}

void     FuzzyShader::init(const char *vsFile, const char *fsFile)
{
	_glProgram = GLProgram::createWithFile(vsFile, fsFile);
	_baseMapLoc = _glProgram->getUniformLocation("g_BaseMap");
	_pixelStepLoc = _glProgram->getUniformLocation("g_PixelStep");
}

void		FuzzyShader::setBaseMap(int textureId, int unit)
{
	if (_baseMapLoc>=0)
	{
		glActiveTexture(GL_TEXTURE0+unit);
		glBindTexture(GL_TEXTURE_2D,textureId);
		glUniform1i(_baseMapLoc,unit);
	}
}

void		FuzzyShader::setPixelStep(const glk::GLVector2 &pixelStep)
{
	if (_pixelStepLoc >= 0)
	{
		glUniform2fv(_pixelStepLoc,1,&pixelStep.x);
	}
}

void  FuzzyShader::perform()
{
	_glProgram->perform();
}