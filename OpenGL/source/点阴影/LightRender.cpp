/*
  *产生深度纹理的阴影实现
  *2016-10-21 19:49:40
  *@Author:小花熊
  */
#include<assert.h>
#include<GL/glew.h>
#include<engine/GLContext.h>
#include "LightRender.h"
LightRender::LightRender()
{
	_glProgram = GLProgram::createWithFile("shader/shadow/shadow.vsh", "shader/shadow/shadow.fsh");
	_mvpMatrixLoc = _glProgram->getUniformLocation("u_lightMatrix");

	int       _default_framebufferId;
	int       _default_texture_textureId;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &_default_framebufferId);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &_default_texture_textureId);

	Size      _size = GLContext::getInstance()->getWinSize();
	glGenFramebuffers(1, &_framebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _framebufferId);
	glGenTextures(1, &_depthTextureId);
	glBindTexture(GL_TEXTURE_2D, _depthTextureId);
	glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,_size.width,_size.height,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTextureId, 0);
//禁止写入颜色
	glDrawBuffer(GL_NONE);
	assert(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
//检查真缓冲区对象的完整性
	glBindTexture(GL_TEXTURE_2D, _default_texture_textureId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _default_framebufferId);
}

LightRender::~LightRender()
{
	_glProgram->release();
	glDeleteFramebuffers(1, &_framebufferId);
	glDeleteRenderbuffers(1, &_depthTextureId);
}