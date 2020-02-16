/*
  *凸包算法实现,2d+3d
  *有一部分代码是从原来的point_polygon目录中分离出来的
  *2020年2月13日5
 */
#include "convex_hull.h"
#include "matrix/matrix.h"
#include "data_struct/link_list.h"
#include <list>
#include<set>
using namespace cocos2d ;

NS_GT_BEGIN
bool polygon_compute_convex_hull(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon_points)
{
	if (points.size() < 3) return false;
	//第一步,对顶点进行排序
	std::vector<Vec2> points_tmp = points;
	//查找y坐标最小,或者x坐标较小的点
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
		//如果角度相同,则离参考点距离最近的排列位置更靠前
		return angle_a < angle_b || (angle_a == angle_b && d_x * d_x + d_y * d_y < f_x * f_x + f_y * f_y);//避免极角相同的点集
	};
	quick_sort<Vec2>(points_tmp.data(), (int)points_tmp.size(), compare_func);

	polygon_points.reserve(points.size());
	polygon_points.push_back(f_point);
	polygon_points.push_back(points_tmp[0]);

	for (int index_j = 1; index_j < points_tmp.size(); ++index_j)
	{
		//对当前的点进行侦测
		Vec2 &target_point = points_tmp.at(index_j);
		//检测是否与以前的向量的走向是一致的
		while (polygon_points.size() > 1 && sign_area(polygon_points.back() - polygon_points[polygon_points.size() - 2], target_point - polygon_points.back()) <= 0)
		{
			polygon_points.pop_back();
		}
		polygon_points.push_back(target_point);
	}

	return polygon_points.size() >= 3;
}

//quick hull算法实现
bool quick_hull_algorithm2d(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon)
{
	//第一步计算左下角与右上角的顶点,此两个顶点必然构成凸多边形的两个支撑点
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
	//初次对所有的顶点进行划分,并生成两个集合
	std::list<QuickHull>   stack_polygon;
	const Vec2 &base_point = points[base_t];
	Vec2  direction = base_point - points[base_l];
	//需要加入两次
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
	//针对所有的顶点,进行划分
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
	//针对所有的堆栈进行处理
	while (stack_polygon.size() > 0)
	{
		QuickHull &quick_hull = stack_polygon.back();
		//此时该边已经没有归属顶点了,需要移除掉,并把该边的低端的端点写入到多边形的点集合中
		if (!quick_hull.operate_points.size())
		{
			polygon.push_back(points.at(quick_hull.start_l));
			stack_polygon.pop_back();
		}
		else//否则需要处理该边所属的所有的候选顶点,注意如果需要分裂边,则分裂后的顺序必须得是逆时针的,否则最后得出的多边形顶点将是错乱的
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
			//选中了某一点后,就需要分裂边,注意分裂的次序
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
			//另一条边
			const Vec2  d2 = Vec2(points.at(quick_hull.final_l).y - other_point.y, other_point.x - points.at(quick_hull.final_l).x);
			//对顶点进行划分
			for (int index_l = 0; index_l < quick_hull.operate_points.size(); ++index_l)
			{
				int  target_index = quick_hull.operate_points[index_l];
				if (target_index != target_l)
				{
					const Vec2 interpolation = points.at(target_index) - other_point;
					//两个结果不可能同时成立
					if (dot(direction, interpolation) > 0)
						last_hull.operate_points.push_back(target_index);
					else if (dot(d2, interpolation) > 0)
						secondary_hull.operate_points.push_back(target_index);
				}
			}
			//移除掉原来的
			stack_polygon.pop_back();
			stack_polygon.push_back(secondary_hull);
			stack_polygon.push_back(last_hull);
		}
	}
	return polygon.size() >= 3;
}
#define plane_normal(plane) cross_normalize(points[plane->v1],points[plane->v2],points[plane->v3])
void static_create_tetrahedron(const std::vector<Vec3> &points,std::list<Plane3*> &maturity)
{
	int array_size = points.size();
	//第一步需要计算出必定成为最终的凸壳上的三条连续的边
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
	//从以上选中的点中选出三个,构成一个平面,这里需要额外的检查一下防止极端的情况出现,比如类似于平行六面体
	Vec3 normal = cross_normalize(points[v1], points[v2], points[v3]);
	bool b_normal = Vec3::ZERO != normal && dot(normal,points[v4] - points[v1]) != 0.0f;
	if (!b_normal)
	{
		v2 = boundary_array[1]; v3 = boundary_array[4];
		normal = cross_normalize(points[v1], points[v2], points[v3]);
		b_normal = Vec3::ZERO != normal && dot(normal,points[v4] - points[v1]) != 0.0f;
	}
	if (!b_normal)
	{
		v1 = boundary_array[1]; v2 = boundary_array[3]; v3 = boundary_array[2];
		normal = cross_normalize(points[v1], points[v2], points[v3]);
		b_normal = Vec3::ZERO != normal && dot(normal, points[v4] - points[v1]) != 0.0f;
	}
	assert(b_normal);
	assert(dot(points[v4] - points[v1],normal) > 0.0f);
	//该四面体的构造过程比较复杂
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
	//图元与边之间的关联,一共6对
	ab1->twin = le3; le3->twin = ab1;
	ab2->twin = bc3; bc3->twin = ab2;
	ab3->twin = ca2; ca2->twin = ab3;

	bc1->twin = le2; le2->twin = bc1;
	bc2->twin = ca3; ca3->twin = bc2;
	ca1->twin = le1; le1->twin = ca1;

	maturity.push_back(plane1);
	maturity.push_back(plane2);
	maturity.push_back(plane3);
	maturity.push_back(plane4);

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
		//过滤掉距离标准测试平面非常小的点
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
}

//检测是否有某一个平面还有尚未处理完毕的冲突点集
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
//将相关的边插入到队列中
bool insert_convex_hull_edge(std::list<ConvexEdge *> &horizontal_edge,ConvexEdge *target)
{
	auto it = horizontal_edge.begin();
	for (; it != horizontal_edge.end(); ++it)
	{
		ConvexEdge *edge = *it;
		//有两种情况
		if (edge->v2 == target->v1)
		{
			horizontal_edge.insert(++it, target);
			return true;
		}
		else if (target->v2 == edge->v1)//此时目标边置于当前边的位置之前
		{
			horizontal_edge.insert(it,target);
			return true;
		}
	}
	horizontal_edge.push_back(target);
	return false;
}
bool assert_convex_hull_edge_continuous(std::list<ConvexEdge *> &horizontal_edge)
{
	bool b_continus = true;
	auto it = horizontal_edge.begin();
	ConvexEdge *edge_head = nullptr,*edge_final = nullptr;

	for (; it != horizontal_edge.end(); ++it)
	{
		ConvexEdge *edge = *it;
		if (!edge_head)edge_head = edge;
		else
			b_continus &= edge_final->v2 == edge->v1;
		edge_final = edge;
	}
	b_continus &= edge_final->v2 == edge_head->v1;
	return b_continus;
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
//针对某一个被选中的平面,检测所有的可见平面,在必要的时候需要重新合并新的平面
void quick_hull_build_new_plane(ConvexHullMemmorySlab &memory_slab,const std::vector<cocos2d::Vec3> &points, std::list<Plane3*> &operate_queue,Plane3 *target_plane,int selected_index)
{
	//检查所有的可见平面,以及那些地平线两侧的平面
	std::list<ConvexEdge *> horizontal_edge;
	//该点被成为视点
	const Vec3 &base_point = points[selected_index];
	int array_size = points.size();
	std::vector<int> volumn_array = target_plane->operate_array;
	char *b_found_array = memory_slab._global_memory;
	memset(b_found_array, 0, array_size);
	unsigned short *index_array = memory_slab._index_array;
	int index_array_size = 0;

	int s_size = target_plane->operate_array.size();
	int *operate_array = target_plane->operate_array.data();
	for (int j = 0; j < s_size; ++j)
	{
		int base_j = operate_array[j];
		if (!b_found_array[base_j])
		{
			b_found_array[base_j] = 1;
			index_array[index_array_size++] = base_j;
		}
	}

	for (auto it = operate_queue.begin(); it != operate_queue.end(); ++it)
	{
		Plane3 *plane = *it;
		const Vec3 &normal = plane->normal;// cross_normalize(points[plane->v1], points[plane->v2], points[plane->v3]);
		float f = dot(base_point - points[plane->v1],normal);
		//检测是否是可见的平面
		if (f > 0.0f)
		{
			plane->ref = 1;//注意这个标志,将意味着相关的平面会被干掉
			//volumn_array.insert(volumn_array.end(),plane->operate_array.begin(),plane->operate_array.end());
			auto *operate_array = plane->operate_array.data();
			int s_size = plane->operate_array.size();
			for (int j = 0; j < s_size; ++j)
			{
				int base_j = operate_array[j];
				if (!b_found_array[base_j])
				{
					b_found_array[base_j] = 1;
					index_array[index_array_size++] = base_j;
				}
			}
		}
		else if (f < 0.0f)
		{
			//检测与该平面的邻接平面是否对视点可见
			ConvexEdge *edge = plane->head;
			int loop_count = 0;
			do 
			{
				Plane3 *adj_plane = edge->twin->owner;
				float f2 = dot(base_point - points[adj_plane->v1], adj_plane->normal);
				//邻接平面对视点的可见性检测
				if (f2 > 0.0f)
				{
					//经过实验,有时候一个原单纯形是可以贡献两条边的,此处引发了一个巨大的坑
					//此时需要找出连续的地平线边,关于该边对应的平面,以及邻接平面,可以由该边的相关数据直接获得
					insert_convex_hull_edge(horizontal_edge, edge);
				}
				edge = edge->next;
			} while (/*!target_edge &&*/ edge != plane->head);//为了代码的整洁
			
		}
	}
	//assert(assert_convex_hull_edge_continuous(horizontal_edge));
	//对于已经检测出的可见平面与地平线,则需要另外生成一些新的平面
	Plane3 *plane_last = nullptr,*plane_head = nullptr;
	for (auto it = horizontal_edge.begin(); it != horizontal_edge.end(); ++it)
	{
		ConvexEdge *edge = *it;
		Plane3 *plane_twin = edge->twin->owner;
		//生成新的平面
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
		operate_queue.push_back(plane_new);
		//针对当前平面,其冲突点的选择需要从与公共边edge相邻接的两个平面中进行筛选,此处不成立
		plane_new->normal = plane_normal(plane_new);
		const Vec3 &normal = plane_new->normal;// plane_normal(plane_new);
		//算法在此处是可以进行优化的,某些平面的可见点注定是与目标平面不可见的,因此经过一些简单的几何运算之后
		//快意快速的判断出点集是否可见
		float df = -FLT_MAX;
		for (int j = 0; j < index_array_size; ++j)
		{
			int base_j = index_array[j];
			b_found_array[base_j] = 1;
			float f = dot(points[base_j] - base_point, normal);
			if (fabsf(f) > 0.001f && f > 0.0f)
			{
				vector_fast_push_back(plane_new->operate_array, base_j);
				if (f > df)
				{
					df = f;
					plane_new->high_v = base_j;
				}
			}
		}
		//还有一部分
		const std::vector<int> &target_array2 = edge->owner->operate_array;
		int array_size2 = target_array2.size();
		for (int j = 0; j < array_size2; ++j)
		{
			int base_j = target_array2[j];
			if (!b_found_array[base_j])
			{
				float f = dot(points[base_j] - base_point, normal);
				if (fabsf(f) > 0.001f && f > 0.0f)
				{
					vector_fast_push_back(plane_new->operate_array, base_j);
					if (f > df)
					{
						df = f;
						plane_new->high_v = base_j;
					}
				}
			}
		}
	}
	//针对第一个以及最后一个平面,其某些相关的边需要再次连接
	plane_last->head->twin = plane_head->tail;
	plane_head->tail->twin = plane_last->head;
	//在最后删除所有的已经标记的平面
	for (auto it = operate_queue.begin(); it != operate_queue.end();)
	{
		Plane3 *plane = *it;
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
			plane->head = plane->tail = nullptr;
			memory_slab.release(plane);

			it = operate_queue.erase(it);
		}
		else
			++it;
	}
}

bool quick_hull_algorithm3d(const std::vector<cocos2d::Vec3> &points, std::list<Plane3 *> &operate_queue)
{
	int array_size = points.size();
	ConvexHullMemmorySlab memory_slab(array_size);
	static_create_tetrahedron(points, operate_queue);

	Plane3 *perform_hull = nullptr;
	while (check_face_conflict_point(operate_queue,&perform_hull))
	{
		//检索出来距离原平面最高的那个顶点
		const Vec3 normal = plane_normal(perform_hull);
		const std::vector<int> &operate_array = perform_hull->operate_array;
		const Vec3 &base_point = points[perform_hull->v1];
		int operate_size = operate_array.size();

		assert(perform_hull->high_v != -1);
		//从该点观察列表中所有的平面,如果该点在某些平面的正侧,则需要重新架构
		quick_hull_build_new_plane(memory_slab,points, operate_queue,perform_hull, perform_hull->high_v);
	}
	return operate_queue.size() != 0;
}
NS_GT_END