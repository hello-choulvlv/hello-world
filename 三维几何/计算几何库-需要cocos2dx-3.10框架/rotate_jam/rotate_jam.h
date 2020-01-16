/*
  *旋转卡壳算法系列
  *@author:xiaohuaxiong
  *@date:2020年1月8日
 */
#ifndef __ROTATE_JAM_H__
#define __ROTATE_JAM_H__
#include "gt_common/geometry_types.h"
#include "line/line.h"
NS_GT_BEGIN
/*
  *直线/射线/线段的超集
 */
enum LineType
{
	LineType_Line = 0,//直线
	LineType_Ray = 1,//射线
	LineType_Segment = 2,//线段
};
struct  SuperLine2D
{
	LineType  line_type;
	cocos2d::Vec2 start_point, unknown;//unknown域可能代表着方向,也可能代表着一个点,这取决于它的类型
};
bool superline_intersect_test(const SuperLine2D &a, const SuperLine2D &b, cocos2d::Vec2 &intersect_point);
//半平面求交测试返回结果类型
enum HalfResultType {
	ResultType_Empty=0,//返回结果,交集为空
	ResultType_Polygon = 1,//返回的结果是一个闭合的凸多边形
	ResultType_Unboundary = 2,//结果集合是无界的
};

/*
  *半平面求交算法实现,我们将始终遵循直线的半平面指代这其方向向量的左侧
  *理论上,其并不具备公认的旋转卡壳算法的步骤特征
  *然而,因为其实现形式具备旋转特征,
  *因此我们仍将其归类到旋转卡壳算法系列
  *算法要求输入的直线的数目必须大于等于 3
  *假设输入中并不存在平行但方向相反的直线
  *算法的最终目的只是计算出半平面的多边形交,如果它们可以形成的话,如果不是,则是不能正确计算出相关情况的.
  *针对任意的情况的实现,请参见下一个算法实现
 */
HalfResultType half_planes_intersect(const std::vector<Line2D> &half_planes, std::vector<SuperLine2D> &super_lines);
/*
  *旋转卡壳算法之求凸包的直径
  *2020年1月11日
  *https://blog.csdn.net/u012328159/article/details/50809014
 */
float rotate_hull_max_distance(const std::vector<cocos2d::Vec2> &hull_points,int &start_index,int &final_index);
/*
  *旋转卡壳-->求凸多边形的宽度
 */
float rotate_hull_width(const std::vector<cocos2d::Vec2> &hull_points,cocos2d::Vec2 &start_point,cocos2d::Vec2 &final_point);
/*
  *求两个凸多边形之间的最大距离
 */
float rotate_hull_max_between(const std::vector<cocos2d::Vec2> &hull_points1,const std::vector<cocos2d::Vec2> &hull_points2,int &ahull_index,int &bhull_index);
/*
  *求两个分离的凸多边形之间的最小距离
  *注意,该两个多边形之间一定不能相交
  *否则计算的结果将会不正确
  *如果想要精确计算是否相交,请参考GJK算法实现,在point_polygon.cpp文件中
 */
float rotate_hull_min_between(const std::vector<cocos2d::Vec2> &hull_points1, const std::vector<cocos2d::Vec2> &hull_points2, cocos2d::Vec2 &ahull_point,cocos2d::Vec2 &bhull_point);
/*
  *求凸多边形的最小面积外接矩形
  *最终的坐标点结果将写入到rect_points种
  *算法的核心思想是,使用两队旋转平行线逐点测试
  *算法的复杂度为O(n)
 */
float rotate_hull_min_area(const std::vector<cocos2d::Vec2> &hull_points,cocos2d::Vec2 rect_points[4]);
/*
  *求凸多边形的最小周长
  *算法的核心思想与上面的算法相同
  *区别只在于求的目标不同,
  *另外,一般情况下两者的计算结果相同
  *然而,如果出现连续平行边时,计算结果将出现差异
 */
float rotate_hull_min_perimeter(const std::vector<cocos2d::Vec2> &hull_points,cocos2d::Vec2 rect_points[4]);
/*
  *洋葱三角剖分算法实现
  *再第一版中,我们只实现基本的算法目标
  *第二版中我们将使用Chazelle的Convex Layer算法来优化算法的实现
 */
void rotate_hull_onion_decomposite(const std::vector<cocos2d::Vec2> &points,std::vector<const cocos2d::Vec2*> &triangle_edges);
/*
  *计算离散点集的螺旋线
 */
int rotate_hull_spiral_line(const std::vector<cocos2d::Vec2> &points,std::vector<const cocos2d::Vec2*> &spiral_points);
/*
  *离散点集的螺旋三角剖分
 */
void rotate_hull_spiral_decomposite(const std::vector<cocos2d::Vec2> &points, std::vector<const cocos2d::Vec2*> &triangle_edges);
NS_GT_END
#endif