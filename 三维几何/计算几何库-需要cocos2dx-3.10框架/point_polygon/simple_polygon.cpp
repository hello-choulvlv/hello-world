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

bool simple_polygon_intersect(const std::vector<cocos2d::Vec2> &polygon1, const std::vector<cocos2d::Vec2>&polygon2, std::vector<cocos2d::Vec2> &intersect_array) {
	//建立算法运行过程中需要使用到的数据结构
	priority_queue<simple_event>  event_queue(polygon1.size() + polygon2.size() << 1);
	red_black_tree<simple_edge>   edge_status;
	//标准比较坐标点
	Vec2 compare_base_point,intersect_point;
	std::function<int(const simple_edge &, const simple_edge &)> edge_compare_func = [&compare_base_point](const simple_edge &a, const simple_edge &b)->int {
		//以x = compare_base_point.x为基准线,求线段与基准线交点的y坐标大小,如果相差非常小,则沿逆时针比较相对位置
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
		////如果两个端点的坐标值相等,则很大程度上可能因为是相互衔接的端点,此时需要额外的判断
		//bool b1 = false, b2 = false;
		//if (a.owner_idx == b.owner_idx && (b1 = a.final_point.equals(b.origin_point) || (b2 = b.final_point.equals(a.origin_point))))
		//	return b1?;
		//否则计算以交点(tx,a_y)为原点的相对位置
		compare_base_point.y = a_ly;
		float f = cross(compare_base_point,a.final_point + Vec2(a_x,a_y),b.final_point + Vec2(b_x,b_y));
		return f > 0.0f ? -1 : (f < 0.0f ? 1 : 0);
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

	int loops = 0;
	while (event_queue.size()) {
		++loops;
		//检查头
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
				//此时将会形成新的交点,并截断原来的线段
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
			//edge2边数据
			above_ptr = edge_status.find_next(edge2_ptr);
			if (above_ptr && above_ptr->tw_value.owner_idx != edge2_ptr->tw_value.owner_idx) {
				//求交点
				simple_edge &target_edge = above_ptr->tw_value;
				bool b_interleave = segment_segment_intersect_test(target_edge.origin_point, target_edge.final_point, edge2.origin_point, edge2.final_point, intersect_point);
				//此时将会形成新的交点,并截断原来的线段
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
				//此时将会形成新的交点,并截断原来的线段
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
		else if (target_event.event_type == EventType_Right && now_point.x > prev_point.x && now_point.x > next_point.x) {//如果是右侧端点
			//此时需要从线段状态中移除掉与相关的端点相连接的两条线段
			simple_edge secondary_edge = {prev_point,now_point};
			edge_status.remove(secondary_edge, edge_compare_func);

			secondary_edge.origin_point = next_point;
			edge_status.remove(secondary_edge,edge_compare_func);
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
		else if (target_event.event_type == EventType_Right) {//右侧端点,此时删除与之关联的边即可
			simple_edge target_edge = {
				target_polygon.polygon[target_event.point_idx],
				target_polygon.polygon[target_event.next_idx],
			};
			edge_status.remove(target_edge, edge_compare_func);
		}
		else {
			//如果是交叉点事件,此时需要记录相关交点
			assert(target_event.event_type == EventType_Interleave);
			vector_fast_push_back(intersect_array,target_event.event_point);
			//同时需要修改两条相关边的起点,以及交换两条边的位置
			auto *edge1_ptr = target_event.cross_edge1, *edge2_ptr = target_event.cross_edge2;
			edge1_ptr->tw_value.origin_point = target_event.event_point;
			edge2_ptr->tw_value.origin_point = target_event.event_point;

			simple_edge t = edge1_ptr->tw_value;
			edge1_ptr->tw_value = edge2_ptr->tw_value;
			edge2_ptr->tw_value = t;
			//重新检查新的临边,原来edge1_ptr的位置在edge2_ptr之上,现在两者交换了
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
			//另一个点
			above_ptr = edge_status.find_next(edge2_ptr);//上侧
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
			//edge2_ptr的下侧
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