//
#include <GL/glew.h>
#include<GL/freeglut.h>
#include<engine/GLContext.h>
#include<engine/TGAImage.h>
#include<engine/GLProgram.h>
#include<engine/glState.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<assert.h>
#include<math.h>
//
//Common  Data  Struct
//场景
struct       UserData
{
	GLProgram   *object;
	GLuint           baseMapId;
	GLuint           baseMapLoc;
//Stencil
	GLuint           stencilMapId;
//vertex buffer
	GLuint            vertexVBO;
};
//
void     Init(GLContext	*_context)
{
	UserData  *_user = (UserData *)_context->userObject;
	_user->object = GLProgram::createWithFile("shader/tex_normal.vsh", "shader/tex_normal.fsh");
	_user->baseMapLoc = _user->object->getUniformLocation("u_baseMap");

	TGAImage   baseMap("tga/haiyang.tga");
	_user->baseMapId = baseMap.genTextureMap();

	TGAImage   stencilMap("tga/city_back.tga");
	_user->stencilMapId = stencilMap.genTextureMap();
//
	float   vertex[20] = {
		           -1.0f,1.0f,0.0f,0.0f,1.0f,
				   -1.0f,-1.0f,0.0f,0.0f,0.0f,
				   1.0f,1.0f,0.0f,1.0f,1.0f,
				   1.0f,-1.0f,0.0f,1.0f,0.0f
	     };
	glGenBuffers(1, &_user->vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _user->vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
//
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearStencil(0x0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glAlphaFunc(GL_GEQUAL, 0.05f);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
}

void         Draw(GLContext	*_context)
{
	UserData   *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	_user->object->enableObject();
	
	glBindBuffer(GL_ARRAY_BUFFER, _user->vertexVBO);
	glEnableVertexAttribArray(GLAttribPosition);
	glVertexAttribPointer(GLAttribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);

	glEnableVertexAttribArray(GLAttribTexCoord);
	glVertexAttribPointer(GLAttribTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(sizeof(float) * 3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->stencilMapId);
	glUniform1i(_user->baseMapLoc, 0);

	glEnable(GL_STENCIL_TEST);
	glEnable(GL_ALPHA_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE,GL_FALSE);
	glDepthMask(GL_FALSE);
	glStencilFunc(GL_ALWAYS, 0x1, 0xFF);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//恢复深度缓冲区,颜色缓冲区的写入
	glDisable(GL_ALPHA_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
//改变模板缓冲区的写入规则,参考值1必须等于模板值时比较才能通过
	glStencilFunc(GL_EQUAL, 0x1, 0xFF);

	_user->object->enableObject();
	glBindBuffer(GL_ARRAY_BUFFER, _user->vertexVBO);
	glEnableVertexAttribArray(GLAttribPosition);
	glVertexAttribPointer(GLAttribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);

	glEnableVertexAttribArray(GLAttribTexCoord);
	glVertexAttribPointer(GLAttribTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(sizeof(float) * 3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->baseMapId);
	glUniform1i(_user->baseMapLoc, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
}
///////////////////////////don not modify below function////////////////////////////////////
void      GLMainEntrySetting(GLContext    *_context)
{
	_context->userObject = new   UserData();
	_context->setDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH|GLUT_STENCIL);
	_context->setWinSize(Size(816, 638));
	_context->setWinPosition(GLVector2(220, 140));
	_context->registerInitFunc(Init);
	_context->registerUpdateFunc(Update);
	_context->registerDrawFunc(Draw);
	_context->registerShutDownFunc(ShutDown);
}
