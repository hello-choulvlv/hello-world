/*
  *liquidʵ��,Shallow Water Equation
  *2018��12��19��
  *@author:xiaohuaxiong
 */
#ifndef __LIQUID_H__
#define __LIQUID_H__
#include "engine/Geometry.h"
typedef unsigned char byte;
class Liquid
{
	int    _resolutionX, _resolutionY;
	float						*_heightArray;
	float                        *_velocityXArray;
	float                        *_velocityYArray;

	float						*_nowHeightArray;
	float						*_nextHeightArray;

	float						*_nowVelocityXArray;
	float						*_nextVelocityXArray;

	float                        *_nowVelocityYArray;
	float                        *_nextVelocityYArray;

	byte      *_normal;
public:
	Liquid();
	~Liquid();
	void  init(int  resolution_x,int resolution_y);
	/*
	  *���Բ�ֵ
	 */
	float  blerp(float  *field,float local_x,float local_y);
	/*
	  *��Ը������������ݶ�����
	 */
	void  advect(float *field_now, float *field_next, float d_x,float d_y,float t);
	/*
	  *���¸߶ȳ�
	 */
	void  updateHeight(float t);
	/*
	  *���±߽�
	 */
	void  updateBoundary(float t);
	/*
	  *�����ٶȳ�
	 */
	void  updateVelocity(float t);
	/*
	  *���㷨��
	 */
	void  computeNormal();
	/*
	  *ÿ֡����
	 */
	void  update(float t);
	/*
	  *��ȡ����
	 */
	byte *getNormal()const;
	/*
	  *������
	 */
	void  touchAt(float x,float y);
};
#endif