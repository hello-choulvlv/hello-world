// OpenGL.cpp : 定义控制台应用程序的入口点。
//
#include <GL/glew.h>
#include<GL/freeglut.h>
#include<engine/GLContext.h>
#include<engine/TGAImage.h>
#include<engine/GLProgram.h>
#include<engine/glState.h>
#include<engine/ESMatrix.h>
#include"sphere.h"
//define   constant
//
//Common  Data  Struct
struct       UserData
{
	GLProgram         *object;
	//矩阵统一变量
	//ESMatrix              mvpMatrix;
	//ESMatrix              mvMatrix;
	//ESMatrix			    pMatrix;
	GLuint                  mvpLoc;
	GLuint                  mvLoc;
	GLuint                  baseMapLoc;
	//vertex buffer
	GLuint                  positionVertexBufferId;
	GLuint                  texCoordVertexBufferId;
	GLuint                  normalVertexBufferId;
	//earth  rotate angle
	float                      earthAngle;
	float                      moonAngle;
	//texture 
	GLuint                  earthMapId;
	GLuint                  moonMapId;
};

void        Init(GLContext    *_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	_user->object = GLProgram::createWithFile("shader/sun/sun.vsh", "shader/sun/sun.fsh");
	_user->mvpLoc = _user->object->getUniformLocation("u_mvpMatrix");
	_user->mvLoc = _user->object->getUniformLocation("u_mvMatrix");
	_user->baseMapLoc = _user->object->getUniformLocation("u_baseMap");

	TGAImage       _earthMap("tga/Earth512x256.tga");
	_user->earthMapId = _earthMap.genTextureMap();

	TGAImage       _moonMap("tga/Moon256x128.tga");
	_user->moonMapId = _moonMap.genTextureMap();
	//load identity
	//esMatrixLoadIdentity(&_user->mvpMatrix);
	////	esTranslate(&_user->mvpMatrix,0.0f,0.0f,-5.0f);
	//esRotate(&_user->mvpMatrix, 23.0f, 1.0f, 0.0f, 0.0f);
	_user->earthAngle = 0.0f;
	_user->moonAngle = 0.0f;
	//esMatrixLoadIdentity(&_user->pMatrix);
	//esOrtho(&_user->pMatrix, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 120.0f);
	//create buffers
	glGenBuffers(1, &_user->positionVertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _user->positionVertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVerts), sphereVerts, GL_STATIC_DRAW);

	glGenBuffers(1, &_user->texCoordVertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _user->texCoordVertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphereTexCoords), sphereTexCoords, GL_STATIC_DRAW);

	glGenBuffers(1, &_user->normalVertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _user->normalVertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphereNormals), sphereNormals, GL_STATIC_DRAW);
	//
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
//	return    true;
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

	glBindBuffer(GL_ARRAY_BUFFER, _user->positionVertexBufferId);
	glEnableVertexAttribArray(0);//position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);

	glBindBuffer(GL_ARRAY_BUFFER, _user->texCoordVertexBufferId);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);

	glBindBuffer(GL_ARRAY_BUFFER, _user->normalVertexBufferId);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	//
	ESMatrix     mvMatrix;
	esMatrixLoadIdentity(&mvMatrix);
	esTranslate(&mvMatrix, 0.0f, 0.0f, -5.0f);
	esRotate(&mvMatrix, 17.7f , 1.0f, 0.0f, 0.0f);
	esRotate(&mvMatrix, _user->earthAngle, 0.0f, 1.0f, 0.0f);

	glUniformMatrix4fv(_user->mvLoc, 1, GL_FALSE,(float*)mvMatrix.m);//mv matrix

	ESMatrix    projMatrix, mvpMatrix;
	esMatrixLoadIdentity(&projMatrix);
	esOrtho(&projMatrix, -1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);
	esMatrixMultiply(&mvpMatrix, &mvMatrix, &projMatrix);
	glUniformMatrix4fv(_user->mvpLoc, 1, GL_FALSE, (float *)mvpMatrix.m);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->earthMapId);
	glUniform1i(_user->baseMapLoc, 0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_TRIANGLES, 0, sphereNumVerts);
	//画出月球
	
_user->object->enableObject();
	glBindBuffer(GL_ARRAY_BUFFER, _user->positionVertexBufferId);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2 * sizeof(float), GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);

	esMatrixLoadIdentity(&mvMatrix);
	esTranslate(&mvMatrix, 0.0f, 0.0f, -5.0f);
	esScale(&mvMatrix, 0.25f, 0.25f, 0.25f);
	esRotate(&mvMatrix, 17.7f, 1.0f, 0.0f, 0.0f);
	esRotate(&mvMatrix, _user->moonAngle, 0.0f, 1.0f, 0.0f);
	esTranslate(&mvMatrix, 4.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(_user->mvLoc, 1, GL_FALSE, (float*)mvMatrix.m);

	esMatrixLoadIdentity(&projMatrix);
	esOrtho(&projMatrix, -1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);
	esMatrixMultiply(&mvpMatrix, &mvMatrix, &projMatrix);
	glUniformMatrix4fv(_user->mvpLoc, 1, GL_FALSE, (float*)mvpMatrix.m);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->moonMapId);
	glUniform1i(_user->baseMapLoc, 0);

	glBlendFunc(GL_ONE, GL_ZERO);
	glDrawArrays(GL_TRIANGLES, 0, sphereNumVerts);
	
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	 _user->object->release();
	glDeleteBuffers(1, &_user->positionVertexBufferId);
	glDeleteBuffers(1, &_user->texCoordVertexBufferId);
	glDeleteBuffers(1, &_user->normalVertexBufferId);
	glDeleteTextures(1, &_user->earthMapId);
	glDeleteTextures(1, &_user->moonMapId);
	_user->object = NULL;
	_user->positionVertexBufferId = 0;
	_user->texCoordVertexBufferId = 0;
	_user->normalVertexBufferId = 0;
	_user->earthMapId = 0;
	_user->moonMapId = 0;
}
//此函数必须实现,并且在这个函数中不能调用OpenGL函数
//函数的目的,设置GLContext里面属性的值,以及各种回调函数
void      glMain(GLContext    *_context)
{
			_context->userObject = new   UserData();
			_context->setDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
			_context->setWinSize(Size(640,480));
			_context->setWinPosition(GLVector2(220,140));
			_context->registerInitFunc(Init);
			_context->registerUpdateFunc(Update);
			_context->registerDrawFunc(Draw);
			_context->registerShutDownFunc(ShutDown);
}
