/*
  *��������,���еĹ��߸��������ʵ�ָ���
  *2018��4��26��
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