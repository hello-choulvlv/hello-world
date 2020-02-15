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
#include<list>

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
	short v1,v2,v3, ref;
	std::vector<int> operate_array;

	ConvexEdge *head, *tail;

	Plane3(short av1, short av2, short av3) :v1(av1),v2(av2),v3(av3),ref(0),head(nullptr),tail(nullptr){};
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
*/
bool quick_hull_algorithm3d(const std::vector<cocos2d::Vec3> &points, std::list<Plane3*> &planes);
NS_GT_END
#endif