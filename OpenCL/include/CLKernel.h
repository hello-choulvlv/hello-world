/*
  *OpenCL�ں˶���,ע��ö���ֻ���ɳ�����󵼳���������ֱ�Ӵ���
  *���ļ��е���������������Ҫ����֮һ
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
  *��Ϊ�����еĺ�����֮һ,�ں˶������ӳ�������е���
  *�����䴫�ݲ���ʱ����ӵ���㹻�������,���ұ���ʹ��
 */
class CLKernel :public Object
{
	friend class clk::CLProgram;
private:
	cl_kernel        _kernel;//�ں˶���
	CLProgram    *_parentCLProgram;//���ں˶���������ĳ������
private:
	CLKernel();
	CLKernel(CLKernel &);
	/*
	  *�����ں˵ĺ�����,��ʼ���ں˶���
	 */
	void           initWithKernel(CLProgram *targetCLProgram, const char *kernelName);
public:
	~CLKernel();
	/*
	  *���ں˶����д��ݲ���
	  *@param:argIndex������λ������
	  *@param:typeSize�������͵Ĵ�С
	  *@param:value��������ֵ
	  */
	void          setKernelArgument(const int argIndex,const int typeSize,const void *value);
	/*
	  *�������ں˶���������ĳ������
	 */
	CLProgram   *getCLProgram()const;
	/*
	  *����OpenCL�ں˶���
	 */
	cl_kernel         getKernel()const;
};
_CLK_NS_END_

#endif