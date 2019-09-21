//
#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/TGAImage.h>
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
//
#include<stdio.h>
//Common  Data  Struct
//引擎测试
//define   constant
//
//Common  Data  Struct
struct       UserData
{
	GLProgram         *object;
	//矩阵统一变量
	GLuint                  mvpLoc;
	GLuint                  mvLoc;
	GLuint                  baseMapLoc;
	GLuint                  lightColorLoc;
	
	GLSphere            *mSphere;

	//earth  rotate angle
	float                      earthAngle;
	float                      moonAngle;
	//texture 
	GLuint                  earthMapId;
	GLuint                  moonMapId;
	ESMatrix              viewMatrix;
};

void        Init(GLContext    *_context)
{
	_context->userObject = new   UserData();
	UserData      *_user = (UserData *)_context->userObject;
	_user->object = GLProgram::createWithFile("shader/sun/sun.vsh", "shader/sun/sun.fsh");
	_user->mvpLoc = _user->object->getUniformLocation("u_mvpMatrix");
	_user->mvLoc = _user->object->getUniformLocation("u_mvMatrix");
	_user->baseMapLoc = _user->object->getUniformLocation("u_baseMap");
	_user->lightColorLoc = _user->object->getUniformLocation("u_lightColor");

	TGAImage       _earthMap("tga/Earth512x256.tga");
	_user->earthMapId = _earthMap.genTextureMap();

	TGAImage       _moonMap("tga/Moon256x128.tga");
	_user->moonMapId = _moonMap.genTextureMap();

	_user->earthAngle = 0.0f;
	_user->moonAngle = 0.0f;

	_user->mSphere = GLSphere::createWithSlice(256, 0.6f);
	//
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
//	glEnable(GL_BLEND);
	//	return    true;
	esMatrixLookAt(&_user->viewMatrix, &GLVector3(0.0f,0.0f,0.0f),&GLVector3(0.0f,0.0f,-5.0f),&GLVector3(0.0f,1.0f,0.0f));
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->earthAngle += _deltaTime*15.0f;
	_user->moonAngle += _deltaTime*10.0f;
	if (_user->earthAngle >= 360.0f)
		_user->earthAngle -= 360.0f;
	if (_user->moonAngle >= 360.0f)
		_user->moonAngle -= 360.0f;
}
void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//very important
	//draw earth
	_user->object->enableObject();

	_user->mSphere->bindVertexObject(0);
	_user->mSphere->bindTexCoordObject(1);
	_user->mSphere->bindNormalObject(2);
	//
	ESMatrix     mvMatrix;
	esMatrixLoadIdentity(&mvMatrix);
	esRotate(&mvMatrix, 17.7f, 1.0f, 0.0f, 0.0f);
	esRotate(&mvMatrix, _user->earthAngle, 0.0f, 1.0f, 0.0f);
	esTranslate(&mvMatrix, 0.0f, 0.0f, -5.0f);

	glUniformMatrix4fv(_user->mvLoc, 1, GL_FALSE, (float*)mvMatrix.m);//mv matrix

	ESMatrix    projMatrix, mvpMatrix;
	esMatrixLoadIdentity(&projMatrix);
	esFrustum(&projMatrix, -1.0f, 1.0f, -1.0f, 1.0f, 3.0f, 10.0f);
	esMatrixMultiply(&mvpMatrix, &mvMatrix, &projMatrix);
	glUniformMatrix4fv(_user->mvpLoc, 1, GL_FALSE, (float *)mvpMatrix.m);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->earthMapId);
	glUniform1i(_user->baseMapLoc, 0);

	glUniform4fv(_user->lightColorLoc, 1, (float *)&GLVector4(0.9f,0.9f,1.0f,1.0f));
	_user->mSphere->drawShape();

//画出月球

	_user->object->enableObject();
	_user->mSphere->bindVertexObject(0);
	_user->mSphere->bindTexCoordObject(1);
	_user->mSphere->bindNormalObject(2);

	esMatrixLoadIdentity(&mvMatrix);
	esScale(&mvMatrix, 0.25f, 0.25f, 0.25f);
	esRotate(&mvMatrix, 17.7f, 1.0f, 0.0f, 0.0f);
	esRotate(&mvMatrix,_user->moonAngle,10.0f,1.0f,0.0f);//自传
	esTranslate(&mvMatrix, 2.0f, 0.0f, 0.0f);
	esRotate(&mvMatrix, _user->moonAngle, 0.0f, 1.0f, 0.0f);
	esTranslate(&mvMatrix, 0.0f, 0.0f, -5.0f);
	esMatrixMultiply(&mvMatrix, &mvMatrix, &_user->viewMatrix);

	glUniformMatrix4fv(_user->mvLoc, 1, GL_FALSE, (float*)mvMatrix.m);

	esMatrixLoadIdentity(&projMatrix);
	esFrustum(&projMatrix, -1.0f, 1.0f, -1.0f, 1.0f, 3.0f, 10.0f);
	esMatrixMultiply(&mvpMatrix, &mvMatrix, &projMatrix);
	glUniformMatrix4fv(_user->mvpLoc, 1, GL_FALSE, (float*)mvpMatrix.m);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->moonMapId);
	glUniform1i(_user->baseMapLoc, 0);

	glUniform4fv(_user->lightColorLoc,1,(float *)&GLVector4(1.0f,1.0f,1.0f,1.0f));
	_user->mSphere->drawShape();

}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->object->release();
	glDeleteTextures(1, &_user->earthMapId);
	glDeleteTextures(1, &_user->moonMapId);
	_user->mSphere->release();
	_user->object = NULL;
	_user->earthMapId = 0;
	_user->moonMapId = 0;
}
///////////////////////////don not modify below function////////////////////////////////////
