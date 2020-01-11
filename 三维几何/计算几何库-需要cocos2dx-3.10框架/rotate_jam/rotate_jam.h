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
NS_GT_END
#endif