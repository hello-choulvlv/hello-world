/*
  *凸包算法,2d/3d实现
  *2020年2月13日
  */
#ifndef __CONVEX_HULL_H__
#define __CONVEX_HULL_H__
#include "gt_common/geometry_types.h"
#include "math/Vec2.h"
#include "math/Vec3.h"
#include<vector>
#include<list>

NS_GT_BEGIN
/*
  *quick hull 2d 算法需要使用的数据结构
 */
struct QuickHull
{
	int start_l, final_l;
	std::vector<int>      operate_points;
};
/*
  *3d小平面的表示
 */
struct Plane3;
struct ConvexEdge
{
	short v1, v2;

	Plane3   *owner;
	ConvexEdge  *next, *prev, *twin;

	ConvexEdge(short av1, short av2, Plane3 *aowner = nullptr) :v1(av1),v2(av2),owner(aowner), next(nullptr), prev(nullptr), twin(nullptr) {};
};

struct Plane3
{
	short v1,v2,v3, ref;
	std::vector<int> operate_array;

	ConvexEdge *head, *tail;
	Plane3 *next;

	Plane3(short av1, short av2, short av3) :v1(av1),v2(av2),v3(av3),ref(0),head(nullptr),tail(nullptr), next(nullptr){};
	Plane3() :ref(0),head(nullptr),tail(nullptr){};
	~Plane3() {
		if (head)
		{
			tail->next = nullptr;
			while (head)
			{
				ConvexEdge *e = head;
				head = head->next;
				delete e;
			}
			head = nullptr; tail = nullptr;
		}
	};
};
/*
  *内存分配管理器
  *针对平面与边
 */
struct ConvexHullMemmorySlab
{
	Plane3 *_plane_cache;
	ConvexEdge  *_edge_cache;
	char     *_global_memory;
	unsigned short   *_index_array;
	int          _plane_size,_edge_size,_plane_capacity,_edge_capacity;

	ConvexHullMemmorySlab(int global_memory_size,int aplane_capacity = 0x7FFFFFFF, int aedge_capacity = 0x7FFFFFFF) :
		_plane_cache(nullptr),
		_edge_cache(nullptr),
		_global_memory(new char[global_memory_size]),
		_index_array(new unsigned short[global_memory_size]),
		_plane_size(0),
		_edge_size(0),
		_plane_capacity(aplane_capacity),
		_edge_capacity(aedge_capacity){};

	~ConvexHullMemmorySlab() {
		while (_plane_cache)
		{
			Plane3 *plane = _plane_cache;
			_plane_cache = _plane_cache->next;
			delete plane;
		}
		while (_edge_cache)
		{
			ConvexEdge *edge = _edge_cache;
			_edge_cache = _edge_cache->next;
			delete edge;
		}
		delete[] _global_memory;
		delete[] _index_array;
	};

	void release(Plane3 *plane) {
		if (_plane_size < _plane_capacity)
		{
			++_plane_size;
			plane->next = _plane_cache;
			_plane_cache = plane;
		}
		else
			delete plane;
	};
	void release(ConvexEdge *edge)
	{
		if (_edge_size < _edge_capacity)
		{
			++_edge_size;
			edge->next = _edge_cache;
			_edge_cache = edge;
		}
		else
			delete edge;
	}

	Plane3 *apply(short av1, short av2, short av3) {
		Plane3 *plane = nullptr;
		if (_plane_cache)
		{
			plane = _plane_cache;
			_plane_cache = _plane_cache->next;
			--_plane_size;

			plane->v1 = av1; plane->v2 = av2; plane->v3 = av3;
			plane->ref = 0;
			plane->operate_array.clear();
			plane->head = nullptr;
			plane->tail = nullptr;
			plane->next = nullptr;
		}
		else
			plane = new Plane3(av1, av2, av3);
		return plane;
	};
	ConvexEdge *apply(short av1, short av2, Plane3 *aowner = nullptr) {
		ConvexEdge *edge = nullptr;
		if (_edge_cache)
		{
			edge = _edge_cache;
			_edge_cache = _edge_cache->next;
			--_edge_size;

			edge->v1 = av1; edge->v2 = av2; edge->owner = aowner;
			edge->next = edge->prev = edge->twin = nullptr;
		}
		else
			edge = new ConvexEdge(av1, av2, aowner);
		return edge;
	};
};

/*
*求点集的最小凸多边形
*旋转卡壳算法实现
*/
bool polygon_compute_convex_hull(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon_points);
/*
*quick hull算法实现
*2d实现
*/
bool quick_hull_algorithm2d(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon);
/*
*quick hull 算法实现,该算法只实现基本的单纯多胞体,至于出现多点共面的情况,则暂时不考虑
*如果读者需要使用完备的算法实现,则可以自行改动代码实现,原理相同,算法结构也只是稍微复杂一些而已
*最小3d凸平面,注意该算法并没有经过优化,在经过正确性测试后,我们将会对其进行优化.
*输出,每三个顶点构成一个空间平面
*在下以版本中,我们将会对算法进行优化.包括内存分配算法,相关数据查找算法.
*/
bool quick_hull_algorithm3d(const std::vector<cocos2d::Vec3> &points, std::list<Plane3*> &planes);
NS_GT_END
#endif