/*
  *OpenCL上下文初始化
  *2019年2月9日
  *@author:xiaohuyaxiong
 */
#include "CSKContext.h"
#include "csk_types.h"
#include<fstream>
#include<vector>

CSKContext::CSKContext():
	_platform(nullptr)
	,_context(nullptr)
	,_device(nullptr)
	,_commandQueue(nullptr)
	, _deviceType(CL_DEVICE_TYPE_GPU)
{
}

CSKContext::~CSKContext()
{
	clReleaseCommandQueue(_commandQueue);
	clReleaseContext(_context);
}

bool CSKContext::init()
{
	//获取平台数据结构
	cl_uint                  platform_count = 0;
	cl_int  result = clGetPlatformIDs(1, &_platform, &platform_count);
	//创建上下文
	cl_context_properties  propertys[3] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)_platform,
		0,
	};
	_context = clCreateContextFromType(propertys, CL_DEVICE_TYPE_GPU, nullptr, nullptr, &result);
	if (!_context || result != CL_SUCCESS)
	{
		_context = clCreateContextFromType(propertys, CL_DEVICE_TYPE_CPU, nullptr, nullptr, &result);
		_deviceType = CL_DEVICE_TYPE_CPU;
		if (!_context || result != CL_SUCCESS)
		{
			printf("clCreateContextFromType error,failed to create context.");
			_context = nullptr;
		}
	}
	CL_ASSERT(_context !=nullptr,"create cl context error.");
	//获取设备
	cl_uint  device_count = 0;
	clGetContextInfo(_context, CL_CONTEXT_DEVICES, 1, nullptr, &device_count);
	std::vector<cl_device_id>  devices(device_count);
	clGetContextInfo(_context, CL_CONTEXT_DEVICES, device_count, devices.data(), &device_count);
	_device = devices[0];
	CL_ASSERT(_device !=nullptr,"clGetContextInfo for device error.");
	/*
	  *创建命令队列
	 */
	cl_command_queue_properties   queue_property = 0;
	_commandQueue = clCreateCommandQueue(_context, _device, queue_property,&result);

	return true;
}

cl_program CSKContext::createCLProgram(const char *filename)
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
	cl_program  program = clCreateProgramWithSource(_context, 1, (const char **)&buffer, nullptr, nullptr);
	if (!program)
	{
		printf("failed to create program,file->%s\n", filename);
		return nullptr;
	}
	int result = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);
	if (result != CL_SUCCESS)
	{
		size_t  buffer_size = 0;
		clGetProgramBuildInfo(program, _device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &buffer_size);
		char  *buffer = new char[buffer_size + 2];
		clGetProgramBuildInfo(program, _device, CL_PROGRAM_BUILD_LOG, buffer_size, buffer, nullptr);
		buffer[buffer_size] = 0;
		printf("Compile error:%s\n", buffer);
		delete[] buffer;
		buffer = nullptr;
		clReleaseProgram(program);
		program = nullptr;
	}
	return program;
}