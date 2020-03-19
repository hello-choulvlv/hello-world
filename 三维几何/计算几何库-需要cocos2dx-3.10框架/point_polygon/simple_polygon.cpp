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
#include <map>

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

bool simple_polygon_intersect(const std::vector<cocos2d::Vec2> &polygon1, const std::vector<cocos2d::Vec2>&polygon2, std::vector<simple_interleave> &intersect_array) {
	//�����㷨���й�������Ҫʹ�õ������ݽṹ
	priority_queue<simple_event>  event_queue(polygon1.size() + polygon2.size() << 1);
	red_black_tree<simple_edge>   edge_status;
	//��׼�Ƚ������
	Vec2 compare_base_point,intersect_point;
	std::function<int(const simple_edge &, const simple_edge &)> edge_compare_func = [&compare_base_point](const simple_edge &a, const simple_edge &b)->int {
		assert(compare_base_point.x >= a.origin_point.x && compare_base_point.x <= a.final_point.x);
		assert(compare_base_point.x >=b.origin_point.x && compare_base_point.x <= b.final_point.x);
		if (a.owner_idx == b.owner_idx && a.point_idx == b.point_idx && a.next_idx == b.next_idx)
			return 0;
		//��x = compare_base_point.xΪ��׼��,���߶����׼�߽����y�����С,������ǳ�С,������ʱ��Ƚ����λ��
		float a_x = a.final_point.x - a.origin_point.x;
		float a_y = a.final_point.y - a.origin_point.y;
		float d = a_x != 0.0f ? 1.0f / a_x : 0.0f;
		float a_ly = a.origin_point.y + (compare_base_point.x - a.origin_point.x) * d * a_y;

		float b_x = b.final_point.x - b.origin_point.x;
		float b_y = b.final_point.y - b.origin_point.y;
		d = b_x != 0.0f ? 1.0f / b_x : 0.0f;
		float b_ly = b.origin_point.y + (compare_base_point.x - b.origin_point.x) * d * b_y;

		if (fabsf(a_ly - b_ly) >= gt_eps)
			return a_ly > b_ly ? 1 : -1;
		//�Ƚ�,˭��б�ʸ�����+y��
		bool b2 = compare_base_point.x == a.final_point.x && compare_base_point.x == b.final_point.x;
		float f = cross_normalize(Vec2(a_x, a_y), Vec2(b_x, b_y));
		if (fabsf(f) <= gt_eps)return 0;

		return f*(b2?-1:1) > gt_eps ? -1 : 1;
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

	while (event_queue.size()) {
		//���ͷ
		const simple_event target_event = event_queue.head();
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
				if (b_interleave && intersect_point.x >= compare_base_point.x) {
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
				if (b_interleave && intersect_point.x >= compare_base_point.x) {
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
				if (b_interleave && intersect_point.x >= compare_base_point.x) {
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
				if (b_interleave && intersect_point.x >= compare_base_point.x) {
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
			//��ʱ��Ҫ���߶�״̬���Ƴ�������صĶ˵������ӵ������߶�,ɾ��֮�󽫻�ֲ��ı�ԭ���߶�֮��Ĺ�ϵ,�����Ҫ���Ӷ�����ж�
			simple_edge secondary_edge = {prev_point,now_point,target_event.owner_idx,prev_idx,target_event.point_idx};
			auto *edge_ptr = edge_status.lookup(secondary_edge, edge_compare_func);
			assert(edge_ptr != nullptr);

			secondary_edge.point_idx = next_idx;
			secondary_edge.origin_point = next_point;
			auto *edge2_ptr = edge_status.lookup(secondary_edge, edge_compare_func);
			assert(edge2_ptr != nullptr);

			//��Ҫ��������ߵ����λ��
			float f = cross_normalize(now_point - prev_point, now_point - next_point);
			bool b2 =  f > 0.0f;//�Ƿ����²�

			auto *above_ptr = edge_status.find_next(b2 ?edge_ptr:edge2_ptr);
			auto *below_ptr = edge_status.find_prev(!b2 ?edge_ptr:edge2_ptr);
			 
			edge_status.remove_case(edge_ptr);
			edge_status.remove_case(edge2_ptr);
			//�������
			if (above_ptr && below_ptr && above_ptr->tw_value.owner_idx != below_ptr->tw_value.owner_idx) {
				bool b_interleave = segment_segment_intersect_test(above_ptr->tw_value.origin_point,above_ptr->tw_value.final_point,below_ptr->tw_value.origin_point,below_ptr->tw_value.final_point,intersect_point);
				if (b_interleave && intersect_point.x > compare_base_point.x) {
					simple_event event_point = {
						intersect_point,
						SimpleEventType::EventType_Interleave,
						target_event.owner_idx,
						target_event.point_idx,target_event.next_idx,
						invalid_idx,
						above_ptr,
						below_ptr,
					};
					event_queue.insert(event_point, event_compare_func, modify_func);
				}
			}
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
				if (b_interleave && intersect_point.x >= compare_base_point.x) {
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
				if (b_interleave && intersect_point.x >= compare_base_point.x) {
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
				target_event.owner_idx,
				target_event.point_idx,target_event.next_idx,
			};
			auto *edge_ptr = edge_status.lookup(target_edge, edge_compare_func);
			assert(edge_ptr != nullptr);
			//�ҳ������ھ�
			auto *above_ptr = edge_status.find_next(edge_ptr);
			auto *below_ptr = edge_status.find_prev(edge_ptr);
			if (above_ptr && below_ptr && above_ptr->tw_value.owner_idx != below_ptr->tw_value.owner_idx) {
				bool b_interleave = segment_segment_intersect_test(above_ptr->tw_value.origin_point,above_ptr->tw_value.final_point,below_ptr->tw_value.origin_point,below_ptr->tw_value.final_point,intersect_point);
				if (b_interleave && intersect_point.x > compare_base_point.x) {
					simple_event event_point = {
						intersect_point,
						SimpleEventType::EventType_Interleave,
						target_event.owner_idx,
						target_event.point_idx,target_event.next_idx,
						invalid_idx,
						above_ptr,
						below_ptr,
					};
					event_queue.insert(event_point, event_compare_func, modify_func);
				}
			}
			edge_status.remove_case(edge_ptr);
		}
		else {
			//����ǽ�����¼�,��ʱ��Ҫ��¼��ؽ���
			assert(target_event.event_type == EventType_Interleave);
			auto *edge1_ptr = target_event.cross_edge1, *edge2_ptr = target_event.cross_edge2;
			auto &edge1_ref = !edge1_ptr->tw_value.owner_idx ? edge1_ptr->tw_value:edge2_ptr->tw_value;
			auto &edge2_ref = edge2_ptr->tw_value.owner_idx ? edge2_ptr->tw_value : edge1_ptr->tw_value;

			int s = min_f(edge1_ref.point_idx, edge1_ref.next_idx), t3 = max_f(edge1_ref.point_idx, edge1_ref.next_idx);
			if (s == 0 && t3 == polygon1.size() - 1) s = t3;
			int x = min_f(edge2_ref.point_idx, edge2_ref.next_idx), y = max_f(edge2_ref.point_idx, edge2_ref.next_idx);
			if (x == 0 && y == polygon2.size() - 1) x = y;

			simple_interleave interleave_point = {
				target_event.event_point,s,x, 
			};
			vector_fast_push_back(intersect_array, interleave_point);
			//ͬʱ��Ҫ�޸�������رߵ����,�Լ����������ߵ�λ��
			edge1_ptr->tw_value.origin_point = target_event.event_point;
			edge2_ptr->tw_value.origin_point = target_event.event_point;
			//ʵ����,�ô������ǿ����Ż���,ǰ���Ƕ�red_black_tree�Ĵ���ṹ�ǳ��˽�
			//ע��,��صı߶��������ɾ��Ȼ�����µ���,������ֱ�ӵĽ�������,��Ϊ���������
			//֮��,��ʹ��������ض���,����������λ��Ҳ��һ������ȷ��,���п����ƻ�red_black_tree������ṹ
			simple_edge &t = edge1_ptr->tw_value;
			edge_status.remove_case(edge1_ptr);
			edge1_ptr = edge_status.insert(t, edge_compare_func);

			simple_edge &t2 = edge2_ptr->tw_value;
			edge_status.remove_case(edge2_ptr);
			edge2_ptr = edge_status.insert(t2, edge_compare_func);
			//���¼���µ��ٱ�,ԭ��edge1_ptr��λ����edge2_ptr֮��,�������߽�����
			auto *below_ptr = edge_status.find_prev(edge2_ptr);//below_ptr != nullptr
			assert(below_ptr != nullptr);
			if (below_ptr != edge1_ptr && below_ptr->tw_value.owner_idx != edge2_ptr->tw_value.owner_idx) {
				bool b_interleave = segment_segment_intersect_test(below_ptr->tw_value.origin_point,below_ptr->tw_value.final_point, edge2_ptr->tw_value.origin_point, edge2_ptr->tw_value.final_point,intersect_point);
				if (b_interleave && intersect_point.x >= compare_base_point.x) {
					simple_event interleave_event = {
						intersect_point,
						SimpleEventType::EventType_Interleave,
						target_event.owner_idx,
						target_event.point_idx,target_event.next_idx,
						invalid_idx,
						edge2_ptr,
						below_ptr,
					};
					event_queue.insert(interleave_event,event_compare_func,modify_func);
				}
			}

			auto *above_ptr = edge_status.find_next(edge2_ptr);//assert(above_ptr != nullptr)
			assert(above_ptr != edge1_ptr);
			if (above_ptr && above_ptr->tw_value.owner_idx != edge2_ptr->tw_value.owner_idx) {
				bool b_interleave = segment_segment_intersect_test(above_ptr->tw_value.origin_point, above_ptr->tw_value.final_point, edge2_ptr->tw_value.origin_point, edge2_ptr->tw_value.final_point, intersect_point);
				if (b_interleave && intersect_point.x >= compare_base_point.x) {
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
			//��һ����
			above_ptr = edge_status.find_next(edge1_ptr);//�ϲ�
			assert(above_ptr != nullptr);
			if (above_ptr != edge2_ptr && above_ptr->tw_value.owner_idx != edge1_ptr->tw_value.owner_idx) {
				bool b_interleave = segment_segment_intersect_test(above_ptr->tw_value.origin_point, above_ptr->tw_value.final_point, edge1_ptr->tw_value.origin_point, edge1_ptr->tw_value.final_point, intersect_point);
				if (b_interleave && intersect_point.x >= compare_base_point.x) {
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
			//edge2_ptr���²�
			below_ptr = edge_status.find_prev(edge1_ptr);
			assert(below_ptr != edge2_ptr);
			if (below_ptr != nullptr && below_ptr->tw_value.owner_idx != edge1_ptr->tw_value.owner_idx) {
				bool b_interleave = segment_segment_intersect_test(below_ptr->tw_value.origin_point, below_ptr->tw_value.final_point, edge1_ptr->tw_value.origin_point, edge1_ptr->tw_value.final_point, intersect_point);
				if (b_interleave && intersect_point.x >= compare_base_point.x) {
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
		}
	}
	return intersect_array.size() > 0;
}

bool simple_polygon_interleave_set(const std::vector<cocos2d::Vec2>&polygon1,const std::vector<cocos2d::Vec2> &polygon2, const std::vector<simple_interleave>&interleave_array, std::list<std::vector<cocos2d::Vec2>> &polygon_intersect_array) {
	//ע�����û���κεĽ���,Ҫô����ȫ�ķ���,Ҫô��һ���������ȫ��������һ�����ڲ�
	//��������������Ժ����

	//��һ���Զ������ͳ��
	std::map<int, std::vector<simple_interleave_ref>> polygon_edge1;
	std::map<int, std::vector<simple_interleave_ref>> polygon_edge2;
	std::map<int, simple_interleave> polygon_edge;

	Vec2 base_point;
	std::function<bool(const simple_interleave_ref &,const simple_interleave_ref &)> compare_func = [&base_point](const simple_interleave_ref &a, const simple_interleave_ref &b)->bool {
		return length2(a.simple_ref->interleave_point,base_point) < length2(b.simple_ref->interleave_point,base_point);
	};

	std::function<bool(const simple_interleave_ref &,const simple_interleave_ref &)> equal_func = [](const simple_interleave_ref&a, const simple_interleave_ref&b)->bool {
		return a.simple_ref->point_idx1 == b.simple_ref->point_idx1 
			&&a.simple_ref->point_idx2 == b.simple_ref->point_idx2 
			&& a.point_idx == b.point_idx;
	};

#define map_key(x,y) (((y)<<16)+x)
	for (int j = 0; j < interleave_array.size(); ++j) {
		const simple_interleave &interleave_point = interleave_array[j];
		simple_interleave_ref interleave_ref = {
			&interleave_point,interleave_point.point_idx2,
		};

		auto it = polygon_edge1.find(interleave_point.point_idx1);
		std::vector<simple_interleave_ref> *array_ptr = nullptr;
		if (it == polygon_edge1.end())
			array_ptr = &(polygon_edge1[interleave_point.point_idx1] = std::vector<simple_interleave_ref>());
		else
			array_ptr = &it->second;

		base_point = polygon1[interleave_point.point_idx1];
		if (check_array_same_elem<simple_interleave_ref>(array_ptr->data(), (int)array_ptr->size(),interleave_ref,equal_func) == -1)
			vector_fast_push_back((*array_ptr), interleave_ref);
		insert_sort<simple_interleave_ref>(array_ptr->data(),(int)array_ptr->size(),compare_func);
		//////////////////////////////////////////////////////////////////////////
		interleave_ref.point_idx = interleave_point.point_idx1;
		it = polygon_edge2.find(interleave_point.point_idx2);
		array_ptr = nullptr;
		if (it == polygon_edge2.end())
			array_ptr = &(polygon_edge2[interleave_point.point_idx2] = std::vector<simple_interleave_ref>());
		else
			array_ptr = &it->second;

		base_point = polygon2[interleave_point.point_idx2];
		if(check_array_same_elem<simple_interleave_ref>(array_ptr->data(),(int)array_ptr->size(),interleave_ref,equal_func) == -1)
			vector_fast_push_back((*array_ptr), interleave_ref);
		insert_sort<simple_interleave_ref>(array_ptr->data(), (int)array_ptr->size(), compare_func);

		int key_code = map_key(interleave_point.point_idx1,interleave_point.point_idx2);
		if (polygon_edge.find(key_code) == polygon_edge.end()) {
			auto &st = polygon_edge[key_code] = interleave_point;
			//���ϱ�־,��ʾδ�����ʹ�
			st.ref = 0;
		}
	}
	//�ڶ���,��߱���,�����Ĺ����е㸴��

	if (!polygon_edge.size())return false;
	std::function<int(const std::vector<simple_interleave_ref> &, short)> lookup_func = [](const std::vector<simple_interleave_ref> &ref_array, short target_idx)->int {
		int array_size = ref_array.size();
		for (int j = 0; j < array_size; ++j) {
			if (ref_array.at(j).point_idx == target_idx) 
				return j < array_size - 1 ? ref_array.at(j+1).point_idx : -1;
		}
		return -1;
	};

	for (auto it = polygon_edge.begin(); it != polygon_edge.end(); ++it) {
		if (it->second.ref)continue;
		std::vector<Vec2>  polygon;
		bool b_found = true;
		int array_size1 = polygon1.size();
		int array_size2 = polygon2.size();
		//�����㷨�ĺ���˼�������ö���ε�����������ֻ��һ�����������
		const simple_interleave &interleave_st = it->second;
		float f = cross(interleave_st.interleave_point, polygon1[(interleave_st.point_idx1 + 1) % array_size1], polygon2[(interleave_st.point_idx2 + 1) % array_size2]);
		bool b2 = f < 0.0f;
		//���f < 0.0,���ʾ���������ԭ���ı߼���ǰ��,���򽻻�˳��
		std::map<int, std::vector<simple_interleave_ref>> *other_edge_ptr = b2 ? &polygon_edge1 : &polygon_edge2;
		std::map<int, std::vector<simple_interleave_ref>> *tripple_edge_ptr = b2 ? &polygon_edge2 : &polygon_edge1;

		const std::vector<Vec2> *polygon1_ptr = b2 ? &polygon1 : &polygon2;
		const std::vector<Vec2> *polygon2_ptr = b2 ? &polygon2 : &polygon1;

		int point_idx1 = b2 ? it->second.point_idx1 : it->second.point_idx2;
		int point_idx2 = b2 ? it->second.point_idx2 : it->second.point_idx1;

		if (!b2) {
			array_size1 = polygon2.size();
		}
		auto it2 = it;
		bool b3 = b2;
		int point_idx = point_idx1;
		while (b_found) {
			vector_fast_push_back(polygon, it2->second.interleave_point);
			it2->second.ref = 1;
			const std::vector<simple_interleave_ref> &ref_array = (*other_edge_ptr)[point_idx1];
			int lookup_idx = lookup_func(ref_array, point_idx2);
			//ѭ�����������Ķ���
			while (lookup_idx == -1) {
				point_idx1 = (point_idx1 + 1)%array_size1;
				vector_fast_push_back(polygon, (*polygon1_ptr)[point_idx1%array_size1]);
				auto st2 = other_edge_ptr->find(point_idx1);
				lookup_idx = st2 == other_edge_ptr->end() ? -1 : st2->second[0].point_idx;
			}
			b2 = !b2;
			//��⵱ǰ�Ƿ�ص���ԭ��
			point_idx2 = point_idx1;
			point_idx1 = lookup_idx;
			if (b2 == b3 && lookup_idx == point_idx)
				break;
			//�����µĽ����
			int key_code = b2?map_key(point_idx1, point_idx2):map_key(point_idx2,point_idx1);
			it2 = polygon_edge.find(key_code);
			//��������ĳЩ���ݽṹ
			other_edge_ptr = b2 ? &polygon_edge1 : &polygon_edge2;
			polygon1_ptr = b2 ? &polygon1 : &polygon2;
			array_size1 = b2 ? polygon1.size() : polygon2.size();
		}
		polygon_intersect_array.push_back(polygon);
	}
#undef map_key
	return true;
}
NS_GT_END