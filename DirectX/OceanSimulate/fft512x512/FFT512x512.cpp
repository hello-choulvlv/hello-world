//--------------------------------------------------------------------------------------
// File: fft_512x512_c2c.cpp
//
// Run FFT by using AMP
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "FFT512x512.h"
#include <amp_math.h>
using namespace concurrency;
using namespace concurrency::graphics;
//蝴蝶操作
//a = c + w
//b = c - w
void FFT512x512::ft2(float_2 &a, float_2 &b) restrict(amp)
{
	float t;

	t = a.x;
	a.x += b.x;
	b.x = t - b.x;

	t = a.y;
	a.y += b.y;
	b.y = t - b.y;
}
//复数之间的乘法
void FFT512x512::cmul_forward(float_2 &a, float bx, float by) restrict(amp)
{
	float t = a.x;
	a.x = t * bx - a.y * by;
	a.y = t * by + a.y * bx;
}
//一种无法理解的计算方法
void FFT512x512::upd_forward(float_2 &a, float_2 &b) restrict(amp)
{
	float A = a.x;
	float B = b.y;

	a.x += b.y;
	b.y = a.y + b.x;
	a.y -= b.x;
	b.x = A - B;
}

void FFT512x512::fft_forward_4(float_2 complex_num[8]) restrict(amp)
{
	ft2(complex_num[0], complex_num[2]);
	ft2(complex_num[1], complex_num[3]);
	ft2(complex_num[0], complex_num[1]);

	upd_forward(complex_num[2], complex_num[3]);
}

void FFT512x512::fft_forward_8(float_2 complex_num[8]) restrict(amp)
{
	ft2(complex_num[0], complex_num[4]);
	ft2(complex_num[1], complex_num[5]);
	ft2(complex_num[2], complex_num[6]);
	ft2(complex_num[3], complex_num[7]);

	upd_forward(complex_num[4], complex_num[6]);
	upd_forward(complex_num[5], complex_num[7]);

	cmul_forward(complex_num[5], TWIDDLE_1_8);
	cmul_forward(complex_num[7], TWIDDLE_3_8);

	fft_forward_4(complex_num);
	ft2(complex_num[4], complex_num[5]);
	ft2(complex_num[6], complex_num[7]);
}
/*
*复数与单位复数的乘法
*/
void FFT512x512::twiddle(float_2 &d, float phase) restrict(amp)
{
	float tx, ty;
	fast_math::sincos(phase, &ty, &tx);

	float t = d.x;
	d.x = t * tx - d.y * ty;
	d.y = t * ty + d.y * tx;
}

void FFT512x512::twiddle_8(float_2 complex_num[8], float phase) restrict(amp)
{
	twiddle(complex_num[4], 1 * phase);
	twiddle(complex_num[2], 2 * phase);
	twiddle(complex_num[6], 3 * phase);
	twiddle(complex_num[1], 4 * phase);
	twiddle(complex_num[5], 5 * phase);
	twiddle(complex_num[3], 6 * phase);
	twiddle(complex_num[7], 7 * phase);
}

void FFT512x512::radix008A_amp(array_view<float_2> dst,
	array_view<const float_2> src,
	UINT thread_count,
	UINT istride,
	change_percall change_percall)
{
	extent<1> compute_domain(thread_count);
	//缓冲区对象的总长度为3 x 512 x 512
	if (istride > 1)
	{
		parallel_for_each(m_av, compute_domain, [=](index<1> idx) restrict(amp)
		{
			const int idx_index = idx[0];
			// if (idx_index >= change_percall.thread_count)
			//     return;

			// Fetch 8 complex numbers
			float_2 complex_num[8];
			//将输入的数据按边界对齐,并且每一个数据块是8个复数,参与运算的数据块之间间隔是固定的
			//每数据块内固定的偏移
			unsigned int imod = idx_index & (change_percall.istride - 1);
			unsigned int iaddr = ((idx_index - imod) << 3) + imod;
			for (int i = 0; i < 8; i++)
				complex_num[i] = src[index<1>(iaddr + i * change_percall.istride)];

			// Math
			fft_forward_8(complex_num);
			//可以理解为在输入数据的块大小的范围内,按照pstride字节对齐,以pstride块为单位做递进的单位复根运算
			unsigned int p = idx_index & (change_percall.istride - change_percall.pstride);
			float phase = change_percall.phase_base * (float)p;
			twiddle_8(complex_num, phase);

			// Store the result
			unsigned int omod = idx_index & (change_percall.ostride - 1);
			unsigned int oaddr = ((idx_index - omod) << 3) + omod;

			dst[oaddr + 0 * change_percall.ostride] = complex_num[0];
			dst[oaddr + 1 * change_percall.ostride] = complex_num[4];
			dst[oaddr + 2 * change_percall.ostride] = complex_num[2];
			dst[oaddr + 3 * change_percall.ostride] = complex_num[6];
			dst[oaddr + 4 * change_percall.ostride] = complex_num[1];
			dst[oaddr + 5 * change_percall.ostride] = complex_num[5];
			dst[oaddr + 6 * change_percall.ostride] = complex_num[3];
			dst[oaddr + 7 * change_percall.ostride] = complex_num[7];
		});
	}
	else
	{
		parallel_for_each(m_av, compute_domain, [=](index<1> idx) restrict(amp)
		{
			const int idx_index = idx[0];
			// if (idx_index >= change_percall.thread_count)
			//     return;

			// Fetch 8 complex numbers
			float_2 complex_num[8];

			unsigned int imod = idx_index & (change_percall.istride - 1);//diffence
			unsigned int iaddr = ((idx_index - imod) << 3) + imod;
			for (int i = 0; i < 8; i++)
				complex_num[i] = src[index<1>(iaddr + i * change_percall.istride)];

			// Math
			fft_forward_8(complex_num);
			unsigned int p = idx_index & (change_percall.istride - change_percall.pstride);//do not 0 ?
			float phase = change_percall.phase_base * (float)p;
			twiddle_8(complex_num, phase);

			// Store the result
			unsigned int omod = idx_index & (change_percall.ostride - 1);
			unsigned int oaddr = ((idx_index - omod) << 3) + omod;

			dst[oaddr + 0 * change_percall.ostride] = complex_num[0];
			dst[oaddr + 1 * change_percall.ostride] = complex_num[4];
			dst[oaddr + 2 * change_percall.ostride] = complex_num[2];
			dst[oaddr + 3 * change_percall.ostride] = complex_num[6];
			dst[oaddr + 4 * change_percall.ostride] = complex_num[1];
			dst[oaddr + 5 * change_percall.ostride] = complex_num[5];
			dst[oaddr + 6 * change_percall.ostride] = complex_num[3];
			dst[oaddr + 7 * change_percall.ostride] = complex_num[7];
		});
	}

	dst.synchronize();
}

/*
Perform 512 * 512 FFT by using C++ AMP
src: input, time information.
dst: output, wave height information.
*/
void FFT512x512::fft_512x512_c2c_amp(array_view<float_2> dst, array_view<const float_2> src)
{
	const UINT thread_count = m_slices * (512 * 512) / 8;
	//512==>8 x 8 x 8
	UINT istride = 512 * 512 / 8;
	radix008A_amp(m_tmp_array_view, src, thread_count, istride, m_change_percall[0]);

	istride /= 8;//==>4096
	radix008A_amp(dst, m_tmp_array_view, thread_count, istride, m_change_percall[1]);

	istride /= 8;//===>512
	radix008A_amp(m_tmp_array_view, dst, thread_count, istride, m_change_percall[2]);

	istride /= 8;//==>64
	radix008A_amp(dst, m_tmp_array_view, thread_count, istride, m_change_percall[3]);

	istride /= 8;//==>8
	radix008A_amp(m_tmp_array_view, dst, thread_count, istride, m_change_percall[4]);

	istride /= 8;//==>1
	radix008A_amp(dst, m_tmp_array_view, thread_count, istride, m_change_percall[5]);
}


void FFT512x512::create_input()
{
	//缓冲区对象的总长度为3 * 512 * 512
	//pstride:凡是大于等于512的istride,都是按照512对齐,否则都是按照1字节对齐
	// Buffer 0
	const UINT thread_count = m_slices * (512 * 512) / 8;//==>3 x 512 x 512/8
	UINT ostride = 512 * 512 / 8;//==>64 * 64
	UINT istride = ostride;
	double phase_base = -TWO_PI / (512.0 * 512.0);

	this->m_change_percall[0].thread_count = thread_count;
	this->m_change_percall[0].ostride = ostride;//512 x 64
	this->m_change_percall[0].istride = istride; //512 x 64
	this->m_change_percall[0].pstride = 512;
	this->m_change_percall[0].phase_base = (float)phase_base;

	// Buffer 1
	istride /= 8;
	phase_base *= 8.0;

	this->m_change_percall[1].thread_count = thread_count;
	this->m_change_percall[1].ostride = ostride;//==>512 x 64
	this->m_change_percall[1].istride = istride;// ==>4096
	this->m_change_percall[1].pstride = 512;
	this->m_change_percall[1].phase_base = (float)phase_base;

	// Buffer 2
	istride /= 8;
	phase_base *= 8.0;

	this->m_change_percall[2].thread_count = thread_count;
	this->m_change_percall[2].ostride = ostride;//==>512 x 64
	this->m_change_percall[2].istride = istride;// ==>512
	this->m_change_percall[2].pstride = 512;
	this->m_change_percall[2].phase_base = (float)phase_base;

	// Buffer 3
	istride /= 8;
	phase_base *= 8.0;
	ostride /= 512;

	this->m_change_percall[3].thread_count = thread_count;
	this->m_change_percall[3].ostride = ostride;//==>64
	this->m_change_percall[3].istride = istride;//==>64
	this->m_change_percall[3].pstride = 1;
	this->m_change_percall[3].phase_base = (float)phase_base;

	// Buffer 4
	istride /= 8;
	phase_base *= 8.0;

	this->m_change_percall[4].thread_count = thread_count;
	this->m_change_percall[4].ostride = ostride;//==>64
	this->m_change_percall[4].istride = istride;//==>8
	this->m_change_percall[4].pstride = 1;
	this->m_change_percall[4].phase_base = (float)phase_base;

	// Buffer 5
	istride /= 8;
	phase_base *= 8.0;

	this->m_change_percall[5].thread_count = thread_count;
	this->m_change_percall[5].ostride = ostride;//==>64
	this->m_change_percall[5].istride = istride;//==>1
	this->m_change_percall[5].pstride = 1;
	this->m_change_percall[5].phase_base = (float)phase_base;
}
