#include <GL/glew.h>
#include<engine/GLContext.h>
#include"PointShadow.h"
//水面波纹
struct       UserData
{
	PointShadow  *_godRay;
};
//

void        Init(glk::GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	_user->_godRay = new PointShadow();
	_user->_godRay->initShadow();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(4.0f, 4.0f);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//开启颜色混溶
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
	//glEnable(GL_POINT_SPRITE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glPolygonOffset(2, 2.5f);
}
//
void         Update(glk::GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_godRay->update(_deltaTime);
}

void         Draw(glk::GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_POLYGON_OFFSET_FILL);
	glColorMask(0,0,0,0);
	glViewport(0, 0, 512, 512);
	//
	_user->_godRay->renderShadow();
	//
	glColorMask(1,1,1,1);
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	glViewport(0, 0, winSize.width, winSize.height);
	_user->_godRay->render();
}

void         ShutDown(glk::GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_godRay->release();
	_user->_godRay = nullptr;
}
///////////////////////////don not modify below function////////////////////////////////////
