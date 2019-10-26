/*
  *三维空间直线之间的关系
  *以及线段与线段之间的关系算法实现
  *2019年6月22日
  *@author:xiaoxiong
*/
#ifndef _GT_LINE_H__
#define _GT_LINE_H__
#include "gt_common/geometry_types.h"
#include "math/Vec2.h"
#include "math/Vec3.h"
#include<vector>
NS_GT_BEGIN
struct Sphere;
struct AABB;
struct OBB;
struct Triangle;
/*
  *直线
 */
struct  Line
{
	cocos2d::Vec3   start_point;//直线上的任意一点
	cocos2d::Vec3   direction;//直线的方向
};
struct Line2D
{
	cocos2d::Vec2   start_point;//直线上的任意一点
	cocos2d::Vec2   direction;//直线的方向
};
/*
  *线段
 */
struct Segment
{
	cocos2d::Vec3	start_point;//线段的起始点
	cocos2d::Vec3  final_point;//线段的终点
};
struct Segment2D
{
	cocos2d::Vec2 start_point;
	cocos2d::Vec2 final_point;
};
/*
  *射线
 */
struct Ray
{
	cocos2d::Vec3 origin;
	cocos2d::Vec3 direction;
};
/*
  *平面方程
  *标准化
 */
struct Plane
{
	cocos2d::Vec3  normal;
	float                    distance;
	//求一点到平面的距离
	float distanceTo(const cocos2d::Vec3 &point)const;
};
//创建直线
void  line_create(Line &a,const cocos2d::Vec3 &start_point,const cocos2d::Vec3 &final_point);
/*
  *创建线段
 */
void segment_create(Segment  &segment,const cocos2d::Vec3 &a,const cocos2d::Vec3 &b);
/*
  *射线
 */
void ray_create(Ray &ray,const cocos2d::Vec3 &origin,const cocos2d::Vec3 &direction);
/*
  *创建平面
 */
void plane_create(Plane &plane,const cocos2d::Vec3 &a,const cocos2d::Vec3 &b,const cocos2d::Vec3 &c);
void plane_create(Plane &plane,const cocos2d::Vec3 &normal,float d);
void plane_create(Plane &plane,const cocos2d::Vec3 &normal,const cocos2d::Vec3 &point);
/*
  *计算两个直线的最近距离
 */
float  line_line_minimum_distance(const Line  &a,const Line &b,cocos2d::Vec3 &intersect_apoint,cocos2d::Vec3 & intersect_bpoint);
/*
  *求两个线段的最近距离
  *并同时给出最近距离的两个点
*/
float  segment_segment_minimum_distance(const Segment  &a, const Segment &b, cocos2d::Vec3 &intersect_apoint, cocos2d::Vec3 & intersect_bpoint);
/*
  *求点到平面的正交投影点,以及相关的距离
 */
float plane_point_distance(const cocos2d::Vec3 &point,const Plane &plane,cocos2d::Vec3 &proj_point);
/*
  *点到线段的最近点
  *并给出线段上最近点的坐标
  *注意参数的不同
 */
float  line_point_minimum_distance(const cocos2d::Vec3 &point,const Segment  &segment,cocos2d::Vec3  *intersect_point);
/*
  *光线与球体的相交测试
 */
bool ray_sphere_intersect_test(const Ray &ray, const Sphere &sphere, cocos2d::Vec3 &intersect_point);
/*
  *线段与球体的相交测试
  */
bool segment_sphere_intersect_test(const Segment &segment,const Sphere &sphere);
/*
  *光线与AABB的相交测试
  *算法的核心思想为计算平行空间的交集
 */
bool ray_aabb_intersect_test(const Ray &ray,const AABB &aabb);
/*
  *光线与OBB相交测试
 */
bool ray_obb_intersect_test(const Ray &ray,const OBB &obb);
/*
  *直线与三角形之间的相交测试
 */
bool line_triangle_intersect_test(const Line &line, const Triangle &triangle);
/*
  *线段与三角形之间的相交测试
  *该算法将以克莱姆法则实现
 */
bool segment_triangle_intersect_test(const Segment &segment,const Triangle &triangle);
/*
  *两个平面之间的相交测试
  *如果相交,返回相交的直线方程
  *否则返回false
 */
bool plane_plane_intersect_test(const Plane &plane1,const Plane &plane2,Line &line);
/*
  *三个平面的相交测试
  *返回数据表明
  *0,相互平行
  *1,一个平面与另外两个平面分别相交于一条直线
  *2:三个平面两两相交于一条直线
  *3：三个平面相交于三条独立的直线
  *4,相交于一点,并给出交点
 */
bool plane_plane_plane_intersect_test(const Plane &plane1, const Plane &plane2, const Plane &plane3/*,std::vector<Line> &lines,*/,cocos2d::Vec3 &intersect_point);
/*
  * bresenham直线算法
 */
void bresenham_line_algorithm(int start_x,int start_y,int final_x,int final_y,int horizontal_num,int vertivcal_num,std::vector<cocos2d::Vec2> &location_array);
/*
  *线段与空间网格相交测试
  *3d算法,其2d实现与此算法类似
 */
void segment_grid3d_intersect_test(const Segment &segment,int horizontal_num,int vertical_num,int depth_num,const cocos2d::Vec3 &extent_unit,std::vector<cocos2d::Vec3> &intersect_array);
/*
 *线段与2d网格相交测试
 */
void segment_grid2d_intersect_test(const cocos2d::Vec2 &start_point,const cocos2d::Vec2 &final_point,int horizontal_num, int vertical_num, const cocos2d::Vec2 &extent_unit, std::vector<cocos2d::Vec2> &intersect_array);
/*
  *确定N条线段中是否有两条线段相交
  *如果有,则返回true,如果全部都不相交,则返回false
 */
bool segments2d_N_intersect_test(const Segment2D  *line_array,int line_size);
/*
  *两线段相交测试
  *并给出交点,如果相交
 */
bool segment_segment_intersect_test(const Segment2D &a,const Segment2D &b);
bool segment_segment_intersect_test(const Segment2D &a, const Segment2D &b, cocos2d::Vec2 &intersect_point);
/*
  *判断一个点是否在一个线段的左侧,如果刚好处于线段上,也将视为左侧
 */
bool point_segment_relative_location(const cocos2d::Vec2 &s,const cocos2d::Vec2 &b,const cocos2d::Vec2 &point);
/*
  *求N条线段之间的所有交点
  *输出所有的线段之间的交点,以及交点的数目
 */
int   segment_n_intersect_point(const std::vector<Segment2D> &segments,std::vector<cocos2d::Vec2> &intersect_points);
/*
  *线段求交的朴素算法实现
 */
int segment_n_intersect_prim(const std::vector<Segment2D> &segments, std::vector<cocos2d::Vec2> &intersect_points);
NS_GT_END
#endif