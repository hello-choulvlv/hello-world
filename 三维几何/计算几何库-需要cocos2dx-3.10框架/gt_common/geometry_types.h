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
//#define vector_reverse(v){for(int left_l = 0,right_l = v.size()-1;left_l < right_l ;++left_l,--right_l)std::swap(v[left_l],v[right_l]);}
#define vector_fast_push_front(v,s) {if(v.size() >= v.capacity())v.reserve(v.size() * 2);v.insert(v.begin(),s);}
#define vector_fast_insert(param_v,position,other_v,start_l,end_l) {if(v.size() + (end_l - start_l) >= param_v.capacity())param_v.reserve(param_v.size() + (end_l - start_l));param_v.insert(paraam_v.begin() + position,other_v.begin() + start_l,other_v.begin() + end_l);}

/*
  *���ĳһЩ���ݽṹ�Ŀ���Ѱַʱʹ��
 */
#ifndef __offsetof
#define __offsetof(s,m) (long)((char *)&((s *)nullptr)->m)
#endif

#endif