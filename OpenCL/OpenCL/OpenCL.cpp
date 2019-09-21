// OpenCL.cpp : 定义控制台应用程序的入口点。
//
#include<stdio.h>
#include<stdlib.h>
#include<fstream>
#include<sstream>
#include<string>
#include<assert.h>
#include<CL/OpenCL.h>
#include<CLContext.h>
#include<CLProgram.h>
#include<CLKernel.h>
// Constants
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

void checkError(const int errorCode,const char *errorMessage)
{
	if (errorCode != CL_SUCCESS)
	{
		printf("%s\n",errorMessage);
		exit(-1);
	}
}

int main()
{
	int								errorCode;
	cl_command_queue clQueue;
	cl_mem                       inputBuffer;
	cl_mem                       outputBuffer;
	cl_mem                       maskBuffer;
	
	clk::CLContext   *clContext = clk::CLContext::getInstance();
	clk::CLProgram  *clProgram = clk::CLProgram::createWithFile("Convolution.cl");
	//创建内核对象
	clk::CLKernel	*clKernel = clProgram->getCLKernel("convolve");
	//创建内存对象
	inputBuffer = clCreateBuffer(clContext->getCLContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int)*inputSignalWidth*inputSignalHeight,inputSignal,&errorCode);
	checkError(errorCode, "clCreateBuffer(inputBuffer)");
	outputBuffer = clCreateBuffer(clContext->getCLContext(), CL_MEM_WRITE_ONLY, sizeof(int)*outputSignalWidth*outputSignalHeight,NULL,&errorCode);
	checkError(errorCode, "clCreateBuffer(outputBuffer)");
	maskBuffer = clCreateBuffer(clContext->getCLContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int)*maskWidth*maskHeight,mask,&errorCode);
	checkError(errorCode, "clCreateBuffer(maskBuffer)");
	//创建命令队列
	clQueue = clContext->getCommandQueue();
	checkError(errorCode, "clCreateCommandQueue");
	//设置内核参数
	clKernel->setKernelArgument(0,sizeof(cl_mem),&inputBuffer);
	clKernel->setKernelArgument(1, sizeof(cl_mem), &maskBuffer);
	clKernel->setKernelArgument(2, sizeof(cl_mem),&outputBuffer);
	clKernel->setKernelArgument(3, sizeof(int),&inputSignalWidth);
	clKernel->setKernelArgument(4, sizeof(int),&outputSignalHeight);
	//分发事件
	const size_t globalWorkSize[1] = {outputSignalWidth * outputSignalHeight};
	const size_t localWorkSize[1] = {1};
	errorCode = clEnqueueNDRangeKernel(clQueue, clKernel->getKernel(), 1,NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
	checkError(errorCode, "clEnqueueNDRangeKernel");
	//读取数据
	errorCode = clEnqueueReadBuffer(clQueue, outputBuffer, CL_TRUE, 0, sizeof(int)*outputSignalWidth*outputSignalHeight,outputSignal,0,NULL,NULL);
	checkError(errorCode, "clEnqueueReadBuffer");
	for (int i = 0; i < outputSignalHeight; ++i)
	{
		for (int j = 0; j < outputSignalWidth; ++j)
		{
			printf("%d  ", outputSignal[i][j]);
		}
		printf("\n");
	}
	//
	getchar();
	delete clContext;
	clProgram->release();
    return 0;
}

