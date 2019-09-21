/*
  *计算水面网格的法线
 */
#include "GL/glew.h"
#include "WaterNormalShader.h"

WaterNormalShader::WaterNormalShader() :
	_glProgram(nullptr)
	,_meshUnitSizeLoc(-1)
{

}

WaterNormalShader::~WaterNormalShader()
{
	_glProgram->release();
	_glProgram = nullptr;
}

void WaterNormalShader::initWithFile(const char *csFile)
{
	_glProgram = glk::ComputeShader::createWithFile(csFile);
	_meshUnitSizeLoc = _glProgram->getUniformLocation("g_MeshUnitSize");
}

WaterNormalShader *WaterNormalShader::create(const char *csFile)
{
	WaterNormalShader *shader = new WaterNormalShader();
	shader->initWithFile(csFile);
	return shader;
}

void WaterNormalShader::setMeshUnitSize(float unitSize)const
{
	if (_meshUnitSizeLoc >= 0)
	{
		glUniform1f(_meshUnitSizeLoc,unitSize);
	}
}

void WaterNormalShader::perform()const
{
	_glProgram->perform();
}

void WaterNormalShader::dispatch(int xCount, int yCount)const
{
	_glProgram->dispatch(xCount, yCount,1);
}