/*
  *RTTʵ��,
  *@date:2017-6-27
  *@Author:xiaohuaxiong
*/
#include "GL/glew.h"
#include "engine/RenderTexture.h"
#include<assert.h>
#include<unordered_map>
__NS_GLK_BEGIN
//RenderTexture��ʹ�õ���ɫö�ٵ�OpenGL��ɫ��ʽ֮���ת��
static  std::unordered_map<RenderTexture::ColorFormatType, GLenum>   static_format_to_color_map = {
	{RenderTexture::ColorFormatType::ColorFormatType_R8,GL_RED},
	{RenderTexture::ColorFormatType::ColorFormatType_RG8,GL_RG8},
	{ RenderTexture::ColorFormatType::ColorFormatType_RGB8,GL_RGB8},
	{ RenderTexture::ColorFormatType::ColorFormatType_RGBA8,GL_RGBA8},
	//�븡��
	{ RenderTexture::ColorFormatType::ColorFormatType_R_HALF,GL_R16F},
	{ RenderTexture::ColorFormatType::ColorFormatType_RG_HALF,GL_RG16F},
	{ RenderTexture::ColorFormatType::ColorFormatType_RGB_HALF,GL_RGB16F},
	{ RenderTexture::ColorFormatType::ColorFormatType_RGBA_HALF,GL_RGBA16F},
	//ȫ����
	{ RenderTexture::ColorFormatType::ColorFormatType_R_FLOAT,GL_R32F},
	{ RenderTexture::ColorFormatType::ColorFormatType_RG_FLOAT,GL_RG32F},
	{ RenderTexture::ColorFormatType::ColorFormatType_RGB_FLOAT,GL_RGB32F},
	{ RenderTexture::ColorFormatType::ColorFormatType_RGBA_FLOAT,GL_RGBA32F},
};
//RenderTextureʹ�õ���ȸ�ʽ��OpenGL����ȸ�ʽ֮���ת��
static std::unordered_map<RenderTexture::DepthFormatType, GLenum> static_format_to_depth_map = {
	{ RenderTexture::DepthFormatType ::DepthFormatType_Normal,GL_DEPTH_COMPONENT24},
	{ RenderTexture::DepthFormatType ::DepthFormatType_32F,GL_DEPTH_COMPONENT32F},
	{ RenderTexture::DepthFormatType ::DepthFormatType_32,GL_DEPTH_COMPONENT32}
};
//���������κε�һ�ָ�ʽ,�Ƿ�����˸����ʽ
static std::unordered_map<int, bool>static_format_is_float_map = {
	//8λ����
	{ RenderTexture::ColorFormatType::ColorFormatType_R8,false },
	{ RenderTexture::ColorFormatType::ColorFormatType_RG8,false },
	{ RenderTexture::ColorFormatType::ColorFormatType_RGB8,false },
	{ RenderTexture::ColorFormatType::ColorFormatType_RGBA8,false },
	//�븡��
	{ RenderTexture::ColorFormatType::ColorFormatType_R_HALF,true },
	{ RenderTexture::ColorFormatType::ColorFormatType_RG_HALF,true },
	{ RenderTexture::ColorFormatType::ColorFormatType_RGB_HALF,true },
	{ RenderTexture::ColorFormatType::ColorFormatType_RGBA_HALF,true },
	//ȫ����
	{ RenderTexture::ColorFormatType::ColorFormatType_R_FLOAT,true },
	{ RenderTexture::ColorFormatType::ColorFormatType_RG_FLOAT,true },
	{ RenderTexture::ColorFormatType::ColorFormatType_RGB_FLOAT,true },
	{ RenderTexture::ColorFormatType::ColorFormatType_RGBA_FLOAT,true },
	//
	{ RenderTexture::DepthFormatType::DepthFormatType_Normal,false },
	{ RenderTexture::DepthFormatType::DepthFormatType_32F,true },
	{ RenderTexture::DepthFormatType::DepthFormatType_32,false }
};
RenderTexture::RenderTexture()
{
	_framebufferId = 0;
	_colorbufferId = 0;
	_depthbufferId = 0;
	_stencilbufferId = 0;
	_lastFramebufferId = 0;
	_isRestoreLastFramebuffer = true;
	_isNeedClearBuffer = true;
	_bufferBit = 0;
}

RenderTexture::~RenderTexture()
{
	if (_framebufferId)
		glDeleteFramebuffers(1,&_framebufferId);
	if (_colorbufferId)
		glDeleteTextures(1, &_colorbufferId);
	if (_depthbufferId)
		glDeleteTextures(1, &_depthbufferId);
	if (_stencilbufferId)
		glDeleteTextures(1, &_stencilbufferId);
}

RenderTexture *RenderTexture::createRenderTexture(const Size &frameSize, unsigned genType, ColorFormatType colorType, DepthFormatType depthType)
{
	RenderTexture *rtt = new RenderTexture();
	if (!rtt->initWithRender(frameSize, genType, colorType,depthType))
	{
		rtt->release();
		rtt = nullptr;
	}
	return rtt;
}

RenderTexture *RenderTexture::createRenderTexture(const Size &frameSize, unsigned genType)
{
	RenderTexture *render = new RenderTexture();
	if (!render->initWithRender(frameSize, genType))
	{
		render->release();
		render = nullptr;
	}
	return render;
}
bool  RenderTexture::initWithRender(const Size &frameSize, unsigned genType)
{
	return initWithRender(frameSize,genType,ColorFormatType::ColorFormatType_RGBA8,DepthFormatType::DepthFormatType_Normal);
}
bool RenderTexture::initWithRender(const Size &frameSize, unsigned genType, ColorFormatType colorType, DepthFormatType depthType)
{
	_frameSize = frameSize;
	assert(frameSize.width>0 && frameSize.height>0);
	int defaultFramebufferId,colorbufferId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFramebufferId);
	if(genType)
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &colorbufferId);
	
	glGenFramebuffers(1, &_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);
	//��ɫ
	if (genType & RenderType::ColorBuffer)
	{
		glGenTextures(1, &_colorbufferId);
		glBindTexture(GL_TEXTURE_2D, _colorbufferId);
		
		glTexStorage2D(GL_TEXTURE_2D, 1, static_format_to_color_map[colorType], frameSize.width, frameSize.height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorbufferId, 0);
		_bufferBit |= GL_COLOR_BUFFER_BIT;
	}
	if (genType & RenderType::DepthBuffer)
	{
		glGenTextures(1, &_depthbufferId);
		glBindTexture(GL_TEXTURE_2D, _depthbufferId);
		glTexStorage2D(GL_TEXTURE_2D, 1,static_format_to_depth_map[depthType], frameSize.width, frameSize.height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthbufferId, 0);
		_bufferBit |= GL_DEPTH_BUFFER_BIT;
	}
	else
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	}
	if (genType & RenderType::StencilBuffer)
	{
		glGenTextures(1, &_stencilbufferId);
		glBindTexture(GL_TEXTURE_2D, _stencilbufferId);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_STENCIL_COMPONENTS, frameSize.width, frameSize.height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_TEXTURE_2D, GL_STENCIL_COMPONENTS, GL_TEXTURE_2D, _stencilbufferId, 0);
		_bufferBit |= GL_STENCIL_BUFFER_BIT;
	}
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//���֡�����������������
	const int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);
	//restore
	if (genType)
		glBindTexture(GL_TEXTURE_2D, colorbufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferId);
	return status == GL_FRAMEBUFFER_COMPLETE;
}

void RenderTexture::activeFramebuffer()
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_lastFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);
	if (_isNeedClearBuffer)
		glClear(_bufferBit);
}

void RenderTexture::disableFramebuffer()const
{
	if (_isRestoreLastFramebuffer)
		glBindFramebuffer(GL_FRAMEBUFFER, _lastFramebufferId);
}

void RenderTexture::setRestoreLastFramebuffer(bool b)
{
	_isRestoreLastFramebuffer = b;
}

bool RenderTexture::isRestoreLastFramebuffer()const
{
	return _isRestoreLastFramebuffer;
}

void RenderTexture::setClearBuffer(bool b)
{
	_isNeedClearBuffer = b;
}

bool RenderTexture::isClearBuffer()const
{
	return _isNeedClearBuffer;
}

unsigned RenderTexture::getColorBuffer()const
{
	return _colorbufferId;
}

unsigned RenderTexture::getDepthBuffer()const
{
	return _depthbufferId;
}

__NS_GLK_END