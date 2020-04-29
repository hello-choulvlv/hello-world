/*
  *点定位算法实现
  *2020年4月13日
  *@author:xiaohuaxiong
 */
#ifndef __point_location_h__
#define __point_location_h__
#include "math/Vec2.h"
#include "line/line.h"
#include "gt_common/geometry_types.h"
#include <vector>

NS_GT_BEGIN
//节点的类型
enum LocalType {
	LocalType_Endpoint = 0,//端点
	LocalType_Segment = 1,//线段
	LocalType_Trapzoid = 2,//梯形
};
struct NodeLocal;
//梯形
struct Trapzoid {
	cocos2d::Vec2   left_point, right_point;//左右端点
	Segment2D       *up_seg_ptr, *low_seg_ptr;//上线段,下线段
	NodeLocal	        *left_up, *left_low, *right_up, *right_low;//左上/左下/右上/右下梯形邻居,注意left_up,left_low/right_up,right_low的值可能相等

	Trapzoid(const cocos2d::Vec2 &aleft,const cocos2d::Vec2 &aright) :left_point(aleft),right_point(aright)
		,up_seg_ptr(nullptr)
		,low_seg_ptr(nullptr)
		,left_up(nullptr)
		,left_low(nullptr)
		,right_up(nullptr)
		,right_low(nullptr){};
};

struct NodeLocal {
	LocalType  node_type;
	cocos2d::Vec2   *endpoint_ptr;//端点
	Segment2D         *segment_ptr;//线段
	Trapzoid              *trap_ptr;//梯形
	NodeLocal  *child_l, *child_r;
	int                           ref;//访问标志,在相关节点被销毁时使用

	NodeLocal(LocalType  atype) :node_type(atype)
		,endpoint_ptr(nullptr)
		,segment_ptr(nullptr)
		,trap_ptr(nullptr)
		,child_l(nullptr)
		,child_r(nullptr)
		, ref(0){};
};

struct LocationLexer {
	NodeLocal *root;
	int                  size;

	LocationLexer() :root(nullptr), size(0) {};
	~LocationLexer();
};
/*
  *遍历
 */
void local_point_visit(LocationLexer &lexer, std::vector<NodeLocal *> &node_array, short ref_result);
/*
*点查找算法实现
*/
NodeLocal*   local_point_find_location(LocationLexer &lexer, const cocos2d::Vec2 &point);
/*
  *建立梯形图
  *输入参数:初始梯形图数据结构,最后的两个线段表示
  *初始边界,其足够涵盖所有的线段端点,边界的排列方式位下线段,上线段,其对应者一个平行坐标轴的矩形的下上边界
  *输入的线段表示中,start_point.x > final_point.x始终保持真值,否则算法将返回产生错误
 */
void local_point_create_trapzoid(LocationLexer &lexer,std::vector<Segment2D> &segments);

NS_GT_END
#endif