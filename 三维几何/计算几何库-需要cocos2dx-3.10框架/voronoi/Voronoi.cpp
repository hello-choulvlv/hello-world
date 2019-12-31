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

bool operator==(const DelaunayTriangle &a, const DelaunayTriangle &b)
{
	short u1 = max_f(a.v1,a.v2);
	short u2 = min_f(a.v1,a.v2);
	short u3 = a.v3;
	if (u3 > u1)
	{
		u3 = u2;
		u2 = u1;
		u1 = a.v3;
	}
	else if (u3 > u2)
	{
		u3 = u2;
		u2 = a.v3;
	}

	short v1 = max_f(b.v1, b.v2);
	short v2 = min_f(b.v1, b.v2);
	short v3 = b.v3;
	if (v3 > v1)
	{
		v3 = v2;
		v2 = v1;
		v1 = b.v3;
	}
	else if (v3 > v2)
	{
		v3 = v2;
		v2 = b.v3;
	}
	return u1 == v1 && u2 == v2 && u3 == v3;
}

bool operator==(const DelaunayNode &a, const DelaunayNode &b)
{
	short u1 = max_f(a.v1, a.v2);
	short u2 = min_f(a.v1, a.v2);
	short u3 = a.v3;
	if (u3 > u1)
	{
		u3 = u2;
		u2 = u1;
		u1 = a.v3;
	}
	else if (u3 > u2)
	{
		u3 = u2;
		u2 = a.v3;
	}

	short v1 = max_f(b.v1, b.v2);
	short v2 = min_f(b.v1, b.v2);
	short v3 = b.v3;
	if (v3 > v1)
	{
		v3 = v2;
		v2 = v1;
		v1 = b.v3;
	}
	else if (v3 > v2)
	{
		v3 = v2;
		v2 = b.v3;
	}
	return u1 == v1 && u2 == v2 && u3 == v3;
}

bool operator==(const DelaunayNode &a, const DelaunayTriangle &b)
{
	short u1 = max_f(a.v1, a.v2);
	short u2 = min_f(a.v1, a.v2);
	short u3 = a.v3;
	if (u3 > u1)
	{
		u3 = u2;
		u2 = u1;
		u1 = a.v3;
	}
	else if (u3 > u2)
	{
		u3 = u2;
		u2 = a.v3;
	}

	short v1 = max_f(b.v1, b.v2);
	short v2 = min_f(b.v1, b.v2);
	short v3 = b.v3;
	if (v3 > v1)
	{
		v3 = v2;
		v2 = v1;
		v1 = b.v3;
	}
	else if (v3 > v2)
	{
		v3 = v2;
		v2 = b.v3;
	}
	return u1 == v1 && u2 == v2 && u3 == v3;
}

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

bool triangle_contains_point(const std::vector<cocos2d::Vec2> &disper_points,const DelaunayNode *target_node,const Vec2 &target_point)
{
	const Vec2 &a = disper_points[target_node->v1];
	const Vec2 &b = disper_points[target_node->v2];
	const Vec2 &c = disper_points[target_node->v3];

	float f1 = cross(target_point, a, b);
	float f2 = cross(target_point, b, c);
	float f3 = cross(target_point, c, a);
	//包含关系
	return f1 * f2 >= 0 && f2 * f3 >= 0;
}

DelaunayNode  *DelaunaySearch::insert(int point_index)
{
	DelaunayNode  *target_node = lookup(point_index);
	//分裂成为三个三角形
	DelaunayTriangle  delaunay_triangle = {short(point_index),target_node->v1,target_node->v2};
	target_node->lchild = new DelaunayNode(delaunay_triangle);

	delaunay_triangle.v2 = target_node->v2;
	delaunay_triangle.v3 = target_node->v3;
	target_node->mchild = new DelaunayNode(delaunay_triangle);

	delaunay_triangle.v2 = target_node->v3;
	delaunay_triangle.v3 = target_node->v1;
	target_node->rchild = new DelaunayNode(delaunay_triangle);

	node_size += 3;
	return target_node;
}

void DelaunaySearch::merge(DelaunayNode *target,DelaunayNode *left, DelaunayNode *right)
{
	//assert(!target->lchild && !target->mchild);
	target->lchild = left;
	target->mchild = right;

	//++left->ref;
	//++right->ref;

	node_size += 1;
	//assert(!target->rchild);
}
#define delaunay_not_leaf(node) (node->lchild || node->mchild || node->rchild)
DelaunayNode  *DelaunaySearch::lookup(int point_index)
{
	DelaunayNode  *target_node = &root;
	const Vec2 &target_point = disper_points[point_index];

	while (delaunay_not_leaf(target_node))
	{
		if (triangle_contains_point(disper_points, target_node->lchild, target_point))
			target_node = target_node->lchild;
		else if (triangle_contains_point(disper_points, target_node->mchild, target_point))
			target_node = target_node->mchild;
		else //if (target_node->rchild && triangle_contains_point(disper_points, target_node->rchild, target_point))
			target_node = target_node->rchild;
	}
	//assert(triangle_contains_point(disper_points, target_node, target_point));
	return target_node;
}

void DelaunaySearch::destroy(std::set<DelaunayNode *> &nodes_array)
{

}

void DelaunaySearch::visit(std::vector<DelaunayNode *> &nodes_array,bool visit_leaf)
{
	//需要一次层序遍历
	std::list<DelaunayNode*> layer_visit_queue;
	DelaunayNode *delaunay_node = &root;
	nodes_array.resize(node_size);
	int base_j = 0;

	layer_visit_queue.push_back(delaunay_node);
	while (layer_visit_queue.size())
	{
		DelaunayNode *node = layer_visit_queue.front();
		layer_visit_queue.pop_front();

		if (delaunay_not_leaf(node))
		{
			if (!node->lchild->ref)
				layer_visit_queue.push_back(node->lchild),node->lchild->ref = 1;

			if (!node->mchild->ref)
				layer_visit_queue.push_back(node->mchild),node->mchild->ref=1;
		}

		if (node->rchild && !node->rchild->ref)
			layer_visit_queue.push_back(node->rchild),node->rchild->ref=1;

		//if (!visit_leaf || !node->lchild && !node->mchild && !node->rchild)
			//nodes_array.insert(node);
		nodes_array.at(base_j++) = node;
	}
}

DelaunaySearch::~DelaunaySearch()
{
	//std::set<DelaunayNode *>  visit_nodes_array;
	//visit(visit_nodes_array,nullptr);
	//for (auto it = visit_nodes_array.begin(); it != visit_nodes_array.end(); ++it)
	//{
	//	DelaunayNode *node = *it;
	//	if(node != &root)
	//		delete node;
	//}
}
#undef delaunay_not_leaf

VoronoiEdge *VoronoiMemorySlab::apply(VoronoiSite *target_site, const cocos2d::Vec2 &start_point, const cocos2d::Vec2 &final_point)
{
	VoronoiEdge  *edge = nullptr;
	if (slab_head)
	{
		edge = slab_head;
		slab_head = slab_head->next;
		--size;
		edge->twin = edge->prev = edge->next = nullptr;
		edge->origin = start_point;
		edge->bottom = final_point;
		edge->owner_site = target_site;
	}
	else
		edge = new VoronoiEdge(target_site,start_point,final_point);
	return edge;
}

void VoronoiMemorySlab::release(VoronoiEdge *edge)
{
	if (size + 1 < capacity)
	{
		edge->next = slab_head;
		slab_head = edge;
		++size;
	}
	else
		delete edge;
}

VoronoiMemorySlab::~VoronoiMemorySlab()
{
	VoronoiEdge *edge = slab_head;
	while (edge != nullptr)
	{
		VoronoiEdge *other = edge;
		edge = edge->next;
		delete other;
	}
	slab_head = nullptr;
	size = 0;
	edge = nullptr;
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

void static_create_cycle(Cycle &cycle,const Vec2 &a,const Vec2 &b,const Vec2 &c)
{
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

void static_create_cycle_by_triangle(Cycle &cycle,const std::vector<Vec2> &disper_points,const DelaunayTriangle &delaunay_triangle)
{
	const Vec2 &a = disper_points[delaunay_triangle.v1];
	const Vec2 &b = disper_points[delaunay_triangle.v2];
	const Vec2 &c = disper_points[delaunay_triangle.v3];

	static_create_cycle(cycle, a, b, c);
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
/*
  *判断某一个点落在了那个三角形之中
 */
const DelaunayTriangle *check_triangle_contains_point(const std::vector<Vec2> &disper_points,const std::list<DelaunayTriangle> &triangle_queue,const Vec2 &target_point)
{
	for (auto it = triangle_queue.begin(); it != triangle_queue.end(); ++it)
	{
		auto &triangle = *it;
		const Vec2 &a = disper_points[triangle.v1];
		const Vec2 &b = disper_points[triangle.v2];
		const Vec2 &c = disper_points[triangle.v3];

		float f1 = cross(target_point,a,b);
		float f2 = cross(target_point,b,c);
		float f3 = cross(target_point,c,a);

		if (f1 * f2 > 0 && f2 * f3 > 0)
			return &triangle;
	}
	return nullptr;
}

void delaunay_triangulate_random(const std::vector<cocos2d::Vec2> &disper_points, std::vector<DelaunayTriangle> &triangle_sequence, int &real_size)
{
	const int array_size = disper_points.size();
	DelaunayTriangle  delaunay_triangle = {short(array_size -3),short(array_size-2),short(array_size-1)};
	//记录所有的三角形行列表
	DelaunaySearch   delaunay_search(disper_points,delaunay_triangle);

	DelaunayEdge  delaunay_edge = {short(array_size-3),short(array_size-2)};
	//记录边与三角形之间的关系,可以用该数据结构快速定位相关的三角形
	std::map<DelaunayEdge, TwinNode>  double_connect_map;
	TwinNode  twin_node = { &delaunay_search.root,nullptr};
	double_connect_map[delaunay_edge] = twin_node;

	delaunay_edge.v1 = array_size - 2;
	delaunay_edge.v2 = array_size - 1;
	double_connect_map[delaunay_edge] = twin_node;

	delaunay_edge.v1 = array_size - 1;
	delaunay_edge.v2 = array_size - 3;
	double_connect_map[delaunay_edge] = twin_node;

	for (int index_l = 0; index_l < array_size - 3; ++index_l)
	{
		//查找相关的三角形
		DelaunayNode *target_node = delaunay_search.insert(index_l);// check_triangle_contains_point(disper_points, undetermine_triangles, disper_points[index_l]);
		//分别连接当前点与三角形的三个点形成三个新的三角形,删除原来的,并重置新三角形与原来的三角形邻接三角形之间的关系
		delaunay_triangle.v1 = index_l;

		DelaunayNode *t1 = target_node->lchild;//index_l,v1,v2
		DelaunayNode *t2 = target_node->mchild;//index_l,v2,v3
		DelaunayNode *t3 = target_node->rchild;//index_l,v3,v1

		std::list<DelaunayEdge>	adjust_edge_queue;
		//删除掉原来的三角形,并调整当前的三个三角形与原邻接三角形之间的关系
#define insert_delaunay_edge_map(av1,av2,tx) {\
		delaunay_edge.v1 = av1;delaunay_edge.v2 = av2;\
		auto it = double_connect_map.find(delaunay_edge);\
		if(it->second.left_node == target_node)it->second.left_node = tx;else it->second.right_node = tx;\
		}
		insert_delaunay_edge_map(target_node->v1, target_node->v2,t1);
		adjust_edge_queue.push_front(delaunay_edge);

		//BC
		insert_delaunay_edge_map(target_node->v2, target_node->v3, t2);
		adjust_edge_queue.push_front(delaunay_edge);

		//CA
		insert_delaunay_edge_map(target_node->v3, target_node->v1, t3);
		adjust_edge_queue.push_front(delaunay_edge);

#define insert_twin_edge_map(av1,av2,twin1,twin2){\
			delaunay_edge.v1 = av1,delaunay_edge.v2 = av2;\
			twin_node.left_node = twin1;twin_node.right_node = twin2;\
			double_connect_map[delaunay_edge] = twin_node;\
		}
		//RA
		insert_twin_edge_map(index_l, target_node->v1,t3,t1);
		insert_twin_edge_map(index_l, target_node->v2,t1,t2);
		insert_twin_edge_map(index_l, target_node->v3,t2,t3);

#undef insert_twin_edge_map
#undef insert_delaunay_edge_map
		//原来的三角形不会被干掉,针对以上新的三角形,逐个的调整他们之间的关系
		while (adjust_edge_queue.size())
		{
			DelaunayEdge  adjust_edge = adjust_edge_queue.front();
			adjust_edge_queue.pop_front();

			delaunay_triangle.v2 = adjust_edge.v1;
			delaunay_triangle.v3 = adjust_edge.v2;
			//判断该边的邻接三角形是否的对角顶点是否在在三点的外接圆之内
			auto it = double_connect_map.find(adjust_edge);
			//有可能已经达到了边界,因此需要作出判断
			if (it->second.left_node && it->second.right_node)
			{
				DelaunayNode *left_triangle = *it->second.left_node == delaunay_triangle ?it->second.left_node :it->second.right_node;
				DelaunayNode *right_triangle = it->second.left_node != left_triangle ? it->second.left_node :it->second.right_node;
				short v1 = adjust_edge.v1, v2 = adjust_edge.v2;
				//找到第三个点
				short v3 = right_triangle->v1 != v1 && right_triangle->v1 != v2? right_triangle->v1:(right_triangle->v2 != v1 && right_triangle->v2 != v2 ? right_triangle->v2: right_triangle->v3);
				Cycle cycle;
				static_create_cycle_by_triangle(cycle, disper_points, delaunay_triangle);
				//此时新插入的点非法,需要调整公共边
				if (check_point_insideof_cycle(cycle, disper_points[v3]))
				{
					delaunay_triangle.v2 = v3;
					delaunay_triangle.v3 = v1;
					DelaunayNode  *tf1 = new DelaunayNode(delaunay_triangle);

					delaunay_triangle.v2 = v3;
					delaunay_triangle.v3 = v2;
					DelaunayNode *tf2 = new DelaunayNode(delaunay_triangle);

					delaunay_search.merge(left_triangle, tf1, tf2);
					delaunay_search.merge(right_triangle,tf1,tf2);

					//删除原来的边与三角形之间的对应关系
					double_connect_map.erase(adjust_edge);
					adjust_edge.v1 = index_l;
					adjust_edge.v2 = v3;

					twin_node.left_node = tf1;
					twin_node.right_node = tf2;
					double_connect_map[adjust_edge] = twin_node;
					//修正正新边与三角形之间的对应关系,一共有额外的四条边
					delaunay_edge.v1 = v2;
					delaunay_edge.v2 = index_l;
					auto it1 = double_connect_map.find(delaunay_edge);
					if (it1->second.left_node == left_triangle)
						it1->second.left_node = tf2;
					else
						it1->second.right_node = tf2;

					delaunay_edge.v1 = index_l;
					delaunay_edge.v2 = v1;
					auto it2 = double_connect_map.find(delaunay_edge);
					if (it2->second.left_node == left_triangle)
						it2->second.left_node = tf1;
					else
						it2->second.right_node = tf1;
					//3
					delaunay_edge.v1 = v1;
					delaunay_edge.v2 = v3;
					auto it3 = double_connect_map.find(delaunay_edge);
					if (it3->second.left_node == right_triangle)
						it3->second.left_node = tf1;
					else
						it3->second.right_node = tf1;
					adjust_edge_queue.push_front(delaunay_edge);
					//4
					delaunay_edge.v1 = v3;
					delaunay_edge.v2 = v2;
					auto it4 = double_connect_map.find(delaunay_edge);
					if (it4->second.left_node == right_triangle)
						it4->second.left_node = tf2;
					else
						it4->second.right_node = tf2;
					adjust_edge_queue.push_front(delaunay_edge);
				}
			}
		}
	}
	//将所有获得的三角形序列写入到数组里面,注意，下面的代码并没有经过优化
	int boundary_l = array_size - 3;
	std::vector<DelaunayNode *> nodes_array;
	delaunay_search.visit(nodes_array,true);
	for (auto it = nodes_array.begin(); it != nodes_array.end(); ++it)
	{
		auto delaunay_node = *it;
		int f = long(delaunay_node->lchild) + long(delaunay_node->mchild) + long(delaunay_node->rchild);
		if (!f && delaunay_node->v1 < boundary_l && delaunay_node->v2 < boundary_l && delaunay_node->v3 < boundary_l)
		{
			delaunay_triangle.v1 = delaunay_node->v1, delaunay_triangle.v2 = delaunay_node->v2, delaunay_triangle.v3 = delaunay_node->v3;
			vector_fast_push_back(triangle_sequence, delaunay_triangle);
		}
		if (delaunay_node != &delaunay_search.root)
			delete delaunay_node;
	}
	real_size = triangle_sequence.size();
}

void voronoi_delaunay_triangle(const std::vector<cocos2d::Vec2> &disper_points, std::vector<cocos2d::Vec2> &edge_points, std::vector<int> &edge_index_array, std::vector<int> &other_ray_array)
{
	std::vector<DelaunayTriangle>  triangle_sequence;
	int real_size = 0;
	delaunay_triangulate_bowyer_washton(disper_points, triangle_sequence, real_size);
	//针对新得出的没一个三角形,逐个的计算其外接圆的圆心,并奖相关的离散点存入到数组中
	std::map<DelaunayEdge, TwinTriangle>  edge_triangle_map;
	DelaunayEdge  delaunay_edge;
	TwinTriangle twin_triangle;
#define insert_triangle_map(t_v1,t_v2)\
{\
	delaunay_edge.v1 = t_v1, delaunay_edge.v2 = t_v2;\
\
	auto it = edge_triangle_map.find(delaunay_edge);\
	if (it != edge_triangle_map.end())\
	{\
		if (it->second.left_triangle != triangle)\
			it->second.right_triangle = triangle;\
	}\
	else\
	{\
		twin_triangle.left_triangle = triangle;\
		twin_triangle.right_triangle = nullptr;\
		edge_triangle_map[delaunay_edge] = twin_triangle;\
	}}

	for (int index_l = 0; index_l < real_size; ++index_l)
	{
		DelaunayTriangle *triangle = &triangle_sequence[index_l];
		insert_triangle_map(triangle->v1, triangle->v2);
		insert_triangle_map(triangle->v2,triangle->v3);
		insert_triangle_map(triangle->v3,triangle->v1);

		Cycle cycle;
		static_create_cycle_by_triangle(cycle, disper_points, *triangle);
		vector_fast_push_back(edge_points,cycle.center);
	}
#undef insert_triangle_map
	//遍历每一个三角形形的每一条边
	DelaunayTriangle *base_triangle = triangle_sequence.data();
	const Vec2 *base_point = disper_points.data();
	for (auto it = edge_triangle_map.begin(); it != edge_triangle_map.end();++it)
	{
		auto &triangle_edge = it->first;
		auto &twin_triangle = it->second;
		//检查是否该三角形的某一条边只有一个邻接三角形
		DelaunayTriangle *left_triangle = twin_triangle.left_triangle;
		DelaunayTriangle *right_triangle = twin_triangle.right_triangle;
		if (left_triangle && right_triangle)
		{
			vector_fast_push_back(edge_index_array,left_triangle - base_triangle);
			vector_fast_push_back(edge_index_array,right_triangle - base_triangle);
		}
		else
		{
			DelaunayTriangle *t = left_triangle ? left_triangle : right_triangle;
			short v3 = t->v1 != triangle_edge.v1 && t->v1 != triangle_edge.v2?t->v1:(t->v2 !=triangle_edge.v1 && t->v2!= triangle_edge.v2?t->v2:t->v3);
			Vec2 normal = normalize(base_point[triangle_edge.v1] - base_point[triangle_edge.v2]);
			float sign_f = cross(base_point[triangle_edge.v1],base_point[triangle_edge.v2],base_point[v3]) > 0?-1:1;
			vector_fast_push_back(edge_points,Vec2(normal.y * sign_f,-normal.x *sign_f));

			vector_fast_push_back(other_ray_array,t - base_triangle);
			vector_fast_push_back(other_ray_array,edge_points.size() - 1);
		}
	}
}
/*
*射线与线段求教,如果没有相交,则返回false,否则返回true,与相交点
*/
bool static_ray_segment_intersect(const Vec2 &origin, const Vec2 &normal, const Vec2 &start_point, const Vec2 &final_point, Vec2 &intersect_point)
{
	float f1 = cross(start_point - origin, normal);
	float f2 = cross(normal, final_point - origin);
	if (f1 * f2 < 0) return false;
	float f = f1 / (f1 + f2);
	intersect_point = start_point + (final_point - start_point) * f;
	return dot(intersect_point - origin, normal) >= 0.0f;
}
/*
  *目标射线与边界的四条线段相交测试,返回第一条
 */
int static_ray_segments_intersect(const Vec2 &origin, const Vec2 &normal, const Vec2 *points_stride[5], Vec2 &intersect_point)
{
	for (int index_l = 0; index_l < 4; ++index_l)
	{
		if (static_ray_segment_intersect(origin, normal, *points_stride[index_l], *points_stride[index_l + 1], intersect_point))
			return index_l;
	}
	assert(false);
	return -1;
}
/*
  *首先求出三个离散点集的Voronoi图
 */
void create_voronoi3(const std::vector<cocos2d::Vec2> &disper_points,const Vec2 *points_stride[5],std::vector<VoronoiSite> &voronoi_sites)
{
	Cycle cycle;
	//求三个点的外接圆的圆心
	const Vec2 &a = disper_points[0];
	const Vec2 &b = disper_points[1];
	const Vec2 &c = disper_points[2];
	static_create_cycle(cycle,a,b,c);

	float f = cross(a,b,c);
	const Vec2 &bf = f > 0.0f ? b : c;
	const Vec2 &cf = f > 0.0f ? c : b;
	VoronoiSite *site_array = voronoi_sites.data();
	//假设外界有一个足够大的包围框
	const int array_size = disper_points.size();
	//边的生成规则太特么的复杂了,怀疑人生
	//A
	VoronoiSite *site_a = site_array;
	VoronoiSite *site_b = site_array +1;
	VoronoiSite *site_c = site_array + 2;

	site_a->location = &a;
	site_b->location = &bf;
	site_c->location = &cf;
	//每一个基点都拥有两条射线,a==>AB,向量(x,y)的法向量为(-y,x)
	Vec2 normal_ba = normalize(bf,a),intersect_point_ab,intersect_point_bc,intersect_point_ca;
	int l1 = static_ray_segments_intersect(cycle.center,Vec2(-normal_ba.y,normal_ba.x), points_stride,intersect_point_ab);

	Vec2 normal_ac = normalize(a,cf);
	int l2 = static_ray_segments_intersect(cycle.center, Vec2(-normal_ac.y,normal_ac.x), points_stride,intersect_point_ca);

	Vec2 normal_cb = normalize(cf, bf);
	int l3 = static_ray_segments_intersect(cycle.center, Vec2(-normal_cb.y, normal_cb.x), points_stride, intersect_point_bc);

	VoronoiEdge *a1 = new VoronoiEdge(site_a, intersect_point_ab, cycle.center);
	VoronoiEdge *a2 = new VoronoiEdge(site_a,cycle.center,intersect_point_ca);
	VoronoiEdge  *last_edge = a2;
	const Vec2 *last_point = &intersect_point_ca;
	a1->next = a2; a2->prev = a1;
	//需要检查是否需要生成中间的边,如果有的话,不会超过3个
	if (l2 != l1)
	{
		int l0 = l2;
		do 
		{
			l0 = (l0 +1)%4;
			VoronoiEdge *e = new VoronoiEdge(site_a,*last_point,*points_stride[l0]);
			last_point = points_stride[l0];
			last_edge->next = e;
			e->prev = last_edge;
			last_edge = e;
		} while (l0 != l1);
	}
	//此时中间只有一条边,可以直接连接,稍后我们会将公共代码抽取出来.
	VoronoiEdge *a3 = new VoronoiEdge(site_a, *last_point, intersect_point_ab);
	a3->prev = last_edge; a3->next = a1;
	a1->prev = a3; last_edge->next = a3;
	site_a->head = a1; site_a->tail = a3;

	//B
	VoronoiEdge *b1 = new VoronoiEdge(site_b, intersect_point_bc,cycle.center);
	VoronoiEdge *b2 = new VoronoiEdge(site_b,cycle.center, intersect_point_ab);
	last_edge = b2;
	last_point = &intersect_point_ab;
	b1->next = b2; b2->prev = b1; 
	//检查中间的生成边,有可能会多出三条边
	if (l1 != l3)
	{
		int l0 = l1;
		do
		{
			l0 = (l0 + 1) % 4;
			VoronoiEdge *e = new VoronoiEdge(site_b, *last_point, *points_stride[l0]);
			last_point = points_stride[l0];
			last_edge->next = e;
			e->prev = last_edge;
			last_edge = e;
		} while (l0 != l3);
	}
	//最后一条必然会出现的边
	VoronoiEdge *b3 = new VoronoiEdge(site_b, *last_point, intersect_point_bc);
	b3->prev = last_edge; b3->next = b1;
	b1->prev = b3; last_edge->next = b3;
	site_b->head = b1; site_b->tail = b3;

	//C
	VoronoiEdge *c1 = new VoronoiEdge(site_c, intersect_point_ca, cycle.center);
	VoronoiEdge *c2 = new VoronoiEdge(site_c, cycle.center,intersect_point_bc);
	last_edge = c2;
	last_point = &intersect_point_bc;
	c1->next = c2; c2->prev = c1; 
	//检查中间的生成边,有可能会多出三条边
	if (l3 != l2)
	{
		int l0 = l3;
		do
		{
			l0 = (l0 + 1) % 4;
			VoronoiEdge *e = new VoronoiEdge(site_c, *last_point, *points_stride[l0]);
			last_point = points_stride[l0];
			last_edge->next = e;
			e->prev = last_edge;
			last_edge = e;
		} while (l0 != l2);
	}
	//最后一条必然会出现的边
	VoronoiEdge *c3 = new VoronoiEdge(site_c, *last_point, intersect_point_ca);
	c3->prev = last_edge; c3->next = c1;
	c1->prev = c3; last_edge->next = c3;
	site_c->head = c1; site_c->tail = c3;
	//记录某些边之间的关系
#define twin_couple(t1,t2) t1->twin = t2;t2->twin=t1;
	twin_couple(a1, b2);
	twin_couple(b1,c2);
	twin_couple(a2,c1);
#undef twin_couple
}
/*
  *沿着原来的Voronoi单元,向周围的单元沿逆时针旋转构造新的Voronoi单元
  *注意,遍历周围单元内部的时候是沿着顺时针的,
  *上面所说的顺时针是指就整体遍历周围的单元的相对方向
 */
VoronoiEdge *static_voronoi_rotate_left(VoronoiMemorySlab &mem_slab,VoronoiEdge *secondary_edge,VoronoiEdge *edge2,VoronoiSite &site_x,const Vec2 &target_location,Vec2 &last_origin,/*std::vector<Vec2> &interval_points,*/VoronoiEdge **final_edge)
{
	VoronoiEdge *last_new_edge = *final_edge;
	while (secondary_edge && secondary_edge != edge2)
	{
		Vec2 u = *secondary_edge->owner_site->location - target_location;
		Vec2 ray = normalize(-u.y, u.x);
		VoronoiEdge  *tripple_edge = secondary_edge->prev;//此时为顺时针遍历
		Vec2 intersect_point3;
		while (!static_ray_segment_intersect(last_origin, ray, tripple_edge->origin, tripple_edge->bottom, intersect_point3))
		{
			VoronoiEdge *internal_edge = tripple_edge;
			tripple_edge = tripple_edge->prev;
			mem_slab.release(internal_edge);
		}
		//将以前的局部线段截断
		secondary_edge->origin = last_origin;
		tripple_edge->bottom = intersect_point3;
		VoronoiEdge *new_edge0 = mem_slab.apply(&site_x,last_origin,intersect_point3);// new VoronoiEdge(&site_x, last_origin, intersect_point3);
		last_new_edge->next = new_edge0;
		new_edge0->prev = last_new_edge;
		//Twin
		VoronoiEdge *twin_edge = mem_slab.apply(secondary_edge->owner_site,intersect_point3,last_origin);// new VoronoiEdge(secondary_edge->owner_site, intersect_point3, last_origin);
		twin_edge->prev = tripple_edge;
		twin_edge->next = secondary_edge;

		secondary_edge->prev = twin_edge;
		tripple_edge->next = twin_edge;

		new_edge0->twin = twin_edge;
		twin_edge->twin = new_edge0;

		tripple_edge->owner_site->head = secondary_edge;
		tripple_edge->owner_site->tail = twin_edge;

		last_origin = intersect_point3;
		secondary_edge = tripple_edge->twin;
		last_new_edge = new_edge0;
	}
	*final_edge = last_new_edge;
	return secondary_edge;
}
/*
  *沿着原来的Voronoi单元向周围的邻接单元顺时针旋转
  *单元内部是逆时针旋转
 */
VoronoiEdge *static_voronoi_rotate_right(VoronoiMemorySlab &mem_slab,VoronoiEdge *secondary_edge, VoronoiEdge *edge, VoronoiSite &site_x, const Vec2 &target_location, Vec2 &last_origin, VoronoiEdge **final_edge)
{
	VoronoiEdge *last_new_edge = *final_edge;
	while (secondary_edge && secondary_edge != edge)
	{
		Vec2 u = *secondary_edge->owner_site->location - target_location;
		Vec2 ray = normalize(u.y, -u.x);//此时射线的方向应该朝向右侧
		VoronoiEdge  *tripple_edge = secondary_edge->next;//此时为逆时针遍历
		Vec2 intersect_point3;
		//在循环之前,可能得需要把公共边干掉
		while (!static_ray_segment_intersect(last_origin, ray, tripple_edge->origin, tripple_edge->bottom, intersect_point3))
		{
			VoronoiEdge *internal_edge = tripple_edge;
			tripple_edge = tripple_edge->next;
			mem_slab.release(internal_edge);
		}
		//将以前的局部线段截断
		secondary_edge->bottom = last_origin;
		tripple_edge->origin = intersect_point3;
		VoronoiEdge *new_edge0 = mem_slab.apply(&site_x,intersect_point3,last_origin);// new VoronoiEdge(&site_x, intersect_point3, last_origin);
		last_new_edge->prev = new_edge0;
		new_edge0->next = last_new_edge;
		//Twin
		VoronoiEdge *twin_edge = mem_slab.apply(secondary_edge->owner_site,last_origin,intersect_point3);// new VoronoiEdge(secondary_edge->owner_site, last_origin, intersect_point3);
		twin_edge->next = tripple_edge;
		twin_edge->prev = secondary_edge;

		secondary_edge->next = twin_edge;
		tripple_edge->prev = twin_edge;

		new_edge0->twin = twin_edge;
		twin_edge->twin = new_edge0;

		tripple_edge->owner_site->head = twin_edge;
		tripple_edge->owner_site->tail = secondary_edge;

		last_origin = intersect_point3;
		secondary_edge = tripple_edge->twin;
		last_new_edge = new_edge0;
	}
	*final_edge = last_new_edge;
	return secondary_edge;
}
/*
  *向Voronoi图中插入一个新的顶点
  *顶点的索引为target_index
  *Voronoi图的单元为:index_v
 */
void static_voronoi_insert(VoronoiMemorySlab &mem_slab,const std::vector<cocos2d::Vec2> &disper_points, const Vec2 *points_stride[5], std::vector<VoronoiSite> &voronoi_sites,int index_v,int target_index)
{
	const int array_size = disper_points.size();
	const Vec2 *points_array = disper_points.data();
	VoronoiSite *sites_array = voronoi_sites.data();
	//先处理当前单元index_v中的边分割,求target_index-->disper_points[index_v]的垂直平分线
	VoronoiSite &site_now = sites_array[index_v];
	VoronoiSite &site_x = sites_array[target_index];
	const Vec2 &target_location = points_array[target_index];
	site_x.location = &target_location;

	Vec2 v = *site_now.location - target_location;
	Vec2 normal = normalize(-v.y,v.x);
	Vec2 normal2(-normal.x,-normal.y);//相反方向的射线
	Vec2 origin = (*site_now.location + target_location) * 0.5f;
	//逆时针扫过当前的Voronoi单元,扫过的区域就是被分割的区域
	VoronoiEdge  *edge = site_now.head,*internal_edge = nullptr;
	Vec2 intersect_point,intersect_point2,intersect_point3, intersect_point4;
	//查找第一个相交的边
	while (!static_ray_segment_intersect(origin, normal, edge->origin, edge->bottom, intersect_point))
		edge = edge->next;
	//查找第二条相交的边
	VoronoiEdge *edge2 = edge->next;
	while (!static_ray_segment_intersect(origin, normal2, edge2->origin, edge2->bottom, intersect_point2))
	{
		internal_edge = edge2;
		edge2 = edge2->next;
		mem_slab.release(internal_edge);
	}
	//针对新建立的Voronoi单元,创建第一条边
	VoronoiEdge *new_edge = mem_slab.apply(&site_x,intersect_point2,intersect_point);// new VoronoiEdge(&site_x, intersect_point2, intersect_point);
	site_x.head = new_edge;

	VoronoiEdge *twin_old = mem_slab.apply(&site_now,intersect_point,intersect_point2);// new VoronoiEdge(&site_now, intersect_point, intersect_point2);
	new_edge->twin = twin_old; twin_old->twin = new_edge;

	twin_old->prev = edge;
	twin_old->next = edge2;

	edge->next = twin_old;
	edge2->prev = twin_old;

	site_now.head = twin_old;
	site_now.tail = edge;
	//需要更新原来的数据结构VoronoiSite,因为可能在中间遍历的过程中,原来的某些数据已经被清理掉了
	//从交点intersect_point开始逆时针遍历,直到再次遇到边edge2,注意后面我们将会拆分周围的邻接单元,此过程比较复杂
	VoronoiEdge *secondary_edge = edge->twin;
	VoronoiEdge  *last_new_edge = new_edge;
	//注意,secondary可能为空,这发生在边界单元的过程中
	if (secondary_edge != nullptr)
	{
		Vec2 last_origin = intersect_point;//记录射线上次的起点
		secondary_edge = static_voronoi_rotate_left(mem_slab,secondary_edge,edge2, site_x, target_location, last_origin,&last_new_edge);
		if (!secondary_edge)
		{
			//需要再次访问左侧
			VoronoiEdge *tripple_edge = new_edge;
			secondary_edge = edge2->twin;
			last_origin = intersect_point2;
			//此时的函数调用返回值必然为空,因此可以不用理会
			static_voronoi_rotate_right(mem_slab,secondary_edge, edge, site_x, target_location, last_origin, &tripple_edge);
			//求出当前last_new_edge与四个周边相交的索引
			//左侧
			int l0 = static_ray_segments_intersect(last_new_edge->origin, normalize(last_new_edge->origin,last_new_edge->bottom),points_stride,intersect_point3);
			//右侧
			int l1 = static_ray_segments_intersect(tripple_edge->bottom,normalize(tripple_edge->bottom,tripple_edge->origin),points_stride,intersect_point4);
			const Vec2 *lst_pos = &intersect_point3;
			while (l0 != l1)
			{
				int l2 = (l0 + 1)%4;
				VoronoiEdge *n0 = mem_slab.apply(&site_x,*lst_pos,*points_stride[l2]);// new VoronoiEdge(&site_x, *lst_pos, *points_stride[l2]);
				last_new_edge->next = n0; n0->prev = last_new_edge;
				last_new_edge = n0;

				lst_pos = points_stride[l2];
				l0 = l2;
			}

			VoronoiEdge *nl = mem_slab.apply(&site_x, *lst_pos, intersect_point4);// new VoronoiEdge(&site_x, *lst_pos, intersect_point4);
			last_new_edge->next = nl; nl->prev = last_new_edge;
			last_new_edge = nl;
			
			last_new_edge->next = tripple_edge; tripple_edge->prev = last_new_edge;

			site_x.head = tripple_edge;
		}
	}
	else if(edge2->twin != nullptr)//该处理过程相对于上面的选择分支要稍微简单一些
	{
		secondary_edge = edge2->twin;
		Vec2 last_origin = intersect_point2;//记录射线上次的起点
		secondary_edge = static_voronoi_rotate_right(mem_slab,secondary_edge, edge, site_x, target_location, last_origin, &last_new_edge);
		//if (!secondary_edge)//此时必然为空值
		assert(!secondary_edge);
		{
			int l0 = static_ray_segments_intersect(new_edge->origin,normalize(new_edge->origin, new_edge->bottom),points_stride,intersect_point3);
			int l1 = static_ray_segments_intersect(last_new_edge->bottom,normalize(last_new_edge->bottom, last_new_edge->origin),points_stride,intersect_point4);
			//生成边界线
			const Vec2 *start_point = &intersect_point4;
			//注意,扫过的顺序是顺时针,注意边界点集的选取与上面的算法有所不同
			while(l1 != l0)
			{
				int l2 = (l1 - 1 + 4)%4;
				VoronoiEdge *n0 = mem_slab.apply(&site_x, *points_stride[l1], *start_point);// new VoronoiEdge(&site_x, *points_stride[l1], *start_point);
				n0->next = last_new_edge; last_new_edge->prev = n0;
				last_new_edge = n0;

				start_point = points_stride[l1];
				l1 = l2;
			}
			VoronoiEdge *l3 = mem_slab.apply(&site_x, intersect_point3, *start_point);// new VoronoiEdge(&site_x, intersect_point3, *start_point);
			l3->next = last_new_edge; last_new_edge->prev = l3;
			last_new_edge = l3;
		}
		//注意因为计算VD的过程是顺时针的,跟算法整体的假设刚好相反,因此这里需要逆转过来
		site_x.head = last_new_edge;
		last_new_edge = new_edge;
	}
	else//此时只可能有两条额外的边
	{
		VoronoiEdge  *a2 = mem_slab.apply(&site_x, new_edge->bottom, edge->bottom);// new VoronoiEdge(&site_x, new_edge->bottom, edge->bottom);
		new_edge->next = a2; a2->prev = new_edge;

		VoronoiEdge *a3 = mem_slab.apply(&site_x, edge->bottom, new_edge->origin);// new VoronoiEdge(&site_x, edge->bottom, new_edge->origin);
		a3->prev = a2;a2->next = a3;

		last_new_edge = a3;
		site_x.head = new_edge;
	}
	//注意,也有可能以上两个选择语句中的有效代码根本没有经过任何的执行
	site_x.tail = last_new_edge;
	last_new_edge->next = site_x.head;
	site_x.head->prev = last_new_edge;

	edge->bottom = intersect_point;
	edge2->origin = intersect_point2;
}

void voronoi_increament_policy(const std::vector<cocos2d::Vec2> &disper_points, std::vector<VoronoiSite> &voronoi_sites)
{
	voronoi_sites.resize(disper_points.size() - 2);
	//对于后面的点,逐个的遍历
	const int array_size = disper_points.size();
	const Vec2 *points_array = disper_points.data();
	const Vec2 &left_bottom = disper_points[array_size - 2];
	const Vec2 &right_top = disper_points[array_size - 1];
	const Vec2 left_top = Vec2(left_bottom.x, right_top.y);
	const Vec2 right_bottom(right_top.x, left_bottom.y);
	const Vec2 *points_stride[5] = { &left_bottom,&right_bottom,&right_top,&left_top,&left_bottom };
	const VoronoiSite *sites_array = voronoi_sites.data();

	VoronoiMemorySlab  mem_slab;
	//首先生成前三个Voronoi图
	create_voronoi3(disper_points, points_stride, voronoi_sites);
	for (int index_l = 3; index_l < array_size - 2; ++index_l)
	{
		//计算最近点
		float min_distance = length(points_array[index_l], *sites_array[0].location);
		int base_j = 0;
		for (int compare_index = 1; compare_index < index_l; ++compare_index)
		{
			float f = length(points_array[index_l], *sites_array[compare_index].location);
			if (f < min_distance)
			{
				base_j = compare_index;
				min_distance = f;
			}
		}
		//将顶点disper_points[index_l]插入到base_j的Voronoi图中
		static_voronoi_insert(mem_slab,disper_points, points_stride,voronoi_sites, base_j, index_l);
	}
}
NS_GT_END