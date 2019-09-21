/*
  *延迟着色实现
  *2017年12月22日
  *@Author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "engine/DefferedShader.h"
#include <assert.h>
#include<stdio.h>
__NS_GLK_BEGIN

DefferedShader::DefferedShader():
	_framebufferId(0)
	,_lastFramebufferId(0)
	,_colorbufferId()
	,_colorbufferCount(0)
	,_depthbufferId(0)
	,_stenclebufferId(0)
	,_bufferBit(0)
	, _cleanBuffer(true)
	, _needRestoreLastFramebuffer(true)
{

}

DefferedShader::~DefferedShader()
{
	//先删除颜色缓冲区对象
	glDeleteTextures(_colorbufferCount, _colorbufferId);
	_colorbufferCount = 0;
	_colorbufferId[0] = 0;
	//
	glDeleteTextures(1, &_depthbufferId);
	_depthbufferId = 0;
	//
	glDeleteTextures(1, &_stenclebufferId);
	_stenclebufferId = 0;
	//
	glDeleteFramebuffers(1, &_framebufferId);
}

bool DefferedShader::init(const Size &framebufferSize, int colorbufferCount)
{
	assert(framebufferSize.width>0 && framebufferSize.height>0 && colorbufferCount>0 && colorbufferCount<5);
	_framebufferSize = framebufferSize;
	_colorbufferCount = colorbufferCount;
	//生成帧缓冲区对象
	int lastFramebufferId,lastTextureId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING,&lastFramebufferId);
	glGetIntegerv(GL_TEXTURE_BINDING_2D,&lastTextureId);
	//
	glGenFramebuffers(1, &_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER,_framebufferId);
	//产生颜色缓冲区对象,注意,只有第一个是装载颜色使用的,后续的是装载位置与法线,以及其他数据使用的
	glGenTextures(colorbufferCount, _colorbufferId);
	unsigned  drawBuffer[4];
	for (int k = 0; k < colorbufferCount; ++k) 
	{
		glBindTexture(GL_TEXTURE_2D,_colorbufferId[k]);
		//注意采样方式的设置
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//注意数据的存储方式
		glTexStorage2D(GL_TEXTURE_2D, 1, !k ? GL_RGBA8 : GL_RGBA16F, framebufferSize.width, framebufferSize.height);
		//绑定
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + k, GL_TEXTURE_2D, _colorbufferId[k], 0);
		drawBuffer[k] = GL_COLOR_ATTACHMENT0 + k;
	}
	glDrawBuffers(_colorbufferCount, drawBuffer);
	//深度缓冲区对象
	glGenTextures(1, &_depthbufferId);
	glBindTexture(GL_TEXTURE_2D,_depthbufferId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, framebufferSize.width, framebufferSize.height);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthbufferId, 0);
	//模板缓冲区对象,暂时没有实现
	//check
	int checkStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
#if _DEBUG
	assert(checkStatus == GL_FRAMEBUFFER_COMPLETE);
#else
	if (checkStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Gen frambuffer error:%d\n",checkStatus);
	}
#endif
	//
	glBindTexture(GL_TEXTURE_2D,lastTextureId);
	glBindFramebuffer(GL_FRAMEBUFFER, lastFramebufferId);

	return checkStatus == GL_FRAMEBUFFER_COMPLETE;
}

DefferedShader *DefferedShader::create(const Size &framebufferSize, int colorbufferCount)
{
	DefferedShader *shader = new DefferedShader();
	if (shader->init(framebufferSize, colorbufferCount))
		return shader;
	shader->release();
	shader = nullptr;
	return shader;
}

void DefferedShader::active()
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING,&_lastFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);
	if (_cleanBuffer)
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void DefferedShader::restore()
{
	if(_needRestoreLastFramebuffer)
		glBindFramebuffer(GL_FRAMEBUFFER, _lastFramebufferId);
}

void DefferedShader::getColorBuffer(unsigned *colorbuffer, int *bufferCount)const
{
	for (int k = 0; k < _colorbufferCount; ++k)
		colorbuffer[k] = _colorbufferId[k];
	*bufferCount = _colorbufferCount;
}

unsigned DefferedShader::getColorBuffer()const
{
	return _colorbufferId[0];
}

__NS_GLK_END