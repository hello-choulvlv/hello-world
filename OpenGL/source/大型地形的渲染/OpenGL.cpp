#include <GL/glew.h>
#include<engine/GLContext.h>
#include"Terrain.h"
//水面波纹
struct       UserData
{
	Terrain  *_terrain;
};
//

void        Init(glk::GLContext    *_context)
{ 
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	_user->_terrain = Terrain::createTerrainWithFile("tga/terrain/Terrain.bin");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(4.0f, 4.0f);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//开启颜色混溶
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
}
//
void         Update(glk::GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_terrain->update(_deltaTime);
}

void         Draw(glk::GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//glEnable(GL_POLYGON_OFFSET_FILL);
	_user->_terrain->render();
}

void         ShutDown(glk::GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_terrain->release();
	_user->_terrain = nullptr;
}
///////////////////////////don not modify below function////////////////////////////////////
