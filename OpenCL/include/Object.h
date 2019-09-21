/*
  *所有引擎中类的父类
  *@date:2017-5-3
  *@Version:1.0
  */
#ifndef __OBJECT_H__
#define __OBJECT_H__
#include<clMicros.h>

_CLK_NS_BEGIN_

class Object
{
private:
	int     _referCount;
private:
	Object(Object &);
public:
	Object();
	virtual ~Object();
	void     retain();
	void     release();
	int       getReferCount()const;
};

_CLK_NS_END_

#endif