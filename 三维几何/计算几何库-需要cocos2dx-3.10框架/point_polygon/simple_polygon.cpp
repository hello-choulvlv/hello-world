/*
  *简单多边形的交并差算法实现
  *2020年3月13日
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
  *初始化
 */
static void simple_polygon_init_event(const std::vector<cocos2d::Vec2> &polygon,short owner_idx,priority_queue<simple_event> &event_queue,std::function<bool (const simple_event&,const simple_event &)> &compare_func,std::function<int (simple_event &,int)> &queue_modify_func) {
	int array_size = polygon.size();
	const short invalid_value = -1;
	for (short j = 0; j < array_size; ++j) {
		const Vec2 &point = polygon[j];
		//检测是否是左/右侧的极点
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
		else {//此时需要生成两个事件点
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
	//建立算法运行过程中需要使用到的数据结构
	priority_queue<simple_event>  event_queue(polygon1.size() + polygon2.size() << 1);
	red_black_tree<simple_edge>   edge_status;
	//标准比较坐标点
	Vec2 compare_base_point,intersect_point;
	std::function<int(const simple_edge &, const simple_edge &)> edge_compare_func = [&compare_base_point](const simple_edge &a, const simple_edge &b)->int {
		assert(compare_base_point.x >= a.origin_point.x && compare_base_point.x <= a.final_point.x);
		assert(compare_base_point.x >=b.origin_point.x && compare_base_point.x <= b.final_point.x);
		if (a.owner_idx == b.owner_idx && a.point_idx == b.point_idx && a.next_idx == b.next_idx)
			return 0;
		//以x = compare_base_point.x为基准线,求线段与基准线交点的y坐标大小,如果相差非常小,则沿逆时针比较相对位置
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
		//比较,谁的斜率更靠近+y轴
		bool b2 = compare_base_point.x == a.final_point.x && compare_base_point.x == b.final_point.x;
		float f = cross_normalize(Vec2(a_x, a_y), Vec2(b_x, b_y));
		if (fabsf(f) <= gt_eps)return 0;

		return f*(b2?-1:1) > gt_eps ? -1 : 1;
	};

	std::function<bool(const simple_event&, const simple_event&)> event_compare_func = [](const simple_event &a, const simple_event &b)->bool {
		if (a.event_point.x < b.event_point.x || a.event_point.x == b.event_point.x && a.event_point.y < b.event_point.y)
			return true;
		//下面的代码可以化简
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
	//初始化多边形事件点
	simple_polygon_init_event(polygon1, 0, event_queue, event_compare_func, modify_func);
	simple_polygon_init_event(polygon2, 1, event_queue, event_compare_func, modify_func);

	struct polygons_info {
		const std::vector<cocos2d::Vec2> &polygon;
		int polygon_size;
	}polygon_array[2] = {{polygon1,polygon1.size()},{polygon2,polygon2.size()}};
	const short invalid_idx = 0;

	while (event_queue.size()) {
		//检查头
		const simple_event target_event = event_queue.head();
		event_queue.remove_head(event_compare_func, modify_func);
		const polygons_info &target_polygon = polygon_array[target_event.owner_idx];
		short prev_idx = target_event.point_idx ? target_event.point_idx - 1 : target_polygon.polygon_size - 1;
		short next_idx = target_event.point_idx < target_polygon.polygon_size - 1 ? target_event.point_idx+1:0;
		const Vec2 &prev_point = target_polygon.polygon[prev_idx];
		const Vec2 &next_point = target_polygon.polygon[next_idx];
		const Vec2 &now_point = target_event.event_point;
		//如果是左侧的极点,此时该点的类型必然为左侧事件点
		bool b_need_remove = true;
		compare_base_point = now_point;
		if (target_event.event_type == EventType_Left && now_point.x < prev_point.x && now_point.x < next_point.x) {
			//assert(target_event.event_type == EventType_Left);
			//将与该事件点相关的两条边插入到平衡树中
			simple_edge edge1 = {
				now_point,prev_point,target_event.owner_idx,target_event.point_idx,prev_idx,
			};
			auto *edge1_ptr = edge_status.insert(edge1, edge_compare_func);

			simple_edge edge2 = {
				now_point,next_point,target_event.owner_idx,target_event.point_idx,next_idx,
			};
			auto *edge2_ptr = edge_status.insert(edge2,edge_compare_func);
			//将目标边的上下侧的边进行比较,如果有的话
			auto *above_ptr = edge_status.find_next(edge1_ptr);
			if (above_ptr && above_ptr->tw_value.owner_idx != edge1_ptr->tw_value.owner_idx) {
				//求交点
				simple_edge &target_edge = above_ptr->tw_value;
				bool b_interleave = segment_segment_intersect_test(target_edge.origin_point, target_edge.final_point,edge1.origin_point,edge1.final_point,intersect_point);
				//此时将会形成新的交点,并截断原来的线段
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
				//此时将会形成新的交点,并截断原来的线段
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
			//edge2边数据
			above_ptr = edge_status.find_next(edge2_ptr);
			if (above_ptr && above_ptr->tw_value.owner_idx != edge2_ptr->tw_value.owner_idx) {
				//求交点
				simple_edge &target_edge = above_ptr->tw_value;
				bool b_interleave = segment_segment_intersect_test(target_edge.origin_point, target_edge.final_point, edge2.origin_point, edge2.final_point, intersect_point);
				//此时将会形成新的交点,并截断原来的线段
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
				//此时将会形成新的交点,并截断原来的线段
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
		else if (target_event.event_type == EventType_Right && now_point.x > prev_point.x && now_point.x > next_point.x) {//如果是右侧端点
			//此时需要从线段状态中移除掉与相关的端点相连接的两条线段,删除之后将会局部改变原来线段之间的关系,因此需要增加额外的判断
			simple_edge secondary_edge = {prev_point,now_point,target_event.owner_idx,prev_idx,target_event.point_idx};
			auto *edge_ptr = edge_status.lookup(secondary_edge, edge_compare_func);
			assert(edge_ptr != nullptr);

			secondary_edge.point_idx = next_idx;
			secondary_edge.origin_point = next_point;
			auto *edge2_ptr = edge_status.lookup(secondary_edge, edge_compare_func);
			assert(edge2_ptr != nullptr);

			//需要求出两条边的相对位置
			float f = cross_normalize(now_point - prev_point, now_point - next_point);
			bool b2 =  f > 0.0f;//是否在下侧

			auto *above_ptr = edge_status.find_next(b2 ?edge_ptr:edge2_ptr);
			auto *below_ptr = edge_status.find_prev(!b2 ?edge_ptr:edge2_ptr);
			 
			edge_status.remove_case(edge_ptr);
			edge_status.remove_case(edge2_ptr);
			//求出交点
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
		else if (target_event.event_type == EventType_Left) {//如果是普通的左侧端点
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
		else if (target_event.event_type == EventType_Right) {//右侧端点,此时删除与之关联的边即可
			simple_edge target_edge = {
				target_polygon.polygon[target_event.point_idx],
				target_polygon.polygon[target_event.next_idx],
				target_event.owner_idx,
				target_event.point_idx,target_event.next_idx,
			};
			auto *edge_ptr = edge_status.lookup(target_edge, edge_compare_func);
			assert(edge_ptr != nullptr);
			//找出上下邻居
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
			//如果是交叉点事件,此时需要记录相关交点
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
			//同时需要修改两条相关边的起点,以及交换两条边的位置
			edge1_ptr->tw_value.origin_point = target_event.event_point;
			edge2_ptr->tw_value.origin_point = target_event.event_point;
			//实际上,该处代码是可以优化的,前提是对red_black_tree的代码结构非常了解
			//注意,相关的边对象必须先删除然后重新导入,而不能直接的交换内容,因为经过交叉点
			//之后,即使交换了相关对象,但是其排序位置也不一定是正确的,很有可能破坏red_black_tree的排序结构
			simple_edge &t = edge1_ptr->tw_value;
			edge_status.remove_case(edge1_ptr);
			edge1_ptr = edge_status.insert(t, edge_compare_func);

			simple_edge &t2 = edge2_ptr->tw_value;
			edge_status.remove_case(edge2_ptr);
			edge2_ptr = edge_status.insert(t2, edge_compare_func);
			//重新检查新的临边,原来edge1_ptr的位置在edge2_ptr之上,现在两者交换了
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
			//另一个点
			above_ptr = edge_status.find_next(edge1_ptr);//上侧
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
			//edge2_ptr的下侧
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
	//注意如果没有任何的交点,要么是完全的分离,要么是一个多边形完全处于另外一个的内部
	//这两种情况我们稍后分析

	//第一步对顶点进行统计
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
			//作上标志,表示未被访问过
			st.ref = 0;
		}
	}
	//第二步,逐边遍历,遍历的过程有点复杂

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
		//以下算法的核心思想是利用多边形的两条边至多只有一个交点的性质
		const simple_interleave &interleave_st = it->second;
		float f = cross(interleave_st.interleave_point, polygon1[(interleave_st.point_idx1 + 1) % array_size1], polygon2[(interleave_st.point_idx2 + 1) % array_size2]);
		bool b2 = f < 0.0f;
		//如果f < 0.0,则表示会继续沿着原来的边继续前进,否则交换顺序
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
			//循环遍历孤立的顶点
			while (lookup_idx == -1) {
				point_idx1 = (point_idx1 + 1)%array_size1;
				vector_fast_push_back(polygon, (*polygon1_ptr)[point_idx1%array_size1]);
				auto st2 = other_edge_ptr->find(point_idx1);
				lookup_idx = st2 == other_edge_ptr->end() ? -1 : st2->second[0].point_idx;
			}
			b2 = !b2;
			//检测当前是否回到了原点
			point_idx2 = point_idx1;
			point_idx1 = lookup_idx;
			if (b2 == b3 && lookup_idx == point_idx)
				break;
			//计算新的交叉点
			int key_code = b2?map_key(point_idx1, point_idx2):map_key(point_idx2,point_idx1);
			it2 = polygon_edge.find(key_code);
			//重新设置某些数据结构
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