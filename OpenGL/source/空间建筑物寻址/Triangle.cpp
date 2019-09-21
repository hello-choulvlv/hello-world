/*
  *通用三角形实现
  *@date:2017/11/28
  *@Author:xiaohuaxiong
 */
#include "Triangle.h"
__US_GLK__

Triangle::Triangle():
	_edgeAB(0)
	,_edgeBC(0)
	,_edgeCA(0)
{
}

Triangle::Triangle(const glk::GLVector3 &vertexA, const glk::GLVector3 &vertexB, const glk::GLVector3 &vertexC)
{
	init(vertexA,vertexB,vertexC);
}

void Triangle::init(const glk::GLVector3 &vertexA, const glk::GLVector3 &vertexB, const glk::GLVector3 &vertexC)
{
	//三个顶点
	_vertexA = vertexA;
	_vertexB = vertexB;
	_vertexC = vertexC;
	//三条边,按照顺序
	_abVector = vertexB - vertexA;
	_bcVector = vertexC - vertexB;
	_caVector = vertexA - vertexC;
	//
	_edgeAB = _abVector.length();
	_edgeBC = _bcVector.length();
	_edgeCA = _caVector.length();
	//单位化三条边向量
	_abVector /= _edgeAB;
	_bcVector /= _edgeBC;
	_caVector /= _edgeCA;
	//三角形的平面方程
	_nEquation.init(_caVector.cross(_abVector).normalize(),vertexA,false);
	const GLVector3 upVector(0, 1, 0);
	const GLVector3 zeroVector;
	const GLVector3 &normal = _nEquation._normal;
	_nXZ = normal.y>-1 && normal.y<1?GLVector3(normal.x,0,normal.z).normalize():zeroVector;
	//分别经过三角形的三条边以及法线,三个顶点的内向平面方程
	_extraEquationA.init(normal.cross(_abVector),vertexA,false);
	_extraEquationB.init(normal.cross(_bcVector),vertexB,false);
	_extraEquationC.init(normal.cross(_caVector),vertexC,false);
	//分别垂直于三角形的三条边,且与+Y轴保持同向的法线,以及与三角形的三个顶点形成的平面方程
	GLVector3  ynormal1 = _abVector.y>-1 && _abVector.y<1?(upVector - _abVector*_abVector.y).normalize():zeroVector;
	_yABEquation.init(ynormal1,vertexA,false);
	//
	GLVector3 ynormal2 = _bcVector.y>-1 && _bcVector.y<1?(upVector-_bcVector*_bcVector.y).normalize():zeroVector;
	_yBCEquation.init(ynormal2,vertexB,false);
	//
	GLVector3 ynormal3 = _caVector.y>-1 &&_caVector.y<1?(upVector-_caVector*_caVector.y).normalize():zeroVector;
	_yCAEquation.init(ynormal3,vertexC,false);
	// 
	//与上面的平面的法向量与三角形的三条形成的平面相垂直的法向量
	_zABEquation.init(_abVector.cross(ynormal1),vertexA,false);
	_zBCEquation.init(_bcVector.cross(ynormal2),vertexB,false);
	_zCAEquation.init(_caVector.cross(ynormal3),vertexC,false);
}

bool Triangle::isInside(const glk::GLVector3 &point)const
{
	//必须同时满足这三个条件,又因为该函数只在这个类之内使用,并且他的触发需要很多外在的条件,
	//后面我们会看到,我们是如何使用这个函数来判断一个顶点是否坐落在这个三角形的有限区域内部的
	return _extraEquationA.distance(point) >= 0 && _extraEquationB.distance(point) >= 0 && _extraEquationC.distance(point)>=0;
}

bool Triangle::rayIntersectTriangle(const glk::Ray &ray, float &minDistance, glk::GLVector3 &intersectPoint)
{
	float ndotRay = -_nEquation.getNormal().dot(ray.getDirecction());
	if (ndotRay > 0)
	{
		//求射线的起点到三角平面的距离
		float rayToTriangleDistance = _nEquation.distance(ray.getStartPoint())/ndotRay;
		if (rayToTriangleDistance > 0 && rayToTriangleDistance < minDistance)
		{
			//判断点是否在三角形平面之内
			GLVector3 pointInPlane = ray.getStartPoint() + ray.getDirecction() * rayToTriangleDistance;
			if (isInside(pointInPlane))
			{
				minDistance = rayToTriangleDistance;
				intersectPoint = pointInPlane;
				return true;
			}
		}
	}
	return false;
}

bool Triangle::rayIntersectUnder(const glk::GLVector3 &eyePosition, float eyeKneeDistance,float &minDistance, float &height)
{
	float ndotRay = _nEquation.getNormal().y;
	if (ndotRay > 0)
	{
		float eyeToPlaneDistance = _nEquation.distance(eyePosition)/ndotRay;
		if (eyeToPlaneDistance > eyeKneeDistance && eyeToPlaneDistance < minDistance)
		{
			GLVector3  pointOnPlane(eyePosition.x,eyePosition.y - eyeToPlaneDistance,eyePosition.z);
			if (isInside(pointOnPlane))
			{
				minDistance = eyeToPlaneDistance;
				height = pointOnPlane.y;
				return true;
			}
		}
	}
	return false;
}

bool Triangle::rayIntersectAbove(const glk::GLVector3 &eyePosition, float &minDistance, float &height)
{
	float ndotRay = -_nEquation.getNormal().y;
	if (ndotRay > 0)
	{
		float eyeToPlaneDistance = _nEquation.distance(eyePosition)/ndotRay;
		if (eyeToPlaneDistance > 0 && eyeToPlaneDistance < minDistance)
		{
			GLVector3 pointOnPlane(eyePosition.x,eyePosition.y+eyeToPlaneDistance,eyePosition.z);
			if (isInside(pointOnPlane))
			{
				minDistance = eyeToPlaneDistance;
				height = pointOnPlane.y;
				return true;
			}
		}
	}
	return false;
}

bool Triangle::rayDistanceTest(const glk::GLVector3 &eyePosition, float eyeKneeDistance, float closetDistance, float &minDistance, glk::GLVector3 &compensation)
{
	bool   distanceTest = false;
	//检测是否超过了某一个三角形的背面
	float  ndotXZ = _nEquation._normal.dot(_nXZ);
	if (ndotXZ > 0)
	{
		float eyeToPlane = _nEquation.distance(eyePosition);
		if (eyeToPlane > 0 && eyeToPlane < minDistance)
		{
			GLVector3 pointInPlane = eyePosition + _nEquation.getNormal() *eyeToPlane;
			if (isInside(pointInPlane))
			{
				distanceTest = true;
				minDistance = eyeToPlane;
				compensation = _nXZ *(closetDistance -  eyeToPlane)/ndotXZ;
			}
		}
	}
	//下面的代码一般用来在窗户附近的检测
	Plane			*yPlanes			= &_yABEquation;
	Plane           *vPlane			= &_zABEquation;
	GLVector3  *edges				= &_abVector;
	GLVector3  *vertex			=	&_vertexA;
	float             *edgeLength  = &_edgeAB;
	for (int k = 0; k < 3; ++k)
	{
		//检测竖直方向的向量
		const GLVector3 &normal = yPlanes[k]._normal;
		if (normal.y > 0)
		{
			//求眼睛到平面的距离
			float distanceToPlaneY = yPlanes[k].distance(eyePosition) / normal.y;
			//该距离必须小于眼睛到膝盖的距离,因为需要防止角色可以直接越过高度大于膝盖的的窗户
			if (distanceToPlaneY > 0 && distanceToPlaneY < eyeKneeDistance)
			{
				//求眼睛沿着水平方向直达竖直平面的距离
				float   distanceToPlaneV = vPlane[k].distance(eyePosition);
				//如果此距离大于0且小于最短距离
				if (distanceToPlaneV > 0 && distanceToPlaneV < minDistance)
				{
					//求眼睛沿着竖直方向所能达到的平面上的点的坐标
					GLVector3 eyeOnPlane(eyePosition.x,eyePosition.y-distanceToPlaneY,eyePosition.z);
					//检测顶点与边的关系
					float  ndotEdge = edges[k].dot(eyeOnPlane - vertex[k]);
					if (ndotEdge > 0 && ndotEdge < edgeLength[k])
					{
						distanceTest= true;
						minDistance = distanceToPlaneV;
						compensation = vPlane[k].getNormal()*(closetDistance - distanceToPlaneV);
					}
				}
			}
		}
	}
	return distanceTest;
}