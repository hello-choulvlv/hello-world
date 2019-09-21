/*
  *应用程序状态机
  *2018年3月1日
  *@Author:xiaohuaxiong
 */
#include "AppEntry.h"
#include "engine/DirectEngine.h"
//#include "PeaksAndValleys.h"
//#include "WaterMesh.h"
//#include "Texture.h"
#include "Reflect.h"
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

	Reflect *scene = new Reflect();
	scene->init();

	engine->addScene(scene);
	scene->release();
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