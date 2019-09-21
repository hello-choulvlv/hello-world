/*
  *Ìì¿ÕºÐ
  *2017-12-10 17:32:35
  *@Author:xiaohuaxiong
 */
#include"GL/glew.h"
#include"Skybox.h"
#include "engine/GLContext.h"

#include"engine/event/EventManager.h"

Skybox::Skybox() :
	_skyboxProgram(nullptr)
	,_camera(0)
	,_cubeMap(nullptr)
	,_skyboxVertexId(0)
	, _skyboxVertexArrayId(0)
	,_touchListener(nullptr)
	,_keyListener(nullptr)
{

}

Skybox::~Skybox()
{
	_skyboxProgram->release();
	_camera->release();
	_cubeMap->release();
	glDeleteBuffers(1, &_skyboxVertexId);
	_skyboxVertexId = 0;
	//
	glk::EventManager::getInstance()->removeListener(_touchListener);
	_touchListener->release();
	glk::EventManager::getInstance()->removeListener(_keyListener);
	_keyListener->release();
}

void Skybox::initSkybox()
{
	_skyboxProgram = glk::GLProgram::createWithFile("shader/skybox/Skybox_VS.glsl", "shader/skybox/Skybox_FS.glsl");
	//cube map tga/water/sky/
	//const char *fileList[6] = {
	//	"tga/skybox/jajlands1_right.tga",//+X
	//	"tga/skybox/jajlands1_left.tga",//-X
	//	"tga/skybox/jajlands1_top.tga",//+Y
	//	"tga/skybox/jajlands1_bottom.tga",//-Y
	//	"tga/skybox/jajlands1_front.tga",//+Z
	//	"tga/skybox/jajlands1_back.tga",//-Z
	//};
	const char *fileList[6] = {
		"tga/skybox/ixpos.bmp",//+X
		"tga/skybox/ixneg.bmp",//-X
		"tga/skybox/iypos.bmp",//+Y
		"tga/skybox/iyneg.bmp",//-Y
		"tga/skybox/izpos.bmp",//+Z
		"tga/skybox/izneg.bmp",//-Z
	};
	_cubeMap = glk::GLCubeMap::createWithFiles(fileList);

	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	_camera = glk::Camera2::createWithPerspective(45.0f,winSize.width/winSize.height,0.5f,100);
	_camera->lookAt(glk::GLVector3(),glk::GLVector3(0,0,-1));
	_camera2 = glk::Camera::createCamera(glk::GLVector3(),glk::GLVector3(0,0,-1),glk::GLVector3(0,1,0));
	_camera2->setPerspective(45, winSize.width / winSize.height, 0.125, 500);
	//
	_touchListener = glk::TouchEventListener::createTouchListener(this, glk_touch_selector(Skybox::onTouchBegan),
		glk_move_selector(Skybox::onTouchMoved),glk_release_selector(Skybox::onTouchEnded));
	glk::EventManager::getInstance()->addTouchEventListener(_touchListener, 0);
	typedef glk::GLVector3 vec3;
	vec3 VertexData[] = {
	vec3(1.0f, -1.0f, -1.0f), vec3(1.0f, -1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, -1.0f), vec3(1.0f, -1.0f, -1.0f),
		vec3(-1.0f, -1.0f, 1.0f), vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, 1.0f, -1.0f), vec3(-1.0f, 1.0f, -1.0f), vec3(-1.0f, 1.0f, 1.0f), vec3(-1.0f, -1.0f, 1.0f),
		vec3(-1.0f, 1.0f, -1.0f), vec3(1.0f, 1.0f, -1.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), vec3(-1.0f, 1.0f, 1.0f), vec3(-1.0f, 1.0f, -1.0f),
		vec3(-1.0f, -1.0f, 1.0f), vec3(1.0f, -1.0f, 1.0f), vec3(1.0f, -1.0f, -1.0f), vec3(1.0f, -1.0f, -1.0f), vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, -1.0f, 1.0f),
		vec3(1.0f, -1.0f, 1.0f), vec3(-1.0f, -1.0f, 1.0f), vec3(-1.0f, 1.0f, 1.0f), vec3(-1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, -1.0f, 1.0f),
		vec3(-1.0f, -1.0f, -1.0f), vec3(1.0f, -1.0f, -1.0f), vec3(1.0f, 1.0f, -1.0f), vec3(1.0f, 1.0f, -1.0f), vec3(-1.0f, 1.0f, -1.0f), vec3(-1.0f, -1.0f, -1.0f)
	};
	glGenVertexArrays(1, &_skyboxVertexArrayId);
	glBindVertexArray(_skyboxVertexArrayId);

	glGenBuffers(1,&_skyboxVertexId);
	glBindBuffer(GL_ARRAY_BUFFER,_skyboxVertexId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glk::GLVector3),nullptr);

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Skybox::update(float deltaTime)
{

}

void Skybox::render()
{
	_skyboxProgram->perform();

	//glBindBuffer(GL_ARRAY_BUFFER,_skyboxVertexId);

	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(glk::GLVector3),nullptr);
	glBindVertexArray(_skyboxVertexArrayId);
	int mvpMatrixLoc = _skyboxProgram->getUniformLocation("g_MVPMatrix");
	int cameraPositionLoc = _skyboxProgram->getUniformLocation("g_CameraPosition");
	int skyboxMapLoc = _skyboxProgram->getUniformLocation("g_SkyboxMap");

	glUniformMatrix4fv(mvpMatrixLoc,1,GL_FALSE, _camera->getViewProjMatrix().pointer());
	if(cameraPositionLoc !=-1)
		glUniform3fv(cameraPositionLoc,1,&_camera->getEyePosition().x);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP,_cubeMap->getName());
	glUniform1i(skyboxMapLoc,0);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
}

bool Skybox::onTouchBegan(const glk::GLVector2 &touchPoint)
{
	_offsetVec = touchPoint;
	return true;
}

void Skybox::onTouchMoved(const glk::GLVector2 &touchPoint)
{
	float x = touchPoint.x - _offsetVec.x;
	float y = touchPoint.y - _offsetVec.y;
	_camera->rotate(-x*0.25f, y*0.25f);
	//_camera2->updateRotateMatrix(-y*0.25, x*0.25);
	_offsetVec = touchPoint;
}

void Skybox::onTouchEnded(const glk::GLVector2 &touchPoint)
{

}