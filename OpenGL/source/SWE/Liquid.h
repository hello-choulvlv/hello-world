/*
  *liquid实现,Shallow Water Equation
  *2018年12月19日
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
	  *线性插值
	 */
	float  blerp(float  *field,float local_x,float local_y);
	/*
	  *针对给定的域增加梯度向量
	 */
	void  advect(float *field_now, float *field_next, float d_x,float d_y,float t);
	/*
	  *更新高度场
	 */
	void  updateHeight(float t);
	/*
	  *更新边界
	 */
	void  updateBoundary(float t);
	/*
	  *更新速度场
	 */
	void  updateVelocity(float t);
	/*
	  *计算法线
	 */
	void  computeNormal();
	/*
	  *每帧调用
	 */
	void  update(float t);
	/*
	  *获取法线
	 */
	byte *getNormal()const;
	/*
	  *触屏点
	 */
	void  touchAt(float x,float y);
};
#endif