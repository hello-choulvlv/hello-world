//
#include <GL/glew.h>
#include<assert.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
#include<engine/Sprite.h>
#include "RenderSphere.h"
/*
  *延迟着色
 */
//普通前阶段着色器

struct       UserData
{
	RenderSphere		*_renderSphere;
	GLProgram			*_afterProgram;//光照处理着色器
	Sprite                      *_sprite;
	unsigned                  _globleTextureId[3];//与G-buffer相关的纹理
	unsigned                  _depthTextureId;//深度纹理
	unsigned                  _framebufferId;//帧缓冲区对象
	unsigned                  _vertexbufferId;
//程序数据
	Matrix                    _identity;
	Matrix                    _projectMatrix;
	GLVector3              _lightPosition;
	GLVector3              _ambientColor;
	GLVector3              _lightColor;
	GLVector3              _eyePosition;
	float                        _specularCoeff;
//位置
	unsigned                _mvpMatrixLoc;
	unsigned                _globleMapLoc;
	unsigned                _globlePositionLoc;
	unsigned                _globleNormalLoc;
	unsigned                _lightColorLoc;
	unsigned                _lightPositionLoc;
	unsigned                _ambientColorLoc;
	unsigned                _eyePositionLoc;
	unsigned                _specularCoeffLoc;
};
//

void        Init(GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	
	_user->_renderSphere =new   RenderSphere() ;//RenderSphere::create("tga/Earth512x256.tga");
	_user->_renderSphere->initWithFile("tga/Earth512x256.tga");
	_user->_afterProgram = GLProgram::createWithFile("shader/deffered_shader/after.vsh", "shader/deffered_shader/after.fsh");
	_user->_sprite = Sprite::createWithFile("tga/city_back.tga");
//与延迟着色相关的G-buffer纹理,记忆真缓冲区对象
	Size          _screenSize = _context->getWinSize();
	int            defaultFramebufferId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFramebufferId);
	glGenTextures(3, _user->_globleTextureId);
	glGenFramebuffers(1, &_user->_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _user->_framebufferId);
	for (int i = 0; i < 3; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, _user->_globleTextureId[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _screenSize.width, _screenSize.height, 0, GL_RGBA, GL_FLOAT, NULL);
//绑定真缓冲区对象
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _user->_globleTextureId[i], 0);
	}
//深度缓冲区
	glGenRenderbuffers(1, &_user->_depthTextureId);
	glBindRenderbuffer(GL_RENDERBUFFER, _user->_depthTextureId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, _screenSize.width, _screenSize.height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _user->_depthTextureId);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
//检测帧缓冲区对象的完整性
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);
//MRT
	unsigned     _attach[3] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
	};
	glDrawBuffers(3, _attach);
//还原
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferId);
	float         _VertexData[20] = {
		-1.0f,1.0f,0.0f,   0.0f,1.0f,
		-1.0f,-1.0f,0.0f,  0.0f,0.0f,
		1.0f,1.0f,0.0f,     1.0f,1.0f,
		1.0f,-1.0f,0.0f,   1.0f,0.0f,
	};
	glGenBuffers(1, &_user->_vertexbufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _user->_vertexbufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_VertexData), _VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
//////////////////////////////////////程序数据/////////////////////////////////////
	_user->_identity.identity();
 //光照的颜色
	_user->_lightColor = GLVector3(1.0f,1.0f,1.0f);
	_user->_lightPosition = GLVector3(1.0f,2.0f,-2.0f);
	_user->_ambientColor = GLVector3(0.1f,0.1f,0.1f);
	_user->_eyePosition = GLVector3(0.0f,0.0f,1.0f);
	_user->_specularCoeff = 0.36f;//散射指数
//位置
	_user->_mvpMatrixLoc = _user->_afterProgram->getUniformLocation("u_mvpMatrix");
	_user->_globleMapLoc = _user->_afterProgram->getUniformLocation("u_globleMap");
	_user->_globlePositionLoc = _user->_afterProgram->getUniformLocation("u_globlePosition");
	_user->_globleNormalLoc = _user->_afterProgram->getUniformLocation("u_globleNormal");
	_user->_lightColorLoc = _user->_afterProgram->getUniformLocation("u_lightColor");
	_user->_lightPositionLoc = _user->_afterProgram->getUniformLocation("u_lightPosition");
	_user->_eyePositionLoc = _user->_afterProgram->getUniformLocation("u_eyePosition");
	_user->_specularCoeffLoc = _user->_afterProgram->getUniformLocation("u_specularCoeff");
	_user->_ambientColorLoc = _user->_afterProgram->getUniformLocation("u_ambientColor");
//
	_user->_projectMatrix.identity();
	_user->_projectMatrix.orthoProject(-4.0f, 4.0f, -4.0f, 4.0f, 0.0f, 10.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_renderSphere->update(_deltaTime);
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
//	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	int        originFramebufferId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originFramebufferId);

	glBindFramebuffer(GL_FRAMEBUFFER, _user->_framebufferId);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_user->_renderSphere->setPosition(GLVector3(0.0f,0.0f,-4.0f));
	_user->_renderSphere->draw(_user->_projectMatrix, 0x1);

	_user->_renderSphere->setPosition(GLVector3(-2.0f,2.0f,-4.0f));
	_user->_renderSphere->draw(_user->_projectMatrix,0x1);

	_user->_renderSphere->setPosition(GLVector3(2.0f,2.0f,-4.0f));
	_user->_renderSphere->draw(_user->_projectMatrix,0x1);

	_user->_renderSphere->setPosition(GLVector3(-2.0f,-2.0f,-4.0f));
	_user->_renderSphere->draw(_user->_projectMatrix,0x1);

	_user->_renderSphere->setPosition(GLVector3(2.0f,-2.0f,-4.0f));
	_user->_renderSphere->draw(_user->_projectMatrix,0x1);

	glBindFramebuffer(GL_FRAMEBUFFER, originFramebufferId);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//执行后处理着色器
	_user->_afterProgram->enableObject();
	glBindBuffer(GL_ARRAY_BUFFER, _user->_vertexbufferId);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
//矩阵
	glUniformMatrix4fv(_user->_mvpMatrixLoc, 1, GL_FALSE, _user->_identity.pointer());
//三重纹理
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->_globleTextureId[0]);
	glUniform1i(_user->_globlePositionLoc	,0);//位置

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _user->_globleTextureId[1]);
	glUniform1i(_user->_globleNormalLoc,1);//法线

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _user->_globleTextureId[2]);
	glUniform1i(_user->_globleMapLoc,2);//颜色
//与光照有关的数据
	glUniform3fv(_user->_lightColorLoc, 1, &_user->_lightColor.x);
	glUniform3fv(_user->_lightPositionLoc, 1, &_user->_lightPosition.x);
	glUniform3fv(_user->_ambientColorLoc, 1, &_user->_ambientColor.x);
	glUniform3fv(_user->_eyePositionLoc, 1, &_user->_eyePosition.x);
	glUniform1f(_user->_specularCoeffLoc, _user->_specularCoeff);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//增加颜色混合
	Size    _size = _context->getWinSize();
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _user->_framebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originFramebufferId);
	glBlitFramebuffer(0, 0, _size.width, _size.height, 0, 0, _size.width, _size.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, originFramebufferId);
//
	Matrix          _identity;
	_identity.scale(4.0f, 4.0f, 4.0f);
	_identity.translate(0.0f, 1.0f, -4.0f);
	_identity.multiply(_user->_projectMatrix);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	_user->_sprite->render(&_identity);
	glDisable(GL_BLEND);
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_renderSphere->release();
	_user->_afterProgram->release();
	_user->_sprite->release();
	glDeleteTextures(3, _user->_globleTextureId);
	glDeleteRenderbuffers(1,&_user->_depthTextureId	);
	glDeleteFramebuffers(1, &_user->_framebufferId);
	glDeleteBuffers(1, &_user->_vertexbufferId);
}
///////////////////////////don not modify below function////////////////////////////////////
