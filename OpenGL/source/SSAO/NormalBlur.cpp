/*
 *普通模糊特效实现
  *2016-10-14 19:13:24
 */
#include<GL/glew.h>
#include<assert.h>
#include<engine/GLContext.h>
#include "NormalBlur.h"

NormalBlur::NormalBlur()
{
	int          _default_framebufferId, _default_textureId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_default_framebufferId);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &_default_textureId);

	glGenFramebuffers(1, &_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);
	Size         _size = GLContext::getInstance()->getWinSize();

	glGenTextures(1, &_textureId);
	glBindTexture(GL_TEXTURE_2D, _textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _size.width, _size.height, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureId, 0);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
//还原缓冲区绑定
	glBindTexture(GL_TEXTURE_2D, _default_textureId);
	glBindFramebuffer(GL_FRAMEBUFFER, _default_framebufferId);
//
	_glProgram = GLProgram::createWithFile("shader/ssao/ssao_blur.vsh", "shader/ssao/ssao_blur.fsh");
	_mvpMatrixLoc = _glProgram->getUniformLocation("u_mvpMatrix");
	_baseMapLoc = _glProgram->getUniformLocation("u_baseMap");
	_screenPixelLoc = _glProgram->getUniformLocation("u_textureStep");
	_kernelCountLoc = _glProgram->getUniformLocation("u_kernelCount");
}

NormalBlur::~NormalBlur()
{
	_glProgram->release();
	glDeleteTextures(1, &_textureId);
	glDeleteFramebuffers(1, &_framebufferId);
}
unsigned          NormalBlur::loadBlurTexture(unsigned    targetTextureId)
{
	int       _default_framebufferId;
	int       _default_textureId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_default_framebufferId);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &_default_textureId);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);

	_glProgram->perform();
	glBindBuffer(GL_ARRAY_BUFFER, GLContext::getInstance()->loadBufferIdentity());

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(sizeof(float) * 3));

	glUniformMatrix4fv(_mvpMatrixLoc, 1, GL_FALSE, _mvpMatrix.pointer());
	Size       _size = GLContext::getInstance()->getWinSize();
	Size       _screenPixel(1.0f / _size.width, 1.0f / _size.height);
	glUniform2fv(_screenPixelLoc, 1, &_screenPixel.width);
	glUniform1f(_kernelCountLoc, 4.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, targetTextureId);
	glUniform1i(_baseMapLoc, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindTexture(GL_TEXTURE_2D, _default_textureId);
	glBindFramebuffer(GL_FRAMEBUFFER, _default_framebufferId);

	return _textureId;
}
