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
	short v1,v2,v3, ref,high_v,index_queue;
	std::vector<int> operate_array;

	ConvexEdge *head, *tail;
	Plane3 *next;
	cocos2d::Vec3    normal;

	Plane3(short av1, short av2, short av3) :v1(av1),v2(av2),v3(av3),ref(0), high_v(-1), index_queue(-1),head(nullptr),tail(nullptr), next(nullptr){};
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

			plane->v1 = av1; plane->v2 = av2; plane->v3 = av3,plane->high_v = -1,plane->index_queue = -1;
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
*以前的测试的数据量都是128
*@version:1.0实现基本算法
*@version:1.0->algorithm cost time:11.70
*@version:2.0修复算法中有时会遗漏某些点的处理错误,并优化内存分配,减少数据复制,以及点处理的运算量
*@version2.0->algorithm cost time:10.30
*@version:3.0消除求极点的运算过程,将该过程与求冲突点过程合并,以及将平面的法线直接存储,不再每次遍历时都重新计算.
*@version:3.0经过反复的实验,算法的运行时间明显的缩短了
*@version3.0->algorithm cost time:7.01
*******************************************************************
*@version:4.0在第四版中,我们将会使用一些高级的数据结构来进一步的提升算法的运行时性能.
*@version:4.1->seed:1581910632:优化前:7.95,优化后:6.49,
*@version4.2:在使用了优先级队列之后,运行时间为algorithm cost time:4.06
*@note:以上的优化过程都是为了下一个更复杂也更为高效的算法的实现过程作准备.
*/
bool quick_hull_algorithm3d(const std::vector<cocos2d::Vec3> &points, std::vector<Plane3*> &planes);
/*
  *三维凸壳算法之优化实现
  *该算法源自于<计算几何>的第11章
 */
bool convex_hull_3d_optimal(const std::vector<cocos2d::Vec3> &points,std::vector<Plane3*> &planes);
NS_GT_END
#endif