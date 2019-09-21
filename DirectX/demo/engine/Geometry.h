/*
  *为了便于D3d程序设计,需要一套易于使用的空间几何类
  *D3D自身所附带的矩阵操作函数使用太繁琐
  *并且命名冗杂,在本文件中,我们将实现一套完整的空间几何操作函数
  *全局将使用左手坐标系
  *2018年3月6日 
  *@author:xiaohuaxiong
 */
#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__
#include <d3dx10.h>
/*
  *描述资源的尺寸
 */
struct  Size
{
	float width, height;
	Size();
	Size(float w, float h);
};
//Vec2
struct Vec2
{
	float x, y;
	Vec2();
	Vec2(float );
	Vec2(float x, float y);
};
//Vec3
struct Vec3
{
	float x, y, z;
	Vec3();
	Vec3(float );
	Vec3(float ax, float ay, float az);
};
//Vec4
struct Vec4
{
	float x, y, z, w;
	Vec4();
	Vec4(float);
	Vec4(float x,float y,float z,float w);
};
/////////////////////////////////矩阵算法////////////////////////
/*
  *创建镜像矩阵
 */
void    CreateReflectMatrix(D3DXMATRIX  &reflect,const D3DXPLANE &plane);
#endif