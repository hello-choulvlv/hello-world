/*
  *3d������,�ı���,����
  *2019��7��12��
  *@author:xiaohuaxiong
 */
#ifndef __GT_TRIANGLE_H__
#define __GT_TRIANGLE_H__
#include "gt_common/geometry_types.h"
#include "math/Vec3.h"
#include "math/Vec2.h"
NS_GT_BEGIN
struct Plane;
struct Sphere;
struct Line;
struct Segment;
/*
  *3d����
  *�������ֲ�ͬ�Ķ��巽ʽ
 */
struct Rectangle
{
	cocos2d::Vec3 center;
	cocos2d::Vec2 extent;
	cocos2d::Vec3 xaxis, yaxis;
};
/*
  *�������㶨��ľ���
  *���߱��봹ֱ
*/
struct Rectangle3v
{
	cocos2d::Vec3 a, b, c;
};
/*
  *�����������ֲ�ͬ�Ķ���
 */
struct Triangle
{
	cocos2d::Vec3 a, b, c;
};
/*
  *������
 */
struct Tetrahedron
{
	cocos2d::Vec3 a, b, c, d;
};
/*
  *׶��
 */
struct Cone
{
	cocos2d::Vec3   top;//׶�εĶ���
	cocos2d::Vec3   normal;//׶�εķ���
	float                     h, r;//׶�εĳ���,�Լ��ײ��İ뾶
};
/*
  *Բ����
 */
struct Cylinder
{
	cocos2d::Vec3 bottom;//��������ĵ�
	cocos2d::Vec3 direction;//Բ����ķ�������,��λ��
	float                   length, r;//ĸ�ߵĳ���,�����İ뾶
};
/*
  *����3d����
 */
bool rectangle_create(Rectangle &rect,const cocos2d::Vec3 &min_vertex,const cocos2d::Vec3 &max_vertex,const cocos2d::Vec3 &axis,float angle);
void rectangle_create(Rectangle &rect,const cocos2d::Vec3 &center,const cocos2d::Vec3 &xaxis,const cocos2d::Vec3 &yaxis,const cocos2d::Vec2 &extent);
//��ȡ����
void rectangle_get_vertex(const Rectangle &rect,cocos2d::Vec3 *vertex);
/*
  *������һ��3d����
 */
void rectangle3v_create(Rectangle3v &rect, const cocos2d::Vec3 &corner,const cocos2d::Vec2 &extent ,const cocos2d::Vec3 &axis, float angle);
bool rectangle3v_create(Rectangle3v &rect,const cocos2d::Vec3 &corner,const cocos2d::Vec3 &b,const cocos2d::Vec3 &c);
bool rectangle3v_create(Rectangle3v &rect,const cocos2d::Vec3 &corner,const cocos2d::Vec3 &xaxis,const cocos2d::Vec3 &yaxis,const cocos2d::Vec2 &extent);
//��ȡ����
void rectangle3v_get_vertex(const Rectangle3v &rect, cocos2d::Vec3 *vertex);
/*
  *��һ���㵽3d���ε��������,�Լ������
 */
float rectangle_point_min_distance(const Rectangle &rect,const cocos2d::Vec3 &point,cocos2d::Vec3 &intersect_point);
float rectangle3v_point_min_distance(const Rectangle3v &rect,const cocos2d::Vec3 &point,cocos2d::Vec3 &intersect_point);
#pragma mark --------------------triangle------------------------------------------------------
void triangle_create(Triangle &triangle,const cocos2d::Vec3 &a,const cocos2d::Vec3 &b,const cocos2d::Vec3 &c);
void triangle_create(Triangle &triangle,const cocos2d::Vec3 *v);
/*
  *����������ε��������,�������
  *�������ļ���ͼԪ���,�����εĲ��ԱȽϸ���,
  *���л��漰��Voronoi����ж�
 */
float triangle_point_min_distance(const Triangle &triangle,const cocos2d::Vec3 &point, cocos2d::Vec3 &intersect_point);
/*
  *�߶ε�����ƽ����������
 */
float triangle_segment_distance(const Triangle &triangle,const cocos2d::Vec3 *vert,cocos2d::Vec3 &triangle_point,cocos2d::Vec3 &segment_point);
/*
  *�㵽ƽ��ľ���
 */
float plane_point_distance(const cocos2d::Vec3 &p,const cocos2d::Vec3 &a,const cocos2d::Vec3 &b,const cocos2d::Vec3 &c);
/*
  *����������
 */
void tetrahedron_create(Tetrahedron &tet, const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &d);
void tetrahedron_create(Tetrahedron &tet, const cocos2d::Vec3 *vertex);
/*
  *��㵽��������������
  *���㷨�����ֲ�ͬ����ʽ
 */
float tetrahedron_point_min_distance(const Tetrahedron &tet, const cocos2d::Vec3 &point, cocos2d::Vec3 &intersect_point);
float tetrahedron_point_min_distance(const cocos2d::Vec3 *vert,const cocos2d::Vec3 &point,cocos2d::Vec3 &intersect_point);
/*
  *����׶��
 */
void cone_create(Cone &cone,const cocos2d::Vec3 &top,const cocos2d::Vec3 &normal,float h,float r);
/*
  *׶����ƽ����ཻ����
 */
bool cone_plane_intersect_test(const Cone &cone,const Plane &plane);
/*
  *��������������ཻ����
  */
bool triangle_sphere_intersect_test(const Triangle &triangle,const Sphere &sphere);
/*
  *��������������֮����ཻ����
 */
bool triangle_triangle_intersect_test(const Triangle &t1,const Triangle &t2);
/*
  *��������ֱ��֮����ཻ����
 */
bool triangle_line_intersect_test(const Triangle &triangle,const Line &line);
/*
  *���������߶�֮����ཻ����
  *���㷨��������������
  *���Լ����㷨����ʽ�������Խ��
  *ע��ú�����û�о����Ż�
 */
bool triangle_segment_intersect_test(const Triangle &triangle,const Segment &segment);
/*
  *Բ����
 */
void cylinder_create(Cylinder &cylinder,const cocos2d::Vec3 &bottom,const cocos2d::Vec3 &direction,float length,float r);
void cylinder_create(Cylinder &cylinder,const cocos2d::Vec3 &bottom,const cocos2d::Vec3 &top,float r);
/*
  *Բ������ֱ��֮����ཻ����
 */
bool cylinder_line_intersect_test(const Cylinder &cylinder,const Line &line);
/*
  *Բ�������߶�֮����ཻ����
  *����Ϊ���ֱ����Բ������ཻ���Զ���Ҫ��΢����һЩ
 */
bool cylinder_segment_intersect_test(const Cylinder &cylinder,const Segment &segment);
NS_GT_END
#endif