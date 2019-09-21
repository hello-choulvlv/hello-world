//
#include <GL/glew.h>
#include<assert.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
#include<engine/GLTexture.h>
#include<engine/Sprite.h>
#include<engine/Shape.h>
#include "LightRender.h"
/*
  *阴影
 */
struct       ShadowLight
{
	GLProgram		*_afterProgram;
	unsigned                _mvpMatrixLoc;
	unsigned                _modelMatrixLoc;
	unsigned                _lightMatrixLoc;
	unsigned                _normalMatrixLoc;
	unsigned                _baseMapLoc;
	unsigned                _depthMapLoc;
	unsigned                _lightColorLoc;
	unsigned                _lightDirectionLoc;
	unsigned                _lightPositionLoc;
	unsigned                _ambientColorLoc;
	unsigned                _eyePositionLoc;
	unsigned                _specularCoeffLoc;
	unsigned                _nearFarLoc;
	ShadowLight();
	~ShadowLight();
};
ShadowLight::ShadowLight()
{
	_afterProgram = GLProgram::createWithFile("shader/shadow/shadow_light.vsh", "shader/shadow/shadow_light.fsh");
	_mvpMatrixLoc = _afterProgram->getUniformLocation("u_mvpMatrix");
	_modelMatrixLoc = _afterProgram->getUniformLocation("u_modelMatrix");
	_lightMatrixLoc = _afterProgram->getUniformLocation("u_lightMatrix");
	_normalMatrixLoc = _afterProgram->getUniformLocation("u_normalMatrix");
	_baseMapLoc = _afterProgram->getUniformLocation("u_baseMap");
	_depthMapLoc = _afterProgram->getUniformLocation("u_depthMap");
	_lightColorLoc = _afterProgram->getUniformLocation("u_lightColor");
	_lightDirectionLoc = _afterProgram->getUniformLocation("u_lightDirection");
	_eyePositionLoc = _afterProgram->getUniformLocation("u_eyePosition");
	_ambientColorLoc = _afterProgram->getUniformLocation("u_ambientColor");
	_specularCoeffLoc = _afterProgram->getUniformLocation("u_specularCoeff");
	_nearFarLoc = _afterProgram->getUniformLocation("u_nearFar");
}
ShadowLight::~ShadowLight()
{
	_afterProgram->release();
}
struct       UserData
{
	ShadowLight          _shadowLight;
	LightRender	        *_lightRender;
	Mesh                       *_groundShape;
	Sphere                    *_sphereShape;
	GLTexture             *_groundTexture;
	GLTexture             *_sphereTexture;
	Sprite                     *_testSprite;
//地面的模型矩阵
	Matrix                   _groundModelMatrix;
	Matrix3                 _groundNormalMatrix;
//以光源的角度的视图矩阵
	GLVector3             _lightEyePosition;
	GLVector3             _lightViewPosition;
	Matrix                    _lightViewMatrix;
//以人的眼睛观察得到的矩阵
	Matrix                    _eyeViewMatrix;
	GLVector3             _eyePosition;
	GLVector3             _viewPosition;
//程序数据
	Matrix                    _identity;
	Matrix                    _projectMatrix;
	Matrix					    _otherProjMatrix;
	GLVector3             _lightDirection;
	GLVector3              _lightPosition;
	GLVector3              _ambientColor;
	GLVector3              _lightColor;
	float                        _specularCoeff;
//位置
	float                       _angle;
};
//
 
void        Init(GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;
    
	_user->_lightRender = new   LightRender();
	_user->_groundShape = Mesh::createWithIntensity(4, 1.0f, 1.0f, 1.0f);
	_user->_sphereShape = Sphere::createWithSlice(128, 1.0f);
	_user->_groundTexture = GLTexture::createWithFile("tga/map/ground.tga");
	_user->_sphereTexture = GLTexture::createWithFile("tga/Earth512x256.tga");
//	_user->_testSprite = Sprite::createWithFile("tga/global_bg_big.tga");
//关于光源的矩阵
	_user->_lightEyePosition = GLVector3(3.0,15.0,-10.0);
	_user->_lightViewPosition= GLVector3(0.0f,0.0f,-10.0f);
	_user->_lightViewMatrix.lookAt(_user->_lightEyePosition, _user->_lightViewPosition, GLVector3(0.0f, 1.0, 0.0f));
//设置关于眼睛的参数
//正交矩阵使用
//	_user->_eyePosition = GLVector3(3.0f,8.0f,-5.0f);
//	_user->_viewPosition = GLVector3(0.0f,0.0f,-10.0f);
//透视矩阵使用
	_user->_eyePosition = GLVector3(0.0f,16.0f,-0.0f);
	_user->_viewPosition = GLVector3(0.0f,0.0f,-6.0f);
	_user->_eyeViewMatrix.lookAt(_user-> _eyePosition, _user->_viewPosition, GLVector3(0.0f,1.0f,0.0f));
//	_user->_eyeViewMatrix.lookAt(_user->_lightEyePosition, _user->_lightViewPosition, GLVector3(0.0f, 1.0f, 0.0f));
//设置正交投影矩阵
	_user->_projectMatrix.orthoProject(-15.0f,15.0f,-15.0f,15.0f,-10.0f,30.0f);
	Size     _size = _context->getWinSize();
//	_user->_projectMatrix.frustum(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 30.0f);
//	_user->_projectMatrix.perspective(60.0f, _size.width / _size.height, 0.1f,30.0f);
	_user->_otherProjMatrix.perspective(60.0f, _size.width / _size.height, 0.1f, 30.0f);
	_context->setNearFarPlane(GLVector2(0.1f,30.0f));
//和地面相关的矩阵
	_user->_groundModelMatrix.scale(10.0f, 10.0f, 10.0f);
	_user->_groundModelMatrix.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
	_user->_groundModelMatrix.translate(0.0f, 0.0f, -10.0f);
	_user->_groundNormalMatrix = _user->_groundModelMatrix.normalMatrix();
//设置和光线相关的数据
	_user->_lightColor = GLVector3(1.0f,1.0f,1.0f);
	_user->_lightDirection =(_user->_lightEyePosition-_user->_lightViewPosition).normalize() ;
	_user->_ambientColor = GLVector3(0.2f,0.2f,0.2f);
	_user->_specularCoeff = 0.36f;
	_user->_angle = 0.0f;
//
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
//注意,在阴影的生成中,一般都需要要开启多边形偏移,因为在一般的情况下,都会因为深度的冲突而造成阴影的混乱,读者可以尝试把下面的
//两句代码注释掉重编译,看一下生成的结果
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 100.0f);
}
//阴影
void         drawShadow(GLContext    *_context)
{
	UserData	*_user = (UserData *)_context->userObject;
	int      _default_framebufferId;
	LightRender		*_lightRender = _user->_lightRender;

	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &_default_framebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _lightRender->_framebufferId);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	_lightRender->_glProgram->perform();
//地面
	_user->_groundShape->bindVertexObject(0);
	Matrix                  _mvpMatrix = _user->_groundModelMatrix * _user->_lightViewMatrix * _user->_projectMatrix;
	glUniformMatrix4fv(_lightRender->_mvpMatrixLoc, 1, GL_FALSE, _mvpMatrix.pointer());
	_user->_groundShape->drawShape();
//球体
	_lightRender->_glProgram->perform();
	_user->_sphereShape->bindVertexObject(0);
	Matrix      _modelMatrix;
	_modelMatrix.scale(3.0f,3.0f,3.0f);
	_modelMatrix.translate(2.0f,0.0f,0.0f);
	_modelMatrix.rotate(_user->_angle, 0.0f, 1.0f, 0.0f);
	_modelMatrix.translate(-1.0f, 10.0f, -8.0f);
	_mvpMatrix = _modelMatrix * _user->_lightViewMatrix * _user->_projectMatrix;
	glUniformMatrix4fv(_lightRender->_mvpMatrixLoc,1,GL_FALSE,_mvpMatrix.pointer());
	_user->_sphereShape->drawShape();
//
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _default_framebufferId);
}

void         drawShadowLight(GLContext		*_context)
{
	UserData		*_user = (UserData *)_context->userObject;

	//画地面
	Matrix          _mvpMatrix = _user->_groundModelMatrix * _user->_eyeViewMatrix *_user->_otherProjMatrix;
	Matrix          _lightMatrix = _user->_groundModelMatrix*_user->_lightViewMatrix*_user->_projectMatrix;
	//perform shader
	ShadowLight		*_afterLight = &_user->_shadowLight;
	_afterLight->_afterProgram->perform();
	_user->_groundShape->bindVertexObject(0);
	_user->_groundShape->bindTexCoordObject(1);
	_user->_groundShape->bindNormalObject(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->_groundTexture->name());
	glUniform1i(_afterLight->_baseMapLoc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _user->_lightRender->_depthTextureId);
	glUniform1i(_afterLight->_depthMapLoc,1);

	glUniformMatrix4fv(_afterLight->_mvpMatrixLoc, 1, GL_FALSE, _mvpMatrix.pointer());
	glUniformMatrix4fv(_afterLight->_modelMatrixLoc, 1, GL_FALSE, _user->_groundModelMatrix.pointer());
	glUniformMatrix4fv(_afterLight->_lightMatrixLoc, 1, GL_FALSE, _lightMatrix.pointer());
	glUniformMatrix3fv(_afterLight->_normalMatrixLoc, 1, GL_FALSE, _user->_groundNormalMatrix.pointer());

	glUniform3fv(_afterLight->_lightColorLoc, 1, &_user->_lightColor.x);
	glUniform3fv(_afterLight->_lightDirectionLoc, 1, &_user->_lightDirection.x);
	glUniform3fv(_afterLight->_ambientColorLoc, 1, &_user->_ambientColor.x);
	glUniform3fv(_afterLight->_eyePositionLoc, 1, &_user->_eyePosition.x);
	glUniform1f(_afterLight->_specularCoeffLoc,_user->_specularCoeff);
//	glUniform2fv(_afterLight->_nearFarLoc, 1, &_context->getNearFarPlane().x);

	_user->_groundShape->drawShape();
////球
	_afterLight->_afterProgram->perform();
	_user->_sphereShape->bindVertexObject(0);
	_user->_sphereShape->bindTexCoordObject(1);
	_user->_sphereShape->bindNormalObject(2);

	glUniform3fv(_afterLight->_lightColorLoc, 1, &_user->_lightColor.x);
	glUniform3fv(_afterLight->_lightDirectionLoc, 1, &_user->_lightDirection.x);
	glUniform3fv(_afterLight->_ambientColorLoc, 1, &_user->_ambientColor.x);
	glUniform3fv(_afterLight->_eyePositionLoc, 1, &_user->_eyePosition.x);
	glUniform1f(_afterLight->_specularCoeffLoc, _user->_specularCoeff);
//	glUniform2fv(_afterLight->_nearFarLoc, 1, &_context->getNearFarPlane().x);

	Matrix           _modelMatrix;
	_modelMatrix.scale(3.0f, 3.0f, 3.0f);
	_modelMatrix.translate(2.0f, 0.0f, 0.0f);
	_modelMatrix.rotate(_user->_angle, 0.0f, 1.0f, 0.0f);
	_modelMatrix.translate(-1.0f, 10.0f, -8.0f);

	Matrix3       _normalMatrix = _modelMatrix.normalMatrix();
	_mvpMatrix = _modelMatrix * _user->_eyeViewMatrix * _user->_otherProjMatrix;
	_lightMatrix = _modelMatrix*_user->_lightViewMatrix*_user->_projectMatrix;
	glUniformMatrix4fv(_afterLight->_mvpMatrixLoc, 1, GL_FALSE, _mvpMatrix.pointer());
	glUniformMatrix4fv(_afterLight->_modelMatrixLoc, 1, GL_FALSE, _modelMatrix.pointer());
	glUniformMatrix4fv(_afterLight->_lightMatrixLoc, 1, GL_FALSE, _lightMatrix.pointer());
	glUniformMatrix3fv(_afterLight->_normalMatrixLoc, 1, GL_FALSE, _normalMatrix.pointer());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->_sphereTexture->name());
	glUniform1i(_afterLight->_baseMapLoc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _user->_lightRender->_depthTextureId);
	glUniform1i(_afterLight->_depthMapLoc, 1);

	_user->_sphereShape->drawShape();
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_angle += 12.0f*_deltaTime;
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawShadow(_context);
	drawShadowLight(_context);
//	Matrix  _identity;
//	_user->_testSprite->render(&_identity,_user->_lightRender->_depthTextureId);
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_lightRender->release();
	_user->_groundShape->release();
	_user->_sphereShape->release();
	_user->_groundTexture->release();
	_user->_sphereTexture->release();
	delete  _user;
	_context->userObject = NULL;
}
///////////////////////////don not modify below function////////////////////////////////////
