/*
  *csk�ļ�ͷ
  *2019��2��9��
  *@author:xiaohuaxiong
 */
#ifndef __CSK_TYPES_H__
#define __CSK_TYPES_H__
#include<stdio.h>
#include<assert.h>
#include<CL/cl.h>

#define CL_ASSERT(condition,error_message)   if(!(condition))\
{\
	printf("function %s,line %d,%s-->%s\n",__FUNCTION__,__LINE__,#condition,error_message);\
	assert(false);\
}
/*
  *�����������
 */
cl_program   create_program(cl_context context, cl_device_id device, const char *filename);
/*
  *����ͼ������,������CL�����ڴ����
 */
cl_mem  create_texture(cl_context  context,const char *filename,int  *width,int *height);
/*
  *����ͼ������
 */
void  save_texture(cl_context  context,const char *buffer, const char *filename, int width, int height);
#endif