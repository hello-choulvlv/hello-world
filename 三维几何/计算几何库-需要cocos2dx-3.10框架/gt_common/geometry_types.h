/*
*三维几何数据结构
*2019年5月29日
*@author:xiaohuaxiong
*/
#ifndef _GEOMETRY_TYPES_H__
#define _GEOMETRY_TYPES_H__
/*
  *几何函数库的命名空间定义
 */
#define NS_GT_BEGIN namespace gt{
#define NS_GT_END     }

#define min_f(x,y)  ((x) < (y) ?(x):(y))
#define max_f(x,y)  ((x) > (y)?(x):(y))
/*
  *std::vector快速追加元素
 */
#define vector_fast_push_back(v,s) {if(v.size() >= v.capacity())v.reserve(v.size() * 2);v.push_back(s);}
#endif