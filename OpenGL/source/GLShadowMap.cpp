/*
  *阴影纹理实现
  *2016-6-16 20:19:25
*/
#include<engine/GLShadowMap.h>
#include<GL/glew.h>
#include<assert.h>
#include<stdlib.h>
__NS_GLK_BEGIN
//////////////////////////////阴影纹理/////////////////////////////////
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
	//创建阴影纹理
	glGenTextures(1, &_shadowMapId);
	glBindTexture(GL_TEXTURE_2D, _shadowMapId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//设置硬件比较参数,这是阴影纹理与常规纹理的最重要的区别所在
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//设置纹理尺寸
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	//帧缓冲区对象,并将纹理绑定
	glGenFramebuffers(1, &_shadowFramebufferId);
	int    _defaultFramebufferId = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &_defaultFramebufferId);//获取当前的帧缓冲区绑定,待完成之后再还原
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _shadowFramebufferId);
	GLenum			_none = GL_NONE;//不写入任何颜色
	glDrawBuffers(1, &_none);
	//绑定深度纹理
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
//获取真缓冲区对象
unsigned int     GLShadowMap::framebuffer()
{
	return  _shadowFramebufferId;
}
//
unsigned int  GLShadowMap::shadowMap()
{
	return _shadowMapId;
}
//绑定
void  GLShadowMap::bindFramebuffer()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _shadowFramebufferId);
}

__NS_GLK_END