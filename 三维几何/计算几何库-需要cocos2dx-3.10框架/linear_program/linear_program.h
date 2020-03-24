/*
  *���Թ滮�㷨
  *2020��3��23�� 
  *@author:xiaohuaxiong
  *@@version:1.0ʵ�ֻ����Ķ�ά���Թ滮
 */
#ifndef __linear_program_h__
#define __linear_program_h__
#include  "math/Vec2.h"
#include <vector>
#include "gt_common/geometry_types.h"
#include "line/line.h"

NS_GT_BEGIN
/*
*��ά���Թ滮����,����Լ��,ֱ�ߵ����Ϊ��ƽ������
*�����Ѿ��ٶ�,������ֱ�߽߱��������Ѿ��������������߽��,Ҳ����������ֱ���޶��ı߽�����
*���������������ݵ����,˳��Ϊ��/��/��/��
*����û���ظ�������,����������������ֱ��������ͬ�ķ���
*target_coeff:Ŀ�꺯����ϵ��
*����ҵ����ֵ,�򷵻�1,���������н�
*����޽�,����ֵΪ2�ҷ�����ص����޵�������
*���򷵻�0
*���㷨����S&I�㷨˼��
*/
int linearly_program_2d(const std::vector<Line2D> &lines, float coeff_array[3], std::vector<cocos2d::Vec2>&intersect_array2, cocos2d::Vec2 &interect_point, cocos2d::Vec2 &direction);

NS_GT_END
#endif