/*
 *当前C++函数只能在iOS/android下运行
 *2021/1/16
 *@author:xiaoxiong
 */

#ifndef __arm_neon_ios_h__
#define __arm_neon_ios_h__
/*
 *矩阵乘法neon intrinsic指令实现
 */
void neon_matrix_multiply(const float *m1,const float *m2,float *dst);
/*
 *一般矩阵乘法
 */
void matrix_multiply(const float *m1,const float *m2,float *dst);

#endif /* __arm_neon_ios_h__ */
