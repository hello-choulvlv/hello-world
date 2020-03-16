/*
  *�򵥶���εĽ������㷨ʵ��
  *2020��3��13��
  *@author:xiaoxiong
  *@version:1.0
 */
#include "simple_polygon.h"
//#include "data_struct/balance_tree.h"
#include "data_struct/priority_queue.h"
#include "line/line.h"
#include "matrix/matrix.h"
#include <functional>
#include <math.h>
#include <assert.h>

using namespace cocos2d ;
NS_GT_BEGIN
/*
  *��ʼ��
 */
static void simple_polygon_init_event(const std::vector<cocos2d::Vec2> &polygon,short owner_idx,priority_queue<simple_event> &event_queue,std::function<bool (const simple_event&,const simple_event &)> &compare_func,std::function<int (simple_event &,int)> &queue_modify_func) {
	int array_size = polygon.size();
	const short invalid_value = -1;
	for (short j = 0; j < array_size; ++j) {
		const Vec2 &point = polygon[j];
		//����Ƿ�����/�Ҳ�ļ���
		short secondary_l = j?j-1:array_size-1;
		short tripple_l = j < array_size - 1?j+1:0;
		const Vec2 &prev_point = polygon[secondary_l];
		const Vec2 &next_point = polygon[tripple_l];

		if (point.x < prev_point.x && point.x < next_point.x) {
			simple_event origin_event = {
				polygon[j],SimpleEventType::EventType_Left,owner_idx,j,invalid_value,invalid_value,
			};
			event_queue.insert(origin_event, compare_func, queue_modify_func);
		}
		else if (point.x > prev_point.x && point.x > next_point.x) {
			simple_event origin_event = {
				polygon[j],SimpleEventType::EventType_Right,owner_idx,j,invalid_value,invalid_value,
			};
			event_queue.insert(origin_event, compare_func, queue_modify_func);
		}
		else {//��ʱ��Ҫ���������¼���
			simple_event origin_event = {
				polygon[j],SimpleEventType::EventType_Left,owner_idx,j,prev_point.x > next_point.x?secondary_l:tripple_l,invalid_value,
			};
			event_queue.insert(origin_event, compare_func, queue_modify_func);

			simple_event origin_event2 = {
				polygon[j],SimpleEventType::EventType_Right,owner_idx,prev_point.x < next_point.x?secondary_l:tripple_l,j,-1,
			};
			event_queue.insert(origin_event2, compare_func, queue_modify_func);
		}
	}
}

bool simple_polygon_intersect(const std::vector<cocos2d::Vec2> &polygon1, const std::vector<cocos2d::Vec2>&polygon2, std::vector<cocos2d::Vec2> &intersect_array) {
	//�����㷨���й�������Ҫʹ�õ������ݽṹ
	priority_queue<simple_event>  event_queue(polygon1.size() + polygon2.size() << 1);
	red_black_tree<simple_edge>   edge_status;
	//��׼�Ƚ������
	Vec2 compare_base_point,intersect_point;
	std::function<int(const simple_edge &, const simple_edge &)> edge_compare_func = [&compare_base_point](const simple_edge &a, const simple_edge &b)->int {
		//��x = compare_base_point.xΪ��׼��,���߶����׼�߽����y�����С,������ǳ�С,������ʱ��Ƚ����λ��
		float a_x = a.final_point.x - a.origin_point.x;
		float a_y = a.final_point.y - a.origin_point.y;
		float d = a_x != 0.0f ? 1.0f / a_x : 0.0f;
		float a_ly = a.origin_point.y + (compare_base_point.x - a.origin_point.x) * d * a_y;

		float b_x = b.final_point.x - b.origin_point.x;
		float b_y = b.final_point.y - b.origin_point.y;
		d = b_x != 0.0f ? 1.0f / b_x : 0.0f;
		float b_ly = b.origin_point.y + (compare_base_point.x - b.origin_point.x) * d * b_y;

		if (fabsf(a_ly - b_ly) > gt_eps) 
			return a_ly > b_ly ? 1 : -1;
		////��������˵������ֵ���,��ܴ�̶��Ͽ�����Ϊ���໥�νӵĶ˵�,��ʱ��Ҫ������ж�
		//bool b1 = false, b2 = false;
		//if (a.owner_idx == b.owner_idx && (b1 = a.final_point.equals(b.origin_point) || (b2 = b.final_point.equals(a.origin_point))))
		//	return b1?;
		//��������Խ���(tx,a_y)Ϊԭ������λ��
		compare_base_point.y = a_ly;
		float f = cross(compare_base_point,a.final_point + Vec2(a_x,a_y),b.final_point + Vec2(b_x,b_y));
		return f > 0.0f ? -1 : (f < 0.0f ? 1 : 0);
	};

	std::function<bool(const simple_event&, const simple_event&)> event_compare_func = [](const simple_event &a, const simple_event &b)->bool {
		if (a.event_point.x < b.event_point.x || a.event_point.x == b.event_point.x && a.event_point.y < b.event_point.y)
			return true;
		//����Ĵ�����Ի���
		bool b2 = a.event_point.equals(b.event_point);
		if (a.owner_idx == b.owner_idx && b2)
			return a.event_type == EventType_Right;

		return b2 && a.event_type < b.event_type;
	};

	std::function<int(simple_event &, int)> modify_func = [](simple_event &event, int queue_idx)->int {
		if (queue_idx != -1)
			event.queue_idx = queue_idx;
		return event.queue_idx;
	};
	//��ʼ��������¼���
	simple_polygon_init_event(polygon1, 0, event_queue, event_compare_func, modify_func);
	simple_polygon_init_event(polygon2, 1, event_queue, event_compare_func, modify_func);

	struct polygons_info {
		const std::vector<cocos2d::Vec2> &polygon;
		int polygon_size;
	}polygon_array[2] = {{polygon1,polygon1.size()},{polygon2,polygon2.size()}};
	const short invalid_idx = 0;

	int loops = 0;
	while (event_queue.size()) {
		++loops;
		//���ͷ
		const simple_event target_event = event_queue.head();
		if (target_event.point_idx == 3 && target_event.event_type == EventType_Left) {
			int x = 0;
			int y = 0;
		}
		event_queue.remove_head(event_compare_func, modify_func);
		const polygons_info &target_polygon = polygon_array[target_event.owner_idx];
		short prev_idx = target_event.point_idx ? target_event.point_idx - 1 : target_polygon.polygon_size - 1;
		short next_idx = target_event.point_idx < target_polygon.polygon_size - 1 ? target_event.point_idx+1:0;
		const Vec2 &prev_point = target_polygon.polygon[prev_idx];
		const Vec2 &next_point = target_polygon.polygon[next_idx];
		const Vec2 &now_point = target_event.event_point;
		//��������ļ���,��ʱ�õ�����ͱ�ȻΪ����¼���
		bool b_need_remove = true;
		compare_base_point = now_point;
		if (target_event.event_type == EventType_Left && now_point.x < prev_point.x && now_point.x < next_point.x) {
			//assert(target_event.event_type == EventType_Left);
			//������¼�����ص������߲��뵽ƽ������
			simple_edge edge1 = {
				now_point,prev_point,target_event.owner_idx,target_event.point_idx,prev_idx,
			};
			auto *edge1_ptr = edge_status.insert(edge1, edge_compare_func);

			simple_edge edge2 = {
				now_point,next_point,target_event.owner_idx,target_event.point_idx,next_idx,
			};
			auto *edge2_ptr = edge_status.insert(edge2,edge_compare_func);
			//��Ŀ��ߵ����²�ı߽��бȽ�,����еĻ�
			auto *above_ptr = edge_status.find_next(edge1_ptr);
			if (above_ptr && above_ptr->tw_value.owner_idx != edge1_ptr->tw_value.owner_idx) {
				//�󽻵�
				simple_edge &target_edge = above_ptr->tw_value;
				bool b_interleave = segment_segment_intersect_test(target_edge.origin_point, target_edge.final_point,edge1.origin_point,edge1.final_point,intersect_point);
				//��ʱ�����γ��µĽ���,���ض�ԭ�����߶�
				if (b_interleave) {
					simple_event interleave_event = {
						intersect_point,
						SimpleEventType::EventType_Interleave,
						target_event.owner_idx,
						target_event.point_idx,target_event.next_idx,
						invalid_idx,
						above_ptr,
						edge1_ptr,
					};
					event_queue.insert(interleave_event, event_compare_func,modify_func);
				}
			}

			auto *below_ptr = edge_status.find_prev(edge1_ptr);
			if (below_ptr && below_ptr->tw_value.owner_idx != edge1_ptr->tw_value.owner_idx) {
				simple_edge &target_edge = below_ptr->tw_value;
				bool b_interleave = segment_segment_intersect_test(target_edge.origin_point, target_edge.final_point, edge1.origin_point, edge1.final_point, intersect_point);
				//��ʱ�����γ��µĽ���,���ض�ԭ�����߶�
				if (b_interleave) {
					simple_event interleave_event = {
						intersect_point,
						SimpleEventType::EventType_Interleave,
						target_event.owner_idx,
						target_event.point_idx,target_event.next_idx,
						invalid_idx,
						edge1_ptr,
						below_ptr,
					};
					event_queue.insert(interleave_event, event_compare_func, modify_func);
				}
			}
			//edge2������
			above_ptr = edge_status.find_next(edge2_ptr);
			if (above_ptr && above_ptr->tw_value.owner_idx != edge2_ptr->tw_value.owner_idx) {
				//�󽻵�
				simple_edge &target_edge = above_ptr->tw_value;
				bool b_interleave = segment_segment_intersect_test(target_edge.origin_point, target_edge.final_point, edge2.origin_point, edge2.final_point, intersect_point);
				//��ʱ�����γ��µĽ���,���ض�ԭ�����߶�
				if (b_interleave) {
					simple_event interleave_event = {
						intersect_point,
						SimpleEventType::EventType_Interleave,
						target_event.owner_idx,
						target_event.point_idx,target_event.next_idx,
						invalid_idx,
						above_ptr,
						edge2_ptr,
					};
					event_queue.insert(interleave_event, event_compare_func, modify_func);
				}
			}
			below_ptr = edge_status.find_prev(edge2_ptr);
			if (below_ptr && below_ptr->tw_value.owner_idx != edge2_ptr->tw_value.owner_idx) {
				simple_edge &target_edge = below_ptr->tw_value;
				bool b_interleave = segment_segment_intersect_test(target_edge.origin_point, target_edge.final_point, edge2.origin_point, edge2.final_point, intersect_point);
				//��ʱ�����γ��µĽ���,���ض�ԭ�����߶�
				if (b_interleave) {
					simple_event interleave_event = {
						intersect_point,
						SimpleEventType::EventType_Interleave,
						target_event.owner_idx,
						target_event.point_idx,target_event.next_idx,
						invalid_idx,
						edge2_ptr,
						below_ptr,
					};
					event_queue.insert(interleave_event, event_compare_func, modify_func);
				}
			}
			/////////
		}
		else if (target_event.event_type == EventType_Right && now_point.x > prev_point.x && now_point.x > next_point.x) {//������Ҳ�˵�
			//��ʱ��Ҫ���߶�״̬���Ƴ�������صĶ˵������ӵ������߶�
			simple_edge secondary_edge = {prev_point,now_point};
			edge_status.remove(secondary_edge, edge_compare_func);

			secondary_edge.origin_point = next_point;
			edge_status.remove(secondary_edge,edge_compare_func);
		}
		else if (target_event.event_type == EventType_Left) {//�������ͨ�����˵�
			simple_edge edge1 = {
				target_polygon.polygon[target_event.point_idx],target_polygon.polygon[target_event.next_idx],
				target_event.owner_idx,target_event.point_idx,target_event.next_idx,
			};
			auto *edge_ptr = edge_status.insert(edge1, edge_compare_func);

			auto *above_ptr = edge_status.find_next(edge_ptr);
			if (above_ptr && above_ptr->tw_value.owner_idx != target_event.owner_idx) {
				bool b_interleave = segment_segment_intersect_test(edge1.origin_point,edge1.final_point,above_ptr->tw_value.origin_point,above_ptr->tw_value.final_point,intersect_point);
				if (b_interleave) {
					simple_event interleave_event = {
						intersect_point,
						SimpleEventType::EventType_Interleave,
						target_event.owner_idx,
						target_event.point_idx,target_event.next_idx,
						invalid_idx,
						above_ptr,
						edge_ptr,
					};
					event_queue.insert(interleave_event, event_compare_func, modify_func);
				}
			}

			auto *below_ptr = edge_status.find_prev(edge_ptr);
			if (below_ptr && below_ptr->tw_value.owner_idx != target_event.owner_idx) {
				bool b_interleave = segment_segment_intersect_test(edge1.origin_point, edge1.final_point, below_ptr->tw_value.origin_point, below_ptr->tw_value.final_point, intersect_point);
				if (b_interleave) {
					simple_event interleave_event = {
						intersect_point,
						SimpleEventType::EventType_Interleave,
						target_event.owner_idx,
						target_event.point_idx,target_event.next_idx,
						invalid_idx,
						edge_ptr,
						below_ptr,
					};
					event_queue.insert(interleave_event, event_compare_func, modify_func);
				}
			}
		}
		else if (target_event.event_type == EventType_Right) {//�Ҳ�˵�,��ʱɾ����֮�����ı߼���
			simple_edge target_edge = {
				target_polygon.polygon[target_event.point_idx],
				target_polygon.polygon[target_event.next_idx],
			};
			edge_status.remove(target_edge, edge_compare_func);
		}
		else {
			//����ǽ�����¼�,��ʱ��Ҫ��¼��ؽ���
			assert(target_event.event_type == EventType_Interleave);
			vector_fast_push_back(intersect_array,target_event.event_point);
			//ͬʱ��Ҫ�޸�������رߵ����,�Լ����������ߵ�λ��
			auto *edge1_ptr = target_event.cross_edge1, *edge2_ptr = target_event.cross_edge2;
			edge1_ptr->tw_value.origin_point = target_event.event_point;
			edge2_ptr->tw_value.origin_point = target_event.event_point;

			simple_edge t = edge1_ptr->tw_value;
			edge1_ptr->tw_value = edge2_ptr->tw_value;
			edge2_ptr->tw_value = t;
			//���¼���µ��ٱ�,ԭ��edge1_ptr��λ����edge2_ptr֮��,�������߽�����
			auto *below_ptr = edge_status.find_prev(edge1_ptr);//below_ptr != nullptr
			assert(below_ptr != nullptr);
			if (below_ptr != edge2_ptr && below_ptr->tw_value.owner_idx != edge1_ptr->tw_value.owner_idx) {
				bool b_interleave = segment_segment_intersect_test(below_ptr->tw_value.origin_point,below_ptr->tw_value.final_point,edge1_ptr->tw_value.origin_point,edge1_ptr->tw_value.final_point,intersect_point);
				if (b_interleave) {
					simple_event interleave_event = {
						intersect_point,
						SimpleEventType::EventType_Interleave,
						target_event.owner_idx,
						target_event.point_idx,target_event.next_idx,
						invalid_idx,
						edge1_ptr,
						below_ptr,
					};
					event_queue.insert(interleave_event,event_compare_func,modify_func);
				}
			}

			auto *above_ptr = edge_status.find_next(edge1_ptr);//assert(above_ptr != nullptr)
			assert(above_ptr != edge2_ptr);
			if (above_ptr && above_ptr != edge2_ptr && above_ptr->tw_value.owner_idx != edge1_ptr->tw_value.owner_idx) {
				bool b_interleave = segment_segment_intersect_test(above_ptr->tw_value.origin_point, above_ptr->tw_value.final_point, edge1_ptr->tw_value.origin_point, edge1_ptr->tw_value.final_point, intersect_point);
				if (b_interleave) {
					simple_event interleave_event = {
						intersect_point,
						SimpleEventType::EventType_Interleave,
						target_event.owner_idx,
						target_event.point_idx,target_event.next_idx,
						invalid_idx,
						above_ptr,
						edge1_ptr,
					};
					event_queue.insert(interleave_event, event_compare_func, modify_func);
				}
			}
			//��һ����
			above_ptr = edge_status.find_next(edge2_ptr);//�ϲ�
			assert(above_ptr != nullptr);
			if (above_ptr != edge1_ptr && above_ptr->tw_value.owner_idx != edge2_ptr->tw_value.owner_idx) {
				bool b_interleave = segment_segment_intersect_test(above_ptr->tw_value.origin_point, above_ptr->tw_value.final_point, edge2_ptr->tw_value.origin_point, edge2_ptr->tw_value.final_point, intersect_point);
				if (b_interleave) {
					simple_event interleave_event = {
						intersect_point,
						SimpleEventType::EventType_Interleave,
						target_event.owner_idx,
						target_event.point_idx,target_event.next_idx,
						invalid_idx,
						above_ptr,
						edge2_ptr,
					};
					event_queue.insert(interleave_event, event_compare_func, modify_func);
				}
			}
			//edge2_ptr���²�
			below_ptr = edge_status.find_prev(edge2_ptr);
			assert(below_ptr != edge1_ptr);
			if (below_ptr && below_ptr->tw_value.owner_idx != edge2_ptr->tw_value.owner_idx) {
				bool b_interleave = segment_segment_intersect_test(below_ptr->tw_value.origin_point, below_ptr->tw_value.final_point, edge2_ptr->tw_value.origin_point, edge2_ptr->tw_value.final_point, intersect_point);
				if (b_interleave) {
					simple_event interleave_event = {
						intersect_point,
						SimpleEventType::EventType_Interleave,
						target_event.owner_idx,
						target_event.point_idx,target_event.next_idx,
						invalid_idx,
						edge2_ptr,
						below_ptr,
					};
					event_queue.insert(interleave_event, event_compare_func, modify_func);
				}
			}
		}
	}
	return intersect_array.size() > 0;
}

NS_GT_END