/*
 *屏幕空间环境光遮蔽,此程序为一个工具类,需要实时调用
 *2016-10-17 17:03:13
 *@Author:小花熊
 */
#include<GL/glew.h>
#include<assert.h>
#include<engine/GLContext.h>
#include"SSAORender.h"
SSAORender::SSAORender()
{
	_glProgram = NULL;
	_framebufferId = 0;
	_ssaoTextureId = 0;
	_noiseTextureId = 0;
	_kernelRadius = 1.0f;
	initSSAO();
}
SSAORender::~SSAORender()
{
	_glProgram->release();
	glDeleteFramebuffers(1, &_framebufferId);
	glDeleteTextures(1, &_noiseTextureId);
}
void         SSAORender::initSSAO()
{
	const       int         KernelSize=64;
	int           _default_framebufferId;
	int           _default_textureId;
	int           i,j;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_default_framebufferId);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &_default_textureId);

	glGenFramebuffers(1, &_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);
	glClear(GL_COLOR_BUFFER_BIT);

	GLContext     *_context = GLContext::getInstance();
	Size         _size = _context->getWinSize();
	glGenTextures(1, &_ssaoTextureId);
	glBindTexture(GL_TEXTURE_2D, _ssaoTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _size.width, _size.height, 0, GL_RGB, GL_FLOAT,NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ssaoTextureId,0);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);
//生成随机噪声以决定所选定的比较对象
	for (i = 0; i < KernelSize; ++i)
	{
		_samples[i] = GLVector3(_context->randomValue()*2.0f-1.0f,_context->randomValue()*2.0f-1.0f,_context->randomValue());
	}
//生成随机切线
	float         _tangent[32][32*3];
	for (i = 0; i < 32; ++i)
	{
		for (j = 0; j < 32; ++j)
		{
			_tangent[i][j * 3] = _context->randomValue()*2.0f-1.0f;
			_tangent[i][j * 3 + 1] = _context->randomValue()*2.0f-1.0f;
			_tangent[i][j * 3 + 2] = 0.0f;
		}
	}
	glGenTextures(1, &_noiseTextureId);
	glBindTexture(GL_TEXTURE_2D, _noiseTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 32, 32, 0, GL_RGB, GL_FLOAT, _tangent);//注意,噪声纹理一定要使用32位/16位的浮点数

	glBindTexture(GL_TEXTURE_2D, _default_textureId);
	glBindFramebuffer(GL_FRAMEBUFFER, _default_framebufferId);

	_noiseScale = GLVector2(_size.width/32,_size.height/32);
	_glProgram = GLProgram::createWithFile("shader/ssao/ssao_depth.vsh", "shader/ssao/ssao_depth.fsh");
	_mvpMatrixLoc = _glProgram->getUniformLocation("u_mvpMatrix");
	_globlePositionLoc = _glProgram->getUniformLocation("u_globlePosition");
	_globleNormalLoc = _glProgram->getUniformLocation("u_globleNormal");
	_noiseMapLoc = _glProgram->getUniformLocation("u_noiseMap");
	_samplesLoc = _glProgram->getUniformLocation("u_samples");
	_nearFarDistanceLoc = _glProgram->getUniformLocation("u_nearFarDistance");
	_noiseScaleLoc = _glProgram->getUniformLocation("u_noiseScale");
	_kernelRadiusLoc = _glProgram->getUniformLocation("u_kernelRadius");
	_projMatrixLoc = _glProgram->getUniformLocation("u_projMatrix");
}
//生成SSAO纹理
unsigned    SSAORender::loadSSAOTexture(Matrix &projMatrix, unsigned _positionTextureId, unsigned _normalTextureId)
{
	int       _default_framebufferId;
	int       _default_textureId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_default_framebufferId);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &_default_textureId);
//使用引擎自带的缓冲区对象
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);
	glClear(GL_COLOR_BUFFER_BIT);
	_glProgram->perform();
	glBindBuffer(GL_ARRAY_BUFFER, GLContext::getInstance()->loadBufferIdentity());

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float)*3));
//加载统一变量
	Matrix         _identity;
	glUniformMatrix4fv(_mvpMatrixLoc, 1, GL_FALSE, _identity.pointer());
	glUniformMatrix4fv(_projMatrixLoc, 1, GL_FALSE, projMatrix.pointer());
	glUniform3fv(_samplesLoc, 64, &_samples[0].x);
	glUniform2fv(_nearFarDistanceLoc, 1, &GLContext::getInstance()->getNearFarPlane().x);
	glUniform2fv(_noiseScaleLoc, 1, &_noiseScale.x);
	glUniform1f(_kernelRadiusLoc, _kernelRadius);
//纹理对象
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _positionTextureId);
	glUniform1i(_globlePositionLoc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _normalTextureId);
	glUniform1i(_globleNormalLoc, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _noiseTextureId);
	glUniform1i(_noiseMapLoc, 2);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindTexture(GL_TEXTURE_2D, _default_textureId);
	glBindFramebuffer(GL_FRAMEBUFFER, _default_framebufferId);
	return  _ssaoTextureId;
}