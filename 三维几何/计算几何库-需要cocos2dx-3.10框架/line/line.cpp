/*
  *直线,线段算法实现
  *2019年6月22日
  *@author:xiaohuaxiong
 */
#include "line.h"
#include "matrix/matrix.h"
#include "cycle_sphere/cycle_sphere.h"
#include "aabb_obb/aabb_obb.h"
#include "triangle/triangle.h"
#include<list>
using namespace cocos2d;
NS_GT_BEGIN
void  line_create(Line &a, const cocos2d::Vec3 &start_point, const cocos2d::Vec3 &final_point)
{
	a.start_point = start_point;
	a.direction = normalize(final_point - start_point);
}

void segment_create(Segment &segment, const cocos2d::Vec3 &a, const cocos2d::Vec3 &b)
{
	segment.start_point = a;
	segment.final_point = b;
}

void ray_create(Ray &ray, const cocos2d::Vec3 &origin, const cocos2d::Vec3 &direction)
{
	ray.origin = origin;
	ray.direction = normalize(direction);
}

void plane_create(Plane &plane, const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c)
{
	plane.normal = gt::cross_normalize(b - a, c - a);
	plane.distance = gt::dot(plane.normal,a);
}

void plane_create(Plane &plane, const cocos2d::Vec3 &normal, const cocos2d::Vec3 &point)
{
	plane.normal = gt::normalize(normal);
	plane.distance = gt::dot(plane.normal,point);
}

void plane_create(Plane &plane, const cocos2d::Vec3 &normal, float d)
{
	float length = normal.length();
	plane.distance = d / length;
	plane.normal = normal / length;
}

float line_line_minimum_distance(const Line &Q1, const Line &Q2,cocos2d::Vec3 &intersect_apoint, cocos2d::Vec3 & intersect_bpoint)
{
	float b = dot(Q1.direction,Q2.direction);
	Vec3 r = Q1.start_point - Q2.start_point;
	float c = dot(Q1.direction, r);
	float f = dot(Q2.direction,r);

	float  coef = b * b - 1;
	if (fabs(coef) >= 0.001f)//两个直线不平行
	{
		float s = (c - b * f)/coef;
		float t = (b* c  - f)/coef;

		intersect_apoint = Q1.start_point + Q1.direction * s;
		intersect_bpoint = Q2.start_point + Q2.direction * t;

		return (intersect_bpoint - intersect_apoint).length();
	}
	//平行直线
	intersect_apoint = Q1.start_point;
	float  project = dot(r,Q2.direction);
	intersect_bpoint = Q2.start_point + Q2.direction * project;
	return (intersect_bpoint - intersect_apoint).length();
}

float segment_segment_minimum_distance(const Segment &l1, const Segment &l2, cocos2d::Vec3 &intersect_apoint, cocos2d::Vec3 & intersect_bpoint)
{
	//其算法思想跟直线的最近点求法类似,但是实现的过程要复杂很多
	Vec3 d1 = l1.final_point  - l1.start_point;
	Vec3 d2 = l2.final_point - l2.start_point;
	Vec3 r = l1.start_point - l2.start_point;
	float a = dot(d1,d1);
	float e = dot(d2,d2);
	float b = dot(d1,d2);
	float f = dot(d2,r);

	float s = 0, t = 0;
	//按直线之间的最近距离计算最近点
	float c = dot(r,d1);
	float denom = a * e - b * b;
	if (denom != 0)
		s = clampf(0, 1, (b * f - c * e)/denom);
	t = (s * b + f)/e;
	//求边界
	if (t < 0.0f)
	{
		t = 0.0f;
		s = clampf(0,1,-c/a);
	}
	else if (t > 1.0f)
	{
		t = 1.0f;
		s = clampf(0,1,(b - c)/a);
	}
	//求目标点以及距离
	intersect_apoint = l1.start_point + d1 * s;
	intersect_bpoint = l2.start_point + d2 * t;
	return (intersect_apoint - intersect_bpoint).length();
}

float Plane::distanceTo(const cocos2d::Vec3 &point)const
{
	return gt::dot(normal,point) - distance;
}

float plane_point_distance(const cocos2d::Vec3 &point, const Plane &plane, cocos2d::Vec3 &proj_point)
{
	float distance = plane.distanceTo(point);
	proj_point = point - plane.normal * distance;

	return distance;
}

float line_point_minimum_distance(const cocos2d::Vec3 &point, const Segment &segment,cocos2d::Vec3 *intersect_point)
{
	Vec3 ab = segment.final_point - segment.start_point;
	Vec3 ac = point - segment.start_point;

	float angle = gt::dot(ab,ac);
	float length_ab = gt::dot(ab,ab);

	if (!intersect_point)
	{
		if (angle <= 0)
			return (point - segment.start_point).length();
		if (angle >= length_ab)
			return (point - segment.final_point).length();
		return gt::dot(ac,ac) - angle * angle/length_ab;
	}

	float t = clampf(0, length_ab, angle);
	*intersect_point = segment.start_point + ab * (t/length_ab);
	return (*intersect_point - segment.start_point).length();
}

bool ray_sphere_intersect_test(const Ray &ray, const Sphere &sphere,cocos2d::Vec3 &intersect_point)
{
	const Vec3 c = ray.origin - sphere.center;
	float r2 = sphere.radius * sphere.radius;
	float b = dot(c,ray.direction);
	float a = length2(c);
	//此时射线偏离了球体
	if (a > r2 && b > 0)
		return false;
	float t = b * b - a + r2;
	//二次方程的跟的判别式小于0,此时方程无解
	if (t < 0)
		return false;
	float s = -b - sqrtf(t);
	if (s < 0)
		s = 0;
	intersect_point = ray.origin + ray.direction * s;
	return true;
}

bool segment_sphere_intersect_test(const Segment &segment, const Sphere &sphere)
{
	const Vec3 direction = segment.final_point - segment.start_point;
	const Vec3 normal = normalize(direction);
	const Vec3 c = segment.start_point - sphere.center;
	float r2 = sphere.radius * sphere.radius;
	float l = length2(c);
	float b = dot(c,normal);
	if (l > r2 && b > 0)
		return false;
	float t = b* b - (l-r2);
	if (t < 0)
		return false;
	float ft = sqrtf(t);
	float s2 = -b + ft;
	//
	float s = -b - ft;
	//若条件为反时,此时交点在线段的反延长线上,或者在线段的正延长线上
	return s2 >= 0 && s <= direction.length();
}

bool ray_aabb_intersect_test(const Ray &ray, const AABB &aabb)
{
	float min_f = 0.0f;
	float max_f = FLT_MAX;
	//x
	if (ray.direction.x == 0)
	{
		if (ray.origin.x < aabb.bb_max.x || ray.origin.x > aabb.bb_max.x)
			return false;
	}
	else
	{
		float d = 1.0f / ray.direction.x;
		float f1 = (aabb.bb_min.x - ray.origin.x) * d;
		float f2 = (aabb.bb_max.x - ray.origin.x) * d;

		max_f = fminf(max_f,fmaxf(f1,f2));
		min_f = fmaxf(min_f,fminf(f1,f2));
		if (min_f > max_f)
			return false;
	}
	//y
	if (ray.direction.y == 0)
	{
		if (ray.origin.y < aabb.bb_max.y || ray.origin.y > aabb.bb_max.y)
			return false;
	}
	else
	{
		float d = 1.0f / ray.direction.y;
		float f1 = (aabb.bb_min.y - ray.origin.y) * d;
		float f2 = (aabb.bb_max.y - ray.origin.y) * d;

		float t1 = fmaxf(f1,f2);
		float t2 = fminf(f1,f2);

		if (t1 < min_f)
			return false;
		min_f = fmaxf(min_f,t2);
		max_f = fminf(max_f,t1);
	}
	//z
	if (ray.direction.z == 0)
	{
		if (ray.origin.z < aabb.bb_min.z || ray.origin.z > aabb.bb_max.z)
			return false;
	}
	else
	{
		float d = 1.0f / ray.direction.z;
		float f1 = (aabb.bb_min.z - ray.origin.z) *d;
		float f2 = (aabb.bb_max.z - ray.origin.z) * d;

		float t1 = fmaxf(f1,f2);
		float t2 = fminf(f1,f2);
		if (t1 < min_f)
			return false;
		min_f = fmaxf(min_f,t2);
		max_f = fminf(max_f,t1);
	}
	return min_f <= max_f;
}
/*
  *该算法并没有针对OBB进行优化
  *该算法实现的我另一个版本是分离轴测试
  *详细实现请参见aabb_obb.cpp文件
 */
bool ray_obb_intersect_test(const Ray &ray, const OBB &obb)
{
	//坐标系变换
	gt::AABB   aabb;
	aabb_create(aabb,- obb.extent,obb.extent);

	//变换到OBB的局部坐标系中
	Vec3 origin = ray.origin - obb.center;
	gt::Ray  secondary_ray = {
		Vec3(origin.dot(obb.xaxis), origin.dot(obb.yaxis),origin.dot(obb.zaxis)),
		Vec3(ray.direction.dot(obb.xaxis),ray.direction.dot(obb.yaxis),ray.direction.dot(obb.zaxis)),
	};

	return ray_aabb_intersect_test(secondary_ray, aabb);
}

bool line_triangle_intersect_test(const Line &line, const Triangle &triangle)
{
	//判断是否直线平行于三角平面,或者与三角平面共线
	const Vec3 pa = triangle.a - line.start_point;
	const Vec3 pb = triangle.b - line.start_point;
	const Vec3 pc = triangle.c - line.start_point;
	//三角平面的法线,以下代码并没有经过优化
	const Vec3 ab = triangle.b - triangle.a;
	const Vec3 bc = triangle.c - triangle.b;
	const Vec3 normal = cross_normalize(ab, bc);
	float f = dot(normal,line.direction);
	//平行或者共面
	if (fabsf(f) <= 0.001f)
	{
		float distance = dot(normal,line.start_point - triangle.a);
		//平行
		if (fabsf(distance) > 0.001f)
			return false;
		//共面,AB
		float f1 = dot(line.direction,triangle.a - line.start_point);
		float f2 = dot(line.direction,triangle.b - line.start_point);
		if (f1 * f2 <= 0)
			return true;
		//BC
		float f3 = dot(line.direction,triangle.c - line.start_point);
		if (f2 * f3 <= 0)
			return true;
		//CA
		if (f3 * f1 <= 0)
			return true;
		return false;
	}
	//求标量三重积
	float f1 = dot(cross(pa,pb), line.direction);
	float f2 = dot(cross(pb,pc),line.direction);
	float f3 = dot(cross(pc,pa),line.direction);

	return f1 * f2 >= 0 && f2 * f3 >= 0;
}

bool segment_triangle_intersect_test(const Segment &segment, const Triangle &triangle)
{
	const Vec3 pq = segment.final_point - segment.start_point;
	const Vec3 ab = triangle.b - triangle.a;
	const Vec3 ac = triangle.c - triangle.a;
	const Vec3 normal = cross(ab,ac);
	//求线性方程组的系数矩阵的行列式
	float d = dot(pq,normal);
	//如果系数矩阵的行列式等于0,则说明线段平行于三角平面或者于平面共面
	if (fabsf(d) <= 0.001f)
	{
		const Vec3 v1 = segment.start_point - triangle.a;
		float distance = dot(normal,v1);
		if (fabsf(distance) > 0.001f)
			return false;
		//检测三角形面的边与线段的交线
		//AB
		const Vec3 v2 = triangle.b - segment.start_point;
		const Vec3 v4 = segment.final_point - triangle.a;
		float f1 = -dot(cross(pq,v1),normal);
		float f2 = dot(cross(pq,v2),normal);
		float f3 = dot(cross(ab,v1),normal);
		float f4 = dot(cross(ab,v4),normal);
		if (f1 * f2 <= 0 && f3 * f4 <= 0)
			return true;
		//BC
		const Vec3 v3 = triangle.c - segment.start_point;
		const Vec3 v5 = segment.start_point - triangle.b;
		const Vec3 v6 = segment.final_point - triangle.c;
		const Vec3 bc = triangle.c - triangle.b;
		f1 = dot(cross(pq,v2),normal);
		f2 = dot(cross(pq,v3),normal);
		f3 = dot(cross(bc,v5),normal);
		f4 = dot(cross(bc,v6),normal);
		if (f1 * f2 <= 0 && f3 *f4 <= 0)
			return true;
		//CA
		f1 = dot(cross(pq,v3),normal);
		f2 = -dot(cross(pq,v1),normal);
		f3 = -dot(cross(ac,segment.start_point - triangle.c),normal);
		f4 = -dot(cross(ac,segment.final_point -triangle.c),normal);
		if (f1 * f2 <= 0 && f3 * f4 <= 0)
			return true;
		//
		const Vec3 normal_ab = cross(ab,normal);
		const Vec3 normal_bc = cross(bc,normal);
		const Vec3 normal_ca = cross(normal,ac);
		if (dot(normal_ab, v1) <= 0 && dot(normal_ab, v4) <= 0.0f && dot(normal_bc, v5) <= 0.0f && dot(normal_bc, v6) <= 0.0f && dot(normal_ca, v6) <= 0.0f && dot(normal_ca, v1) <= 0.0f)
			return true;
		return false;
	}
	//求取各个系数矩阵的行列式
	const Vec3 pa = triangle.a - segment.start_point;
	float l = 1.0f / d;
	float t = dot(pa,normal) * l;
	if (t <0 || t > 1.0f)
		return false;
	const Vec3 fv = cross(pq,pa);
	float v = -dot(ac, fv) * l;
	if (v < 0 || v >1.0f)
		return false;
	float w = dot(ab, fv) * l;
	if (w <0 || w >1.0f)
		return false;
	return v + w <= 1.0f;
}

bool plane_plane_intersect_test(const Plane &plane1, const Plane &plane2, Line &line)
{
	//求两个平面的叉乘向量
	const Vec3 d = cross(plane1.normal,plane2.normal);
	float l = length2(d);
	//平行/或者共面
	if (l < 0.001f)
	{
		//求出平面2上一点
		const Vec3 target_point(10,10,10);
		const Vec3 inside_point = target_point - plane2.normal * plane2.distanceTo(target_point);
		//平行
		if(plane1.distanceTo(inside_point) > 0)
			return false;
		//共面,随机衍生出一条直线
		line.start_point = inside_point;
		const Vec3 other_point(10,20,20);
		const Vec3 other_inside_point = other_point - plane2.normal * plane2.distanceTo(other_point);
		line.direction = normalize(other_inside_point - inside_point);
		return true;
	}
	//求公共直线
	line.direction = normalize(d);
	//使用Cramer法则求出直线上一点的坐标 
	float r = dot(plane1.normal, plane2.normal);
	float D = 1.0f - r * r;
	float K1 = (plane1.distance - r * plane2.distance)/D;
	float K2 = (plane2.distance - r * plane1.distance)/D;

	line.start_point = plane1.normal * K1 + plane2.normal * K2;
	return true;
}

bool plane_plane_plane_intersect_test(const Plane &plane1, const Plane &plane2, const Plane &plane3/*, std::vector<Line> &lines,*/, cocos2d::Vec3 &intersect_point)
{
	const Vec3 normal = cross(plane2.normal, plane3.normal);
	float demon = dot(plane1.normal,normal);
	if (fabsf(demon) < 0.001f)
		return false;
	float f1 = plane1.distance/demon;
	float f2 = plane2.distance / demon;
	float f3 = plane3.distance / demon;

	intersect_point = normal * f1 + cross(plane3.normal	,plane1.normal) * f2 + cross(plane1.normal,plane2.normal) * f3;
	return true;
}

void bresenham_line_algorithm(int start_x, int start_y, int final_x, int final_y, int horizontal_num, int vertivcal_num, std::vector<cocos2d::Vec2> &location_array)
{
	int delta_x = final_x - start_x;
	int delta_y = final_y - start_y;
	int abs_x = delta_x >= 0 ? delta_x : -delta_x;
	int abs_y = delta_y >= 0 ? delta_y : -delta_y;

	int step_x = final_x > start_x ? 1 : (final_x < start_x ? -1 : 0);
	int step_y = final_y > start_y ? 1 : (final_y < start_y ? -1 : 0);
	
	if (abs_x >= abs_y)
	{
		int loops_count = abs_x + 1;
		int e = 0;
		location_array.resize(loops_count);
		for (int x = 0; x < loops_count; ++x)
		{
			e += abs_y << 1;
			location_array[x] = Vec2(start_x, start_y);
			start_x += step_x;
			if (e >= abs_x)
			{
				start_y += step_y;
				e -= abs_x << 1;
			}
		}
	}
	else
	{
		int loops_count = abs_y + 1;
		int e = 0;
		location_array.resize(loops_count);
		for (int y = 0; y < loops_count; ++y)
		{
			location_array[y] = Vec2(start_x, start_y);
			e += abs_x << 1;
			start_y += step_y;
			if (e >= abs_y)
			{
				start_x += step_x;
				e -= abs_y << 1;
			}
		}
	}
}

void segment_grid3d_intersect_test(const Segment &segment, int horizontal_num, int vertical_num, int depth_num, const cocos2d::Vec3 &extent_unit, std::vector<cocos2d::Vec3> &intersect_array)
{
	int start_x = segment.start_point.x / extent_unit.x;
	int start_y = segment.start_point.y / extent_unit.y;
	int start_z = segment.start_point.z / extent_unit.z;
	Vec3 pixel(start_x * extent_unit.x, start_y * extent_unit.y,start_z * extent_unit.z);

	Vec3 delta, accumulate;
	int step_x, step_y,step_z;
	const Vec3 normal = normalize(segment.final_point - segment.start_point);

	if (normal.x == 0.0f)
	{
		accumulate.x = FLT_MAX;
		step_x = 0;
	}
	else if (normal.x > 0.0f)
	{
		accumulate.x = (pixel.x + extent_unit.x - segment.start_point.x) / normal.x;
		step_x = 1;
	}
	else
	{
		accumulate.x = (pixel.x - segment.start_point.x) / normal.x;
		step_x = -1;
	}

	if (normal.y == 0.0f)
	{
		accumulate.y = FLT_MAX;
		step_y = 0;
	}
	else if (normal.y > 0.0f)
	{
		accumulate.y = (pixel.y + extent_unit.y - segment.start_point.y) / normal.y;
		step_y = 1;
	}
	else
	{
		accumulate.y = (pixel.y - segment.start_point.y) / normal.y;
		step_y = -1;
	}

	if (normal.z == 0.0f)
	{
		accumulate.z = FLT_MAX;
		step_z = 0;
	}
	else if (normal.z > 0)
	{
		accumulate.z = (pixel.z + extent_unit.z - segment.start_point.z) / normal.z;
		step_z = 1;
	}
	else
	{
		accumulate.z = (pixel.z - segment.start_point.z) / normal.z;
		step_z = -1;
	}

	delta.x = step_x * extent_unit.x / (normal.x == 0.0f ? 0.0001f : normal.x);
	delta.y = step_y * extent_unit.y / (normal.y == 0.0f ? 0.0001f : normal.y);
	delta.z = step_z * extent_unit.z / (normal.z == 0.0f ? 0.0001f : normal.z);

	int loop_count = 0;

	int final_x = segment.final_point.x / extent_unit.x;
	int final_y = segment.final_point.y / extent_unit.y;
	int final_z = segment.final_point.z / extent_unit.z;
	int total_count = horizontal_num + vertical_num + depth_num;

	intersect_array.reserve(total_count);
	while (loop_count < total_count)
	{
		if (start_x >= 0 && start_x < horizontal_num && start_y >= 0 && start_y < vertical_num && start_z >=0 && start_z < depth_num)
		{
			intersect_array.push_back(Vec3(start_x,start_y,start_z));
		}
		if (start_x == final_x && start_y == final_y && start_z == final_z)
			break;
		float min_f = fminf(accumulate.x,fminf(accumulate.y, accumulate.z));
		if (min_f == accumulate.x)
		{
			accumulate.x += delta.x;
			start_x += step_x;
		}

		if (min_f == accumulate.y)
		{
			accumulate.y += delta.y;
			start_y += step_y;
		}

		if (min_f == accumulate.z)
		{
			accumulate.z += delta.z;
			start_z += step_z;
		}

		++loop_count;
	}
}

void segment_grid2d_intersect_test(const cocos2d::Vec2 &start_point, const cocos2d::Vec2 &final_point, int horizontal_num, int vertical_num, const cocos2d::Vec2 &extent_unit,std::vector<cocos2d::Vec2> &intersect_array)
{
	int start_x = start_point.x / extent_unit.x;
	int start_y = start_point.y / extent_unit.y;
	Vec2 pixel(start_x * extent_unit.x, start_y * extent_unit.y);

	Vec3 delta, accumulate;
	int step_x, step_y;
	const Vec2 normal = normalize(final_point - start_point);
	//求平行空间的间隔
	if (normal.x == 0.0f)
	{
		accumulate.x = FLT_MAX;
		step_x = 0;
	}
	else if (normal.x > 0.0f)
	{
		accumulate.x = (pixel.x + extent_unit.x - start_point.x) / normal.x;
		step_x = 1;
	}
	else
	{
		accumulate.x = (pixel.x - start_point.x) / normal.x;
		step_x = -1;
	}

	if (normal.y == 0.0f)
	{
		accumulate.y = FLT_MAX;
		step_y = 0;
	}
	else if (normal.y > 0.0f)
	{
		accumulate.y = (pixel.y + extent_unit.y - start_point.y) / normal.y;
		step_y = 1;
	}
	else
	{
		accumulate.y = (pixel.y - start_point.y) / normal.y;
		step_y = -1;
	}


	delta.x = step_x * extent_unit.x / (normal.x == 0.0f ? 0.0001f : normal.x);
	delta.y = step_y * extent_unit.y / (normal.y == 0.0f ? 0.0001f : normal.y);

	int loop_count = 0;

	int final_x = final_point.x / extent_unit.x;
	int final_y = final_point.y / extent_unit.y;

	intersect_array.reserve(horizontal_num + vertical_num);
	while (loop_count < horizontal_num + vertical_num)
	{
		if (start_x >= 0 && start_x < horizontal_num && start_y >= 0 && start_y < vertical_num)
		{
			intersect_array.push_back(Vec2(start_x, start_y));
		}
		if (start_x == final_x && start_y == final_y)
			break;
		float min_f = fminf(accumulate.x, accumulate.y);
		if (min_f == accumulate.x)
		{
			accumulate.x += delta.x;
			start_x += step_x;
		}

		if (min_f == accumulate.y)
		{
			accumulate.y += delta.y;
			start_y += step_y;
		}

		++loop_count;
	}
}

//not Optimal
template<typename TM>
void quick_sort(TM *source, int tk_num, std::function<bool(const TM &a, const TM &b)> &compare_func)
{
	//排序算法目前先采用插入排序,后面我们将会使用归并排序
	TM   *bubble = new TM[tk_num];
	int     step = 1, half = tk_num / 2;
	TM  *t1 = source, *t2 = bubble;
	for (; step < tk_num; step *= 2)
	{
		int  base_j = 0;
		//compare and exchange
		for (int index_j = 0; index_j < tk_num; index_j += step * 2)
		{
			int  base_j = index_j;
			int  l_index = index_j;
			int  other_j = index_j + step;
			int l_boundary = other_j < tk_num ? other_j : tk_num;
			int r_boundary = other_j + step < tk_num ? other_j + step : tk_num;

			while (base_j < l_boundary && other_j < r_boundary)//边界
			{
				if (compare_func(t1[base_j], t1[other_j]))
				{
					t2[l_index] = t1[base_j];
					++base_j;
				}
				else
				{
					t2[l_index] = t1[other_j];
					++other_j;
				}
				++l_index;
			}
			//检查是否有某些元素还没有完全参与计算
			for (; base_j < l_boundary; ++base_j, ++l_index) t2[l_index] = t1[base_j];
			for (; other_j < r_boundary; ++other_j, ++l_index)t2[l_index] = t1[other_j];
		}
		TM *t = t1;
		t1 = t2; t2 = t;
	}
	if (t1 != source)
		memcpy(source, t1, sizeof(TM) * tk_num);

	delete[] bubble;
}
//另一种排序算法,与之前的相比较而言,区别在于比较函数
template<typename TM>
void quick_sort_origin_type(TM *source, int tk_num, std::function<bool(const TM a, const TM b)> &compare_func)
{
	//排序算法目前先采用插入排序,后面我们将会使用归并排序
	TM   *bubble = new TM[tk_num];
	int     step = 1, half = tk_num / 2;
	TM  *t1 = source, *t2 = bubble;
	for (; step < tk_num; step *= 2)
	{
		int  base_j = 0;
		//compare and exchange
		for (int index_j = 0; index_j < tk_num; index_j += step * 2)
		{
			int  base_j = index_j;
			int  l_index = index_j;
			int  other_j = index_j + step;
			int l_boundary = other_j < tk_num ? other_j : tk_num;
			int r_boundary = other_j + step < tk_num ? other_j + step : tk_num;

			while (base_j < l_boundary && other_j < r_boundary)//边界
			{
				if (compare_func(t1[base_j], t1[other_j]))
				{
					t2[l_index] = t1[base_j];
					++base_j;
				}
				else
				{
					t2[l_index] = t1[other_j];
					++other_j;
				}
				++l_index;
			}
			//检查是否有某些元素还没有完全参与计算
			for (; base_j < l_boundary; ++base_j, ++l_index) t2[l_index] = t1[base_j];
			for (; other_j < r_boundary; ++other_j, ++l_index)t2[l_index] = t1[other_j];
		}
		TM *t = t1;
		t1 = t2; t2 = t;
	}
	if (t1 != source)
		memcpy(source, t1, sizeof(TM) * tk_num);

	delete[] bubble;
}

struct VPoint
{
	Vec2  point;
	int      point_type;//0代表起始端点,1代表终止端点
	const Segment2D   *segment;
};

static cocos2d::Node *s_layer = nullptr;
void  set_layer(cocos2d::Node *layer)
{
	s_layer = layer;
}
//将值插入到链表队列中
void static_list_insert(std::list<VPoint *> &list_segment,VPoint  *p)
{
	auto it = list_segment.begin();
	while (it != list_segment.end())
	{
		VPoint  *q = *it;
		const Segment2D &segment = *q->segment;
		//计算线段与扫除线的交点
		float d_x = p->point.x - segment.start_point.x;
		float y = segment.start_point.y;
		if (segment.final_point.x != segment.start_point.x)
			y += d_x * (segment.final_point.y - segment.start_point.y) / (segment.final_point.x - segment.start_point.x);
		if (p->point.y > y)
			break;
		++it;
	}
	list_segment.insert(it, p);
}

void static_list_delete(std::list<VPoint*> &list_segment, VPoint *p)
{
	for (auto it = list_segment.begin(); it != list_segment.end(); ++it)
	{
		if ((*it)->segment == p->segment)
		{
			list_segment.erase(it);
			break;
		}
	}
}

VPoint  *static_list_get_above(std::list<VPoint*> &list_segment, VPoint *p)
{
	auto it = list_segment.begin();
	auto above = list_segment.end();
	for (; it != list_segment.end(); ++it)
	{
		if (*it == p)
			break;
		above = it;
	}
	return it != list_segment.end() && above  != list_segment.end()?*above :nullptr;
}

VPoint *static_list_get_below(std::list<VPoint *> &list_segment, VPoint *p)
{
	auto it = list_segment.begin();
	for (; it != list_segment.end(); ++it)
	{
		if (*it == p)
			break;
	}
	return it != list_segment.end() && ++it != list_segment.end() ?*it:nullptr;
}

bool segments2d_N_intersect_test(const Segment2D *line_array, int line_size)
{
	//第一遍先预处理,或者假设线段已经全部预处理过了
	//处理的规则为:起始端点的相关坐标x一定小于等于终止端点的坐标x,如果相等则y分量一定要小于等于终止坐标的y分量
	std::vector<VPoint>   point_vec(line_size * 2);
	for (int index_l = 0; index_l < line_size; ++index_l)
	{
		auto &line = point_vec.at(index_l);
		const Segment2D &origin_line = line_array[index_l];
		VPoint   vpoint = { origin_line.start_point,0,line_array + index_l };

		point_vec[index_l * 2] = vpoint;

		vpoint.point = origin_line.final_point;
		vpoint.point_type = 1;
		point_vec[index_l * 2 + 1] = vpoint;
	}
	std::function<bool(const VPoint &a, const VPoint &b)>  compare_func = [](const VPoint &a, const VPoint &b)->bool {
		if (a.point.x < b.point.x)
			return true;
		if (a.point.x == b.point.x)
		{
			if (a.point.y < b.point.y)
				return true;
			if (a.point.y == b.point.y)
				return a.point_type > b.point_type;
		}
		return false;
		//以上判断可以归结为一句代码
		//return a.point.x < b.point.x || a.point.x == b.point.x && (a.point.y < b.point.y || a.point.y == b.point.y && a.point_type > b.point_type);
	};
	quick_sort<VPoint>(point_vec.data(),(int) point_vec.size(), compare_func);
	//std::sort<VPoint>(point_vec.begin(), point_vec.end(), compare_func);
	//针对以上所有的顶点,遍历
	//std::set<VPoint*>  set_segment;
	//已知set_queue中数据的排列是降序的,因此在实际的实现过程中,可以使用更为快速的set实现
	//这里之所以没有使用,是因为std::set针对指针的排序是固定的,无法变通的,因此如果有需要使用
	//算法的快速实现的需求,可以自定义实现
	std::list<VPoint *>  list_segment;

	//set_segment.key_comp();
	for (int index_l = 0; index_l < point_vec.size(); ++index_l)
	{
		VPoint   *p = point_vec.data() + index_l;
		//计算相关的键值,键值的大小跟扫除线的x坐标有关,并且跟扫除线与线段的交点的y坐标也有关系
		int key_code = p->point.x * 1000;
		//如果是起始端点
		if (p->point_type == 0)
		{
			static_list_insert(list_segment, p);
			//检查是其上/其下是否有相交的线段
			VPoint  *a, *b;
			if ((a = static_list_get_above(list_segment, p)) && segment_segment_intersect_test(*p->segment, *a->segment) || (b = static_list_get_below(list_segment, p)) && segment_segment_intersect_test(*p->segment, *b->segment))
				return true;
		}
		else
		{
			VPoint  *a, *b;
			if ((a = static_list_get_below(list_segment, p)) && (b = static_list_get_below(list_segment, p)) && segment_segment_intersect_test(*a->segment, *b->segment))
				return true;
			static_list_delete(list_segment, p);
		}
	}
	return false;
}

bool segment_segment_intersect_test(const Segment2D &a, const Segment2D &b)
{
	Vec2 a1 = b.start_point - a.start_point;

	Vec2 direction = a.final_point - a.start_point;
	Vec2 d2 = b.final_point - b.start_point;

	return cross(direction, a1) * cross(direction, b.final_point - a.start_point) <= 0 && cross(d2,a1) * cross(d2,a.final_point - b.start_point) >= 0;
}

bool segment_segment_intersect_test(const Segment2D &a, const Segment2D &b,Vec2 &intersect_point)
{
	Vec2 a1 = b.start_point - a.start_point;
	Vec2 a2 = b.final_point - a.start_point;

	Vec2 d1 = a.final_point - a.start_point;
	Vec2 d2 = b.final_point - b.start_point;

	float fa = cross(a1, d1);
	float fb = cross(d1, a2);
	float f2 = cross(d2, a1) * cross(d2, a.final_point - b.start_point);

	if (fa * fb >= 0 && f2 >= 0)
	{
		intersect_point = b.start_point + d2 * (fa/(fa+fb));
		return true;
	}

	return false;
}
/*
  *线段端点的类型
 */
enum SegmentEndPointType
{
	PointType_Origin = 0,//线段的起始端点
	PointType_Intersect = 1,//线段之间的交叉点
	PointType_Over = 2,//线段的终止端点
};

struct SegmentEndPoint
{
	Vec2  end_point;
	Segment2D  *segment,*other;
	SegmentEndPointType point_type;
};
//将一个事件点所代表的直线加入到扫描线状态中
//在第一版中,只实现基本的算法
//在第二版中,我们将考虑各种退化情况
void static_segment_insert_sweep_status(std::vector<Segment2D *> &segments,Segment2D &segment,std::vector<Vec2> &intersect_points)
{
	float y = segment.start_point.y;
	int target_l = 0;
	for (int index_l = 0; index_l < segments.size(); ++index_l,++target_l)
	{
		Segment2D *target_segment = segments[index_l];
		//求线段与当前的扫描线之间的交点的x坐标
		float d_y = target_segment->final_point.y - target_segment->start_point.y;
		float d_x = target_segment->final_point.x - target_segment->start_point.x;
		float f = d_y ==0?0:(y - target_segment->start_point.y)/d_y;
		float x = target_segment->start_point.x + d_x * f;

		if (segment.start_point.x < x)
			break;
		//应该考虑相等的情况,此算法将会更加复杂,此种情况目前暂时不考虑
	}
	//插入到队列中
	segments.insert(segments.begin() + target_l,&segment);
}
//检查线段的左邻居
Segment2D  *static_segment_check_left_neightbor(std::vector<Segment2D *> &segments,const Segment2D *target_segment)
{
	for (int index_l = 0; index_l < segments.size(); ++index_l)
	{
		//此时需要检测前面的数据
		if (segments[index_l] == target_segment && index_l > 0)
			return segments[index_l -1];
	}
	return nullptr;
}
/*
  *检查线段的右邻居
 */
Segment2D  *static_segment_check_right_neightbor(std::vector<Segment2D *> &segments, const Segment2D *target_segment)
{
	for (int index_l = 0; index_l < segments.size(); ++index_l)
	{
		//此时需要检测前面的数据
		if (segments[index_l] == target_segment && index_l < segments.size() - 1)
			return segments[index_l + 1];
	}
	return nullptr;
}
/*
  *计算是否两条线段相交,
  *如果相交,则给出交点,并且产生新的事件点,加入到事件点队列中
  *否则不产生任何的副作用
 */
static void static_segment_intersect_event(float event_y,const Segment2D &a,const Segment2D &b,std::vector<Vec2> &intersect_points,std::vector<SegmentEndPoint> &end_point_event)
{
	const Vec2 ca = a.start_point - b.start_point;
	const Vec2 cb = a.final_point - b.start_point;
	const Vec2 d = b.final_point - b.start_point;
	const Vec2 d2 = a.final_point - a.start_point;
	float f1 = cross(d, ca);
	float f2 = cross(cb, d);
	//如果相交,则求出交点
	if (f1 * f2 >= 0 && cross(d2, ca) * cross(d2, b.final_point - a.start_point) >= 0)
	{
		const Vec2 intersect_point = a.start_point + d2 * (f1 / (f1 + f2));
		//该事件点应该小于当前扫描线的y坐标,也就是事件点的坐标y
		if (intersect_point.y < event_y)
		{
			intersect_points.push_back(intersect_point);
			//将相关的事件点插入到队列中
			SegmentEndPoint  new_event = {
				intersect_point,
				(Segment2D*)&a,(Segment2D*)&b,
				PointType_Intersect,
			};
			int index_l = 0;
			for (; index_l < end_point_event.size(); ++index_l)
			{
				SegmentEndPoint  &now_event = end_point_event.at(index_l);
				//事件点排列在最小的最大值之前
				if (intersect_point.y > now_event.end_point.y || intersect_point.y == now_event.end_point.y && (intersect_point.x < now_event.end_point.x || intersect_point.x == now_event.end_point.x && now_event.point_type > PointType_Intersect))
					break;
			}
			end_point_event.insert(end_point_event.begin() + index_l, new_event);
		}
	}
}
/*
  *删除指定的线段,如果删除后有新的邻居,则返回
 */
bool static_segment_remove_target(std::vector<Segment2D *> &sweep_status,Segment2D *segment,Segment2D **l_segment,Segment2D **r_segment)
{
	int array_size = sweep_status.size();
	for (int index_l = 0; index_l < array_size; ++index_l)
	{
		//此时需要检测前面的数据
		if (sweep_status[index_l] == segment)
		{
			if (index_l < array_size - 1)
				*r_segment = sweep_status[index_l + 1];
			if (index_l > 0)
				*l_segment = sweep_status[index_l - 1];
			sweep_status.erase(sweep_status.begin() + index_l);
			break;
		}
	}
	return *l_segment && *r_segment;
}
/*
   *交换两个线段的位置
  */
void static_segment_exchange_place(std::vector<Segment2D *> &sweep_status,Segment2D *l_segment, Segment2D *r_segment)
{
	int base_l = -1, secondary_l = -1;
	for (int index_l = 0; index_l < sweep_status.size(); ++index_l)
	{
		//此时需要检测前面的数据
		if (sweep_status[index_l] == l_segment)
			base_l = index_l;
		if (sweep_status[index_l] == r_segment)
			secondary_l = index_l;
	}
	if (base_l != -1 && secondary_l != -1)
	{
		Segment2D *t = sweep_status[base_l];
		sweep_status[base_l] = sweep_status[secondary_l];
		sweep_status[secondary_l] = t;
	}
}
/*
  *算法假设输入的数线段据中,起始端点是大于终止端点的,其标准参见比较函数
  *该算法需要两个数据结构
  *一个是事件点,
  *一个是当前与扫描线相交的线段的集合,我们称之为扫描线的状态.
  *目前该算法只是用了基本的数据结构,为的是简单容易的表达出,并没有经过其他的优化
  *注意,该算法并没有经过优化
 */
int segment_n_intersect_point(const std::vector<Segment2D> &segments, std::vector<cocos2d::Vec2> &intersect_points)
{
	//第一步收集所有线段的端点,目前算法暂时不考虑各种退化情况
	//比如一个线段的起始端点落在了另一个线段之上,两个线段局部重合,某几条线段相交于一点,某几个线段起始/终止于一点
	std::vector<SegmentEndPoint>  end_point_event(segments.size() * 2);
	Segment2D  *segment_array = (Segment2D *)segments.data();
	SegmentEndPoint *end_point_array = end_point_event.data();
	for (int index_l = 0; index_l < segments.size(); ++index_l)
	{
		end_point_array[index_l * 2].end_point = segment_array[index_l].start_point;
		end_point_array[index_l * 2].point_type = PointType_Origin;
		end_point_array[index_l * 2].segment = &segment_array[index_l];
		end_point_array[index_l * 2].other = nullptr;

		end_point_array[index_l * 2 + 1].end_point = segment_array[index_l].final_point;
		end_point_array[index_l * 2 + 1].point_type = PointType_Over;
		end_point_array[index_l * 2 + 1].segment = &segment_array[index_l];
		end_point_array[index_l * 2 + 1].other = nullptr;
	}
	//排序
	std::function<bool(const SegmentEndPoint &, const SegmentEndPoint &)> compare_func = [](const SegmentEndPoint &a, const SegmentEndPoint &b)->bool {
		//端点的Y坐标越大,其优先级越高
		if (a.end_point.y > b.end_point.y)
			return true;
		//如果Y坐标相等,那么就比较x坐标,x坐标越大,其优先级越高,或者在坐标相同的条件下,比较端点的类型
		if (a.end_point.y == b.end_point.y && (a.end_point.x < b.end_point.x || a.end_point.x == b.end_point.x && a.point_type < b.point_type))
			return true;
		//剩下的情况,要么是a的y坐标小于b的y坐标的,或者相等然而a的x坐标大于b的x坐标,或者x坐标相等,然而类型的优先级较小
		return false;//以上代码可以优化,这里为了注释的清晰,就省略了
	};
	quick_sort<SegmentEndPoint>(end_point_array, end_point_event.size(), compare_func);
	std::vector<Segment2D*>   sweep_status;
	sweep_status.reserve(end_point_event.size());
	//针对每一个事件点,进行遍历
	while (end_point_event.size() > 0)
	{
		//删除掉第一个事件点
		SegmentEndPoint   now_event = end_point_event.front();
		end_point_event.erase(end_point_event.begin());
		//如果是起始端点,则加入到扫描线中,并且检查该线段的两边是否有邻居,如果有,则一一检测
		if (now_event.point_type == PointType_Origin)
		{
			static_segment_insert_sweep_status(sweep_status, *now_event.segment, intersect_points);
			Segment2D  *l_segment = static_segment_check_left_neightbor(sweep_status, now_event.segment);
			Segment2D  *r_segment = static_segment_check_right_neightbor(sweep_status, now_event.segment);
			if (l_segment)
				static_segment_intersect_event(now_event.end_point.y,*l_segment,*now_event.segment,intersect_points,end_point_event);
			if(r_segment)
				static_segment_intersect_event(now_event.end_point.y,*now_event.segment, *r_segment, intersect_points, end_point_event);
		}
		else if (now_event.point_type == PointType_Over)//如果是结束端点,则将相关的线段移除掉,并对新的邻居进行测试
		{
			Segment2D *l_segment = nullptr,*r_segment = nullptr;
			static_segment_remove_target(sweep_status,now_event.segment,&l_segment,&r_segment);
			if (l_segment && r_segment)
				static_segment_intersect_event(now_event.end_point.y,*l_segment, *r_segment, intersect_points, end_point_event);
		}
		else//如果是中间交叉点,则需要交换他们之间的顺序,并对新形成的邻居重新计算相交点
		{
			static_segment_exchange_place(sweep_status, now_event.segment, now_event.other);
			Segment2D	*l1 = static_segment_check_left_neightbor(sweep_status, now_event.other);
			if (l1)
				static_segment_intersect_event(now_event.end_point.y,*l1,*now_event.other,intersect_points,end_point_event);

			Segment2D  *r2 = static_segment_check_right_neightbor(sweep_status, now_event.segment);
			if (r2)
				static_segment_intersect_event(now_event.end_point.y,*now_event.segment, *r2, intersect_points, end_point_event);
		}
	}
	return intersect_points.size();
}

int segment_n_intersect_prim(const std::vector<Segment2D> &segments, std::vector<cocos2d::Vec2> &intersect_points)
{
	int array_size = segments.size();
	for (int index_l = 0; index_l < array_size - 1; ++index_l)
	{
		for (int index_j = index_l + 1; index_j < array_size; ++index_j)
		{
			Vec2 intersect_point;
			if (segment_segment_intersect_test(segments[index_l], segments[index_j],intersect_point))
			{
				if (intersect_points.size() >= intersect_points.capacity())
					intersect_points.reserve(intersect_points.size() * 2);
				intersect_points.push_back(intersect_point);
			}
		}
	}
	return intersect_points.size();
}
NS_GT_END