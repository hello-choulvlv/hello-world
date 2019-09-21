/*
  *VSM实现
  *2018年8月2日
  *@author:xiaohuaxiong
 */
#include "ShadowMapVSM.h"
#include "engine/GLCacheManager.h"
#include<assert.h>
ShadowMapVSM::ShadowMapVSM():
	_lastFramebufferId(0),
	_depthTextureId(0),
	_fuzzyShader(nullptr),
	_sampleCount(5),
	_vertexBufferId(0)
{
	_framebufferIds[0] = 0;
	_framebufferIds[1] = 0;
	_framebufferIds[2] = 0;

	_textureIds[0] = 0;
	_textureIds[1] = 0;
	_totalFactor = 1.0f / ((_sampleCount+1.0f)*(_sampleCount+2.0f));
}

ShadowMapVSM::~ShadowMapVSM()
{
	glDeleteTextures(2,_textureIds);
	glDeleteRenderbuffers(1, &_depthTextureId);
	glDeleteFramebuffers(3, _framebufferIds);

	_fuzzyShader->release();
	glDeleteBuffers(1, &_vertexBufferId);
}

bool ShadowMapVSM::init(const glk::Size &framebufferSize)
{
	_framebufferSize = framebufferSize;
	//创建颜色纹理
	int defaultTexture, defaultFramebufferId,renderbufferId;
	glGetIntegerv(GL_TEXTURE_BINDING_2D,&defaultTexture);
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderbufferId);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFramebufferId);

	glGenTextures(2,_textureIds);
	float    anisotropic = 0;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropic);
	for (int k = 0; k < 2; ++k)
	{
		glBindTexture(GL_TEXTURE_2D, _textureIds[k]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,8);
		//int uu = GL_VERSION_3_0;
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG32F,framebufferSize.width,framebufferSize.height);
		//glTexImage2D(GL_TEXTURE_2D,0,GL_RG32F,framebufferSize.width,framebufferSize.height,0,GL_RG,GL_FLOAT,nullptr);
	}
	//depth-texture
	glGenRenderbuffers(1, &_depthTextureId);
	glBindRenderbuffer(GL_RENDERBUFFER, _depthTextureId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, framebufferSize.width, framebufferSize.height);
	//
	glGenFramebuffers(3, _framebufferIds);
	glBindFramebuffer(GL_FRAMEBUFFER,_framebufferIds[0]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureIds[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,_depthTextureId,0);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER,_framebufferIds[1]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureIds[1], 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER,_framebufferIds[2]);
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,_textureIds[0],0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);

	glBindTexture(GL_TEXTURE_2D,defaultTexture);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferId);

	initGLProgram();
	initBuffer();

	return true;
}

void    ShadowMapVSM::initGLProgram()
{
	_fuzzyShader = glk::GLCacheManager::getInstance()->findGLProgram(glk::GLCacheManager::GLProgramType_FuzzyBoxTextureVSM);
	_fuzzyShader->retain();
	_baseMapLoc = _fuzzyShader->getUniformLocation("g_BaseTexture");
	_pixelStepLoc = _fuzzyShader->getUniformLocation("g_PixelStep");
	_sampleCountLoc = _fuzzyShader->getUniformLocation("g_SampleCount");
}

void  ShadowMapVSM::initBuffer()
{
	int vertex_buffer = 0;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&vertex_buffer);

	float vertex_data[] = {
		-1.0f,1.0f,0.0f,0.0f,1.0f,//
		-1.0f,-1.0f,0.0f,0.0f,0.0f,//1
		1.0f,1.0f,0.0f,1.0f,1.0f,//2
		1.0f,-1.0f,0.0f,1.0f,0.0f,//3
	};
	glGenBuffers(1, &_vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER,_vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data),vertex_data,GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
}

ShadowMapVSM *ShadowMapVSM::create(const glk::Size &framebufferSize)
{
	ShadowMapVSM *vsm = new ShadowMapVSM();
	vsm->init(framebufferSize);
	return vsm;
}

void ShadowMapVSM::setSampleCount(int sampleCount)
{
	_sampleCount = sampleCount;
}

int ShadowMapVSM::getSampleCount()const
{
	return _sampleCount;
}

void    ShadowMapVSM::save()
{
	glGetIntegerv(GL_FRAMEBUFFER, &_lastFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferIds[0]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ShadowMapVSM::restore()
{
	glBindFramebuffer(GL_FRAMEBUFFER,_lastFramebufferId);
}

unsigned ShadowMapVSM::getColorTexture()const
{
	return _textureIds[0];
}

void ShadowMapVSM::fuzzy()
{
	_fuzzyShader->perform();
	//横向模糊
	glUniform2f(_pixelStepLoc,1.0f/_framebufferSize.width,0);
	glUniform1i(_sampleCountLoc,_sampleCount);
	glBindBuffer(GL_ARRAY_BUFFER,_vertexBufferId);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)(sizeof(float)*3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_textureIds[0]);
	glUniform1i(_baseMapLoc,0);

	//framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER,_framebufferIds[1]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	////纵向的模糊
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _textureIds[1]);
	glUniform1i(_baseMapLoc,0);

	glUniform2f(_pixelStepLoc, 0.0f, 1.0f/_framebufferSize.height);

	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferIds[2]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}