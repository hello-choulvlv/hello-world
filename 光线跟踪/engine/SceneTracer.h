/*
  *场景跟踪,所有的光线跟踪类必须实现该类
  *2018年4月26日
  *@author:xiaohuaxiong
 */
#ifndef  __SCENE_TRACER_H__
#define __SCENE_TRACER_H__
#include "Object.h"
#include "Geometry.h"
class SceneTracer :public Object
{
	int    _refCount;
public:
	SceneTracer();
	//
	virtual ~SceneTracer();
	virtual   void        rayTrace(float x,float y,float_3 &color)=0;
};
#endif