/*
  *�ཻ����
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
  *AABB��ײ���
 */
struct AABB
{
	cocos2d::Vec3  bb_min,bb_max;
};
/*
  *OBB��ײ����
 */
struct  OBB 
{
	cocos2d::Vec3 center;//����
	cocos2d::Vec3 extent;//OBB�뾶
	cocos2d::Vec3 xaxis, yaxis, zaxis;//����������
};
//����AABB
void  aabb_create(AABB &aabb,const cocos2d::Vec3 &min_bb,const cocos2d::Vec3 &max_bb);
/*
  *����OBB
   *����ֱ�Ӹ���,���ߴ�һ��AABB��ʼ��
   *���߸���һ��AABB,Ȼ�����һ��ѡ��ת����
   *���߸���AABB,���������������ת����Ĳ���
 */
void obb_create(OBB  &obb,const AABB &aabb);
void obb_create(OBB  &obb, const AABB &aabb, const mat3x3 &mat);
void obb_create(OBB  &obb, const AABB &aabb, const cocos2d::Vec3 &rotate_axis,float angle);
void obb_create(OBB &obb,const cocos2d::Vec3 &min_bb,const cocos2d::Vec3 &max_bb);
void obb_create(OBB  &obb, const cocos2d::Vec3 &min_bb, const cocos2d::Vec3 &max_bb, const mat3x3 &mat);
void obb_create(OBB  &obb, const cocos2d::Vec3 &min_bb, const cocos2d::Vec3 &max_bb, const cocos2d::Vec3 &rotate_axis, float angle);
/*
  *����OBB��8������
  *�����˳���ǰ����
 */
void  obb_create_obb_vertex8(const OBB &obb,cocos2d::Vec3  *vertex);
/*
  *AABB�ཻ����
 */
bool aabb_aabb_intersect_test(const AABB &a,const AABB &b);
/*
  *OBB��ײ����,���������,�㷨��ʵ������ <��ײ����㷨����>��3��
  *���㷨��ʵ�ֱȽϸ���,���Ҷ������и������㷨�����˼����Ż�
 */
bool obb_obb_intersect_test(const OBB &a,const OBB &b);
/*
  *�㵽AABB���������
 */
float aabb_point_minimum_distance(const AABB &aabb,const cocos2d::Vec3 &point,cocos2d::Vec3 &intersect_point);
/*
  *�㵽OBB���������
 */
float obb_point_minimum_distance(const OBB &obb, const cocos2d::Vec3 &point,cocos2d::Vec3 &intersect_point);
/*
  *AABB��ƽ����ཻ����
  *�����˼��Ϊ���������
  *��Զ���,aabb�����㷨��OBB�㷨�ļ򻯰�
 */
bool aabb_plane_intersect_test(const AABB &aabb,const Plane &plane);
/*
  *OBB��ƽ����ཻ����
  *OBB�㷨��Ը���,���㷨˼����Ϊ���������
 */
bool obb_plane_intersect_test(const OBB &obb,const Plane &plane);
/*
  *AABB��������ཻ����
 */
bool aabb_sphere_intersect_test(const AABB &aabb,const Sphere &sphere);
/*
  *OBB��������ཻ����
 */
bool obb_sphere_intersect_test(const OBB &obb,const Sphere &sphere);
/*
  *AABB�������ε��ཻ����
 */
bool aabb_triangle_intersect_test(const AABB &aabb,const Triangle &triangle);
/*
  *OBB�������ε��ཻ����
 */
bool obb_triangle_intersect_test(const OBB &obb,const Triangle &triangle);
/*
  *AABB���߶�֮����ཻ����
  *���������
 */
bool aabb_segment_intersect_test(const AABB &aabb,const Segment &segment);
/*
  *OBB���߶�֮����ཻ����
  *���������
 */
bool obb_segment_intersect_test(const OBB &obb,const Segment &segment);
NS_GT_END
#endif