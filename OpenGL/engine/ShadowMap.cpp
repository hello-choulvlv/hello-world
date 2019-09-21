/*
  *产生深度纹理的阴影实现
  *2016-10-21 19:49:40
  *@Author:小花熊
  */
#include<assert.h>
#include<GL/glew.h>
//#include<engine/GLContext.h>
#include<engine/ShadowMap.h>
__NS_GLK_BEGIN
ShadowMap::ShadowMap()
{
	_framebufferId = 0;
	_depthTextureId = 0;
	_oldFramebufferId = 0;
	_isDepthLayerArray = false;
	_numberOfLayer = 0;
	_isViewportChange = true;
}

ShadowMap::~ShadowMap()
{
	glDeleteFramebuffers(1, &_framebufferId);
	glDeleteTextures(1, &_depthTextureId);
}

bool	ShadowMap::initWithMapSize(const Size &shadowMapSize)
{
	int       _default_framebufferId;
	int       _default_texture_textureId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_default_framebufferId);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &_default_texture_textureId);

	//Size      _size = GLContext::getInstance()->getWinSize();
	glGenFramebuffers(1, &_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);
	glGenTextures(1, &_depthTextureId);
	glBindTexture(GL_TEXTURE_2D, _depthTextureId);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, _size.width, _size.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexStorage2D(GL_TEXTURE_2D,1,GL_DEPTH_COMPONENT32F, shadowMapSize.width, shadowMapSize.height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTextureId, 0);
	//禁止写入颜色
	glDrawBuffer(GL_NONE);
	assert(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	//检查真缓冲区对象的完整性
	glBindTexture(GL_TEXTURE_2D, _default_texture_textureId);
	glBindFramebuffer(GL_FRAMEBUFFER, _default_framebufferId);
	_shadowMapSize = shadowMapSize;
	return true;
}

bool  ShadowMap::initWithMapLayer(const Size &mapSize, const int numberOfLayer)
{
	int defaultFramebufferId;
	int defaultTextureId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING,&defaultFramebufferId);
	glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY,&defaultTextureId);
	//创建帧缓冲区对象
	glGenFramebuffers(1, &_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER,_framebufferId);
	//创建深度纹理对象
	glGenTextures(1, &_depthTextureId);
	glBindTexture(GL_TEXTURE_2D_ARRAY,_depthTextureId);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F, mapSize.width, mapSize.height, numberOfLayer);
	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthTextureId, 0);
	//禁止颜色写入
	glDrawBuffer(GL_NONE);
	//Check Status Of Draw-Framebuffer 
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);
	//
	glBindTexture(GL_TEXTURE_BINDING_2D_ARRAY,defaultTextureId);
	glBindFramebuffer(GL_FRAMEBUFFER,defaultFramebufferId);
	_shadowMapSize = mapSize;
	_isDepthLayerArray = true;
	_numberOfLayer = numberOfLayer;
	return true;
}

const  unsigned ShadowMap::getDepthTexture()const
{
	return _depthTextureId;
}

const bool ShadowMap::isDepthArrayLayer()const
{
	return _isDepthLayerArray;
}

const Size& ShadowMap::getMapSize()const
{
	return _shadowMapSize;
}

const int ShadowMap::getMapLayer()const
{
	return _numberOfLayer;
}

void  ShadowMap::activeShadowFramebuffer()
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING,&_oldFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);
	if(_isViewportChange)
		glViewport(0, 0, _shadowMapSize.width, _shadowMapSize.height);
	glClear(GL_DEPTH_BUFFER_BIT);
	//const float depthValue = 1.0f;
	//glClearBufferfv(GL_DEPTH,0,&depthValue);
}

void ShadowMap::setViewportChange(const bool b)
{
	_isViewportChange = b;
}

bool ShadowMap::isViewportChange()const
{
	return _isViewportChange;
}

void ShadowMap::restoreFramebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, _oldFramebufferId);
	//_oldFramebufferId = 0;
}

ShadowMap    *ShadowMap::createWithMapLayer(const Size &mapSize, const int numberOfLayer)
{
	ShadowMap *map = new ShadowMap();
	if (!map->initWithMapLayer(mapSize, numberOfLayer))
	{
		map->release();
		map = nullptr;
	}
	return map;
}

ShadowMap   *ShadowMap::createWithMapSize(const Size &mapSize)
{
	ShadowMap *shadowMap = new ShadowMap();
	if (!shadowMap->initWithMapSize(mapSize))
	{
		shadowMap->release();
		shadowMap = nullptr;
	}
	return shadowMap;
}

__NS_GLK_END