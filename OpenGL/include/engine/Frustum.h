/*
  *平截头体实现,主要用在模型可视性判断方面
  *@date:2017-5-20
  *@Author:xiaohuaxiong
*/
#ifndef __FRUSTUM_H__
#define __FRUSTUM_H__
#include"engine/Geometry.h"
__NS_GLK_BEGIN
class Frustum
{
	//六个平面方程
	Plane     _planes[6];
public:
	//使用视图投影矩阵来生成六个平面
	Frustum(const Matrix &viewProjMatrix);
	Frustum();
	//使用视图投影矩阵生成六个平面方程
	void       init(const Matrix &viewProjMatrix);
	/*
	  *使用屏幕控制键NDC坐标的一部分建立视椎体
	  *lndcX,rndcX means left and right of ndc coordinate boundary
	  *bndcY,tndcY means bottom and top ndc coordinate boundary
	 */
	void       init(float lndcX,float rndcX, float bndcY,float tndcY,const Matrix &viewProjMatrixInverse);
	//判断模型的包围盒是否在平截头体中可见
	bool       isOutOfFrustum(const AABB &box)const;
	//判断定点是否在视锥体内可见
	bool      isOutOfFrustum(const GLVector3 &position)const;
};
__NS_GLK_END
#endif