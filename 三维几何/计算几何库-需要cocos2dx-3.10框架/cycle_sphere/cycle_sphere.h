/*
  *�ռ��ά,��άԲ�Լ����嶨��
  *2019��5��23��
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
*��ά�������,��ʾһ��Բ��Ҫ
*��һ��ά��,����Բ�Ĵ���Բ�ĵķ�����
*/
struct Cycle3
{
	cocos2d::Vec3  center;
	cocos2d::Vec3  normal;
	float                    radius;
};
void sphere_create(Sphere &sphere,const cocos2d::Vec3 &center,float radius);
/*
*�Ƿ�һ������Բ��
*/
bool check_point_insideof_cycle(const Cycle &cycle, const cocos2d::Vec2 &point);
/*
*��ά����,����׶��İ�ƽ��Լ������
*�˺���ר������ӰͶ��ü�
*/
//void  shadow_frustum_half_planes(const cocos2d::Frustum	&fustum, const cocos2d::Vec3 &light_direction, cocos2d::Plane *planes, int &plane_length);
/*
*����,����ȷ��һ��Բ
*/
void cycle_create(Cycle &cycle,const cocos2d::Vec2 &center,float radius);
void cycle_create(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b, Cycle &cycle);
void cycle_create(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b, const cocos2d::Vec2 &c, Cycle &cycle);
//����Բ,��������Ϊ,�߽���봩��Ŀ���,��ʱ���γɵ�Բ��һ���������Բ
//ǰ��������Ϊ�߽��,��һ�������Բ��,Ҳ�����ڱ߽���
void  create_mininum_cycle3(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b, const cocos2d::Vec2 &c, Cycle &cycle);
/*
*������ɢ�㼯�ϵİ�ΧԲ
*��ͨ���㷨
*/
void  compute_points_cycle_normal(const std::vector<cocos2d::Vec2> &points, Cycle  &cycle);

/*
*������ɢ�㼯�ϵ���С��ΧԲ
*Welzl�㷨
*/
void  compute_points_minimum_cycle(const std::vector<cocos2d::Vec2> &points, Cycle  &cycle);
//�����㷨,��С��ΧԲ,����㸴�Ӷ�ҪС��������㷨
void  compute_points_minimum_cycle_wangwei(const std::vector<cocos2d::Vec2> &points, Cycle  &cycle);
//�ռ��������С��ΧԲ
void  create_mininum_cycle3(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, Cycle3 &cycle);
//��������
void  create_sphere2(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, Sphere &sphere);
void  create_sphere3(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, Sphere &sphere);
void  create_sphere4(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &d, Sphere &sphere);
/*
*������С��Χ��
*����ĸ����㶼����ͬһ��ƽ����
*/
void  create_minimum_sphere4_plane(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &d, Sphere &sphere);
void  create_minimum_sphere4(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &d, Sphere &sphere);
bool check_point_insideof_sphere(const Sphere &sphere, const cocos2d::Vec3 &point);
bool check_point_onsurfaceof_sphere(const Sphere &sphere, const cocos2d::Vec3 &point);
/*
*�󶥵㼯�ϵ���С��Χ��
*һ���㷨
*/
void  compute_minimum_sphere_normal(const std::vector<cocos2d::Vec3> &points, Sphere  &sphere);
/*
*�󶥵����С��Χ��
*Welzl�㷨
*/
void  compute_vertex_minimum_sphere(const std::vector<cocos2d::Vec3> &points, Sphere  &sphere);
/*
  *���嵽ƽ��ľ���
 */
float sphere_plane_minimum_distance(const Sphere &sphere,const Plane &plane);
/*
  *������AABB���ཻ����
 */

NS_GT_END
#endif