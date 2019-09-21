/* 
  *Hdr + Mrt 实现
 */
#include<GL/glew.h>
#include<assert.h>
#include<stdlib.h>
#include "HdrMrt.h"

HdrMrt::HdrMrt()
{
	_hdr_mrt_framebufferId = 0;
	_origin_framebufferId = 0;
	_normal_textureId = 0;
	_hdr_textureId = 0;
	_depth_textureId = 0;
	_framebuffer_width = 0;
	_framebuffer_height = 0;
}

HdrMrt::~HdrMrt()
{
	glDeleteFramebuffers(1, &_hdr_mrt_framebufferId);
	_hdr_mrt_framebufferId = 0;
	glDeleteTextures(1, &_normal_textureId);
	_normal_textureId = 0;
	glDeleteTextures(1, &_hdr_textureId);
	_hdr_textureId = 0;
	glDeleteRenderbuffers(1, &_depth_textureId);
	_depth_textureId = 0;
}

void     HdrMrt::initHdrMrt(int width, int height)
{
	_framebuffer_width = width;
	_framebuffer_height = height;

	int        defaultFramebufferId,defaultTextureId,defaultRenderbufferId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING,&defaultFramebufferId);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &defaultTextureId);
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &defaultRenderbufferId);
	glGenFramebuffers(1, &_hdr_mrt_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _hdr_mrt_framebufferId);
//生成两阶段纹理对象,并绑定到帧缓冲区对象上
	glGenTextures(1, &_normal_textureId);
	glGenTextures(1, &_hdr_textureId);
	int     _textureId[2] = {_normal_textureId,_hdr_textureId};
	for (int i = 0; i < 2; ++i)
	{
		glBindTexture(GL_TEXTURE_2D,_textureId[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//绑定到帧缓冲区对象
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _textureId[i], 0);
	}
	glGenRenderbuffers(1, &_depth_textureId);
	glBindRenderbuffer(GL_RENDERBUFFER, _depth_textureId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_textureId);
	const   GLenum    colorAttach[2] = {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, colorAttach);
//检查生成的framebuffer对象是否完整
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);
//还原真缓冲区对象,2D纹理对象
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferId);
	glBindTexture(GL_TEXTURE_2D, defaultTextureId);
	glBindRenderbuffer(GL_RENDERBUFFER, defaultRenderbufferId);
}

HdrMrt		*HdrMrt::createHdrMrt(int width, int height)
{
	HdrMrt		*_hdr = new   HdrMrt();
	_hdr->initHdrMrt(width, height);
	return  _hdr;
}

unsigned        HdrMrt::normalTexture()
{
	return _normal_textureId;
}

unsigned       HdrMrt::hdrTexture()
{
	return   _hdr_textureId;
}

void            HdrMrt::activateFramebuffer()
{
//	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (int *)&_origin_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _hdr_mrt_framebufferId);
}

void         HdrMrt::resumeFramebuffer()
{
//	glBindFramebuffer(GL_FRAMEBUFFER_BINDING,_origin_framebufferId);
}