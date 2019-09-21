/*
  *水渲染/计算着色器实现
  *2017-7-26
  *@Author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "WaterComputeShader.h"

WaterComputeShader::WaterComputeShader() :
	_glProgram(nullptr)
	,_waterMeshLoc(-1)
	,_waterParamLoc(-1)
	,_waterResolutionHalfLoc(-1)
{

}

WaterComputeShader::~WaterComputeShader()
{
	_glProgram->release();
	_glProgram = nullptr;
}

void  WaterComputeShader::initWithFile(const char *csFile)
{
	_glProgram = glk::ComputeShader::createWithFile(csFile);
	_waterParamLoc= _glProgram->getUniformLocation("g_WaveParam");
	_waterResolutionHalfLoc = _glProgram->getUniformLocation("g_WaveResolutionHalf");
	_waterMeshLoc = _glProgram->getUniformLocation("g_MeshSize");
}

WaterComputeShader *WaterComputeShader::create(const char *csFile)
{
	WaterComputeShader *shader = new WaterComputeShader();
	shader->initWithFile(csFile);
	return shader;
}

void WaterComputeShader::setWaterParam(const glk::GLVector4 &param)const
{
	if (_waterParamLoc >= 0)
	{
		glUniform4fv(_waterParamLoc, 1, &param.x);
	}
}

void WaterComputeShader::setWaterResolutionHalf(const glk::GLVector2 &halfResolution)const
{
	if (_waterResolutionHalfLoc >= 0)
	{
		glUniform2f(_waterResolutionHalfLoc,halfResolution.x,halfResolution.y);
	}
}

void WaterComputeShader::setWaterMeshSize(int meshSize)const
{
	if (_waterMeshLoc >= 0)
	{
		glUniform1i(_waterMeshLoc,meshSize);
	}
}

void WaterComputeShader::perform()const
{
	_glProgram->perform();
}

void WaterComputeShader::dispatch(int xCount, int yCount)
{
	_glProgram->dispatch(xCount, yCount, 1);
}