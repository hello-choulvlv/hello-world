#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/GLTexture.h>
#include<engine/Geometry.h>
#include "ChoppySimulate.h"
//Ë®Ãæ²¨ÎÆ
struct       UserData
{
	ChoppySimulate		*_choppySimulate;
};
//

void        Init(GLContext    *_context)
{ 
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	_user->_choppySimulate = new ChoppySimulate();
	_user->_choppySimulate->init();
	Size    _size = _context->getWinSize();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_choppySimulate->update(_deltaTime);
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_user->_choppySimulate->draw();
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_choppySimulate->release();
}
///////////////////////////don not modify below function////////////////////////////////////
