/*
  *OpenCL�������
  *@date:2017-5-4
  *@Version:1.0 ʵ����������ĳ�����󴴽�����,������Ϣ���
  *@Version:2.0 ʵ�������ں˶���֮��Ĺ���
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
	  *�洢�ں˶���Ļ������,�˶��������������,���Ҷ�̬����
	  *Ҳ����ֻ�е�һ�ε������ɺ�����ʱ��Żᴴ��
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
	  *�ø����ĳ���Դ���봴��OpenCL�������
	 */
	static	CLProgram    *createWithString(const char *src);
	/*
	  *�ø������ļ�����������
	 */
	static   CLProgram    *createWithFile(const char *filename);
	/*
	 *������������������ں˶���
	 *Ŀǰ������汾�����ǲ�ʹ�û������
	 *����һ���汾�У����ǻὫ���������
	 */
	CLKernel        *getCLKernel(const char *kernelName);
	/*
	  *��ȡ�������
	  */
	cl_program     getProgram()const;
};

_CLK_NS_END_
#endif