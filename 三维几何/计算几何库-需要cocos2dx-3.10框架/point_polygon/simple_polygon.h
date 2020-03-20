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
#include<list>
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
//简单多边形之间的交点
struct simple_interleave {
	cocos2d::Vec2 interleave_point;//交叉点的坐标
	short point_idx1,point_idx2;//所属的多边形以及当前交叉点所在的边,point_idx-----point_idx+1
	short ref;//访问标志0未被访问过,1已经被访问过
};

struct simple_interleave_ref {
	const simple_interleave *simple_ref;
	int	   point_idx;
};
/*
  *第一版中,先求出所有的多边形交点
  *在第二版中,我们将所有的交点按顺序组织起来
  *函数假设所有的顶点按照顺时针排列,并且多边形中没有孔洞
  *在后面的函数视实现中,我们将会扩展该函数的功能.
  *并且完整的实现两简单多边形的交并差算法.
 */
bool simple_polygon_intersect(const std::vector<cocos2d::Vec2> &polygon1,const std::vector<cocos2d::Vec2>&polygon2,std::vector<simple_interleave> &intersect_array);
/*
  *将给出的交叉点数据结构转换为多边形的交集,注意有可能会产生不止一个多边形
  *中间如果有多个多边形,则使用索引-1隔开
  *如果多边形都是星型多边形,则算法将会更加简单
  *在这里假设只是普通的简单多边形
  *目前给出了求交算法,实际上根据下述算法的解码过程,完全可以写出求并,差的算法
  *该任务留给了读者自己实现,哥太累了最近.
 */
bool simple_polygon_interleave_set(const std::vector<cocos2d::Vec2>&, const std::vector<cocos2d::Vec2> &polygon2, const std::vector<simple_interleave>&,std::list<std::vector<cocos2d::Vec2>> &polygon_intersect_array);
/*
  *简单多边形与目标点之间的包含测试
  *算法的核心思想为,以目标点为基准点向上下作垂直射线
  *计算simple polygon的边与两个射线的相交数目,如果两侧相交的数目都为奇数,则表示相交
  *分则则意味着分离
 */
bool simple_polygon_contains_point(const std::vector<cocos2d::Vec2>&simple_polygon,const cocos2d::Vec2 &target_point);
NS_GT_END

#endif