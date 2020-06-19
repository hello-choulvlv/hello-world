/*
  *离散点集的Voronoi图之海滩线算法实现
  *2020年6月9日
  *@author:xiaohuaxiong
 */
#ifndef __fk_algorithm_h__
#define __fk_algorithm_h__
#include "gt_common/geometry_types.h"
#include "data_struct/priority_queue.h"
#include "fk_data.h"
#include "fk_diagram.h"
#include "beach_line.h"
#include "math/Vec2.h"
#include <functional>

NS_GT_BEGIN

class FortuneAlgorithm {
	FkDiagram    _diagram;

	std::function<float(const FxArc *,const FxArc *)> _compute_func;
	std::function<bool(FkEvent *const&, FkEvent *const &)>         _compare_func;
	std::function<int(FkEvent *&, int)>   _modify_func;

	//事件队列
	priority_queue<FkEvent*>   _event_queue;
	FxBeachLine                             _beach_line;
	//海滩线的基准线
	float                _beachLineY;
public:
	FortuneAlgorithm(const std::vector<cocos2d::Vec2> &locations);
	~FortuneAlgorithm() = default;

	void  build();
	void  bound();//设定边界
	void  handleSiteEvent(FkEvent  *event_ptr);
	void  handleCircleEvent(FkEvent *event_ptr);

	//分裂抛物线弧
	FxArc*  breakArc(FxArc  *middle_arc, FkSite  *target_site);
	//删除与弧相关联的事件
	void  removeEvent(FxArc  *target_arc);
	//删除相关联的弧
	void removeArc(FkEvent  *event_ptr);
	//在两条相邻的弧之间增加关联边
	void addEdge(FxArc *left_edge,FxArc *right_edge);
	//添加事件,该事件特指圆事件
	void addEvent(FxArc *left_arc,FxArc *middle_arc, FxArc *right_arc);
	//
	void setDestination(FxArc *left_arc, FxArc *right_arc,cocos2d::Vec2 *location);
	void setOrigin(FxArc *left_arc, FxArc *right_arc,cocos2d::Vec2 *location);
	void setPrevEdge(FxEdge *prev, FxEdge *next);

	FkDiagram &getDiagram() { return _diagram; };
};

NS_GT_END
#endif