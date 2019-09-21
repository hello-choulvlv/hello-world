//
#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
#include<engine/Sprite.h>
#include "Chest.h"
#include "LightChest.h"
#include "HdrMrt.h"
#include "GaussianPerform.h"
#include "HdrMerge.h"

struct       UserData
{
	Chest           *_userChest;
	Chest           *_userChest2;
	LightChest  *_lightChest;
	GLTexture    *_testTexture;
	Sprite            *_sprite;
//hdr + mrt对象
	HdrMrt		*_hdrmrt;
//高斯模糊
	GaussianPerform		*_gauPerform;
//将所得到的数据融合
	HdrMerge	*_hdrMerge;
//关于全局的一些数据
	Matrix         _projectMatrix;
	GLVector3   _lightPosition;
	GLVector3   _ambientColor;
	GLVector3   _lightColor;
//高斯模糊次数
	int                _performTimes;
//高光指数
	float              _exposure;
};
//

void        Init(GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	_user->_lightPosition = GLVector3(0.0f,1.0f,-4.0f);
	_user->_lightColor = GLVector3(5.0f,0.0f,4.0f);
	_user->_ambientColor = GLVector3(0.15f,0.15f,0.15f);
	_user->_performTimes = 10;
	_user->_exposure = 0.5f;
//	_user->_testTexture = GLTexture::createWithFile("tga/basemap.tga");
//	_user->_sprite = Sprite::createWithFile("tga/basemap.tga");
//
	_user->_userChest = Chest::createChest("tga/basemap.tga");
	_user->_userChest->setScale(0.7f);
	_user->_userChest->setPosition(GLVector3(-3.0f,-0.5f,-4.0f));
	_user->_userChest->setRotateAngle(30.0f, GLVector3(1.0f,1.0f,1.0f));
	_user->_userChest->setAmbientColor(_user->_ambientColor);
	_user->_userChest->setLightColor(_user->_lightColor);
	_user->_userChest->setLightPosition(_user->_lightPosition);
	_user->_userChest->setSpecularCoeff(0.45f);

	_user->_userChest2 = Chest::createChest("tga/basemap.tga");
	_user->_userChest2->setScale(0.7f);
	_user->_userChest2->setRotateAngle(-30.0f, GLVector3(1.0f,1.0f,1.0f));
	_user->_userChest2->setPosition(GLVector3(1.0f,-0.5f,-4.0f));
	_user->_userChest2->setAmbientColor(_user->_ambientColor);
	_user->_userChest2->setLightColor(_user->_lightColor);
	_user->_userChest2->setLightPosition(_user->_lightPosition);
	_user->_userChest->setSpecularCoeff(0.36f);

	_user->_lightChest = LightChest::createLightChest(_user->_lightColor);
	_user->_lightChest->setPosition(_user->_lightPosition);
	_user->_lightChest->setScale(0.4f);
	_user->_lightChest->rotate(90.0f, GLVector3(-1.0f,1.0f,-1.0f)-GLVector3(1.0f,-1.0f,1.0f));
//hdr + mrt
	Size        _size = _context->getWinSize();
	_user->_hdrmrt = HdrMrt::createHdrMrt(_size.width,_size.height);
    _user->_gauPerform = GaussianPerform::createGaussianPerform(_size.width, _size.height, _user->_performTimes);
	_user->_hdrMerge = HdrMerge::createHdrMerge(_user->_exposure);
//
	_user->_projectMatrix.identity();
	_user->_projectMatrix.orthoProject(-4.0f, 4.0f, -4.0f, 4.0f, 0.0f, 10.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_userChest->update(_deltaTime);
	_user->_userChest2->update(_deltaTime);
	_user->_lightChest->update(_deltaTime);
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
//	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	int        originFramebufferId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originFramebufferId);

	_user->_hdrmrt->activateFramebuffer();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_user->_userChest->draw(_user->_projectMatrix, 0);
	_user->_userChest2->draw(_user->_projectMatrix,0);
	_user->_lightChest->draw(_user->_projectMatrix,0);
//
	_user->_gauPerform->perform(_user->_hdrmrt->hdrTexture());
//恢复帧缓冲区绑定
	glBindFramebuffer(GL_FRAMEBUFFER, originFramebufferId);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//使用处理后的纹理与以前的基础文理着色
	_user->_hdrMerge->drawMerge(_user->_hdrmrt->normalTexture(), _user->_gauPerform->performedTexture());
//	_user->_hdrMerge->drawMerge(_user->_hdrmrt->normalTexture(), _user->_hdrmrt->hdrTexture());
//	_user->_hdrMerge->drawMerge(_user->_testTexture->name(), _user->_hdrmrt->hdrTexture());
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_userChest->release();
	_user->_userChest2->release();
	_user->_lightChest->release();
	_user->_hdrmrt->release();
	_user->_hdrMerge->release();
	_user->_gauPerform->release();
}
///////////////////////////don not modify below function////////////////////////////////////
