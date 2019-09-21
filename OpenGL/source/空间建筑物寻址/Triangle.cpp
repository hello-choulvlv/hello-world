/*
  *ͨ��������ʵ��
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
	//��������
	_vertexA = vertexA;
	_vertexB = vertexB;
	_vertexC = vertexC;
	//������,����˳��
	_abVector = vertexB - vertexA;
	_bcVector = vertexC - vertexB;
	_caVector = vertexA - vertexC;
	//
	_edgeAB = _abVector.length();
	_edgeBC = _bcVector.length();
	_edgeCA = _caVector.length();
	//��λ������������
	_abVector /= _edgeAB;
	_bcVector /= _edgeBC;
	_caVector /= _edgeCA;
	//�����ε�ƽ�淽��
	_nEquation.init(_caVector.cross(_abVector).normalize(),vertexA,false);
	const GLVector3 upVector(0, 1, 0);
	const GLVector3 zeroVector;
	const GLVector3 &normal = _nEquation._normal;
	_nXZ = normal.y>-1 && normal.y<1?GLVector3(normal.x,0,normal.z).normalize():zeroVector;
	//�ֱ𾭹������ε��������Լ�����,�������������ƽ�淽��
	_extraEquationA.init(normal.cross(_abVector),vertexA,false);
	_extraEquationB.init(normal.cross(_bcVector),vertexB,false);
	_extraEquationC.init(normal.cross(_caVector),vertexC,false);
	//�ֱ�ֱ�������ε�������,����+Y�ᱣ��ͬ��ķ���,�Լ��������ε����������γɵ�ƽ�淽��
	GLVector3  ynormal1 = _abVector.y>-1 && _abVector.y<1?(upVector - _abVector*_abVector.y).normalize():zeroVector;
	_yABEquation.init(ynormal1,vertexA,false);
	//
	GLVector3 ynormal2 = _bcVector.y>-1 && _bcVector.y<1?(upVector-_bcVector*_bcVector.y).normalize():zeroVector;
	_yBCEquation.init(ynormal2,vertexB,false);
	//
	GLVector3 ynormal3 = _caVector.y>-1 &&_caVector.y<1?(upVector-_caVector*_caVector.y).normalize():zeroVector;
	_yCAEquation.init(ynormal3,vertexC,false);
	// 
	//�������ƽ��ķ������������ε������γɵ�ƽ���ഹֱ�ķ�����
	_zABEquation.init(_abVector.cross(ynormal1),vertexA,false);
	_zBCEquation.init(_bcVector.cross(ynormal2),vertexB,false);
	_zCAEquation.init(_caVector.cross(ynormal3),vertexC,false);
}

bool Triangle::isInside(const glk::GLVector3 &point)const
{
	//����ͬʱ��������������,����Ϊ�ú���ֻ�������֮��ʹ��,�������Ĵ�����Ҫ�ܶ����ڵ�����,
	//�������ǻῴ��,���������ʹ������������ж�һ�������Ƿ���������������ε����������ڲ���
	return _extraEquationA.distance(point) >= 0 && _extraEquationB.distance(point) >= 0 && _extraEquationC.distance(point)>=0;
}

bool Triangle::rayIntersectTriangle(const glk::Ray &ray, float &minDistance, glk::GLVector3 &intersectPoint)
{
	float ndotRay = -_nEquation.getNormal().dot(ray.getDirecction());
	if (ndotRay > 0)
	{
		//�����ߵ���㵽����ƽ��ľ���
		float rayToTriangleDistance = _nEquation.distance(ray.getStartPoint())/ndotRay;
		if (rayToTriangleDistance > 0 && rayToTriangleDistance < minDistance)
		{
			//�жϵ��Ƿ���������ƽ��֮��
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
	//����Ƿ񳬹���ĳһ�������εı���
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
	//����Ĵ���һ�������ڴ��������ļ��
	Plane			*yPlanes			= &_yABEquation;
	Plane           *vPlane			= &_zABEquation;
	GLVector3  *edges				= &_abVector;
	GLVector3  *vertex			=	&_vertexA;
	float             *edgeLength  = &_edgeAB;
	for (int k = 0; k < 3; ++k)
	{
		//�����ֱ���������
		const GLVector3 &normal = yPlanes[k]._normal;
		if (normal.y > 0)
		{
			//���۾���ƽ��ľ���
			float distanceToPlaneY = yPlanes[k].distance(eyePosition) / normal.y;
			//�þ������С���۾���ϥ�ǵľ���,��Ϊ��Ҫ��ֹ��ɫ����ֱ��Խ���߶ȴ���ϥ�ǵĵĴ���
			if (distanceToPlaneY > 0 && distanceToPlaneY < eyeKneeDistance)
			{
				//���۾�����ˮƽ����ֱ����ֱƽ��ľ���
				float   distanceToPlaneV = vPlane[k].distance(eyePosition);
				//����˾������0��С����̾���
				if (distanceToPlaneV > 0 && distanceToPlaneV < minDistance)
				{
					//���۾�������ֱ�������ܴﵽ��ƽ���ϵĵ������
					GLVector3 eyeOnPlane(eyePosition.x,eyePosition.y-distanceToPlaneY,eyePosition.z);
					//��ⶥ����ߵĹ�ϵ
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