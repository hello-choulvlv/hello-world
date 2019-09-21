//--------------------------------------------------------------------------------------
// File: fft_512x512.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#ifndef _FFT_512x512_H__
#define _FFT_512x512_H__

#include <amp.h>
#include <amp_math.h>
#include <amp_short_vectors.h>

////////////////////////////////////////////////////////////////////////////////
// Common constants
////////////////////////////////////////////////////////////////////////////////
#define TWO_PI 6.283185307179586476925286766559
#define MAX_BUF_LEN 0x600000
#define COS_PI_4_16 0.70710678118654752440084436210485f
#define TWIDDLE_1_8 COS_PI_4_16, -COS_PI_4_16
#define TWIDDLE_3_8 -COS_PI_4_16, -COS_PI_4_16

///////////////////////////////////////////////////////////////////////////////
// Common types
///////////////////////////////////////////////////////////////////////////////

struct change_percall
{
	//线程数
	unsigned int thread_count;
	//输出数据的间隔
	unsigned int ostride;
	//输入数据的间隔
	unsigned int istride;
	//对齐方式,凡是大于等于512字节的都是按照512字节对齐,否则都是按照1字节对齐
	unsigned int pstride;
	//单位复根的基本指数
	float phase_base;
};

class FFT512x512
{
public:
	// Constructor
	FFT512x512(UINT slices, concurrency::accelerator_view av)
		: m_slices(slices),
		m_av(av),
		//临时数组的容量= 3 x 512 x 512 x sizeof(float_2)
		m_tmp_array_view(concurrency::array<concurrency::graphics::float_2>((MAX_BUF_LEN / sizeof(concurrency::graphics::float_2)), av))
	{
		create_input();
	}

	void fft_512x512_c2c_amp(concurrency::array_view<concurrency::graphics::float_2> dst, concurrency::array_view<const concurrency::graphics::float_2> src);

private:
	void radix008A_amp(concurrency::array_view<concurrency::graphics::float_2> dst,
		concurrency::array_view<const concurrency::graphics::float_2> src,
		UINT thread_count,
		UINT istride,
		change_percall change_percall);

	static void ft2(concurrency::graphics::float_2 &a, concurrency::graphics::float_2 &b) restrict(amp);
	static void cmul_forward(concurrency::graphics::float_2 &a, float bx, float by) restrict(amp);
	static void upd_forward(concurrency::graphics::float_2 &a, concurrency::graphics::float_2 &b) restrict(amp);
	static void fft_forward_4(concurrency::graphics::float_2 complex_num[8]) restrict(amp);
	static void fft_forward_8(concurrency::graphics::float_2 complex_num[8]) restrict(amp);
	static void twiddle(concurrency::graphics::float_2 &d, float phase) restrict(amp);
	static void twiddle_8(concurrency::graphics::float_2 complex_num[8], float phase) restrict(amp);

	void create_input();

	UINT m_slices;

	// For 512x512 config, we need 6 buffers
	change_percall m_change_percall[6];

	// Temp data
	concurrency::array_view<concurrency::graphics::float_2> m_tmp_array_view;

	concurrency::accelerator_view m_av;
};

#endif
