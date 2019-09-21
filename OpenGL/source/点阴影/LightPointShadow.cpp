/*
  *���Դǰ�˴���ʱ��
  *ע����Ҫʹ�ü�����ɫ��,��������಻���ڰ�׿/iOS�豸������
  *2016-10-25 20:29:34
  *@Author:С����
 */
#include<GL/glew.h>
#include<assert.h>
#include<stdio.h>
#include<engine/GLContext.h>
#include "LightPointShadow.h"
//��Ҫע�����,��ΪCUBE_MAP��ʵ�ʲ�����ʱ���������������Y�᷽�����ǳʾ����ϵ,������ʵ�ʵ���ͼ������,��Ҫ����һ���������,
//����ֱ�ӽ�Y�������ϵȡ��
LightPointShadow::LightPointShadow()
{
	int           _default_framebufferId;
	int           _default_textureId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_default_framebufferId);
	glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &_default_textureId);

	glGenFramebuffers(1, &_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);
#ifdef __GEOMETRY_SHADOW__
	glGenTextures(1, &_depthTextureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _depthTextureId);
	Size      _size = GLContext::getInstance()->getWinSize();
//ע��.��������ͼ�ĳ��ȺͿ�ȱ�����ͬ
	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
//		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,_depthTextureId,0);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthTextureId, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, _default_textureId); 
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//�������
	_shadowProgram = GLProgram::createWithFile("shader/shadow/shadow_cubemap.vsh", "shader/shadow/shadow_cubemap.gsh", "shader/shadow/shadow_cubemap.fsh");
	_lightPositionLoc = _shadowProgram->getUniformLocation("u_lightPosition");
	_modelMatrixLoc = _shadowProgram->getUniformLocation("u_modelMatrix");
	for (int i = 0; i < 6; ++i)
	{
		char   _buffer[128];
		sprintf(_buffer,"u_viewProjMatrix[%d]",i);
		_viewProjMatrixLoc[i] = _shadowProgram->getUniformLocation(_buffer);
	}
	_maxDistanceLoc = _shadowProgram->getUniformLocation("u_maxDistance");
#else
	int            _default_texture2dId;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &_default_texture2dId);
	glGenTextures(1,&_depthTextureId);
	glBindTexture(GL_TEXTURE_2D, _depthTextureId);
	Size         _size = GLContext::getInstance()->getShadowSize();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, _size.width, _size.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTextureId,0);
//generate cube map
	glGenTextures(1, &_cubeMapId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _cubeMapId);
	for (int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R32F, _size.width, _size.height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, _default_framebufferId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _default_textureId);
	glBindTexture(GL_TEXTURE_2D, _default_texture2dId);
//GLProgram  
	_shadowProgram = GLProgram::createWithFile("shader/shadow/shadow_six_face.vsh","shader/shadow/shadow_six_face.fsh");
	_modelMatrixLoc = _shadowProgram->getUniformLocation("u_modelMatrix");
	_mvpMatrixLoc = _shadowProgram->getUniformLocation("u_mvpMatrix");
	_lightPositionLoc = _shadowProgram->getUniformLocation("u_lightPosition");
#endif
}

LightPointShadow::~LightPointShadow()
{
	_shadowProgram->release();
	glDeleteFramebuffers(1, &_framebufferId);
	glDeleteTextures(1, &_depthTextureId);
}

#ifndef  __GEOMETRY_SHADOW__
struct    __SixFaceVertexTexture
{
	int                   _shadowFace;
	GLVector3     _viewPosition;
	GLVector3     _upVector3;
};
static    __SixFaceVertexTexture          __sixFaceMap[6] = {
		{ GL_TEXTURE_CUBE_MAP_POSITIVE_X, GLVector3(1.0f, 0.0f, 0.0f), GLVector3(0.0f, -1.0f, 0.0f) },
		{ GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GLVector3(-1.0f, 0.0f, 0.0f), GLVector3(0.0f, -1.0f, 0.0f) },
		{ GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GLVector3(0.0f, 1.0f, 0.0f), GLVector3(0.0f, 0.0f, -1.0f) },
		{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GLVector3(0.0f, -1.0f, 0.0f), GLVector3(1.0f, 0.0f, 1.0f) },
		{ GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GLVector3(0.0f, 0.0f, 1.0f), GLVector3(0.0f, -1.0f, 0.0f) },
		{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GLVector3(0.0f, 0.0f, -1.0f), GLVector3(0.0f, -1.0f, 0.0f) }
};
void       LightPointShadow::bindTextureUnit(int _texture_unit)
{
	glActiveTexture(GL_TEXTURE0 + _texture_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _cubeMapId);
}

void     LightPointShadow::writeFace(int index, GLVector3  & _lightPosition)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, _cubeMapId, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	_viewMatrix.identity();
	_viewMatrix.lookAt(_lightPosition, _lightPosition + __sixFaceMap[index]._viewPosition, __sixFaceMap[index]._upVector3);
}

Matrix&       LightPointShadow::loadViewMatrix()
{
	return _viewMatrix;
}
#endif