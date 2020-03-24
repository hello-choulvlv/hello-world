/*
  *线性规划算法
  *2020年3月23日 
  *@author:xiaohuaxiong
  *@@version:1.0实现基本的二维线性规划
 */
#ifndef __linear_program_h__
#define __linear_program_h__
#include  "math/Vec2.h"
#include <vector>
#include "gt_common/geometry_types.h"
#include "line/line.h"

NS_GT_BEGIN
/*
*二维线性规划问题,函数约定,直线的左侧为半平面区域
*函数已经假定,给出的直线边界条件中已经给出了最大区域边界框,也就是由四条直线限定的边界区域
*并且其在输入数据的最后,顺序为下/右/上/左
*并且没有重复的输入,不过可以允许两条直线有着相同的方向
*target_coeff:目标函数的系数
*如果找到最大值,则返回1,并给出最有解
*如果无界,返回值为2且返回相关的无限递增射线
*否则返回0
*本算法采用S&I算法思想
*/
int linearly_program_2d(const std::vector<Line2D> &lines, float coeff_array[3], std::vector<cocos2d::Vec2>&intersect_array2, cocos2d::Vec2 &interect_point, cocos2d::Vec2 &direction);

NS_GT_END
#endif