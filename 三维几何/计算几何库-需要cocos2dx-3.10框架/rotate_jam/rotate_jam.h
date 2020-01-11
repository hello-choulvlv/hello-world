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
NS_GT_END
#endif