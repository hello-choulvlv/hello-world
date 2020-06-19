/*
  *对离散点集的Voronoi图进行限制的数据结构
  *2020年6月12日
  *@author:xiaohuaxiong
 */
#ifndef __fk_box_h__
#define __fk_box_h__
#include "gt_common/geometry_types.h"
#include "math/Vec2.h"

NS_GT_BEGIN
//包围盒的面类型枚举
enum FkBoxSideType {
	SideType_None = -1,
	SideType_Left = 0,
	SideType_Bottom = 1,
	SideType_Right = 2,
	SideType_Top = 3,
};
/*
  *交点数据结构
 */
struct FxIntersection {
	FkBoxSideType side_type;
	cocos2d::Vec2   intersect_point;
};
/*
  *限制包围盒
 */
struct FkBox {
	cocos2d::Vec2  left_bottom, extent;

	//线段求交
	bool intersectWith(const cocos2d::Vec2 &origin,const cocos2d::Vec2 &normal,FxIntersection &intersect_info);
};
NS_GT_END
#endif