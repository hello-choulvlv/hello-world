/*
  *���п��ж����������,�ƹܶ�������ü���
  &2016-4-23
  */
#ifndef    __GLOBJECT_H__
#define   __GLOBJECT_H__
  class      GLObject
 {
private:
            int              _referenceCount;//���ü���,��ʼΪ1,Ϊ0��ʱ���ͷ�
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