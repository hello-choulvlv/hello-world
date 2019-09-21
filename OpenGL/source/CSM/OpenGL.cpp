#include <GL/glew.h>
#include<engine/GLContext.h>
#include"CascadeShadowMap.h"
//ˮ�沨��
struct       UserData
{
	CascadeShadowMap  *_shadowMapCascade;
};
//

void        Init(glk::GLContext    *_context)
{ 
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	_user->_shadowMapCascade = CascadeShadowMap::createCascadeShadowMap();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
}
//
void         Update(glk::GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_shadowMapCascade->update(_deltaTime);
}

void         Draw(glk::GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//glEnable(GL_POLYGON_OFFSET_FILL);
	_user->_shadowMapCascade->renderLightView();
	_user->_shadowMapCascade->renderCameraView();
}

void         ShutDown(glk::GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_shadowMapCascade->release();
	_user->_shadowMapCascade = nullptr;
}
///////////////////////////don not modify below function////////////////////////////////////
