/*
  *����ɢ�㼯��Voronoiͼ�������Ƶ����ݽṹ
  *2020��6��12��
  *@author:xiaohuaxiong
 */
#ifndef __fk_box_h__
#define __fk_box_h__
#include "gt_common/geometry_types.h"
#include "math/Vec2.h"

NS_GT_BEGIN
//��Χ�е�������ö��
enum FkBoxSideType {
	SideType_None = -1,
	SideType_Left = 0,
	SideType_Bottom = 1,
	SideType_Right = 2,
	SideType_Top = 3,
};
/*
  *�������ݽṹ
 */
struct FxIntersection {
	FkBoxSideType side_type;
	cocos2d::Vec2   intersect_point;
};
/*
  *���ư�Χ��
 */
struct FkBox {
	cocos2d::Vec2  left_bottom, extent;

	//�߶���
	bool intersectWith(const cocos2d::Vec2 &origin,const cocos2d::Vec2 &normal,FxIntersection &intersect_info);
};
NS_GT_END
#endif