/*
  *Object
 */
#include<Object.h>
#include<assert.h>
_CLK_NS_BEGIN_

Object::Object()
{
	_referCount = 1;
}

Object::~Object()
{
	assert( ! _referCount);
}

void Object::retain()
{
	++_referCount;
}

void Object::release()
{
	--_referCount;
	if (!_referCount)
		delete this;
}

int Object::getReferCount()const
{
	return _referCount;
}

_CLK_NS_END_