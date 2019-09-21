/*
  *OpenCL图像处理
  *2019年2月9日
 */
#include<stdio.h>
#include<vector>
#include<FreeImage/FreeImage.h>
#include "CSKContext.h"
#include "csk_types.h"

int  main(int argc, char *argv[])
{
	CSKContext    csk_context;
	csk_context.init();
	//显示device 扩展
	char  *device_extension = nullptr;
	size_t  extension_length = 0;
	clGetDeviceInfo(csk_context._device, CL_DEVICE_EXTENSIONS, 0, nullptr,&extension_length);
	device_extension = new char[extension_length + 4];
	clGetDeviceInfo(csk_context._device, CL_DEVICE_EXTENSIONS, extension_length, device_extension,nullptr);
	int  index_j = 0;
	int  index_i = 0;
	char  extension[128];
	while (index_j < extension_length)
	{
		if (device_extension[index_j] != ' ')
			extension[index_i++] = device_extension[index_j];
		else
		{
			extension[index_i] = '\0';
			index_i = 0;
			printf("%s\n", extension);
		}
		++index_j;
	}
	delete[] device_extension;
	device_extension = nullptr;

	int  width = 0, height = 0;
	cl_mem   image_source = create_texture(csk_context._context, "image/global_img_heng4.tga",&width,&height);
	CL_ASSERT(image_source != nullptr,"create_texture error.");
	/*
	  *创建目标纹理
	 */
	int  result = 0;
	cl_image_format  image_format;
	image_format.image_channel_order = CL_RGBA;
	image_format.image_channel_data_type = CL_UNORM_INT8;
	cl_mem  target_image = clCreateImage2D(csk_context._context, CL_MEM_WRITE_ONLY,&image_format,width,height,0,nullptr,&result);
	CL_ASSERT(target_image!=nullptr && result ==CL_SUCCESS,"clCreateImage2D error.");
	/*
	  *创建采样器对象
	 */
	cl_sampler  sampler = clCreateSampler(csk_context._context,CL_FALSE,CL_ADDRESS_CLAMP_TO_EDGE,CL_FILTER_LINEAR,&result);
	CL_ASSERT(sampler!=nullptr && result == CL_SUCCESS,"clCreateSampler error");
	/*
	  *创建程序对象与内核
	 */
	cl_program   program = csk_context.createCLProgram("kernel/image_filter.cl");
	CL_ASSERT(program != nullptr,"createCLProgram error.");
	cl_kernel kernel= clCreateKernel(program, "gaussion_filter", &result);
	CL_ASSERT(kernel!=nullptr && result == CL_SUCCESS,"clCreateKernel error.");
	/*设置内核参数
	 */
	result = clSetKernelArg(kernel, 0, sizeof(cl_mem), &image_source);
	result |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &target_image);
	result |= clSetKernelArg(kernel, 2, sizeof(cl_sampler),&sampler);
	result |= clSetKernelArg(kernel, 3, sizeof(cl_int), &width);
	result |= clSetKernelArg(kernel, 4, sizeof(cl_int), &height);
	CL_ASSERT(result == CL_SUCCESS,"clSetKernelArg error");
	//派发队列
	size_t localWorkSize[2] = {16,16};
	size_t globalWorkSize[2] = {ceil(width/16.0f) * 16,ceil(height/16.0f) * 16};
	cl_event  event = nullptr;
	result = clEnqueueNDRangeKernel(csk_context._commandQueue, kernel, 2, nullptr, globalWorkSize, localWorkSize, 0, nullptr, &event);
	clWaitForEvents(1, &event);

	char  *buffer = new char[width * height * 4];
	size_t origin_location[3] = {
		0,0,0
	};
	size_t region_rect[3] = {
		width,height,1,
	};
	result = clEnqueueReadImage(csk_context._commandQueue, target_image, CL_FALSE, origin_location, region_rect, 0, 0, buffer, 0, nullptr, &event);
	clWaitForEvents(1, &event);

	save_texture(csk_context._context, buffer, "image/global_img_heng4.png", width, height);
	delete[] buffer;
	buffer = nullptr;

	clReleaseSampler(sampler);
	clReleaseMemObject(image_source);
	clReleaseMemObject(target_image);
	clReleaseKernel(kernel);
	clReleaseProgram(program);

	getchar();
	return 0;
}
