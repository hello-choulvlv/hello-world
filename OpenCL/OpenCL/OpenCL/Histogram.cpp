/*
*图像立方图OpenCL实现
*2019年2月16日
*@author:xiaohuaxiong
*/
#include "CSKContext.h"
#include "csk_types.h"
#include<stdlib.h>
#include<math.h>
#include<string.h>
/*
*朴素方法实现
*/
typedef unsigned char byte;
static const int  s_pixel_step = 3;
void  test_prim(byte  *buffer_data, int  *statistics, int width, int height)
{
	int  loops = width * height * s_pixel_step;
	int  *color_r = statistics;
	int  *color_g = statistics + 256;
	int  *color_b = statistics + 512;

	for (int index_j = 0; index_j < loops; index_j += s_pixel_step)
	{
		++color_r[buffer_data[index_j]];
		++color_g[buffer_data[index_j + 1]];
		++color_b[buffer_data[index_j + 2]];
	}
}
/*
*产生图形数据
*/
byte  *create_graphics_buffer(int width, int height)
{
	int  graphics_number = width *height * s_pixel_step;
	byte  *buffer = new byte[graphics_number];
	for (int index_j = 0; index_j < graphics_number; index_j += s_pixel_step)
	{
		buffer[index_j] = rand() & 0xFF;
		buffer[index_j + 1] = rand() & 0xFF;
		buffer[index_j + 2] = rand() & 0xFF ;
	}
	return buffer;
}
/*
*检查结果是否一致
*/
bool verify_histogram_results(const int *histogram, const int *ref_result, int  number)
{
	for (int index_j = 0; index_j < number; ++index_j)
	{
		if (histogram[index_j] != ref_result[index_j])
		{
			printf("error verify_histogram_results at index %d\n", index_j);
			return false;
		}
	}
	return true;
}

int main(int  argc, char *argv[])
{
	CSKContext   vk_context;
	vk_context.init();
	cl_int     ref_result = 0;
	/*
	*内核对象
	*/
	cl_program  program = vk_context.createCLProgram("kernel/histogram.cl");
	CL_ASSERT(program != nullptr, "vk_context.createCLProgram error.");
	cl_kernel      kernel_image_tong = clCreateKernel(program, "horigram_image_trong", &ref_result);
	cl_kernel      kernel_merge = clCreateKernel(program, "histogram_merge", &ref_result);
	CL_ASSERT(kernel_image_tong != nullptr &&kernel_merge != nullptr, "clCreateKernel error.");
	/*
	*创建OpenCL图像
	*/
	const int  image_width = 1920;
	const int  image_height = 1080;
	byte  *image_buffer = create_graphics_buffer(image_width, image_height);
	const cl_image_format image_format = {
		CL_RGBA,
		CL_UNORM_INT8,
	};
	cl_mem  image_mem = create_texture_read_only(vk_context._context, image_buffer, image_width, image_height, image_format);
	CL_ASSERT(image_mem != nullptr, "create_texture_read_only error.");
	/*
	*创建OpenCL内存对象,各个线程组
	*/
	size_t  globalWork[2], localWork[2];
	size_t  max_group_size = 0;
	int        pixel_number_thread = 32;//每线程处理的像素的数目s
	clGetKernelWorkGroupInfo(kernel_image_tong, vk_context._device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &max_group_size, nullptr);
	if (max_group_size < 256)
	{
		localWork[0] = max_group_size / 16;
		localWork[1] = 16;
	}
	else
	{
		localWork[0] = max_group_size / 32;
		localWork[1] = 32;
	}
	globalWork[0] = ceilf(1.0f * image_width / pixel_number_thread);
	globalWork[0] = (globalWork[0] + localWork[0] - 1) / localWork[0] * localWork[0];
	globalWork[1] = ceilf(1.0f *image_height / localWork[1]) * localWork[1];
	//内存对象
	const int array_size = 256 * 3;
	const int group_size = globalWork[0] * globalWork[1] / (localWork[0] * localWork[1]);
	cl_mem histogram_mem = clCreateBuffer(vk_context._context, CL_MEM_READ_WRITE, sizeof(cl_uint) *group_size * array_size, nullptr, &ref_result);
	CL_ASSERT(histogram_mem != nullptr && ref_result == CL_SUCCESS, "clCreateBuffer error.");

	cl_mem merge_buffer = clCreateBuffer(vk_context._context, CL_MEM_READ_WRITE, sizeof(cl_uint) * array_size, nullptr, &ref_result);
	CL_ASSERT(merge_buffer != nullptr && ref_result == CL_SUCCESS, "clCreateBuffer error.");
	/*
	*设置内和参数的值
	*/
	ref_result = clSetKernelArg(kernel_image_tong, 0, sizeof(cl_mem), &image_mem);
	ref_result |= clSetKernelArg(kernel_image_tong, 1, sizeof(cl_int), &pixel_number_thread);
	ref_result |= clSetKernelArg(kernel_image_tong, 2, sizeof(cl_mem), &histogram_mem);
	CL_ASSERT(ref_result == CL_SUCCESS, "clSetKernelArg error.");
	cl_event    event1 = nullptr;
	ref_result = clEnqueueNDRangeKernel(vk_context._commandQueue, kernel_image_tong, 2, nullptr, globalWork, localWork, 0, nullptr, &event1);
	CL_ASSERT(ref_result == CL_SUCCESS, "clEnqueueNDRangeKernel error.");
	/*
	*第二阶段,使用上一阶段返回的数据计算新的数据
	*/
	clWaitForEvents(1, &event1);
	event1 = nullptr;

	ref_result = clSetKernelArg(kernel_merge, 0, sizeof(cl_mem), &histogram_mem);
	ref_result |= clSetKernelArg(kernel_merge, 1, sizeof(cl_int), &group_size);
	ref_result |= clSetKernelArg(kernel_merge, 2, sizeof(cl_mem), &merge_buffer);
	CL_ASSERT(ref_result == CL_SUCCESS, "clSetKernelArg error.");

	globalWork[0] = array_size;
	localWork[0] = localWork[0] > 256 ? 256 : localWork[0];
	ref_result = clEnqueueNDRangeKernel(vk_context._commandQueue, kernel_merge, 1, nullptr, globalWork, localWork, 0, nullptr, &event1);
	CL_ASSERT(ref_result == CL_SUCCESS, "clEnqueueNDRangeKernel error.");
	clWaitForEvents(1, &event1);
	//read back
	cl_int   result_buffer[array_size];
	clEnqueueReadBuffer(vk_context._commandQueue, merge_buffer, CL_FALSE, 0, array_size * sizeof(cl_uint), result_buffer, 0, nullptr, &event1);
	clWaitForEvents(1, &event1);
	//compare
	int   origin_merge_buffer[array_size];
	memset(origin_merge_buffer, 0, sizeof(int)*array_size);
	test_prim(image_buffer, origin_merge_buffer, image_width, image_height);
	verify_histogram_results(origin_merge_buffer, result_buffer, array_size);
	//release

	delete[] image_buffer;
	clReleaseMemObject(image_mem);
	clReleaseMemObject(histogram_mem);
	clReleaseMemObject(merge_buffer);

	clReleaseKernel(kernel_image_tong);
	clReleaseKernel(kernel_merge);

	printf("finished....");
	getchar();
	return 0;
}