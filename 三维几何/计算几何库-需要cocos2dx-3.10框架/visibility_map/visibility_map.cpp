/*
  *可见性图算法实现
  *2020年8月8日
  *@author:xiaohuaxiong
 */
#include "visibility_map.h"
#include "matrix/matrix.h"
#include "data_struct/balance_tree.h"
using namespace cocos2d;

NS_GT_BEGIN
/*
  *射线与线段相交测试
 */
bool rt_ray_segment_intersect(rtVertex *source_ptr,const Vec2 &normal_v2,rtSegment *segment_ptr,cocos2d::Vec2 *intersect_ptr) {
	const Vec2 *location_ptr = source_ptr->location_ptr;
	const Vec2 a_v2 = *segment_ptr->start_ptr->location_ptr - *location_ptr;
	const Vec2 b_v2 = *segment_ptr->dest_ptr->location_ptr - *location_ptr;
	
	float d1 = cross(a_v2,normal_v2);
	float d2 = cross(b_v2,normal_v2);
	float f2 = d1 / (d1 - d2);

	Vec2 intersect_point = *segment_ptr->start_ptr->location_ptr * (1.0f - f2) + *segment_ptr->dest_ptr->location_ptr * f2;
	if (intersect_ptr) * intersect_ptr = intersect_point;

	float f3 = d1 * d2;
	if (f3 > 0.001f )return false;
	return dot(intersect_point - *location_ptr,normal_v2) >=0.0f;
}
/*
  *判断是否顶点可见
 */
bool rt_check_vertex_visible(rtVertex	*target_vertex,rtVertex *vertex_ptr,red_black_tree<rtSegment*> &interleave_segments) {
	//bool b2 = !target_node || target_node->tw_value->start_ptr == vertex_ptr || target_node->tw_value->dest_ptr == vertex_ptr;
	//如果处于同一个多边形之中,则需要检测是否相交于多边形的内部,
	//实际上该处的代码是可以优化的,因为已经假设所有的多边形都是凸多边形
	//如果没有这种假设,则这里就需要使用一般多边形与线段相交测试算法
	std::vector<Vec2> *polygon_ptr = vertex_ptr->owner_ptr;
	if (polygon_ptr != nullptr && polygon_ptr == target_vertex->owner_ptr) {
		int array_size = polygon_ptr->size();
		int vertex_idx = vertex_ptr->location_ptr - polygon_ptr->data();
		int target_idx = target_vertex->location_ptr - polygon_ptr->data();

		return (vertex_idx +1)% array_size == target_idx || (vertex_idx - 1 + array_size)% array_size == target_idx;
	}
	//还有一种情况就是,target_vertex/vertex_ptr,last_vertex_ptr共线,此时的检测就要复杂一些,这里我们暂时不理会
	//这种情况,如果读者对该情况有兴趣,可以在这里自行实现
	red_black_tree<rtSegment*>::internal_node *target_node = interleave_segments.find_minimum();
	return !target_node || target_node->tw_value->start_ptr == vertex_ptr || target_node->tw_value->dest_ptr == vertex_ptr;
}
/*
  *求与目标顶点可见的所有可见顶点集合
 */
void rt_compute_visibility_vertex(red_black_tree<rtSegment*> &interleave_segments,rtVertex *target_vertex,rtVertex *rt_vertex_array,std::vector<rtVertex *> &rt_vertex_memory,int array_size,rtSegment *segment_array,std::vector<rtVertex*> &visibility_vertex) {
	//对所有的顶点进行排序,排序的原则为目标点与源点的连线与+X 轴的夹角大小升序排列
	cocos2d::Vec2 *target_location = target_vertex->location_ptr;
	int secondary_idx = target_vertex->owner_ptr? target_location - target_vertex->owner_ptr->data(): array_size * 2;
	int polygon_array_size = target_vertex->owner_ptr ? target_vertex->owner_ptr->size() : 10000;
	//clear
	for (int j = 0; j < array_size; ++j) {
		rtVertex  *vertex_ptr = rt_vertex_array + j;
		segment_array[j].start_ptr = nullptr;
		segment_array[j].dest_ptr = nullptr;
		segment_array[j].internal_ptr = nullptr;

		vertex_ptr->visible = false;
		vertex_ptr->next_seg_ptr = nullptr;
		vertex_ptr->next_seg_ptr2 = nullptr;
		vertex_ptr->prev_seg_ptr = nullptr;
		vertex_ptr->prev_seg_ptr2 = nullptr;
	}

	for (int base_j = 0, j = 0; j < array_size; ++j) {
		rtVertex  *vertex_ptr = rt_vertex_array + j;
		if (vertex_ptr != target_vertex)
			rt_vertex_memory[base_j++] = vertex_ptr;
		//同时处理新生成的线段,每处理一个顶点,需要计算两条线段
		//将源点/目标点/以及与源点处于同一个多边形的邻接顶点排除在外,注意nullptr
		if (j >= 2 && (vertex_ptr->owner_ptr != target_vertex->owner_ptr || vertex_ptr != target_vertex)) {
			//求下一个顶点索引,在求取的过程中,稍微有点繁琐
			int tripple_idx = vertex_ptr->owner_ptr ? vertex_ptr->location_ptr - vertex_ptr->owner_ptr->data() : -array_size * 2;
			int next_idx = tripple_idx + 1>= vertex_ptr->owner_ptr->size()? j+ 1 - vertex_ptr->owner_ptr->size():j+1;
			rtVertex  *next_ptr = rt_vertex_array + next_idx;
			if (next_ptr == target_vertex)continue;//跳跃出临边
			//注意这里不能通过比较两向量与+X轴之间的夹角,原因是在跨越-X轴的时候会产生错误的结果,因此需要比较相对位置
			float f2 = cross(*target_location,*vertex_ptr->location_ptr,*next_ptr->location_ptr);
			if (f2 > 0.0f) {
				assert(vertex_ptr->next_seg_ptr == nullptr || vertex_ptr->next_seg_ptr2 == nullptr);
				if (!vertex_ptr->next_seg_ptr)
					vertex_ptr->next_seg_ptr = segment_array + j;
				else
					vertex_ptr->next_seg_ptr2 = segment_array + j;

				assert(next_ptr->prev_seg_ptr == nullptr || next_ptr->prev_seg_ptr2 == nullptr);
				if (!next_ptr->prev_seg_ptr)
					next_ptr->prev_seg_ptr = segment_array + j;
				else
					next_ptr->prev_seg_ptr2 = segment_array + j;

				segment_array[j].start_ptr = vertex_ptr;
				segment_array[j].dest_ptr = next_ptr;
			}
			else {
				if (!next_ptr->next_seg_ptr)
					next_ptr->next_seg_ptr = segment_array + j;
				else
					next_ptr->next_seg_ptr2 = segment_array + j;

				if (!vertex_ptr->prev_seg_ptr)
					vertex_ptr->prev_seg_ptr = segment_array + j;
				else
					vertex_ptr->prev_seg_ptr2 = segment_array + j;

				segment_array[j].start_ptr = next_ptr;
				segment_array[j].dest_ptr = vertex_ptr;
			}
		}
	}
	//排序
	std::function<bool(rtVertex * a, rtVertex *b)> compare_func = [target_location](rtVertex *a, rtVertex *b)->bool {
		float a_x = a->location_ptr->x - target_location->x;
		float a_y = a->location_ptr->y - target_location->y;

		float b_x = b->location_ptr->x - target_location->x;
		float b_y = b->location_ptr->y - target_location->y;

		float a_f = atan2f(a_y,a_x);
		float b_f = atan2f(b_y,b_x);

		return a_f < b_f || a_f == b_f && sqrtf(a_x * a_x + a_y * a_y) < sqrtf(b_x * b_x + b_y * b_y);
	};

	quick_sort_origin_type<rtVertex*>(rt_vertex_memory.data(),array_size - 1,compare_func);
	//针对已经排好序的顶点,需要再次做一下处理,此次是为了计算相关的线段
	Vec2 normal_v2(-1.0f,0.0f);
	std::function<int(rtSegment *const &a_ptr,rtSegment *const &b_ptr)> lookup_func = [&normal_v2, target_vertex](rtSegment *const &a_ptr,rtSegment *const &b_ptr)->int {
		//求出交叉点,如果交叉点相同,则比较其相对方向
		Vec2 intersect_point,intersect_point2;
		bool b2 = rt_ray_segment_intersect(target_vertex,normal_v2,a_ptr,&intersect_point);
		bool b3 = rt_ray_segment_intersect(target_vertex, normal_v2, b_ptr, &intersect_point2);

		//assert(b2 && b3);

		float f2 = length(*target_vertex->location_ptr,intersect_point);
		float f3 = length(*target_vertex->location_ptr,intersect_point2);

		float f4 = fabsf(f2 - f3);
		if (f4 < 0.01f)
			return cross(intersect_point,*a_ptr->dest_ptr->location_ptr,*b_ptr->dest_ptr->location_ptr) > 0.0f?1:-1;
		return f2 < f3?-1:1;
	};


	for (int j = 0; j < array_size ; ++j) {
		rtSegment  *segment_ptr = segment_array + j;
		//将与从target_vertex出发沿-X轴方向的射线真相交的全部线段加入到状态集合中
		if (segment_ptr->start_ptr && rt_ray_segment_intersect(target_vertex, normal_v2,segment_ptr,nullptr)) 
			segment_ptr->internal_ptr =  interleave_segments.insert(segment_ptr, lookup_func);
	}
	//针对所有的顶点,逐个的筛查
	for (int j = 0; j < array_size - 1; ++j) {
		rtVertex *vertex_ptr = rt_vertex_memory[j];
		//重新设定射线的方向
		normal_v2 = normalize(*target_vertex->location_ptr,*vertex_ptr->location_ptr);
		//查找最左侧是否有遮挡物
		//red_black_tree<rtSegment*>::internal_node *target_node = interleave_segments.find_minimum();
		//bool b2 = !target_node || target_node->tw_value->start_ptr == vertex_ptr || target_node->tw_value->dest_ptr == vertex_ptr;
		bool b2 = rt_check_vertex_visible(target_vertex, vertex_ptr, interleave_segments);
		if (b2)
			vector_fast_push_back(visibility_vertex,vertex_ptr);
		vertex_ptr->visible = b2;
		//删除以vertex_ptr结束的引用节点
		if (vertex_ptr->prev_seg_ptr != nullptr && vertex_ptr->prev_seg_ptr->internal_ptr != nullptr)
			interleave_segments.remove_case(vertex_ptr->prev_seg_ptr->internal_ptr);
		if (vertex_ptr->prev_seg_ptr2 != nullptr && vertex_ptr->prev_seg_ptr2->internal_ptr != nullptr)
			interleave_segments.remove_case(vertex_ptr->prev_seg_ptr2->internal_ptr);

		//将以vertex_ptr为开头的线段添加进入状态集合中
		if (vertex_ptr->next_seg_ptr != nullptr)
			vertex_ptr->next_seg_ptr->internal_ptr = interleave_segments.insert(vertex_ptr->next_seg_ptr, lookup_func);
		if (vertex_ptr->next_seg_ptr2 != nullptr)
			vertex_ptr->next_seg_ptr2->internal_ptr = interleave_segments.insert(vertex_ptr->next_seg_ptr2,lookup_func);
	}
}

std::vector<rtVertex*>* rt_compute_visibility_map(std::vector<cocos2d::Vec2> *polygons, int array_size, cocos2d::Vec2 &start_point, cocos2d::Vec2 &target_point) {
	//统计相关顶点数据
	int  total_array_size = 2;
	for (int j = 0; j < array_size; ++j)
		total_array_size += polygons[j].size();
	/*
	  *该段内存一次性的分配
	  *在后面将会多次的反复的使用
	 */
	rtVertex  *rt_vertex_array = new rtVertex[total_array_size];
	rt_vertex_array[0].location_ptr = &start_point;
	rt_vertex_array[1].location_ptr = &target_point;

	int base_j = 2;
	for (int j = 0; j < array_size; ++j) {
		auto &polygon = polygons[j];
		for (int k = 0; k < polygon.size(); ++k) {
			rt_vertex_array[base_j].location_ptr = polygon.data() + k;
			rt_vertex_array[base_j].owner_ptr = &polygon;

			++base_j;
		}
	}
	//针对每一个顶点逐次的处理,在处理的过程中,确实有可能需要平方量级的内存
	std::vector<rtVertex*>  *every_vertex_array = new std::vector<rtVertex *>[total_array_size];
	rtSegment *segment_array = new rtSegment[total_array_size];
	std::vector<rtVertex *> rt_vertex_memory(total_array_size);
	red_black_tree<rtSegment*>  interleave_segments;

	for (int j = 0; j < total_array_size; ++j) {
		rt_compute_visibility_vertex(interleave_segments, rt_vertex_array + j, rt_vertex_array, rt_vertex_memory, total_array_size, segment_array, every_vertex_array[j]);
		interleave_segments.clear();
	}

	delete[] rt_vertex_array;
	delete[] segment_array;

	return every_vertex_array;
}

NS_GT_END