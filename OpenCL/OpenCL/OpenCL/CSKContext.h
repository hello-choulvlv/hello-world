/*
  *OpenCL������
  *2019��2��9��
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
	//�豸������
	cl_int                   _deviceType;

	CSKContext();
	~CSKContext();

	bool   init();
	/*
	  *�����������
	 */
	cl_program  createCLProgram(const char *filename);
};

#endif