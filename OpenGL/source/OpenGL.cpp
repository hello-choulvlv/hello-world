//
#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/glState.h>
#include<assert.h>

//
//Common  Data  Struct
struct       UserData
{
};
void        Init(GLContext    *_context)
{
	UserData      *_user = (UserData *)_context->userObject;

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
}

void         Draw(GLContext	*_context)
{

}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
}
