/*
  *引擎需要用到的宏
  *@date:2017-5-3
  *@Version:1.0
  *@Author:xiaohuaxiong
  */
#ifndef __CL_MICROS_H__
#define __CL_MICROS_H__
//是否第一优先选择GPU
#define _PLATFORM_GPU_PRIORITY_ 
//创建OpenCL程序对象的时候是否启用缓存
#define _USE_CLPROGRAM_CACHE_
//引擎的命名空间
#define _CLK_NS_BEGIN_   namespace clk{
#define _CLK_NS_END_      }
#endif