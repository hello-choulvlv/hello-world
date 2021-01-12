/*
  *2021��1��8��
  *gjk+chung-wang+sseָ���Ż��㷨ʵ��
  *@author:xiaohuaxiong
 */
#include "polygon_mix.h"
#include "matrix/matrix.h"

NS_GT_BEGIN
/*
  *�����һ������������,�����ֵ��
 */
const cocos2d::Vec2&  gjk_compute_support_point(const std::vector<cocos2d::Vec2> &polygon,const cocos2d::Vec2 &direction) {
	int target_j = 0;
	float f3 = -FLT_MAX;
	//��ǰʹ��˳�����,�Ժ�ʹ�ö��ַ�����
	for (int j = 0; j < polygon.size(); ++j) {
		float f4 = dot(polygon[j], direction);
		if (f4 > f3) {
			f3 = f4;
			target_j = j;
		}
	}
	return polygon[target_j];
}

bool gjk_minimum_simplex(cocos2d::Vec2 simplex_array[4],int &simplex_count,cocos2d::Vec2 &direction) {
	if (simplex_count == 1) {
		direction = -simplex_array[0];
		return false;
	}
	//��������ĿΪ2��ʱ��,��ʱ�ļ�������Ϊ����
	if (simplex_count == 2) {
		float d_x = -simplex_array[1].y + simplex_array[0].y;
		float d_y = simplex_array[1].x - simplex_array[0].x;
		//�ж�ԭ���뵱ǰ�߶�֮���λ�ù�ϵ,ԭ������˵�����ֹ�ϵ,�����Ѿ����������Ϊ͹�����,��˶�������ֿ��Բ����ж�
		direction.x = d_x; 
		direction.y = d_y;
		if (simplex_array[0].x * d_x + simplex_array[0].y * d_y > 0.0f) {//�Ҳ�
			direction.x = -d_x;
			direction.y = -d_y;
		}
		return false;
	}
	assert(simplex_count == 3);
	//������ĿΪ3��ʱ��,��Ҫ�ж�ԭ���Ƿ�λ�ڵ�����֮��
	float f1 = cross(simplex_array[0],simplex_array[1]);
	float f2 = cross(simplex_array[1],simplex_array[2]);
	float f3 = cross(simplex_array[2],simplex_array[0]);
	if (f1 * f2 >= 0.0f && f2 * f3 >= 0.0f)
		return true;
	//������Ҫ�����ǰ���������ԭ�����С����������
	float f4 = cross(simplex_array[0],simplex_array[1],simplex_array[2]);
	if (f4 < 0.0f) {
		cocos2d::Vec2 t2 = simplex_array[1];
		simplex_array[1] = simplex_array[2];
		simplex_array[2] = t2;
	}
	cocos2d::Vec2 &a = simplex_array[0], &b = simplex_array[1], &c = simplex_array[2];
	//A
	cocos2d::Vec2 ab = b - a;
	cocos2d::Vec2 ac = c - a;
	float fab = -dot(a,ab);
	float fac = -dot(a,ac);
	if (fab < 0.0f && fac < 0.0f) {
		simplex_count = 1;
		direction = -a;
		return false;
	}
	//B
	cocos2d::Vec2 bc = c - b;
	float fbc = -dot(b,bc);
	float fba = dot(b,ab);
	if (fba < 0.0f && fbc < 0.0f) {
		simplex_count = 1;
		simplex_array[0] = b;
		direction = -b;
		return false;
	}
	//C
	float fcb = dot(c,bc);
	float fca = dot(c,ac);
	if (fca < 0.0f && fcb < 0.0f) {
		simplex_count = 1;
		simplex_array[0] = c;
		direction = -c;
		return false;
	}
	//AC
	if (fca > 0.0f && fac > 0.0f && cross(ac,a) < 0.0f) {
		simplex_count = 2;
		simplex_array[1] = c;
		direction.x = -ac.y;
		direction.y = ac.x;
		return false;
	}
	//AB
	if (fab > 0.0f && fba > 0.0f && cross(ab,a) > 0.0f) {
		simplex_count = 2;
		direction.x = ab.y;
		direction.y = -ab.x;
		return false;
	}
	//BC
	if (fcb > 0.0f && fbc > 0.0f && cross(bc,b) > 0.0f) {
		simplex_array[0] = simplex_array[1];
		simplex_array[1] = simplex_array[2];
		simplex_count = 2;
		direction.x = bc.y;
		direction.y = -bc.x;
		return false;
	}
	assert(false);
	return false;
}

bool gjk_algorithm_optimal(const std::vector<cocos2d::Vec2> &polygon1, const std::vector<cocos2d::Vec2> &polygon2, cocos2d::Vec2 near_points[2]) {
	cocos2d::Vec2 simplex[4],direction(randomf10(),randomf10());
	int simplex_count = 0;

	const cocos2d::Vec2 *polygon1_ptr = polygon1.data();
	const cocos2d::Vec2 *polygon2_ptr = polygon2.data();

	simplex[0] = gjk_compute_support_point(polygon1, direction) - gjk_compute_support_point(polygon2, -direction);
	simplex_count = 1;
	direction = -simplex[0];

	int loop_count = polygon1.size() + polygon2.size();
	int j = 0;
	for (j = 0; j < loop_count; ++j) {
		//���Ŀ�귽��
		cocos2d::Vec2 s0 = gjk_compute_support_point(polygon1, direction) - gjk_compute_support_point(polygon2, -direction);
		if (dot(s0, direction) < 0.0f)
			return false;
		//������s0���뵽�����μ�����,����������С����
		simplex[simplex_count++] = s0;
		//��ԭ�������ϵ����μ���֮��Ĺ�ϵ,�Լ���һ�ַ��ߵķ���,��С�����μ���
		if (gjk_minimum_simplex(simplex, simplex_count, direction))
			return true;
	}
	CCLOG("loops time-->%d",j);
	return true;
}

void  cw_compute_support_point(const std::vector<cocos2d::Vec2> &polygon, const cocos2d::Vec2 &direction,cocos2d::Vec2 &max_point,cocos2d::Vec2 &min_point) {
	float f3 = -FLT_MAX;
	float f5 = FLT_MAX;
	//��ǰʹ��˳�����,�Ժ�ʹ�ö��ַ�����
	for (int j = 0; j < polygon.size(); ++j) {
		float f4 = dot(polygon[j], direction);
		if (f4 > f3) {
			f3 = f4;
			max_point = polygon.at(j);
		}

		if (f4 < f5) {
			f5 = f4;
			min_point = polygon.at(j);
		}
	}
}

bool chung_wang_seperate_algorithm(const std::vector<cocos2d::Vec2> &polygon1, const std::vector<cocos2d::Vec2> &polygon2, cocos2d::Vec2 &seperate_vec2) {
	//�㷨��ʼ����һ�������
	cocos2d::Vec2  direction = cocos2d::Vec2(1.0f, 0.0f);// randomf10(), randomf10());
	int j = 0;
	for (; j < polygon1.size() + polygon2.size(); ++j) {
		cocos2d::Vec2 p1, p2, q1, q2;
		 cw_compute_support_point(polygon1, direction,p1,p2);
		cw_compute_support_point(polygon2, -direction,q1,q2);
		//��ʱdirectionΪ������
		if(dot(p2,direction) > dot(q2,direction) || dot(p1,direction) < dot(q1,direction))
			return false;
		//����,��Ҫ���¼���
		cocos2d::Vec2 normal = normalize(p1,q2);
		const cocos2d::Vec2 ortho(-normal.y, normal.x);

		float f2 = 2.0f * dot(direction,ortho);
		direction -= f2 * ortho;
	}
	return true;
}

bool triangle_contains_point(const cocos2d::Vec2 &a,const cocos2d::Vec2 &b,const cocos2d::Vec2 &c,const cocos2d::Vec2 &check_point) {
	float f1 = cross(check_point,a,b);
	float f2 = cross(check_point,b,c);
	float f3 = cross(check_point,c,a);

	return f1 * f2 >= 0.0f && f2 * f3 >= 0.0f;
}

void simple_polygon_ear_triangulate(const std::vector<cocos2d::Vec2> &polygon, std::vector<short> &triangle_list) {
	//�����ε���ĿΪpolygon.size() - 2
	int  vertex_count = polygon.size();
	std::vector<int>  sequence_array(vertex_count * 2);
	int  *next_array = sequence_array.data();
	int  *prev_array = sequence_array.data() + vertex_count;

	for (int j = 0; j < vertex_count; ++j) {
		next_array[j] = j+1;
		prev_array[j] = j - 1;
	}
	next_array[vertex_count - 1] = 0;
	prev_array[0] = vertex_count - 1;

	int target_j = 0;
	triangle_list.reserve((vertex_count - 2) * 3);
	const cocos2d::Vec2 *polygon_ptr = polygon.data();
	while (vertex_count > 3) {
		bool is_ear_vertex = false;
		int prev_j = prev_array[target_j];
		int next_j = next_array[target_j];
		if (cross(polygon_ptr[target_j], polygon_ptr[next_j], polygon_ptr[prev_j]) > 0.0f) {
			is_ear_vertex = true;
			int compare_j = next_array[next_j];
			while (compare_j != prev_j) {
				if (triangle_contains_point(polygon_ptr[target_j], polygon_ptr[next_j], polygon_ptr[prev_j], polygon_ptr[compare_j])) {
					is_ear_vertex = false;
					break;
				}
				compare_j = next_array[compare_j];
			}
		}
		if (is_ear_vertex) {
			vertex_count -= 1;
			triangle_list.push_back(prev_j);
			triangle_list.push_back(target_j);
			triangle_list.push_back(next_j);

			target_j = prev_j;
			next_array[prev_j] = next_j;
			prev_array[next_j] = prev_j;
		}
		else 
			target_j = next_array[target_j];
	}
	//���һ��������
	triangle_list.push_back(prev_array[target_j]);
	triangle_list.push_back(target_j);
	triangle_list.push_back(next_array[target_j]);
}
NS_GT_END