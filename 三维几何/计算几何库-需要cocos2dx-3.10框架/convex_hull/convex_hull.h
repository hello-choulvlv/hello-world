/*
  *͹���㷨,2d/3dʵ��
  *2020��2��13��
  */
#ifndef __CONVEX_HULL_H__
#define __CONVEX_HULL_H__
#include "gt_common/geometry_types.h"
#include "math/Vec2.h"
#include "math/Vec3.h"
#include<vector>

NS_GT_BEGIN
/*
  *quick hull 2d �㷨��Ҫʹ�õ����ݽṹ
 */
struct QuickHull
{
	int start_l, final_l;
	std::vector<int>      operate_points;
};
/*
  *3dСƽ��ı�ʾ
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
  *�ڴ���������
  *���ƽ�����
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
*��㼯����С͹�����
*��ת�����㷨ʵ��
*/
bool polygon_compute_convex_hull(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon_points);
/*
*quick hull�㷨ʵ��
*2dʵ��
*/
bool quick_hull_algorithm2d(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon);
/*
*quick hull �㷨ʵ��,���㷨ֻʵ�ֻ����ĵ��������,���ڳ��ֶ�㹲������,����ʱ������
*���������Ҫʹ���걸���㷨ʵ��,��������иĶ�����ʵ��,ԭ����ͬ,�㷨�ṹҲֻ����΢����һЩ����
*��С3d͹ƽ��,ע����㷨��û�о����Ż�,�ھ�����ȷ�Բ��Ժ�,���ǽ����������Ż�.
*���,ÿ�������㹹��һ���ռ�ƽ��
*�����԰汾��,���ǽ�����㷨�����Ż�.�����ڴ�����㷨,������ݲ����㷨.
*��ǰ�Ĳ��Ե�����������128
*@version:1.0ʵ�ֻ����㷨
*@version:1.0->algorithm cost time:11.70
*@version:2.0�޸��㷨����ʱ����©ĳЩ��Ĵ������,���Ż��ڴ����,�������ݸ���,�Լ��㴦���������
*@version2.0->algorithm cost time:10.30
*@version:3.0�����󼫵���������,���ù��������ͻ����̺ϲ�,�Լ���ƽ��ķ���ֱ�Ӵ洢,����ÿ�α���ʱ�����¼���.
*@version:3.0����������ʵ��,�㷨������ʱ�����Ե�������
*@version3.0->algorithm cost time:7.01
*******************************************************************
*@version:4.0�ڵ��İ���,���ǽ���ʹ��һЩ�߼������ݽṹ����һ���������㷨������ʱ����.
*@version:4.1->seed:1581910632:�Ż�ǰ:7.95,�Ż���:6.49,
*@version4.2:��ʹ�������ȼ�����֮��,����ʱ��Ϊalgorithm cost time:4.06
*@note:���ϵ��Ż����̶���Ϊ����һ��������Ҳ��Ϊ��Ч���㷨��ʵ�ֹ�����׼��.
*/
bool quick_hull_algorithm3d(const std::vector<cocos2d::Vec3> &points, std::vector<Plane3*> &planes);
/*
  *��ά͹���㷨֮�Ż�ʵ��
  *���㷨Դ����<���㼸��>�ĵ�11��
 */
bool convex_hull_3d_optimal(const std::vector<cocos2d::Vec3> &points,std::vector<Plane3*> &planes);
NS_GT_END
#endif