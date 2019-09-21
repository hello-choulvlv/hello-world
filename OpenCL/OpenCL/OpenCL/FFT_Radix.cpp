/*
  *快速傅里叶变换(Fast Fourier Transform)实现
  *2019年3月7日
  *@author:xiaoxiong
 */
#include "CSKContext.h"
#include "csk_types.h"
#include <stdio.h>
#include<cmath>
//程序中使用的数组的长度
static const int  s_fftNumber = 256;
/*
  *复数的定义
 */
struct Complex
{
	float x,y;
};
/*
  *傅里叶变换的朴素实现
 */
void  fourier_prim_realize(Complex    *source,Complex  *target,int  length)
{
	// Compute the forward DFT for reference
	for (int i = 0; i < length; i++)
	{
		target[i].x = 0.0f;
		target[i].y = 0.0f;

		float fArg = -1.0f * 2.0f * M_PI * i / length;
		for (int j = 0; j < length; j++)
		{
			float fCos = cosf(j * fArg);
			float fSin = sinf(j * fArg);

			target[i].x += source[j].x * fCos - source[j].y * fSin;
			target[i].y += source[j].x * fSin + source[j].y * fCos;
		}
	}
}
//逆傅里叶变换
void  fourier_reverse_prim_realize(Complex    *source, Complex  *target, int  length)
{
	// Compute the inverse DFT for reference
	for (int i = 0; i < length; i++)
	{
		target[i].x = 0.0f;
		target[i].y = 0.0f;

		float fArg = 2.0f * M_PI * i / length;
		for (int j = 0; j < length; j++)
		{
			float fCos = cosf(j * fArg);
			float fSin = sinf(j * fArg);

			target[i].x += source[j].x * fCos - source[j].y * fSin;
			target[i].y += source[j].x * fSin + source[j].y * fCos;
		}

		target[i].x /= length;
		target[i].y /= length;
	}
}
/*
  *比较生成的数据的差异
 */
bool  check_fft_complex(const Complex *source,const Complex  *target,int  length)
{
	for (int index_j = 0; index_j < length; ++index_j)
	{
		if (abs(source[index_j].x - target[index_j].x) > 0.1f || abs(source[index_j].y - target[index_j].y) > 0.1f)
		{
			printf("error ,check_fft_complex at index:%d\n",index_j);
			return false;
		}
	}
	printf("succeed in check_fft_complex.");
	return true;
}

void  init_fft_buffer(Complex  *source,int array_size)
{
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		source[index_j].x = 10.0f * rand() / RAND_MAX;
		source[index_j].y = 10.0f * rand() / RAND_MAX;
	}
}

int  main(int  argc,char *argv[])
{
	CSKContext  vkContext;
	vkContext.init();
	cl_int  ref_result = 0;
	//创建程序对象,内核函数
	cl_program program = create_program(vkContext._context, vkContext._device, "kernel/fft_radix.cl");
	cl_kernel     fft_kernel = clCreateKernel(program, "fft_radix4", &ref_result);
	CL_ASSERT(program != nullptr && fft_kernel != nullptr && ref_result == CL_SUCCESS,"clCreateKernel error.");
	//程序数据以及相关的内存对象
	Complex    source[s_fftNumber];
	Complex    target[s_fftNumber];
	Complex    ref_buffer[s_fftNumber];
	//初始化相关的数据
	init_fft_buffer(source, s_fftNumber);
	cl_mem   fft_buffer1 = clCreateBuffer(vkContext._context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Complex) * s_fftNumber, source, &ref_result);
	CL_ASSERT(fft_buffer1!=nullptr && ref_result == CL_SUCCESS,"clCreateBuffer error.");
	cl_mem   fft_buffer2 = clCreateBuffer(vkContext._context,CL_MEM_READ_WRITE,sizeof(Complex) * s_fftNumber,nullptr,&ref_result);
	CL_ASSERT(fft_buffer2!=nullptr && ref_result == CL_SUCCESS,"clCreateBuffer error.");

	int  loops = 0;
	cl_mem   buffers[2] = {fft_buffer1,fft_buffer2};//ping pong
	const int    buffer_log2 = logf(s_fftNumber);
	const size_t  globalWorkSize = s_fftNumber / 4;
	const size_t localWorkSize = 4;// 16;

	for (int p = 1; p < s_fftNumber; p <<= 2)
	{
		ref_result = clSetKernelArg(fft_kernel,0,sizeof(cl_mem),&buffers[loops]);
		ref_result |= clSetKernelArg(fft_kernel, 1, sizeof(cl_mem),&(buffers[(loops +1) & 0x1]));
		ref_result |= clSetKernelArg(fft_kernel, 2, sizeof(int), &p);
		CL_ASSERT(ref_result == CL_SUCCESS,"clSetKernelArg error.");

		cl_event   wait_event = nullptr;
		ref_result = clEnqueueNDRangeKernel(vkContext._commandQueue, fft_kernel, 1, nullptr, &globalWorkSize, &localWorkSize, 0, nullptr, &wait_event);
		CL_ASSERT(ref_result == CL_SUCCESS,"clEnqueueNDRangeKernel error.");
		clWaitForEvents(1, &wait_event);

		loops = (loops + 0x1) & 0x1;
	}
	//read buffer
	cl_event   read_event = nullptr;
	ref_result = clEnqueueReadBuffer(vkContext._commandQueue,buffers[loops],CL_FALSE,0,sizeof(Complex)*s_fftNumber,ref_buffer,0,nullptr,&read_event);
	CL_ASSERT(ref_result == CL_SUCCESS,"clEnqueueReadBuffer error.");
	clWaitForEvents(1, &read_event);
	//check result
	fourier_prim_realize(source, target, s_fftNumber);
	check_fft_complex(target, ref_buffer, s_fftNumber);
	//relase
	clReleaseKernel(fft_kernel);
	clReleaseProgram(program);
	clReleaseMemObject(fft_buffer1);
	clReleaseMemObject(fft_buffer2);

	getchar();
	return 0;
}
