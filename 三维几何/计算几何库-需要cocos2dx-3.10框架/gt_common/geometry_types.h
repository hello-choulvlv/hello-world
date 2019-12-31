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
//#define vector_reverse(v){for(int left_l = 0,right_l = v.size()-1;left_l < right_l ;++left_l,--right_l)std::swap(v[left_l],v[right_l]);}
#define vector_fast_push_front(v,s) {if(v.size() >= v.capacity())v.reserve(v.size() * 2);v.insert(v.begin(),s);}
#define vector_fast_insert(param_v,position,other_v,start_l,end_l) {if(v.size() + (end_l - start_l) >= param_v.capacity())param_v.reserve(param_v.size() + (end_l - start_l));param_v.insert(paraam_v.begin() + position,other_v.begin() + start_l,other_v.begin() + end_l);}

/*
  *针对某一些数据结构的快速寻址时使用
 */
#ifndef __offsetof
#define __offsetof(s,m) (long)((char *)&((s *)nullptr)->m)
#endif

#endif