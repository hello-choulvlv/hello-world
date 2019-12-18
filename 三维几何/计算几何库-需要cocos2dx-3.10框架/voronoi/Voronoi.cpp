/*
  *Voronoi图算法 + Delaunay三角剖分算法集合
  *2019年12月18日
  *@author:xiaohuaxiong
 */
#include "Voronoi.h"
#include<functional>
#include<list>
#include<set>
#include<map>
#include<string>
#include "matrix/matrix.h"
#include "cycle_sphere/cycle_sphere.h"

using namespace cocos2d;
NS_GT_BEGIN

bool operator==(const DelaunayEdge &a, const DelaunayEdge &other)
{
	return a.v1 == other.v1 && a.v2 == other.v2 || a.v1 == other.v2 && a.v2 == other.v1;
}

bool operator<(const DelaunayEdge &a, const DelaunayEdge &other)
{
	short u1 = min_f(a.v1,a.v2);
	short u2 = max_f(a.v1,a.v2);

	short w1 = min_f(other.v1, other.v2);
	short w2 = max_f(other.v1, other.v2);

	return u1 < w1 || u1 == w1 && u2 < w2;
}

bool operator >(const DelaunayEdge &a, const DelaunayEdge &other)
{
	short u1 = min_f(a.v1, a.v2);
	short u2 = max_f(a.v1, a.v2);

	short w1 = min_f(other.v1, other.v2);
	short w2 = max_f(other.v1, other.v2);

	return u1 > w1 || u1 == w1 && u2 > w2;
}

void rect_outerline_triangle(const cocos2d::Vec2 &origin, const cocos2d::Vec2 &extent, cocos2d::Vec2 triangle[3])
{
	float delta_h = extent.y > 200 ?extent.y * 0.2: 10;
	float delta_w = extent.x / extent.y * delta_h;
	float half_w = extent.x * 0.5f;

	triangle[0].x = origin.x - half_w - delta_w;
	triangle[0].y = origin.y - delta_h;

	triangle[1].x = origin.x + 3.0f * half_w + delta_w;
	triangle[1].y = origin.y - delta_h;

	triangle[2].x = origin.x + half_w;
	triangle[2].y = origin.y + 2.0f * extent.y + delta_h;
}

void static_create_cycle_by_triangle(Cycle &cycle,const std::vector<Vec2> &disper_points,const DelaunayTriangle &delaunay_triangle)
{
	const Vec2 &a = disper_points[delaunay_triangle.v1];
	const Vec2 &b = disper_points[delaunay_triangle.v2];
	const Vec2 &c = disper_points[delaunay_triangle.v3];

	Vec2  pb = b - a;
	Vec2  pc = c - a;
	Vec2  bc = c - b;

	float	length_ab = length(pb);
	float   length_ac = length(pc);
	float   length_bc = length(bc);

	float  cos_v = pb.dot(pc) / (length_ab * length_ac);
	float  sin_v = sqrtf(1.0f - cos_v * cos_v);
	cycle.radius = length_bc / (2.0f * sin_v);
	//圆心
	float ax_cx = a.x - c.x;
	float bx_cx = b.x - c.x;
	float by_cy = b.y - c.y;
	float ay_cy = a.y - c.y;

	float ax_a_cx = a.x + c.x;
	float ay_a_cy = a.y + c.y;
	float bx_a_cx = b.x + c.x;
	float by_a_cy = b.y + c.y;

	float   det_f = 2.0f * (ax_cx * by_cy - ay_cy * bx_cx);
	float   x = (ax_cx * ax_a_cx + ay_cy * ay_a_cy) * by_cy - (bx_cx * bx_a_cx + by_cy * by_a_cy) * ay_cy;
	float  y = ax_cx * (bx_cx * bx_a_cx + by_cy * by_a_cy) - bx_cx * (ax_cx * ax_a_cx + ay_cy * ay_a_cy);

	cycle.center.x = x / det_f;
	cycle.center.y = y / det_f;
}

void delaunay_triangulate_bowyer_washton(const std::vector<cocos2d::Vec2> &disper_points, std::vector<DelaunayTriangle> &triangle_sequence, int &real_size)
{
	int array_size = disper_points.size();
	//第一步先对顶点的顺序进行排序
	std::function<bool(const int, const int)> compare_func = [disper_points](const int la, const int lb)->bool {
		return disper_points[la].x < disper_points[lb].x || disper_points[la].x == disper_points[lb].x && disper_points[la].y > disper_points[lb].y;
	};

	std::vector<int>  disper_index_array(array_size);
	for (int j = 0; j < disper_points.size(); ++ j)disper_index_array[j] = j;
	quick_sort_origin_type<int>(disper_index_array.data(), array_size - 3,compare_func);

	//计算目标三角形序列
	struct DelaunayTriangle  delaunay_triangle = { short(array_size -3),short(array_size -2),short(array_size -1)};
	std::list<DelaunayTriangle>  triangle_queue;
	triangle_queue.push_back(delaunay_triangle);
	struct Cycle cycle;
	DelaunayEdge   delaunay_edge;

	for (int index_l = 0; index_l < array_size - 3; ++index_l)
	{
		int base_j = disper_index_array[index_l];
		const Vec2 &target_point = disper_points[base_j];
		//初始化边缓存,此缓存需要用到set
		//std::set<DelaunayEdge>  triangle_edge_set;
		std::map<DelaunayEdge, int>  triangle_edge_map;
		//针对每一个待定的三角形,逐个的检测
		for (auto it = triangle_queue.begin(); it != triangle_queue.end();)
		{
			//求三角形的外接圆
			auto &check_triangle = *it;
			static_create_cycle_by_triangle(cycle, disper_points, check_triangle);
			//如果该点在圆的右侧,则说明目标三角形已经确定了.
			if (target_point.x > cycle.center.x + cycle.radius)
			{
				vector_fast_push_back(triangle_sequence, check_triangle);
				it = triangle_queue.erase(it);
			}
			else if (check_point_insideof_cycle(cycle, target_point))//此时点在相关的三角形内,需要再次分解三角形
			{
				delaunay_edge.v1 = check_triangle.v1, delaunay_edge.v2 = check_triangle.v2;
				auto ft = triangle_edge_map.find(delaunay_edge);
				if (ft == triangle_edge_map.end())
					triangle_edge_map[delaunay_edge] = 0;
				else
					++ft->second;

				delaunay_edge.v1 = check_triangle.v2, delaunay_edge.v2 = check_triangle.v3;
				ft = triangle_edge_map.find(delaunay_edge);
				if (ft == triangle_edge_map.end())
					triangle_edge_map[delaunay_edge] = 0;
				else
					++ft->second;

				delaunay_edge.v1 = check_triangle.v3, delaunay_edge.v2 = check_triangle.v1;
				ft = triangle_edge_map.find(delaunay_edge);
				if (ft == triangle_edge_map.end())
					triangle_edge_map[delaunay_edge] = 0;
				else
					++ft->second;
				it = triangle_queue.erase(it);
			}
			else
				++it;
		}
		//针对所有的边,与目标点建立新的三角形
		for (auto ft = triangle_edge_map.begin(); ft != triangle_edge_map.end(); ++ft)
		{
			if (!ft->second)
			{
				delaunay_triangle.v1 = base_j, delaunay_triangle.v2 = ft->first.v1, delaunay_triangle.v3 = ft->first.v2;
				triangle_queue.push_back(delaunay_triangle);
			}
		}
	}
	for (auto it = triangle_queue.begin(); it != triangle_queue.end(); ++it)
		vector_fast_push_back(triangle_sequence, *it);
	//最后一步,去除掉与外接三角形相关的边
	real_size = triangle_sequence.size();
	real_size = 0;
	int boundary_l = array_size - 3;
	for (int index_l = 0; index_l < triangle_sequence.size(); ++index_l)
	{
		auto &triangle = triangle_sequence[index_l];
		if (triangle.v1 < boundary_l && triangle.v2 < boundary_l && triangle.v3 < boundary_l)
		{
			if (index_l != real_size)
				triangle_sequence[real_size] = triangle;
			++real_size;
		}
	}
}

NS_GT_END