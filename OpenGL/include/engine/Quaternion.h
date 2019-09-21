/*
  *��Ԫ��ʵ��
  *2016-5-23 19:19:18
  *Ŀǰ�Ѿ���Geometry.h/Geometry.cpp��ʵ����
  */
  //��Ԫ��
#ifndef __QUATERNION_H__
#define __QUATERNION_H__
#include "engine/Geometry.h"
   /*
	 *��Ԫ����ʵ��
	 *������Ƕ�+����,��ת�����໥ת��
	 */
__NS_GLK_BEGIN
class        Quaternion
{
	friend   class   Matrix;
public:
	float        w;
	float        x;
	float        y;
	float        z;
public:
	//ʹ�ýǶ�+������ʼ����Ԫ��
	Quaternion(const float w, const float x, const float y, const float z);
	//
	Quaternion(const float    angle, const GLVector3  &);
	//ʹ����ת�����ʼ������,ע��˱���Ϊ��ת����,��������
	Quaternion(const Matrix      &);
	//Ĭ�ϴ������ǵ�λ��Ԫ��
	Quaternion();
	//���ص�λ��Ԫ��
	void                   identity();
	//��Ԫ���˷�
	void                    multiply(Quaternion   &);
	//��λ��
	void                    normalize();
	//���
	float                   dot(const Quaternion &other)const;
	//��ת��ά����
	GLVector3         rotate(const GLVector3 &src)const;
	//����
	Quaternion        reverse()const;
	//������Ԫ��
	Quaternion		   conjugate();
	//��������ת����
	Matrix               toRotateMatrix();
	//��һ�ֵ�����ת����ķ�ʽ,��ʵ���о���ʹ����һ��,ȡ����ʹ���ߵİ���
	void                   toRotateMatrix(Matrix &rotateMatrix)const;
	//�˷����������
	Quaternion		operator*(const Quaternion	&)const;
	GLVector3      operator*(const GLVector3 &)const;
	//��������Ԫ��֮��������Բ�ֵ
	static    Quaternion	   lerp(const Quaternion  &p, const Quaternion	&q, const float      lamda);
	//��������Ԫ��֮������������Բ�ֵ
	static    Quaternion     slerp(const Quaternion	&p, const Quaternion		&q, const float     lamda);
};
__NS_GLK_END
#endif