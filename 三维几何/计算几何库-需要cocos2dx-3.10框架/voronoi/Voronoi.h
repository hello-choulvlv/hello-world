/*
  *Voronoi图算法+平面点集的三角剖分算法
  *因为该算法系列比较庞大而且复杂,并且有着多种的完全不同的思路,两者之间有着密切的关系
  *因此需要单独抽取出来形成一个文件.
  *2019年12月12日
  *@author:xiaoxiong
*/
#include "gt_common/geometry_types.h"
#include "math/Vec2.h"
#include "math/CCGeometry.h"

NS_GT_BEGIN
struct Cycle;
/*
  *三角形顶点序列
 */
struct DelaunayTriangle
{
	short v1, v2, v3;
};
//边
struct DelaunayEdge
{
	short v1, v2;
};
bool operator ==(const DelaunayEdge &, const DelaunayEdge &other);
bool operator >(const DelaunayEdge &, const DelaunayEdge &);
bool operator < (const DelaunayEdge &, const DelaunayEdge &);

void static_create_cycle_by_triangle(Cycle &cycle, const std::vector<cocos2d::Vec2> &disper_points, const DelaunayTriangle &delaunay_triangle);
/*
  *求给定矩形的最小外接三角形
  *此三角形为一个正三角形
 */
void rect_outerline_triangle(const cocos2d::Vec2 &origin,const cocos2d::Vec2 &extent,cocos2d::Vec2 triangle[3]);
/*
  *计算离散点集的Delaunay三角剖分
  *使用Bowyer-Watson算法
  *算法对于输入已经做了相关的假设
  *其外接超级三角形的顶点坐标位于最后三个
  *且其有效顶点的数据大于3个
*/
void delaunay_triangulate_bowyer_washton(const std::vector<cocos2d::Vec2> &disper_points,std::vector<DelaunayTriangle> &triangle_sequence,int &real_size);
NS_GT_END