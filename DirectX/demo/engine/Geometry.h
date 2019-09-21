/*
  *Ϊ�˱���D3d�������,��Ҫһ������ʹ�õĿռ伸����
  *D3D�����������ľ����������ʹ��̫����
  *������������,�ڱ��ļ���,���ǽ�ʵ��һ�������Ŀռ伸�β�������
  *ȫ�ֽ�ʹ����������ϵ
  *2018��3��6�� 
  *@author:xiaohuaxiong
 */
#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__
#include <d3dx10.h>
/*
  *������Դ�ĳߴ�
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
/////////////////////////////////�����㷨////////////////////////
/*
  *�����������
 */
void    CreateReflectMatrix(D3DXMATRIX  &reflect,const D3DXPLANE &plane);
#endif