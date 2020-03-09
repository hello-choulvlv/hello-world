/*
  *͹���㷨ʵ��,2d+3d
  *��һ���ִ����Ǵ�ԭ����point_polygonĿ¼�з��������
  *2020��2��13��5
 */
#include "convex_hull.h"
#include "matrix/matrix.h"
#include "data_struct/link_list.h"
#include "data_struct/priority_queue.h"
#include <list>
#include<set>
#include<map>
using namespace cocos2d ;

NS_GT_BEGIN
bool polygon_compute_convex_hull(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon_points)
{
	if (points.size() < 3) return false;
	//��һ��,�Զ����������
	std::vector<Vec2> points_tmp = points;
	//����y������С,����x�����С�ĵ�
	int target_j = 0;
	for (int index_j = 1; index_j < points_tmp.size(); ++index_j)
	{
		if (points_tmp[index_j].y < points_tmp[target_j].y || (points_tmp[index_j].y == points_tmp[target_j].y && points_tmp[index_j].x > points_tmp[target_j].x))
			target_j = index_j;
	}

	Vec2 f_point = points_tmp[target_j];
	points_tmp.erase(points_tmp.begin() + target_j);
	std::function<bool(const Vec2 &a, const Vec2 &b)>  compare_func = [f_point](const Vec2 &a, const Vec2 &b)->bool {
		const float d_x = a.x - f_point.x;
		const float d_y = a.y - f_point.y;

		const float f_x = b.x - f_point.x;
		const float f_y = b.y - f_point.y;

		float angle_a = atan2f(d_y, d_x);
		float angle_b = atan2f(f_y, f_x);
		//����Ƕ���ͬ,����ο���������������λ�ø���ǰ
		return angle_a < angle_b || (angle_a == angle_b && d_x * d_x + d_y * d_y < f_x * f_x + f_y * f_y);//���⼫����ͬ�ĵ㼯
	};
	quick_sort<Vec2>(points_tmp.data(), (int)points_tmp.size(), compare_func);

	polygon_points.reserve(points.size());
	polygon_points.push_back(f_point);
	polygon_points.push_back(points_tmp[0]);

	for (int index_j = 1; index_j < points_tmp.size(); ++index_j)
	{
		//�Ե�ǰ�ĵ�������
		Vec2 &target_point = points_tmp.at(index_j);
		//����Ƿ�����ǰ��������������һ�µ�
		while (polygon_points.size() > 1 && sign_area(polygon_points.back() - polygon_points[polygon_points.size() - 2], target_point - polygon_points.back()) <= 0)
		{
			polygon_points.pop_back();
		}
		polygon_points.push_back(target_point);
	}

	return polygon_points.size() >= 3;
}

//quick hull�㷨ʵ��
bool quick_hull_algorithm2d(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon)
{
	//��һ���������½������ϽǵĶ���,�����������Ȼ����͹����ε�����֧�ŵ�
	int base_l = 0, base_t = 0;
	for (int index_l = 1; index_l < points.size(); ++index_l)
	{
		const Vec2 &compare_point = points.at(base_l);
		const Vec2 &secondary_point = points.at(base_t);
		const Vec2 &target_point = points.at(index_l);
		if (target_point.x < compare_point.x || target_point.x == compare_point.x && target_point.y < compare_point.y)
			base_l = index_l;
		if (target_point.x > secondary_point.x || target_point.x == secondary_point.x && target_point.y > secondary_point.y)
			base_t = index_l;
	}
	//���ζ����еĶ�����л���,��������������
	std::list<QuickHull>   stack_polygon;
	const Vec2 &base_point = points[base_t];
	Vec2  direction = base_point - points[base_l];
	//��Ҫ��������
	QuickHull   quick_vertex;
	stack_polygon.push_back(quick_vertex);
	stack_polygon.push_back(quick_vertex);

	QuickHull  &top = stack_polygon.back();
	top.start_l = base_l;
	top.final_l = base_t;
	top.operate_points.reserve(points.size() - 2);

	QuickHull &back = stack_polygon.front();
	back.start_l = base_t;
	back.final_l = base_l;
	back.operate_points.reserve(points.size() - 2);
	//������еĶ���,���л���
	for (int index_l = 0; index_l < points.size(); ++index_l)
	{
		if (index_l != base_l && index_l != base_t)
		{
			const Vec2 &target_point = points.at(index_l);
			float f = -direction.y * (target_point.x - base_point.x) + direction.x * (target_point.y - base_point.y);
			if (f < 0)
				top.operate_points.push_back(index_l);
			else if (f > 0)
				back.operate_points.push_back(index_l);
		}
	}
	polygon.reserve(64);
	//������еĶ�ջ���д���
	while (stack_polygon.size() > 0)
	{
		QuickHull &quick_hull = stack_polygon.back();
		//��ʱ�ñ��Ѿ�û�й���������,��Ҫ�Ƴ���,���ѸñߵĵͶ˵Ķ˵�д�뵽����εĵ㼯����
		if (!quick_hull.operate_points.size())
		{
			polygon.push_back(points.at(quick_hull.start_l));
			stack_polygon.pop_back();
		}
		else//������Ҫ����ñ����������еĺ�ѡ����,ע�������Ҫ���ѱ�,����Ѻ��˳����������ʱ���,�������ó��Ķ���ζ��㽫�Ǵ��ҵ�
		{
			const Vec2 &base_point = points.at(quick_hull.start_l);
			direction.x = points.at(quick_hull.final_l).y - base_point.y;
			direction.y = base_point.x - points.at(quick_hull.final_l).x;

			int target_l = -1;
			float max_f = -FLT_MAX;
			for (int index_l = 0; index_l < quick_hull.operate_points.size(); ++index_l)
			{
				int select_l = quick_hull.operate_points[index_l];
				float f = dot(direction, points[select_l] - base_point);
				if (f > max_f)
				{
					max_f = f;
					target_l = select_l;
				}
			}
			assert(target_l != -1);
			//ѡ����ĳһ���,����Ҫ���ѱ�,ע����ѵĴ���
			QuickHull  secondary_hull;
			secondary_hull.start_l = target_l;
			secondary_hull.final_l = quick_hull.final_l;
			secondary_hull.operate_points.reserve(quick_hull.operate_points.size() - 1);

			QuickHull last_hull;
			last_hull.start_l = quick_hull.start_l;
			last_hull.final_l = target_l;
			last_hull.operate_points.reserve(quick_hull.operate_points.size() - 1);

			const Vec2 &other_point = points.at(target_l);
			direction.x = other_point.y - points.at(quick_hull.start_l).y;
			direction.y = points.at(quick_hull.start_l).x - other_point.x;
			//��һ����
			const Vec2  d2 = Vec2(points.at(quick_hull.final_l).y - other_point.y, other_point.x - points.at(quick_hull.final_l).x);
			//�Զ�����л���
			for (int index_l = 0; index_l < quick_hull.operate_points.size(); ++index_l)
			{
				int  target_index = quick_hull.operate_points[index_l];
				if (target_index != target_l)
				{
					const Vec2 interpolation = points.at(target_index) - other_point;
					//�������������ͬʱ����
					if (dot(direction, interpolation) > 0)
						last_hull.operate_points.push_back(target_index);
					else if (dot(d2, interpolation) > 0)
						secondary_hull.operate_points.push_back(target_index);
				}
			}
			//�Ƴ���ԭ����
			stack_polygon.pop_back();
			stack_polygon.push_back(secondary_hull);
			stack_polygon.push_back(last_hull);
		}
	}
	return polygon.size() >= 3;
}
#define plane_normal(plane) cross_normalize(points[plane->v1],points[plane->v2],points[plane->v3])
void static_create_tetrahedron(const std::vector<Vec3> &points, priority_queue<Plane3*> &operate_queue, std::function<bool(Plane3 *const &a, Plane3 *const &b)> &compare_func, std::function<int(Plane3 *&plane, int index_queue)> &modify_func)
{
	int array_size = points.size();
	//��һ����Ҫ������ض���Ϊ���յ�͹���ϵ����������ı�
	int boundary_array[6] = { 0 };
	for (int index_l = 1; index_l < array_size; ++index_l)
	{
		const Vec3 &point = points[index_l];
		//-X
		if (points[boundary_array[0]].x > point.x)boundary_array[0] = index_l;
		//+X
		if (points[boundary_array[1]].x < point.x) boundary_array[1] = index_l;
		//-Y
		if (points[boundary_array[2]].y > point.y)boundary_array[2] = index_l;
		//+Y
		if (points[boundary_array[3]].y < point.y)boundary_array[3] = index_l;
		//-Z
		if (points[boundary_array[4]].z > point.z)boundary_array[4] = index_l;
		//+Z
		if (points[boundary_array[5]].z < point.z)boundary_array[5] = index_l;
	}
	short v1 = boundary_array[0], v2 = boundary_array[5], v3 = boundary_array[1],v4 = boundary_array[3];
	//������ѡ�еĵ���ѡ������,����һ��ƽ��,������Ҫ����ļ��һ�·�ֹ���˵��������,����������ƽ��������
	Vec3 normal = cross_normalize(points[v1], points[v2], points[v3]);
	float f = dot(normal, points[v4] - points[v1]);
	bool b_normal = Vec3::ZERO != normal && f > 0.0f;
	if (!b_normal)
	{
		b_normal = true;
		if (f < 0.0f)
		{
			std::swap(v2, v3);
			normal = -normal;
		}
		else
		{
			v2 = boundary_array[1]; v3 = boundary_array[4];
			normal = cross_normalize(points[v1], points[v2], points[v3]);
			f = dot(normal, points[v4] - points[v1]);
			b_normal = Vec3::ZERO != normal && f > 0.0f;
		}
	}
	if (!b_normal)
	{
		b_normal = true;
		if (f < 0.0f)
		{
			std::swap(v2, v3);
			normal = -normal;
		}
		else
		{
			v1 = boundary_array[1]; v2 = boundary_array[3]; v3 = boundary_array[2];
			normal = cross_normalize(points[v1], points[v2], points[v3]);
			f = dot(normal, points[v4] - points[v1]);
			b_normal = Vec3::ZERO != normal && f > 0.0f;
		}
	}
	assert(b_normal);
	assert(dot(points[v4] - points[v1],normal) > 0.0f);
	//��������Ĺ�����̱Ƚϸ���
	Plane3  *plane1 = new Plane3(v1,v2,v4);//a,b
	ConvexEdge  *ab1 = new ConvexEdge(v1, v2, plane1);
	ConvexEdge  *ab2 = new ConvexEdge(v2,v4,plane1);
	ConvexEdge  *ab3 = new ConvexEdge(v4,v1,plane1);
	ab1->next = ab2; ab2->prev = ab1;
	ab2->next = ab3; ab3->prev = ab2;
	ab3->next = ab1; ab1->prev = ab3;
	plane1->head = ab1; plane1->tail = ab3;

	Plane3 *plane2 = new Plane3(v2,v3,v4);
	ConvexEdge *bc1 = new ConvexEdge(v2,v3,plane2);
	ConvexEdge *bc2 = new ConvexEdge(v3,v4,plane2);
	ConvexEdge *bc3 = new ConvexEdge(v4,v2,plane2);
	bc1->next = bc2; bc2->prev = bc1;
	bc2->next = bc3; bc3->prev = bc2;
	bc3->next = bc1; bc1->prev = bc3;
	plane2->head = bc1; plane2->tail = bc3;

	Plane3 *plane3 = new Plane3(v3,v1,v4);
	ConvexEdge *ca1 = new ConvexEdge(v3,v1,plane3);
	ConvexEdge *ca2 = new ConvexEdge(v1,v4,plane3);
	ConvexEdge	*ca3 = new ConvexEdge(v4,v3,plane3);
	ca1->next = ca2; ca2->prev = ca1;
	ca2->next = ca3; ca3->prev = ca2;
	ca3->next = ca1; ca1->prev = ca3;
	plane3->head = ca1; plane3->tail = ca3;

	Plane3 *plane4 = new Plane3(v1,v3,v2);
	ConvexEdge *le1 = new ConvexEdge(v1,v3,plane4);
	ConvexEdge *le2 = new ConvexEdge(v3,v2,plane4);
	ConvexEdge *le3 = new ConvexEdge(v2,v1,plane4);
	le1->next = le2; le2->prev = le1;
	le2->next = le3; le3->prev = le2;
	le3->next = le1; le1->prev = le3;
	plane4->head = le1; plane4->tail = le3;
	//ͼԪ���֮��Ĺ���,һ��6��
	ab1->twin = le3; le3->twin = ab1;
	ab2->twin = bc3; bc3->twin = ab2;
	ab3->twin = ca2; ca2->twin = ab3;

	bc1->twin = le2; le2->twin = bc1;
	bc2->twin = ca3; ca3->twin = bc2;
	ca1->twin = le1; le1->twin = ca1;

	const Vec3 &base_point = points[v4];
	plane1->normal = plane_normal(plane1);
	plane2->normal = plane_normal(plane2);
	plane3->normal = plane_normal(plane3);
	plane4->normal = plane_normal(plane4);

	float df1 = -FLT_MAX, df2 = -FLT_MAX, df3 = -FLT_MAX, df4 = -FLT_MAX;
	for (int index_l = 0; index_l < array_size; ++index_l)
	{
		Vec3 interpolation = points[index_l] - base_point;
		float f1 = dot(interpolation, plane1->normal),f2,f3,f4;
		Plane3 *target_plane = nullptr;
		//���˵������׼����ƽ��ǳ�С�ĵ�
		if (fabsf(f1) > 0.001f && f1 > 0.0f)
		{
			vector_fast_push_back(plane1->operate_array, index_l);
			if (f1 > df1)
			{
				df1 = f1;
				plane1->high_v = index_l;
			}
		}
		else if (fabsf(f2 = dot(interpolation, plane2->normal)) > 0.001f && f2 > 0.0f)
		{
			vector_fast_push_back(plane2->operate_array, index_l);
			if (f2 > df2)
			{
				df2 = f2;
				plane2->high_v = index_l;
			}
		}
		else if (fabsf(f3 = dot(interpolation, plane3->normal)) > 0.001f && f3 > 0.0f)
		{
			vector_fast_push_back(plane3->operate_array, index_l);
			if (f3 > df3)
			{
				df3 = f3;
				plane3->high_v = index_l;
			}
		}
		else if (fabsf(f4 = dot(points[index_l] - points[v1], plane4->normal)) > 0.001f && f4 > 0.0f)
		{
			vector_fast_push_back(plane4->operate_array, index_l);
			if (f4 > df4)
			{
				df4 = f4;
				plane4->high_v = index_l;
			}
		}
	}
	operate_queue.insert(plane1, compare_func, modify_func);
	operate_queue.insert(plane2, compare_func, modify_func);
	operate_queue.insert(plane3, compare_func, modify_func);
	operate_queue.insert(plane4, compare_func, modify_func);
}

//����Ƿ���ĳһ��ƽ�滹����δ������ϵĳ�ͻ�㼯
bool check_face_conflict_point(std::list<Plane3 *> &operate_queue,Plane3 **target_plane)
{
	for (auto it = operate_queue.begin(); it != operate_queue.end(); ++it)
	{
		Plane3 *plane = *it;
		if (plane->operate_array.size())
		{
			*target_plane = plane;
			return true;
		}
	}
	return false;
}
//����صı߲��뵽������
bool insert_convex_hull_edge(link_list<ConvexEdge *> &horizontal_edge,ConvexEdge *target)
{
	auto *it_ptr = horizontal_edge.head();
	for (; it_ptr != nullptr; it_ptr = horizontal_edge.next(it_ptr))
	{
		ConvexEdge *edge = it_ptr->tv_value;
		//���������
		if (edge->v2 == target->v1)
		{
			horizontal_edge.insert_after(it_ptr,target);
			return true;
		}
		else if (target->v2 == edge->v1)//��ʱĿ������ڵ�ǰ�ߵ�λ��֮ǰ
		{
			horizontal_edge.insert_before(it_ptr,target);
			return true;
		}
	}
	horizontal_edge.push_back(target);
	return false;
}
bool assert_convex_hull_edge_continuous(link_list<ConvexEdge *> &horizontal_edge)
{
	assert(horizontal_edge.size() > 2);
	auto *node_ptr = horizontal_edge.head();
	while (node_ptr->next) {
		auto *next_ptr = node_ptr->next;

		if (node_ptr->tv_value->v2 != next_ptr->tv_value->v1) {
			auto *tripple_ptr = next_ptr->next;
			while (tripple_ptr && node_ptr->tv_value->v2 != tripple_ptr->tv_value->v1) {
				tripple_ptr = tripple_ptr->next;
			}
			if (!tripple_ptr)return false;

			while (tripple_ptr && node_ptr->tv_value->v2 == tripple_ptr->tv_value->v1) {
				auto *s_node = tripple_ptr->next;
				horizontal_edge.remove(tripple_ptr, false);
				horizontal_edge.insert_after(node_ptr, tripple_ptr);
				node_ptr = tripple_ptr;
				tripple_ptr = s_node;
			}
		}
		else node_ptr = next_ptr;
	}

	return true;
}

bool assert_convex_hull_valid(std::list<Plane3 *> &operate_queue)
{
	bool b = true;
	for (auto it = operate_queue.begin(); it != operate_queue.end(); ++it)
	{
		Plane3 *plane = *it;
		ConvexEdge *edge = plane->head;
		do 
		{
			ConvexEdge *twin = edge->twin;
			b &= twin->v2 == edge->v1 && twin->v1 == edge->v2;

			ConvexEdge *t = edge;
			edge = edge->next;
			b &= t->v2 == edge->v1;
		} while (edge != plane->head);
	}
	return b;
}
//���ĳһ����ѡ�е�ƽ��,������еĿɼ�ƽ��,�ڱ�Ҫ��ʱ����Ҫ���ºϲ��µ�ƽ��
void quick_hull_build_new_plane(ConvexHullMemmorySlab &memory_slab,link_list<ConvexEdge *> &horizontal_edge,const std::vector<cocos2d::Vec3> &points, priority_queue<Plane3*> &operate_queue,int selected_index, std::function<bool(Plane3 *const &a,Plane3 *const &b)> &compare_func, std::function<int(Plane3 *&plane, int index_queue)> &modify_func)
{
	//������еĿɼ�ƽ��,�Լ���Щ��ƽ�������ƽ��
	//�õ㱻��Ϊ�ӵ�
	const Vec3 &base_point = points[selected_index];
	int array_size = points.size();
	char *b_found_array = memory_slab._global_memory;
	memset(b_found_array, 0, array_size);
	unsigned short *index_array = memory_slab._index_array;
	int index_array_size = 0;

	Plane3 **operate_queue_array = operate_queue.data();
	std::vector<Plane3 *>  plane_array;
	plane_array.reserve(32);

	for (int s = 0; s < operate_queue.size(); ++s)
	{
		Plane3 *plane = operate_queue_array[s];
		const Vec3 &normal = plane->normal;// cross_normalize(points[plane->v1], points[plane->v2], points[plane->v3]);
		float f = dot(base_point - points[plane->v1],normal);
		//����Ƿ��ǿɼ���ƽ��
		if (f > 0.0f)
		{
			plane->ref = 1;//ע�������־,����ζ����ص�ƽ��ᱻ�ɵ�
			auto *operate_array = plane->operate_array.data();
			int s_size = plane->operate_array.size();
			for (int j = 0; j < s_size; ++j)
				index_array[index_array_size++] = operate_array[j];
			vector_fast_push_back(plane_array,plane);
		}
		else if (f < 0.0f)
		{
			//������ƽ����ڽ�ƽ���Ƿ���ӵ�ɼ�
			ConvexEdge *edge = plane->head;
			do 
			{
				Plane3 *adj_plane = edge->twin->owner;
				float f2 = dot(base_point - points[adj_plane->v1], adj_plane->normal);
				//�ڽ�ƽ����ӵ�Ŀɼ��Լ��
				if (f2 > 0.0f)
				{
					//����ʵ��,��ʱ��һ��ԭ�������ǿ��Թ��������ߵ�,�˴�������һ���޴�Ŀ�
					//��ʱ��Ҫ�ҳ������ĵ�ƽ�߱�,���ڸñ߶�Ӧ��ƽ��,�Լ��ڽ�ƽ��,�����ɸñߵ��������ֱ�ӻ��
					insert_convex_hull_edge(horizontal_edge, edge);
				}
				edge = edge->next;
			} while (/*!target_edge &&*/ edge != plane->head);//Ϊ�˴��������
			
		}
	}
	assert(index_array_size <= array_size);
	//assert(assert_convex_hull_edge_continuous(horizontal_edge));
	//�����Ѿ������Ŀɼ�ƽ�����ƽ��,����Ҫ��������һЩ�µ�ƽ��
	Plane3 *plane_last = nullptr,*plane_head = nullptr;
	for(auto *it = horizontal_edge.head();it ;it = horizontal_edge.next(it))
	{
		ConvexEdge *edge = it->tv_value;
		Plane3 *plane_twin = edge->twin->owner;
		//�����µ�ƽ��
		Plane3 *plane_new = memory_slab.apply(selected_index,edge->v2,edge->v1);// new Plane3(selected_index, edge->v2, edge->v1);
		ConvexEdge *e1 = memory_slab.apply(selected_index, edge->v2, plane_new);// new ConvexEdge(selected_index, edge->v2, plane_new);
		ConvexEdge *e2 = memory_slab.apply(edge->v2, edge->v1, plane_new);// new ConvexEdge(edge->v2, edge->v1, plane_new);
		ConvexEdge *e3 = memory_slab.apply(edge->v1, selected_index, plane_new);// new ConvexEdge(edge->v1, selected_index, plane_new);

		e1->next = e2; e2->prev = e1;
		e2->next = e3; e3->prev = e2;
		e3->next = e1; e1->prev = e3;
		plane_new->head = e1; plane_new->tail = e3;

		e2->twin = edge; edge->twin = e2;
		if (plane_last)
		{
			plane_last->head->twin = e3;
			e3->twin = plane_last->head;
		}

		if (!plane_head)
			plane_head = plane_new;

		plane_last = plane_new;
		//��Ե�ǰƽ��,���ͻ���ѡ����Ҫ���빫����edge���ڽӵ�����ƽ���н���ɸѡ,�˴�������
		plane_new->normal = plane_normal(plane_new);
		const Vec3 &normal = plane_new->normal;// plane_normal(plane_new);
		//�㷨�ڴ˴��ǿ��Խ����Ż���,ĳЩƽ��Ŀɼ���ע������Ŀ��ƽ�治�ɼ���,��˾���һЩ�򵥵ļ�������֮��
		//������ٵ��жϳ��㼯�Ƿ�ɼ�
		float df = -FLT_MAX;
		for (int j = 0; j < index_array_size; ++j)
		{
			int base_j = index_array[j];
			if (!b_found_array[base_j])
			{
				float f = dot(points[base_j] - base_point, normal);
				if (fabsf(f) > 0.001f && f > 0.0f)
				{
					b_found_array[base_j] = 1;
					vector_fast_push_back(plane_new->operate_array, base_j);
					if (f > df)
					{
						df = f;
						plane_new->high_v = base_j;
					}
				}
			}
		}
		operate_queue.insert(plane_new, compare_func, modify_func);
	}
	//��Ե�һ���Լ����һ��ƽ��,��ĳЩ��صı���Ҫ�ٴ�����
	plane_last->head->twin = plane_head->tail;
	plane_head->tail->twin = plane_last->head;
	//�����ɾ�����е��Ѿ���ǵ�ƽ��
	for (int s =0;s <plane_array.size();++s)
	{
		Plane3 *plane = plane_array[s];
		if (plane->ref)
		{
			ConvexEdge *edge = plane->head;
			if (plane->tail)plane->tail->next = nullptr;
			while (edge)
			{
				ConvexEdge *next_edge = edge->next;
				memory_slab.release(edge);
				edge = next_edge;
			}
			operate_queue.remove(plane, compare_func, modify_func);

			plane->head = plane->tail = nullptr;
			memory_slab.release(plane);
		}
	}
}

bool quick_hull_algorithm3d(const std::vector<cocos2d::Vec3> &points, std::vector<Plane3 *> &operate_queue)
{
	int array_size = points.size();
	ConvexHullMemmorySlab memory_slab(array_size);
	priority_queue<Plane3*>  plane_priority(array_size,true);

	std::function<int(Plane3 *&plane, int index_queue)> modify_func = [](Plane3 *&plane,int index_queue)->int {
		if (index_queue != -1)
			plane->index_queue = index_queue;
		return plane->index_queue;
	};

	std::function<bool(Plane3 *const &a, Plane3 *const &b)> compare_func = [](Plane3 *const &a,Plane3 *const &b)->bool {
		return a->operate_array.size() > b->operate_array.size();
	};

	static_create_tetrahedron(points, plane_priority,compare_func,modify_func);

	link_list<ConvexEdge*>  horizontal_edge;
	Plane3 *perform_hull = nullptr;
	//����ʹ�öѽṹ�����㷨�Ĳ��ҹ���
	while (plane_priority.size() && (perform_hull = plane_priority.head())->operate_array.size())
	{
		//������������ԭƽ����ߵ��Ǹ�����
		assert(perform_hull->high_v != -1);
		//�Ӹõ�۲��б������е�ƽ��,����õ���ĳЩƽ�������,����Ҫ���¼ܹ�
		quick_hull_build_new_plane(memory_slab, horizontal_edge,points, plane_priority, perform_hull->high_v, compare_func, modify_func);
		horizontal_edge.clear();
	}

	int queue_size = plane_priority.size();
	if (queue_size)
	{
		operate_queue.resize(queue_size);
		memcpy(operate_queue.data(),plane_priority.data(),sizeof(Plane3*)*queue_size);
	}
	return queue_size != 0;
}
/*
  *ע��,�ú�������������źܴ�Ĳ�ͬ
 */
void static_create_tetrahedron(const std::vector<Vec3> &points, link_list<Plane3*> &operate_queue,std::map<short, red_black_tree<Plane3*>> &point_to_face,std::vector<short> &remind_index_array, 
	std::function<int(const short &a, const short &b)> &compare_func, std::function<int(Plane3 *const &plane1, Plane3 *const &plane2)> &compare_func2, 
	red_black_tree_alloc<red_black_tree<short>::internal_node, short>  &point_mem_alloc, red_black_tree_alloc<red_black_tree<Plane3*>::internal_node, Plane3 *>  &plane_mem_alloc)
{
	int array_size = points.size();
	//��һ����Ҫ������ض���Ϊ���յ�͹���ϵ����������ı�
	int boundary_array[6] = { 0 };
	for (int index_l = 1; index_l < array_size; ++index_l)
	{
		const Vec3 &point = points[index_l];
		//-X
		if (points[boundary_array[0]].x > point.x)boundary_array[0] = index_l;
		//+X
		if (points[boundary_array[1]].x < point.x) boundary_array[1] = index_l;
		//-Y
		if (points[boundary_array[2]].y > point.y)boundary_array[2] = index_l;
		//+Y
		if (points[boundary_array[3]].y < point.y)boundary_array[3] = index_l;
		//-Z
		if (points[boundary_array[4]].z > point.z)boundary_array[4] = index_l;
		//+Z
		if (points[boundary_array[5]].z < point.z)boundary_array[5] = index_l;
	}
	short v1 = boundary_array[0], v2 = boundary_array[5], v3 = boundary_array[1], v4 = boundary_array[3];
	//������ѡ�еĵ���ѡ������,����һ��ƽ��,������Ҫ����ļ��һ�·�ֹ���˵��������,����������ƽ��������
	Vec3 normal = cross_normalize(points[v1], points[v2], points[v3]);
	float f = dot(normal, points[v4] - points[v1]);
	bool b_normal = Vec3::ZERO != normal && f > 0.0f;
	if (!b_normal)
	{
		b_normal = true;
		if (f < 0.0f)
		{
			std::swap(v2, v3);
			normal = -normal;
		}
		else
		{
			v2 = boundary_array[1]; v3 = boundary_array[4];
			normal = cross_normalize(points[v1], points[v2], points[v3]);
			f = dot(normal, points[v4] - points[v1]);
			b_normal = Vec3::ZERO != normal && f > 0.0f;
		}
	}
	if (!b_normal)
	{
		b_normal = true;
		if (f < 0.0f)
		{
			std::swap(v2, v3);
			normal = -normal;
		}
		else
		{
			v1 = boundary_array[1]; v2 = boundary_array[3]; v3 = boundary_array[2];
			normal = cross_normalize(points[v1], points[v2], points[v3]);
			f = dot(normal, points[v4] - points[v1]);
			b_normal = Vec3::ZERO != normal && f > 0.0f;
		}
	}
	assert(b_normal);
	assert(dot(points[v4] - points[v1], normal) > 0.0f);
	for (int j = 0,base_j = 0; j < array_size; ++j)
	{
		if (j != v1 && j != v2&&j != v3&&j != v4)
			remind_index_array[base_j++] = j;
	}
	//��������Ĺ�����̱Ƚϸ���
	Plane3  *plane1 = new Plane3(v1, v2, v4,&point_mem_alloc);//a,b
	ConvexEdge  *ab1 = new ConvexEdge(v1, v2, plane1);
	ConvexEdge  *ab2 = new ConvexEdge(v2, v4, plane1);
	ConvexEdge  *ab3 = new ConvexEdge(v4, v1, plane1);
	ab1->next = ab2; ab2->prev = ab1;
	ab2->next = ab3; ab3->prev = ab2;
	ab3->next = ab1; ab1->prev = ab3;
	plane1->head = ab1; plane1->tail = ab3;

	Plane3 *plane2 = new Plane3(v2, v3, v4, &point_mem_alloc);
	ConvexEdge *bc1 = new ConvexEdge(v2, v3, plane2);
	ConvexEdge *bc2 = new ConvexEdge(v3, v4, plane2);
	ConvexEdge *bc3 = new ConvexEdge(v4, v2, plane2);
	bc1->next = bc2; bc2->prev = bc1;
	bc2->next = bc3; bc3->prev = bc2;
	bc3->next = bc1; bc1->prev = bc3;
	plane2->head = bc1; plane2->tail = bc3;

	Plane3 *plane3 = new Plane3(v3, v1, v4, &point_mem_alloc);
	ConvexEdge *ca1 = new ConvexEdge(v3, v1, plane3);
	ConvexEdge *ca2 = new ConvexEdge(v1, v4, plane3);
	ConvexEdge	*ca3 = new ConvexEdge(v4, v3, plane3);
	ca1->next = ca2; ca2->prev = ca1;
	ca2->next = ca3; ca3->prev = ca2;
	ca3->next = ca1; ca1->prev = ca3;
	plane3->head = ca1; plane3->tail = ca3;

	Plane3 *plane4 = new Plane3(v1, v3, v2, &point_mem_alloc);
	ConvexEdge *le1 = new ConvexEdge(v1, v3, plane4);
	ConvexEdge *le2 = new ConvexEdge(v3, v2, plane4);
	ConvexEdge *le3 = new ConvexEdge(v2, v1, plane4);
	le1->next = le2; le2->prev = le1;
	le2->next = le3; le3->prev = le2;
	le3->next = le1; le1->prev = le3;
	plane4->head = le1; plane4->tail = le3;
	//ͼԪ���֮��Ĺ���,һ��6��
	ab1->twin = le3; le3->twin = ab1;
	ab2->twin = bc3; bc3->twin = ab2;
	ab3->twin = ca2; ca2->twin = ab3;

	bc1->twin = le2; le2->twin = bc1;
	bc2->twin = ca3; ca3->twin = bc2;
	ca1->twin = le1; le1->twin = ca1;

	const Vec3 &base_point = points[v4];
	plane1->normal = plane_normal(plane1);
	plane2->normal = plane_normal(plane2);
	plane3->normal = plane_normal(plane3);
	plane4->normal = plane_normal(plane4);

	//�ڱ����Ĺ�����,�㷨�����������ֵ��
	Plane3 *plane_array[4];
	int           plane_array_size = 0;
	for (int index_l = 0; index_l < array_size - 4; ++index_l)
	{
		int base_j = remind_index_array[index_l];
		Vec3 interpolation = points[base_j] - base_point;
		float f1 = dot(interpolation, plane1->normal), f2, f3, f4;
		//���˵������׼����ƽ��ǳ�С�ĵ�
		if (fabsf(f1) > 0.001f && f1 > 0.0f)
		{
			plane1->point_set.insert(base_j,compare_func);
			plane_array[plane_array_size++] = plane1;
		}
		if (fabsf(f2 = dot(interpolation, plane2->normal)) > 0.001f && f2 > 0.0f)
		{
			plane2->point_set.insert(base_j, compare_func);
			plane_array[plane_array_size++] = plane2;
		}
		if (fabsf(f3 = dot(interpolation, plane3->normal)) > 0.001f && f3 > 0.0f)
		{
			plane3->point_set.insert(base_j, compare_func);
			plane_array[plane_array_size++] = plane3;
		}
		if (fabsf(f4 = dot(points[base_j] - points[v1], plane4->normal)) > 0.001f && f4 > 0.0f)
		{
			plane4->point_set.insert(base_j, compare_func);
			plane_array[plane_array_size++] = plane4;
		}

		if (plane_array_size)
		{
			auto &balance_tree =  point_to_face[base_j] = red_black_tree<Plane3*>(0,&plane_mem_alloc);
			for (int j = 0; j < plane_array_size; ++j)
				balance_tree.insert(plane_array[j], compare_func2);
		}
		plane_array_size = 0;
	}

	plane1->other_ptr = operate_queue.push_back(plane1);
	plane2->other_ptr =  operate_queue.push_back(plane2);
	plane3->other_ptr =  operate_queue.push_back(plane3);
	plane4->other_ptr =  operate_queue.push_back(plane4);
}

void convex_hull_build_new_plane(const std::vector<Vec3> &points,int select_index,ConvexHullMemmorySlab &mem_slab,red_black_tree<Plane3*> &plane_remove,
	link_list<ConvexEdge *> &horizontal_edge,link_list<Plane3*> &operate_queue,
	std::map<short,red_black_tree<Plane3*>> &point_to_face,
	std::function<int(const short &a, const short &b)> &compare_func, std::function<int(Plane3 *const &plane1, Plane3 *const &plane2)> &compare_func2,
	red_black_tree_alloc<red_black_tree<short>::internal_node, short>  &point_mem_alloc, red_black_tree_alloc<red_black_tree<Plane3*>::internal_node, Plane3 *>  &plane_mem_alloc)
{
	int array_size = points.size();
	int plane_size = plane_remove.size();
	char *b_found_array = mem_slab._global_memory;
	const Vec3 &base_point = points[select_index];
	//����ÿһ����ɾ����ƽ��,�������ƽ�߼���
	int c1 = 0, c2 = 0;
	for (auto jt = plane_remove.find_minimum(); jt; jt = plane_remove.find_next(jt))
	{
		Plane3 *plane = jt->tw_value;
		//�����߽�,�鿴�Ƿ��ж��ڵ�ǰĿ�����˵���ɼ����ڽ�ƽ��
		ConvexEdge *edge = plane->head;
		do
		{
			Plane3 *plane_adj = edge->twin->owner;
			float f = dot(base_point - points[plane_adj->v1],plane_adj->normal);

			if (f < 0.0f)
				insert_convex_hull_edge(horizontal_edge, edge->twin);

			edge = edge->next;
		} while (edge != plane->head);
	}
	assert(assert_convex_hull_edge_continuous(horizontal_edge));
	//���ڵ�ƽ�߱ߵļ���,����������ص�ƽ��
	auto *it_ptr = horizontal_edge.head();
	Plane3 *plane_last = nullptr, *plane_head = nullptr;
	for (; it_ptr; it_ptr = horizontal_edge.next(it_ptr))
	{
		ConvexEdge *edge = it_ptr->tv_value;
		Plane3 *plane_twin = edge->twin->owner;
		Plane3 *plane_old = edge->owner;
		//�����µ�ƽ��
		Plane3 *plane_new = mem_slab.apply(select_index, edge->v2, edge->v1,&plane_mem_alloc);
		ConvexEdge *e1 = mem_slab.apply(select_index, edge->v2, plane_new);
		ConvexEdge *e2 = mem_slab.apply(edge->v2, edge->v1, plane_new);
		ConvexEdge *e3 = mem_slab.apply(edge->v1, select_index, plane_new);

		e1->next = e2; e2->prev = e1;
		e2->next = e3; e3->prev = e2;
		e3->next = e1; e1->prev = e3;
		plane_new->head = e1; plane_new->tail = e3;

		e2->twin = edge; edge->twin = e2;
		if (plane_last)
		{
			plane_last->head->twin = e3;
			e3->twin = plane_last->head;
		}

		if (!plane_head)
			plane_head = plane_new;

		plane_last = plane_new;
		//��Ե�ǰƽ��,���ͻ���ѡ����Ҫ���빫����edge���ڽӵ�����ƽ���н���ɸѡ,�˴�������
		plane_new->normal = plane_normal(plane_new);
		const Vec3 &normal = plane_new->normal;
		//��һ��ƽ��
		red_black_tree<short> &operate_array2 = plane_twin->point_set;
		for (auto it = operate_array2.find_minimum(); it; it = operate_array2.find_next(it))
		{
			int base_j = it->tw_value;
			float f = dot(points[base_j] - base_point, normal);
			if (fabsf(f) > 0.001f && f > 0.0f)
			{
				plane_new->point_set.insert(base_j,compare_func);
				auto it2 = point_to_face.find(base_j);
				//������Ҫ���ӷ���ӳ��
				it2->second.insert(plane_new,compare_func2);
			}
		}
		//������ƽ��ĵ����ɸѡ
		red_black_tree<short> &operate_array1 = plane_old->point_set;
		for (auto jt = operate_array1.find_minimum(); jt; jt = operate_array1.find_next(jt))
		{
			int base_j = jt->tw_value;
			float f = dot(points[base_j] - base_point, normal);
			if (fabsf(f) > 0.001f && f > 0.0f)
			{
				plane_new->point_set.insert(base_j,compare_func);
				auto it = point_to_face.find(base_j);
				it->second.insert(plane_new,compare_func2);
			}
		}
		assert(plane_new->tail && plane_new->head);
		plane_new->other_ptr = operate_queue.push_back(plane_new);
	}
	//��Ե�һ���Լ����һ��ƽ��,��ĳЩ��صı���Ҫ�ٴ�����
	plane_last->head->twin = plane_head->tail;
	plane_head->tail->twin = plane_last->head;
	//ɾ��Ŀ��Сƽ��
	for (auto jt = plane_remove.find_minimum(); jt; jt = plane_remove.find_minimum())
	{
		Plane3 *plane = jt->tw_value;
		for (auto it = plane->point_set.find_minimum(); it;it = plane->point_set.find_next(it))
		{
			auto pt = point_to_face.find(it->tw_value);
			//assert(st->second.erase(plane));//ɾ��Ԫ�ر����ǳɹ���
			pt->second.remove(plane, compare_func2);
		}
		//������Դ
		ConvexEdge *edge = plane->head;
		if (plane->tail)plane->tail->next = nullptr;
		while (edge)
		{
			ConvexEdge *next_edge = edge->next;
			mem_slab.release(edge);
			edge = next_edge;
		}
		operate_queue.remove(plane->other_ptr);
		plane->head = plane->tail = nullptr;
		mem_slab.release(plane);
	}
}

bool convex_hull_3d_optimal(const std::vector<cocos2d::Vec3> &points, std::vector<Plane3 *> &planes)
{
	int array_size = points.size();
	link_list<Plane3*> operate_queue;
	link_list<ConvexEdge*> horizontal_edge;
	std::function<int(const short &a, const short &b)> compare_func = [](const short &a, const short &b)->int {
		return a > b ? 1 : (a < b?-1:0);
	};
	std::function<int(Plane3 *const &plane1, Plane3 *const &plane2)> compare_func2 = [](Plane3 *const &plane1, Plane3 *const &plane2)->int {
		return plane1 > plane2 ? 1:(plane1 < plane2?-1:0);
	};
	//���㵽���ӳ��,�Լ�ƽ�浽�����ӳ��
	std::map<short, red_black_tree<Plane3*>> point_to_face;
	std::vector<short>  remind_index_array(array_size - 4);
	//�ڴ����������
	ConvexHullMemmorySlab memory_slab(array_size);
	red_black_tree_alloc<red_black_tree<Plane3*>::internal_node, Plane3 *>  plane_mem_alloc(1024);
	red_black_tree_alloc<red_black_tree<short>::internal_node, short>  point_mem_alloc(1024);

	//���ȴ���һ��������
	static_create_tetrahedron(points, operate_queue,point_to_face, remind_index_array, compare_func, compare_func2, point_mem_alloc, plane_mem_alloc);
	//����ʣ�µĵ㼯��,����ı���,������������Ӧ�Ŀɼ�ƽ����������¼���
	for (int j = 0; j < array_size - 4; ++j)
	{
		int base_j = remind_index_array[j];
		//������ǰ�㼯������Сƽ��,����ֵĻ�
		auto it = point_to_face.find(base_j);
		if (it != point_to_face.end() && it->second.size())
		{
			convex_hull_build_new_plane(points, base_j, memory_slab, it->second, horizontal_edge, operate_queue, point_to_face,compare_func,compare_func2,point_mem_alloc,plane_mem_alloc);
			point_to_face.erase(it);
			horizontal_edge.clear();
		}
	}

	int j = 0;
	planes.resize(operate_queue.size());
	for (auto *it_ptr = operate_queue.head(); it_ptr; it_ptr = operate_queue.next(it_ptr))
	{
		Plane3 *plane = it_ptr->tv_value;
		planes[j++] = plane;
	}

	return planes.size() != 0;
}
NS_GT_END