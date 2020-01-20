/*
  *旋转卡壳算法系列实现
  *@date:2020年1月8日
  *@author:xiaohuaxiong
 */
#include "rotate_jam.h"
#include "matrix/matrix.h"
#include <functional>
#include <math.h>

USING_NS_CC;

NS_GT_BEGIN
/*
  *超直线的相交测试
 */
bool superline_intersect_test(const SuperLine2D &a, const SuperLine2D &b,Vec2 &intersect_point)
{
	//记录一个方法表,由于函数的输入参数不同,需要先包装成一个个的dummy函数才能使用这种算法
	//因此作者就不想再那么做了,读者可以自己实现
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
	else//线段与线段之间的相交测试
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
	//首先对半平面进行排序,其排序原则遵循极角坐标系的逆时针旋转特征,如果有两个直线方向相同,则靠左侧的在后
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
	//第二部,剔除掉平行且方向相同的直线,注意 剔除的原则
	HalfResultType   result_type = ResultType_Unboundary;
	int base_j = 0, erase_l = 1;
	const float scale = 80000.0f;
	Vec2  intersect_point;

	super_lines[0].line_type = LineType_Ray;
	super_lines[0].start_point = super_lines[0].start_point - super_lines[0].unknown *scale;

	//下一步进行逐直线计算
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
		//注意以下的代码判断的含义,如果没有交点,根据我们的假设,则说明:当前直线与前射线/线段的交集为空,此时算法无解
		if( !superline_intersect_test(super_line, super_lines[right - 1], intersect_point))
			return ResultType_Empty;//实际上有一种无界的情况在这里会被误认为是交集为空
		polygon_points[right] = intersect_point;
		//由于有新的交点的加入,当前直线与上一个超直线将形成新的界
		super_lines[right].line_type = LineType_Ray;
		super_lines[right].start_point = polygon_points[right];

		super_lines[right - 1].line_type = LineType_Segment;
		super_lines[right - 1].unknown = polygon_points[right];
	}
	//算法的最后,第一条直线新形成对最后一条直线的约束
	const SuperLine2D &line = super_lines[left];
	while (right > left + 1 && line_point_distance(line.start_point, line.unknown, polygon_points[right]) < 0.0f)
		--right;
	//检测最后是否相交
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
	//其他的情况必然是无界的
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
		//一下的处理方式与上面的算法不同,注意其关键的区别
		//沿着对面的点,引出垂直于当前边并朝向外侧的射线,计算其与当前边的交点,如果相交的话.
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
	  *第一步先定位到离目标边最远的顶点,注意因为该距离可能为负数,因此
	  *如果直接使用上面的算法模式,则可能会出现不正确的结果.
	 */
	int past_l = array_size2-1;
	while (cross(hull_points1[0], hull_points1[1], hull_points2[base_j]) < cross(hull_points1[0], hull_points1[1], hull_points2[past_l]))
	{
		base_j = past_l;
		past_l = (past_l -1 + array_size2)%array_size2;
	}

	Segment2D a = { hull_points1[0],hull_points1[1] }, b = { hull_points2[base_j],hull_points2[(base_j +1)%array_size2] };
	distance = segment_segment_minimum_distance(a,b,ahull_point,bhull_point);
	//另一侧
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
		//右
		int tripple_l = (right+1)%array_size;
		while(dot(start_point, final_point, hull_points[right]) < dot(start_point, final_point, hull_points[tripple_l]))
		{
			right = tripple_l;
			tripple_l = (tripple_l + 1) % array_size;
		}
		if (!index_l)
			up = right;
		//上
		tripple_l = (up+1)%array_size;
		while (cross(start_point, final_point, hull_points[up]) < cross(start_point, final_point, hull_points[tripple_l]))
		{
			up = tripple_l;
			tripple_l = (tripple_l+1)%array_size;
		}
		//左
		if (!index_l)
			left = up;
		tripple_l = (left+1)%array_size;
		while (dot(final_point, start_point, hull_points[left]) < dot(final_point, start_point, hull_points[tripple_l]))
		{
			left = tripple_l;
			tripple_l = (tripple_l + 1)%array_size;
		}
		//计算当前形成的外接矩形的面积
		float h = cross(start_point,final_point,hull_points[up])/length(start_point,final_point);//注意该行代码的含义
		Vec2 stride = hull_points[right] - hull_points[left];
		float w = dot(stride,normalize(start_point,final_point));
		float f = w * h;
		if (f < area)
		{
			area = f;
			l = left; r = right; b = index_l; u = up;
		}
	}
	//计算相关的外接矩形
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
		//右
		int tripple_l = (right + 1) % array_size;
		while (dot(start_point, final_point, hull_points[right]) < dot(start_point, final_point, hull_points[tripple_l]))
		{
			right = tripple_l;
			tripple_l = (tripple_l + 1) % array_size;
		}
		if (!index_l)
			up = right;
		//上
		tripple_l = (up + 1) % array_size;
		while (cross(start_point, final_point, hull_points[up]) < cross(start_point, final_point, hull_points[tripple_l]))
		{
			up = tripple_l;
			tripple_l = (tripple_l + 1) % array_size;
		}
		//左
		if (!index_l)
			left = up;
		tripple_l = (left + 1) % array_size;
		while (dot(final_point, start_point, hull_points[left]) < dot(final_point, start_point, hull_points[tripple_l]))
		{
			left = tripple_l;
			tripple_l = (tripple_l + 1) % array_size;
		}
		//计算当前形成的外接矩形的面积
		float h = cross(start_point, final_point, hull_points[up]) / length(start_point, final_point);//注意该行代码的含义
		Vec2 stride = hull_points[right] - hull_points[left];
		float w = dot(stride, normalize(start_point, final_point));
		float f = 2.0f * (w + h);
		if (f < perimeter)
		{
			perimeter = f;
			l = left; r = right; b = index_l; u = up;
		}
	}
	//计算相关的外接矩形
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
	//计算两个凸多边形的最小y顶点坐标,再计算的过程中,我们已经假设第一个顶点的y坐标轴最小,因此直接使用
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
		//检测需要调整那一条边
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
	//第一步,对顶点进行排序
	std::vector<const Vec2 *> points_tmp(array_size);
	memcpy(points_tmp.data(),points,sizeof(Vec2*) * array_size);
	//查找y坐标最小,或者x坐标较小的点
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
		//如果角度相同,则离参考点距离最近的排列位置更靠前
		return angle_a < angle_b || (angle_a == angle_b && d_x * d_x + d_y * d_y < f_x * f_x + f_y * f_y);//避免极角相同的点集
	};
	quick_sort_origin_type<const Vec2*>(points_tmp.data(), array_size - 1, compare_func);

	points[0] = f_point;
	memcpy(points +1,points_tmp.data(),sizeof(Vec2*) * (array_size - 1));

	polygon.reserve(array_size);
	polygon.push_back(f_point);
	polygon.push_back(points_tmp[0]);

	for (int index_j = 1; index_j < array_size - 1; ++index_j)
	{
		//对当前的点进行侦测
		const Vec2 *target_point = points_tmp.at(index_j);
		//检测是否与以前的向量的走向是一致的
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
	//循环进行旋转卡壳算法
	std::vector<const Vec2*>	polygon1,polygon2;
	int  remind_size = array_size;
	bool  swap_exchange = false;

	while (remind_size > 2)
	{
		std::vector<const Vec2 *>  &last_polygon = swap_exchange?polygon2:polygon1;
		std::vector<const Vec2 *> &now_polygon = swap_exchange ? polygon1:polygon2;

		now_polygon.clear();
		rotate_hull_base_method(disper_points.data(), remind_size,now_polygon);

		//将已经选出的凸包顶点冲原数组中移除掉,所需的时间为O(n)
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
	//对于最后剩下的顶点,需要额外的单独处理,目前暂时不足任何的动作
}
/*
  *与上面的函数相比,该函数的计算过程非常的复杂
  *其再角度的比较计算过程中,将伴随着坐标系的旋转
 */
void rotate_hull_base_method(const cocos2d::Vec2 **points, int array_size,int erase_index,const Vec2 &ortho_axis, std::vector<const cocos2d::Vec2*> &polygon)
{
	//第一步,对顶点进行排序
	int array_new_size = array_size - 1;
	std::vector<const Vec2 *> points_tmp(array_size);
	memcpy(points_tmp.data(), points, sizeof(Vec2*) * array_size);
	float fix_angle = radian_from(ortho_axis,Vec2::UNIT_Y);
	float sin_f = sinf(fix_angle), cos_f = cosf(fix_angle);
	//查找y坐标最小,或者x坐标较小的点
	const Vec2 *f_point = points[erase_index];
	points_tmp.erase(points_tmp.begin()+ erase_index);
	std::function<bool(const Vec2 *a, const Vec2 *b)>  compare_func = [f_point,sin_f,cos_f](const Vec2 *a, const Vec2 *b)->bool {
		const float d_x = a->x - f_point->x;
		const float d_y = a->y - f_point->y;

		const float f_x = b->x - f_point->x;
		const float f_y = b->y - f_point->y;

		float angle_a = atan2f(d_y * cos_f + d_x * sin_f, d_x * cos_f - d_y * sin_f);
		float angle_b = atan2f(f_y * cos_f + f_x * sin_f, f_x * cos_f - f_y * sin_f);
		//如果角度相同,则离参考点距离最近的排列位置更靠前
		return angle_a < angle_b || (angle_a == angle_b && d_x * d_x + d_y * d_y < f_x * f_x + f_y * f_y);//避免极角相同的点集
	};
	quick_sort_origin_type<const Vec2*>(points_tmp.data(), array_new_size, compare_func);

	points[0] = f_point;
	memcpy(points + 1, points_tmp.data(), sizeof(Vec2*) * array_new_size);

	polygon.reserve(array_size);
	polygon.push_back(f_point);
	polygon.push_back(points_tmp[0]);

	for (int index_j = 1; index_j < array_new_size; ++index_j)
	{
		//对当前的点进行侦测
		const Vec2 *target_point = points_tmp.at(index_j);
		//检测是否与以前的向量的走向是一致的
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
	//第一步筛选出y坐标最小的点
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
		//记录下第一次转弯的索引
		if (!corner_index)corner_index = polygon.size() - 1;

		//移动数据
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
		//计算与base_ptr所成的夹角最小的那个顶点,实际上下面的代码可以合并到上面的循环中,这里为了流程清晰,就分开了
		compare_base = tmp_array[0];
		erase_index = 0;
		base_ptr = polygon.back();
		for (int index_l = 1; index_l < remind_count; ++index_l)
		{
			if (cross(*base_ptr, *compare_base, *tmp_array[index_l]) < 0.0f)
				compare_base = tmp_array[index_l], erase_index = index_l;
		}
	}
	//如果有剩下的点,则需要单独的处理,其处理过程形式上类似,但是又有所不同
	if (remind_count == 2)
	{
		//对剩下的两个顶点排序,排序的原则与上面一样
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
	//首先计算离散点集的螺旋线
	int  split_index = rotate_hull_spiral_line(points, spiral_line_points);
	//根据索引split_index,将螺旋线划分为外侧以及内侧,另外需要计算内侧的终止索引
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
	//对螺旋线进行三角剖分,分解算法的核心思想仍然是旋转平行线
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
	//需要额外的连线
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
	//针对剩余的点,将最后一点与从start_index-->array_size - 2的点逐个的连接即可
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
	//计算两凸多边形的公切线,如果没有找到,则说明,一个多边形必然在另一个多边形之内
	int a_index = 0, b_index = 0;
	int a_select_index = 0, b_select_index = 0;
	bool b_found = false;

	while (!b_found && (a_index < array_size1 || b_index < array_size2))
	{
		int a_next = (a_select_index+1)%array_size1;
		int a_prev = (a_select_index-1 +array_size1)%array_size1;

		int b_next = (b_select_index +1)%array_size2;
		int b_prev = (b_select_index -1+array_size2)%array_size2;
		//需要动态选择分支类型,原因是可能会出现A嵌套于B,或者B嵌套于A中的情况,如果只有一种,程序将限于死循环
		//虽然有动态选择,但是总的运行时间仍然是O(m+n)
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
	//检测是否查找到,如果条件为真,则必有一个多边形嵌套在另一个之内
	if (!b_found)
	{
		//任意取两个顶点
		if (cross(polygon1[0], polygon1[1], polygon2[0]) > 0.0f)
			polygon_union = polygon1;
		else
			polygon_union = polygon2;
		return;
	}
	//剩下的情况必然为或者交叠,或者分离,此时需要用上一次求出的公切线继续求解
	int a_next_tangent = a_select_index;
	int b_next_tangent = b_select_index;
	//
	a_index = a_select_index;
	b_index = b_select_index;
	//求下一个公切线,需要交替的交换公切线形成的顺序
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
			//注意处理选择语句的时候优先级是动态的
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
		//此时需要将中间的顶点写入到数组中,至于如何选择,则有着相当严格的区别方法
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
  *辅助函数
  *是否线段b在线段a所形成的直线的同一侧
 */
bool static_rotate_hull_segment_same_side(const Vec2 &astart_point,const Vec2 &afinal_point,const Vec2 &bstart_point,const Vec2 &bfinal_point)
{
	return cross(astart_point,afinal_point,bstart_point) * cross(astart_point,afinal_point,bfinal_point) > 0.0f;
}
/*
  *辅助函数
  *求离最近的公切线最近的两个相交线段
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
	//计算第一个公切线
	int a_index = 0, b_index = 0;
	int a_select_index = 0, b_select_index = 0;
	bool b_found = false;

	while (!b_found && (a_index < array_size1 || b_index < array_size2))
	{
		int a_next = (a_select_index + 1) % array_size1;
		int a_prev = (a_select_index - 1 + array_size1) % array_size1;

		int b_next = (b_select_index + 1) % array_size2;
		int b_prev = (b_select_index - 1 + array_size2) % array_size2;
		//需要动态选择分支类型,原因是可能会出现A嵌套于B,或者B嵌套于A中的情况,如果只有一种,程序将限于死循环
		//虽然有动态选择,但是总的运行时间仍然是O(m+n)
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
	//使用上面计算出的结果进一步去判断是否一个多边形包含另一个
	if (!b_found)
	{
		if (cross(polygon1[0], polygon1[1], polygon2[0]) > 0.0f)
			polygon_intersect = polygon2;
		else
			polygon_intersect = polygon1;
		return true;
	}
	//如果找到了公切线,此时就需要判断两多边形是否存在交点
	a_index = b_index = 0;
	int a_compare_index = a_select_index, b_compare_index = b_select_index;
	b_found = static_rotate_polygon_near_segment(polygon1, polygon2, a_select_index, b_select_index,a_compare_index,b_compare_index);
	//此时交集为空
	if (!b_found)return false;
	//接下来,逐个的求公切线,并计算相关的交点
	int a_next_tangent = a_select_index;
	int b_next_tangent = b_select_index;
	//
	a_index = a_select_index;
	b_index = b_select_index;
	//求下一个公切线,需要交替的交换公切线形成的顺序
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
			//注意处理选择语句的时候优先级是动态的
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
		//此时需要将中间的顶点写入到数组中,至于如何选择,则有着相当严格的区别方法
		int a_new_index = 0, b_new_index = 0;
		if (need_exchange)
		{
			bool b = segment_segment_intersect_test(polygon1[a_compare_index], polygon1[(a_compare_index + 1) % array_size1], polygon2[b_compare_index], polygon2[(b_compare_index - 1 + array_size2) % array_size2], intersect_point);
			assert(b);
			vector_fast_push_back(polygon_intersect, intersect_point);

			bool b_intersect = static_rotate_polygon_near_segment(polygon2, polygon1, b_next_tangent, a_next_tangent, b_new_index, a_new_index);
			assert(b_intersect);
			//求当前的线段交点的索引
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