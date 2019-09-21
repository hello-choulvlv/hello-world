/*
  *OpenCL上下文
  *2019年2月9日
  *@author:xiaohuaxiong
 */
#ifndef __CSK_CONTEXT_H__
#define __CSK_CONTEXT_H__
#include<CL/cl.h>

struct  CSKContext
{
	cl_platform_id  _platform;
	cl_context           _context;
	cl_device_id       _device;
	cl_command_queue  _commandQueue;
	//设备的类型
	cl_int                   _deviceType;

	CSKContext();
	~CSKContext();

	bool   init();
	/*
	  *创建程序对象
	 */
	cl_program  createCLProgram(const char *filename);
};

#endif