/*
  *公共函数集合
 *matrix变换,针对3x3,4x4,全部使用行矩阵,并且矩阵乘法,矩阵与向量的乘法也严格遵照行矩阵变换进行
 *一些基本排序算法
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
float cross(const cocos2d::Vec2 &base,const cocos2d::Vec2 &fc1,const cocos2d::Vec2 &fc2);
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

/*
*排序函数
*/
template<typename TK>
void  quick_sort(TK *source, int   tk_num, std::function<bool(const TK &a, const TK &b)> &compare_func)
{
	//排序算法目前先采用插入排序,后面我们将会使用归并排序
	TK   *bubble = new TK[tk_num];
	int     step = 1, half = tk_num / 2;
	TK  *t1 = source, *t2 = bubble;
	for (; step < tk_num; step *= 2)
	{
		int  base_j = 0;
		//compare and exchange
		for (int index_j = 0; index_j < tk_num; index_j += step * 2)
		{
			int  base_j = index_j;
			int  l_index = index_j;
			int  other_j = index_j + step;
			int l_boundary = other_j < tk_num ? other_j : tk_num;
			int r_boundary = other_j + step < tk_num ? other_j + step : tk_num;

			while (base_j < l_boundary && other_j < r_boundary)//边界
			{
				if (compare_func(t1[base_j], t1[other_j]))
				{
					t2[l_index] = t1[base_j];
					++base_j;
				}
				else
				{
					t2[l_index] = t1[other_j];
					++other_j;
				}
				++l_index;
			}
			//检查是否有某些元素还没有完全参与计算
			for (; base_j < l_boundary; ++base_j, ++l_index) t2[l_index] = t1[base_j];
			for (; other_j < r_boundary; ++other_j, ++l_index)t2[l_index] = t1[other_j];
		}
		TK *t = t1;
		t1 = t2; t2 = t;
	}
	if (t1 != source)
		memcpy(source, t1, sizeof(TK) * tk_num);

	delete[] bubble;
}

//另一种排序算法,与之前的相比较而言,区别在于比较函数
template<typename TM>
void quick_sort_origin_type(TM *source, int tk_num, std::function<bool(const TM a, const TM b)> &compare_func)
{
	//排序算法目前先采用插入排序,后面我们将会使用归并排序
	TM   *bubble = new TM[tk_num];
	int     step = 1, half = tk_num / 2;
	TM  *t1 = source, *t2 = bubble;
	for (; step < tk_num; step *= 2)
	{
		int  base_j = 0;
		//compare and exchange
		for (int index_j = 0; index_j < tk_num; index_j += step * 2)
		{
			int  base_j = index_j;
			int  l_index = index_j;
			int  other_j = index_j + step;
			int l_boundary = other_j < tk_num ? other_j : tk_num;
			int r_boundary = other_j + step < tk_num ? other_j + step : tk_num;

			while (base_j < l_boundary && other_j < r_boundary)//边界
			{
				if (compare_func(t1[base_j], t1[other_j]))
				{
					t2[l_index] = t1[base_j];
					++base_j;
				}
				else
				{
					t2[l_index] = t1[other_j];
					++other_j;
				}
				++l_index;
			}
			//检查是否有某些元素还没有完全参与计算
			for (; base_j < l_boundary; ++base_j, ++l_index) t2[l_index] = t1[base_j];
			for (; other_j < r_boundary; ++other_j, ++l_index)t2[l_index] = t1[other_j];
		}
		TM *t = t1;
		t1 = t2; t2 = t;
	}
	if (t1 != source)
		memcpy(source, t1, sizeof(TM) * tk_num);

	delete[] bubble;
}
NS_GT_END
#endif