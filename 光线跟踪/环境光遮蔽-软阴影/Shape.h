/*
  *四边形,球体实现,为光线跟踪算法中物体之间的遮挡提供基础算法
  *@2018年5月18日
  *@author:xiaohuaxiong
 */
#ifndef __SHAPE_H__
#define __SHAPE_H__
#include "engine/Geometry.h"
#include "Texture.h"
enum  ShapeType
{
	ShapeType_None = 0,//无效的几何体
	ShapeType_Quad = 1,//四边形
	ShapeType_Sphere = 2,//球体
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
	//子类必须实现的函数
	virtual bool    intersectTest(const float_3 &point, const float_3 &ray, float &maxDistance, float_3 &intersectPoint)=0;
	virtual bool    intersectTest(const float_3 &point,const float_3 &ray,float maxDistance, float &testDistance)=0;
};
class RayTracer;
//四边形
class Quad:public Shape
{
	friend class RayTracer;
	//四边形的四个顶点,以及中心
	float_3    _a, _b, _c, _d,_m;
	float_3    _tangent, _binormal;
	//边
	float_3    _ab, _ad;
	//法线
	float_3   _normal;
	//沿着四个边与_normal共面的的法线
	float_3   _normala, _normalb, _normalc, _normald;
	//常数项
	float        _cn, _ca, _cb, _cc, _cd;
public:
	Quad(const float_3 &a,const float_3 &b,const float_3 &c,const float_3 &d,const float_3 &color,Texture *texture);
	//计算射线与几何体是否相交,如果相交,求出交点与射线起始点的距离,以及交点
	virtual bool intersectTest(const float_3 &point, const float_3 &ray, float &maxDistance, float_3 &intersectPoint);
	virtual bool intersectTest(const float_3 &point, const float_3 &ray, float maxDistance, float &testDistance);
	virtual float_3 getPosition();
	//点Point是否位于Quad平面中,需要额外的前提条件,此函数不能单独使用
	bool     isInside(const float_3 &point);
};
//球体
class Sphere :public Shape
{
	friend class RayTracer;
	//球的中心点
	float_3    _center;
	float         _radius;
	float         _reflectRate;//反射率
	float         _refractRate;//折射率
	//折射系数
	float         _refractCoef;
public:
	Sphere(const float_3 &center,float radius,const float_3 &color,Texture *texture, float reflectRate, float refractRate, float refractCoef);
	virtual bool  intersectTest(const float_3 &point, const float_3 &ray, float maxDistance,float &testDistance);
	virtual bool  intersectTest(const float_3 &point, const float_3 &ray, float &maxDistance, float_3 &intersectPoint);
	virtual float_3 getPosition();
};
#endif