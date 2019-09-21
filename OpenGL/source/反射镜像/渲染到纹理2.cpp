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
//mirror
	GLProgram      *mirrorObject;
	GLuint               mirrorMVMatrixLoc;
	GLuint               mirrorBaseMapLoc;
	GLuint               normalLoc;
//cubeMap

//frame buffer
	GLuint              framebufferId;
	GLuint              renderbufferId;
	GLuint              renderTextureId;
//VBO
	GLuint              vertexVBO;
	GLuint              normalVBO;
	GLuint              texVBO;
	GLuint              indiceVBO;
	GLint                numberOfIndice;
//time
	float                  angle;
	GLuint              vertexBufferId;
};
void        InitFramebuffer(GLContext   *_context)
{
	UserData   *_user = (UserData *)_context->userObject;
	Size             _size = _context->getWinSize();
	glGenFramebuffers(1, &_user->framebufferId);
	glGenRenderbuffers(1,& _user->renderbufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _user->framebufferId);//bind framebuffer

	glBindRenderbuffer(GL_RENDERBUFFER, _user->renderbufferId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, (int)_size.width, (int)_size.height);//setting usage
//
	glGenTextures(1, &_user->renderTextureId);
	glBindTexture(GL_TEXTURE_2D, _user->renderTextureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (int)_size.width, (int)_size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	//attachment	
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _user->renderbufferId);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _user->renderTextureId, 0);

//检查完整性
	assert(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		       
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//
}
void        Init(GLContext    *_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	_user->object = GLProgram::createWithFile("shader/chapter-8/framebuffer2.vsh", "shader/chapter-8/framebuffer2.fsh");
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
		_user->mirrorObject = GLProgram::createWithFile("shader/chapter-8/reflect.vsh", "shader/chapter-8/reflect.fsh");
		_user->mirrorMVMatrixLoc = _user->mirrorObject->getUniformLocation("u_mvMatrix");
		_user->mirrorBaseMapLoc = _user->mirrorObject->getUniformLocation("u_baseMap");
		_user->normalLoc = _user->mirrorObject->getUniformLocation("u_normal");
//create framebuffer
	InitFramebuffer(_context);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	_user->angle = 0.0f;
//	glEnable(GL_DEPTH_TEST);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->angle += _deltaTime*15.0f;
	if (_user->angle >= 360.0f)
		_user->angle -= 360.0f;
}
void         SetupSphereVertex(GLContext  *_context)
{
	UserData		*_user = (UserData *)_context->userObject;
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
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, NULL);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->baseMapId);
	glUniform1i(_user->baseMapLoc, 0);

	ESMatrix		mvMatrix;
	esMatrixLoadIdentity(&mvMatrix);
	esTranslate(&mvMatrix, 0.0f, 0.0f, 0.0f);
	esScale(&mvMatrix, 0.5f, 0.5f, 0.5f);
	esRotate(&mvMatrix, _user->angle, 0.0f, 1.0f, 0.0f);

	glUniformMatrix4fv(_user->mvMatrixLoc, 1, GL_FALSE, (float *)mvMatrix.m);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _user->indiceVBO);
	glDrawElements(GL_TRIANGLES, _user->numberOfIndice, GL_UNSIGNED_INT, NULL);
}
void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	Size                 _size = _context->getWinSize();
	int                  width=_size.width;
	int                  height=_size.height;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _user->framebufferId);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SetupSphereVertex(_context);
//resume
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_user->mirrorObject->enableObject();
	glBindBuffer(GL_ARRAY_BUFFER, _user->vertexBufferId);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->baseMapId);
	glUniform1i(_user->mirrorBaseMapLoc, 0);
//matrix
	ESMatrix      mvMatrix;
	esMatrixLoadIdentity(&mvMatrix);
	esTranslate(&mvMatrix, 0.0f, 0.0f, -0.5f);
	esScale(&mvMatrix, 0.5f, 0.5f, 0.5f);
	esRotate(&mvMatrix, 50.0f, 0.0f, 1.0f, 0.0f);

	glUniformMatrix4fv(_user->mirrorMVMatrixLoc, 1, GL_FALSE, (float *)mvMatrix.m);
//normal vector
	glUniform3f(_user->normalLoc,0.0f,0.0f,1.0f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	SetupSphereVertex(_context);
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
