/*
  *Scene.cpp
  *2018/3/16
  *@author:xiaohuaxiong
 */
#include "Scene.h"
#include <d3dx10.h>
#include "DirectEngine.h"

Scene::Scene()
{
	_device = DirectEngine::getInstance()->getDevice();
}