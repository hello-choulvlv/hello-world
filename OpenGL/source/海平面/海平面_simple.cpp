#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/TGAImage.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
struct       UserData
{
	GLProgram	*object;
	//纹理
	unsigned          baseMapId;
	Matrix             u_mvpMatrix;
	GLVector3       u_originPosition;
	GLVector3       u_totalAngle;
	float                 u_spanWidth;
	float                 u_frameAngle;
	//位置
	unsigned         u_mvpMatrixLoc;
	unsigned         u_baseMapLoc;
	unsigned         u_originPositionLoc;
	unsigned         u_totalAngleLoc;
	unsigned         u_spanWidthLoc;
	unsigned         u_frameAngleLoc;
	Mesh              *_meshObject;
};
//

void        Init(GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	_user->object = GLProgram::createWithFile("shader/water/water_simple.vsh", "shader/water/water_simple.fsh");
	_user->u_mvpMatrixLoc = _user->object->getUniformLocation("u_mvpMatrix");
	_user->u_baseMapLoc = _user->object->getUniformLocation("u_baseMap");
	_user->u_originPositionLoc = _user->object->getUniformLocation("u_originPosition");
	_user->u_spanWidthLoc = _user->object->getUniformLocation("u_spanWidth");
	_user->u_frameAngleLoc = _user->object->getUniformLocation("u_frameAngle");
	_user->u_totalAngleLoc = _user->object->getUniformLocation("u_totalAngle");
//写入数据
	_user->u_mvpMatrix.identity();
	_user->u_mvpMatrix.rotate(-80.0f,1.0f,0.0f,0.0f);
_user->u_mvpMatrix.translate(0.0f, -0.9f, 0.0f);
//视图矩阵
	Matrix        viewMatrix;
	viewMatrix.lookAt(GLVector3(0,8.0f,1.0f),GLVector3(0.0f,0.0f,-10.0f),GLVector3(0.0f,1.0f,0.0f));
	_user->u_mvpMatrix.multiply(viewMatrix);
	Size     _size = _context->getWinSize();
	_user->u_mvpMatrix.perspective(45.0f, _size.width/_size.height,0.1f,300.0f);

	_user->u_originPosition = GLVector3(0.0f,0.0f,-2.0f);
	_user->u_totalAngle = GLVector3(20.0f*__MATH_PI__, 20.0f*__MATH_PI__, 20.0f*__MATH_PI__);
	_user->u_spanWidth = 40.0f;
	_user->u_frameAngle = 0.0f;
//网格对象
	_user->_meshObject = Mesh::createWithIntensity(20, 16.0f, 16.0f, 16.0f);

	TGAImage		_baseMap("tga/water/water.bmp");
	_user->baseMapId = _baseMap.genTextureMap(GL_REPEAT);
//
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->u_frameAngle += _deltaTime*0.787;
	//if (_user->u_frameAngle >= 360.0f)
	//	_user->u_frameAngle -= 360.0f;
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_user->object->enableObject();
	_user->_meshObject->bindVertexObject(0);
	_user->_meshObject->bindTexCoordObject(1);
//矩阵,向量,标量
	glUniformMatrix4fv(_user->u_mvpMatrixLoc, 1, GL_FALSE, _user->u_mvpMatrix.pointer());
	glUniform3fv(_user->u_originPositionLoc, 1, &_user->u_originPosition.x);
	glUniform3fv(_user->u_totalAngleLoc, 1, &_user->u_totalAngle.x);
	glUniform1f(_user->u_frameAngleLoc, _user->u_frameAngle);
	glUniform1f(_user->u_spanWidthLoc, _user->u_spanWidth);
//纹理
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->baseMapId);
	glUniform1i(_user->u_baseMapLoc, 0);

	_user->_meshObject->drawShape();
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->object->release();
	glDeleteTextures(1, &_user->baseMapId);
	_user->_meshObject->release();
}
///////////////////////////don not modify below function////////////////////////////////////
