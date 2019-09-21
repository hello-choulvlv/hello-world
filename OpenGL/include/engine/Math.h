/*
  *引擎中将会使用的数学函数
  *@note:需要指出的是Math.h与Geometry.h之间的区别在于,Math.h里面的函数之间的关系比较松散
  *@note:另外也包含了各个图形领域中可能会用到的一些专有函数，比如生成海平面的海浪高度场的函数phillips函数
  *@note:以及在统计波模型中需要用到的高斯函数
  *@date:2017-4-11
  *@Author:xiaohuaxiong
*/
#ifndef __MATH_H__
#define __MATH_H__
#include<engine/GLState.h>
__NS_GLK_BEGIN
//设置随机数种子
void		initSeed(unsigned seed);
//返回[0.0--1.0]之间的浮点数
float		randomValue();
//高斯函数
void		gauss(float work[2]);
/*Phillips函数
	*@param:a:幅度
	*@param:k:波数
	*@param:wind:风的速度
*/
float phillips(float	a, const float k[2], const float wind[2]);
/*
  *返回二元高斯概率分布
  *x,y坐标分布
  *want:数学期望
  *variance:方差
 */
float  gauss_distribution(float x,float y,float want_x,float ant_y,float variance);
__NS_GLK_END

#endif