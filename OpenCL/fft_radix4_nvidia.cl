
__kernel void fft_radix4(__global const float2 * x,__global float2 * y,int p)

{

  int t = get_global_size(0); // number of threads

  int i = get_global_id(0); // current thread

  int k = i & (p-1); // index in input sequence, in 0..P-1

  // Inputs indices are I+{0,1,2,3}*T

  x += i;

  // Output indices are J+{0,1,2,3}*P, where

  // J is I with two 0 bits inserted at bit log2(P)

  y += ((i-k)<<2) + k;



  // Load and twiddle inputs

  // Twiddling factors are exp(_I*PI*{0,1,2,3}*K/2P)

  float alpha = -FFT_PI*(float)k/(float)(2*p);



  // Load and twiddle, one exp_alpha computed instead of 3

  float2 twiddle = exp_alpha_1(alpha);

  float2 u0 = x[0];

  float2 u1 = mul_1(twiddle,x[t]);

  float2 u2 = x[2*t];

  float2 u3 = mul_1(twiddle,x[3*t]);

  twiddle = sqr_1(twiddle);

  u2 = mul_1(twiddle,u2);

  u3 = mul_1(twiddle,u3);



  // 2x DFT2 and twiddle

  float2 v0 = u0 + u2;

  float2 v1 = u0 - u2;

  float2 v2 = u1 + u3;

  float2 v3 = mul_p1q2(u1 - u3); // twiddle



  // 2x DFT2 and store

  y[0] = v0 + v2;

  y[p] = v1 + v3;

  y[2*p] = v0 - v2;

  //v3.y = -v3.y; <-- doesn't change

  y[3*p] = v1 - v3; <-- error in sign