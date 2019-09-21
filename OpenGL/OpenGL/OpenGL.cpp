#include <GL/glew.h>
#include<engine/GLContext.h>
#include "VSM.h"
#include "LightSpaceSM.h"
//#define USE_LIGHT_SPACE_SM
//ˮ�沨��
struct       UserData
{
#ifdef USE_LIGHT_SPACE_SM
	LightSpaceSM *_godRay;
#else
	VSM  *_godRay;
#endif
};
//

void        Init(glk::GLContext    *_context)
{ 
	UserData	*_user = new   UserData();
	_context->userObject = _user;
#ifdef USE_LIGHT_SPACE_SM
	_user->_godRay = LightSpaceSM::create();
#else
	_user->_godRay =new  VSM();
	_user->_godRay->init();
#endif

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//������ɫ����
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
}
//
void         Update(glk::GLContext   *_context, float   deltaTime,float delayTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_godRay->update(deltaTime,delayTime);
}

void         Draw(glk::GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//glEnable(GL_POLYGON_OFFSET_FILL);
	_user->_godRay->render();
}

void         ShutDown(glk::GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	//_user->_godRay->release();
	delete _user->_godRay;
	_user->_godRay = nullptr;
}
///////////////////////////don not modify below function////////////////////////////////////