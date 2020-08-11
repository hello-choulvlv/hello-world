/*
  *���·��+��ɼ���ͼ�㷨ʵ��
  *2020��8��7��
  *@author:xiaohuaxiong
 */
#ifndef __visibility_map_h__
#define __visibility_map_h__
#include "math/Vec2.h"
#include <vector>
#include "gt_common/geometry_types.h"
#include "data_struct/balance_tree.h"

NS_GT_BEGIN
struct rtSegment;
/*
  *����,ע��,��������ݳ���location_ptr֮��,���������п�����nullptr
 */
struct rtVertex {
	cocos2d::Vec2  *location_ptr;//Ŀ��������
	std::vector<cocos2d::Vec2> *owner_ptr;//�������ڵĶ���ζ�����������
	bool                      visible;//��¼��ɼ���,���Ժ���㷨����������,���ǽ���ʹ�õ��������
	rtSegment         *next_seg_ptr, *next_seg_ptr2;//next��ʾ��һ������ɨ�ӵ��߶�,�п��ܻ�������
	rtSegment			*prev_seg_ptr,*prev_seg_ptr2;//prev_seg��ʾ����/�Ѿ�������߶�,ע�ⶥ���ǰ���߶ο���������

	rtVertex(cocos2d::Vec2 *location_ptrc = nullptr,std::vector<cocos2d::Vec2> *owner_ptrc = nullptr) :location_ptr(location_ptrc), owner_ptr(owner_ptrc), visible(false),next_seg_ptr(nullptr), next_seg_ptr2(nullptr),prev_seg_ptr(nullptr), prev_seg_ptr2(nullptr){};
};
/*
  *�߶�
 */
struct rtSegment {
	rtVertex  *start_ptr, *dest_ptr;
	red_black_tree<rtSegment*>::internal_node *internal_ptr;//������ٲ���/��λ/ɾ��

	rtSegment(rtVertex *astart_ptr = nullptr,rtVertex *bdest_ptr = nullptr):start_ptr(astart_ptr),dest_ptr(bdest_ptr), internal_ptr(nullptr){};
};
/*
  *�ɼ���ͼ�㷨ʵ��
  *����Ķ���ζ������м��趼�ǰ�����ʱ������
 */
std::vector<rtVertex*>* rt_compute_visibility_map(std::vector<cocos2d::Vec2> *polygons,int  array_size,cocos2d::Vec2 &start_point,cocos2d::Vec2 &target_point);
/*
  *�Ͻ�˹�����㷨ʵ��
  *����Ȥ�Ķ��߿�������ʵ��,��Ϊ����һ��������/���Ϸǳ��꾡���㷨,������������ǾͲ���׸��
  *���㼸���㷨������й����������Ҫ������
  *�������������ӵĻ�,��ֻ�������ǵ�,���������мƻ�,ϵͳ�Ĺ���
  *����ҲҪ���Ա���,���ߵļ����㷨ѧϰ�ƻ�Ҳ�ս���
  *2020/8/11/17:35
 */
bool rt_dijkstra_algorithm(rtVertex *vertex_adj,int array_size,float *distance_array,int *path);
NS_GT_END
#endif