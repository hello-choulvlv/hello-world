//
#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/GLTexture.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<engine/Sprite.h>
#include<assert.h>
#include "SeaRender.h"
//
//Common  Data  Struct
struct       UserData
{
	GroundRender         *_groundRender;
	GLTexture                *_groundTexture;
	GLTexture                *_lightTexture;
	GLTexture                *_skyTexture;
	Sprite                        *_testSprite;
	Mesh                         *_waterMesh;
	Mesh                         *_groundMesh;
//Matrix
	Matrix                      _groundModelMatrix;
	Matrix3                    _groundNormalMatrix;
	Matrix                      _groundMVPMatrix;
//Water
	Matrix                      _waterModelMatrix;
//View Matrix,ProjMatrix
	Matrix                     _viewMatrix;
	Matrix                     _projMatrix;
//海平面离远点的距离
	float                         _seaPlaneDistance;
	GLVector4               _seaPlaneEquation;
	float                         _elapseTime;
};
void        Init(GLContext    *_context)
{
	UserData      *_user = new  UserData();
	_context->userObject=_user;
//shader and texture
	_user->_groundRender = new  GroundRender();
	_user->_groundTexture = GLTexture::createWithFile("tga/water/ground.tga");
	_user->_lightTexture = GLTexture::createWithFile("tga/light/light.tga");
	_user->_skyTexture = GLTexture::createWithFile("tga/water/sky.tga");
//	_user->_testSprite = Sprite::createWithFile("tga/light/light.tga");
	_user->_groundTexture->setWrapParam(GL_TEXTURE_WRAP_S, GL_REPEAT);
	_user->_groundTexture->setWrapParam(GL_TEXTURE_WRAP_T, GL_REPEAT);
	_user->_lightTexture->setWrapParam(GL_TEXTURE_WRAP_S, GL_REPEAT);
	_user->_lightTexture->setWrapParam(GL_TEXTURE_WRAP_T, GL_REPEAT);
//Vertex
	_user->_groundMesh = Mesh::createWithIntensity(4, 1.0f, 1.0f, 5.0f);
//Ground Matrix
	_user->_groundModelMatrix.scale(10.0f, 10.0f, 10.0f);
	_user->_groundModelMatrix.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
	_user->_groundModelMatrix.translate(0.0f, -8.0f, -8.0f);
	_user->_groundNormalMatrix = _user->_groundModelMatrix.normalMatrix();
//View Matrix
	_user->_viewMatrix.lookAt(GLVector3(8.0f,6.0f,-4.0f),GLVector3(-2.0f,0.0f,-6.0f),GLVector3(0.0f,1.0f,0.0f));
//Project Matrix
	Size        _size = _context->getWinSize();
	_user->_projMatrix.perspective(45.0f, _size.width/_size.height,0.1f,100.0f);
	_user->_groundMVPMatrix = _user->_groundModelMatrix*_user->_viewMatrix*_user->_projMatrix;
//Equation of Sea Plane
	float                 _ydistance = -6.0f;
	GLVector3      _normalabc=GLVector3(0.0f,0.0f,1.0f);
	_user->_seaPlaneDistance = _ydistance;
	_user->_seaPlaneEquation = GLVector4(_normalabc.x,_normalabc.y,_normalabc.z,_ydistance);
//Time
	_user->_elapseTime = 0.0f;
//OpenGL Settings
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_elapseTime += _deltaTime*0.57f;
}

void         Draw(GLContext	*_context)
{
	UserData		*_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	GroundRender	*_groundRender = _user->_groundRender;

	_groundRender->_groundProgram->perform();
	_user->_groundMesh->bindVertexObject(0);
	_user->_groundMesh->bindNormalObject(1);
	_user->_groundMesh->bindTexCoordObject(2);
//Uniform
	glUniformMatrix4fv(_groundRender->_modelMatrixLoc, 1, GL_FALSE, _user->_groundModelMatrix.pointer());
	glUniformMatrix4fv(_groundRender->_mvpMatrixLoc,1,GL_FALSE,_user->_groundMVPMatrix.pointer());
	glUniformMatrix3fv(_groundRender->_normalMatrixLoc, 1, GL_FALSE, _user->_groundNormalMatrix.pointer());

//	glUniform4fv(_groundRender->_planeEquationLoc, 1, &_user->_seaPlaneEquation.x);
	glUniform1f(_groundRender->_timeLoc,_user->_elapseTime);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->_groundTexture->name());
	glUniform1i(_groundRender->_baseMapLoc,0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _user->_lightTexture->name());
	glUniform1i(_groundRender->_lightMapLoc,1);

	_user->_groundMesh->drawShape();
 // _user->_testSprite->render();
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
}
