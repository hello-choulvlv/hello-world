/*
  *相交测试
  *2019/6/16
  *Author:xiaohuaxiong
 */
#ifndef _GT_INTERSECT_TEST_H__
#define _GT_INTERSECT_TEST_H__
#include "math/Vec2.h"
#include "math/Vec3.h"
#include "matrix/matrix.h"
NS_GT_BEGIN
struct Plane;
struct Sphere;
struct Triangle;
struct Segment;
/*
  *AABB碰撞检测
 */
struct AABB
{
	cocos2d::Vec3  bb_min,bb_max;
};
/*
  *OBB碰撞测试
 */
struct  OBB 
{
	cocos2d::Vec3 center;//中心
	cocos2d::Vec3 extent;//OBB半径
	cocos2d::Vec3 xaxis, yaxis, zaxis;//三个坐标轴
};
//创建AABB
void  aabb_create(AABB &aabb,const cocos2d::Vec3 &min_bb,const cocos2d::Vec3 &max_bb);
/*
  *创建OBB
   *或者直接给出,或者从一个AABB初始化
   *或者给出一个AABB,然后外加一个选择转矩阵
   *或者给出AABB,外加上两个计算旋转矩阵的参数
 */
void obb_create(OBB  &obb,const AABB &aabb);
void obb_create(OBB  &obb, const AABB &aabb, const mat3x3 &mat);
void obb_create(OBB  &obb, const AABB &aabb, const cocos2d::Vec3 &rotate_axis,float angle);
void obb_create(OBB &obb,const cocos2d::Vec3 &min_bb,const cocos2d::Vec3 &max_bb);
void obb_create(OBB  &obb, const cocos2d::Vec3 &min_bb, const cocos2d::Vec3 &max_bb, const mat3x3 &mat);
void obb_create(OBB  &obb, const cocos2d::Vec3 &min_bb, const cocos2d::Vec3 &max_bb, const cocos2d::Vec3 &rotate_axis, float angle);
/*
  *计算OBB的8个顶点
  *顶点的顺序从前到后
 */
void  obb_create_obb_vertex8(const OBB &obb,cocos2d::Vec3  *vertex);
/*
  *AABB相交测试
 */
bool aabb_aabb_intersect_test(const AABB &a,const AABB &b);
/*
  *OBB碰撞测试,分离轴测试,算法的实现依据 <碰撞检测算法技术>第3章
  *该算法的实现比较复杂,并且对文献中给出的算法进行了简化与优化
 */
bool obb_obb_intersect_test(const OBB &a,const OBB &b);
/*
  *点到AABB的最近距离
 */
float aabb_point_minimum_distance(const AABB &aabb,const cocos2d::Vec3 &point,cocos2d::Vec3 &intersect_point);
/*
  *点到OBB的最近距离
 */
float obb_point_minimum_distance(const OBB &obb, const cocos2d::Vec3 &point,cocos2d::Vec3 &intersect_point);
/*
  *AABB与平面的相交测试
  *其核心思想为分离轴测试
  *相对而言,aabb测试算法是OBB算法的简化版
 */
bool aabb_plane_intersect_test(const AABB &aabb,const Plane &plane);
/*
  *OBB与平面的相交测试
  *OBB算法相对复杂,但算法思想仍为分离轴测试
 */
bool obb_plane_intersect_test(const OBB &obb,const Plane &plane);
/*
  *AABB与球体的相交测试
 */
bool aabb_sphere_intersect_test(const AABB &aabb,const Sphere &sphere);
/*
  *OBB与球体的相交测试
 */
bool obb_sphere_intersect_test(const OBB &obb,const Sphere &sphere);
/*
  *AABB与三角形的相交测试
 */
bool aabb_triangle_intersect_test(const AABB &aabb,const Triangle &triangle);
/*
  *OBB与三角形的相交测试
 */
bool obb_triangle_intersect_test(const OBB &obb,const Triangle &triangle);
/*
  *AABB与线段之间的相交测试
  *分离轴测试
 */
bool aabb_segment_intersect_test(const AABB &aabb,const Segment &segment);
/*
  *OBB与线段之间的相交测试
  *分离轴测试
 */
bool obb_segment_intersect_test(const OBB &obb,const Segment &segment);
NS_GT_END
#endif