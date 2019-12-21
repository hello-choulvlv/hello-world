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

	++left->ref;
	++right->ref;

	node_size += 2;
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

void DelaunaySearch::visit(std::set<DelaunayNode *> &nodes_array,bool visit_leaf)
{
	//需要一次层序遍历
	std::list<DelaunayNode*> layer_visit_queue;
	DelaunayNode *delaunay_node = &root;
	//std::set<DelaunayNode*>   visit_map;

	layer_visit_queue.push_back(delaunay_node);
	while (layer_visit_queue.size())
	{
		DelaunayNode *node = layer_visit_queue.front();
		layer_visit_queue.pop_front();

		if (delaunay_not_leaf(node))
		{
			if (nodes_array.find(node->lchild) == nodes_array.end())
				layer_visit_queue.push_back(node->lchild), nodes_array.insert(node->lchild);

			if (nodes_array.find(node->mchild) == nodes_array.end())
				layer_visit_queue.push_back(node->mchild), nodes_array.insert(node->mchild);
		}

		if (node->rchild && nodes_array.find(node->rchild) == nodes_array.end())
			layer_visit_queue.push_back(node->rchild), nodes_array.insert(node->rchild);

		//if (!visit_leaf || !node->lchild && !node->mchild && !node->rchild)
			nodes_array.insert(node);
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
	std::set<DelaunayNode *> nodes_array;
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

NS_GT_END