/*
  *离散点集的最远点Voronoi图算法实现
  *2020年6月25日
  *@author:xiaohuaxiong
  *@version 1.0:实现基本的功能
  *@version 2.0:优化内存分配与释放
 */
#include "fx_remote_voronoi.h"
#include "matrix/matrix.h"
#include "line/line.h"
#include <assert.h>
using namespace cocos2d;
NS_GT_BEGIN

bool fx_create_clipper(std::vector<cocos2d::Vec2> &discard_points, link_list<cocos2d::Vec2> &clipper_list) {
	assert(discard_points.size() >= 3);
	Vec2  base_point = discard_points.front();
	int      base_j = 0;
	for (int j = 1; j < discard_points.size(); ++j) {
		if (discard_points[j].y < base_point.y || discard_points[j].y == base_point.y && discard_points[j].x < base_point.x) {
			base_point = discard_points[j];
			base_j = j;
		}
	}
	//swap
	if (base_j != 0) {
		discard_points[base_j] = discard_points[0];
		discard_points[0] = base_point;
	}

	std::function<bool(const Vec2 &, const Vec2 &)> compare_func = [&base_point](const Vec2 &a, const Vec2 &b)->bool {
		float f_x = a.x - base_point.x;
		float f_y = a.y - base_point.y;
		float l_x = b.x - base_point.x;
		float l_y = b.y - base_point.y;

		float a_g = atan2f(f_y,f_x);
		float b_g = atan2f(l_y,l_x);
		return a_g < b_g || a_g == b_g && sqrtf(f_x * f_x + f_y * f_y) > sqrtf(l_x * l_x + l_y * l_y);
	};
	quick_sort<Vec2>(discard_points.data() + 1, discard_points.size() - 1, compare_func);

	clipper_list.push_back(base_point);
	clipper_list.push_back(discard_points[1]);
	clipper_list.push_back(discard_points[2]);

	for (int j = 3; j < discard_points.size(); ++j) {
		const Vec2 &target_point = discard_points[j];
		while(clipper_list.size() > 1 && cross(clipper_list.back()->prev->tv_value, clipper_list.back()->tv_value, target_point) <= 0.0f){
			clipper_list.pop_back();
		}
		clipper_list.push_back(target_point);
	}
	return clipper_list.size() >= 3;
}

void fx_create_remote_voronoi3(link_list<cocos2d::Vec2> &clipper_list, std::vector<FxvSite> &site_array) {
	assert(clipper_list.size() >= 3);
	auto *head_ptr = clipper_list.head();
	FxvSite  *a_site = site_array.data(), *b_site = site_array.data() + 1, *c_site = site_array.data() + 2;
	//第一步,求三点的外接圆
	Vec2  center;
	float r = fx_center_point3(center,a_site->location,b_site->location,c_site->location);
	float length_fx = 10000.0f;
	//a,b,c呈逆时针排列,因此以下代码在这个假设的前提下可以得到极大的化简,边的排列方向呈逆时针
	const Vec2 normal_ab = normalize(a_site->location,b_site->location);
	const Vec2 ortho_ab(-normal_ab.y,normal_ab.x);
	const Vec2 remote_ab = center + ortho_ab * length_fx;

	const Vec2 normal_bc = normalize(b_site->location,c_site->location);
	const Vec2 ortho_bc(-normal_bc.y,normal_bc.x);
	const Vec2 remote_bc = center + ortho_bc * length_fx;

	const Vec2 normal_ca = normalize(c_site->location,a_site->location);
	const Vec2 ortho_ca(-normal_ca.y,normal_ca.x);
	const Vec2 remote_ca = center + ortho_ca * length_fx;
	//边之间的关系构建
	FxvEdge  *a0_edge = new FxvEdge(remote_ca,center,a_site);
	FxvEdge  *a1_edge = new FxvEdge(center, remote_ab, a_site);

	FxvEdge *b0_edge = new FxvEdge(remote_ab,center,b_site);
	FxvEdge *b1_edge = new FxvEdge(center,remote_bc,b_site);

	FxvEdge *c0_edge = new FxvEdge(remote_bc,center,c_site);
	FxvEdge *c1_edge = new FxvEdge(center,remote_ca,c_site);

	a0_edge->next = a1_edge; a1_edge->prev = a0_edge;
	b0_edge->next = b1_edge; b1_edge->prev = b0_edge;
	c0_edge->next = c1_edge; c1_edge->prev = c0_edge;

	a0_edge->twin = c1_edge; c1_edge->twin = a0_edge;
	a1_edge->twin = b0_edge; b0_edge->twin = a1_edge;
	c0_edge->twin = b1_edge; b1_edge->twin = c0_edge;

	a_site->head_ptr = a0_edge;
	a_site->tail_ptr = a1_edge;

	b_site->head_ptr = b0_edge;
	b_site->tail_ptr = b1_edge;

	c_site->head_ptr = c0_edge;
	c_site->tail_ptr = c1_edge;
}

void fx_create_remote_voronoi(std::vector<cocos2d::Vec2> &discard_points, std::vector<FxvSite> &site_array) {
	//第一步,求离散点集的凸壳,凸壳之内的点必定没有最远点Voronoi单元
	link_list<Vec2>  clipper_list;
	fx_create_clipper(discard_points, clipper_list);
	//
	site_array.resize(clipper_list.size());
	auto *it_ptr = clipper_list.head();
	for (int j = 0; j < clipper_list.size(); ++j) {
		site_array[j].location = it_ptr->tv_value;
		it_ptr = it_ptr->next;
	}
	fx_create_remote_voronoi3(clipper_list, site_array);
	//从第4个点开始,逐个的遍历
	const float length_fx = 10000.0f;
	Vec2  intersect_point,intersect_point2;
	FxvSite  *array_ptr = site_array.data(),*ptr2 = site_array.data()+1,*ptr3 = site_array.data() + 2;
	for (int base_j = 3;base_j < clipper_list.size() ; ++base_j) {
		//首先获取其前驱节点,由prev_site指向now_site的直线的正交射线与prev_site所对应的voronoi单元的交点
		FxvSite &now_site = site_array[base_j];
		FxvSite &prev_site = site_array[base_j - 1];
		
		const Vec2 origin = (prev_site.location + now_site.location) * 0.5f;
		const Vec2 normal = normalize(prev_site.location,now_site.location);
		const Vec2 ortho(-normal.y,normal.x);
		const Vec2 destination = origin + ortho * length_fx;

		FxvEdge  *edge_ptr = prev_site.head_ptr;
		//遍历,查找第一个不相交的边,从edge_ptr到第一个相交的边形成的交点到新的边,这些序列构成了prev_site的新的Voronoi单元
		while (edge_ptr != nullptr && !segment_segment_intersect_test(edge_ptr->origin, edge_ptr->destination, origin, destination, intersect_point)) {
			edge_ptr = edge_ptr->next;
		}
		assert(edge_ptr != nullptr);
		//先记录下后续的边,在稍后我们将会将他们一起释放掉
		//FxvEdge  *secondary_edge = edge_ptr->next;
		edge_ptr->destination = intersect_point;
		FxvEdge *suffix_edge = new FxvEdge(intersect_point,destination,&prev_site);
		edge_ptr->next = suffix_edge; suffix_edge->prev = edge_ptr;
		prev_site.tail_ptr = suffix_edge;
		//同时需要为当前的Voronoi单元生成新的边
		FxvEdge *new_edge = new FxvEdge(destination,intersect_point,&now_site);
		now_site.head_ptr = new_edge;
		//新的邻接关系接入
		suffix_edge->twin = new_edge; new_edge->twin = suffix_edge;
		//再处理其它的邻接Voronoi单元
		FxvSite  *ccw_site = &site_array[0];
		while (edge_ptr->twin->site_ptr != ccw_site) {
			FxvEdge  *other_edge = edge_ptr->twin;
			other_edge->origin = intersect_point;
			FxvSite  *target_site = other_edge->site_ptr;
			int other_idx = target_site - site_array.data();

			const Vec2 normal2 = normalize(now_site.location,target_site->location);
			const Vec2 ortho2(-normal2.y,normal2.x);
			const Vec2 dest2 = intersect_point + ortho2 * length_fx;

			FxvEdge  *prev_edge = other_edge->prev;
			while (prev_edge != nullptr && !segment_segment_intersect_test(intersect_point, dest2, prev_edge->origin, prev_edge->destination, intersect_point2)) {
				prev_edge = prev_edge->prev;
			}
			assert(prev_edge != nullptr);
			//新建一条边,以及相关的twin,联通原来的旧边
			FxvEdge  *other_new_edge = new FxvEdge(intersect_point2,intersect_point,target_site);
			FxvEdge  *new2_edge = new FxvEdge(intersect_point,intersect_point2,&now_site);
			other_new_edge->twin = new2_edge; new2_edge->twin = other_new_edge;

			new_edge->next = new2_edge; new2_edge->prev = new_edge;
			
			other_new_edge->prev = prev_edge; prev_edge->next = other_new_edge;
			other_new_edge->next = other_edge;other_edge->prev = other_new_edge; 
			
			prev_edge->destination = intersect_point2;
			intersect_point = intersect_point2;
			new_edge = new2_edge;
			edge_ptr = prev_edge;
		}
		//最后,针对ccw_site,则需要一次单独的运算
		const Vec2 normal3 = normalize(now_site.location,ccw_site->location);
		const Vec2 ortho3(-normal3.y,normal3.x);
		const Vec2 origin3 = (now_site.location + ccw_site->location) * 0.5f;
		const Vec2 dest3 = origin3 + ortho3 * length_fx;

		FxvEdge  *tail_edge = new FxvEdge(intersect_point,dest3,ccw_site);
		FxvEdge  *ccw_edge = new FxvEdge(dest3,intersect_point,&now_site);
		tail_edge->twin = ccw_edge; ccw_edge->twin = tail_edge;

		FxvEdge  *twin_edge = edge_ptr->twin;//ccw edge
		twin_edge->origin = intersect_point;

		ccw_edge->next = twin_edge; twin_edge->prev = ccw_edge;
		tail_edge->prev = new_edge; new_edge->next = tail_edge;

		now_site.tail_ptr = tail_edge;
		ccw_site->head_ptr = ccw_edge;
	}
}

NS_GT_END