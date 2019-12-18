/*
  *离散点集的处理
  *2019/6/16
 */
#ifndef _GT_POINT_POLYGON_H__
#define _GT_POINT_POLYGON_H__
#include "math/Vec2.h"
#include "math/Vec3.h"
#include<vector>
#include "gt_common/geometry_types.h"

NS_GT_BEGIN
struct Line;
struct Segment;
struct Ray;
struct Cycle;
/*
  *2D平面
 */
struct Plane2D
{
	cocos2d::Vec2 normal;
	float                    distance;
};
void plane2d_create(Plane2D &plane,const cocos2d::Vec2 &a,const cocos2d::Vec2 &b);
void plane2d_create(Plane2D &plane,const cocos2d::Vec2 &normal,float d);
/*
  *凸多变形
  *其构成方式有两种
  *半空间交集
  *边的集合
 */
struct Polygon
{
	std::vector<Plane2D>     plane_array;
	std::vector<cocos2d::Vec2>  point_array;
};
//创建多边形,注意函数不会检查是否多边形的半空间交集会形成一个凸多边形
void polygon_create(Polygon &polygon,const std::vector<cocos2d::Vec2 > &points);
//判断线段是否与多边形相交
//注意直线的第三位数据将会被直接的忽略
bool polygon_line_intersect_test(const Polygon &polygon,const Line &line);
//线段与多边形相交测试
bool polygon_segment_intersect_test(const Polygon &polygon,const Segment &segment);
//射线与多边形之间的相交测试
bool polygon_ray_intersect_test(const Polygon &polygon,const Ray &ray);
///点与多边形的包含测试
bool polygon_contains_point(const Polygon &polygon,const cocos2d::Vec2 &point);
//三角平面是否包含某一个点
bool triangle_contains_point(const cocos2d::Vec2 vertex[3],const cocos2d::Vec2 &point);
//多边形与圆的相交测试
bool polygon_cycle_intersect_test(const Polygon &polygon,const Cycle &cycle);
//////////////////////////////////////////////////////////////////////////
/*
*计算质心坐标,如果给定的点不与其他的点共面,则返回false
*/
bool compute_barycentric(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &p, cocos2d::Vec3 &coord);

/*
*判断目标点是否位于三角形所在的局部平面之内
*/
bool check_in_target_plane(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &p);

/*
*求点集的最小凸多边形
*旋转卡壳算法实现
*/
bool polygon_compute_minimum(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon_points);
/*
  *计算两个多边形的公切线
  *对输入已经假设两个多边形不会相交,并且其顶点形式都是逆时针排列的
  *输出是两对顶点的索引,<p1,p2>,<p1,p2>分别代表上公切线,下公切线
 */
void polygon_polygon_tangent_line(const std::vector<cocos2d::Vec2> &polygon1,const std::vector<cocos2d::Vec2> &polygon2,int tangent_index_array[4]);
/*
  *求多边形的最远距离
  *并给出距离最远的两个点坐标
 */
float polygon_compute_max_distance(const std::vector<cocos2d::Vec2> &polygon_points,cocos2d::Vec2 &max_a,cocos2d::Vec2 &max_b);
/*
*quick hull算法实现
*2d实现
*/
bool quick_hull_algorithm2d(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon);

/*
*quick hull 算法实现
*最小3d凸平面
*输出,每三个顶点构成一个空间平面
*/
bool quick_hull_algorithm3d(const std::vector<cocos2d::Vec3> &points, std::vector<cocos2d::Vec3> &planes);
/*
  *计算N个离散点的最近距离
  *2d是实现
 */
float point_compute_minimum_distance(const std::vector<cocos2d::Vec2> &points,cocos2d::Vec2 &a,cocos2d::Vec2 &b);
/*
  *最近点对朴素算法实现
  *2d/3d
 */
float point_prim_compute_minimum_distance(const std::vector<cocos2d::Vec2> &points, cocos2d::Vec2 &a, cocos2d::Vec2 &b);
float point_prim_compute_minimum_distance(const std::vector<cocos2d::Vec3> &points, cocos2d::Vec3 &a, cocos2d::Vec3 &b);
/*
  *计算N个离散点对的最近距离
  *3d实现
 */
float point_compute_minimum_distance(const std::vector<cocos2d::Vec3> &points,cocos2d::Vec3 &a,cocos2d::Vec3 &b);
/*
  *GJK算法实现
  *2d平面多边形相交测试
  *如果想要返回最近点,请修改这个函数的布局实现过程
  *修改非常简单
 */
bool polygon_polygon_intersect_test(const Polygon &pa,const Polygon &pb,cocos2d::Vec2 &a,cocos2d::Vec2 &b);
/*
  *三角剖分算法实现
  *其包含了一系列的函数
  *其输入假设是简单多边形，并且其顶点的顺序为逆时针
  *其输出将包含了一些列的单调Y多边形
 */
bool polygon_simple_decompose(const std::vector<cocos2d::Vec2> &polygon_points,std::map<int,int>  &addtional_edge_map,std::vector<int> &points_sequence,std::vector<int> &boundary_index);
/*
  *计算单调剖分图中的圆环序列
  *核心算法位为广度优先级搜索
 */
void polygon_simple_cycle_sequence(int point_number,const std::map<int,int> &adj_edge, std::vector<int> &points_sequence, std::vector<int> &boundary_index);
/*
  *简单多边形的生成算法
 */
void polygon_simple_generate(std::vector<cocos2d::Vec2> &polygon_points, std::vector<cocos2d::Vec2> &simple_polygon);
/*
  *y单调多边形的三角剖分算法实现
  *输出生成的三角形顶点序列,以及新增加的边
  *注意,算法并没有经过优化
  *目前,暂时没有将生成三角形序列的算法实现,读者可以自己去完成
 */
void polygon_monotone_triangulate(const std::vector<cocos2d::Vec2> &points_array,const int *sequence_array,int array_size,std::vector<int>  &triangle_sequence,std::map<int,int> &addtional_edge_map);
NS_GT_END
#endif