/*
  *最远点Voronoi图算法
  *@date:2020年6月25日
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
  *求离散点集的凸壳
 */
bool fx_create_clipper(std::vector<cocos2d::Vec2> &discard_points,link_list<cocos2d::Vec2> &cliper_list);
/*
  *求三个点的最远点Voronoi图
 */
void fx_create_remote_voronoi3(link_list<cocos2d::Vec2> &cliper_list,std::vector<FxvSite> &site_array);
/*
  *求离散点集的最远点Voronoi图
  *然而与常规Voronoi图算法不同,最远点Voronoi图并不能保证每一个点都会有相关的数据结构
  *即通常,discard_points.size() != site_array.size()
 */
void fx_create_remote_voronoi(std::vector<cocos2d::Vec2> &discard_points,std::vector<FxvSite> &site_array);

NS_GT_END

#endif