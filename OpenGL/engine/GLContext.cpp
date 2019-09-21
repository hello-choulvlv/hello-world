/*
  *OpenGL上下文实现
  &2016-4-30
  */
#include<engine/GLContext.h>
#include<stdlib.h>
#include<time.h>
#ifdef  _WIN32
#include<windows.h>
#else
#include<sys/time.h>
#endif
  //#include<GL/freeglut.h>
  //#include<engine/GLCacheManager.h>
  //#include<engine/event/EventManager.h>

__NS_GLK_BEGIN
GLContext          GLContext::_singleGLContext;
static       unsigned       _static_last_seed = 0;
GLContext          *GLContext::getInstance()
{
	return    &_singleGLContext;
}
//
GLContext::GLContext()
{
	this->userObject = NULL;
	this->update = NULL;
	this->draw = NULL;
	this->init = NULL;
	this->finalize = NULL;
#ifdef  _WIN32
	_lastTickCount = GetTickCount();
#else
	struct     timeval      tv;
	gettimeofday(&tv, NULL);
	_baseTickCount = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	_lastTickCount = 0;
#endif
	winSize.width = 960;
	winSize.height = 640;
}
//
void    GLContext::registerUpdateFunc(void(*_update)(GLContext *, float,float))
{
	this->update = _update;
}
//
void    GLContext::registerDrawFunc(void(*_draw)(GLContext *))
{
	this->draw = _draw;
}
//
void    GLContext::registerInitFunc(void(*_init)(GLContext *))
{
	this->init = _init;
}
//
void   GLContext::registerShutDownFunc(void(*_finalize)(GLContext *))
{
	this->finalize = _finalize;
}
//
void    GLContext::setWinSize(glk::Size  &_size)
{
	this->winSize = _size;
}
//
void    GLContext::setWinPosition(glk::GLVector2 &_position)
{
	this->winPosition = _position;
}
//
const Size     &GLContext::getWinSize()const
{
	return   this->winSize;
}
//
void       GLContext::setDisplayMode(int  flag)
{
	this->displayMode = flag;
}
//
int          GLContext::getDisplayMode()
{
	return     this->displayMode;
}

void          GLContext::setNearFarPlane(GLVector2 &near_far)
{
	_near_far_plane = near_far;
}

GLVector2     &GLContext::getNearFarPlane()
{
	return _near_far_plane;
}

void        GLContext::setProjMatrix(Matrix   &projMatrix)
{
	_projMatrix = projMatrix;
}

Matrix        &GLContext::getProjMatrix()
{
	return _projMatrix;
}
__NS_GLK_END