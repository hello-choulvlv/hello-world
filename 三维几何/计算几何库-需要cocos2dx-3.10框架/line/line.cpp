/*
  *ֱ��,�߶��㷨ʵ��
  *2019��6��22��
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
	if (fabs(coef) >= 0.001f)//����ֱ�߲�ƽ��
	{
		float s = (c - b * f)/coef;
		float t = (b* c  - f)/coef;

		intersect_apoint = Q1.start_point + Q1.direction * s;
		intersect_bpoint = Q2.start_point + Q2.direction * t;

		return (intersect_bpoint - intersect_apoint).length();
	}
	//ƽ��ֱ��
	intersect_apoint = Q1.start_point;
	float  project = dot(r,Q2.direction);
	intersect_bpoint = Q2.start_point + Q2.direction * project;
	return (intersect_bpoint - intersect_apoint).length();
}

float segment_segment_minimum_distance(const Segment &l1, const Segment &l2, cocos2d::Vec3 &intersect_apoint, cocos2d::Vec3 & intersect_bpoint)
{
	//���㷨˼���ֱ�ߵ������������,����ʵ�ֵĹ���Ҫ���Ӻܶ�
	Vec3 d1 = l1.final_point  - l1.start_point;
	Vec3 d2 = l2.final_point - l2.start_point;
	Vec3 r = l1.start_point - l2.start_point;
	float a = dot(d1,d1);
	float e = dot(d2,d2);
	float b = dot(d1,d2);
	float f = dot(d2,r);

	float s = 0, t = 0;
	//��ֱ��֮������������������
	float c = dot(r,d1);
	float denom = a * e - b * b;
	if (denom != 0)
		s = clampf(0, 1, (b * f - c * e)/denom);
	t = (s * b + f)/e;
	//��߽�
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
	//��Ŀ����Լ�����
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
	//��ʱ����ƫ��������
	if (a > r2 && b > 0)
		return false;
	float t = b * b - a + r2;
	//���η��̵ĸ����б�ʽС��0,��ʱ�����޽�
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
	//������Ϊ��ʱ,��ʱ�������߶εķ��ӳ�����,�������߶ε����ӳ�����
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
  *���㷨��û�����OBB�����Ż�
  *���㷨ʵ�ֵ�����һ���汾�Ƿ��������
  *��ϸʵ����μ�aabb_obb.cpp�ļ�
 */
bool ray_obb_intersect_test(const Ray &ray, const OBB &obb)
{
	//����ϵ�任
	gt::AABB   aabb;
	aabb_create(aabb,- obb.extent,obb.extent);

	//�任��OBB�ľֲ�����ϵ��
	Vec3 origin = ray.origin - obb.center;
	gt::Ray  secondary_ray = {
		Vec3(origin.dot(obb.xaxis), origin.dot(obb.yaxis),origin.dot(obb.zaxis)),
		Vec3(ray.direction.dot(obb.xaxis),ray.direction.dot(obb.yaxis),ray.direction.dot(obb.zaxis)),
	};

	return ray_aabb_intersect_test(secondary_ray, aabb);
}

bool line_triangle_intersect_test(const Line &line, const Triangle &triangle)
{
	//�ж��Ƿ�ֱ��ƽ��������ƽ��,����������ƽ�湲��
	const Vec3 pa = triangle.a - line.start_point;
	const Vec3 pb = triangle.b - line.start_point;
	const Vec3 pc = triangle.c - line.start_point;
	//����ƽ��ķ���,���´��벢û�о����Ż�
	const Vec3 ab = triangle.b - triangle.a;
	const Vec3 bc = triangle.c - triangle.b;
	const Vec3 normal = cross_normalize(ab, bc);
	float f = dot(normal,line.direction);
	//ƽ�л��߹���
	if (fabsf(f) <= 0.001f)
	{
		float distance = dot(normal,line.start_point - triangle.a);
		//ƽ��
		if (fabsf(distance) > 0.001f)
			return false;
		//����,AB
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
	//��������ػ�
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
	//�����Է������ϵ�����������ʽ
	float d = dot(pq,normal);
	//���ϵ�����������ʽ����0,��˵���߶�ƽ��������ƽ�������ƽ�湲��
	if (fabsf(d) <= 0.001f)
	{
		const Vec3 v1 = segment.start_point - triangle.a;
		float distance = dot(normal,v1);
		if (fabsf(distance) > 0.001f)
			return false;
		//�����������ı����߶εĽ���
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
	//��ȡ����ϵ�����������ʽ
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
	//������ƽ��Ĳ������
	const Vec3 d = cross(plane1.normal,plane2.normal);
	float l = length2(d);
	//ƽ��/���߹���
	if (l < 0.001f)
	{
		//���ƽ��2��һ��
		const Vec3 target_point(10,10,10);
		const Vec3 inside_point = target_point - plane2.normal * plane2.distanceTo(target_point);
		//ƽ��
		if(plane1.distanceTo(inside_point) > 0)
			return false;
		//����,���������һ��ֱ��
		line.start_point = inside_point;
		const Vec3 other_point(10,20,20);
		const Vec3 other_inside_point = other_point - plane2.normal * plane2.distanceTo(other_point);
		line.direction = normalize(other_inside_point - inside_point);
		return true;
	}
	//�󹫹�ֱ��
	line.direction = normalize(d);
	//ʹ��Cramer�������ֱ����һ������� 
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
	//��ƽ�пռ�ļ��
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
	//�����㷨Ŀǰ�Ȳ��ò�������,�������ǽ���ʹ�ù鲢����
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

			while (base_j < l_boundary && other_j < r_boundary)//�߽�
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
			//����Ƿ���ĳЩԪ�ػ�û����ȫ�������
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
	int      point_type;//0������ʼ�˵�,1������ֹ�˵�
	const Segment2D   *segment;
};

static cocos2d::Node *s_layer = nullptr;
void  set_layer(cocos2d::Node *layer)
{
	s_layer = layer;
}
//��ֵ���뵽���������
void static_list_insert(std::list<VPoint *> &list_segment,VPoint  *p)
{
	auto it = list_segment.begin();
	while (it != list_segment.end())
	{
		VPoint  *q = *it;
		const Segment2D &segment = *q->segment;
		//�����߶���ɨ���ߵĽ���
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
	//��һ����Ԥ����,���߼����߶��Ѿ�ȫ��Ԥ�������
	//����Ĺ���Ϊ:��ʼ�˵���������xһ��С�ڵ�����ֹ�˵������x,��������y����һ��ҪС�ڵ�����ֹ�����y����
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
		//�����жϿ��Թ��Ϊһ�����
		//return a.point.x < b.point.x || a.point.x == b.point.x && (a.point.y < b.point.y || a.point.y == b.point.y && a.point_type > b.point_type);
	};
	quick_sort<VPoint>(point_vec.data(),(int) point_vec.size(), compare_func);
	//std::sort<VPoint>(point_vec.begin(), point_vec.end(), compare_func);
	//����������еĶ���,����
	//std::set<VPoint*>  set_segment;
	//��֪set_queue�����ݵ������ǽ����,�����ʵ�ʵ�ʵ�ֹ�����,����ʹ�ø�Ϊ���ٵ�setʵ��
	//����֮����û��ʹ��,����Ϊstd::set���ָ��������ǹ̶���,�޷���ͨ��,����������Ҫʹ��
	//�㷨�Ŀ���ʵ�ֵ�����,�����Զ���ʵ��
	std::list<VPoint *>  list_segment;

	//set_segment.key_comp();
	for (int index_l = 0; index_l < point_vec.size(); ++index_l)
	{
		VPoint   *p = point_vec.data() + index_l;
		//������صļ�ֵ,��ֵ�Ĵ�С��ɨ���ߵ�x�����й�,���Ҹ�ɨ�������߶εĽ����y����Ҳ�й�ϵ
		int key_code = p->point.x * 1000;
		//�������ʼ�˵�
		if (p->point_type == 0)
		{
			static_list_insert(list_segment, p);
			//���������/�����Ƿ����ཻ���߶�
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
NS_GT_END