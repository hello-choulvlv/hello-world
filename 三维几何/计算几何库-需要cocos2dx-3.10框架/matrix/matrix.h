/*
  *������������
 *matrix�任,���3x3,4x4,ȫ��ʹ���о���,���Ҿ���˷�,�����������ĳ˷�Ҳ�ϸ������о���任����
 *һЩ���������㷨
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
//�������α任����
/*
*����Ļ������Ե��
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
  *��ѧ����
 */
float clampf(float min,float max,float v);
float randomf10();
float random();
/*
*��Զ�ά��Ĳ���,�з������
*/
float sign_area(const cocos2d::Vec2 &db, const cocos2d::Vec2 &dc);
//���������֮��ĳ˷�,���º�����������ת������3ά����֮��ı任
void  vec3_transfrom_mat3(const cocos2d::Vec3 &t,const mat3x3 &mat,cocos2d::Vec3 &dst);
void  mat3_transform_vec3(const mat3x3 &mat,const cocos2d::Vec3 &t,cocos2d::Vec3 &dst);
/*
*�����Ĳ��
*/
cocos2d::Vec3  cross(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b);
cocos2d::Vec3  cross_normalize(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b);
float cross(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b);
float cross(const cocos2d::Vec2 &base,const cocos2d::Vec2 &fc1,const cocos2d::Vec2 &fc2);
/*
*�����淶��
*/
cocos2d::Vec3  normalize(const cocos2d::Vec3 &v);
cocos2d::Vec3 normalize(float x, float y, float z);
cocos2d::Vec2 normalize(const cocos2d::Vec2 &v);
cocos2d::Vec2 normalize(const cocos2d::Vec2 &u, const cocos2d::Vec2 &v);
cocos2d::Vec2 normalize(float x,float y);
/*
  *3x3����任
 */
//���ص�λ����
void  mat3_load_identity(mat3x3 &mat);
//��ת����,axis�����ǵ�λ����
void  mat3_create_rotate(mat3x3 &mat,const cocos2d::Vec3 &axis,float angle);
/*
  *3x3����˷�
 */
void  mat3_mutiply(const mat3x3 &a,const mat3x3 &b,mat3x3 &dst);

////////////////////////////mat4x4����任////////////////////////////
void mat4_load_identity(mat4x4 &mat);
void mat4_multiply(const mat4x4 &a,const mat4x4 &b,mat4x4 &dst);

/*
*������
*/
template<typename TK>
void  quick_sort(TK *source, int   tk_num, std::function<bool(const TK &a, const TK &b)> &compare_func)
{
	//�����㷨Ŀǰ�Ȳ��ò�������,�������ǽ���ʹ�ù鲢����
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

			while (base_j < l_boundary && other_j < r_boundary)//�߽�
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
			//����Ƿ���ĳЩԪ�ػ�û����ȫ�������
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

//��һ�������㷨,��֮ǰ����Ƚ϶���,�������ڱȽϺ���
template<typename TM>
void quick_sort_origin_type(TM *source, int tk_num, std::function<bool(const TM a, const TM b)> &compare_func)
{
	//�����㷨Ŀǰ�Ȳ��ò�������,�������ǽ���ʹ�ù鲢����
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

			while (base_j < l_boundary && other_j < r_boundary)//�߽�
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
			//����Ƿ���ĳЩԪ�ػ�û����ȫ�������
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
/*
  *��תvector�е�Ԫ��
 */
template<typename VT>
void vector_reverse(std::vector<VT> &param_v) {
	VT *data_array = param_v.data();
	for (int left_l = 0, right_l = param_v.size() - 1; left_l < right_l; ++left_l, --right_l)
		std::swap(data_array[left_l],data_array[right_l]);
}
NS_GT_END
#endif