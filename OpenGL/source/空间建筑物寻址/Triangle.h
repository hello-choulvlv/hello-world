/*
  *三角形测试,注意Triangle类比起常规的三角形实现要复杂很多
  *因为它是用来计算空间建筑物寻址用的
  *所包含的数据不仅有传统的三个顶点/法线,还有内外向量
  *@date:2017/11/28
  *@Author:xiaohuaxiong
 */
#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__
#include "engine/Geometry.h"
class Triangle
{
private:
	//三角形的三个顶点
	glk::GLVector3  _vertexA, _vertexB, _vertexC;
	//三角形的三条边形成的向量
	glk::GLVector3  _abVector,_bcVector,_caVector;
	//三角的三条边的长度
	float                      _edgeAB,_edgeBC,_edgeCA;
	//三角形的平面方程
	glk::Plane          _nEquation;
	glk::GLVector3  _nXZ;//三角形的平面方程在XOZ平面形成的投影向量,必须经过单位化
	//三角形的以三边以及法线为平面,分别经过三个顶点的三个平面方程
	glk::Plane          _extraEquationA,_extraEquationB,_extraEquationC;
	//分别垂直于三角形的三条边,且与+Y轴同向的方向向量,经过三角形的三个顶点形成的三个平面方程
	glk::Plane          _yABEquation,_yBCEquation,_yCAEquation;
	//法向量垂直于三角形的边与上面的方程中的法向量形成的平面，且经过三角形的每一个顶点形成的平面
	glk::Plane          _zABEquation,_zBCEquation,_zCAEquation;
private:
	/*
	  *测试一个顶点是否坐落在以三角形的法线为方向形成的三角棱柱内
	  *此函数用来快速计算顶点是否位于三角平面之内
	 */
	bool      isInside(const glk::GLVector3 &point)const;
public:
	Triangle();
	Triangle(const glk::GLVector3 &vertexA,const glk::GLVector3 &vertexB,const glk::GLVector3 &vertexC);
	void     init(const glk::GLVector3 &vertexA,const glk::GLVector3 &vertexB,const glk::GLVector3 &vertexC);
	  ///////////////*下面的函数为空间几何相交性测试*////////////////////////////
	/*
	  *空间直线测试,是否与三角平面相交
	  *这个函数是其他的很多函数的基础
	 */
	bool     rayIntersectTriangle(const glk::Ray &ray,float &minDistance,glk::GLVector3 &intersectPoint);
	/*
	  *当眼睛在平面正上方的时候检测眼睛沿着方向direction是否与三角平面相交
	  *eyeKneeDistance为眼睛到膝盖的距离
	*/
	bool    rayIntersectUnder(const glk::GLVector3 &eyePosition,float eyeKneeDistance,float &minDistance,float &height);
	/*
	  *当眼睛在三角平面的下方的时候,眼睛沿着+Y轴方向向上看,是否与三角平面相交,如果相交,求出最短距离
	 */
	bool   rayIntersectAbove(const glk::GLVector3 &eyePosition,float &minDistance,float &height);
	/*
	  *距离测试,用来测试眼睛的距离是否位于某个三角形的正上方
	  *eyePosition:眼睛的距离
	  *eyeKneeDistance:眼睛到膝盖的距离
	  *closetDistance:邻接距离,
	  *minDistance:眼睛所能达到的最短距离
	  *compensation:补偿偏移量
	 */
	bool   rayDistanceTest(const glk::GLVector3 &eyePosition,float eyeKneeDistance,float closetDistance,float &minDistance,glk::GLVector3 &compensation);
};
#endif