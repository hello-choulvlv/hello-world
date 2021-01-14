/*
  *GJK�㷨+Chung-Wang�㷨+�򵥶���εĶ�ʽ�����ʷ��㷨+SSEָ���Ż�ʵ��
  *�����㷨���ٶ�,����εĶ������ʱ������
  *@2021��1��8��
  *@author:xiaoxiong
 */
#ifndef __polygon_mix_h__
#define __polygon_mix_h__
#include "math/Vec2.h"
#include "gt_common/geometry_types.h"
#include <vector>
NS_GT_BEGIN
/*
  *gjk�㷨ʵ��+Optimal
 */
bool gjk_algorithm_optimal(const std::vector<cocos2d::Vec2> &polygon1,const std::vector<cocos2d::Vec2> &polygon2,cocos2d::Vec2 near_points[2]);
/*
  *Chung-Wang����������
  *������������,�ٶ�ά�����,���㷨���ȶ���Զ������͹�����֮����ڹ������㷨
 */
bool chung_wang_seperate_algorithm(const std::vector<cocos2d::Vec2> &polygon1, const std::vector<cocos2d::Vec2> &polygon2, cocos2d::Vec2 &seperate_vec2);
/*
  *�򵥶���εĶ�ʽ�����ʷ�
 */
void simple_polygon_ear_triangulate(const std::vector<cocos2d::Vec2> &polygon,std::vector<short> &triangle_list);
/*
  ************************sseָ��Sample**********************************
 */
void sse_code_sample();
//һ�����˷�
void matrix_multiply(const float m1[16],const float m2[16],float dst[16]);
//sse�����˷�
void sse_matrix_multiply(const float m1[16],const float m2[16],float dst[16]);
NS_GT_END
#endif