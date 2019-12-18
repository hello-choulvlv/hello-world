/*
  *Voronoiͼ�㷨+ƽ��㼯�������ʷ��㷨
  *��Ϊ���㷨ϵ�бȽ��Ӵ���Ҹ���,�������Ŷ��ֵ���ȫ��ͬ��˼·,����֮���������еĹ�ϵ
  *�����Ҫ������ȡ�����γ�һ���ļ�.
  *2019��12��12��
  *@author:xiaoxiong
*/
#include "gt_common/geometry_types.h"
#include "math/Vec2.h"
#include "math/CCGeometry.h"

NS_GT_BEGIN
struct Cycle;
/*
  *�����ζ�������
 */
struct DelaunayTriangle
{
	short v1, v2, v3;
};
//��
struct DelaunayEdge
{
	short v1, v2;
};
bool operator ==(const DelaunayEdge &, const DelaunayEdge &other);
bool operator >(const DelaunayEdge &, const DelaunayEdge &);
bool operator < (const DelaunayEdge &, const DelaunayEdge &);

void static_create_cycle_by_triangle(Cycle &cycle, const std::vector<cocos2d::Vec2> &disper_points, const DelaunayTriangle &delaunay_triangle);
/*
  *��������ε���С���������
  *��������Ϊһ����������
 */
void rect_outerline_triangle(const cocos2d::Vec2 &origin,const cocos2d::Vec2 &extent,cocos2d::Vec2 triangle[3]);
/*
  *������ɢ�㼯��Delaunay�����ʷ�
  *ʹ��Bowyer-Watson�㷨
  *�㷨���������Ѿ�������صļ���
  *����ӳ��������εĶ�������λ���������
  *������Ч��������ݴ���3��
*/
void delaunay_triangulate_bowyer_washton(const std::vector<cocos2d::Vec2> &disper_points,std::vector<DelaunayTriangle> &triangle_sequence,int &real_size);
NS_GT_END