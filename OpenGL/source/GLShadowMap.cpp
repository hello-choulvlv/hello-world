/*
  *��Ӱ����ʵ��
  *2016-6-16 20:19:25
*/
#include<engine/GLShadowMap.h>
#include<GL/glew.h>
#include<assert.h>
#include<stdlib.h>
__NS_GLK_BEGIN
//////////////////////////////��Ӱ����/////////////////////////////////
GLShadowMap::GLShadowMap()
{
	_shadowMapId = 0;
	_shadowFramebufferId = 0;
}
GLShadowMap::~GLShadowMap()
{
	glDeleteTextures(1, &_shadowMapId);
	glDeleteFramebuffers(1, &_shadowFramebufferId);
	_shadowMapId = 0;
	_shadowFramebufferId = 0;
}
//
void       GLShadowMap::initWithSize(int width, int  height)
{
	assert(width > 0 && height > 0);
	//������Ӱ����
	glGenTextures(1, &_shadowMapId);
	glBindTexture(GL_TEXTURE_2D, _shadowMapId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//����Ӳ���Ƚϲ���,������Ӱ�����볣�����������Ҫ����������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//��������ߴ�
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	//֡����������,���������
	glGenFramebuffers(1, &_shadowFramebufferId);
	int    _defaultFramebufferId = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &_defaultFramebufferId);//��ȡ��ǰ��֡��������,�����֮���ٻ�ԭ
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _shadowFramebufferId);
	GLenum			_none = GL_NONE;//��д���κ���ɫ
	glDrawBuffers(1, &_none);
	//���������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _shadowMapId, 0);
	assert(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _defaultFramebufferId);
}
//
GLShadowMap              *GLShadowMap::createWithSize(int  _width, int  _height)
{
	GLShadowMap    *_shadowMap = new   GLShadowMap();
	_shadowMap->initWithSize(_width, _height);
	return  _shadowMap;
}
//��ȡ�滺��������
unsigned int     GLShadowMap::framebuffer()
{
	return  _shadowFramebufferId;
}
//
unsigned int  GLShadowMap::shadowMap()
{
	return _shadowMapId;
}
//��
void  GLShadowMap::bindFramebuffer()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _shadowFramebufferId);
}

__NS_GLK_END