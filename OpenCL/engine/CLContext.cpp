/*
  *OpenCL上下文
  *@date:2017-5-3
  @Version:1.0
  @Author:xiaohuaxiong
 */
#include<stdio.h>
#include<assert.h>
#include<CLContext.h>
_CLK_NS_BEGIN_
CLContext     *CLContext::_static_clContext = nullptr;
CLContext::CLContext()
{
	_clContext = nullptr;
	_clQueue = nullptr;
	_clDevice = nullptr;
	_clDeviceCount = 0;
}

CLContext::~CLContext()
{
	clReleaseCommandQueue(_clQueue);
	clReleaseContext(_clContext);
	delete[] _clDevice;

	_clQueue = nullptr;
	_clContext = nullptr;
	_static_clContext = nullptr;
	_clDevice = nullptr;
	_clDeviceCount = 0;
}

CLContext *CLContext::getInstance()
{
	if (!_static_clContext)
	{
		_static_clContext = new CLContext();
		_static_clContext->initCLContext();
	}
	return _static_clContext;
}

static void CL_CALLBACK __static_clContextCallback(const char *errorMessage, const void *privateMessage, size_t callbackBlock, void *userData)
{
	printf("Error occurred during cl context use,%s", errorMessage);
}

void CLContext::initCLContext()
{
	int						 errorCode = 0;
	unsigned  			 platformCount=0;
	cl_platform_id   *platformIds = nullptr;
	//获取平台的数目
	errorCode = clGetPlatformIDs(0, nullptr, &platformCount);
	assert(errorCode == CL_SUCCESS && platformCount>0);
	platformIds = new cl_platform_id[platformCount];
	//
	errorCode = clGetPlatformIDs(platformCount, platformIds, nullptr);
	assert(errorCode==CL_SUCCESS);
	//
	int index = 0;
	
	for (; index < platformCount; ++index)
	{
	//如果定义了这个宏,优先使用GPU
#ifdef _PLATFORM_GPU_PRIORITY_
		errorCode = clGetDeviceIDs(platformIds[index], CL_DEVICE_TYPE_GPU, 0,nullptr,&_clDeviceCount);
#else
		errorCode = clGetDeviceIDs(platformIds[index], CL_DEVICE_TYPE_CPU, 0, nullptr, &_clDeviceCount);
#endif
		assert(errorCode==CL_SUCCESS && _clDeviceCount>0);
		_clDevice = new cl_device_id[_clDeviceCount];
#ifdef _PLATFORM_GPU_PRIORITY_
		errorCode = clGetDeviceIDs(platformIds[index], CL_DEVICE_TYPE_GPU, _clDeviceCount, _clDevice, nullptr);
#else
		errorCode = clGetDeviceIDs(platformIds[index], CL_DEVICE_TYPE_CPU, _clDeviceCount, _clDevice, nullptr);
#endif
		break;
	}
	cl_context_properties  clPriority[] = {CL_CONTEXT_PLATFORM,(int)platformIds[index],0};
	_clContext = clCreateContext(clPriority, _clDeviceCount,_clDevice, __static_clContextCallback,nullptr,&errorCode);
	assert(errorCode == CL_SUCCESS);
	//创建队列
	_clQueue = clCreateCommandQueue(_clContext, *_clDevice, 0, &errorCode);
	assert(errorCode==CL_SUCCESS);
	//
	delete[] platformIds;
	platformIds = nullptr;
}

cl_context CLContext::getCLContext()const
{
	return _clContext;
}

cl_device_id *CLContext::getDevices()const
{
	return _clDevice;
}

int CLContext::getDeviceCount()const
{
	return _clDeviceCount;
}

cl_command_queue  CLContext::getCommandQueue()const
{
	return _clQueue;
}

_CLK_NS_END_