/*
  *ƽ��ͷ��ʵ��,��Ҫ����ģ�Ϳ������жϷ���
  *@date:2017-5-20
  *@Author:xiaohuaxiong
*/
#ifndef __FRUSTUM_H__
#define __FRUSTUM_H__
#include"engine/Geometry.h"
__NS_GLK_BEGIN
class Frustum
{
	//����ƽ�淽��
	Plane     _planes[6];
public:
	//ʹ����ͼͶӰ��������������ƽ��
	Frustum(const Matrix &viewProjMatrix);
	Frustum();
	//ʹ����ͼͶӰ������������ƽ�淽��
	void       init(const Matrix &viewProjMatrix);
	/*
	  *ʹ����Ļ���Ƽ�NDC�����һ���ֽ�����׵��
	  *lndcX,rndcX means left and right of ndc coordinate boundary
	  *bndcY,tndcY means bottom and top ndc coordinate boundary
	 */
	void       init(float lndcX,float rndcX, float bndcY,float tndcY,const Matrix &viewProjMatrixInverse);
	//�ж�ģ�͵İ�Χ���Ƿ���ƽ��ͷ���пɼ�
	bool       isOutOfFrustum(const AABB &box)const;
	//�ж϶����Ƿ�����׶���ڿɼ�
	bool      isOutOfFrustum(const GLVector3 &position)const;
};
__NS_GLK_END
#endif