#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/TGAImage.h>
#include"SSAO.h"
//水面波纹
struct       UserData
{
	SSAO  *_godRay;
};
//

void        Init(glk::GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	_user->_godRay = SSAO::create();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//开启颜色混溶
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
	//glEnable(GL_POINT_SPRITE);
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	//glPolygonOffset(2, 2.5f);
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
	//
	glEnable(GL_POLYGON_OFFSET_FILL);
	_user->_godRay->defferedRender();
	//glViewport(0, 0, 960, 640);
	//
	glDisable(GL_DEPTH_TEST);
	_user->_godRay->updateOcclusion();
	//
	_user->_godRay->fuzzyOcclusion();
	//
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_user->_godRay->render();
	//
}

void         ShutDown(glk::GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_godRay->release();
	_user->_godRay = nullptr;
}
///////////////////////////don not modify below function////////////////////////////////////
