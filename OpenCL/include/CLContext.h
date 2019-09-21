/*
  *OpenCL的上下文,管理平台,设备,以及平台的上下文的创建
  *@date:2017-5-3
  *@Version:1.0
  *@Author:xiaohuaxiong
 */
#ifndef __CL_CONTEXT_H__
#define __CL_CONTEXT_H__
#ifndef __APPLE__
	#include<CL/Opencl.h>
#else
	#include<OpenCL/OpenCL.h>
#endif
#include<clMicros.h>
_CLK_NS_BEGIN_
class CLContext
{
private:
	static CLContext		*_static_clContext;
	//OpenCL上下文
	cl_context					 _clContext;
	//命令队列
	cl_command_queue   _clQueue;
	//设备
	cl_device_id                *_clDevice;
	unsigned                        _clDeviceCount;
private:
	CLContext(CLContext &);
	CLContext();
	void							initCLContext();
public:
	~CLContext();
	//全局只有一个上下文
	static CLContext   *getInstance();
	//获取上下文
	cl_context				getCLContext()const;
	//获取设备id,设备的数目
	cl_device_id			*getDevices()const;
	//
	int							getDeviceCount()const;
	/*
	  *获取与第一个设备有关的命令队列
	 */
	cl_command_queue   getCommandQueue()const;
};
_CLK_NS_END_
#endif