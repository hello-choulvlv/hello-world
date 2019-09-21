/*
  *内核对象实现
  *@date:2017-5-4
  *@Version:1.0
  *@Author:xiaohuaxiong
 */
#include<CLKernel.h>
#include<stdio.h>
#include<assert.h>
_CLK_NS_BEGIN_

CLKernel::CLKernel()
{
	_kernel = nullptr;
	_parentCLProgram = nullptr;
}

CLKernel::~CLKernel()
{
	clReleaseKernel(_kernel);
	_kernel = nullptr;
	_parentCLProgram = nullptr;
}

void CLKernel::initWithKernel(CLProgram *targetCLProgram, const char *kernelName)
{
	_parentCLProgram = targetCLProgram;
	int errorCode = 0;
	_kernel =  clCreateKernel(targetCLProgram->getProgram(), kernelName, &errorCode);
	assert(errorCode==CL_SUCCESS);
}

void CLKernel::setKernelArgument(const int argIndex,const int typeSize,const void *value)
{
	assert(argIndex>=0 && typeSize>0);
	int errorCode = clSetKernelArg(_kernel, argIndex, typeSize, value);
	assert(errorCode==CL_SUCCESS);
}

CLProgram *CLKernel::getCLProgram()const
{
	return _parentCLProgram;
}

cl_kernel      CLKernel::getKernel()const
{
	return _kernel;
}
_CLK_NS_END_