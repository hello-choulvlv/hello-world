/*
  *�ӳ���ɫʵ��
  *2017��12��22��
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
	//��ɾ����ɫ����������
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
	//����֡����������
	int lastFramebufferId,lastTextureId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING,&lastFramebufferId);
	glGetIntegerv(GL_TEXTURE_BINDING_2D,&lastTextureId);
	//
	glGenFramebuffers(1, &_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER,_framebufferId);
	//������ɫ����������,ע��,ֻ�е�һ����װ����ɫʹ�õ�,��������װ��λ���뷨��,�Լ���������ʹ�õ�
	glGenTextures(colorbufferCount, _colorbufferId);
	unsigned  drawBuffer[4];
	for (int k = 0; k < colorbufferCount; ++k) 
	{
		glBindTexture(GL_TEXTURE_2D,_colorbufferId[k]);
		//ע�������ʽ������
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//ע�����ݵĴ洢��ʽ
		glTexStorage2D(GL_TEXTURE_2D, 1, !k ? GL_RGBA8 : GL_RGBA16F, framebufferSize.width, framebufferSize.height);
		//��
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + k, GL_TEXTURE_2D, _colorbufferId[k], 0);
		drawBuffer[k] = GL_COLOR_ATTACHMENT0 + k;
	}
	glDrawBuffers(_colorbufferCount, drawBuffer);
	//��Ȼ���������
	glGenTextures(1, &_depthbufferId);
	glBindTexture(GL_TEXTURE_2D,_depthbufferId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, framebufferSize.width, framebufferSize.height);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthbufferId, 0);
	//ģ�建��������,��ʱû��ʵ��
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