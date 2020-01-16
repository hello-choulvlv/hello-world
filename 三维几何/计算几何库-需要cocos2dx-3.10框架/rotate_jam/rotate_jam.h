/*
  *��ת�����㷨ϵ��
  *@author:xiaohuaxiong
  *@date:2020��1��8��
 */
#ifndef __ROTATE_JAM_H__
#define __ROTATE_JAM_H__
#include "gt_common/geometry_types.h"
#include "line/line.h"
NS_GT_BEGIN
/*
  *ֱ��/����/�߶εĳ���
 */
enum LineType
{
	LineType_Line = 0,//ֱ��
	LineType_Ray = 1,//����
	LineType_Segment = 2,//�߶�
};
struct  SuperLine2D
{
	LineType  line_type;
	cocos2d::Vec2 start_point, unknown;//unknown����ܴ����ŷ���,Ҳ���ܴ�����һ����,��ȡ������������
};
bool superline_intersect_test(const SuperLine2D &a, const SuperLine2D &b, cocos2d::Vec2 &intersect_point);
//��ƽ���󽻲��Է��ؽ������
enum HalfResultType {
	ResultType_Empty=0,//���ؽ��,����Ϊ��
	ResultType_Polygon = 1,//���صĽ����һ���պϵ�͹�����
	ResultType_Unboundary = 2,//����������޽��
};

/*
  *��ƽ�����㷨ʵ��,���ǽ�ʼ����ѭֱ�ߵİ�ƽ��ָ�����䷽�����������
  *������,�䲢���߱����ϵ���ת�����㷨�Ĳ�������
  *Ȼ��,��Ϊ��ʵ����ʽ�߱���ת����,
  *��������Խ�����ൽ��ת�����㷨ϵ��
  *�㷨Ҫ�������ֱ�ߵ���Ŀ������ڵ��� 3
  *���������в�������ƽ�е������෴��ֱ��
  *�㷨������Ŀ��ֻ�Ǽ������ƽ��Ķ���ν�,������ǿ����γɵĻ�,�������,���ǲ�����ȷ�������������.
  *�������������ʵ��,��μ���һ���㷨ʵ��
 */
HalfResultType half_planes_intersect(const std::vector<Line2D> &half_planes, std::vector<SuperLine2D> &super_lines);
/*
  *��ת�����㷨֮��͹����ֱ��
  *2020��1��11��
  *https://blog.csdn.net/u012328159/article/details/50809014
 */
float rotate_hull_max_distance(const std::vector<cocos2d::Vec2> &hull_points,int &start_index,int &final_index);
/*
  *��ת����-->��͹����εĿ��
 */
float rotate_hull_width(const std::vector<cocos2d::Vec2> &hull_points,cocos2d::Vec2 &start_point,cocos2d::Vec2 &final_point);
/*
  *������͹�����֮���������
 */
float rotate_hull_max_between(const std::vector<cocos2d::Vec2> &hull_points1,const std::vector<cocos2d::Vec2> &hull_points2,int &ahull_index,int &bhull_index);
/*
  *�����������͹�����֮�����С����
  *ע��,�����������֮��һ�������ཻ
  *�������Ľ�����᲻��ȷ
  *�����Ҫ��ȷ�����Ƿ��ཻ,��ο�GJK�㷨ʵ��,��point_polygon.cpp�ļ���
 */
float rotate_hull_min_between(const std::vector<cocos2d::Vec2> &hull_points1, const std::vector<cocos2d::Vec2> &hull_points2, cocos2d::Vec2 &ahull_point,cocos2d::Vec2 &bhull_point);
/*
  *��͹����ε���С�����Ӿ���
  *���յ����������д�뵽rect_points��
  *�㷨�ĺ���˼����,ʹ��������תƽ����������
  *�㷨�ĸ��Ӷ�ΪO(n)
 */
float rotate_hull_min_area(const std::vector<cocos2d::Vec2> &hull_points,cocos2d::Vec2 rect_points[4]);
/*
  *��͹����ε���С�ܳ�
  *�㷨�ĺ���˼����������㷨��ͬ
  *����ֻ�������Ŀ�겻ͬ,
  *����,һ����������ߵļ�������ͬ
  *Ȼ��,�����������ƽ�б�ʱ,�����������ֲ���
 */
float rotate_hull_min_perimeter(const std::vector<cocos2d::Vec2> &hull_points,cocos2d::Vec2 rect_points[4]);
/*
  *��������ʷ��㷨ʵ��
  *�ٵ�һ����,����ֻʵ�ֻ������㷨Ŀ��
  *�ڶ��������ǽ�ʹ��Chazelle��Convex Layer�㷨���Ż��㷨��ʵ��
 */
void rotate_hull_onion_decomposite(const std::vector<cocos2d::Vec2> &points,std::vector<const cocos2d::Vec2*> &triangle_edges);
/*
  *������ɢ�㼯��������
 */
int rotate_hull_spiral_line(const std::vector<cocos2d::Vec2> &points,std::vector<const cocos2d::Vec2*> &spiral_points);
/*
  *��ɢ�㼯�����������ʷ�
 */
void rotate_hull_spiral_decomposite(const std::vector<cocos2d::Vec2> &points, std::vector<const cocos2d::Vec2*> &triangle_edges);
NS_GT_END
#endif