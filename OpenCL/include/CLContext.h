/*
  *OpenCL��������,����ƽ̨,�豸,�Լ�ƽ̨�������ĵĴ���
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
	//OpenCL������
	cl_context					 _clContext;
	//�������
	cl_command_queue   _clQueue;
	//�豸
	cl_device_id                *_clDevice;
	unsigned                        _clDeviceCount;
private:
	CLContext(CLContext &);
	CLContext();
	void							initCLContext();
public:
	~CLContext();
	//ȫ��ֻ��һ��������
	static CLContext   *getInstance();
	//��ȡ������
	cl_context				getCLContext()const;
	//��ȡ�豸id,�豸����Ŀ
	cl_device_id			*getDevices()const;
	//
	int							getDeviceCount()const;
	/*
	  *��ȡ���һ���豸�йص��������
	 */
	cl_command_queue   getCommandQueue()const;
};
_CLK_NS_END_
#endif