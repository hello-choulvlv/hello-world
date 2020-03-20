/*
  *�򵥶�����㷨,������,���͹�����,�򵥶���ε��㷨Ҫ���Ӻܶ�
  *2020��3��10��
  *@author:xiaoxiong
  *@version:1.0
 */
#ifndef __simple_polygon_h__
#define __simple_polygon_h__
#include "math/Vec2.h"
#include <vector>
#include<list>
#include "gt_common/geometry_types.h"
#include "data_struct/balance_tree.h"

NS_GT_BEGIN
enum SimpleEventType {
	EventType_Left = 0,//��ඥ��
	EventType_Right = 1,//�Ҳඥ��
	EventType_Interleave = 2,//�����
};
/*
  *Simple Polygon edge
 */
struct simple_edge {
	cocos2d::Vec2 origin_point,final_point ;
	short owner_idx,point_idx,next_idx;
};

struct simple_event {
	cocos2d::Vec2 event_point;
	SimpleEventType event_type;
	short owner_idx, point_idx,next_idx,queue_idx;
	//��������������߽�����������¼���,����Ҫָ����صı�,�ߵ�˳����ѭ�߶�״̬�и�����˳��
	//simple_edge *cross_edge1,*cross_edge2;
	red_black_tree<simple_edge>::internal_node *cross_edge1, *cross_edge2;
};
//�򵥶����֮��Ľ���
struct simple_interleave {
	cocos2d::Vec2 interleave_point;//����������
	short point_idx1,point_idx2;//�����Ķ�����Լ���ǰ��������ڵı�,point_idx-----point_idx+1
	short ref;//���ʱ�־0δ�����ʹ�,1�Ѿ������ʹ�
};

struct simple_interleave_ref {
	const simple_interleave *simple_ref;
	int	   point_idx;
};
/*
  *��һ����,��������еĶ���ν���
  *�ڵڶ�����,���ǽ����еĽ��㰴˳����֯����
  *�����������еĶ��㰴��˳ʱ������,���Ҷ������û�п׶�
  *�ں���ĺ�����ʵ����,���ǽ�����չ�ú����Ĺ���.
  *����������ʵ�����򵥶���εĽ������㷨.
 */
bool simple_polygon_intersect(const std::vector<cocos2d::Vec2> &polygon1,const std::vector<cocos2d::Vec2>&polygon2,std::vector<simple_interleave> &intersect_array);
/*
  *�������Ľ�������ݽṹת��Ϊ����εĽ���,ע���п��ܻ������ֹһ�������
  *�м�����ж�������,��ʹ������-1����
  *�������ζ������Ͷ����,���㷨������Ӽ�
  *���������ֻ����ͨ�ļ򵥶����
  *Ŀǰ���������㷨,ʵ���ϸ��������㷨�Ľ������,��ȫ����д����,����㷨
  *�����������˶����Լ�ʵ��,��̫�������.
 */
bool simple_polygon_interleave_set(const std::vector<cocos2d::Vec2>&, const std::vector<cocos2d::Vec2> &polygon2, const std::vector<simple_interleave>&,std::list<std::vector<cocos2d::Vec2>> &polygon_intersect_array);
/*
  *�򵥶������Ŀ���֮��İ�������
  *�㷨�ĺ���˼��Ϊ,��Ŀ���Ϊ��׼������������ֱ����
  *����simple polygon�ı����������ߵ��ཻ��Ŀ,��������ཻ����Ŀ��Ϊ����,���ʾ�ཻ
  *��������ζ�ŷ���
 */
bool simple_polygon_contains_point(const std::vector<cocos2d::Vec2>&simple_polygon,const cocos2d::Vec2 &target_point);
NS_GT_END

#endif