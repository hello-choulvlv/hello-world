/*
  *所有库中对象的祖先类,掌管对象的引用计数
  &2016-4-23
  */
#ifndef    __OBJECT_H__
#define   __OBJECT_H__
  class  Object
 {
private:
     int              _referenceCount;//引用计数,初始为1,为0的时候释放
private:
    Object(Object &);   
public:
	Object();
	virtual           ~Object();
	void                retain();
	void                release();
	int                   getReferenceCount()const;
  };
#endif