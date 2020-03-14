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

struct simple_polygon {

};
/*
  *��һ����,��������еĶ���ν���
  *�ڵڶ�����,���ǽ����еĽ��㰴˳����֯����
  *�����������еĶ��㰴��˳ʱ������,���Ҷ������û�п׶�
  *�ں���ĺ�����ʵ����,���ǽ�����չ�ú����Ĺ���.
  *����������ʵ�����򵥶���εĽ������㷨.
 */
bool simple_polygon_intersect(const std::vector<cocos2d::Vec2> &polygon1,const std::vector<cocos2d::Vec2>&polygon2,std::vector<cocos2d::Vec2> &intersect_array);
NS_GT_END

#endif