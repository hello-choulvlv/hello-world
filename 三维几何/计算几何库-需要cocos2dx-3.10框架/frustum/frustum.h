/*
  *��׶��ü��㷨,������ü�,��Ӱ��ü�
  *2020��4��17��
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
  *����������,Ϊ����cocos2d�����е���׶����������,���������Ҳ�����䶯
 */
class Frustum2 {
	cocos2d::Vec4   _planes[6];//������ü�,6��ƽ�淽��,���߷��������
	cocos2d::Vec4   _shaodwPlanes[12];//��Ӱ��ü�,�ü�ƽ�����Ŀ�����ǹ̶���,
	int                        _shadowNum;
	
public:
	Frustum2():_shadowNum(0) {};
	/*
	  *��ʼ��������ü�ƽ��
	 */
	void initGeometryPlanes(const cocos2d::Mat4 &view_proj_mat);
	/*
	  *��ʼ����Ӱ��ü�ƽ��
	  *�ü��Ĺ���Ϊ:��׶��ƽ�� + ���ߵķ�����
	  *�������Ϊ:��׶���8����������,��������й���Ϊ:��ƽ�� + Զƽ��,�����˳��Ϊ�����½ǿ�ʼ,��ʱ��˳������
	 */
	void initShadowPlanes(const cocos2d::Vec3 frustum_vertex[8],const cocos2d::Vec3 &light_direction);
	/*
	  *�Ƿ񼸺���λ����׶��ü�ƽ��֮��,Ҳ�����ཻ���
	 */
	bool isLocateInFrustum(const AABB &aabb)const;
	bool isLocateInFrustum(const OBB &obb)const;
	/*
	  *�Ƿ񼸺���λ����׶�����Ӱ��ü�ƽ��֮��,�����ཻ�����
	 */
	bool isLocateInShadowVolumn(const AABB &aabb)const;
	bool isLocateInShadowVolumn(const OBB &obb)const;
};
NS_GT_END
#endif