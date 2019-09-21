/*
  *�����н���ʹ�õ���ѧ����
  *@note:��Ҫָ������Math.h��Geometry.h֮�����������,Math.h����ĺ���֮��Ĺ�ϵ�Ƚ���ɢ
  *@note:����Ҳ�����˸���ͼ�������п��ܻ��õ���һЩר�к������������ɺ�ƽ��ĺ��˸߶ȳ��ĺ���phillips����
  *@note:�Լ���ͳ�Ʋ�ģ������Ҫ�õ��ĸ�˹����
  *@date:2017-4-11
  *@Author:xiaohuaxiong
*/
#ifndef __MATH_H__
#define __MATH_H__
#include<engine/GLState.h>
__NS_GLK_BEGIN
//�������������
void		initSeed(unsigned seed);
//����[0.0--1.0]֮��ĸ�����
float		randomValue();
//��˹����
void		gauss(float work[2]);
/*Phillips����
	*@param:a:����
	*@param:k:����
	*@param:wind:����ٶ�
*/
float phillips(float	a, const float k[2], const float wind[2]);
/*
  *���ض�Ԫ��˹���ʷֲ�
  *x,y����ֲ�
  *want:��ѧ����
  *variance:����
 */
float  gauss_distribution(float x,float y,float want_x,float ant_y,float variance);
__NS_GLK_END

#endif