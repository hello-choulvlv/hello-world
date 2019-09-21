/*
  *��ɢ�㼯�Ĵ���
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
  *2Dƽ��
 */
struct Plane2D
{
	cocos2d::Vec2 normal;
	float                    distance;
};
void plane2d_create(Plane2D &plane,const cocos2d::Vec2 &a,const cocos2d::Vec2 &b);
void plane2d_create(Plane2D &plane,const cocos2d::Vec2 &normal,float d);
/*
  *͹�����
  *�乹�ɷ�ʽ������
  *��ռ佻��
  *�ߵļ���
 */
struct Polygon
{
	std::vector<Plane2D>     plane_array;
	std::vector<cocos2d::Vec2>  point_array;
};
//���������,ע�⺯���������Ƿ����εİ�ռ佻�����γ�һ��͹�����
void polygon_create(Polygon &polygon,const std::vector<cocos2d::Vec2 > &points);
//�ж��߶��Ƿ��������ཻ
//ע��ֱ�ߵĵ���λ���ݽ��ᱻֱ�ӵĺ���
bool polygon_line_intersect_test(const Polygon &polygon,const Line &line);
//�߶��������ཻ����
bool polygon_segment_intersect_test(const Polygon &polygon,const Segment &segment);
//����������֮����ཻ����
bool polygon_ray_intersect_test(const Polygon &polygon,const Ray &ray);
///�������εİ�������
bool polygon_contains_point(const Polygon &polygon,const cocos2d::Vec2 &point);
//����ƽ���Ƿ����ĳһ����
bool triangle_contains_point(const cocos2d::Vec2 vertex[3],const cocos2d::Vec2 &point);
//�������Բ���ཻ����
bool polygon_cycle_intersect_test(const Polygon &polygon,const Cycle &cycle);
/*
*������
*/
template<typename TK>
void  quick_sort(TK *source, int   tk_num, std::function<bool(const TK &a, const TK &b)> &compare_func);
//////////////////////////////////////////////////////////////////////////
/*
*������������,��������ĵ㲻�������ĵ㹲��,�򷵻�false
*/
bool compute_barycentric(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &p, cocos2d::Vec3 &coord);

/*
*�ж�Ŀ����Ƿ�λ�����������ڵľֲ�ƽ��֮��
*/
bool check_in_target_plane(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &p);

/*
*��㼯����С͹�����
*��ת�����㷨ʵ��
*/
bool polygon_compute_minimum(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon_points);
/*
  *�����ε���Զ����
  *������������Զ������������
 */
float polygon_compute_max_distance(const std::vector<cocos2d::Vec2> &polygon_points,cocos2d::Vec2 &max_a,cocos2d::Vec2 &max_b);
/*
*quick hull�㷨ʵ��
*2dʵ��
*/
bool quick_hull_algorithm2d(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon);

/*
*quick hull �㷨ʵ��
*��С3d͹ƽ��
*���,ÿ�������㹹��һ���ռ�ƽ��
*/
bool quick_hull_algorithm3d(const std::vector<cocos2d::Vec3> &points, std::vector<cocos2d::Vec3> &planes);
/*
  *����N����ɢ����������
  *2d��ʵ��
 */
float point_compute_minimum_distance(const std::vector<cocos2d::Vec2> &points,cocos2d::Vec2 &a,cocos2d::Vec2 &b);
/*
  *�����������㷨ʵ��
  *2d/3d
 */
float point_prim_compute_minimum_distance(const std::vector<cocos2d::Vec2> &points, cocos2d::Vec2 &a, cocos2d::Vec2 &b);
float point_prim_compute_minimum_distance(const std::vector<cocos2d::Vec3> &points, cocos2d::Vec3 &a, cocos2d::Vec3 &b);
/*
  *����N����ɢ��Ե��������
  *3dʵ��
 */
float point_compute_minimum_distance(const std::vector<cocos2d::Vec3> &points,cocos2d::Vec3 &a,cocos2d::Vec3 &b);
/*
  *GJK�㷨ʵ��
  *2dƽ�������ཻ����
  *�����Ҫ���������,���޸���������Ĳ���ʵ�ֹ���
  *�޸ķǳ���
 */
bool polygon_polygon_intersect_test(const Polygon &pa,const Polygon &pb,cocos2d::Vec2 &a,cocos2d::Vec2 &b);
NS_GT_END
#endif