/*
  *空间二维,三维圆以及球体定义
  *2019年5月23日
  *@author:xiaohuaxiong
 */
#ifndef _GT_CYCLE_SPHERE_H__
#define _GT_CYCLE_SPHERE_H__
#include "math/Vec2.h"
#include "math/Vec3.h"
#include "gt_common/geometry_types.h"
NS_GT_BEGIN
struct Plane;
struct Sphere
{
	cocos2d::Vec3  center;
	float                    radius;
};

struct Cycle
{
	cocos2d::Vec2  center;
	float                    radius;
};
/*
*三维的情况下,表示一个圆需要
*另一个维度,就是圆的穿过圆心的法向量
*/
struct Cycle3
{
	cocos2d::Vec3  center;
	cocos2d::Vec3  normal;
	float                    radius;
};
void sphere_create(Sphere &sphere,const cocos2d::Vec3 &center,float radius);
/*
*是否一个点在圆内
*/
bool check_point_insideof_cycle(const Cycle &cycle, const cocos2d::Vec2 &point);
/*
*三维几何,求视锥体的半平面约束集合
*此函数专用于阴影投射裁剪
*/
//void  shadow_frustum_half_planes(const cocos2d::Frustum	&fustum, const cocos2d::Vec3 &light_direction, cocos2d::Plane *planes, int &plane_length);
/*
*两点,三点确定一点圆
*/
void cycle_create(Cycle &cycle,const cocos2d::Vec2 &center,float radius);
void cycle_create(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b, Cycle &cycle);
void cycle_create(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b, const cocos2d::Vec2 &c, Cycle &cycle);
//创建圆,满足条件为,边界必须穿过目标点,此时所形成的圆不一定就是外接圆
//前两个顶点为边界点,后一个点可在圆内,也可以在边界上
void  create_mininum_cycle3(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b, const cocos2d::Vec2 &c, Cycle &cycle);
/*
*计算离散点集合的包围圆
*普通的算法
*/
void  compute_points_cycle_normal(const std::vector<cocos2d::Vec2> &points, Cycle  &cycle);

/*
*计算离散点集合的最小包围圆
*Welzl算法
*/
void  compute_points_minimum_cycle(const std::vector<cocos2d::Vec2> &points, Cycle  &cycle);
//汪卫算法,最小包围圆,其计算复杂度要小于上面的算法
void  compute_points_minimum_cycle_wangwei(const std::vector<cocos2d::Vec2> &points, Cycle  &cycle);
//空间三点的最小包围圆
void  create_mininum_cycle3(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, Cycle3 &cycle);
//创建球体
void  create_sphere2(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, Sphere &sphere);
void  create_sphere3(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, Sphere &sphere);
void  create_sphere4(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &d, Sphere &sphere);
/*
*创建最小包围球
*如果四个顶点都处于同一个平面内
*/
void  create_minimum_sphere4_plane(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &d, Sphere &sphere);
void  create_minimum_sphere4(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &d, Sphere &sphere);
bool check_point_insideof_sphere(const Sphere &sphere, const cocos2d::Vec3 &point);
bool check_point_onsurfaceof_sphere(const Sphere &sphere, const cocos2d::Vec3 &point);
/*
*求顶点集合的最小包围球
*一般算法
*/
void  compute_minimum_sphere_normal(const std::vector<cocos2d::Vec3> &points, Sphere  &sphere);
/*
*求顶点的最小包围球
*Welzl算法
*/
void  compute_vertex_minimum_sphere(const std::vector<cocos2d::Vec3> &points, Sphere  &sphere);
/*
  *球体到平面的距离
 */
float sphere_plane_minimum_distance(const Sphere &sphere,const Plane &plane);
/*
  *球体与AABB的相交测试
 */

NS_GT_END
#endif