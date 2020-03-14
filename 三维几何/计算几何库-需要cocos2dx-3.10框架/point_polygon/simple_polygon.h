/*
  *简单多边形算法,交并差,相对凸多边形,简单多边形的算法要复杂很多
  *2020年3月10日
  *@author:xiaoxiong
  *@version:1.0
 */
#ifndef __simple_polygon_h__
#define __simple_polygon_h__
#include "math/Vec2.h"
#include <vector>
#include "gt_common/geometry_types.h"
#include "data_struct/balance_tree.h"

NS_GT_BEGIN
enum SimpleEventType {
	EventType_Left = 0,//左侧顶点
	EventType_Right = 1,//右侧顶点
	EventType_Interleave = 2,//交叉点
};
/*
  *Simple Polygon edge
 */
struct simple_edge {
	cocos2d::Vec2 origin_point,final_point ;
	short owner_idx,point_idx,next_idx;
};

struct simple_event {
	cocos2d::Vec2 event_point;
	SimpleEventType event_type;
	short owner_idx, point_idx,next_idx,queue_idx;
	//如果是由于两条边交叉而产生的事件点,则需要指出相关的边,边的顺序遵循线段状态中给出的顺序
	//simple_edge *cross_edge1,*cross_edge2;
	red_black_tree<simple_edge>::internal_node *cross_edge1, *cross_edge2;
};

struct simple_polygon {

};
/*
  *第一版中,先求出所有的多边形交点
  *在第二版中,我们将所有的交点按顺序组织起来
  *函数假设所有的顶点按照顺时针排列,并且多边形中没有孔洞
  *在后面的函数视实现中,我们将会扩展该函数的功能.
  *并且完整的实现两简单多边形的交并差算法.
 */
bool simple_polygon_intersect(const std::vector<cocos2d::Vec2> &polygon1,const std::vector<cocos2d::Vec2>&polygon2,std::vector<cocos2d::Vec2> &intersect_array);
NS_GT_END

#endif