/*
  *���п��ж����������,�ƹܶ�������ü���
  &2016-4-23
  */
#ifndef    __OBJECT_H__
#define   __OBJECT_H__
  class  Object
 {
private:
     int              _referenceCount;//���ü���,��ʼΪ1,Ϊ0��ʱ���ͷ�
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