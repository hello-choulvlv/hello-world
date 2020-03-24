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

NS_GT_END