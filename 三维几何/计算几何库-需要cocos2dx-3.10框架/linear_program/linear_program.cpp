/*
  *线性规划算法实现
  *2020年3月23日
  *@date:2020年3月23日
  *@author:xiaohuaxiong
 */
#include "linear_program/linear_program.h"
#include "matrix/matrix.h"
#include <math.h>
#include <functional>
#include <assert.h>
#include <map>
using namespace cocos2d ;

NS_GT_BEGIN

int linearly_program_2d(const std::vector<Line2D> &line_array, float coeff_array[3],std::vector<cocos2d::Vec2>&intersect_array2, cocos2d::Vec2 &intersect_point, cocos2d::Vec2 &direction) {
	//第一步,对所有的直线进行排序
	std::function<bool(const Line2D &, const Line2D&)> compare_func = [](const Line2D &a, const Line2D&b)->bool {
		float angle_a = atan2f(a.direction.y,a.direction.x);
		float angle_b = atan2f(b.direction.y,b.direction.x);

		return angle_a < angle_b || (angle_a == angle_b && -(a.start_point.x - b.start_point.x) * b.direction.y + (a.start_point.y - b.start_point.y) * b.direction.x > 0.0f);
	};

	int array_size = line_array.size();
	std::vector<Line2D>  other_line_array = line_array;
	quick_sort<Line2D>(other_line_array.data(),array_size,compare_func);

	//直线的交点集合
	std::vector<cocos2d::Vec2>	intersect_array(array_size + 1);
	std::vector<const Line2D*> line_queue(array_size);

	int left_idx = -1, right_idx = 0;
	line_queue[0] = other_line_array.data();

	for (int j = 1; j < array_size; ++j) {
		const Line2D &line = other_line_array[j];
		//如果交点在直线的右侧,则过滤掉相关的直线
		while (right_idx - left_idx > 1 && line_point_distance(line.start_point,line.direction,intersect_array[right_idx]) < 0.0f)
			--right_idx;
		while (right_idx - left_idx > 1 && line_point_distance(line.start_point, line.direction, intersect_array[left_idx + 2]) < 0.0f)
			++left_idx;
		//如果法线全部的交点都被过滤掉了,则此时必定无解
		if (right_idx == left_idx) return 0;
		line_queue[++right_idx] = &line;
		line_line_intersect_point(line, *line_queue[right_idx - 1], intersect_array[right_idx]);
	}
	//在最后,用队列开头的直线再一次剔除掉最后的交点
	while (right_idx - left_idx > 1 && line_point_distance(line_queue[left_idx+1]->start_point, line_queue[left_idx+1]->direction, intersect_array[right_idx]) < 0.0f)
		--right_idx;
	//此时只剩下一条直线,一个交点,无论如何也够不成一个区域
	if (right_idx - left_idx < 3)return 0;

	right_idx += 1;
	line_line_intersect_point(*line_queue[left_idx + 1] ,*line_queue[right_idx - 1],intersect_array[right_idx]);
	//计算最最终的可行解区域是否是有界的,并且如果存在边界条件,则给出相关的数值
	const Vec2 &coeff = *(Vec2 *)coeff_array;
	float max_value = -FLT_MAX,min_value = FLT_MAX;
	float boundary_x = coeff_array[0] > 0.0f?line_array[array_size - 3].start_point.x:line_array[array_size-4].start_point.x;
	float boundary_y = coeff_array[1] > 0.0f ? line_array[array_size - 2].start_point.y:line_array[array_size-4].start_point.y;

	int point_idx = -1,point_idx2 = -1;
	left_idx += 2;
	int cycle_size = right_idx - left_idx + 1;

	for (int j = left_idx; j <= right_idx; ++j) {
		const Vec2 &point = intersect_array[j];
		float f = dot(coeff,point);
		if (f > max_value){
			f = max_value;
			intersect_point = point;
			point_idx = j;
		}
		if (f < min_value) {
			min_value = f;
			point_idx2 = j;
		}
	}

	intersect_array2.resize(cycle_size);
	memcpy(intersect_array2.data(),intersect_array.data()+left_idx,sizeof(Vec2) * cycle_size);

	//检测是否处于无界状态,此时向两侧中的任一侧进行寻址,即可得出相关的目标射线
	if (intersect_point.x == boundary_x || intersect_point.y == boundary_y) {
		
		int next_idx = left_idx + (point_idx - left_idx +1)%cycle_size;
		int tripple_idx = left_idx + (next_idx - left_idx + 1)%cycle_size;

		intersect_point = intersect_array[point_idx2];
		direction = normalize(intersect_array[tripple_idx], intersect_array[next_idx]);
		return 2;
	}

	return 1;
}
//主元变换
void simplex_pivot_swap(std::vector<float*> &constraints, std::vector<float> &const_array, std::vector<float> &exp_array,std::map<int, int> &basic_variables, std::map<int, int> &nonebasic_variables, int row_idx,int swap_idx) {
	float *target_array = constraints[row_idx];
	float coef = 1.0f/target_array[swap_idx];
	int basic_idx = basic_variables[row_idx];
	int array_size = constraints.size();

	//需要更新常系数以及交换位置后基变量所对应的系数
	const_array[row_idx] *= coef;
	target_array[basic_idx] = coef;
	//代入其他的表达式
	for (auto it = nonebasic_variables.begin(); it != nonebasic_variables.end(); ++it) {
		int none_idx = it->first;
		target_array[none_idx] *= coef;
		//针对所有的组进行遍历
		for (int j = 0; j < array_size; ++j) {
			if (j == row_idx)continue;
			float *target_array2 = constraints[j];
			//首先针对系数,以及新增的另一个非基本变量
			if (none_idx != swap_idx) 
				target_array2[none_idx] -= target_array2[swap_idx] * target_array[none_idx];
			else {
				const_array[j] -= target_array2[none_idx] * const_array[row_idx];
				target_array2[basic_idx] = -target_array2[none_idx] * coef;
			}
		}
		if (none_idx != swap_idx)
			exp_array[none_idx] -= exp_array[swap_idx] * target_array[none_idx];
	}
	//针对目标表达式需要做出一番调整
	target_array[swap_idx] = 0.0f;
	exp_array[basic_idx] = -coef * exp_array[swap_idx];
	exp_array.back() += exp_array[swap_idx] * const_array[row_idx];
	exp_array[swap_idx] = 0.0f;
	//最后交换基变量与非基变量索引
	basic_variables[row_idx] = swap_idx;
	nonebasic_variables.erase(swap_idx);
	nonebasic_variables[basic_idx] = basic_idx;
}
//求解线性规划,松弛变量过程
//在当前的版本中,暂时不做优化
//在后续的版本中,我们将使用一些高级数据结构进行优化
SimplexType simplex_slack_variable(std::vector<float*> &constraints, std::vector<float> &const_array, std::vector<float> &exp_array,std::vector<float>&record_array, std::map<int, int> &basic_variables, std::map<int, int> &nonebasic_variables) {
	assert(basic_variables.size() + nonebasic_variables.size() + 1 == exp_array.size());
	assert(constraints.size() == const_array.size());
	while (true){
		int target_idx = -1;
		for (auto it = nonebasic_variables.begin(); it != nonebasic_variables.end(); ++it) {
			if (exp_array[it->first] > 0.0f && (target_idx == -1 || exp_array[it->first] > exp_array[target_idx]))
				target_idx = it->first;
		}
		if (target_idx == -1)
			break;
		float inf = FLT_MAX;
		int    row_idx = -1;
		for (int j = 0; j < constraints.size(); ++j) {
			float *array_ptr = constraints[j];
			if (array_ptr[target_idx] > 0.0f) {
				float f2 = const_array[j] / array_ptr[target_idx];
				if (inf > f2) {
					inf = f2;
					row_idx = j;
				}
			}
		}
		if (inf == FLT_MAX)
			return SimplexType::SimplexType_Unboundary;
		//选取新的主元
		simplex_pivot_swap(constraints, const_array, exp_array, basic_variables, nonebasic_variables, row_idx, target_idx);
	}
	//最后整理结果,所有的基变量的取值都是对应着相关行的常系数值,所有的非基变量都对应着0
	for (auto it = nonebasic_variables.begin(); it != nonebasic_variables.end(); ++it) {
		record_array[it->first] = 0.0f;
	}
	for (auto it = basic_variables.begin(); it != basic_variables.end(); ++it) {
		record_array[it->second] = const_array[it->first];
	}
	return SimplexType::SimplexType_Success;
}
/*
  *初始化单纯型算法
  *该算法或者返回可行,并给出一个经过初次迭代后的初始解
  *或者返回无解
 */
SimplexType simplex_initialize_program(std::vector<float*> &constraints, std::vector<float> &const_array, std::vector<float> &exp_array,std::vector<float> &record_array,std::map<int,int> &basic_variables,std::map<int,int> &nonebasic_variables) {
	assert(constraints.size() == const_array.size());
	assert(record_array.size() + 1 == exp_array.size());
	assert(nonebasic_variables.size() + basic_variables.size() == record_array.size());
	assert(basic_variables.size() == const_array.size());
	//第一步,遍历常系数
	float f = FLT_MAX;
	int swap_l = -1;
	int array_size = const_array.size();
	for (int j = 0; j < array_size; ++j) {
		if (const_array[j] < f) {
			f = const_array[j];
			swap_l = j;
		}
	}
	//如果最小的系数大于等于0,则可以直接返回
	if (f >= 0.0f)return SimplexType::SimplexType_Success;
	int variable_num = nonebasic_variables.size() + basic_variables.size();
	//否则,进行计算是否有可行解,增加另一个变量 Xn+1,求表达式f(x) = Xn+1的最优解
	std::vector<float*> other_constraints(constraints.size());
	std::vector<float>  other_exp_array(exp_array.size() +1);
	std::vector<float>  other_record_array(record_array.size() + 1);

	std::vector<float>  matrix_array(constraints.size() * other_record_array.size());
	for (int j = 0; j < other_constraints.size(); ++j) {
		other_constraints[j] = matrix_array.data() + j * other_record_array.size();
		memcpy(other_constraints[j],constraints[j],sizeof(float) * variable_num);
		other_constraints[j][variable_num] = -1.0f;
	}
	other_exp_array[variable_num] = -1.0f;
	nonebasic_variables[variable_num] = variable_num;
	//第一步,交换主元
	simplex_pivot_swap(other_constraints,const_array,other_exp_array,basic_variables,nonebasic_variables,swap_l, variable_num);
	//第二部求解
	SimplexType type_ref = simplex_slack_variable(other_constraints, const_array, other_exp_array, other_record_array, basic_variables, nonebasic_variables);
	assert(other_exp_array.back() <=0.0f);//注意有时因为浮点数的不稳定性,也有可能会稍大于0,此时需要使用者自己调整
	if (fabsf(other_exp_array.back()) > gt_eps)return SimplexType::SimplexType_Failure;
	//否则转换为原来的方程式
	for (int j = 0; j < other_constraints.size(); ++j) 
		memcpy(constraints[j],other_constraints[j],sizeof(float)*variable_num);
	//将原目标表达式中的基变量重新表达为非基变量
	nonebasic_variables.erase(variable_num);
	for (auto it = basic_variables.begin(); it != basic_variables.end(); ++it) {
		int row_idx = it->first;
		int basic_idx = it->second;//基变量的索引号
		float f = exp_array[basic_idx];
		assert(basic_idx != variable_num);
		if (f != 0.0f) {
			//首先更新常系数值
			exp_array.back() += f * const_array[row_idx];
			float *target_array = constraints[row_idx];
			//再更新每一个其他的非基变量前面的系数
			for (auto ot = nonebasic_variables.begin(); ot != nonebasic_variables.end(); ++ot) {
				exp_array[ot->first] -= f * target_array[ot->first];
			}
			//同时将该基变量前面的系数清零,因为后续的计算将依赖该系数的值
			exp_array[basic_idx] = 0.0f;
		}
	}
	return SimplexType::SimplexType_Success;
}

SimplexType simplex_linear_program(std::vector<float*> &constraints, std::vector<float> &const_array, std::vector<float> &exp_array,std::vector<float> &record_array) {
	assert(constraints.size() == const_array.size());
	assert(record_array.size() +1 == exp_array.size());//目标表达式数组的长度比实际参变量的数目大1,最后一个位置将存储最终的目标值
	//定义基本变量,非基本变量
	std::map<int,int>	basic_variables,nonebasic_variables;
	int array_size1 = record_array.size() - const_array.size();
	int array_size2 = const_array.size();
	for (int j = 0; j < array_size1; ++j)
		nonebasic_variables[j] = j;
	for (int j = 0; j < array_size2; ++j)
		basic_variables[j] = j+array_size1;

	SimplexType type_ref = simplex_initialize_program(constraints, const_array, exp_array, record_array, basic_variables, nonebasic_variables);
	if (type_ref != SimplexType::SimplexType_Success)
		return SimplexType::SimplexType_Failure;
	return simplex_slack_variable(constraints, const_array, exp_array, record_array, basic_variables, nonebasic_variables);
}
NS_GT_END