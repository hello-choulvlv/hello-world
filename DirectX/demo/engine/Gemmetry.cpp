/*
  *关于空间几何的类集合
  *2018年3月15日
  *@author:xiaohuaxiong
 */
#include "Geometry.h"
////////////////////////////Size///////////////////////////
Size::Size() :
	width(0)
	,height(0)
{
}

Size::Size(float w, float h):
	width(w)
	,height(h)
{
}
///////////////////////////Vec2////////////////////////////
Vec2::Vec2() :
	x(0)
	,y(0)
{
}

Vec2::Vec2(float t) :
	x(t)
	,y(t)
{
}

Vec2::Vec2(float ax, float ay):
	x(ax)
	,y(ay)
{
}
//////////////////////////Vec3///////////////////////////
Vec3::Vec3() :
	x(0)
	,y(0)
	,z(0)
{
}

Vec3::Vec3(float t) :
	x(t)
	,y(t)
	,z(t)
{
}

Vec3::Vec3(float ax, float ay, float az) :
	x(ax)
	,y(ay)
	,z(az)
{
}
/////////////////////////////////Vec4/////////////////////
Vec4::Vec4():
x(0)
,y(0)
,z(0)
,w(0)
{
}

Vec4::Vec4(float t) :
	x(t)
	,y(t)
	,z(t)
	,w(t)
{
}

Vec4::Vec4(float ax, float ay, float az, float aw):
	x(ax)
	,y(ay)
	,z(az)
	,w(aw)
{
}
/////////////////////////////Matrix////////////////////////
void  CreateReflectMatrix(D3DXMATRIX &reflect, const D3DXPLANE &plane)
{
	reflect.m[0][0] = 1.0f - 2.0f * plane.a * plane.a;
	reflect.m[0][1] = -2.0f*plane.a * plane.b;
	reflect.m[0][2] = -2.0f*plane.a*plane.c;
	reflect.m[0][3] = 0;

	reflect.m[1][0] = -2.0f*plane.a*plane.b;
	reflect.m[1][1] = 1.0f - 2.0f*plane.b*plane.b;
	reflect.m[1][2] = -2.0f*plane.b*plane.c;
	reflect.m[1][3] = 0;

	reflect.m[2][0] = -2.0f*plane.a*plane.c;
	reflect.m[2][1] = -2.0f*plane.b*plane.c;
	reflect.m[2][2] = 1.0f - 2.0f*plane.c*plane.c;
	reflect.m[2][3] = 0.0f;

	reflect.m[3][0] = -2.0f * plane.a * plane.d;
	reflect.m[3][1] = -2.0f*plane.b * plane.d;
	reflect.m[3][2] = -2.0f *plane.c*plane.d;
	reflect.m[3][3] = 1.0f;
}
