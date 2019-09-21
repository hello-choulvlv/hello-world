/*
  *四元数实现
  *2016-5-23 19:19:18
  *目前已经在Geometry.h/Geometry.cpp中实现了
  */
  //四元数
#ifndef __QUATERNION_H__
#define __QUATERNION_H__
#include "engine/Geometry.h"
   /*
	 *四元数的实现
	 *可以与角度+向量,旋转矩阵相互转换
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
	//使用角度+向量初始化四元数
	Quaternion(const float w, const float x, const float y, const float z);
	//
	Quaternion(const float    angle, const GLVector3  &);
	//使用旋转矩阵初始化向量,注意此必须为旋转矩阵,否则会崩溃
	Quaternion(const Matrix      &);
	//默认创建的是单位四元数
	Quaternion();
	//加载单位四元数
	void                   identity();
	//四元数乘法
	void                    multiply(Quaternion   &);
	//单位化
	void                    normalize();
	//点乘
	float                   dot(const Quaternion &other)const;
	//旋转三维向量
	GLVector3         rotate(const GLVector3 &src)const;
	//求逆
	Quaternion        reverse()const;
	//求共轭四元数
	Quaternion		   conjugate();
	//导出到旋转矩阵
	Matrix               toRotateMatrix();
	//另一种导出旋转矩阵的方式,在实践中具体使用哪一个,取决于使用者的爱好
	void                   toRotateMatrix(Matrix &rotateMatrix)const;
	//乘法运算符重载
	Quaternion		operator*(const Quaternion	&)const;
	GLVector3      operator*(const GLVector3 &)const;
	//在两个四元数之间进行线性插值
	static    Quaternion	   lerp(const Quaternion  &p, const Quaternion	&q, const float      lamda);
	//在两个四元数之间进行球面线性插值
	static    Quaternion     slerp(const Quaternion	&p, const Quaternion		&q, const float     lamda);
};
__NS_GLK_END
#endif