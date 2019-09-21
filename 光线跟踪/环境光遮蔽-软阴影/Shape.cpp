/*
  *光线跟踪四边形/球几何体实现
  *2018年5月18日
  *@author:xiaohuaxiong
 */
#include "Shape.h"
#include<math.h>
Quad::Quad(const float_3 &a, const float_3 &b, const float_3 &c, const float_3 &d, const float_3 &color,Texture *texture):Shape(ShapeType_Quad,color,texture),
_a(a),
_b(b),
_c(c),
_d(d),
_m((a+b+c+d)/4.0f)
{
	_ab = b - a;
	_ad = d - a;

	_normal = _ab.cross(c-a).normalize();
	_normala = _normal.cross(b-a).normalize();
	_normalb = _normal.cross(c-b).normalize();
	_normalc = _normal.cross(d -c).normalize();
	_normald = _normal.cross(a -d).normalize();

	_tangent = (b-a).normalize();
	_binormal = _normal.cross(_tangent);

	_cn = -_normal.dot(a);
	_ca = -_normala.dot(a);
	_cb = -_normalb.dot(b);
	_cc = -_normalc.dot(c);
	_cd = -_normald.dot(d);
}

float_3 Quad::getPosition()
{
	return _m;
}

bool  Quad::intersectTest(const float_3 &point, const float_3 &ray, float &maxDistance, float_3 &intersectPoint)
{
	float nDotL = -_normal.dot(ray);
	if (nDotL > 0)
	{
		float distance = (_normal.dot(point) + _cn)/nDotL;
		if (distance > 0 && distance < maxDistance)
		{
			float_3  targetPoint = point + ray * distance;
			if (isInside(targetPoint))
			{
				maxDistance = distance;
				intersectPoint = targetPoint;
				return true;
			}
		}
	}
	return false;
}

bool Quad::intersectTest(const float_3 &point, const float_3 &ray, float maxDistance, float &testDistance)
{
	float nDotL = -_normal.dot(ray);
	if (nDotL > 0)
	{
		float distance = (_normal.dot(point) + _cn) / nDotL;
		if (distance > 0 && distance < maxDistance)
		{
			float_3  targetPoint = point + ray * distance;
			if (isInside(targetPoint))
			{
				testDistance = distance;
				return true;
			}
		}
	}
	return false;
}

bool Quad::isInside(const float_3 &point)
{
	if (_normala.dot(point) + _ca < 0)
		return false;
	if (_normalb.dot(point) + _cb < 0)
		return false;
	if (_normalc.dot(point) + _cc < 0)
		return false;
	if (_normald.dot(point) + _cd < 0)
		return false;
	return true;
}
////////////////////////////////Sphere//////////////////////////////////////////
Sphere::Sphere(const float_3 &center, float radius, const float_3 &color,Texture *texture,float reflectRate,float refractRate,float refractCoef):Shape(ShapeType_Sphere,color,texture),
_center(center),
_radius(radius),
_reflectRate(reflectRate),
_refractRate(refractRate),
_refractCoef(refractCoef)
{
}

float_3 Sphere::getPosition()
{
	return _center;
}

bool Sphere::intersectTest(const float_3 &point, const float_3 &ray, float &maxDistance, float_3 &intersectPoint)
{
	float_3    direction = _center - point;
	float         nDotL = direction.dot(ray);
	//必须保证视线与几何体必须在同一侧
	if (nDotL > 0)
	{
		float distance2 = direction.x*direction.x+direction.y*direction.y+direction.z*direction.z;
		float shortDistance2 = distance2  - nDotL*nDotL;
		float r2 = _radius * _radius;
		//判断射线是否与球体相交
		if (shortDistance2 <= r2)
		{
			float distance = nDotL - sqrtf(_radius*_radius - shortDistance2);
			//当位于球体的内部时,distance为负数
			if (distance>=0 && distance < maxDistance)
			{
				intersectPoint =  point + ray * distance;
				maxDistance = distance;
				return true;
			}
		}
	}
	return false;
}

bool Sphere::intersectTest(const float_3 &point, const float_3 &ray, float maxDistance, float &testDistance)
{
	float_3    direction = _center - point;
	float         nDotL = direction.dot(ray);
	//必须保证视线与几何体必须在同一侧
	if (nDotL > 0)
	{
		float distance2 = direction.x*direction.x + direction.y*direction.y + direction.z*direction.z;
		float shortDistance2 = distance2 - nDotL*nDotL;
		float r2 = _radius * _radius;
		//判断射线是否与球体相交
		float distance = nDotL - sqrtf(_radius*_radius - shortDistance2);
		if (shortDistance2 <= r2 && distance < maxDistance)
		{
			testDistance = distance;
			return true;
		}
	}
	return false;
}