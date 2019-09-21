/*
  *Sobel 检测
  *OpenCL实现
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

int  main(int  argc,char *argv[])
{
	cl_platform_id	   platform = nullptr;
	cl_context				context = nullptr;
	cl_device_id        device = nullptr;
	cl_uint                  platform_number = 0;

	int  result = clGetPlatformIDs(1, nullptr, &platform_number);
	CL_ASSERT(result == CL_SUCCESS && platform_number > 0,"get OpenCL platform failed.");
	std::vector<cl_platform_id>    platformIds(platform_number);
	clGetPlatformIDs(platform_number, platformIds.data(), nullptr);
	/*
	  *针对所有的平台进行检测
	 */
	struct  PlatformDevice
	{
		cl_platform_id  platform;
		std::vector<cl_device_id>  devices;
	};
	std::vector<PlatformDevice>  gpu_queue;
	std::vector<PlatformDevice>  cpu_queue;
	for (cl_uint index_j = 0; index_j < platform_number; ++index_j)
	{
		cl_uint  device_count = 0;
		clGetDeviceIDs(platformIds[index_j], CL_DEVICE_TYPE_GPU, 0,nullptr,&device_count);
		/*
		  *如果发现了GPU设备,则立刻记录下来并跳出循环
		 */
		if (device_count > 0)
		{
			std::vector<cl_device_id>  devices(device_count);
			clGetDeviceIDs(platformIds[index_j], CL_DEVICE_TYPE_GPU, device_count, devices.data(), nullptr);

			PlatformDevice platform_device = {
				platformIds[index_j],
				devices,
			};
			gpu_queue.push_back(platform_device);
		}

		clGetDeviceIDs(platformIds[index_j], CL_DEVICE_TYPE_CPU, 0, nullptr, &device_count);
		if (device_count > 0)
		{
			std::vector<cl_device_id>  devices(device_count);
			clGetDeviceIDs(platformIds[index_j], CL_DEVICE_TYPE_GPU, device_count, devices.data(), nullptr);
			
			PlatformDevice platform_device = {
				platformIds[index_j],
				devices,
			};
			cpu_queue.push_back(platform_device);
		}
	}
	//如果有GPU设备
	if (gpu_queue.size() > 0)
	{
		platform = gpu_queue[0].platform;
		device = gpu_queue[0].devices[0];
	}
	else if (cpu_queue.size() > 0)
	{
		platform = cpu_queue[0].platform;
		device = cpu_queue[0].devices[0];
	}
	CL_ASSERT(platform != nullptr && device !=nullptr,"Could not find target platform and device.");
	/*
	  *创建上下文
	 */
	cl_context_properties   propertites[3] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platform,
		0,
	};
	context = clCreateContext(propertites, 1, &device,nullptr, nullptr, &result);
	CL_ASSERT(context && result == CL_SUCCESS,"create context failed.");
	/*
	  *创建程序对象
	 */
	cl_program  program = create_CLProgram(context, device, "kernel/sobel_check.cl");
	CL_ASSERT(program,"create program failed.");
	/*
	  *创建内核对象
	 */
	cl_kernel kernel = clCreateKernel(program, "sobel_check", &result);
	CL_ASSERT(kernel && result == CL_SUCCESS,"createKernel failed");
	/*
	  *创建命令队列
	 */
	cl_command_queue queue = clCreateCommandQueue(context, device, 0,&result);
	CL_ASSERT(queue && result == CL_SUCCESS,"create command queue failed.");
	/*
	  *创建内存对象
	 */
	const unsigned int inputSignalWidth = 8;
	const unsigned int inputSignalHeight = 8;

	cl_uint inputSignal[inputSignalWidth][inputSignalHeight] =
	{
		{ 3, 1, 1, 4, 8, 2, 1, 3 },
		{ 4, 2, 1, 1, 2, 1, 2, 3 },
		{ 4, 4, 4, 4, 3, 2, 2, 2 },
		{ 9, 8, 3, 8, 9, 0, 0, 0 },
		{ 9, 3, 3, 9, 0, 0, 0, 0 },
		{ 0, 9, 0, 8, 0, 0, 0, 0 },
		{ 3, 0, 8, 8, 9, 4, 4, 4 },
		{ 5, 9, 8, 1, 8, 1, 1, 1 }
	};

	const unsigned int outputSignalWidth = 6;
	const unsigned int outputSignalHeight = 6;

	cl_uint outputSignal[outputSignalWidth][outputSignalHeight];

	const unsigned int maskWidth = 3;
	const unsigned int maskHeight = 3;

	cl_uint mask[maskWidth][maskHeight] =
	{
		{ 1, 1, 1 },{ 1, 0, 1 },{ 1, 1, 1 },
	};
	cl_mem   input_mem_object = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(inputSignal),inputSignal,&result);
	cl_mem   mask_mem_object = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(mask),mask,&result);
	cl_mem   output_mem_object = clCreateBuffer(context,CL_READ_WRITE_CACHE,sizeof(int)*outputSignalWidth * outputSignalHeight,nullptr,&result);
	CL_ASSERT(input_mem_object && mask_mem_object && output_mem_object,"create buffer failed.");
	/*
	  *设置内核参数
	 */
	result = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_mem_object);
	result |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &mask_mem_object);
	result |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_mem_object);
	result |= clSetKernelArg(kernel, 3, sizeof(inputSignalWidth), &inputSignalWidth);
	result |= clSetKernelArg(kernel, 4, sizeof(maskWidth), &maskWidth);
	CL_ASSERT(result == CL_SUCCESS,"set kernel arg failed.");
	/*
	  *派发线程
	 */
	size_t  globalWorkSize = outputSignalWidth * outputSignalHeight;
	size_t  localWorkSize = 1;
	result = clEnqueueNDRangeKernel(queue, kernel,1,nullptr,&globalWorkSize,&localWorkSize,0,nullptr,nullptr);
	CL_ASSERT(result == CL_SUCCESS,"enqueue failed.");
	/*
	  *回读数据
	 */
	result = clEnqueueReadBuffer(queue, output_mem_object, CL_TRUE, 0, sizeof(int)*outputSignalWidth * outputSignalHeight, outputSignal,0,nullptr,nullptr);
	CL_ASSERT(result == CL_SUCCESS,"clEnqueueReadBuffer failed.");
	/*
	  *display
	 */
	for (int i = 0; i < outputSignalWidth; ++ i)
	{
		for (int j = 0; j < outputSignalHeight; ++j)
		{
			printf("%d  ", outputSignal[i][j]);
		}
		printf("\n");
	}
	/*
	  *clean
	 */
	clReleaseMemObject(output_mem_object);
	clReleaseMemObject(mask_mem_object);
	clReleaseMemObject(input_mem_object);

	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	getchar();
	return 0;
}