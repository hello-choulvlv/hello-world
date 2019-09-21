/*
  *应用程序状态机
  *2018年3月1日
  *@Author:xiaohuaxiong
 */
#include "AppEntry.h"
#include "DirectEngine.h"
//#include "PeaksAndValleys.h"
#include "WaterMesh.h"
AppEntry::AppEntry():
	_appId(0)
{

}

AppEntry::~AppEntry()
{

}

void AppEntry::onCreateComplete()
{

	DirectEngine *engine = DirectEngine::getInstance();

	WaterMesh *mesh = new WaterMesh();
	mesh->init(129, 129, 1.0f, 0.03f, 3.25f,0.4f);

	engine->addScene(mesh);
	mesh->release();
}

void AppEntry::onEnterBackground()
{

}

void AppEntry::onEnterForeground()
{

}

void AppEntry::onDestroy()
{
	DirectEngine *engine = DirectEngine::getInstance();
	engine->destroy();
}