/*
  *所有库中对象的祖先类,掌管对象的引用计数
  &2016-4-23
  */
#ifndef    __GLOBJECT_H__
#define   __GLOBJECT_H__
  class      GLObject
 {
private:
            int              _referenceCount;//引用计数,初始为1,为0的时候释放
private:
            GLObject(GLObject &);   
public:
             GLObject();
			 virtual           ~GLObject();
             void                retain();
             void                release();
             int                   getReferenceCount();
  };
#endif