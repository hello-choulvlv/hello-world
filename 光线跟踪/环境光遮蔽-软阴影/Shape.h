/*
  *�ı���,����ʵ��,Ϊ���߸����㷨������֮����ڵ��ṩ�����㷨
  *@2018��5��18��
  *@author:xiaohuaxiong
 */
#ifndef __SHAPE_H__
#define __SHAPE_H__
#include "engine/Geometry.h"
#include "Texture.h"
enum  ShapeType
{
	ShapeType_None = 0,//��Ч�ļ�����
	ShapeType_Quad = 1,//�ı���
	ShapeType_Sphere = 2,//����
};
class  Shape
{
	friend class RayTracer;
	ShapeType   _shapeType;
	Texture        *_texture;
	float_3           _color;
public:
	Shape(ShapeType type,const float_3 &color,Texture *texture) :_shapeType(type),_color(color),_texture(texture) {};
	virtual ~Shape() {};
	ShapeType  getType()const { return _shapeType; };
	void     setTexture(Texture *texture) { _texture = texture; };
	Texture *getTexture()const { return _texture; };
	virtual float_3    getPosition()=0;
	//virtual float_3    getPosition(float s,float t)=0;
	//�������ʵ�ֵĺ���
	virtual bool    intersectTest(const float_3 &point, const float_3 &ray, float &maxDistance, float_3 &intersectPoint)=0;
	virtual bool    intersectTest(const float_3 &point,const float_3 &ray,float maxDistance, float &testDistance)=0;
};
class RayTracer;
//�ı���
class Quad:public Shape
{
	friend class RayTracer;
	//�ı��ε��ĸ�����,�Լ�����
	float_3    _a, _b, _c, _d,_m;
	float_3    _tangent, _binormal;
	//��
	float_3    _ab, _ad;
	//����
	float_3   _normal;
	//�����ĸ�����_normal����ĵķ���
	float_3   _normala, _normalb, _normalc, _normald;
	//������
	float        _cn, _ca, _cb, _cc, _cd;
public:
	Quad(const float_3 &a,const float_3 &b,const float_3 &c,const float_3 &d,const float_3 &color,Texture *texture);
	//���������뼸�����Ƿ��ཻ,����ཻ,���������������ʼ��ľ���,�Լ�����
	virtual bool intersectTest(const float_3 &point, const float_3 &ray, float &maxDistance, float_3 &intersectPoint);
	virtual bool intersectTest(const float_3 &point, const float_3 &ray, float maxDistance, float &testDistance);
	virtual float_3 getPosition();
	//��Point�Ƿ�λ��Quadƽ����,��Ҫ�����ǰ������,�˺������ܵ���ʹ��
	bool     isInside(const float_3 &point);
};
//����
class Sphere :public Shape
{
	friend class RayTracer;
	//������ĵ�
	float_3    _center;
	float         _radius;
	float         _reflectRate;//������
	float         _refractRate;//������
	//����ϵ��
	float         _refractCoef;
public:
	Sphere(const float_3 &center,float radius,const float_3 &color,Texture *texture, float reflectRate, float refractRate, float refractCoef);
	virtual bool  intersectTest(const float_3 &point, const float_3 &ray, float maxDistance,float &testDistance);
	virtual bool  intersectTest(const float_3 &point, const float_3 &ray, float &maxDistance, float_3 &intersectPoint);
	virtual float_3 getPosition();
};
#endif