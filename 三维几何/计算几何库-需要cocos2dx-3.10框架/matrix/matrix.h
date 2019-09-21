/*
 *matrix变换,针对3x3,4x4,全部使用行矩阵,并且矩阵乘法,矩阵与向量的乘法也严格遵照行矩阵变换进行
 *2019/6/16
 *@author:xiaohuaxiong
 */
#ifndef _GT_MATRIX_H__
#define _GT_MATRIX_H__
#include "math/Vec2.h"
#include "math/Vec3.h"
#include "gt_common/geometry_types.h"
NS_GT_BEGIN
struct mat3x3
{
	float m[9];
};

struct mat4x4
{
	float m[16];
};
//基本几何变换函数
/*
*顶点的基本测试点乘
*/
float  dot(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b);
float  dot(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b);
float  dot_abs(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b);

float  length2(const cocos2d::Vec3 &a);
float  length2(const cocos2d::Vec2 &a);
float  length2(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b);
float  length2(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b);

float  length(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b);
float  length(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b);
float  length(const cocos2d::Vec2 &a);
float  length(const cocos2d::Vec3 &a);
/*
  *数学函数
 */
float clampf(float min,float max,float v);
float randomf10();
float random();
/*
*针对二维点的测试,有符号面积
*/
float sign_area(const cocos2d::Vec2 &db, const cocos2d::Vec2 &dc);
//向量与矩阵之间的乘法,以下函数多用于旋转矩阵与3维向量之间的变换
void  vec3_transfrom_mat3(const cocos2d::Vec3 &t,const mat3x3 &mat,cocos2d::Vec3 &dst);
void  mat3_transform_vec3(const mat3x3 &mat,const cocos2d::Vec3 &t,cocos2d::Vec3 &dst);
/*
*向量的叉乘
*/
cocos2d::Vec3  cross(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b);
cocos2d::Vec3  cross_normalize(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b);
float cross(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b);
/*
*向量规范化
*/
cocos2d::Vec3  normalize(const cocos2d::Vec3 &v);
cocos2d::Vec3 normalize(float x, float y, float z);
cocos2d::Vec2 normalize(const cocos2d::Vec2 &v);
cocos2d::Vec2 normalize(float x,float y);
/*
  *3x3矩阵变换
 */
//加载单位矩阵
void  mat3_load_identity(mat3x3 &mat);
//旋转矩阵,axis必须是单位向量
void  mat3_create_rotate(mat3x3 &mat,const cocos2d::Vec3 &axis,float angle);
/*
  *3x3矩阵乘法
 */
void  mat3_mutiply(const mat3x3 &a,const mat3x3 &b,mat3x3 &dst);

////////////////////////////mat4x4矩阵变换////////////////////////////
void mat4_load_identity(mat4x4 &mat);
void mat4_multiply(const mat4x4 &a,const mat4x4 &b,mat4x4 &dst);

NS_GT_END
#endif