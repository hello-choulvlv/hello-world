/*
  *视锥体裁剪算法,几何体裁剪,阴影体裁剪
  *2020年4月17日
  *@author:xiaohuaxiong
 */
#ifndef __frustum2_h__
#define __frustum2_h__
#include "math/Vec4.h"
#include "math/Vec3.h"
#include "math/Mat4.h"
#include "gt_common/geometry_types.h"
#include "aabb_obb/aabb_obb.h"

NS_GT_BEGIN
/*
  *关于命名上,为了与cocos2d引擎中的视锥体有所区别,因此命名上也有所变动
 */
class Frustum2 {
	cocos2d::Vec4   _planes[6];//几何体裁剪,6个平面方程,法线方向朝向外侧
	cocos2d::Vec4   _shaodwPlanes[12];//阴影体裁剪,裁剪平面的数目并非是固定的,
	int                        _shadowNum;
	
public:
	Frustum2():_shadowNum(0) {};
	/*
	  *初始化几何体裁剪平面
	 */
	void initGeometryPlanes(const cocos2d::Mat4 &view_proj_mat);
	/*
	  *初始化阴影体裁剪平面
	  *裁剪的规则为:视锥体平面 + 光线的反方向
	  *输入参数为:视锥体的8个顶点坐标,坐标的排列规则为:近平面 + 远平面,顶点的顺序为从左下角开始,逆时针顺序排列
	 */
	void initShadowPlanes(const cocos2d::Vec3 frustum_vertex[8],const cocos2d::Vec3 &light_direction);
	/*
	  *是否几何体位于视锥体裁剪平面之内,也包括相交情况
	 */
	bool isLocateInFrustum(const AABB &aabb)const;
	bool isLocateInFrustum(const OBB &obb)const;
	/*
	  *是否几何体位于视锥体的阴影体裁剪平面之内,包括相交的情况
	 */
	bool isLocateInShadowVolumn(const AABB &aabb)const;
	bool isLocateInShadowVolumn(const OBB &obb)const;
};
NS_GT_END
#endif