/*
  *2021年1月8日
  *gjk+chung-wang+sse指令优化算法实现
  *@author:xiaohuaxiong
 */
#include "polygon_mix.h"
#include "matrix/matrix.h"
//#include <mmintrin.h>   //mmx
#include <xmmintrin.h>  //sse
#include <emmintrin.h>  //sse2
#include <pmmintrin.h>  //sse3


//#include <mmintrin.h> //MMX
//#include <xmmintrin.h> //SSE(include mmintrin.h)
//#include <emmintrin.h> //SSE2(include xmmintrin.h)
//#include <pmmintrin.h> //SSE3(include emmintrin.h)
//#include <tmmintrin.h>//SSSE3(include pmmintrin.h)
//#include <smmintrin.h>//SSE4.1(include tmmintrin.h)
//#include <nmmintrin.h>//SSE4.2(include smmintrin.h)
//#include <wmmintrin.h>//AES(include nmmintrin.h)
//#include <immintrin.h>//AVX(include wmmintrin.h)
//#include <intrin.h>//(include immintrin.h)

#if defined(__MACH__) && !TARGET_OS_IPHONE
#include <x86intrin.h>
#include <xmmintrin.h>
#define __mac_x86_sse__
#endif

#ifdef _WIN32
#include <intrin.h>
#endif

#if defined(__mac_x86_sse__) || defined(_WIN32)
#define __x86_sse_enabled
#endif

//total
//#include <intrin.h>
#define _align_xmm _declspec(align(16))
// -march=armv7-a -mfloat-abi=hard -mfpu=neon -ftree-vectorize
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include <arm_acle.h>
#include <arm_neon.h>
#endif
NS_GT_BEGIN
/*
  *针对任一给定方向向量,求出极值点
 */
const cocos2d::Vec2&  gjk_compute_support_point(const std::vector<cocos2d::Vec2> &polygon,const cocos2d::Vec2 &direction) {
	int target_j = 0;
	float f3 = -FLT_MAX;
	//当前使用顺序遍历,稍后将使用二分法遍历
	for (int j = 0; j < polygon.size(); ++j) {
		float f4 = dot(polygon[j], direction);
		if (f4 > f3) {
			f3 = f4;
			target_j = j;
		}
	}
	return polygon[target_j];
}

bool gjk_minimum_simplex(cocos2d::Vec2 simplex_array[4],int &simplex_count,cocos2d::Vec2 &direction) {
	if (simplex_count == 1) {
		direction = -simplex_array[0];
		return false;
	}
	//当顶点数目为2的时候,此时的计算规则较为复杂
	if (simplex_count == 2) {
		float d_x = -simplex_array[1].y + simplex_array[0].y;
		float d_y = simplex_array[1].x - simplex_array[0].x;
		//判断原点与当前线段之间的位置关系,原则上来说有三种关系,但是已经假设输入的为凸多边形,因此额外的两种可以不用判断
		direction.x = d_x; 
		direction.y = d_y;
		if (simplex_array[0].x * d_x + simplex_array[0].y * d_y > 0.0f) {//右侧
			direction.x = -d_x;
			direction.y = -d_y;
		}
		return false;
	}
	assert(simplex_count == 3);
	//顶点数目为3的时候,需要判断原点是否位于单纯形之内
	float f1 = cross(simplex_array[0],simplex_array[1]);
	float f2 = cross(simplex_array[1],simplex_array[2]);
	float f3 = cross(simplex_array[2],simplex_array[0]);
	if (f1 * f2 >= 0.0f && f2 * f3 >= 0.0f)
		return true;
	//否则需要求出当前单纯形相对原点的最小范数单纯形
	float f4 = cross(simplex_array[0],simplex_array[1],simplex_array[2]);
	if (f4 < 0.0f) {
		cocos2d::Vec2 t2 = simplex_array[1];
		simplex_array[1] = simplex_array[2];
		simplex_array[2] = t2;
	}
	cocos2d::Vec2 &a = simplex_array[0], &b = simplex_array[1], &c = simplex_array[2];
	//A
	cocos2d::Vec2 ab = b - a;
	cocos2d::Vec2 ac = c - a;
	float fab = -dot(a,ab);
	float fac = -dot(a,ac);
	if (fab < 0.0f && fac < 0.0f) {
		simplex_count = 1;
		direction = -a;
		return false;
	}
	//B
	cocos2d::Vec2 bc = c - b;
	float fbc = -dot(b,bc);
	float fba = dot(b,ab);
	if (fba < 0.0f && fbc < 0.0f) {
		simplex_count = 1;
		simplex_array[0] = b;
		direction = -b;
		return false;
	}
	//C
	float fcb = dot(c,bc);
	float fca = dot(c,ac);
	if (fca < 0.0f && fcb < 0.0f) {
		simplex_count = 1;
		simplex_array[0] = c;
		direction = -c;
		return false;
	}
	//AC
	if (fca > 0.0f && fac > 0.0f && cross(ac,a) < 0.0f) {
		simplex_count = 2;
		simplex_array[1] = c;
		direction.x = -ac.y;
		direction.y = ac.x;
		return false;
	}
	//AB
	if (fab > 0.0f && fba > 0.0f && cross(ab,a) > 0.0f) {
		simplex_count = 2;
		direction.x = ab.y;
		direction.y = -ab.x;
		return false;
	}
	//BC
	if (fcb > 0.0f && fbc > 0.0f && cross(bc,b) > 0.0f) {
		simplex_array[0] = simplex_array[1];
		simplex_array[1] = simplex_array[2];
		simplex_count = 2;
		direction.x = bc.y;
		direction.y = -bc.x;
		return false;
	}
	assert(false);
	return false;
}

bool gjk_algorithm_optimal(const std::vector<cocos2d::Vec2> &polygon1, const std::vector<cocos2d::Vec2> &polygon2, cocos2d::Vec2 near_points[2]) {
	cocos2d::Vec2 simplex[4],direction(randomf10(),randomf10());
	int simplex_count = 0;

	const cocos2d::Vec2 *polygon1_ptr = polygon1.data();
	const cocos2d::Vec2 *polygon2_ptr = polygon2.data();

	simplex[0] = gjk_compute_support_point(polygon1, direction) - gjk_compute_support_point(polygon2, -direction);
	simplex_count = 1;
	direction = -simplex[0];

	int loop_count = polygon1.size() + polygon2.size();
	int j = 0;
	for (j = 0; j < loop_count; ++j) {
		//检测目标方向
		cocos2d::Vec2 s0 = gjk_compute_support_point(polygon1, direction) - gjk_compute_support_point(polygon2, -direction);
		if (dot(s0, direction) < 0.0f)
			return false;
		//将顶点s0加入到单纯形集合中,并计算其最小范数
		simplex[simplex_count++] = s0;
		//求原点与以上单纯形集合之间的关系,以及下一轮法线的方向,最小单纯形集合
		if (gjk_minimum_simplex(simplex, simplex_count, direction))
			return true;
	}
	CCLOG("loops time-->%d",j);
	return true;
}

void  cw_compute_support_point(const std::vector<cocos2d::Vec2> &polygon, const cocos2d::Vec2 &direction,cocos2d::Vec2 &max_point,cocos2d::Vec2 &min_point) {
	float f3 = -FLT_MAX;
	float f5 = FLT_MAX;
	//当前使用顺序遍历,稍后将使用二分法遍历
	for (int j = 0; j < polygon.size(); ++j) {
		float f4 = dot(polygon[j], direction);
		if (f4 > f3) {
			f3 = f4;
			max_point = polygon.at(j);
		}

		if (f4 < f5) {
			f5 = f4;
			min_point = polygon.at(j);
		}
	}
}

bool chung_wang_seperate_algorithm(const std::vector<cocos2d::Vec2> &polygon1, const std::vector<cocos2d::Vec2> &polygon2, cocos2d::Vec2 &seperate_vec2) {
	//算法起始于任一随机向量
	cocos2d::Vec2  direction = cocos2d::Vec2(1.0f, 0.0f);// randomf10(), randomf10());
	int j = 0;
	for (; j < polygon1.size() + polygon2.size(); ++j) {
		cocos2d::Vec2 p1, p2, q1, q2;
		 cw_compute_support_point(polygon1, direction,p1,p2);
		cw_compute_support_point(polygon2, -direction,q1,q2);
		//此时direction为分离轴
		if(dot(p2,direction) > dot(q2,direction) || dot(p1,direction) < dot(q1,direction))
			return false;
		//否则,需要重新计算
		cocos2d::Vec2 normal = normalize(p1,q2);
		const cocos2d::Vec2 ortho(-normal.y, normal.x);

		float f2 = 2.0f * dot(direction,ortho);
		direction -= f2 * ortho;
	}
	return true;
}

bool triangle_contains_point(const cocos2d::Vec2 &a,const cocos2d::Vec2 &b,const cocos2d::Vec2 &c,const cocos2d::Vec2 &check_point) {
	float f1 = cross(check_point,a,b);
	float f2 = cross(check_point,b,c);
	float f3 = cross(check_point,c,a);

	return f1 * f2 >= 0.0f && f2 * f3 >= 0.0f;
}

void simple_polygon_ear_triangulate(const std::vector<cocos2d::Vec2> &polygon, std::vector<short> &triangle_list) {
	//三角形的数目为polygon.size() - 2
	int  vertex_count = polygon.size();
	std::vector<int>  sequence_array(vertex_count * 2);
	int  *next_array = sequence_array.data();
	int  *prev_array = sequence_array.data() + vertex_count;

	for (int j = 0; j < vertex_count; ++j) {
		next_array[j] = j+1;
		prev_array[j] = j - 1;
	}
	next_array[vertex_count - 1] = 0;
	prev_array[0] = vertex_count - 1;

	int target_j = 0;
	triangle_list.reserve((vertex_count - 2) * 3);
	const cocos2d::Vec2 *polygon_ptr = polygon.data();
	while (vertex_count > 3) {
		bool is_ear_vertex = false;
		int prev_j = prev_array[target_j];
		int next_j = next_array[target_j];
		if (cross(polygon_ptr[target_j], polygon_ptr[next_j], polygon_ptr[prev_j]) > 0.0f) {
			is_ear_vertex = true;
			int compare_j = next_array[next_j];
			while (compare_j != prev_j) {
				if (triangle_contains_point(polygon_ptr[target_j], polygon_ptr[next_j], polygon_ptr[prev_j], polygon_ptr[compare_j])) {
					is_ear_vertex = false;
					break;
				}
				compare_j = next_array[compare_j];
			}
		}
		if (is_ear_vertex) {
			vertex_count -= 1;
			triangle_list.push_back(prev_j);
			triangle_list.push_back(target_j);
			triangle_list.push_back(next_j);

			target_j = prev_j;
			next_array[prev_j] = next_j;
			prev_array[next_j] = prev_j;
		}
		else 
			target_j = next_array[target_j];
	}
	//最后一个三角形
	triangle_list.push_back(prev_array[target_j]);
	triangle_list.push_back(target_j);
	triangle_list.push_back(next_array[target_j]);
}

void sse_code_sample() {
	_align_xmm float matrix_array[16];

	float *matrix_ptr = (float*)_aligned_malloc(sizeof(float) * 16, 16);


	_aligned_free(matrix_ptr);
	//_mm_malloc(16, 16);
	//_MM_ALIGN16
}

void matrix_multiply(const float m1[16], const float m2[16], float dst[16]) {
	float product[16];
	float *result_ptr = dst != m1 && dst != m2?dst:product;

	result_ptr[0] = m1[0] * m2[0] + m1[4] * m2[1] + m1[8] * m2[2] + m1[12] * m2[3];
	result_ptr[1] = m1[1] * m2[0] + m1[5] * m2[1] + m1[9] * m2[2] + m1[13] * m2[3];
	result_ptr[2] = m1[2] * m2[0] + m1[6] * m2[1] + m1[10] * m2[2] + m1[14] * m2[3];
	result_ptr[3] = m1[3] * m2[0] + m1[7] * m2[1] + m1[11] * m2[2] + m1[15] * m2[3];

	result_ptr[4] = m1[0] * m2[4] + m1[4] * m2[5] + m1[8] * m2[6] + m1[12] * m2[7];
	result_ptr[5] = m1[1] * m2[4] + m1[5] * m2[5] + m1[9] * m2[6] + m1[13] * m2[7];
	result_ptr[6] = m1[2] * m2[4] + m1[6] * m2[5] + m1[10] * m2[6] + m1[14] * m2[7];
	result_ptr[7] = m1[3] * m2[4] + m1[7] * m2[5] + m1[11] * m2[6] + m1[15] * m2[7];

	result_ptr[8] = m1[0] * m2[8] + m1[4] * m2[9] + m1[8] * m2[10] + m1[12] * m2[11];
	result_ptr[9] = m1[1] * m2[8] + m1[5] * m2[9] + m1[9] * m2[10] + m1[13] * m2[11];
	result_ptr[10] = m1[2] * m2[8] + m1[6] * m2[9] + m1[10] * m2[10] + m1[14] * m2[11];
	result_ptr[11] = m1[3] * m2[8] + m1[7] * m2[9] + m1[11] * m2[10] + m1[15] * m2[11];

	result_ptr[12] = m1[0] * m2[12] + m1[4] * m2[13] + m1[8] * m2[14] + m1[12] * m2[15];
	result_ptr[13] = m1[1] * m2[12] + m1[5] * m2[13] + m1[9] * m2[14] + m1[13] * m2[15];
	result_ptr[14] = m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] + m1[14] * m2[15];
	result_ptr[15] = m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] + m1[15] * m2[15];

	if(result_ptr != dst)
	memcpy(dst, product, 16 * sizeof(float));
}
/*
  *sse矩阵乘法,假设输入变量已经16字节对齐
 */
void sse_matrix_multiply(const float m2[16], const float m1[16], float *dst) {
	_MM_ALIGN16  __m128 r0 = _mm_set_ps(m1[12], m1[8], m1[4], m1[0]);
	_MM_ALIGN16  __m128 r1 = _mm_set_ps(m1[13], m1[9], m1[5], m1[1]);
	_MM_ALIGN16  __m128 r2 = _mm_set_ps(m1[14], m1[10], m1[6], m1[2]);
	_MM_ALIGN16  __m128 r3 = _mm_set_ps(m1[15], m1[11], m1[7], m1[3]);
	//row
	_MM_ALIGN16 __m128 r7 = _mm_load_ps(m2);
	//row 0
	_MM_ALIGN16 __m128 rk = _mm_dp_ps(r7, r0, 0xF1);
	_mm_store_ss(dst, rk);//0
						  //1
	rk = _mm_dp_ps(r7, r1, 0xF1);
	_mm_store_ss(dst + 1, rk);
	//2
	rk = _mm_dp_ps(r7, r2, 0xF1);
	_mm_store_ss(dst + 2, rk);
	//3
	rk = _mm_dp_ps(r7, r3, 0xF1);
	_mm_store_ss(dst + 3, rk);
	//row 1
	r7 = _mm_load_ps(m2 + 4);
	//4
	rk = _mm_dp_ps(r7, r0, 0xF1);
	_mm_store_ss(dst + 4, rk);
	//5
	rk = _mm_dp_ps(r7, r1, 0xF1);
	_mm_store_ss(dst + 5, rk);
	//6
	rk = _mm_dp_ps(r7, r2, 0xF1);
	_mm_store_ss(dst + 6, rk);
	//7
	rk = _mm_dp_ps(r7, r3, 0xF1);
	_mm_store_ss(dst + 7, rk);
	//row 2
	r7 = _mm_load_ps(m2 + 8);
	//8
	rk = _mm_dp_ps(r7, r0, 0xF1);
	_mm_store_ss(dst + 8, rk);
	//9
	rk = _mm_dp_ps(r7, r1, 0xF1);
	_mm_store_ss(dst + 9, rk);
	//10
	rk = _mm_dp_ps(r7, r2, 0xF1);
	_mm_store_ss(dst + 10, rk);
	//11
	rk = _mm_dp_ps(r7, r3, 0xF1);
	_mm_store_ss(dst + 11, rk);
	//row 3
	r7 = _mm_load_ps(m2 + 12);
	//12
	rk = _mm_dp_ps(r7, r0, 0xF1);
	_mm_store_ss(dst + 12, rk);
	//13
	rk = _mm_dp_ps(r7, r1, 0xF1);
	_mm_store_ss(dst + 13, rk);
	//14
	rk = _mm_dp_ps(r7, r2, 0xF1);
	_mm_store_ss(dst + 14, rk);
	//15
	rk = _mm_dp_ps(r7, r3, 0xF1);
	_mm_store_ss(dst + 15, rk);
}

void sse_matrix_multiply_optimal(const float m1[16], const float m2[16], float dst[16]) {
	_MM_ALIGN16 __m128 r0 = _mm_loadu_ps(m2);
	_MM_ALIGN16 __m128 r1 = _mm_loadu_ps(m2+4);
	_MM_ALIGN16 __m128 r2 = _mm_loadu_ps(m2+8);
	_MM_ALIGN16 __m128 r3 = _mm_loadu_ps(m2 + 12);

	_MM_ALIGN16 __m128 s0 = _mm_loadu_ps(m1);
	_MM_ALIGN16 __m128 s1 = _mm_loadu_ps(m1 + 4);
	_MM_ALIGN16 __m128 s2 = _mm_loadu_ps(m1 + 8);
	_MM_ALIGN16 __m128 s3 = _mm_loadu_ps(m1 + 12);

	//_mm_mul_

}

void matrix_substruct(const float m[16], float *dst) {
	_MM_ALIGN16 __m128 r0 = _mm_setzero_ps();
	//row 0
	_MM_ALIGN16 __m128 r1 = _mm_loadu_ps(m);
	_MM_ALIGN16 __m128 rk = _mm_sub_ps(r0, r1);
	_mm_storeu_ps(dst, rk);
	//row 1
	r1 = _mm_loadu_ps(m + 4);
	rk = _mm_sub_ps(r0, r1);
	_mm_storeu_ps(dst + 4, rk);
	//row 2
	r1 = _mm_loadu_ps(m + 8);
	rk = _mm_sub_ps(r0, r1);
	_mm_storeu_ps(dst + 8, rk);
	//row 3
	r1 = _mm_loadu_ps(m + 12);
	rk = _mm_sub_ps(r0, r1);
	_mm_storeu_ps(dst + 12, rk);
}

void matrix_add(const float m[16], float scalar,float *dst) {
	_MM_ALIGN16 __m128 r0 = _mm_set_ps1(scalar);
	_MM_ALIGN16 __m128 r1 = _mm_loadu_ps(m);
	_MM_ALIGN16 __m128 rs = _mm_add_ps(r0, r1);
	_mm_storeu_ps(dst, rs);
	//row 1
	r1 = _mm_loadu_ps(m + 4);
	rs = _mm_add_ps(r0, r1);
	_mm_store_ps(dst + 4, rs);
	//row 2
	r1 = _mm_loadu_ps(m + 8);
	rs = _mm_add_ps(r0, r1);
	_mm_store_ps(dst + 8, rs);
	//row 3
	r1 = _mm_loadu_ps(m + 12);
	rs = _mm_add_ps(r0, r1);
	_mm_store_ps(dst + 12, rs);
}
//SSE指令基本信息介绍
//https://zhuanlan.zhihu.com/p/55327037
//https://blog.csdn.net/jgj123321/article/details/95633431?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromBaidu-1.control&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromBaidu-1.control
//intel 有关_mm_dp_ps向量内积指令的详细说明
//https://zhou-yuxin.github.io/articles/2017/%E4%BD%BF%E7%94%A8Intel%20SSE-AVX%E6%8C%87%E4%BB%A4%E9%9B%86%EF%BC%88SIMD%EF%BC%89%E5%8A%A0%E9%80%9F%E5%90%91%E9%87%8F%E5%86%85%E7%A7%AF%E8%AE%A1%E7%AE%97/index.html
//有关新版SSE指令的介绍
//chrome-extension://ohfgljdgelakfkefopgklcohadegdpjf/https://sbel.wiscweb.wisc.edu/wp-content/uploads/sites/569/2018/10/lecture1106-1.pdf
//SSE4.1+SSE4.2新版指令的说明
//https://blog.csdn.net/fengbingchun/article/details/22101981
//SSE汇编指令基本使用方法介绍
//https://blog.csdn.net/tercel_zhang/article/details/80049244
//arm neon 汇编指令
//https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/intrinsics?page=1
//arm neon汇编指令教程intrinsics
//https://zhuanlan.zhihu.com/p/61356656
NS_GT_END