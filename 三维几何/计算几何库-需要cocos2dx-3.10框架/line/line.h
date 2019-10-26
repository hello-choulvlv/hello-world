/*
  *��ά�ռ�ֱ��֮��Ĺ�ϵ
  *�Լ��߶����߶�֮��Ĺ�ϵ�㷨ʵ��
  *2019��6��22��
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
  *ֱ��
 */
struct  Line
{
	cocos2d::Vec3   start_point;//ֱ���ϵ�����һ��
	cocos2d::Vec3   direction;//ֱ�ߵķ���
};
struct Line2D
{
	cocos2d::Vec2   start_point;//ֱ���ϵ�����һ��
	cocos2d::Vec2   direction;//ֱ�ߵķ���
};
/*
  *�߶�
 */
struct Segment
{
	cocos2d::Vec3	start_point;//�߶ε���ʼ��
	cocos2d::Vec3  final_point;//�߶ε��յ�
};
struct Segment2D
{
	cocos2d::Vec2 start_point;
	cocos2d::Vec2 final_point;
};
/*
  *����
 */
struct Ray
{
	cocos2d::Vec3 origin;
	cocos2d::Vec3 direction;
};
/*
  *ƽ�淽��
  *��׼��
 */
struct Plane
{
	cocos2d::Vec3  normal;
	float                    distance;
	//��һ�㵽ƽ��ľ���
	float distanceTo(const cocos2d::Vec3 &point)const;
};
//����ֱ��
void  line_create(Line &a,const cocos2d::Vec3 &start_point,const cocos2d::Vec3 &final_point);
/*
  *�����߶�
 */
void segment_create(Segment  &segment,const cocos2d::Vec3 &a,const cocos2d::Vec3 &b);
/*
  *����
 */
void ray_create(Ray &ray,const cocos2d::Vec3 &origin,const cocos2d::Vec3 &direction);
/*
  *����ƽ��
 */
void plane_create(Plane &plane,const cocos2d::Vec3 &a,const cocos2d::Vec3 &b,const cocos2d::Vec3 &c);
void plane_create(Plane &plane,const cocos2d::Vec3 &normal,float d);
void plane_create(Plane &plane,const cocos2d::Vec3 &normal,const cocos2d::Vec3 &point);
/*
  *��������ֱ�ߵ��������
 */
float  line_line_minimum_distance(const Line  &a,const Line &b,cocos2d::Vec3 &intersect_apoint,cocos2d::Vec3 & intersect_bpoint);
/*
  *�������߶ε��������
  *��ͬʱ������������������
*/
float  segment_segment_minimum_distance(const Segment  &a, const Segment &b, cocos2d::Vec3 &intersect_apoint, cocos2d::Vec3 & intersect_bpoint);
/*
  *��㵽ƽ�������ͶӰ��,�Լ���صľ���
 */
float plane_point_distance(const cocos2d::Vec3 &point,const Plane &plane,cocos2d::Vec3 &proj_point);
/*
  *�㵽�߶ε������
  *�������߶�������������
  *ע������Ĳ�ͬ
 */
float  line_point_minimum_distance(const cocos2d::Vec3 &point,const Segment  &segment,cocos2d::Vec3  *intersect_point);
/*
  *������������ཻ����
 */
bool ray_sphere_intersect_test(const Ray &ray, const Sphere &sphere, cocos2d::Vec3 &intersect_point);
/*
  *�߶���������ཻ����
  */
bool segment_sphere_intersect_test(const Segment &segment,const Sphere &sphere);
/*
  *������AABB���ཻ����
  *�㷨�ĺ���˼��Ϊ����ƽ�пռ�Ľ���
 */
bool ray_aabb_intersect_test(const Ray &ray,const AABB &aabb);
/*
  *������OBB�ཻ����
 */
bool ray_obb_intersect_test(const Ray &ray,const OBB &obb);
/*
  *ֱ����������֮����ཻ����
 */
bool line_triangle_intersect_test(const Line &line, const Triangle &triangle);
/*
  *�߶���������֮����ཻ����
  *���㷨���Կ���ķ����ʵ��
 */
bool segment_triangle_intersect_test(const Segment &segment,const Triangle &triangle);
/*
  *����ƽ��֮����ཻ����
  *����ཻ,�����ཻ��ֱ�߷���
  *���򷵻�false
 */
bool plane_plane_intersect_test(const Plane &plane1,const Plane &plane2,Line &line);
/*
  *����ƽ����ཻ����
  *�������ݱ���
  *0,�໥ƽ��
  *1,һ��ƽ������������ƽ��ֱ��ཻ��һ��ֱ��
  *2:����ƽ�������ཻ��һ��ֱ��
  *3������ƽ���ཻ������������ֱ��
  *4,�ཻ��һ��,����������
 */
bool plane_plane_plane_intersect_test(const Plane &plane1, const Plane &plane2, const Plane &plane3/*,std::vector<Line> &lines,*/,cocos2d::Vec3 &intersect_point);
/*
  * bresenhamֱ���㷨
 */
void bresenham_line_algorithm(int start_x,int start_y,int final_x,int final_y,int horizontal_num,int vertivcal_num,std::vector<cocos2d::Vec2> &location_array);
/*
  *�߶���ռ������ཻ����
  *3d�㷨,��2dʵ������㷨����
 */
void segment_grid3d_intersect_test(const Segment &segment,int horizontal_num,int vertical_num,int depth_num,const cocos2d::Vec3 &extent_unit,std::vector<cocos2d::Vec3> &intersect_array);
/*
 *�߶���2d�����ཻ����
 */
void segment_grid2d_intersect_test(const cocos2d::Vec2 &start_point,const cocos2d::Vec2 &final_point,int horizontal_num, int vertical_num, const cocos2d::Vec2 &extent_unit, std::vector<cocos2d::Vec2> &intersect_array);
/*
  *ȷ��N���߶����Ƿ��������߶��ཻ
  *�����,�򷵻�true,���ȫ�������ཻ,�򷵻�false
 */
bool segments2d_N_intersect_test(const Segment2D  *line_array,int line_size);
/*
  *���߶��ཻ����
  *����������,����ཻ
 */
bool segment_segment_intersect_test(const Segment2D &a,const Segment2D &b);
bool segment_segment_intersect_test(const Segment2D &a, const Segment2D &b, cocos2d::Vec2 &intersect_point);
/*
  *�ж�һ�����Ƿ���һ���߶ε����,����պô����߶���,Ҳ����Ϊ���
 */
bool point_segment_relative_location(const cocos2d::Vec2 &s,const cocos2d::Vec2 &b,const cocos2d::Vec2 &point);
/*
  *��N���߶�֮������н���
  *������е��߶�֮��Ľ���,�Լ��������Ŀ
 */
int   segment_n_intersect_point(const std::vector<Segment2D> &segments,std::vector<cocos2d::Vec2> &intersect_points);
/*
  *�߶��󽻵������㷨ʵ��
 */
int segment_n_intersect_prim(const std::vector<Segment2D> &segments, std::vector<cocos2d::Vec2> &intersect_points);
NS_GT_END
#endif