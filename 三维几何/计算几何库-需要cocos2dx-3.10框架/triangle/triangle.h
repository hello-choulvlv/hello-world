/*
  *3d三角形,四边形,矩形
  *2019年7月12日
  *@author:xiaohuaxiong
 */
#ifndef __GT_TRIANGLE_H__
#define __GT_TRIANGLE_H__
#include "gt_common/geometry_types.h"
#include "math/Vec3.h"
#include "math/Vec2.h"
NS_GT_BEGIN
struct Plane;
struct Sphere;
struct Line;
struct Segment;
/*
  *3d矩形
  *其有两种不同的定义方式
 */
struct Rectangle
{
	cocos2d::Vec3 center;
	cocos2d::Vec2 extent;
	cocos2d::Vec3 xaxis, yaxis;
};
/*
  *三个顶点定义的矩形
  *两边必须垂直
*/
struct Rectangle3v
{
	cocos2d::Vec3 a, b, c;
};
/*
  *三角形有两种不同的定义
 */
struct Triangle
{
	cocos2d::Vec3 a, b, c;
};
/*
  *四面体
 */
struct Tetrahedron
{
	cocos2d::Vec3 a, b, c, d;
};
/*
  *锥形
 */
struct Cone
{
	cocos2d::Vec3   top;//锥形的顶点
	cocos2d::Vec3   normal;//锥形的法线
	float                     h, r;//锥形的长度,以及底部的半径
};
/*
  *圆柱体
 */
struct Cylinder
{
	cocos2d::Vec3 bottom;//底面的中心点
	cocos2d::Vec3 direction;//圆柱体的方向向量,单位化
	float                   length, r;//母线的长度,与底面的半径
};
/*
  *创建3d矩形
 */
bool rectangle_create(Rectangle &rect,const cocos2d::Vec3 &min_vertex,const cocos2d::Vec3 &max_vertex,const cocos2d::Vec3 &axis,float angle);
void rectangle_create(Rectangle &rect,const cocos2d::Vec3 &center,const cocos2d::Vec3 &xaxis,const cocos2d::Vec3 &yaxis,const cocos2d::Vec2 &extent);
//获取顶点
void rectangle_get_vertex(const Rectangle &rect,cocos2d::Vec3 *vertex);
/*
  *创建另一种3d矩形
 */
void rectangle3v_create(Rectangle3v &rect, const cocos2d::Vec3 &corner,const cocos2d::Vec2 &extent ,const cocos2d::Vec3 &axis, float angle);
bool rectangle3v_create(Rectangle3v &rect,const cocos2d::Vec3 &corner,const cocos2d::Vec3 &b,const cocos2d::Vec3 &c);
bool rectangle3v_create(Rectangle3v &rect,const cocos2d::Vec3 &corner,const cocos2d::Vec3 &xaxis,const cocos2d::Vec3 &yaxis,const cocos2d::Vec2 &extent);
//获取顶点
void rectangle3v_get_vertex(const Rectangle3v &rect, cocos2d::Vec3 *vertex);
/*
  *求一个点到3d矩形的最近距离,以及最近点
 */
float rectangle_point_min_distance(const Rectangle &rect,const cocos2d::Vec3 &point,cocos2d::Vec3 &intersect_point);
float rectangle3v_point_min_distance(const Rectangle3v &rect,const cocos2d::Vec3 &point,cocos2d::Vec3 &intersect_point);
#pragma mark --------------------triangle------------------------------------------------------
void triangle_create(Triangle &triangle,const cocos2d::Vec3 &a,const cocos2d::Vec3 &b,const cocos2d::Vec3 &c);
void triangle_create(Triangle &triangle,const cocos2d::Vec3 *v);
/*
  *点距离三角形的最近距离,与最近点
  *与其他的几何图元相比,三角形的测试比较复杂,
  *其中会涉及到Voronoi域的判定
 */
float triangle_point_min_distance(const Triangle &triangle,const cocos2d::Vec3 &point, cocos2d::Vec3 &intersect_point);
/*
  *线段到三角平面的最近距离
 */
float triangle_segment_distance(const Triangle &triangle,const cocos2d::Vec3 *vert,cocos2d::Vec3 &triangle_point,cocos2d::Vec3 &segment_point);
/*
  *点到平面的距离
 */
float plane_point_distance(const cocos2d::Vec3 &p,const cocos2d::Vec3 &a,const cocos2d::Vec3 &b,const cocos2d::Vec3 &c);
/*
  *创建四面体
 */
void tetrahedron_create(Tetrahedron &tet, const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &d);
void tetrahedron_create(Tetrahedron &tet, const cocos2d::Vec3 *vertex);
/*
  *求点到四面体的最近距离
  *该算法有两种不同的形式
 */
float tetrahedron_point_min_distance(const Tetrahedron &tet, const cocos2d::Vec3 &point, cocos2d::Vec3 &intersect_point);
float tetrahedron_point_min_distance(const cocos2d::Vec3 *vert,const cocos2d::Vec3 &point,cocos2d::Vec3 &intersect_point);
/*
  *创建锥形
 */
void cone_create(Cone &cone,const cocos2d::Vec3 &top,const cocos2d::Vec3 &normal,float h,float r);
/*
  *锥形与平面的相交测试
 */
bool cone_plane_intersect_test(const Cone &cone,const Plane &plane);
/*
  *三角形与球体的相交测试
  */
bool triangle_sphere_intersect_test(const Triangle &triangle,const Sphere &sphere);
/*
  *三角形与三角形之间的相交测试
 */
bool triangle_triangle_intersect_test(const Triangle &t1,const Triangle &t2);
/*
  *三角形与直线之间的相交测试
 */
bool triangle_line_intersect_test(const Triangle &triangle,const Line &line);
/*
  *三角形与线段之间的相交测试
  *该算法将计算质心坐标
  *并以几何算法的形式给出测试结果
  *注意该函数并没有经过优化
 */
bool triangle_segment_intersect_test(const Triangle &triangle,const Segment &segment);
/*
  *圆柱体
 */
void cylinder_create(Cylinder &cylinder,const cocos2d::Vec3 &bottom,const cocos2d::Vec3 &direction,float length,float r);
void cylinder_create(Cylinder &cylinder,const cocos2d::Vec3 &bottom,const cocos2d::Vec3 &top,float r);
/*
  *圆柱体与直线之间的相交测试
 */
bool cylinder_line_intersect_test(const Cylinder &cylinder,const Line &line);
/*
  *圆柱体与线段之间的相交测试
  *其行为相对直线与圆柱体的相交测试而言要稍微复杂一些
 */
bool cylinder_segment_intersect_test(const Cylinder &cylinder,const Segment &segment);
NS_GT_END
#endif