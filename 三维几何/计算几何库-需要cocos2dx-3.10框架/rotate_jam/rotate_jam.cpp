/*
  *��ת�����㷨ϵ��ʵ��
  *@date:2020��1��8��
  *@author:xiaohuaxiong
 */
#include "rotate_jam.h"
#include "matrix/matrix.h"
#include <functional>
#include <math.h>

USING_NS_CC;

NS_GT_BEGIN
/*
  *��ֱ�ߵ��ཻ����
 */
bool superline_intersect_test(const SuperLine2D &a, const SuperLine2D &b,Vec2 &intersect_point)
{
	//��¼һ��������,���ں��������������ͬ,��Ҫ�Ȱ�װ��һ������dummy��������ʹ�������㷨
	//������߾Ͳ�������ô����,���߿����Լ�ʵ��
	bool br = false;
	if (a.line_type == LineType_Line)
	{
		if (b.line_type == LineType_Line)
			br = line_line_intersect_point(*(Line2D*)&a.start_point, *(Line2D*)&b.start_point, intersect_point);
		else if (b.line_type == LineType_Ray)
			br = ray_line_intersect(b.start_point, b.unknown, a.start_point, a.unknown, intersect_point);
		else
			br = segment_line_intersect(*(Segment2D*)&b.start_point,*(Line2D*)&a.start_point,intersect_point);
	}
	else if (a.line_type == LineType_Ray)
	{
		if (b.line_type == LineType_Line)
			br = ray_line_intersect(a.start_point, a.unknown, b.start_point, b.unknown, intersect_point);
		else if (b.line_type == LineType_Ray)
			br = ray_ray_intersect_test(a.start_point,a.unknown,b.start_point,b.unknown,intersect_point);
		else
			br = segment_ray_intersect(*(Segment2D*)&b.start_point,*(Ray2D*)&a.start_point,intersect_point);
	}
	else//�߶����߶�֮����ཻ����
	{
		if (b.line_type == LineType_Line)
			br = segment_line_intersect(*(Segment2D*)&a.start_point, *(Line2D*)&b.start_point, intersect_point);
		else if (b.line_type == LineType_Ray)
			br = segment_ray_intersect(*(Segment2D*)&a.start_point,*(Ray2D*)&b.start_point,intersect_point);
		else
			br = segment_segment_intersect_test(*(Segment2D*)&a.start_point, *(Segment2D*)&b.start_point, intersect_point);
	}
	return br;
}

void rearrange_half_planes(std::vector<const Line2D*> &origin_planes,int boundary_l,std::vector<SuperLine2D> &super_lines,int split_l)
{
	auto &line = *origin_planes[split_l];
	super_lines[0].line_type = LineType_Ray;
	super_lines[0].start_point = line.start_point - line.direction * 800000.0f;
	super_lines[0].unknown = line.direction;

	int array_size = boundary_l + 1;
	int base_j = 1;
	int index_l = (split_l + 1)%array_size;
	for (; index_l != split_l; index_l = (index_l + 1) % array_size,++base_j)
	{
		auto &line = *origin_planes[index_l];
		super_lines[base_j].line_type = LineType_Line;
		super_lines[base_j].start_point = line.start_point;
		super_lines[base_j].unknown = line.direction;
	}
}

HalfResultType half_planes_intersect(const std::vector<Line2D> &half_planes, std::vector<SuperLine2D> &super_lines)
{
	//���ȶ԰�ƽ���������,������ԭ����ѭ��������ϵ����ʱ����ת����,���������ֱ�߷�����ͬ,�������ں�
	std::function<bool(const SuperLine2D &a, const SuperLine2D &b)> compare_func = [](const SuperLine2D &a,const SuperLine2D &b)->bool {
		float angle_l = atan2f(a.unknown.y,a.unknown.x);
		float angle_r = atan2f(b.unknown.y,b.unknown.x);

		return angle_l < angle_r || angle_l == angle_r && -b.unknown.y * (a.start_point.x - b.start_point.x) + b.unknown.x * (a.start_point.y - b.start_point.y) > 0.0f;
	};
	const int array_size = half_planes.size();
	std::vector<cocos2d::Vec2> polygon_points(array_size);
	super_lines.resize(array_size);

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		super_lines[index_j].line_type = LineType_Line;
		super_lines[index_j].start_point = half_planes[index_j].start_point;
		super_lines[index_j].unknown = half_planes[index_j].direction;
	}
	quick_sort(super_lines.data(), array_size, compare_func);
	//�ڶ���,�޳���ƽ���ҷ�����ͬ��ֱ��,ע�� �޳���ԭ��
	HalfResultType   result_type = ResultType_Unboundary;
	int base_j = 0, erase_l = 1;
	const float scale = 80000.0f;
	Vec2  intersect_point;

	super_lines[0].line_type = LineType_Ray;
	super_lines[0].start_point = super_lines[0].start_point - super_lines[0].unknown *scale;

	//��һ��������ֱ�߼���
	int left = 0, right = 0;
	for (int index_l = 1; index_l <= base_j; ++index_l)
	{
		auto &super_line = super_lines[index_l];
		while (right > left && line_point_distance(super_line.start_point, super_line.unknown, polygon_points[right]) < 0.0f)
			--right;
		while (right > left &&line_point_distance(super_line.start_point, super_line.unknown, polygon_points[left + 1]) < 0.0f)
			++left;

		++right;
		if (index_l != right)
			super_lines[right] = super_line;
		//ע�����µĴ����жϵĺ���,���û�н���,�������ǵļ���,��˵��:��ǰֱ����ǰ����/�߶εĽ���Ϊ��,��ʱ�㷨�޽�
		if( !superline_intersect_test(super_line, super_lines[right - 1], intersect_point))
			return ResultType_Empty;//ʵ������һ���޽�����������ᱻ����Ϊ�ǽ���Ϊ��
		polygon_points[right] = intersect_point;
		//�������µĽ���ļ���,��ǰֱ������һ����ֱ�߽��γ��µĽ�
		super_lines[right].line_type = LineType_Ray;
		super_lines[right].start_point = polygon_points[right];

		super_lines[right - 1].line_type = LineType_Segment;
		super_lines[right - 1].unknown = polygon_points[right];
	}
	//�㷨�����,��һ��ֱ�����γɶ����һ��ֱ�ߵ�Լ��
	const SuperLine2D &line = super_lines[left];
	while (right > left + 1 && line_point_distance(line.start_point, line.unknown, polygon_points[right]) < 0.0f)
		--right;
	//�������Ƿ��ཻ
	bool b = superline_intersect_test(super_lines[left],super_lines[right], intersect_point);
	if (b && right - left > 1)
	{
		result_type = ResultType_Polygon;
		super_lines[left].line_type = LineType_Segment;
		super_lines[left].start_point = intersect_point;

		super_lines[right].line_type = LineType_Segment;
		super_lines[right].unknown = intersect_point;

		SuperLine2D  *copy_array = (SuperLine2D *)malloc(sizeof(SuperLine2D) * (right - left + 1));
		memcpy(copy_array, super_lines.data() + left, sizeof(SuperLine2D) * (right - left + 1));
		super_lines.resize(right - left + 1);
		memcpy(super_lines.data(), copy_array, sizeof(SuperLine2D) * (right - left + 1));
		free(copy_array);
		copy_array = nullptr;
	}
	//�����������Ȼ���޽��
	return result_type;
}

float rotate_hull_max_distance(const std::vector<cocos2d::Vec2> &hull_points, int &start_index, int &final_index)
{
	int array_size = hull_points.size();
	float distance = 0.0f;

	int base_j = 1;

	for (int index_l = 0; index_l < array_size; ++index_l)
	{
		int secondary_j = (index_l +1)%array_size;
		int tripple_j = (base_j +1)%array_size;

		while (cross(hull_points[index_l], hull_points[secondary_j], hull_points[base_j]) < cross(hull_points[index_l], hull_points[secondary_j], hull_points[tripple_j]))
		{
			base_j = tripple_j;
			tripple_j = (tripple_j + 1) % array_size;
		}

		float f = length(hull_points[secondary_j], hull_points[base_j]);
		if (f > distance)
		{
			start_index = secondary_j;
			final_index = base_j;
			distance = f;
		}
	}
	return distance;
}

float rotate_hull_width(const std::vector<cocos2d::Vec2> &hull_points, cocos2d::Vec2 &astart_point, cocos2d::Vec2 &afinal_point)
{
	const int array_size = hull_points.size();
	cocos2d::Vec2 intersect_point;
	float distance = FLT_MAX,f;
	int base_j = 1;

	for (int index_l = 0; index_l < array_size; ++index_l)
	{
		int secondary_l = (index_l +1)%array_size;
		int tripple_l = (base_j +1)%array_size;

		while (cross(hull_points[index_l], hull_points[secondary_l], hull_points[base_j]) < cross(hull_points[index_l], hull_points[secondary_l], hull_points[tripple_l]))
		{
			base_j = tripple_l;
			tripple_l = (tripple_l +1)%array_size;
		}
		//һ�µĴ���ʽ��������㷨��ͬ,ע����ؼ�������
		//���Ŷ���ĵ�,������ֱ�ڵ�ǰ�߲�������������,�������뵱ǰ�ߵĽ���,����ཻ�Ļ�.
		const Vec2 &start_point = hull_points[base_j];
		const Vec2 normal = normalize(hull_points[secondary_l].y - hull_points[index_l].y, -(hull_points[secondary_l].x - hull_points[index_l].x));
		if (segment_ray_intersect(hull_points[index_l],hull_points[secondary_l],start_point,normal,intersect_point) && (f = length(intersect_point, start_point)) < distance)
		{
			astart_point = start_point;
			afinal_point = intersect_point;
			distance = f;
		}
	}
	return distance;
}

float rotate_hull_max_between(const std::vector<cocos2d::Vec2> &hull_points1, const std::vector<cocos2d::Vec2> &hull_points2, int &ahull_index, int &bhull_index)
{
	const int array_size1 = hull_points1.size();
	const int array_size2 = hull_points2.size();
	float distance = 0.0f;

	int base_j = 0;
	for (int index_l = 0; index_l < array_size1; ++index_l)
	{
		int secondary_l = (index_l +1)%array_size1;
		int tripple_l = (base_j + 1) % array_size2;

		while (cross(hull_points1[index_l], hull_points1[secondary_l], hull_points2[base_j]) < cross(hull_points1[index_l], hull_points1[secondary_l], hull_points2[tripple_l]))
		{
			base_j = tripple_l;
			tripple_l = (tripple_l+1)%array_size2;
		}
		float f = length(hull_points1[secondary_l],hull_points2[base_j]);
		if (f > distance)
		{
			distance = f;
			ahull_index = secondary_l;
			bhull_index = base_j;
		}
	}
	return distance;
}

float rotate_hull_min_between(const std::vector<cocos2d::Vec2> &hull_points1, const std::vector<cocos2d::Vec2> &hull_points2, cocos2d::Vec2 &ahull_point, cocos2d::Vec2 &bhull_point)
{
	const int array_size1 = hull_points1.size();
	const int array_size2 = hull_points2.size();
	Vec2 aintersect_point, bintersect_point;
	float distance = FLT_MAX;
	int base_j = 0;
	/*
	  *��һ���ȶ�λ����Ŀ�����Զ�Ķ���,ע����Ϊ�þ������Ϊ����,���
	  *���ֱ��ʹ��������㷨ģʽ,����ܻ���ֲ���ȷ�Ľ��.
	 */
	int past_l = array_size2-1;
	while (cross(hull_points1[0], hull_points1[1], hull_points2[base_j]) < cross(hull_points1[0], hull_points1[1], hull_points2[past_l]))
	{
		base_j = past_l;
		past_l = (past_l -1 + array_size2)%array_size2;
	}

	Segment2D a = { hull_points1[0],hull_points1[1] }, b = { hull_points2[base_j],hull_points2[(base_j +1)%array_size2] };
	distance = segment_segment_minimum_distance(a,b,ahull_point,bhull_point);
	//��һ��
	b.start_point = hull_points2[past_l];
	b.final_point = hull_points2[base_j];
	float f = segment_segment_minimum_distance(a, b, aintersect_point, bintersect_point);
	if (f < distance)
	{
		distance = f;
		ahull_point = aintersect_point;
		bhull_point = bintersect_point;
	}

	for (int index_l = 1; index_l < array_size1; ++index_l)
	{
		int secondary_l = (index_l+1)%array_size1;
		int tripple_l = (base_j +1)%array_size2;
		while (cross(hull_points1[index_l], hull_points1[secondary_l], hull_points2[base_j]) < cross(hull_points1[index_l], hull_points1[secondary_l], hull_points2[tripple_l]))
		{
			base_j = tripple_l;
			tripple_l = (tripple_l + 1)%array_size2;
		}
		Segment2D a = { hull_points1[index_l],hull_points1[secondary_l] }, b = {hull_points2[base_j],hull_points2[tripple_l]};
		float f = segment_segment_minimum_distance(a, b, aintersect_point, bintersect_point);
		if (f < distance)
		{
			ahull_point = aintersect_point;
			bhull_point = bintersect_point;
			distance = f;
		}
		b.start_point = hull_points2[(base_j - 1 + array_size2)%array_size2];
		b.final_point = hull_points2[base_j];
		f = segment_segment_minimum_distance(a, b, aintersect_point, bintersect_point);
		if (f < distance)
		{
			ahull_point = aintersect_point;
			bhull_point = bintersect_point;
			distance = f;
		}
	}
	return distance;
}

float rotate_hull_min_area(const std::vector<cocos2d::Vec2> &hull_points, cocos2d::Vec2 rect_points[4])
{
	const int array_size = hull_points.size();
	int left = 0, right=1, up=0;
	int l=0, r=0, b=0,u=0;
	float area = FLT_MAX;

	for (int index_l = 0; index_l < array_size; ++index_l)
	{
		int secondary_l = (index_l+1)%array_size;
		const Vec2 &start_point = hull_points[index_l];
		const Vec2 &final_point = hull_points[secondary_l];
		//��
		int tripple_l = (right+1)%array_size;
		while(dot(start_point, final_point, hull_points[right]) < dot(start_point, final_point, hull_points[tripple_l]))
		{
			right = tripple_l;
			tripple_l = (tripple_l + 1) % array_size;
		}
		if (!index_l)
			up = right;
		//��
		tripple_l = (up+1)%array_size;
		while (cross(start_point, final_point, hull_points[up]) < cross(start_point, final_point, hull_points[tripple_l]))
		{
			up = tripple_l;
			tripple_l = (tripple_l+1)%array_size;
		}
		//��
		if (!index_l)
			left = up;
		tripple_l = (left+1)%array_size;
		while (dot(final_point, start_point, hull_points[left]) < dot(final_point, start_point, hull_points[tripple_l]))
		{
			left = tripple_l;
			tripple_l = (tripple_l + 1)%array_size;
		}
		//���㵱ǰ�γɵ���Ӿ��ε����
		float h = cross(start_point,final_point,hull_points[up])/length(start_point,final_point);//ע����д���ĺ���
		Vec2 stride = hull_points[right] - hull_points[left];
		float w = dot(stride,normalize(start_point,final_point));
		float f = w * h;
		if (f < area)
		{
			area = f;
			l = left; r = right; b = index_l; u = up;
		}
	}
	//������ص���Ӿ���
	int secondary_l = (b+1)%array_size;
	float h = cross(hull_points[b],hull_points[secondary_l],hull_points[u])/length(hull_points[b],hull_points[secondary_l]);
	Vec2 stride = hull_points[r] - hull_points[l];
	Vec2 normal = normalize(hull_points[b],hull_points[secondary_l]);
	float w = dot(stride,normal);

	float step_left = dot(hull_points[l] - hull_points[b],normal);
	rect_points[0] = hull_points[b] + normal * step_left;
	rect_points[1] = rect_points[0] + normal * w;

	Vec2 up_normal(-normal.y,normal.x);
	rect_points[2] = rect_points[1] + up_normal * h;
	rect_points[3] = rect_points[0] + up_normal * h;

	return area;
}

float rotate_hull_min_perimeter(const std::vector<cocos2d::Vec2> &hull_points, cocos2d::Vec2 rect_points[4])
{
	const int array_size = hull_points.size();
	int left = 0, right = 1, up = 0;
	int l = 0, r = 0, b = 0, u = 0;
	float perimeter = FLT_MAX;

	for (int index_l = 0; index_l < array_size; ++index_l)
	{
		int secondary_l = (index_l + 1) % array_size;
		const Vec2 &start_point = hull_points[index_l];
		const Vec2 &final_point = hull_points[secondary_l];
		//��
		int tripple_l = (right + 1) % array_size;
		while (dot(start_point, final_point, hull_points[right]) < dot(start_point, final_point, hull_points[tripple_l]))
		{
			right = tripple_l;
			tripple_l = (tripple_l + 1) % array_size;
		}
		if (!index_l)
			up = right;
		//��
		tripple_l = (up + 1) % array_size;
		while (cross(start_point, final_point, hull_points[up]) < cross(start_point, final_point, hull_points[tripple_l]))
		{
			up = tripple_l;
			tripple_l = (tripple_l + 1) % array_size;
		}
		//��
		if (!index_l)
			left = up;
		tripple_l = (left + 1) % array_size;
		while (dot(final_point, start_point, hull_points[left]) < dot(final_point, start_point, hull_points[tripple_l]))
		{
			left = tripple_l;
			tripple_l = (tripple_l + 1) % array_size;
		}
		//���㵱ǰ�γɵ���Ӿ��ε����
		float h = cross(start_point, final_point, hull_points[up]) / length(start_point, final_point);//ע����д���ĺ���
		Vec2 stride = hull_points[right] - hull_points[left];
		float w = dot(stride, normalize(start_point, final_point));
		float f = 2.0f * (w + h);
		if (f < perimeter)
		{
			perimeter = f;
			l = left; r = right; b = index_l; u = up;
		}
	}
	//������ص���Ӿ���
	int secondary_l = (b + 1) % array_size;
	float h = cross(hull_points[b], hull_points[secondary_l], hull_points[u]) / length(hull_points[b], hull_points[secondary_l]);
	Vec2 stride = hull_points[r] - hull_points[l];
	Vec2 normal = normalize(hull_points[b], hull_points[secondary_l]);
	float w = dot(stride, normal);

	float step_left = dot(hull_points[l] - hull_points[b], normal);
	rect_points[0] = hull_points[b] + normal * step_left;
	rect_points[1] = rect_points[0] + normal * w;

	Vec2 up_normal(-normal.y, normal.x);
	rect_points[2] = rect_points[1] + up_normal * h;
	rect_points[3] = rect_points[0] + up_normal * h;

	return perimeter;
}

void static_onion_decomposite(const std::vector<const Vec2*> &outer_convex_hull,const std::vector<const Vec2*> &inner_convex_hull,std::vector<const Vec2*> &triangle_edges)
{
	//��������͹����ε���Сy��������,�ټ���Ĺ�����,�����Ѿ������һ�������y��������С,���ֱ��ʹ��
	int array_size1 = outer_convex_hull.size();
	int array_size2 = inner_convex_hull.size();
	int array_size = min_f(array_size1,array_size2);
	int index_a = 0, index_b = 0;

	vector_fast_push_back(triangle_edges,inner_convex_hull[index_b]);
	vector_fast_push_back(triangle_edges,outer_convex_hull[index_a]);

	while(index_a < array_size1 || index_b < array_size2)
	{
		const Vec2 *p = outer_convex_hull[index_a%array_size1];
		const Vec2 *q = inner_convex_hull[index_b%array_size2];
		//�����Ҫ������һ����
		int secondary_l = (index_a+1)%array_size1;
		int tripple_l = (index_b+1)%array_size2;

		const Vec2 *a = outer_convex_hull[secondary_l];
		const Vec2 *b = inner_convex_hull[tripple_l];

		float f = cross(*a - *p, *b - *q);
		if (f > 0.0f)
		{
			vector_fast_push_back(triangle_edges,p);
			vector_fast_push_back(triangle_edges,a);

			vector_fast_push_back(triangle_edges,a);
			vector_fast_push_back(triangle_edges,q);

			++index_a;
		}
		else if (f < 0.0f)
		{
			vector_fast_push_back(triangle_edges,q);
			vector_fast_push_back(triangle_edges,b);

			vector_fast_push_back(triangle_edges,b);
			vector_fast_push_back(triangle_edges,p);

			++index_b;
		}
		else
		{
			vector_fast_push_back(triangle_edges,q);
			vector_fast_push_back(triangle_edges,b);

			vector_fast_push_back(triangle_edges,b);
			vector_fast_push_back(triangle_edges,p);

			vector_fast_push_back(triangle_edges,p);
			vector_fast_push_back(triangle_edges,a);

			++index_a; ++index_b;
		}
	}
}

void rotate_hull_base_method(const cocos2d::Vec2 **points,int array_size,std::vector<const cocos2d::Vec2*> &polygon)
{
	//��һ��,�Զ����������
	std::vector<const Vec2 *> points_tmp(array_size);
	memcpy(points_tmp.data(),points,sizeof(Vec2*) * array_size);
	//����y������С,����x�����С�ĵ�
	int target_j = 0;
	for (int index_j = 1; index_j < array_size; ++index_j)
	{
		if (points_tmp[index_j]->y < points_tmp[target_j]->y || (points_tmp[index_j]->y == points_tmp[target_j]->y && points_tmp[index_j]->x > points_tmp[target_j]->x))
			target_j = index_j;
	}

	const Vec2 *f_point = points_tmp[target_j];
	points_tmp.erase(points_tmp.begin() + target_j);
	std::function<bool(const Vec2 *a, const Vec2 *b)>  compare_func = [f_point](const Vec2 *a, const Vec2 *b)->bool {
		const float d_x = a->x - f_point->x;
		const float d_y = a->y - f_point->y;

		const float f_x = b->x - f_point->x;
		const float f_y = b->y - f_point->y;

		float angle_a = atan2f(d_y, d_x);
		float angle_b = atan2f(f_y, f_x);
		//����Ƕ���ͬ,����ο���������������λ�ø���ǰ
		return angle_a < angle_b || (angle_a == angle_b && d_x * d_x + d_y * d_y < f_x * f_x + f_y * f_y);//���⼫����ͬ�ĵ㼯
	};
	quick_sort_origin_type<const Vec2*>(points_tmp.data(), array_size - 1, compare_func);

	points[0] = f_point;
	memcpy(points +1,points_tmp.data(),sizeof(Vec2*) * (array_size - 1));

	polygon.reserve(array_size);
	polygon.push_back(f_point);
	polygon.push_back(points_tmp[0]);

	for (int index_j = 1; index_j < array_size - 1; ++index_j)
	{
		//�Ե�ǰ�ĵ�������
		const Vec2 *target_point = points_tmp.at(index_j);
		//����Ƿ�����ǰ��������������һ�µ�
		while (polygon.size() > 1 && sign_area(*polygon.back() - *polygon[polygon.size() - 2], *target_point - *polygon.back()) <= 0)
		{
			polygon.pop_back();
		}
		polygon.push_back(target_point);
	}
}

void rotate_hull_onion_decomposite(const std::vector<cocos2d::Vec2> &points, std::vector<const Vec2*> &triangle_edges)
{
	int array_size = points.size();
	std::vector<const Vec2*>  disper_points(array_size);
	for (int index_j = 0; index_j < array_size; ++index_j)
		disper_points[index_j] = points.data() + index_j;
	//ѭ��������ת�����㷨
	std::vector<const Vec2*>	polygon1,polygon2;
	int  remind_size = array_size;
	bool  swap_exchange = false;

	while (remind_size > 2)
	{
		std::vector<const Vec2 *>  &last_polygon = swap_exchange?polygon2:polygon1;
		std::vector<const Vec2 *> &now_polygon = swap_exchange ? polygon1:polygon2;

		now_polygon.clear();
		rotate_hull_base_method(disper_points.data(), remind_size,now_polygon);

		//���Ѿ�ѡ����͹�������ԭ�������Ƴ���,�����ʱ��ΪO(n)
		int base_l = 0,compare_l = 0;
		for (int index_j = 0; index_j < remind_size;++index_j)
		{
			if (disper_points[index_j] == now_polygon[compare_l])
				++compare_l;
			else
			{
				if (index_j != base_l)
					disper_points[base_l] = disper_points[index_j];
				++base_l;
			}
		}
		assert(base_l + now_polygon.size() == remind_size);

		if (remind_size != array_size)
			static_onion_decomposite(last_polygon, now_polygon, triangle_edges);

		swap_exchange = swap_exchange ^ 1;
		remind_size = base_l;
	}
	//�������ʣ�µĶ���,��Ҫ����ĵ�������,Ŀǰ��ʱ�����κεĶ���
}
/*
  *������ĺ������,�ú����ļ�����̷ǳ��ĸ���
  *���ٽǶȵıȽϼ��������,������������ϵ����ת
 */
void rotate_hull_base_method(const cocos2d::Vec2 **points, int array_size,int erase_index,const Vec2 &ortho_axis, std::vector<const cocos2d::Vec2*> &polygon)
{
	//��һ��,�Զ����������
	int array_new_size = array_size - 1;
	std::vector<const Vec2 *> points_tmp(array_size);
	memcpy(points_tmp.data(), points, sizeof(Vec2*) * array_size);
	float fix_angle = radian_from(ortho_axis,Vec2::UNIT_Y);
	float sin_f = sinf(fix_angle), cos_f = cosf(fix_angle);
	//����y������С,����x�����С�ĵ�
	const Vec2 *f_point = points[erase_index];
	points_tmp.erase(points_tmp.begin()+ erase_index);
	std::function<bool(const Vec2 *a, const Vec2 *b)>  compare_func = [f_point,sin_f,cos_f](const Vec2 *a, const Vec2 *b)->bool {
		const float d_x = a->x - f_point->x;
		const float d_y = a->y - f_point->y;

		const float f_x = b->x - f_point->x;
		const float f_y = b->y - f_point->y;

		float angle_a = atan2f(d_y * cos_f + d_x * sin_f, d_x * cos_f - d_y * sin_f);
		float angle_b = atan2f(f_y * cos_f + f_x * sin_f, f_x * cos_f - f_y * sin_f);
		//����Ƕ���ͬ,����ο���������������λ�ø���ǰ
		return angle_a < angle_b || (angle_a == angle_b && d_x * d_x + d_y * d_y < f_x * f_x + f_y * f_y);//���⼫����ͬ�ĵ㼯
	};
	quick_sort_origin_type<const Vec2*>(points_tmp.data(), array_new_size, compare_func);

	points[0] = f_point;
	memcpy(points + 1, points_tmp.data(), sizeof(Vec2*) * array_new_size);

	polygon.reserve(array_size);
	polygon.push_back(f_point);
	polygon.push_back(points_tmp[0]);

	for (int index_j = 1; index_j < array_new_size; ++index_j)
	{
		//�Ե�ǰ�ĵ�������
		const Vec2 *target_point = points_tmp.at(index_j);
		//����Ƿ�����ǰ��������������һ�µ�
		while (polygon.size() > 1 && sign_area(*polygon.back() - *polygon[polygon.size() - 2], *target_point - *polygon.back()) <= 0)
		{
			polygon.pop_back();
		}
		polygon.push_back(target_point);
	}
}

int rotate_hull_spiral_line(const std::vector<cocos2d::Vec2> &points, std::vector<const cocos2d::Vec2 *> &spiral_points)
{
	auto *array_ptr = points.data();
	const int array_size = points.size();
	//��һ��ɸѡ��y������С�ĵ�
	const Vec2 *compare_base = array_ptr;
	for (int index_l = 1; index_l < array_size; ++index_l)
	{
		if (array_ptr[index_l].y < compare_base->y || array_ptr[index_l].y == compare_base->y && array_ptr[index_l].x < compare_base->x)
			compare_base = array_ptr + index_l;
	}

	spiral_points.resize(array_size);
	std::vector<const Vec2*>  tmp_array(array_size);
	for (int index_l = 0; index_l < points.size(); ++index_l)
		tmp_array[index_l] = array_ptr + index_l;

	int base_j = 0,remind_count = array_size,corner_index = 0;
	int erase_index = compare_base - array_ptr;
	const Vec2 *base_ptr = nullptr;

	while (remind_count > 2)
	{
		std::vector<const Vec2*> polygon;
		rotate_hull_base_method(tmp_array.data(),remind_count, erase_index,!base_j?Vec2::UNIT_Y:normalize(*compare_base,*base_ptr),polygon);

		memcpy(spiral_points.data()+base_j,polygon.data(),sizeof(Vec2*)*polygon.size());
		//��¼�µ�һ��ת�������
		if (!corner_index)corner_index = polygon.size() - 1;

		//�ƶ�����
		int base_l = 0, compare_index = 0;
		for (int index_j = 0; index_j < remind_count; ++index_j)
		{
			if (tmp_array[index_j] == polygon[compare_index])
				++compare_index;
			else
			{
				if (index_j != base_l)
					tmp_array[base_l] = tmp_array[index_j];
				++base_l;
			}
		}
		assert(base_l + polygon.size() == remind_count);
		base_j += polygon.size();
		remind_count = base_l;
		//������base_ptr���ɵļн���С���Ǹ�����,ʵ��������Ĵ�����Ժϲ��������ѭ����,����Ϊ����������,�ͷֿ���
		compare_base = tmp_array[0];
		erase_index = 0;
		base_ptr = polygon.back();
		for (int index_l = 1; index_l < remind_count; ++index_l)
		{
			if (cross(*base_ptr, *compare_base, *tmp_array[index_l]) < 0.0f)
				compare_base = tmp_array[index_l], erase_index = index_l;
		}
	}
	//�����ʣ�µĵ�,����Ҫ�����Ĵ���,�䴦�������ʽ������,������������ͬ
	if (remind_count == 2)
	{
		//��ʣ�µ�������������,�����ԭ��������һ��
		if (cross(*base_ptr, *tmp_array[0], *tmp_array[1]) >= 0.0f)
		{
			spiral_points[base_j] = tmp_array[0];
			spiral_points[base_j + 1] = tmp_array[1];
		}
		else
		{
			spiral_points[base_j] = tmp_array[1];
			spiral_points[base_j + 1] = tmp_array[0];
		}
		base_j += 2;
	}
	else if (remind_count > 0)
	{
		spiral_points[base_j] = tmp_array[0];
		base_j += 1;
	}
	return corner_index;
}

void rotate_hull_spiral_decomposite(const std::vector<cocos2d::Vec2> &points, std::vector<const cocos2d::Vec2 *> &triangle_edges)
{
	std::vector<const Vec2*> spiral_line_points;
	//���ȼ�����ɢ�㼯��������
	int  split_index = rotate_hull_spiral_line(points, spiral_line_points);
	//��������split_index,�������߻���Ϊ����Լ��ڲ�,������Ҫ�����ڲ����ֹ����
	int array_size = spiral_line_points.size();
	int start_index = array_size - 3;
	for (int index_l = 0; index_l < spiral_line_points.size()- 1; ++index_l)
	{
		vector_fast_push_back(triangle_edges,spiral_line_points[index_l]);
		vector_fast_push_back(triangle_edges,spiral_line_points[index_l+1]);
	}

	const Vec2 &start_point = *spiral_line_points[array_size-2];
	const Vec2 normal = normalize(start_point,*spiral_line_points[array_size-1]);
	Vec2 intersect_point;
	while (start_index && !segment_ray_intersect(*spiral_line_points[start_index], *spiral_line_points[start_index - 1], start_point, normal, intersect_point))
		--start_index;
	int inner_outer_index = start_index - 1;
	//�������߽��������ʷ�,�ֽ��㷨�ĺ���˼����Ȼ����תƽ����
	int outer_start_index = 0;
	int inner_start_index = split_index;

	const Vec2 *p = spiral_line_points[outer_start_index];
	const Vec2 *q = spiral_line_points[inner_start_index];

	vector_fast_push_back(triangle_edges,p);
	vector_fast_push_back(triangle_edges,q);

	while (outer_start_index < inner_outer_index && inner_start_index < array_size-1)
	{
		const Vec2 *a = spiral_line_points[outer_start_index +1];
		const Vec2 *b = spiral_line_points[inner_start_index +1];

		float f = cross(*a - *p,*b - *q);
		if (f > 0.0f)
		{
			vector_fast_push_back(triangle_edges,a);
			vector_fast_push_back(triangle_edges,q);

			p = a;
			++outer_start_index;
		}
		else if (f < 0.0f)
		{
			vector_fast_push_back(triangle_edges,b);
			vector_fast_push_back(triangle_edges,p);

			q = b;
			++inner_start_index;
		}
		else
		{
			vector_fast_push_back(triangle_edges,b);
			vector_fast_push_back(triangle_edges,p);

			p = a;
			q = b;
			++outer_start_index; ++inner_start_index;
		}
	}
	//��Ҫ���������
	while (outer_start_index < inner_outer_index)
	{
		vector_fast_push_back(triangle_edges, spiral_line_points[array_size-1]);
		vector_fast_push_back(triangle_edges,spiral_line_points[outer_start_index] );

		++outer_start_index;
	}

	while (inner_start_index < array_size - 1)
	{
		vector_fast_push_back(triangle_edges,spiral_line_points[inner_outer_index]);
		vector_fast_push_back(triangle_edges,spiral_line_points[inner_start_index]);

		++inner_start_index;
	}

	vector_fast_push_back(triangle_edges,spiral_line_points[array_size-1]);
	vector_fast_push_back(triangle_edges,spiral_line_points[inner_outer_index]);
	//���ʣ��ĵ�,�����һ�����start_index-->array_size - 2�ĵ���������Ӽ���
	const Vec2 *center = spiral_line_points[array_size - 1];
	for (int index_l = start_index; index_l < array_size - 2; ++index_l)
	{
		vector_fast_push_back(triangle_edges, center);
		vector_fast_push_back(triangle_edges, spiral_line_points[index_l]);
	}
}

void rotate_hull_polygon_union(const std::vector<cocos2d::Vec2> &polygon1, const std::vector<cocos2d::Vec2> &polygon2, std::vector<cocos2d::Vec2> &polygon_union)
{
	int array_size1 = polygon1.size();
	int array_size2 = polygon2.size();
	//������͹����εĹ�����,���û���ҵ�,��˵��,һ������α�Ȼ����һ�������֮��
	int a_index = 0, b_index = 0;
	int a_select_index = 0, b_select_index = 0;
	bool b_found = false;

	while (!b_found && (a_index < array_size1 || b_index < array_size2))
	{
		int a_next = (a_select_index+1)%array_size1;
		int a_prev = (a_select_index-1 +array_size1)%array_size1;

		int b_next = (b_select_index +1)%array_size2;
		int b_prev = (b_select_index -1+array_size2)%array_size2;
		//��Ҫ��̬ѡ���֧����,ԭ���ǿ��ܻ����AǶ����B,����BǶ����A�е����,���ֻ��һ��,����������ѭ��
		//��Ȼ�ж�̬ѡ��,�����ܵ�����ʱ����Ȼ��O(m+n)
		if (b_index < array_size2)
		{
			if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon2[b_next]) < 0.0f)
				b_select_index = b_next, ++b_index;
			else if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon2[b_prev]) < 0.0f)
				b_select_index = b_prev, ++b_index;
			else if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon1[a_next]) < 0.0f)
				a_select_index = a_next, ++a_index;
			else if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon1[a_prev]) < 0.0f)
				a_select_index = a_prev, ++a_index;
			else
				b_found = true;
		}
		else
		{
			if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon1[a_next]) < 0.0f)
				a_select_index = a_next, ++a_index;
			else if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon1[a_prev]) < 0.0f)
				a_select_index = a_prev, ++a_index;
			else if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon2[b_next]) < 0.0f)
				b_select_index = b_next, ++b_index;
			else if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon2[b_prev]) < 0.0f)
				b_select_index = b_prev, ++b_index;
			else
				b_found = true;
		}
	}
	//����Ƿ���ҵ�,�������Ϊ��,�����һ�������Ƕ������һ��֮��
	if (!b_found)
	{
		//����ȡ��������
		if (cross(polygon1[0], polygon1[1], polygon2[0]) > 0.0f)
			polygon_union = polygon1;
		else
			polygon_union = polygon2;
		return;
	}
	//ʣ�µ������ȻΪ���߽���,���߷���,��ʱ��Ҫ����һ������Ĺ����߼������
	int a_next_tangent = a_select_index;
	int b_next_tangent = b_select_index;
	//
	a_index = a_select_index;
	b_index = b_select_index;
	//����һ��������,��Ҫ����Ľ����������γɵ�˳��
	bool need_exchange = true;
	do
	{
		a_next_tangent = (a_next_tangent + need_exchange)%array_size1;
		b_next_tangent = (b_next_tangent + (need_exchange ^1))%array_size2;

		b_found = false;
		while (!b_found)
		{
			const Vec2 &start_point = need_exchange ? polygon2[b_next_tangent] : polygon1[a_next_tangent];
			const Vec2 &middle_point = need_exchange ? polygon1[a_next_tangent] : polygon2[b_next_tangent];

			int a_next = (a_next_tangent + 1) % array_size1;
			int b_next = (b_next_tangent + 1) % array_size2;
			//ע�⴦��ѡ������ʱ�����ȼ��Ƕ�̬��
			if (need_exchange)
			{
				if (cross(start_point, middle_point, polygon1[a_next]) < 0.0f)
					a_next_tangent = a_next;
				else if (cross(start_point, middle_point, polygon2[b_next]) < 0.0f)
					b_next_tangent = b_next;
				else b_found = true;
			}
			else
			{
				if (cross(start_point, middle_point, polygon2[b_next]) < 0.0f)
					b_next_tangent = b_next;
				else if (cross(start_point, middle_point, polygon1[a_next]) < 0.0f)
					a_next_tangent = a_next;
				else b_found = true;
			}
		}
		//��ʱ��Ҫ���м�Ķ���д�뵽������,�������ѡ��,�������൱�ϸ�����𷽷�
		if (need_exchange)
		{
			for (int index_l = b_index; index_l != b_next_tangent;index_l = (index_l +1)%array_size2)
				vector_fast_push_back(polygon_union, polygon2[index_l]);
			vector_fast_push_back(polygon_union, polygon2[b_next_tangent]);
		}
		else
		{
			for (int index_l = a_index; index_l != a_next_tangent; index_l = (index_l + 1) % array_size1)
				vector_fast_push_back(polygon_union,polygon1[index_l]);
			vector_fast_push_back(polygon_union, polygon1[a_next_tangent]);
		}

		a_index = a_next_tangent;
		b_index = b_next_tangent;
		need_exchange ^= 1;
	} while (a_next_tangent != a_select_index || b_next_tangent != b_select_index);
}
/*
  *��������
  *�Ƿ��߶�b���߶�a���γɵ�ֱ�ߵ�ͬһ��
 */
bool static_rotate_hull_segment_same_side(const Vec2 &astart_point,const Vec2 &afinal_point,const Vec2 &bstart_point,const Vec2 &bfinal_point)
{
	return cross(astart_point,afinal_point,bstart_point) * cross(astart_point,afinal_point,bfinal_point) > 0.0f;
}
/*
  *��������
  *��������Ĺ���������������ཻ�߶�
 */
bool static_rotate_polygon_near_segment(const std::vector<Vec2> &polygon1,const std::vector<Vec2> &polygon2,int a_select_index,int b_select_index,int &a_near_index,int &b_near_index)
{
	int array_size1 = polygon1.size();
	int array_size2 = polygon2.size();
	int a_index =0,b_index = 0;
	bool b_found = false;
	int a_compare_index = a_select_index, b_compare_index = b_select_index;
	while (!b_found && (a_index < array_size1 || b_index < array_size2))
	{
		int a_next_index = (a_compare_index + 1) % array_size1;
		int b_next_index = (b_compare_index - 1 + array_size2) % array_size2;

		if (b_index < array_size2)
		{
			bool b1 = static_rotate_hull_segment_same_side(polygon1[a_compare_index], polygon1[a_next_index], polygon2[b_compare_index], polygon2[b_next_index]);
			if (b1)
			{
				b_index += 1;
				b_compare_index = (b_compare_index - 1 + array_size2) % array_size2;
			}
			else if (static_rotate_hull_segment_same_side(polygon2[b_compare_index], polygon2[b_next_index], polygon1[a_compare_index], polygon1[a_next_index]))
			{
				a_index += 1;
				a_compare_index = (a_compare_index + 1) % array_size1;
			}
			else
				b_found = true;
		}
		else
		{
			bool b1 = static_rotate_hull_segment_same_side(polygon2[b_compare_index], polygon2[b_next_index], polygon1[a_compare_index], polygon1[a_next_index]);
			if (b1)
			{
				a_index += 1;
				a_compare_index = (a_compare_index + 1) % array_size1;
			}
			else if (static_rotate_hull_segment_same_side(polygon1[a_compare_index], polygon1[a_next_index], polygon2[b_compare_index], polygon2[b_next_index]))
			{
				b_index += 1;
				b_compare_index = (b_compare_index - 1 + array_size2) % array_size2;
			}
			else
				b_found = true;
		}
	}
	a_near_index = a_compare_index;
	b_near_index = b_compare_index;
	return b_found;
}

bool rotate_hull_polygon_intersect(const std::vector<cocos2d::Vec2> &polygon1, const std::vector<cocos2d::Vec2> &polygon2, std::vector<cocos2d::Vec2> &polygon_intersect)
{
	int array_size1 = polygon1.size();
	int array_size2 = polygon2.size();
	//�����һ��������
	int a_index = 0, b_index = 0;
	int a_select_index = 0, b_select_index = 0;
	bool b_found = false;

	while (!b_found && (a_index < array_size1 || b_index < array_size2))
	{
		int a_next = (a_select_index + 1) % array_size1;
		int a_prev = (a_select_index - 1 + array_size1) % array_size1;

		int b_next = (b_select_index + 1) % array_size2;
		int b_prev = (b_select_index - 1 + array_size2) % array_size2;
		//��Ҫ��̬ѡ���֧����,ԭ���ǿ��ܻ����AǶ����B,����BǶ����A�е����,���ֻ��һ��,����������ѭ��
		//��Ȼ�ж�̬ѡ��,�����ܵ�����ʱ����Ȼ��O(m+n)
		if (b_index < array_size2)
		{
			if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon2[b_next]) < 0.0f)
				b_select_index = b_next, ++b_index;
			else if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon2[b_prev]) < 0.0f)
				b_select_index = b_prev, ++b_index;
			else if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon1[a_next]) < 0.0f)
				a_select_index = a_next, ++a_index;
			else if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon1[a_prev]) < 0.0f)
				a_select_index = a_prev, ++a_index;
			else
				b_found = true;
		}
		else
		{
			if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon1[a_next]) < 0.0f)
				a_select_index = a_next, ++a_index;
			else if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon1[a_prev]) < 0.0f)
				a_select_index = a_prev, ++a_index;
			else if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon2[b_next]) < 0.0f)
				b_select_index = b_next, ++b_index;
			else if (cross(polygon1[a_select_index], polygon2[b_select_index], polygon2[b_prev]) < 0.0f)
				b_select_index = b_prev, ++b_index;
			else
				b_found = true;
		}
	}
	//ʹ�����������Ľ����һ��ȥ�ж��Ƿ�һ������ΰ�����һ��
	if (!b_found)
	{
		if (cross(polygon1[0], polygon1[1], polygon2[0]) > 0.0f)
			polygon_intersect = polygon2;
		else
			polygon_intersect = polygon1;
		return true;
	}
	//����ҵ��˹�����,��ʱ����Ҫ�ж���������Ƿ���ڽ���
	a_index = b_index = 0;
	int a_compare_index = a_select_index, b_compare_index = b_select_index;
	b_found = static_rotate_polygon_near_segment(polygon1, polygon2, a_select_index, b_select_index,a_compare_index,b_compare_index);
	//��ʱ����Ϊ��
	if (!b_found)return false;
	//������,�����������,��������صĽ���
	int a_next_tangent = a_select_index;
	int b_next_tangent = b_select_index;
	//
	a_index = a_select_index;
	b_index = b_select_index;
	//����һ��������,��Ҫ����Ľ����������γɵ�˳��
	bool need_exchange = true;
	cocos2d::Vec2  intersect_point,intersect_point2;
	do
	{
		a_next_tangent = (a_next_tangent + need_exchange) % array_size1;
		b_next_tangent = (b_next_tangent + (need_exchange ^ 1)) % array_size2;

		b_found = false;
		while (!b_found)
		{
			const Vec2 &start_point = need_exchange ? polygon2[b_next_tangent] : polygon1[a_next_tangent];
			const Vec2 &middle_point = need_exchange ? polygon1[a_next_tangent] : polygon2[b_next_tangent];

			int a_next = (a_next_tangent + 1) % array_size1;
			int b_next = (b_next_tangent + 1) % array_size2;
			//ע�⴦��ѡ������ʱ�����ȼ��Ƕ�̬��
			if (need_exchange)
			{
				if (cross(start_point, middle_point, polygon1[a_next]) < 0.0f)
					a_next_tangent = a_next;
				else if (cross(start_point, middle_point, polygon2[b_next]) < 0.0f)
					b_next_tangent = b_next;
				else b_found = true;
			}
			else
			{
				if (cross(start_point, middle_point, polygon2[b_next]) < 0.0f)
					b_next_tangent = b_next;
				else if (cross(start_point, middle_point, polygon1[a_next]) < 0.0f)
					a_next_tangent = a_next;
				else b_found = true;
			}
		}
		//��ʱ��Ҫ���м�Ķ���д�뵽������,�������ѡ��,�������൱�ϸ�����𷽷�
		int a_new_index = 0, b_new_index = 0;
		if (need_exchange)
		{
			bool b = segment_segment_intersect_test(polygon1[a_compare_index], polygon1[(a_compare_index + 1) % array_size1], polygon2[b_compare_index], polygon2[(b_compare_index - 1 + array_size2) % array_size2], intersect_point);
			assert(b);
			vector_fast_push_back(polygon_intersect, intersect_point);

			bool b_intersect = static_rotate_polygon_near_segment(polygon2, polygon1, b_next_tangent, a_next_tangent, b_new_index, a_new_index);
			assert(b_intersect);
			//��ǰ���߶ν��������
			for (int index_l = (a_compare_index + 1)%array_size1; index_l != a_new_index; index_l = (index_l + 1) % array_size1)
				vector_fast_push_back(polygon_intersect, polygon1[index_l]);
		}
		else
		{
			bool b = segment_segment_intersect_test(polygon2[b_compare_index], polygon2[(b_compare_index + 1) % array_size2], polygon1[a_compare_index], polygon1[(a_compare_index - 1 + array_size1) % array_size1], intersect_point);
			assert(b);
			vector_fast_push_back(polygon_intersect, intersect_point);

			bool b_intersect = static_rotate_polygon_near_segment(polygon1,polygon2,a_next_tangent,b_next_tangent,a_new_index,b_new_index);
			assert(b_intersect);

			for (int index_l = (b_compare_index + 1)%array_size2; index_l != b_new_index; index_l = (index_l + 1) % array_size2)
				vector_fast_push_back(polygon_intersect, polygon2[index_l]);
		}

		a_compare_index = a_new_index;
		b_compare_index = b_new_index;

		a_index = a_next_tangent;
		b_index = b_next_tangent;
		need_exchange ^= 1;
	} while (a_next_tangent != a_select_index || b_next_tangent != b_select_index);
	return true;
}
NS_GT_END