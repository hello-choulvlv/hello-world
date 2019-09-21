/*
  *OpenCL内核对象,注意该对象只能由程序对象导出，而不能直接创建
  *此文件中的类是引擎中最重要的类之一
  *@date:2017-5-4
  *@Version:1.0
  *@Author:xiaohuaxiong
  */
#ifndef __CL_KERNEL_H__
#define __CL_KERNEL_H__
#include<clMicros.h>
#include<Object.h>
#include<CLProgram.h>
_CLK_NS_BEGIN_
/*
  *作为引擎中的核心类之一,内核对象必须从程序对象中导出
  *并且其传递参数时必须拥有足够的灵活性,而且便于使用
 */
class CLKernel :public Object
{
	friend class clk::CLProgram;
private:
	cl_kernel        _kernel;//内核对象
	CLProgram    *_parentCLProgram;//与内核对象相关联的程序对象
private:
	CLKernel();
	CLKernel(CLKernel &);
	/*
	  *给定内核的函数名,初始化内核对象
	 */
	void           initWithKernel(CLProgram *targetCLProgram, const char *kernelName);
public:
	~CLKernel();
	/*
	  *向内核对象中传递参数
	  *@param:argIndex参数的位置索引
	  *@param:typeSize参数类型的大小
	  *@param:value参数的数值
	  */
	void          setKernelArgument(const int argIndex,const int typeSize,const void *value);
	/*
	  *返回与内核对象相关联的程序对象
	 */
	CLProgram   *getCLProgram()const;
	/*
	  *返回OpenCL内核对象
	 */
	cl_kernel         getKernel()const;
};
_CLK_NS_END_

#endif