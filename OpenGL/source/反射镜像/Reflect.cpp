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
	GLuint               mvMatrixLoc;
//frame buffer
	GLuint              framebufferId;
	GLuint              renderbufferId[4];
//VBO
	GLuint              vertexVBO;
	GLuint              normalVBO;
	GLuint              texVBO;
	GLuint              indiceVBO;
	GLint                numberOfIndice;
//time
	float                  angle;
};
void        InitFramebuffer(GLContext   *_context)
{
	UserData   *_user = (UserData *)_context->userObject;
	Size             _size = _context->getWinSize();
	glGenFramebuffers(1, &_user->framebufferId);
	glGenRenderbuffers(4, _user->renderbufferId);

	glBindRenderbuffer(GL_RENDERBUFFER, _user->renderbufferId[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, (int)_size.width, (int)_size.height);//setting usage

	glBindRenderbuffer(GL_RENDERBUFFER, _user->renderbufferId[1]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, (int)_size.width, (int)_size.height);

	glBindRenderbuffer(GL_RENDERBUFFER, _user->renderbufferId[2]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, (int)_size.width, (int)_size.height);

	glBindRenderbuffer(GL_RENDERBUFFER, _user->renderbufferId[3]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, (int)_size.width, (int)_size.height);
//attachment
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _user->framebufferId);//bind framebuffer
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _user->renderbufferId[0]);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _user->renderbufferId[1]);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, _user->renderbufferId[2]);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, _user->renderbufferId[3]);
//检查完整性
	assert(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		       
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//
}
void        Init(GLContext    *_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	_user->object = GLProgram::createWithFile("shader/chapter-8/framebuffer.vsh", "shader/chapter-8/framebuffer.fsh");
	_user->baseMapLoc = _user->object->getUniformLocation("u_baseMap");
	_user->mvMatrixLoc = _user->object->getUniformLocation("u_mvMatrix");

	TGAImage       baseMap("tga/IceMoon.tga");
	_user->baseMapId = baseMap.genTextureMap();
//create VBO
	float    *position, *texCoord, *normal;
	int       *indice;
	int       numberOfVertex;
	int      numberOfIndice = esGenSphere(256, 0.6f, &position, &normal, &texCoord, &indice, &numberOfVertex);
	_user->numberOfIndice = numberOfIndice;
//
	glGenBuffers(1, &_user->vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _user->vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numberOfVertex, position, GL_STATIC_DRAW);

	glGenBuffers(1, &_user->normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _user->normalVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numberOfVertex, normal, GL_STATIC_DRAW);

	glGenBuffers(1, &_user->texVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _user->texVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * numberOfVertex, texCoord, GL_STATIC_DRAW);

	glGenBuffers(1, &_user->indiceVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _user->indiceVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*numberOfIndice, indice, GL_STATIC_DRAW);

	free(position);
	free(normal);
	free(texCoord);
	free(indice);
//create framebuffer
	InitFramebuffer(_context);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	Size                 _size = _context->getWinSize();
	int                  width=_size.width;
	int                  height=_size.height;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _user->framebufferId);
	GLenum		_attach[3] = {
		      GL_COLOR_ATTACHMENT0,
			  GL_COLOR_ATTACHMENT1,
			  GL_COLOR_ATTACHMENT2
	      };
	glDrawBuffers(3, _attach);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_user->object->enableObject();
//
	glBindBuffer(GL_ARRAY_BUFFER, _user->vertexVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, _user->normalVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, _user->texVBO);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(float)*2,NULL);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->baseMapId);
	glUniform1i(_user->baseMapLoc, 0);

	ESMatrix		mvMatrix;
	esMatrixLoadIdentity(&mvMatrix);
	esRotate(&mvMatrix, _user->angle, 0.0f, 1.0f, 0.0f);

	glUniformMatrix4fv(_user->mvMatrixLoc, 1, GL_FALSE, (float *)mvMatrix.m);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _user->indiceVBO);
	glDrawElements(GL_TRIANGLES, _user->numberOfIndice, GL_UNSIGNED_INT, NULL);
//resume
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _user->framebufferId);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//read
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, width / 2, height, 0, 0, width / 2, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glBlitFramebuffer(width / 2, 0, width, height,width/2,0,width,height,GL_COLOR_BUFFER_BIT,GL_LINEAR);

	glReadBuffer(GL_COLOR_ATTACHMENT2);
	glBlitFramebuffer(0, 0, width, height, width * 3 / 4, height * 3 / 4, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
}

void      glMain(GLContext    *_context)
{
			_context->userObject = new   UserData();
			_context->setDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
			_context->setWinSize(Size(816,638));
			_context->setWinPosition(GLVector2(220,140));
			_context->registerInitFunc(Init);
			_context->registerUpdateFunc(Update);
			_context->registerDrawFunc(Draw);
			_context->registerShutDownFunc(ShutDown);
}
