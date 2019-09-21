/*
  *RTT实现,
  *@date:2017-6-27
  *@Author:xiaohuaxiong
*/
#include "GL/glew.h"
#include "engine/RenderTexture.h"
#include<assert.h>
#include<unordered_map>
__NS_GLK_BEGIN
//RenderTexture中使用的颜色枚举到OpenGL颜色格式之间的转化
static  std::unordered_map<RenderTexture::ColorFormatType, GLenum>   static_format_to_color_map = {
	{RenderTexture::ColorFormatType::ColorFormatType_R8,GL_RED},
	{RenderTexture::ColorFormatType::ColorFormatType_RG8,GL_RG8},
	{ RenderTexture::ColorFormatType::ColorFormatType_RGB8,GL_RGB8},
	{ RenderTexture::ColorFormatType::ColorFormatType_RGBA8,GL_RGBA8},
	//半浮点
	{ RenderTexture::ColorFormatType::ColorFormatType_R_HALF,GL_R16F},
	{ RenderTexture::ColorFormatType::ColorFormatType_RG_HALF,GL_RG16F},
	{ RenderTexture::ColorFormatType::ColorFormatType_RGB_HALF,GL_RGB16F},
	{ RenderTexture::ColorFormatType::ColorFormatType_RGBA_HALF,GL_RGBA16F},
	//全浮点
	{ RenderTexture::ColorFormatType::ColorFormatType_R_FLOAT,GL_R32F},
	{ RenderTexture::ColorFormatType::ColorFormatType_RG_FLOAT,GL_RG32F},
	{ RenderTexture::ColorFormatType::ColorFormatType_RGB_FLOAT,GL_RGB32F},
	{ RenderTexture::ColorFormatType::ColorFormatType_RGBA_FLOAT,GL_RGBA32F},
};
//RenderTexture使用的深度格式到OpenGL的深度格式之间的转换
static std::unordered_map<RenderTexture::DepthFormatType, GLenum> static_format_to_depth_map = {
	{ RenderTexture::DepthFormatType ::DepthFormatType_Normal,GL_DEPTH_COMPONENT24},
	{ RenderTexture::DepthFormatType ::DepthFormatType_32F,GL_DEPTH_COMPONENT32F},
	{ RenderTexture::DepthFormatType ::DepthFormatType_32,GL_DEPTH_COMPONENT32}
};
//对于以上任何的一种格式,是否代表了浮点格式
static std::unordered_map<int, bool>static_format_is_float_map = {
	//8位分量
	{ RenderTexture::ColorFormatType::ColorFormatType_R8,false },
	{ RenderTexture::ColorFormatType::ColorFormatType_RG8,false },
	{ RenderTexture::ColorFormatType::ColorFormatType_RGB8,false },
	{ RenderTexture::ColorFormatType::ColorFormatType_RGBA8,false },
	//半浮点
	{ RenderTexture::ColorFormatType::ColorFormatType_R_HALF,true },
	{ RenderTexture::ColorFormatType::ColorFormatType_RG_HALF,true },
	{ RenderTexture::ColorFormatType::ColorFormatType_RGB_HALF,true },
	{ RenderTexture::ColorFormatType::ColorFormatType_RGBA_HALF,true },
	//全浮点
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
	//颜色
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
	//检查帧缓冲区对象的完整性
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