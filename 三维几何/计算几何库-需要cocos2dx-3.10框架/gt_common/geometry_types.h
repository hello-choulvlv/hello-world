/*
*��ά�������ݽṹ
*2019��5��29��
*@author:xiaohuaxiong
*/
#ifndef _GEOMETRY_TYPES_H__
#define _GEOMETRY_TYPES_H__
/*
  *���κ�����������ռ䶨��
 */
#define NS_GT_BEGIN namespace gt{
#define NS_GT_END     }

#define min_f(x,y)  ((x) < (y) ?(x):(y))
#define max_f(x,y)  ((x) > (y)?(x):(y))
/*
  *std::vector����׷��Ԫ��
 */
#define vector_fast_push_back(v,s) {if(v.size() >= v.capacity())v.reserve(v.size() * 2);v.push_back(s);}
#endif