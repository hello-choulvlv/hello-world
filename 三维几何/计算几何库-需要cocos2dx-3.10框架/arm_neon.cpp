//
//  arm_neon.cpp
//  arm_neon
//
//  Created by charlie on 2021/1/16.
//

#include "arm_neon_ios.h"
#include <arm_acle.h>
#include <arm_neon.h>
#define fx_dot4_f32(f0,f1) vaddvq_f32(vmulq_f32(f0,f1))
#define fx_set4_f32(v0,f0,f1,f2,f3) vsetq_lane_f32(f3,vsetq_lane_f32(f2,vsetq_lane_f32(f1,vsetq_lane_f32(f0,v0,0),1),2),3)

#define __asm_inline 0
#define __arm_neon_optimal 1
#define __arm_neon_normal 2


#define __code_cpp  __asm_inline

#if __code_cpp == __asm_inline
void neon_matrix_multiply(const float *m1,const float *m2,float *dst){
    asm volatile(
                 "ld1 {v0.4s,v1.4s,v2.4s,v3.4s},[%1] \n\t"
                 "ld1 {v4.4s,v5.4s,v6.4s,v7.4s},[%2] \n\t"
                 
                 "fmul v8.4s, v4.4s,v0.s[0]  \n\t"
                 "fmul v9.4s, v4.4s,v1.s[0]  \n\t"
                 "fmul v10.4s,v4.4s,v2.s[0]  \n\t"
                 "fmul v11.4s,v4.4s,v3.s[0]  \n\t"
                 
                 "fmla v8.4s, v5.4s,v0.s[1]  \n\t"
                 "fmla v9.4s, v5.4s,v1.s[1]  \n\t"
                 "fmla v10.4s,v5.4s,v2.s[1]  \n\t"
                 "fmla v11.4s,v5.4s,v3.s[1]  \n\t"
                 
                 "fmla v8.4s, v6.4s,v0.s[2]  \n\t"
                 "fmla v9.4s, v6.4s,v1.s[2]  \n\t"
                 "fmla v10.4s,v6.4s,v2.s[2]  \n\t"
                 "fmla v11.4s,v6.4s,v3.s[2]  \n\t"
                 
                 "fmla v8.4s, v7.4s,v0.s[3]  \n\t"
                 "fmla v9.4s, v7.4s,v1.s[3]  \n\t"
                 "fmla v10.4s,v7.4s,v2.s[3]  \n\t"
                 "fmla v11.4s,v7.4s,v3.s[3]  \n\t"
                 
                 "st1 {v8.4s,v9.4s,v10.4s,v11.4s},[%0]"
                 :
                 :"r"(dst),"r"(m1),"r"(m2)
                 :"memory","v0","v1","v2","v3","v4","v5","v6","v7","v8","v9","v10","v11"
                 
    );
}
#elif __code_cpp == __arm_neon_optimal
void neon_matrix_multiply(const float *m1,const float *m2,float *dst){
    float32x4x4_t mat_m1 = vld4q_f32(m1);
    float32x4x4_t mat_m2 = vld4q_f32(m2);
    
    float32x4x4_t mat_r;
    mat_r.val[0] = vmulq_n_f32(mat_m2.val[0],vgetq_lane_f32(mat_m1.val[0],0));
    mat_r.val[1] = vmulq_n_f32(mat_m2.val[0],vgetq_lane_f32(mat_m1.val[1],0));
    mat_r.val[2] = vmulq_n_f32(mat_m2.val[0],vgetq_lane_f32(mat_m1.val[2],0));
    mat_r.val[3] = vmulq_n_f32(mat_m2.val[0],vgetq_lane_f32(mat_m1.val[3],0));
    
    //vld4q_f32();
    
#if 0
    mat_r.val[0] = vfmaq_n_f32(mat_r.val[0],mat_m2.val[1],vgetq_lane_f32(mat_m1.val[0],1));
    mat_r.val[1] = vfmaq_n_f32(mat_r.val[1],mat_m2.val[1],vgetq_lane_f32(mat_m1.val[1],1));
    mat_r.val[2] = vfmaq_n_f32(mat_r.val[2],mat_m2.val[1],vgetq_lane_f32(mat_m1.val[2],1));
    mat_r.val[3] = vfmaq_n_f32(mat_r.val[3],mat_m2.val[1],vgetq_lane_f32(mat_m1.val[3],1));
    
    mat_r.val[0] = vfmaq_n_f32(mat_r.val[0],mat_m2.val[2],vgetq_lane_f32(mat_m1.val[0],2));
    mat_r.val[1] = vfmaq_n_f32(mat_r.val[1],mat_m2.val[2],vgetq_lane_f32(mat_m1.val[1],2));
    mat_r.val[2] = vfmaq_n_f32(mat_r.val[2],mat_m2.val[2],vgetq_lane_f32(mat_m1.val[2],2));
    mat_r.val[3] = vfmaq_n_f32(mat_r.val[3],mat_m2.val[2],vgetq_lane_f32(mat_m1.val[3],2));
    
    mat_r.val[0] = vfmaq_n_f32(mat_r.val[0],mat_m2.val[3],vgetq_lane_f32(mat_m1.val[0],3));
    mat_r.val[1] = vfmaq_n_f32(mat_r.val[1],mat_m2.val[3],vgetq_lane_f32(mat_m1.val[1],3));
    mat_r.val[2] = vfmaq_n_f32(mat_r.val[2],mat_m2.val[3],vgetq_lane_f32(mat_m1.val[2],3));
    mat_r.val[3] = vfmaq_n_f32(mat_r.val[3],mat_m2.val[3],vgetq_lane_f32(mat_m1.val[3],3));
#else
    mat_r.val[0] += vmulq_n_f32(mat_m2.val[1],vgetq_lane_f32(mat_m1.val[0],1));
    mat_r.val[1] += vmulq_n_f32(mat_m2.val[1],vgetq_lane_f32(mat_m1.val[1],1));
    mat_r.val[2] += vmulq_n_f32(mat_m2.val[1],vgetq_lane_f32(mat_m1.val[2],1));
    mat_r.val[3] += vmulq_n_f32(mat_m2.val[1],vgetq_lane_f32(mat_m1.val[3],1));
    
    mat_r.val[0] += vmulq_n_f32(mat_m2.val[2],vgetq_lane_f32(mat_m1.val[0],2));
    mat_r.val[1] += vmulq_n_f32(mat_m2.val[2],vgetq_lane_f32(mat_m1.val[1],2));
    mat_r.val[2] += vmulq_n_f32(mat_m2.val[2],vgetq_lane_f32(mat_m1.val[2],2));
    mat_r.val[3] += vmulq_n_f32(mat_m2.val[2],vgetq_lane_f32(mat_m1.val[3],2));
    
    mat_r.val[0] += vmulq_n_f32(mat_m2.val[3],vgetq_lane_f32(mat_m1.val[0],3));
    mat_r.val[1] += vmulq_n_f32(mat_m2.val[3],vgetq_lane_f32(mat_m1.val[1],3));
    mat_r.val[2] += vmulq_n_f32(mat_m2.val[3],vgetq_lane_f32(mat_m1.val[2],3));
    mat_r.val[3] += vmulq_n_f32(mat_m2.val[3],vgetq_lane_f32(mat_m1.val[3],3));
#endif
    
    vst4q_f32(dst,mat_r);
    //vmulq_n_f32()
}
#elif __code_cpp == __arm_neon_normal

void neon_matrix_multiply(const float *m1,const float *m2,float *dst){
    __attribute__((aligned(16))) float row_data[4];
    
    float32x4x4_t mat_m1 = vld4q_f32(m1);
    float32x4x4_t mat_m2 = vld4q_f32(m2);
    
    //vget_low_f32(mat_m2.val[0]);
    //vgetq_lane_f32(mat_m2.val[0],0);
    
    //vtrn2q_f32(mat4x4_m2[0], mat4x4_m2[1]);
    //row_data[0] = m2[0];
    //row_data[1] = m2[4];
    //row_data[2] = m2[8];
    //row_data[3] = m2[12];
    float32x4_t r0 = mat_m2.val[0];//vld1q_f32(row_data);
    //float32x4_t r0 = vmovq_n_f32(0);//;
    //float32x4_t r0;
    //r0 = fx_set4_f32(r0,m2[0],m2[4],m2[8],m2[12]);
    
    //vld1q_dup_f32();
    
    //row_data[0] = m2[1];
    //row_data[1] = m2[5];
    //row_data[2] = m2[9];
    //row_data[3] = m2[13];
    float32x4_t r1 = mat_m2.val[1];//vld1q_f32(row_data);
    //float32x4_t r1;
    //r1 = fx_set4_f32(r1,m2[1], m2[5], m2[9], m2[13]);
    
    //row_data[0] = m2[2];
    //row_data[1] = m2[6];
    //row_data[2] = m2[10];
    //row_data[3] = m2[14];
    float32x4_t r2 = mat_m2.val[2];//vld1q_f32(row_data);
    //float32x4_t r2;
    //r2 = fx_set4_f32(r2, m2[2], m2[6], m2[10], m2[14]);
    
    //row_data[0] = m2[3];
    //row_data[1] = m2[7];
    //row_data[2] = m2[11];
    //row_data[3] = m2[15];
    float32x4_t r3 = mat_m2.val[3];//vld1q_f32(row_data);
    //float32x4_t r3;
    //r3 = fx_set4_f32(r3,m2[3], m2[6], m2[10], m2[14]);
    
    //------0-------
    //float32x4_t rs = mat_m1.val[0];//vld1q_f32(m1);
    dst[0] = vaddvq_f32(vmulq_f32(mat_m1.val[0], r0));
    //1
    dst[1] = vaddvq_f32(vmulq_f32(mat_m1.val[0], r1));
    //2
    dst[2] = vaddvq_f32(vmulq_f32(mat_m1.val[0], r2));
    //3
    dst[3] = vaddvq_f32(vmulq_f32(mat_m1.val[0], r3));
    //-----4-----
    //rs = mat_m1.val[1];//vld1q_f32(m1 + 4);
    dst[4] = vaddvq_f32(vmulq_f32(mat_m1.val[1], r0));
    //5
    dst[5] = vaddvq_f32(vmulq_f32(mat_m1.val[1], r1));
    //6
    dst[6] = vaddvq_f32(vmulq_f32(mat_m1.val[1], r2));
    //7
    dst[7] = vaddvq_f32(vmulq_f32(mat_m1.val[1], r3));
    //------8-------
    //rs = mat_m1.val[2];//vld1q_f32(m1 + 8);
    dst[8] = vaddvq_f32(vmulq_f32(mat_m1.val[2], r0));
    //9
    dst[9] = vaddvq_f32(vmulq_f32(mat_m1.val[2], r1));
    //10
    dst[10] = vaddvq_f32(vmulq_f32(mat_m1.val[2], r2));
    //11
    dst[11] = vaddvq_f32(vmulq_f32(mat_m1.val[2], r3));
    //----------12----------
    //rs = mat_m1.val[3];//vld1q_f32(m1 + 12);
    dst[12] = vaddvq_f32(vmulq_f32(mat_m1.val[3], r0));
    //13
    dst[13] = vaddvq_f32(vmulq_f32(mat_m1.val[3], r1));
    //14
    dst[14] = vaddvq_f32(vmulq_f32(mat_m1.val[3], r2));
    //15
    dst[15] = vaddvq_f32(vmulq_f32(mat_m1.val[3], r3));
    //从向量寄存器存储单个的浮点数
    //vst1q_lane_f32(dst,rs,0);
    //vmulq_n_f32(f32x4_t,float):向量与标量之间c乘法
}
#endif
void matrix_multiply(const float *m2,const float *m1,float *dst){
    float product[16];
    
    product[0]  = m1[0] * m2[0]  + m1[4] * m2[1] + m1[8]   * m2[2]  + m1[12] * m2[3];
    product[1]  = m1[1] * m2[0]  + m1[5] * m2[1] + m1[9]   * m2[2]  + m1[13] * m2[3];
    product[2]  = m1[2] * m2[0]  + m1[6] * m2[1] + m1[10]  * m2[2]  + m1[14] * m2[3];
    product[3]  = m1[3] * m2[0]  + m1[7] * m2[1] + m1[11]  * m2[2]  + m1[15] * m2[3];
    
    product[4]  = m1[0] * m2[4]  + m1[4] * m2[5] + m1[8]   * m2[6]  + m1[12] * m2[7];
    product[5]  = m1[1] * m2[4]  + m1[5] * m2[5] + m1[9]   * m2[6]  + m1[13] * m2[7];
    product[6]  = m1[2] * m2[4]  + m1[6] * m2[5] + m1[10]  * m2[6]  + m1[14] * m2[7];
    product[7]  = m1[3] * m2[4]  + m1[7] * m2[5] + m1[11]  * m2[6]  + m1[15] * m2[7];
    
    product[8]  = m1[0] * m2[8]  + m1[4] * m2[9] + m1[8]   * m2[10] + m1[12] * m2[11];
    product[9]  = m1[1] * m2[8]  + m1[5] * m2[9] + m1[9]   * m2[10] + m1[13] * m2[11];
    product[10] = m1[2] * m2[8]  + m1[6] * m2[9] + m1[10]  * m2[10] + m1[14] * m2[11];
    product[11] = m1[3] * m2[8]  + m1[7] * m2[9] + m1[11]  * m2[10] + m1[15] * m2[11];
    
    product[12] = m1[0] * m2[12] + m1[4] * m2[13] + m1[8]  * m2[14] + m1[12] * m2[15];
    product[13] = m1[1] * m2[12] + m1[5] * m2[13] + m1[9]  * m2[14] + m1[13] * m2[15];
    product[14] = m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] + m1[14] * m2[15];
    product[15] = m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] + m1[15] * m2[15];
    
    memcpy(dst, product, MATRIX_SIZE);
}
