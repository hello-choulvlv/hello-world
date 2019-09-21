//
#include <GL/glew.h>
#include<GL/freeglut.h>
#include<engine/GLContext.h>
#include<engine/TGAImage.h>
#include<engine/GLProgram.h>
#include<engine/glState.h>
#include<engine/ESMatrix.h>
#include<assert.h>
#include<math.h>
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
	GLuint               criticalLoc;
	GLuint               incLoc;
//time
	float                  angle;
	GLuint              vertexBufferId;
};
void        Init(GLContext    *_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	_user->object = GLProgram::createWithFile("shader/chapter-11/Mandel.vsh", "shader/chapter-11/Mandel.fsh");
	_user->baseMapLoc = _user->object->getUniformLocation("u_baseMap");
	_user->renderColorLoc = _user->object->getUniformLocation("u_renderColor");
	_user->criticalLoc = _user->object->getUniformLocation("u_critical");
	_user->incLoc = _user->object->getUniformLocation("u_inc");
	TGAImage       baseMap("tga/haiyang.tga");
	_user->baseMapId = baseMap.genTextureMap();
	//
		float            _vertex[20] = {
		                  -1.0f,1.0f,0.0f,   0.0f,1.0f,
						  -1.0f,-1.0f,0.0f,  0.0f,0.0f,
						  1.0f,1.0f,0.0f,    1.0f,1.0f,
						  1.0f,-1.0f,0.0f,   1.0f,0.0f
	            };
		glGenBuffers(1, &_user->vertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, _user->vertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(_vertex), _vertex, GL_STATIC_DRAW);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	_user->angle = 147.675f;
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->angle += _deltaTime*0.;
	if (_user->angle >= 360.0f)
		_user->angle -= 360.0f;
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	Size                 _size = _context->getWinSize();
	int                  width=_size.width;
	int                  height=_size.height;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_user->object->enableObject();
	glBindBuffer(GL_ARRAY_BUFFER, _user->vertexBufferId);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),(void *)( 3 * sizeof(float)));
//uniform variable
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->baseMapId);
	glUniform1i(_user->baseMapLoc, 0);

	GLVector4   renderColor(1.0f,0.0f,0.0f,1.0f);
	glUniform4fv(_user->renderColorLoc, 1, (float *)&renderColor);
//	float     angle = _user->angle / 180.0f*3.14159265;
	float     x = sin(_user->angle*0.1367) + cos(_user->angle*0.2377)*0.5f;
	float    y = cos(_user->angle*0.1367) + sin(_user->angle*0.2377)*0.5f;
//critical
	glUniform2f(_user->incLoc,x,y);
//	glUniform1f(_user->criticalLoc, 4.5f);
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
