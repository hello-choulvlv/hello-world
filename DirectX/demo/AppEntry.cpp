/*
  *应用程序状态机
  *2018年3月1日
  *@Author:xiaohuaxiong
 */
#include "AppEntry.h"
#include "engine/DirectEngine.h"
//#include "RenderTarget.h"
#include "Particles.h"
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

	//RenderTarget *scene = new RenderTarget();
	//auto &winSize = engine->getWinSize();
	//scene->init(winSize.width,winSize.height);

	Particles * scene = new Particles();
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