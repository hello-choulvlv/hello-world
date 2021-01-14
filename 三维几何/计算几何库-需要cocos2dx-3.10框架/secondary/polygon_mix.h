/*
  *GJK算法+Chung-Wang算法+简单多边形的耳式三角剖分算法+SSE指令优化实现
  *所有算法都假定,多边形的顶点呈逆时针排列
  *@2021年1月8日
  *@author:xiaoxiong
 */
#ifndef __polygon_mix_h__
#define __polygon_mix_h__
#include "math/Vec2.h"
#include "gt_common/geometry_types.h"
#include <vector>
NS_GT_BEGIN
/*
  *gjk算法实现+Optimal
 */
bool gjk_algorithm_optimal(const std::vector<cocos2d::Vec2> &polygon1,const std::vector<cocos2d::Vec2> &polygon2,cocos2d::Vec2 near_points[2]);
/*
  *Chung-Wang分离向量法
  *经过大量测试,再二维情况下,该算法的稳定性远不如求凸多边形之间的内公切线算法
 */
bool chung_wang_seperate_algorithm(const std::vector<cocos2d::Vec2> &polygon1, const std::vector<cocos2d::Vec2> &polygon2, cocos2d::Vec2 &seperate_vec2);
/*
  *简单多边形的耳式三角剖分
 */
void simple_polygon_ear_triangulate(const std::vector<cocos2d::Vec2> &polygon,std::vector<short> &triangle_list);
/*
  ************************sse指令Sample**********************************
 */
void sse_code_sample();
//一般矩阵乘法
void matrix_multiply(const float m1[16],const float m2[16],float dst[16]);
//sse版矩阵乘法
void sse_matrix_multiply(const float m1[16],const float m2[16],float dst[16]);
NS_GT_END
#endif