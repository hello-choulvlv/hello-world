/*
  *@aim:引用计数实现
  *&2016-4-23
  */
#include <engine/Object.h>
#include<assert.h>
__NS_GLK_BEGIN
  Object::Object()
 {
           _referenceCount=1;
  }
  Object::~Object()
  {
           assert(!_referenceCount);
  }
//
 void          Object::retain()
{
             ++_referenceCount;
 }
//ref count
  int            Object::getReferenceCount()
 {
            return     _referenceCount;
  }
//
  void         Object::release()
 {
            assert(_referenceCount>0);
            --_referenceCount;
            if(!  _referenceCount)
                         delete    this;
  }

  __NS_GLK_END