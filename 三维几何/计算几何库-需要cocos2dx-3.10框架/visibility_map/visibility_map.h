/*
  *最短路径+求可见性图算法实现
  *2020年8月7日
  *@author:xiaohuaxiong
 */
#ifndef __visibility_map_h__
#define __visibility_map_h__
#include "math/Vec2.h"
#include <vector>
#include "gt_common/geometry_types.h"
#include "data_struct/balance_tree.h"

NS_GT_BEGIN
struct rtSegment;
/*
  *顶点,注意,里面的数据除了location_ptr之外,其它的域有可能是nullptr
 */
struct rtVertex {
	cocos2d::Vec2  *location_ptr;//目标点的坐标
	std::vector<cocos2d::Vec2> *owner_ptr;//顶点所在的多边形顶点序列容器
	bool                      visible;//记录其可见性,在稍后的算法运作过程中,我们将会使用到这个数据
	rtSegment         *next_seg_ptr, *next_seg_ptr2;//next表示下一个即将扫掠的线段,有可能会有两个
	rtSegment			*prev_seg_ptr,*prev_seg_ptr2;//prev_seg表示正在/已经处理的线段,注意顶点的前驱线段可能有两个

	rtVertex(cocos2d::Vec2 *location_ptrc = nullptr,std::vector<cocos2d::Vec2> *owner_ptrc = nullptr) :location_ptr(location_ptrc), owner_ptr(owner_ptrc), visible(false),next_seg_ptr(nullptr), next_seg_ptr2(nullptr),prev_seg_ptr(nullptr), prev_seg_ptr2(nullptr){};
};
/*
  *线段
 */
struct rtSegment {
	rtVertex  *start_ptr, *dest_ptr;
	red_black_tree<rtSegment*>::internal_node *internal_ptr;//方便快速查找/定位/删除

	rtSegment(rtVertex *astart_ptr = nullptr,rtVertex *bdest_ptr = nullptr):start_ptr(astart_ptr),dest_ptr(bdest_ptr), internal_ptr(nullptr){};
};
/*
  *可见性图算法实现
  *输入的多边形顶点序列假设都是按照逆时针排列
 */
std::vector<rtVertex*>* rt_compute_visibility_map(std::vector<cocos2d::Vec2> *polygons,int  array_size,cocos2d::Vec2 &start_point,cocos2d::Vec2 &target_point);
/*
  *迪杰斯特拉算法实现
  *有兴趣的读者可以自行实现,因为其是一个著名的/资料非常详尽的算法,因此在这里我们就不再赘述
  *计算几何算法库的所有工作到这里就要结束了
  *如果后续还有添加的话,则只会是零星的,而不再是有计划,系统的构建
  *读者也要各自保重,作者的集合算法学习计划也终结了
  *2020/8/11/17:35
 */
bool rt_dijkstra_algorithm(rtVertex *vertex_adj,int array_size,float *distance_array,int *path);
NS_GT_END
#endif