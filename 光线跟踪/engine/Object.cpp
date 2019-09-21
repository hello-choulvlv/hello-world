/*
  *@aim:引用计数实现
  *&2016-4-23
  */
#include "Object.h"
#include<assert.h>
  Object::Object()
 {
	  _referenceCount = 1;
  }
  Object::~Object()
  {
       assert(!_referenceCount);
  }
//
 void   Object::retain()
{
	++_referenceCount;
 }
//ref count
  int  Object::getReferenceCount()const
 {
      return     _referenceCount;
  }
//
  void  Object::release()
 {
	  assert(_referenceCount > 0);
	  --_referenceCount;
	  if (!_referenceCount)
		  delete    this;
  }