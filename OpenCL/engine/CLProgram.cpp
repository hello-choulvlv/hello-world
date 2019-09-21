/*
  *创建OpenCL程序对象
  *@Version:1.0
  *@date:2017-5-4
  *@Author:xiaohuaxiong
 */
#include<CLProgram.h>
#include<CLContext.h>
#include<CLKernel.h>
#include<Tools.h>
#include<stdio.h>
#include<assert.h>
_CLK_NS_BEGIN_
static char *_static_fileName = nullptr;
CLProgram::CLProgram()
{
	_clProgram = nullptr;
}

CLProgram::~CLProgram()
{
	//释放与程序对象关联的程序对象
	std::map<std::string, clk::CLKernel*>::iterator  it = _kernelCache.begin();
	for ( ; it != _kernelCache.end();++ it)
	{
		it->second->release();
		it->second = nullptr;
	}
	_kernelCache.clear();
	clReleaseProgram(_clProgram);
	_clProgram = nullptr;
}

void			CLProgram::initWithString(const char *src)
{
	int		errorCode = 0;
	CLContext   *clContext = CLContext::getInstance();
	_clProgram = clCreateProgramWithSource(clContext->getCLContext(),1,&src,NULL,&errorCode );
	assert(errorCode == CL_SUCCESS);
	//合成程序对象,暂时不设置编译器指示参数
	errorCode = clBuildProgram(_clProgram, clContext->getDeviceCount(), clContext->getDevices(), nullptr,nullptr, nullptr);
	//检查是否有错误
	if (errorCode != CL_SUCCESS)
	{
		if(_static_fileName)
			printf("Compile OpenCL file %s error.\n",_static_fileName);
		//获取错误信息
		unsigned bufferSize = 0;
		const int deviceCount = clContext->getDeviceCount();
		cl_device_id *deviceIds = clContext->getDevices();
		for (int i = 0; i < deviceCount; ++i)
		{
			clGetProgramBuildInfo(_clProgram, deviceIds[i], CL_PROGRAM_BUILD_LOG, 0, nullptr, &bufferSize);
			char *buffer = new char[bufferSize+2];
			clGetProgramBuildInfo(_clProgram, deviceIds[i], CL_PROGRAM_BUILD_LOG, bufferSize + 1, buffer, nullptr);
			buffer[bufferSize] = '\0';
			printf("On device id '%d error occur ,because:\n%s\n'",(int)deviceIds[i],buffer);
			delete[] buffer;
		}
		clReleaseProgram(_clProgram);
		_clProgram = nullptr;
		assert(errorCode==CL_SUCCESS);
	}
}

void     CLProgram::initWithFile(const char *filename)
{
	const char *fileSrc = Tools::getFileContent(filename);
	initWithString(fileSrc);
	delete[] fileSrc;
}

CLProgram *CLProgram::createWithString(const char *src)
{
	CLProgram *clProgram = new CLProgram();
	_static_fileName = nullptr;
	clProgram->initWithString(src);
	return clProgram;
}

CLProgram *CLProgram::createWithFile(const char *filename)
{
	CLProgram *clProgram = new CLProgram();
	_static_fileName = (char *)filename;
	clProgram->initWithFile(filename);
	_static_fileName = nullptr;
	return clProgram;
}

CLKernel *CLProgram::getCLKernel(const char *kernelName)
{
	std::string keyName(kernelName);
	//查找缓存对象,如果查找到了,直接返回,否则创建新的内核对象
	std::map<std::string,CLKernel*>::iterator it= _kernelCache.find(keyName);
	if (it != _kernelCache.end())
		return it->second;
	//创建内核对象
	CLKernel  *kernel = new CLKernel();
	kernel->initWithKernel(this,kernelName);
	_kernelCache[keyName] = kernel;

	return kernel;
}

cl_program CLProgram::getProgram()const
{
	return _clProgram;
}
_CLK_NS_END_
