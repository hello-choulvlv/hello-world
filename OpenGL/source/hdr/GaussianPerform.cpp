/*
  *高斯模糊处理实现
  *2016-9-22 11:23:43
  *@Author:小花熊
 */
#include<GL/glew.h>
#include<assert.h>
#include "GaussianPerform.h"

GaussianPerform::GaussianPerform()
{
	_glProgram = NULL;
	_vertex_bufferId = 0;
	_performTimes = 1;
	_pp_framebufferId[0] = 0;
	_pp_framebufferId[1] = 0;
	_pp_textureId[0] = 0;
	_pp_textureId[1] = 0;
	_pp_texture_index = 0;
}

GaussianPerform::~GaussianPerform()
{
	_glProgram->release();
	glDeleteBuffers(1, &_vertex_bufferId);
	_glProgram = NULL;
	_vertex_bufferId = 0;
	glDeleteFramebuffers(2, _pp_framebufferId);
	glDeleteTextures(2, _pp_textureId);
}

void       GaussianPerform::initGaussianPerform(int  width,int  height, int performTimes)
{
	_performTimes = performTimes;
	_glProgram = GLProgram::createWithFile("shader/hdr/gaussian_blur.vsh", "shader/hdr/gaussian_blur.fsh");
	_glProgram->retain();
//位置
	_mvpMatrixLoc = _glProgram->getUniformLocation("u_mvpMatrix");
	_baseMapLoc = _glProgram->getUniformLocation("u_baseMap");
	_hvstepLoc = _glProgram->getUniformLocation("u_hvstep");
//顶点缓冲区对象
	int         originVertexBuffer,originFramebufferId,originTextureId;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &originVertexBuffer);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originFramebufferId);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &originTextureId);
	float              vVertex3f2f[] = {
		-1.0f,1.0f,0.0f, 0.0f,1.0f,
		-1.0f,-1.0f,0.0f, 0.0f,0.0f,
		1.0f,1.0f,0.0f,   1.0f,1.0f,
		1.0f,-1.0f,0.0f, 1.0f,0.0f
	};
	glGenBuffers(1, &_vertex_bufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _vertex_bufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vVertex3f2f), vVertex3f2f, GL_STATIC_DRAW);
//生成帧缓冲区对象
	glGenFramebuffers(2, _pp_framebufferId);
	glGenTextures(2, _pp_textureId);
	for (int i = 0; i < 2; ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _pp_framebufferId[i]);
		glBindTexture(GL_TEXTURE_2D, _pp_textureId[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width,height,0,GL_RGBA,GL_FLOAT,NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _pp_textureId[i], 0);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);
	}
//还原顶点,帧缓冲区,纹理绑定
	glBindBuffer(GL_ARRAY_BUFFER, originVertexBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, originFramebufferId);
	glBindTexture(GL_TEXTURE_2D, originTextureId);
}

GaussianPerform		*GaussianPerform::createGaussianPerform(int  width,int  height, int performTimes)
{
	GaussianPerform     *_gp = new   GaussianPerform();
	_gp->initGaussianPerform(width,height, performTimes);
	return  _gp;
}

void         GaussianPerform::setPerformTimes(int performTimes)
{
	_performTimes = performTimes;
}

void         GaussianPerform::perform(unsigned  base_textureId)
{
//一共做performTimes*2次的反复处理
	int          textureId = base_textureId;
	int          nowFramebuffer = 0;
	int          originFramebufferId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originFramebufferId);

	_glProgram->enableObject();
	for (int i = 0; i < _performTimes; ++i)
	{
		nowFramebuffer = i  & 0x01;
//水平处理
		if (i)
			textureId = _pp_textureId[(i-1) & 0x1];
		glBindFramebuffer(GL_FRAMEBUFFER,_pp_framebufferId[nowFramebuffer]);

		glBindBuffer(GL_ARRAY_BUFFER, _vertex_bufferId);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glUniform1i(_baseMapLoc, 0);
//注意这一步
		_hvstep = GLVector2(1.0f,0.0f);
		glUniform2fv(_hvstepLoc, 1, &_hvstep.x);
		glUniformMatrix4fv(_mvpMatrixLoc, 1, GL_FALSE, _mvpMatrix.pointer());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//垂直处理
		_hvstep = GLVector2(0.0f,1.0f);
		glUniform2fv(_hvstepLoc, 1, &_hvstep.x);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	_pp_texture_index = (_performTimes-1) & 0x01;
	glBindFramebuffer(GL_FRAMEBUFFER, originFramebufferId);
}
//
unsigned        GaussianPerform::performedTexture()
{
	return   _pp_textureId[_pp_texture_index];
}