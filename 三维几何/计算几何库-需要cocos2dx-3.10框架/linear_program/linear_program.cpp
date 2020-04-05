/*
  *���Թ滮�㷨ʵ��
  *2020��3��23��
  *@date:2020��3��23��
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
	//��һ��,�����е�ֱ�߽�������
	std::function<bool(const Line2D &, const Line2D&)> compare_func = [](const Line2D &a, const Line2D&b)->bool {
		float angle_a = atan2f(a.direction.y,a.direction.x);
		float angle_b = atan2f(b.direction.y,b.direction.x);

		return angle_a < angle_b || (angle_a == angle_b && -(a.start_point.x - b.start_point.x) * b.direction.y + (a.start_point.y - b.start_point.y) * b.direction.x > 0.0f);
	};

	int array_size = line_array.size();
	std::vector<Line2D>  other_line_array = line_array;
	quick_sort<Line2D>(other_line_array.data(),array_size,compare_func);

	//ֱ�ߵĽ��㼯��
	std::vector<cocos2d::Vec2>	intersect_array(array_size + 1);
	std::vector<const Line2D*> line_queue(array_size);

	int left_idx = -1, right_idx = 0;
	line_queue[0] = other_line_array.data();

	for (int j = 1; j < array_size; ++j) {
		const Line2D &line = other_line_array[j];
		//���������ֱ�ߵ��Ҳ�,����˵���ص�ֱ��
		while (right_idx - left_idx > 1 && line_point_distance(line.start_point,line.direction,intersect_array[right_idx]) < 0.0f)
			--right_idx;
		while (right_idx - left_idx > 1 && line_point_distance(line.start_point, line.direction, intersect_array[left_idx + 2]) < 0.0f)
			++left_idx;
		//�������ȫ���Ľ��㶼�����˵���,���ʱ�ض��޽�
		if (right_idx == left_idx) return 0;
		line_queue[++right_idx] = &line;
		line_line_intersect_point(line, *line_queue[right_idx - 1], intersect_array[right_idx]);
	}
	//�����,�ö��п�ͷ��ֱ����һ���޳������Ľ���
	while (right_idx - left_idx > 1 && line_point_distance(line_queue[left_idx+1]->start_point, line_queue[left_idx+1]->direction, intersect_array[right_idx]) < 0.0f)
		--right_idx;
	//��ʱֻʣ��һ��ֱ��,һ������,�������Ҳ������һ������
	if (right_idx - left_idx < 3)return 0;

	right_idx += 1;
	line_line_intersect_point(*line_queue[left_idx + 1] ,*line_queue[right_idx - 1],intersect_array[right_idx]);
	//���������յĿ��н������Ƿ����н��,����������ڱ߽�����,�������ص���ֵ
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

	//����Ƿ����޽�״̬,��ʱ�������е���һ�����Ѱַ,���ɵó���ص�Ŀ������
	if (intersect_point.x == boundary_x || intersect_point.y == boundary_y) {
		
		int next_idx = left_idx + (point_idx - left_idx +1)%cycle_size;
		int tripple_idx = left_idx + (next_idx - left_idx + 1)%cycle_size;

		intersect_point = intersect_array[point_idx2];
		direction = normalize(intersect_array[tripple_idx], intersect_array[next_idx]);
		return 2;
	}

	return 1;
}
//��Ԫ�任
void simplex_pivot_swap(std::vector<float*> &constraints, std::vector<float> &const_array, std::vector<float> &exp_array,std::map<int, int> &basic_variables, std::map<int, int> &nonebasic_variables, int row_idx,int swap_idx) {
	float *target_array = constraints[row_idx];
	float coef = 1.0f/target_array[swap_idx];
	int basic_idx = basic_variables[row_idx];
	int array_size = constraints.size();

	//��Ҫ���³�ϵ���Լ�����λ�ú����������Ӧ��ϵ��
	const_array[row_idx] *= coef;
	target_array[basic_idx] = coef;
	//���������ı��ʽ
	for (auto it = nonebasic_variables.begin(); it != nonebasic_variables.end(); ++it) {
		int none_idx = it->first;
		target_array[none_idx] *= coef;
		//������е�����б���
		for (int j = 0; j < array_size; ++j) {
			if (j == row_idx)continue;
			float *target_array2 = constraints[j];
			//�������ϵ��,�Լ���������һ���ǻ�������
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
	//���Ŀ����ʽ��Ҫ����һ������
	target_array[swap_idx] = 0.0f;
	exp_array[basic_idx] = -coef * exp_array[swap_idx];
	exp_array.back() += exp_array[swap_idx] * const_array[row_idx];
	exp_array[swap_idx] = 0.0f;
	//��󽻻���������ǻ���������
	basic_variables[row_idx] = swap_idx;
	nonebasic_variables.erase(swap_idx);
	nonebasic_variables[basic_idx] = basic_idx;
}
//������Թ滮,�ɳڱ�������
//�ڵ�ǰ�İ汾��,��ʱ�����Ż�
//�ں����İ汾��,���ǽ�ʹ��һЩ�߼����ݽṹ�����Ż�
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
		//ѡȡ�µ���Ԫ
		simplex_pivot_swap(constraints, const_array, exp_array, basic_variables, nonebasic_variables, row_idx, target_idx);
	}
	//���������,���еĻ�������ȡֵ���Ƕ�Ӧ������еĳ�ϵ��ֵ,���еķǻ���������Ӧ��0
	for (auto it = nonebasic_variables.begin(); it != nonebasic_variables.end(); ++it) {
		record_array[it->first] = 0.0f;
	}
	for (auto it = basic_variables.begin(); it != basic_variables.end(); ++it) {
		record_array[it->second] = const_array[it->first];
	}
	return SimplexType::SimplexType_Success;
}
/*
  *��ʼ���������㷨
  *���㷨���߷��ؿ���,������һ���������ε�����ĳ�ʼ��
  *���߷����޽�
 */
SimplexType simplex_initialize_program(std::vector<float*> &constraints, std::vector<float> &const_array, std::vector<float> &exp_array,std::vector<float> &record_array,std::map<int,int> &basic_variables,std::map<int,int> &nonebasic_variables) {
	assert(constraints.size() == const_array.size());
	assert(record_array.size() + 1 == exp_array.size());
	assert(nonebasic_variables.size() + basic_variables.size() == record_array.size());
	assert(basic_variables.size() == const_array.size());
	//��һ��,������ϵ��
	float f = FLT_MAX;
	int swap_l = -1;
	int array_size = const_array.size();
	for (int j = 0; j < array_size; ++j) {
		if (const_array[j] < f) {
			f = const_array[j];
			swap_l = j;
		}
	}
	//�����С��ϵ�����ڵ���0,�����ֱ�ӷ���
	if (f >= 0.0f)return SimplexType::SimplexType_Success;
	int variable_num = nonebasic_variables.size() + basic_variables.size();
	//����,���м����Ƿ��п��н�,������һ������ Xn+1,����ʽf(x) = Xn+1�����Ž�
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
	//��һ��,������Ԫ
	simplex_pivot_swap(other_constraints,const_array,other_exp_array,basic_variables,nonebasic_variables,swap_l, variable_num);
	//�ڶ������
	SimplexType type_ref = simplex_slack_variable(other_constraints, const_array, other_exp_array, other_record_array, basic_variables, nonebasic_variables);
	assert(other_exp_array.back() <=0.0f);//ע����ʱ��Ϊ�������Ĳ��ȶ���,Ҳ�п��ܻ��Դ���0,��ʱ��Ҫʹ�����Լ�����
	if (fabsf(other_exp_array.back()) > gt_eps)return SimplexType::SimplexType_Failure;
	//����ת��Ϊԭ���ķ���ʽ
	for (int j = 0; j < other_constraints.size(); ++j) 
		memcpy(constraints[j],other_constraints[j],sizeof(float)*variable_num);
	//��ԭĿ����ʽ�еĻ��������±��Ϊ�ǻ�����
	nonebasic_variables.erase(variable_num);
	for (auto it = basic_variables.begin(); it != basic_variables.end(); ++it) {
		int row_idx = it->first;
		int basic_idx = it->second;//��������������
		float f = exp_array[basic_idx];
		assert(basic_idx != variable_num);
		if (f != 0.0f) {
			//���ȸ��³�ϵ��ֵ
			exp_array.back() += f * const_array[row_idx];
			float *target_array = constraints[row_idx];
			//�ٸ���ÿһ�������ķǻ�����ǰ���ϵ��
			for (auto ot = nonebasic_variables.begin(); ot != nonebasic_variables.end(); ++ot) {
				exp_array[ot->first] -= f * target_array[ot->first];
			}
			//ͬʱ���û�����ǰ���ϵ������,��Ϊ�����ļ��㽫������ϵ����ֵ
			exp_array[basic_idx] = 0.0f;
		}
	}
	return SimplexType::SimplexType_Success;
}

SimplexType simplex_linear_program(std::vector<float*> &constraints, std::vector<float> &const_array, std::vector<float> &exp_array,std::vector<float> &record_array) {
	assert(constraints.size() == const_array.size());
	assert(record_array.size() +1 == exp_array.size());//Ŀ����ʽ����ĳ��ȱ�ʵ�ʲα�������Ŀ��1,���һ��λ�ý��洢���յ�Ŀ��ֵ
	//�����������,�ǻ�������
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