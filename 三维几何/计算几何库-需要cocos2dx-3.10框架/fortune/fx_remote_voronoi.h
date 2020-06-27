/*
  *��Զ��Voronoiͼ�㷨
  *@date:2020��6��25��
  *@author:xiaohuaxiong
 */
#ifndef __fx_remote_vronoi_h__
#define __fx_remote_vronoi_h__
#include "gt_common/geometry_types.h"
#include "data_struct/link_list.h"
#include "fk_data.h"
#include "math/Vec2.h"
#include <vector>
NS_GT_BEGIN
struct FxvSite;
struct FxvEdge {
	cocos2d::Vec2  origin, destination;
	struct FxvSite *site_ptr;
	FxvEdge *next, *prev,*twin;

	FxvEdge(const cocos2d::Vec2 &og, const cocos2d::Vec2 &dest, FxvSite *site) :origin(og), destination(dest), site_ptr(site),next(nullptr),prev(nullptr),twin(nullptr) {};
	FxvEdge() :site_ptr(nullptr), next(nullptr), prev(nullptr),twin(nullptr) {};
};

struct FxvSite {
	FxvEdge  *head_ptr;
	FxvEdge  *tail_ptr;
	cocos2d::Vec2  location;

	FxvSite(const cocos2d::Vec2 &local) :location(local), head_ptr(nullptr), tail_ptr(nullptr) {};
	FxvSite() :head_ptr(nullptr), tail_ptr(nullptr) {};
};
/*
  *����ɢ�㼯��͹��
 */
bool fx_create_clipper(std::vector<cocos2d::Vec2> &discard_points,link_list<cocos2d::Vec2> &cliper_list);
/*
  *�����������Զ��Voronoiͼ
 */
void fx_create_remote_voronoi3(link_list<cocos2d::Vec2> &cliper_list,std::vector<FxvSite> &site_array);
/*
  *����ɢ�㼯����Զ��Voronoiͼ
  *Ȼ���볣��Voronoiͼ�㷨��ͬ,��Զ��Voronoiͼ�����ܱ�֤ÿһ���㶼������ص����ݽṹ
  *��ͨ��,discard_points.size() != site_array.size()
 */
void fx_create_remote_voronoi(std::vector<cocos2d::Vec2> &discard_points,std::vector<FxvSite> &site_array);

NS_GT_END

#endif