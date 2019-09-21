//
#include <GL/glew.h>
#include<assert.h>
#include<string.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
#include<engine/GLTexture.h>
#include<engine/Sprite.h>
#include<engine/Shape.h>
#include <engine/LightShadowRender.h>
#include "LightPointShadow.h"
/*
  *阴影
 */
struct       ShadowLight
{
	GLProgram		*_afterProgram;
	unsigned                _mvpMatrixLoc;
	unsigned                _modelMatrixLoc;
//	unsigned                _lightMatrixLoc;
	unsigned                _normalMatrixLoc;
	unsigned                _baseMapLoc;
	unsigned                _depthMapLoc;
	unsigned                _lightColorLoc;
	unsigned                _lightPositionLoc;
	unsigned                _ambientColorLoc;
	unsigned                _eyePositionLoc;
	unsigned                _specularCoeffLoc;
	unsigned                _nearFarLoc;
	unsigned                _maxDistanceLoc;
	ShadowLight();
	~ShadowLight();
};
ShadowLight::ShadowLight()
{
	_afterProgram = GLProgram::createWithFile("shader/shadow/shadow_pointlight.vsh", "shader/shadow/shadow_pointlight.fsh");
	_mvpMatrixLoc = _afterProgram->getUniformLocation("u_mvpMatrix");
	_modelMatrixLoc = _afterProgram->getUniformLocation("u_modelMatrix");
//	_lightMatrixLoc = _afterProgram->getUniformLocation("u_lightMatrix");
	_normalMatrixLoc = _afterProgram->getUniformLocation("u_normalMatrix");
	_baseMapLoc = _afterProgram->getUniformLocation("u_baseMap");
	_depthMapLoc = _afterProgram->getUniformLocation("u_depthMap");
	_lightColorLoc = _afterProgram->getUniformLocation("u_lightColor");
	_lightPositionLoc = _afterProgram->getUniformLocation("u_lightPosition");
	_eyePositionLoc = _afterProgram->getUniformLocation("u_eyePosition");
	_ambientColorLoc = _afterProgram->getUniformLocation("u_ambientColor");
	_specularCoeffLoc = _afterProgram->getUniformLocation("u_specularCoeff");
//	_nearFarLoc = _afterProgram->getUniformLocation("u_nearFar");
	_maxDistanceLoc = _afterProgram->getUniformLocation("u_maxDistance");
}
ShadowLight::~ShadowLight()
{
	_afterProgram->release();
} 
struct       UserData
{
	ShadowLight          _shadowLight;
	LightPointShadow	        *_lightRender;
	Mesh                       *_groundShape;
	Sphere                    *_sphereShape;
	GLTexture             *_groundTexture;
	GLTexture             *_sphereTexture;
	Sprite                     *_testSprite;
//地面的模型矩阵
	Matrix                   _groundModelMatrix;
	Matrix3                 _groundNormalMatrix;
//求的模型矩阵
	Matrix                   _sphereModelMatrix;
	Matrix3                 _sphereNormalMatrix;
//以光源的角度的视图矩阵
	GLVector3             _lightEyePosition;
	GLVector3             _lightViewPosition;
	Matrix                    _lightViewMatrix;
	Matrix                    _lightViewMatrixArray[6];//面向六个方向的视图矩阵,注意考虑到
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
	float                        _maxDistance;
//位置
	float                       _angle;
//地面画阴影的时候使用
//	unsigned               _groundShadowVertexArrayId;
//	unsigned               _sphereShadowVertexArrayId;
//地面,球正常的时候使用
	unsigned               _groundVertexArrayId;
	unsigned               _sphereVertexArrayId;
};
//
void       loadLightViewMatrix(GLContext		*_context)
{
	UserData		*_user = (UserData *)_context->userObject;
	Matrix             _mirror;
	float                 *yaxis=_mirror.pointer();
	yaxis[5] = -1.0f;
#define   __USE_NORMAL__
//加载视图矩阵,注意因为立方体贴图在Y轴上会产生镜像反射,所以鸡杂视图矩阵的时候,需要使用相反的上方向
	GLVector3       _lightPosition = _user->_lightPosition;
#ifdef __USE_NORMAL__
	for (int i = 0; i < 6; ++i)
	{
		_user->_lightViewMatrixArray[i].identity();
//		_user->_lightViewMatrixArray[i] = _mirror;
	}
	_user->_lightViewMatrixArray[0].lookAt(_lightPosition,_lightPosition+  GLVector3(1.0f,0.0f,0.0f),GLVector3(0.0f,-1.0f,0.0f));//+X
	_user->_lightViewMatrixArray[1].lookAt(_lightPosition,_lightPosition + GLVector3(-1.0f,0.0f,0.0f),GLVector3(0.0f,-1.0f,0.0f));//-X
	_user->_lightViewMatrixArray[2].lookAt(_lightPosition,_lightPosition+ GLVector3(0.0f,1.0f,0.0f),  GLVector3(0.0f,0.0f,-1.0f));//+Y
	_user->_lightViewMatrixArray[3].lookAt(_lightPosition, _lightPosition + GLVector3(0.0f,-1.0f,0.0f),GLVector3(1.0f,0.0f,1.0f));//-Y
//	_user->_lightViewMatrixArray[2].lookAt(_lightPosition, _lightPosition + GLVector3(0.0f, 1.0f, 0.0f), GLVector3(0.0f, 0.0f, 1.0f));//+Y
//	_user->_lightViewMatrixArray[3].lookAt(_lightPosition, _lightPosition + GLVector3(0.0f, -1.0f, 0.0f), GLVector3(0.0f, 0.0f, -1.0f));//-Y
	_user->_lightViewMatrixArray[4].lookAt(_lightPosition, _lightPosition + GLVector3(0.0f,0.0f,1.0f), GLVector3(0.0f,-1.0f,0.0f));//+Z
	_user->_lightViewMatrixArray[5].lookAt(_lightPosition, _lightPosition + GLVector3(0.0f, 0.0f, -1.0f), GLVector3(0.0f, -1.0f, 0.0f));//-Z
#else
	for (int i = 0; i < 6; ++i)
		memset(_user->_lightViewMatrixArray, 0x0, sizeof(Matrix) * 6);
#define     __array(a,b,c)    p[4*(a) + b]=c 
#define     __array_get(a,b)   p[4*(a)+b]
	GLVector3      V;
	float                 *_other_pointer;
	_lightPosition = GLVector3() - _lightPosition;
#define     __fix_array3x3       _other_pointer=p+12;\
	                                                __array( 3,0,__array_get(0,0)*_lightPosition.x + __array_get(1,0)*_lightPosition.y + __array_get(2,0)*_lightPosition.z    );\
													__array(3,1, __array_get(0,1 )*_lightPosition.x + __array_get(1,1)*_lightPosition.y + __array_get(2,1)*_lightPosition.z);\
													__array(3,2, __array_get(0,2)*_lightPosition.x+__array_get(1,2)*_lightPosition.y +__array_get(2,2)*_lightPosition.z);
//Positive X
	float        *p = _user->_lightViewMatrixArray[0].pointer();
	__array(0,2,-1);
	__array(1, 1, -1);
	__array(2, 0, -1);
	__array(3,3,1);
	__fix_array3x3
//Negative X
	p = _user->_lightViewMatrixArray[1].pointer();
	__array(0,2,1);
	__array(1,1,-1);
	__array(2,0,1);
	__array(3,3,1);
	__fix_array3x3
//Positive Y
	p = _user->_lightViewMatrixArray[2].pointer();
	__array(0,0,1);
	__array(1,2,1);
	__array(2,1,-1);
	__array(3,3,1);
	__fix_array3x3
//Negative Y
	p = _user->_lightViewMatrixArray[3].pointer();
	__array(0,0,1);
	__array(1,2,-1);
	__array(2,1,1);
	__array(3,3,1);
	__fix_array3x3
//Positive Z
	p = _user->_lightViewMatrixArray[4].pointer();
	__array(0,0,1);
	__array(1,1,-1);
	__array(2,2,-1);
	__array(3,3,1);
	__fix_array3x3
//Negative	Z
	p = _user->_lightViewMatrixArray[5].pointer();
	__array(0,0,-1);
	__array(1,1,-1);
	__array(2,2,1);
	__array(3,3,1);
	__fix_array3x3
#undef __fix_array3x3
#undef _array
#endif
	for (int i = 0; i < 6; ++i)
		_user->_lightViewMatrixArray[i] = _user->_lightViewMatrixArray[i]  * _user->_projectMatrix;
}
void        Init(GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;
    
	_user->_lightRender = new   LightPointShadow();
	_user->_groundShape = Mesh::createWithIntensity(4, 1.0f, 1.0f, 1.0f);
	_user->_sphereShape = Sphere::createWithSlice(128, 1.0f);
	_user->_groundTexture = GLTexture::createWithFile("tga/map/ground.tga");
	_user->_sphereTexture = GLTexture::createWithFile("tga/Earth512x256.tga");
//	_user->_testSprite = Sprite::createWithFile("tga/global_bg_big.tga");
//关于光源的矩阵
	_user->_lightEyePosition = GLVector3(6.0,12.0,-10.0);
	_user->_lightViewPosition= GLVector3(0.0f,0.0f,-10.0f);
	_user->_lightViewMatrix.lookAt(_user->_lightEyePosition, _user->_lightViewPosition, GLVector3(0.0f, 1.0, 0.0f));
//	_user->_lightViewPosition = GLVector3(10.0f,10.0f,-10.0f);
	_user->_lightPosition = _user->_lightEyePosition;
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
//	_user->_projectMatrix.orthoProject(-15.0f,15.0f,-15.0f,15.0f,-10.0f,30.0f);
	Size     _size = _context->getWinSize();
//	_user->_projectMatrix.frustum(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 30.0f);
	_user->_projectMatrix.perspective(90.0f, _size.width / _size.height, 0.1f,40.0f);
	_user->_otherProjMatrix.perspective(90.0f, _size.width / _size.height, 0.1f, 60.0f);
	_context->setNearFarPlane(GLVector2(0.1f,30.0f));
	_user->_maxDistance = 30.0f*4.0f*5.0f;
	loadLightViewMatrix(_context);
//和地面相关的矩阵
	_user->_groundModelMatrix.scale(20.0f, 20.0f, 20.0f);
	_user->_groundModelMatrix.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
	_user->_groundModelMatrix.translate(0.0f, 0.0f, -10.0f);
	_user->_groundNormalMatrix = _user->_groundModelMatrix.normalMatrix();
//和球面相关的模型矩阵
	_user->_sphereModelMatrix.scale(2.0f, 2.0f, 2.0f);
	_user->_sphereModelMatrix.translate(2.0f, 0.0f, 0.0f);
	_user->_sphereModelMatrix.rotate(0.0f, 0.0f, 1.0f, 0.0f);
	_user->_sphereModelMatrix.translate(-1.0f, 10.0f, -8.0f);
//设置和光线相关的数据
	_user->_lightColor = GLVector3(1.0f,1.0f,1.0f);
	_user->_lightDirection =(_user->_lightEyePosition-_user->_lightViewPosition).normalize() ;
	_user->_ambientColor = GLVector3(0.2f,0.2f,0.2f);
	_user->_specularCoeff = 0.36f;
	_user->_angle = 0.0f;
//vertex array buffer
	glGenVertexArrays(1, &_user->_groundVertexArrayId);
	glBindVertexArray(_user->_groundVertexArrayId);
	_user->_groundShape->bindVertexObject(0);
	_user->_groundShape->bindTexCoordObject(1);
	_user->_groundShape->bindNormalObject(2);
	_user->_groundShape->bindIndiceObject();
	glBindVertexArray(0);

	glGenVertexArrays(1, &_user->_sphereVertexArrayId);
	glBindVertexArray(_user->_sphereVertexArrayId);
	_user->_sphereShape->bindVertexObject(0);
	_user->_sphereShape->bindTexCoordObject(1);
	_user->_sphereShape->bindNormalObject(2);
	_user->_sphereShape->bindIndiceObject();
	glBindVertexArray(0);
//
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
//注意,在阴影的生成中,一般都需要要开启多边形偏移,因为在一般的情况下,都会因为深度的冲突而造成阴影的混乱,读者可以尝试把下面的
//两句代码注释掉重编译,看一下生成的结果
//	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 100.0f);
	glEnable(GL_TEXTURE_CUBE_MAP);
}
//阴影
#ifdef  __GEOMETRY_SHADOW__
void         drawShadow(GLContext    *_context)
{
	UserData	*_user = (UserData *)_context->userObject;
	int      _default_framebufferId;
	int      _error;
	LightPointShadow		*_lightRender = _user->_lightRender;
	Size    _size = _context->getWinSize();
//	glCullFace(GL_FRONT);
	glViewport(0, 0, 1024, 1024);
	_error = glGetError();
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_default_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _lightRender->_framebufferId);
	glClear(GL_DEPTH_BUFFER_BIT);
//	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	_lightRender->_shadowProgram->perform();
	_error = glGetError();
	glUniform3fv(_lightRender->_lightPositionLoc, 1, &_user->_lightPosition.x);
	glUniformMatrix4fv(_lightRender->_viewProjMatrixLoc[0],6,GL_FALSE,_user->_lightViewMatrixArray[0].pointer());
	glUniform1f(_lightRender->_maxDistanceLoc,_user->_maxDistance);
//	for (int face = 0; face < 6; ++face)
//	{
//地面
		glBindVertexArray(_user->_groundVertexArrayId);
		glUniformMatrix4fv(_lightRender->_modelMatrixLoc, 1, GL_FALSE, _user->_groundModelMatrix.pointer());
		glDrawElements(GL_TRIANGLES, _user->_groundShape->numberOfIndice(), GL_UNSIGNED_INT, NULL);
		_error = glGetError();
		//球体
		_lightRender->_shadowProgram->perform();
		glBindVertexArray(_user->_sphereVertexArrayId);
		glUniformMatrix4fv(_lightRender->_modelMatrixLoc, 1, GL_FALSE, _user->_sphereModelMatrix.pointer());
		glDrawElements(GL_TRIANGLES, _user->_sphereShape->numberOfIndice(), GL_UNSIGNED_INT, NULL);
		_error = glGetError();
//	}
//	GL_INVALID_ENUM;
//
//	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
#else
void         drawShadow(GLContext    *_context)
{
	UserData	*_user = (UserData *)_context->userObject;
	int      _default_framebufferId;
	int      _error;
	LightPointShadow		*_lightRender = _user->_lightRender;
	Size    _size = _context->getShadowSize();

	glViewport(0, 0, _size.width, _size.height);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_default_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _lightRender->_framebufferId);
//全局只使用一个着色器就可以了
	_lightRender->_shadowProgram->perform();
	glUniform3fv(_lightRender->_lightPositionLoc, 1, &_user->_lightPosition.x);
	for (int face = 0; face < 6; ++face)
	{
		_lightRender->writeFace(face, _user->_lightPosition);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Matrix      &_faceViewMatrix = _lightRender->loadViewMatrix();
//地面
		glBindVertexArray(_user->_groundVertexArrayId);
		glUniformMatrix4fv(_lightRender->_modelMatrixLoc, 1, GL_FALSE, _user->_groundModelMatrix.pointer());
		Matrix   _mvpMatrix = _user->_groundModelMatrix * _faceViewMatrix * _user->_projectMatrix;
		glUniformMatrix4fv(_lightRender->_mvpMatrixLoc, 1, GL_FALSE, _mvpMatrix.pointer());
		glDrawElements(GL_TRIANGLES, _user->_groundShape->numberOfIndice(), GL_UNSIGNED_INT, NULL);
//球体
		glBindVertexArray(_user->_sphereVertexArrayId);
		glUniformMatrix4fv(_lightRender->_modelMatrixLoc, 1, GL_FALSE, _user->_sphereModelMatrix.pointer());
		_mvpMatrix = _user->_sphereModelMatrix * _faceViewMatrix *_user->_projectMatrix;
		glUniformMatrix4fv(_lightRender->_mvpMatrixLoc, 1, GL_FALSE, _mvpMatrix.pointer());
		glDrawElements(GL_TRIANGLES, _user->_sphereShape->numberOfIndice(), GL_UNSIGNED_INT, NULL);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, _default_framebufferId);
}
#endif
void         drawShadowLight(GLContext		*_context)
{
	UserData		*_user = (UserData *)_context->userObject;
	glViewport(0, 0, _context->getWinSize().width, _context->getWinSize().height);
//画地面
	Matrix          _mvpMatrix = _user->_groundModelMatrix * _user->_eyeViewMatrix *_user->_otherProjMatrix;
//perform shader
	ShadowLight		*_afterLight = &_user->_shadowLight;
	_afterLight->_afterProgram->perform();
	glBindVertexArray(_user->_groundVertexArrayId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->_groundTexture->name());
	glUniform1i(_afterLight->_baseMapLoc, 0);
#ifndef __GEOMETRY_SHODOW__
	_user->_lightRender->bindTextureUnit(1);
#else
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _user->_lightRender->_depthTextureId);
#endif
	glUniform1i(_afterLight->_depthMapLoc,1);

	glUniformMatrix4fv(_afterLight->_mvpMatrixLoc, 1, GL_FALSE, _mvpMatrix.pointer());
	glUniformMatrix4fv(_afterLight->_modelMatrixLoc, 1, GL_FALSE, _user->_groundModelMatrix.pointer());
	glUniformMatrix3fv(_afterLight->_normalMatrixLoc, 1, GL_FALSE, _user->_groundNormalMatrix.pointer());

	glUniform3fv(_afterLight->_lightColorLoc, 1, &_user->_lightColor.x);
	glUniform3fv(_afterLight->_lightPositionLoc, 1, &_user->_lightEyePosition.x);
	glUniform3fv(_afterLight->_ambientColorLoc, 1, &_user->_ambientColor.x);
	glUniform3fv(_afterLight->_eyePositionLoc, 1, &_user->_eyePosition.x);
	glUniform1f(_afterLight->_specularCoeffLoc,_user->_specularCoeff);
//	glUniform1f(_afterLight->_maxDistanceLoc,_user->_maxDistance);

	glDrawElements(GL_TRIANGLES, _user->_groundShape->numberOfIndice(), GL_UNSIGNED_INT, NULL);
////球
	_afterLight->_afterProgram->perform();
	glBindVertexArray(_user->_sphereVertexArrayId);

	//glUniform3fv(_afterLight->_lightColorLoc, 1, &_user->_lightColor.x);
	//glUniform3fv(_afterLight->_lightPositionLoc, 1, &_user->_lightEyePosition.x);
	//glUniform3fv(_afterLight->_ambientColorLoc, 1, &_user->_ambientColor.x);
	//glUniform3fv(_afterLight->_eyePositionLoc, 1, &_user->_eyePosition.x);
	//glUniform1f(_afterLight->_specularCoeffLoc, _user->_specularCoeff);
	//glUniform1f(_afterLight->_maxDistanceLoc,_user->_maxDistance);

	Matrix3       _normalMatrix = _user->_sphereModelMatrix.normalMatrix();
	_mvpMatrix = _user->_sphereModelMatrix * _user->_eyeViewMatrix * _user->_otherProjMatrix;
	glUniformMatrix4fv(_afterLight->_mvpMatrixLoc, 1, GL_FALSE, _mvpMatrix.pointer());
	glUniformMatrix4fv(_afterLight->_modelMatrixLoc, 1, GL_FALSE, _user->_sphereModelMatrix.pointer());
	glUniformMatrix3fv(_afterLight->_normalMatrixLoc, 1, GL_FALSE, _normalMatrix.pointer());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->_sphereTexture->name());
	glUniform1i(_afterLight->_baseMapLoc, 0);

	glDrawElements(GL_TRIANGLES, _user->_sphereShape->numberOfIndice(), GL_UNSIGNED_INT, NULL);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_angle += 12.0f*_deltaTime;
	if (_user->_angle >= 360.0f)
		_user->_angle -= 360.0f;

	_user->_sphereModelMatrix.identity();
	_user->_sphereModelMatrix.scale(2.0f, 2.0f, 2.0f);
	_user->_sphereModelMatrix.translate(2.0f, 0.0f, 0.0f);
	_user->_sphereModelMatrix.rotate(_user->_angle, 0.0f, 1.0f, 0.0f);
	_user->_sphereModelMatrix.translate(-1.0f, 10.0f, -8.0f);
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawShadow(_context);
	drawShadowLight(_context);
//	Matrix  _identity;
//	_user->_testSprite->render(&_identity,_user->_lightRender->_depthTextureId);
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//	_user->_testSprite->render();
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
//	_user->_lightRender->release();
//	_user->_groundShape->release();
//	_user->_sphereShape->release();
//	_user->_groundTexture->release();
//	_user->_sphereTexture->release();
	delete  _user;
	_context->userObject = NULL;
}
///////////////////////////don not modify below function////////////////////////////////////
