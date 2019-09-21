// OpenCL.cpp : 定义控制台应用程序的入口点。
//
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

cl_program   create_CLProgram(cl_context context,cl_device_id device,const char *filename)
{
	std::ifstream    input_file(filename,std::ios::in|std::ios::binary);
	if (!input_file.is_open())
	{
		printf("file '%s' can not open\n",filename);
		return nullptr;
	}
	input_file.seekg(0, std::ios::end);
	int  size = input_file.tellg();
	input_file.seekg(0, std::ios::beg);

	char  *buffer = new char[size+2];
	input_file.read(buffer, size);
	buffer[size] = 0;
	input_file.close();
	//Compile
	cl_program  program = clCreateProgramWithSource(context, 1, (const char **)&buffer, nullptr, nullptr);
	if (!program)
	{
		printf("failed to create program,file->%s\n",filename);
		return nullptr;
	}
	int result = clBuildProgram(program, 0, nullptr, nullptr, nullptr,nullptr);
	if (result != CL_SUCCESS)
	{
		size_t  buffer_size = 0;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &buffer_size);
		char  *buffer = new char[buffer_size + 2];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, buffer_size, buffer, nullptr);
		buffer[buffer_size] = 0;
		printf("Compile error:%s\n",buffer);
		delete[] buffer;
		buffer = nullptr;
		clReleaseProgram(program);
		program = nullptr;
	}
	return program;
}

int main(int argc,char *argv[])
{
	cl_context   context = nullptr;
	cl_command_queue   cmd_queue = nullptr;
	cl_program   program = nullptr;
	cl_device_id  device = nullptr;
	cl_kernel    kernel = nullptr;

	cl_mem  mem_objects[3] = {nullptr,nullptr,nullptr};
	/*
	  *获取平台
	 */
	cl_platform_id   platform = nullptr;
	cl_uint                  platform_count = 0;
	int   result = clGetPlatformIDs(1, &platform, &platform_count);
	CL_ASSERT(result == CL_SUCCESS && platform_count > 0,"get platform ids error.");
	/*
	  *在平台上创建上下文
	 */
	cl_context_properties   properties[3] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platform,
		0,
	};
	context = clCreateContextFromType(properties, CL_DEVICE_TYPE_GPU, nullptr, nullptr, &result);
	CL_ASSERT(context && result == CL_SUCCESS,"create context from gpu type failed.");
	/*
	  *创建设备和命令队列
	 */
	size_t   device_size = 0;
	result = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, nullptr, &device_size);
	CL_ASSERT(result == CL_SUCCESS && device_size > 0,"device number not available.");
	std::vector<cl_device_id>  devices(device_size);
	result = clGetContextInfo(context, CL_CONTEXT_DEVICES, device_size, devices.data(), nullptr);
	device = devices[0];
	//命令队列
	cmd_queue = clCreateCommandQueue(context, device,0,nullptr);
	CL_ASSERT(cmd_queue,"create command queue error.");
	/*
	  *创建程序对象
	 */
	program = create_CLProgram(context, device, "kernel/hello_world.cl");
	CL_ASSERT(program,"create program failed.");
	/*
	  *创建内核对象
	 */
	kernel = clCreateKernel(program, "hello_world", &result);
	CL_ASSERT(kernel && result == CL_SUCCESS,"create kernel failed.");
	/*
	  *创建内存对象所需要的数据
	 */
	float  array_a[100];
	float  array_b[100];
	for (int index_j = 0; index_j < 100; ++index_j)
	{
		array_a[index_j] = index_j + 1;
		array_b[index_j] = index_j + 2;
	}
	/*
	  *创建内存对象
	 */
	mem_objects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(array_a), array_a,nullptr);
	mem_objects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(array_b), array_b, nullptr);
	mem_objects[2] = clCreateBuffer(context,CL_MEM_READ_WRITE,sizeof(array_b),nullptr,nullptr);
	CL_ASSERT(mem_objects[0] && mem_objects[1] && mem_objects[2],"create memory buffer failed.");
	/*
	  *设置内核参数
	 */
	result = clSetKernelArg(kernel, 0, sizeof(cl_mem),&mem_objects[0]);
	result &= clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem_objects[1]);
	result &= clSetKernelArg(kernel, 2, sizeof(cl_mem), &mem_objects[2]);
	CL_ASSERT(result == CL_SUCCESS,"Set Kernel Arg error.");
	/*
	  *设置全局与局部工作项与工作组
	 */
	size_t  globalWorkSize[1] = {100};
	size_t  localWorkSize[1] = {1};
	/*
	  *将内核加入到命令队列中
	 */
	result = clEnqueueNDRangeKernel(cmd_queue,kernel,1,nullptr,globalWorkSize,localWorkSize,0,nullptr,nullptr);
	CL_ASSERT(result == CL_SUCCESS,"enqueue failed.");
	/*
	  *回读数据
	 */
	result = clEnqueueReadBuffer(cmd_queue, mem_objects[2], CL_TRUE, 0,sizeof(array_b),array_b,0,nullptr,nullptr);
	CL_ASSERT(result == CL_SUCCESS,"enqueue read buffer failed.");
	/*
	  *打印数据
	 */
	for (int index_j = 0; index_j < 100; ++index_j)
	{
		printf("%f		", array_b[index_j]);
		if (index_j % 6 == 0)
			printf("\n");
	}
	/*
	  *清理
	 */
	clReleaseMemObject(mem_objects[0]);
	clReleaseMemObject(mem_objects[1]);
	clReleaseMemObject(mem_objects[2]);

	clReleaseCommandQueue(cmd_queue);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseContext(context);

	getchar();
    return 0; 
}

