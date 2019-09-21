//
#include <GL/glew.h>
#include<GL/freeglut.h>
#include<engine/GLContext.h>
#include<engine/TGAImage.h>
#include<engine/GLProgram.h>
#include<engine/glState.h>
#include<engine/ESMatrix.h>
#include<assert.h>
//#include"sphere.h"
//define   constant
#define      ATTR_POSITION            0
#define      ATTR_TEXCOORD          1
#define      ATTR_NORMAL              2
//
//Common  Data  Struct
struct       UserData
{
	GLProgram      *object;
	GLuint               baseMapId;
	GLint                 baseMapLoc;
	GLuint               renderColorLoc;
	GLuint               offsetLoc;
//time
	float                  angle;
	GLuint              vertexBufferId;
};
void        Init(GLContext    *_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	_user->object = GLProgram::createWithFile("shader/depth.vsh", "shader/depth.fsh");
	_user->baseMapLoc = _user->object->getUniformLocation("u_baseMap");
	_user->renderColorLoc = _user->object->getUniformLocation("u_renderColor");
	_user->offsetLoc = _user->object->getUniformLocation("u_offset");
	TGAImage       baseMap("tga/milk.tga");
	_user->baseMapId = baseMap.genTextureMap();
	//
		float            _vertex[20] = {
		                  -0.5f,0.5f,0.0f,   0.0f,1.0f,
						  -0.5f,-0.5f,0.0f,  0.0f,0.0f,
						  0.5f,0.5f,0.0f,    1.0f,1.0f,
						  0.5f,-0.5f,0.0f,   1.0f,0.0f
	            };
		glGenBuffers(1, &_user->vertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, _user->vertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(_vertex), _vertex, GL_STATIC_DRAW);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	_user->angle = 0.0f;
	glEnable(GL_DEPTH_TEST);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->angle += _deltaTime*15.0f;
	if (_user->angle >= 360.0f)
		_user->angle -= 360.0f;
}
void         SetupVertex(GLContext   *_context)
{
  UserData   *_user=(UserData *)_context->userObject;
  
  _user->object->enableObject();
	glBindBuffer(GL_ARRAY_BUFFER, _user->vertexBufferId);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->baseMapId);
	glUniform1i(_user->baseMapLoc, 0);
}
void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	Size                 _size = _context->getWinSize();
	int                  width=_size.width;
	int                  height=_size.height;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//////////////////////////////////////////////////////////////////
  SetupVertex(_context);
  GLVector3      _offset1(0.3,0.0,0.3);
  GLVector4      _renderColor1(1.0f,0.0f,0.0f,1.0f);
  
  glUniform3fv(_user->offsetLoc,1,(float *)&_offset1);
  glUniform4fv(_user->renderColorLoc,1,(float *)&_renderColor1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//////////////////////////////////////////////////////////////
 SetupVertex(_context);
   GLVector3      _offset2(-0.3,0.0,-0.3);
  GLVector4      _renderColor2(0.0f,1.0f,0.0f,1.0f);
  
  glUniform3fv(_user->offsetLoc,1,(float *)&_offset2);
  glUniform4fv(_user->renderColorLoc,1,(float *)&_renderColor2);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
///////////////////////////////////////////////////////////////////
  SetupVertex(_context);
  GLVector3      _offset3(0.0,0.0,0.0);
  GLVector4      _renderColor3(0.0f,0.0f,1.0f,1.0f);
  
  glUniform3fv(_user->offsetLoc,1,(float *)&_offset3);
  glUniform4fv(_user->renderColorLoc,1,(float *)&_renderColor3);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
}
///////////////////////////don not modify below function////////////////////////////////////
void      glMain(GLContext    *_context)
{
	_context->userObject = new   UserData();
	_context->setDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	_context->setWinSize(Size(816, 638));
	_context->setWinPosition(GLVector2(220, 140));
	_context->registerInitFunc(Init);
	_context->registerUpdateFunc(Update);
	_context->registerDrawFunc(Draw);
	_context->registerShutDownFunc(ShutDown);
}
