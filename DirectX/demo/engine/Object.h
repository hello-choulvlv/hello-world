/*
  *对象类型
  *引用计数
  *2018/3/15
 */
#ifndef __OBJECT_H__
#define __OBJECT_H__
class Object
{
	int  _ref;
public:
	Object();	
	virtual ~Object();

	void   retain();
	void   release();

	int     getRefCount()const { return _ref; };
};
#endif