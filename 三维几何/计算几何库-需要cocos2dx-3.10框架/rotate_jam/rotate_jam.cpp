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

NS_GT_END