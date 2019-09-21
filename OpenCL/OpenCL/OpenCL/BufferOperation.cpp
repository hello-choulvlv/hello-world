/*
  *OpenCL缓冲区对象操作
  *2019年2月8日
  *@author:xiaohuaxiong
 */
#include<stdio.h>
#include<CL/cl.h>
#include<fstream>
#include<vector>
#include<assert.h>

#define CL_ASSERT(condition,error_message)   if(!(condition))\
{\
	printf("function %s,line %d,%s-->%s\n",__FUNCTION__,__LINE__,#condition,error_message);\
	assert(false);\
}

cl_program   create_CLProgram(cl_context context, cl_device_id device, const char *filename)
{
	std::ifstream    input_file(filename, std::ios::in | std::ios::binary);
	if (!input_file.is_open())
	{
		printf("file '%s' can not open\n", filename);
		return nullptr;
	}
	input_file.seekg(0, std::ios::end);
	int  size = input_file.tellg();
	input_file.seekg(0, std::ios::beg);

	char  *buffer = new char[size + 2];
	input_file.read(buffer, size);
	buffer[size] = 0;
	input_file.close();
	//Compile
	cl_program  program = clCreateProgramWithSource(context, 1, (const char **)&buffer, nullptr, nullptr);
	if (!program)
	{
		printf("failed to create program,file->%s\n", filename);
		return nullptr;
	}
	int result = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);
	if (result != CL_SUCCESS)
	{
		size_t  buffer_size = 0;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &buffer_size);
		char  *buffer = new char[buffer_size + 2];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, buffer_size, buffer, nullptr);
		buffer[buffer_size] = 0;
		printf("Compile error:%s\n", buffer);
		delete[] buffer;
		buffer = nullptr;
		clReleaseProgram(program);
		program = nullptr;
	}
	return program;
}

int  main(int argc, char *argv[])
{
	cl_platform_id    platform = nullptr;
	cl_uint                   platform_count=0;
	//platform
	cl_int  result = clGetPlatformIDs(1, &platform, &platform_count);
	CL_ASSERT(result == CL_SUCCESS,"clGetPlatformIDs error");
	//device
	cl_device_id  device = nullptr;
	cl_uint             device_count = 0;
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device,&device_count);
	//create context
	cl_context_properties  context_property[3] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platform,
		0,
	};
	cl_context       context = clCreateContext(context_property,1,&device,nullptr,nullptr,&result);
	CL_ASSERT(context!=nullptr && result == CL_SUCCESS,"clCreateContext error");
	//program
	cl_program  program = create_CLProgram(context, device, "kernel/buffer_operation.cl");
	CL_ASSERT(program != nullptr,"create_CLProgram error.");
	//Kernel
	cl_kernel    kernel = clCreateKernel(program, "buffer_operation", &result);
	CL_ASSERT(kernel != nullptr, "clCreateKernel error.");
	//buffer
	const size_t array_size = 100;
	float   buffer[array_size];
	for (int index_j = 0; index_j < array_size; ++index_j)
		buffer[index_j] = index_j;
	cl_mem  mem_buffer = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY, array_size * sizeof(float),buffer,&result);
	cl_mem  result_buffer = clCreateBuffer(context,CL_MEM_READ_WRITE,sizeof(float)* array_size,nullptr,&result);
	//create sub buffer
	cl_buffer_region  region = {
		0,
		sizeof(float) * array_size/2,
	};
	cl_buffer_region region2 = {
		array_size * sizeof(float)/2,
		array_size * sizeof(float)/2,
	};
	cl_mem  mem_sub_buffer1 = clCreateSubBuffer(mem_buffer, CL_MEM_READ_ONLY,CL_BUFFER_CREATE_TYPE_REGION,&region,&result);
	cl_mem  mem_sub_buffer2 = clCreateSubBuffer(mem_buffer, CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &region2, &result);
	CL_ASSERT(mem_sub_buffer1 && mem_sub_buffer2,"clCreateSubBuffer 1");
	//
	cl_mem  mem_sub_result1 = clCreateSubBuffer(result_buffer,CL_MEM_READ_WRITE,CL_BUFFER_CREATE_TYPE_REGION,&region,&result);
	cl_mem  mem_sub_result2 = clCreateSubBuffer(result_buffer, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &region2, &result);
	CL_ASSERT(mem_sub_result1 && mem_sub_result2,"clCreateSubBuffer 2");
	//set kernel
	clSetKernelArg(kernel, 0, sizeof(cl_mem),&mem_sub_buffer1);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem_sub_result1);
	//创建命令队列
	cl_command_queue_properties queue_property[3] = {
	};
	cl_command_queue  command_queue = clCreateCommandQueue(context, device,0,&result);
	cl_event  events[2] = {nullptr,nullptr};
	const size_t  dispatch_size = array_size / 2;
	clEnqueueNDRangeKernel(command_queue, kernel, 1, nullptr, &dispatch_size,nullptr,0,nullptr,&events[0]);
	clWaitForEvents(1, events);

	float  result_buffer_ptr[array_size] = {0};
	result = clEnqueueReadBuffer(command_queue, mem_sub_result1, CL_FALSE,0,dispatch_size * sizeof(float),result_buffer_ptr, 0, nullptr, &events[1]);
	CL_ASSERT(result == CL_SUCCESS,"clEnqueueReadBuffer error.");
	clWaitForEvents(1, &events[1]);
	//设置第二个缓冲区对象
	clSetKernelArg(kernel,0, sizeof(cl_mem),&mem_sub_buffer2);
	clSetKernelArg(kernel,1,sizeof(cl_mem),&mem_sub_result2);
	clEnqueueNDRangeKernel(command_queue, kernel, 1, nullptr, &dispatch_size, nullptr, 0, nullptr, &events[0]);
	clWaitForEvents(1, events);

	float  *result_ptr = (float *)clEnqueueMapBuffer(command_queue, mem_sub_result2, CL_FALSE, CL_MAP_READ, 0, sizeof(float)*dispatch_size, 0, nullptr, &events[1],&result);
	CL_ASSERT(result == CL_SUCCESS,"clEnqueueMapBuffer error");
	clWaitForEvents(1, &events[1]);
	for (int index_j = 0; index_j < dispatch_size; ++index_j)
	{
		result_buffer_ptr[dispatch_size + index_j] = result_ptr[index_j];
	}
	clEnqueueUnmapMemObject(command_queue, mem_sub_result2, result_ptr, 0, nullptr,&events[0]);
	clWaitForEvents(1, &events[0]);
	//输出打印
	for (int j = 0; j < array_size; ++j)
	{
		printf("%f  ", result_buffer_ptr[j]);
		if (j && j % 8 == 0)
			printf("\n");
	}
	//release
	clReleaseMemObject(mem_sub_result2);
	clReleaseMemObject(mem_sub_result1);
	clReleaseMemObject(mem_sub_buffer2);
	clReleaseMemObject(mem_sub_buffer1);

	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);

	getchar();
	return 0;
}