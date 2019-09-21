/*
  *csk文件头
  *2019年2月9日
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
  *创建程序对象
 */
cl_program   create_program(cl_context context, cl_device_id device, const char *filename);
/*
  *加载图像数据,并创建CL纹理内存对象
 */
cl_mem  create_texture(cl_context  context,const char *filename,int  *width,int *height);
/*
  *保存图像数据
 */
void  save_texture(cl_context  context,const char *buffer, const char *filename, int width, int height);
#endif