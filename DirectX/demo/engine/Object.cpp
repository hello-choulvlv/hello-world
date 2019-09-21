/*
  *引用计数实现
  *@author:xiaohuaxiong
  *2018/3/15
 */
#include "Object.h"
#include<assert.h>
Object::Object() :
	_ref(1)
{
}

Object::~Object()
{
}

void Object::retain()
{
	++_ref;
}

void Object::release()
{
	--_ref;
	assert(_ref>=0);
	if (!_ref)
		delete this;
}