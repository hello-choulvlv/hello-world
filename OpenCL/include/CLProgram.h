/*
  *OpenCL程序对象
  *@date:2017-5-4
  *@Version:1.0 实现了最基本的程序对象创建链接,错误信息输出
  *@Version:2.0 实现了与内核对象之间的关联
  *@Author:xiaohuaxiong
*/
#ifndef __CL_PROGRAM_H__
#define __CL_PROGRAM_H__
#include<clMicros.h>
#include<Object.h>
#include<CL/Opencl.h>
#include<map>
#include<string>
_CLK_NS_BEGIN_
class CLKernel;
class CLProgram :public Object
{
private:
	cl_program            _clProgram;
	/*
	  *存储内核对象的缓存对象,此对象与程序对象关联,并且动态生成
	  *也就是只有第一次调用生成函数的时候才会创建
	*/
	std::map<std::string, clk::CLKernel*>   _kernelCache;
private:
	CLProgram();
	CLProgram(CLProgram &);
	void				initWithString(const char *src);
	void             initWithFile(const char *filename);
public:
	~CLProgram();
	/*
	  *用给定的程序源代码创建OpenCL程序对象
	 */
	static	CLProgram    *createWithString(const char *src);
	/*
	  *用给定的文件名创建对象
	 */
	static   CLProgram    *createWithFile(const char *filename);
	/*
	 *返回与程序对象关联的内核对象
	 *目前在这个版本中我们不使用缓存对象
	 *在下一个版本中，我们会将这个类完善
	 */
	CLKernel        *getCLKernel(const char *kernelName);
	/*
	  *获取程序对象
	  */
	cl_program     getProgram()const;
};

_CLK_NS_END_
#endif