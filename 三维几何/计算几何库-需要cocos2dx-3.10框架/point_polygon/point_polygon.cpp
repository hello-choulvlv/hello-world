/*
 *离散点集与多边形处理
 *2019/6/16
 *Author:小花熊
 */
#include "point_polygon/point_polygon.h"
#include "matrix/matrix.h"
#include "line/line.h"
#include "cycle_sphere/cycle_sphere.h"
#include<unordered_map>
//#include<map>
#include<stack>
#include<set>
#include<assert.h>
using namespace cocos2d;
NS_GT_BEGIN
struct PartionVec3
{
	Vec3  *child_buffer[12];
	int        child_size;

	PartionVec3() {};
};

void polygon_create(Polygon &polygon, const std::vector<cocos2d::Vec2> &points)
{
	//注意即使在2d空间下,我们也将会使用超平面概念,并且坐标系变换上也是用右手坐标系
	polygon.plane_array.reserve(points.size());
	Plane2D  plane;
	for (int index_l = 0; index_l < points.size(); ++index_l)
	{
		const Vec2 &a = points[index_l];
		const Vec2 &b = points[index_l +1 >= points.size()?0:index_l +1];
		const Vec2 direction = normalize(b - a);
		//需要进行一次矩阵变换,这里简化了
		plane.normal.x = -direction.y;
		plane.normal.y = direction.x;
		plane.distance = dot(plane.normal,a);
		polygon.plane_array.push_back(plane);
	}
	polygon.point_array = points;
}
/*
  *直线/线段/射线与多边形的相交测试
 */
static bool polygon_unkonwn_intersect_test(const Polygon &polygon, const Vec2 &a,const Vec2 &direction,float min_f,float max_f)
{
	//针对多边形的相交测试
	for (int index_l = 0; index_l < polygon.plane_array.size(); ++index_l)
	{
		const Plane2D  &plane = polygon.plane_array[index_l];
		float f = dot(plane.normal, direction);
		float d = dot(plane.normal,a) - plane.distance;
		//如果直线与多边形的某一条边垂直
		if (fabsf(f) < 0.001f)
		{
			//如果直线正向远离了多边形的某一条边,此时必定分离
			if (d < 0)
				return false;
		}
		else
		{
			//求出直线与多边形的交点
			float t = -d / f;
			if (f < 0.0f)//此时法线的方向背离了光线的方向
			{
				if (max_f > t)
					max_f = t;
			}
			else
			{
				if (min_f < t)
					min_f = t;
			}
			if (min_f > max_f)
				return false;
		}
	}
	return min_f <= max_f;
}

bool polygon_line_intersect_test(const Polygon &polygon, const Line &line)
{
	return polygon_unkonwn_intersect_test(polygon, *(Vec2*)&line.start_point, *(Vec2*)&line.direction.x,-FLT_MAX,FLT_MAX);
}

bool polygon_segment_intersect_test(const Polygon &polygon, const Segment &segment)
{
	const Vec3 direction = segment.final_point - segment.start_point;
	return polygon_unkonwn_intersect_test(polygon, *(Vec2*)&segment.start_point,normalize(*(Vec2*)&direction),0, direction.length());
}

bool polygon_ray_intersect_test(const Polygon &polygon, const Ray &ray)
{
	return polygon_unkonwn_intersect_test(polygon,*(Vec2*)&ray.start_point,*(Vec2*)&ray.direction,0,FLT_MAX);
}

bool polygon_contains_point(const std::vector<cocos2d::Vec2> &polygon, const cocos2d::Vec2 &point)
{
	int l = 0, h = polygon.size();
	const Vec2 &v0 = polygon[0];
	const Vec2 interpolation = point - v0;
	do 
	{
		int m = (l+h) >> 1;
		if (sign_area(polygon[m] - v0, interpolation) >= 0)//左侧
			l = m;
		else
			h = m;
	} while (l +1 < h);
	//如果穷尽了左侧或者右侧
	if (l == 0 || h == polygon.size())
		return 0;
	return sign_area(polygon[h] - polygon[l],point - polygon[l]) >=0.0f;
}

bool triangle_contains_point(const cocos2d::Vec2 vertex[3], const cocos2d::Vec2 &point)
{
	const Vec2 pa = vertex[0] - point;
	const Vec2 pb = vertex[1] - point;
	const Vec2 pc = vertex[2] - point;

	float f1 = sign_area(pa, pb);
	float f2 = sign_area(pb, pc);
	if (f1 * f2 < 0.0f)
		return false;
	float f3 = sign_area(pc, pa);
	return f2 * f3 >= 0.0f;
}

bool polygon_cycle_intersect_test(const Polygon &polygon, const Cycle &cycle)
{
	//遍历每一条边,求出边与球体的关系
	float r2 = cycle.radius * cycle.radius;
	for (int index_l = 0; index_l < polygon.plane_array.size(); ++index_l)
	{
		const Plane2D &plane = polygon.plane_array[index_l];
		//如果圆处于超平面之外
		float d = dot(plane.normal,cycle.center) - plane.distance;
		if (d < -cycle.radius)
			return false;
		//如果圆心距离超平面在半径之内,则需要进行线段与球心之间的距离检查
		if (d <= cycle.radius)
		{
			const Vec2 &a = polygon.point_array[index_l];
			const Vec2 &b = polygon.point_array[index_l + 1 >= polygon.point_array.size() ? 0 : index_l + 1];
			Vec2 direction = b - a;
			float l = direction.length();
			direction *= 1.0f/l;
			const Vec2 departure = cycle.center - a;
			float t = clampf(0.0f, l,dot(departure,direction));
			const Vec2 v0 = departure - direction * t;
			if (length2(v0) <= r2)
				return true;
		}
	}
	//如果在各个半空间的交集之内,则执行标准的多边形/点包含测试
	//不能直接返回true,有一种情况下,上面的代码无法排除掉,就是圆同时与两个相邻的平面相交,但是却没有与多边形相交
	return polygon_contains_point(polygon.point_array, cycle.center);
}
/*
*遍历顺序表
*/
template<typename TS>
bool check_sequence_have(const std::vector<TS> &rs_vec, const TS &ts_value)
{
	for (auto it = rs_vec.begin(); it != rs_vec.end(); ++it)
		if (*it == ts_value)
			return true;
	return false;
}

bool check_in_target_plane(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &p)
{
	Vec3  d_ab = b - a;
	Vec3  d_ac = c - a;
	Vec3 d_pa = p - a;
	Vec3  normal = cross(d_ab, d_ac);

	float d = dot(d_pa, normal);
	if (fabs(d) < 0.0001f)
		return false;
	//将d_pa表示为d_ab,d_ac的线性组合
	float m = d_ab.x * d_ac.y - d_ac.x * d_ab.y;
	float w = (d_ab.x * d_pa.y - d_pa.x * d_ab.x) / m;
	float v = (d_pa.x * d_ac.y - d_ac.x * d_pa.y) / m;

	return v >= 0 && w >= 0 && v + w <= 1;
}

bool compute_barycentric(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &p, cocos2d::Vec3 &coord)
{
	Vec3  d_ab = b - a;
	Vec3  d_ac = c - a;
	Vec3 d_pa = p - a;
	Vec3  normal = cross(d_ab, d_ac);

	float d = dot(d_pa, normal);
	if (fabs(d) < 0.0001f)
		return false;
	//将d_pa表示为d_ab,d_ac的线性组合
	float m = d_ab.x * d_ac.y - d_ac.x * d_ab.y;
	coord.y = (d_ab.x * d_pa.y - d_pa.x * d_ab.x) / m;
	coord.z = (d_pa.x * d_ac.y - d_ac.x * d_pa.y) / m;
	coord.x = 1.0f - coord.y - coord.z;

	return coord.y >= 0 && coord.z >= 0 && coord.x >= 0.0f;
}

static bool compare_with_vec2(const Vec2 &a, const Vec2 b)
{
	return a.x < b.x || (a.x == b.x && a.y < b.y);
}

void polygon_polygon_tangent_line(const std::vector<cocos2d::Vec2> &polygon1, const std::vector<cocos2d::Vec2> &polygon2, int tangent_index_array[4])
{
	//查找第一个凸多边形的最大y坐标顶点,以及最小y坐标顶点,如果有多个顶点的y坐标相同,则选取的方案有所不同
	const int a_array_size = polygon1.size();
	const int b_array_size = polygon2.size();
	int a_top_index = 0;
	int a_bottom_index = 0;

	int b_top_index = 0;
	int b_bottom_index = 0;
	//需要增加一轮循环,才能计算出来
	bool b_min =true, b_max = true;
	int a_top_prev = a_array_size - 1;
	int a_top_next = 1;

	int a_bottom_prev = a_array_size - 1;
	int a_bottom_next = 1;

	int b_top_prev = b_array_size - 1;
	int b_top_next = 1;

	int b_bottom_prev = b_array_size - 1;
	int b_bottom_next = 1;

	while (b_min || b_max)
	{
		if (b_max)
		{
			if (cross(polygon1[a_top_index], polygon2[b_top_index], polygon1[a_top_prev]) > 0)
				a_top_index = a_top_prev;
			else if (cross(polygon1[a_top_index], polygon2[b_top_index], polygon1[a_top_next]) > 0)
				a_top_index = a_top_prev;
			else if (cross(polygon1[a_top_index], polygon2[b_top_index], polygon2[b_top_prev]) > 0)
				b_top_index = b_top_prev;
			else if (cross(polygon1[a_top_index], polygon2[b_top_index], polygon2[b_top_next]) > 0)
				b_top_index = b_top_next;
			else
				b_max = false, tangent_index_array[0] = a_top_index, tangent_index_array[1] = b_top_index;

			a_top_prev = (a_top_index - 1 + a_array_size) % a_array_size;
			a_top_next = (a_top_index + 1) % a_array_size;

			b_top_prev = (b_top_index - 1 + b_array_size) % b_array_size;
			b_top_next = (b_top_index + 1) % b_array_size;
		}

		if (b_min)
		{
			if (cross(polygon1[a_bottom_index], polygon2[b_bottom_index], polygon1[a_bottom_prev]) < 0)
				a_bottom_index = a_bottom_prev;
			else if (cross(polygon1[a_bottom_index], polygon2[b_bottom_index], polygon1[a_bottom_next]) < 0)
				a_bottom_index = a_bottom_next;
			else if (cross(polygon1[a_bottom_index], polygon2[b_bottom_index], polygon2[b_bottom_prev]) < 0)
				b_bottom_index = b_bottom_prev;
			else if (cross(polygon1[a_bottom_index], polygon2[b_bottom_index], polygon2[b_bottom_next]) < 0)
				b_bottom_index = b_bottom_next;
			else
				b_min = false, tangent_index_array[2] = a_bottom_index, tangent_index_array[3] = b_bottom_index;

			a_bottom_prev = (a_bottom_index - 1 + a_array_size) % a_array_size;
			a_bottom_next = (a_bottom_index + 1) % a_array_size;

			b_bottom_prev = (b_bottom_index - 1 + b_array_size) % b_array_size;
			b_bottom_next = (b_bottom_index + 1) % b_array_size;
		}
	}
}

void plane2d_create(Plane2D &plane, const cocos2d::Vec2 &normal, float d)
{
	plane.normal = normalize(normal);
	plane.distance = d / normal.length();
}

void plane2d_create(Plane2D &plane, const Vec2 &a, const Vec2 &b)
{
	plane.normal = normalize(b-a);
	plane.distance = dot(plane.normal,a);
}

float polygon_compute_max_distance(const std::vector<cocos2d::Vec2> &polygon_points, cocos2d::Vec2 &max_a, cocos2d::Vec2 &max_b)
{
	float distance = 0.0f;
	int    target_j = 1;
	//注意算法为了简单易懂,因此并没有经过优化
	for (int index_l = 0; index_l < polygon_points.size(); ++index_l)
	{
		int secondary_l = index_l + 1 >= polygon_points.size() ? 0 : index_l + 1;
		const Vec2 compare_edge = polygon_points[secondary_l] - polygon_points[index_l];
		while(cross(compare_edge,polygon_points[target_j] - polygon_points[secondary_l]) < cross(compare_edge,polygon_points[(target_j +1)% polygon_points.size()] - polygon_points[secondary_l]))
			target_j = (target_j + 1)% polygon_points.size();
		float d = length(polygon_points[target_j], polygon_points[secondary_l]);
		if (d > distance)
		{
			distance = d;
			max_a = polygon_points[target_j];
			max_b = polygon_points[secondary_l];
		}
	}
	return distance;
}
//插入排序算法实现
template<typename VecType>
static void static_sort_insert(VecType **buffer_array, int buffer_size, VecType *v)
{
	int index_l = buffer_size - 1;
	while (index_l >= 0 && buffer_array[index_l]->y > v->y)
	{
		buffer_array[index_l + 1] = buffer_array[index_l];
		--index_l;
	}
	buffer_array[index_l + 1] = v;
}

template<typename VecType,int compare_count>
static float static_two_space_min_distance(VecType **buffer_array, int buffer_size,float &distance, VecType &a, VecType &b)
{
	float f = FLT_MAX;
	for (int index_l = 0; index_l < buffer_size; ++index_l)
	{
		int boundary_r = index_l + compare_count >= buffer_size?buffer_size:index_l+ compare_count;
		for (int other_l = index_l + 1; other_l < boundary_r; ++other_l)
		{
			float d = length(*buffer_array[index_l],*buffer_array[other_l]);
			if (d < f)
			{
				f = d;
				if (d < distance)
				{
					distance = d;
					a = *buffer_array[index_l];
					b = *buffer_array[other_l];
				}
			}
		}
	}
	return f;
}

float point_compute_minimum_distance(const std::vector<cocos2d::Vec2> &points, cocos2d::Vec2 &a, cocos2d::Vec2 &b)
{
	typedef Vec2*   Vec2Type;
	std::vector<Vec2Type> sorted_points_array(points.size());
	Vec2 *base_p = (Vec2 *)points.data();
	for (int index_l = 0; index_l < points.size(); ++index_l)
		sorted_points_array[index_l] = base_p + index_l;
	//排序
	std::function<bool(const Vec2Type &a, const Vec2Type &b)> compare_func = [](const Vec2Type &a, const Vec2Type &b)->bool {
		return a->x < b->x;
	};
	quick_sort<Vec2Type>(sorted_points_array.data(), (int)sorted_points_array.size(),compare_func);
	//对已经排序的离散点集进行处理
	const int  boundary = points.size();
	Vec2  **buffer_array = new Vec2  *[boundary >> 1];
	int      buffer_size = 0;
	float distance = FLT_MAX;
	float *sub_distance_array = new float[(boundary >> 1) + 1];
	for (int base_j = 0; base_j < boundary; base_j += 2)
	{
		float x = FLT_MAX;
		float s = sorted_points_array[base_j]->x;//分割线
		if (base_j + 1 < boundary)
		{
			x = length(*sorted_points_array[base_j], *sorted_points_array[base_j + 1]);
			s = sorted_points_array[base_j +1]->x;
		}
		sub_distance_array[base_j >> 1] = x;
		if (x < distance)
		{
			distance = x;
			a = *sorted_points_array[base_j];
			b = *sorted_points_array[base_j +1];
		}
	}
	for (int step = 4; step >> 1 < boundary; step *= 2)
	{
		for (int base_j = 0; base_j < boundary; base_j += step)
		{
			int other_j = base_j + (step >> 1);
			if (other_j >= boundary)
			{
				sub_distance_array[base_j / step] = sub_distance_array[base_j * 2/step];
				break;//此时循环不必进行下去了
			}
			float s = sorted_points_array[other_j -1]->x;
			float d = fminf(sub_distance_array[base_j * 2/step], sub_distance_array[other_j*2/step]);
			//分别从分割线两侧取出d范围内的顶点,并且按照Y坐标递增排序
			buffer_size = 0;
			//左侧数据
			int index_l = other_j-1;
			for (; index_l >= base_j && sorted_points_array[index_l]->x + d > s; --index_l)
				static_sort_insert<Vec2>(buffer_array,buffer_size++, sorted_points_array[index_l]);
			//右侧数据
			index_l = other_j;
			int boundary_r = other_j + step / 2 >= boundary ? boundary : other_j + step / 2;
			for (; index_l < boundary_r && sorted_points_array[index_l]->x - d < s;++index_l)
				static_sort_insert<Vec2>(buffer_array, buffer_size++, sorted_points_array[index_l]);
			//求最近距离,需要比较的顶点数目不会超过七个
			float f = static_two_space_min_distance<Vec2,8>(buffer_array, buffer_size,distance, a, b);
			//将最近距离写入到缓冲区中
			sub_distance_array[base_j/step] = fminf(d,f);
		}
	}

	delete[] sub_distance_array;
	delete[] buffer_array;
	return distance;
}

float point_prim_compute_minimum_distance(const std::vector<cocos2d::Vec2> &points, cocos2d::Vec2 &a, cocos2d::Vec2 &b)
{
	float distance = FLT_MAX;
	for (int index_l = 0; index_l < (int)points.size() - 1; ++index_l)
	{
		for (int secondary_l = index_l + 1; secondary_l < points.size(); ++secondary_l)
		{
			float d = length(points[index_l],points[secondary_l]);
			if (d < distance)
			{
				distance = d;
				a = points[index_l];
				b = points[secondary_l];
			}
		}
	}
	return distance;
}

void static_sort_insert_3d(Vec3 **buffer_array,int buffer_size,Vec3 *v,float distance)
{
	int index_l = buffer_size - 1;
	//注意排序算法
	while (index_l >= 0 && (v->y < buffer_array[index_l]->y && v->z + distance >= buffer_array[index_l]->z || v->y >= buffer_array[index_l]->y && v->z < buffer_array[index_l]->z - distance))
	{
		buffer_array[index_l + 1] = buffer_array[index_l];
		--index_l;
	}
	buffer_array[index_l + 1] = v;
}
static int static_least_y_location(Vec3 **buffer_array, int buffer_size,float interval,Vec3 &min_location)
{
	Vec3  *location = buffer_array[0];
	Vec3    max_location = *location;
	min_location = *location;

	for (int index_l = 1; index_l < buffer_size; ++index_l)
	{
		Vec3 *location = buffer_array[index_l];
		min_location.y = fminf(min_location.y, location->y);
		min_location.z = fminf(min_location.z, location->z);

		max_location.y = fmaxf(max_location.y, location->y);
		max_location.z = fmaxf(max_location.z, location->z);
	}

	return ceilf((max_location.z - min_location.z) / interval);
}
//空间划分
static void static_yz_space_partion(std::unordered_map<int64_t, PartionVec3> &partion_map,Vec3 &ref_location,Vec3 **buffer_array,int buffer_size,int extent,float interval)
{
	float t2 = interval * 2.0f;
	for (int index_l = 0; index_l < buffer_size; ++index_l)
	{
		Vec3 *location = buffer_array[index_l];
		int64_t y = (location->y - ref_location.y) / t2;
		int64_t z = (location->z - ref_location.z) / t2;
		//后面的数字在实际的运行中需要动态计算
		int64_t hash_key = y * extent + z;
		auto it = partion_map.find(hash_key);
		if (it == partion_map.end())
		{
			PartionVec3   partion;
			partion_map[hash_key] = partion;
			it = partion_map.find(hash_key);
			it->second.child_size=0;
		}
		PartionVec3  &partion = it->second;
		partion.child_buffer[partion.child_size++] = location;
	}
}

float static_two_space_min_distance(std::unordered_map<int64_t, PartionVec3> &partion_map,const Vec3 &ref_location, Vec3 *location,int extent,float interval, float &distance, Vec3 &a, Vec3 &b)
{
	float f = FLT_MAX;
	float t2 = interval	 * 2.0f;
	//检查是否有与所在的空间相邻的空间,需要检查4个
	int64_t y = (location->y - ref_location.y)/ t2;
	int64_t z = (location->z - ref_location.z)/ t2;

	float fract_y = location->y - y *t2;
	float fract_z = location->z - z * t2;
	//
	PartionVec3   *compare_array[4];
	int compare_size = 0;
	
	int64_t base_key = y * extent + z;
	//最多选中4个,第一个点比较特殊
	auto it = partion_map.find(base_key);
	bool select_one = false;
	if (it != partion_map.end())
		compare_array[compare_size++] = &it->second;

	//y
	int base_y = fract_y < interval ? -1 : 1;
	it = partion_map.find(base_key + base_y * extent);
	if (it != partion_map.end())
		compare_array[compare_size++] = &it->second;

	//z
	int base_z = fract_z < interval ? -1 : 1;
	it = partion_map.find(base_key + base_z);
	if (it != partion_map.end())
		compare_array[compare_size++] = &it->second;

	it = partion_map.find(base_key + base_y * extent + base_z);
	if (it != partion_map.end())
		compare_array[compare_size++] = &it->second;
	
	for (int index_l = 0; index_l < compare_size; ++index_l)
	{
		auto &child = *compare_array[index_l];
		for (int base_l = 0; base_l < child.child_size;++base_l)
		{
			float t = length(*child.child_buffer[base_l], *location);
			if (t < f)
			{
				f = t;
				if (t < distance)
				{
					distance = t;
					a = *child.child_buffer[base_l];
					b = *location;
				}
			}
		}
	}
	return f;
}

float point_compute_minimum_distance(const std::vector<cocos2d::Vec3> &points, cocos2d::Vec3 &a, cocos2d::Vec3 &b)
{
	typedef Vec3*   Vec3Type;
	std::vector<Vec3Type> sorted_points_array(points.size());
	Vec3 *base_p = (Vec3 *)points.data();
	for (int index_l = 0; index_l < points.size(); ++index_l)
		sorted_points_array[index_l] = base_p + index_l;
	//排序
	std::function<bool(const Vec3Type &a, const Vec3Type &b)> compare_func = [](const Vec3Type &a, const Vec3Type &b)->bool {
		return a->x < b->x;
	};
	quick_sort<Vec3Type>(sorted_points_array.data(), (int)sorted_points_array.size(), compare_func);
	//对已经排序的离散点集进行处理
	const int  boundary = points.size();
	Vec3  **buffer_array = new Vec3  *[boundary >> 1];
	int      buffer_size = 0;
	float distance = FLT_MAX;
	float *sub_distance_array = new float[(boundary >> 1) + 1];
	for (int base_j = 0; base_j < boundary; base_j += 2)
	{
		float x = FLT_MAX;
		float s = sorted_points_array[base_j]->x;//分割线
		if (base_j + 1 < boundary)
		{
			x = length(*sorted_points_array[base_j], *sorted_points_array[base_j + 1]);
			s = sorted_points_array[base_j + 1]->x;
		}
		sub_distance_array[base_j >> 1] = x;
		if (x < distance)
		{
			distance = x;
			a = *sorted_points_array[base_j];
			b = *sorted_points_array[base_j + 1];
		}
	}
	std::unordered_map<int64_t, PartionVec3> partion_map;
	partion_map.reserve(256);
	Vec3 ref_location,max_location;
	int extent = 0;
	for (int step = 4; step >> 1 < boundary; step *= 2)
	{
		for (int base_j = 0; base_j < boundary; base_j += step)
		{
			int other_j = base_j + (step >> 1);
			if (other_j >= boundary)
			{
				sub_distance_array[base_j / step] = sub_distance_array[base_j * 2 / step];
				break;//此时循环不必进行下去了
			}
			float s = sorted_points_array[other_j - 1]->x;
			float d = fminf(sub_distance_array[base_j * 2 / step], sub_distance_array[other_j * 2 / step]);
			//分别从分割线两侧取出d范围内的顶点,并且按照Y坐标递增排序
			buffer_size = 0;
			//---------------------------右侧数据-----------------------------------
			ref_location.x = ref_location.y = FLT_MAX;
			max_location.x = max_location.y = -FLT_MAX;
			int index_l = other_j;
			int boundary_r = other_j + step / 2 >= boundary ? boundary : other_j + step / 2;
			for (; index_l < boundary_r && sorted_points_array[index_l]->x - d < s; ++index_l)
			{
				Vec3 *location = sorted_points_array[index_l];
				buffer_array[buffer_size++] = location;

				ref_location.y = fminf(ref_location.y, location->y);
				ref_location.z = fminf(ref_location.z, location->z);

				max_location.y = fmaxf(max_location.y, location->y);
				max_location.z = fmaxf(max_location.z, location->z);
			}
			//求出y坐标最小的点
			float f = FLT_MAX;
			if (buffer_size > 0)
			{
				extent = ceilf((max_location.z - ref_location.z) * 0.5f/d);
				static_yz_space_partion(partion_map, ref_location, buffer_array,buffer_size,extent,d);
				//左侧数据
				index_l = other_j - 1;
				for (; index_l >= base_j && sorted_points_array[index_l]->x + d > s; --index_l)
				f = fminf(f,static_two_space_min_distance(partion_map, ref_location, sorted_points_array[index_l], extent,d, distance, a, b));
				//该算法的最重要的优化就是partion_map的内存重分配
				partion_map.clear();
			}
			//将最近距离写入到缓冲区中
			sub_distance_array[base_j / step] = fminf(d, f);
		}
	}

	delete[] sub_distance_array;
	delete[] buffer_array;
	return distance;
}

float point_prim_compute_minimum_distance(const std::vector<cocos2d::Vec3> &points, cocos2d::Vec3 &a, cocos2d::Vec3 &b)
{
	float distance = FLT_MAX;
	for (int index_l = 0; index_l < (int)points.size() - 1; ++index_l)
	{
		for (int secondary_l = index_l + 1; secondary_l < points.size(); ++secondary_l)
		{
			float d = length(points[index_l], points[secondary_l]);
			if (d < distance)
			{
				distance = d;
				a = points[index_l];
				b = points[secondary_l];
			}
		}
	}
	return distance;
}
/*
  *计算多边形的既定方向的支撑点
 */
static Vec2 static_compute_support_point(const Polygon &polygon,const Vec2 &direction)
{
	float f = dot(polygon.point_array[0],direction);
	int    base_j = 0;
	for (int index_l = 1; index_l < polygon.point_array.size(); ++index_l)
	{
		float t = dot(polygon.point_array.at(index_l),direction);
		if (t > f)
		{
			f = t;
			base_j = index_l;
		}
	}
	return polygon.point_array.at(base_j);
}

bool static_compute_match_simplex(Vec2 *simplex_array,int &simplex_count)
{
	//如果根据单纯形的维度来决定函数的走向
	if (simplex_count == 2)//此时需要计算新的方向
	{
		Vec2  normal = simplex_array[1] - simplex_array[0];
		//计算原点与线段之间的相对关系
		float f = -dot(normal, simplex_array[0]);
		if (f < 0)
			simplex_count = 1;
		else if (f > length2(normal))
		{
			simplex_array[0] = simplex_array[1];
			simplex_count = 1;
		}
		return false;
	}
	assert(simplex_count == 3);
	//求距离原点的最近点,或者也可以理解为计算原点的Voronoi域
	const Vec2 ab = simplex_array[1] - simplex_array[0];
	const Vec2 ac = simplex_array[2] - simplex_array[0];
	//A
	float fvab = -dot(simplex_array[0],ab);
	float fvac = -dot(simplex_array[0],ac);
	if (fvab < 0 && fvac < 0)
	{
		simplex_count = 1;
		return false;
	}
	//B
	const Vec2 bc = simplex_array[2] - simplex_array[1];
	float f2 = dot(simplex_array[1],ab);
	float f3 = -dot(simplex_array[1],bc);
	if (f2 < 0 && f3 < 0)
	{
		simplex_array[0] = simplex_array[1];
		simplex_count = 1;
		return false;
	}
	//C
	float f4 = dot(simplex_array[2], ac);
	float f5 = dot(simplex_array[2],bc);
	if (f4 < 0 && f5 < 0)
	{
		simplex_array[0] = simplex_array[2];
		simplex_count = 1;
		return false;
	}
	//此时并不能区分三角形的顶点顺序是顺时针的的还是逆时针的
	float s = cross(ab,ac);
	//AB
	if (fvab > 0 && f2 > 0 && cross(ab,simplex_array[0]) * s > 0)
	{
		simplex_count = 2;
		return false;
	}
	//BC
	if (f3 > 0 && f5 > 0 && cross(bc, simplex_array[1]) * s > 0)
	{
		simplex_array[0] = simplex_array[1];
		simplex_array[1] = simplex_array[2];
		simplex_count = 2;
		return false;
	}
	//CA
	if (f4 > 0 && fvac > 0 && cross(simplex_array[0], ac) * s > 0)
	{
		simplex_array[1] = simplex_array[2];
		simplex_count = 2;
		return false;
	}
	//此时原点一定在三角形之内
	return true;
}

static void static_compute_prefer_direction(const Vec2 *simplex_array,int simplex_count,Vec2 &direction)
{
	if (simplex_count == 1)
		direction = -simplex_array[0];
	else if (simplex_count == 2)
	{
		Vec2 normal = simplex_array[1] - simplex_array[0];
		//计算线段的方向与原点之间的关系
		if (cross(simplex_array[0], normal) > 0)
		{
			direction.x = -normal.y;
			direction.y = normal.x;
		}
		else
		{
			direction.x = normal.y;
			direction.y = -normal.x;
		}
	}
	assert(simplex_count < 3);
}

bool polygon_polygon_intersect_test(const Polygon &pa, const Polygon &pb, cocos2d::Vec2 &a, cocos2d::Vec2 &b)
{
	//初始给出一个方向,这里随机给出一个
	Vec2 direction = normalize(randomf10(),randomf10());
	Vec2  simplex_array[4];
	int      simplex_count = 0;

	Vec2  s = static_compute_support_point(pa, direction) - static_compute_support_point(pb,-direction);
	simplex_array[0] = s;
	simplex_count += 1;
	direction = -s;

	int index_l,loop_count = pa.point_array.size() + pb.point_array.size();
	for (index_l = 0; index_l < loop_count; ++index_l)
	{
		//计算支撑点
		s = static_compute_support_point(pa, direction) - static_compute_support_point(pb, -direction);
		//是否可知判断出两个凸体对象不相交
		if (dot(s, direction) < 0)
			break;
		//加入到单纯形中
		simplex_array[simplex_count++] = s;
		assert(simplex_count <= 3);
		//是否原点在上面所形成的单纯形之中,如果是则意味着两对象相交,否则给出下一个收敛方向
		bool intersect = static_compute_match_simplex(simplex_array,simplex_count);
		if (intersect)
			return true;
		static_compute_prefer_direction(simplex_array, simplex_count, direction);
	}
	return false;
}

//检测顶点的类型
enum VertexType
{
	VertexType_Start = 0,//开始顶点
	VertexType_Split = 1,//分裂顶点
	VertexType_Merge = 2,//汇合顶点
	VertexType_End = 3,//结束顶点
	VertexType_Normal = 4,//常规顶点
};

VertexType check_vertex_type(int  vertex_index,const std::vector<cocos2d::Vec2> &polygon_points)
{
	const int array_size = polygon_points.size();
	const Vec2  &target_point = polygon_points[vertex_index];
	const Vec2 &next_point = polygon_points[vertex_index < array_size -1? vertex_index +1: 0];
	const Vec2 &prev_point = polygon_points[vertex_index > 0? vertex_index - 1:array_size-1];

	float f = cross(target_point - prev_point, next_point - target_point);
	//如果内角小于Π,则检测其两边的顶点相对当前顶点的高度
	if (f >= 0)
	{
		if (prev_point.y <= target_point.y && next_point.y <= target_point.y)
			return VertexType_Start;
		if (prev_point.y >= target_point.y && next_point.y >= target_point.y)
			return VertexType_End;
	}
	else
	{
		if (prev_point.y >= target_point.y && next_point.y >= target_point.y)
			return VertexType_Merge;
		if (prev_point.y <= target_point.y && next_point.y <= target_point.y)
			return VertexType_Split;
	}
	assert(prev_point.y >= target_point.y && target_point.y >= next_point.y || prev_point.y <= target_point.y && target_point.y <= next_point.y);
	return VertexType_Normal;
}
//可以使用二分算法
void polygon_simple_decompose_insert(std::vector<int> &adj_edge, const std::vector<cocos2d::Vec2> &polygon_points, int point_index)
{
	int array_size = polygon_points.size();
	const Vec2 &target_point = polygon_points[point_index];
	int index_j = 0;
	for (; index_j < adj_edge.size(); ++index_j)
	{
		int  base_index = adj_edge[index_j];
		int  next_index = base_index < array_size - 1?base_index+1:0;
		const Vec2 &base_location = polygon_points[base_index];
		const Vec2 &next_location = polygon_points[next_index];
		//求线段[base_index,next_index]与水平线y = polygon_points[point_index].y的交点的x坐标
		float d_x = next_location.x - base_location.x;
		float d_y = next_location.y - base_location.y;
		float f = d_y!=0.0f?(target_point.y - base_location.y)/ d_y:0.0f;
		float x = base_location.x + d_x * f;

		if (target_point.x < x)
			break;
	}
	adj_edge.insert(adj_edge.begin() + index_j, point_index);
}
//删除目标边,可以使用二分算法
void polygon_simple_decompose_remove(int edge_number, std::vector<int> &adj_edge)
{
	for (int index_j = 0; index_j < adj_edge.size(); ++index_j)
	{
		if (adj_edge[index_j] == edge_number)
		{
			adj_edge.erase(adj_edge.begin() + index_j);
			break;
		}
	}
}
//查找左侧最邻近的边,实际上可以使用二分算法
int polygon_simple_decompose_check_left(std::vector<int> &adj_edge,const std::vector<cocos2d::Vec2> &polygon_points,int target_point_index)
{
	int array_size = polygon_points.size();
	const Vec2 &target_point = polygon_points[target_point_index];
	int index_j = adj_edge.size() - 1;
	for (; index_j >=0; --index_j)
	{
		int  base_index = adj_edge[index_j];
		int  next_index = base_index < array_size - 1 ? base_index + 1 : 0;
		const Vec2 &base_location = polygon_points[base_index];
		const Vec2 &next_location = polygon_points[next_index];
		//求线段[base_index,next_index]与水平线y = polygon_points[point_index].y的交点的x坐标
		float d_x = next_location.x - base_location.x;
		float d_y = next_location.y - base_location.y;
		float f = d_y != 0.0f ? (target_point.y - base_location.y) / d_y : 0.0f;
		float x = base_location.x + d_x * f;

		if (x < target_point.x)
			break;
	}
	return adj_edge[index_j];
}

bool polygon_simple_decompose(const std::vector<cocos2d::Vec2> &polygon_points, std::map<int,int>  &addtional_edge_map, std::vector<int> &points_sequence, std::vector<int> &boundary_index)
{
	//针对所有的多边形,记录其顶点的顺序
	struct PolygonVertex
	{
		Vec2   *vertex_location;
		int       vertex_index;
	};
	//记录所有的顶点,并排序,并且记录下最大以及最小的顶点
	std::vector<PolygonVertex>  polygon_vertexs(polygon_points.size());
	int	max_index = -1,min_index = -1,array_size = polygon_points.size();
	for (int index_l = 0; index_l < array_size; ++index_l)
	{
		polygon_vertexs[index_l].vertex_location = (Vec2 *)(polygon_points.data() + index_l);
		polygon_vertexs[index_l].vertex_index = index_l;

		if (max_index == -1 || polygon_points[max_index].y < polygon_points[index_l].y || (polygon_points[max_index].y == polygon_points[index_l].y && polygon_points[max_index].x > polygon_points[index_l].x))
			max_index = index_l;

		if (min_index == -1 || polygon_points[min_index].y > polygon_points[index_l].y || (polygon_points[min_index].y == polygon_points[index_l].y && polygon_points[min_index].x < polygon_points[index_l].x))
			min_index = index_l;
	}
	//对所有的顶点进行排序
	std::function<bool(const PolygonVertex &a, const PolygonVertex &b)> compare_func = [](const PolygonVertex &a, const PolygonVertex &b)->bool {
		return a.vertex_location->y > b.vertex_location->y || (a.vertex_location->y == b.vertex_location->y && a.vertex_location->x < b.vertex_location->x);
	};
	quick_sort<PolygonVertex>(polygon_vertexs.data(), array_size, compare_func);
	//针对每一个目标顶点,依次处理
#define check_orientation_left(vvv) (polygon_points[vvv].y <= polygon_points[vvv > 0?vvv-1:array_size-1].y && polygon_points[vvv].y >= polygon_points[vvv < array_size -1 ?vvv +1:0].y)
#define safe_insert_map(m,helper,next){assert(m.find(helper) == m.end());m[helper] = next;}
	std::map<int, int>  edge_helper;//边的助手
	std::vector<int>	adj_edge;//在目标顶点左侧的边的集合
	//所有新增加的边
	std::map<int,int>   other_edge_map;
	int helper, last_vertex, edge_index, target_edge_index;
	for (int index_j = 0;index_j < array_size;++index_j)//不会增加新的事件点
	{
		PolygonVertex &polygon_vertex = polygon_vertexs[index_j];
		int vertex_index = polygon_vertex.vertex_index;
		//依据顶点的类型,有不同的处理算法
		VertexType  vertex_type = check_vertex_type(vertex_index, polygon_points);
		switch (vertex_type)
		{
		case VertexType_Start:
			polygon_simple_decompose_insert(adj_edge, polygon_points, vertex_index);
			edge_helper[vertex_index] = vertex_index;
			break;
		case VertexType_Split://分裂顶点
			edge_index = polygon_simple_decompose_check_left(adj_edge, polygon_points, vertex_index);
			helper = edge_helper[edge_index];
			//other_edge_map[helper] = vertex_index;
			safe_insert_map(other_edge_map,helper, vertex_index);
			edge_helper[edge_index] = vertex_index;//更新助手
			edge_helper[vertex_index] = vertex_index;//更新助手
			polygon_simple_decompose_insert(adj_edge, polygon_points, vertex_index);
			break;
		case VertexType_Merge://汇合顶点的处理要稍微复杂一些
			last_vertex = vertex_index > 0 ? vertex_index - 1 : array_size - 1;
			helper = edge_helper[last_vertex];
			if (check_vertex_type(helper, polygon_points) == VertexType_Merge)
				safe_insert_map(other_edge_map, helper, vertex_index);// other_edge_map[helper] = vertex_index;
			polygon_simple_decompose_remove(last_vertex,adj_edge);
			//查找离顶点左侧最近的边
			target_edge_index = polygon_simple_decompose_check_left(adj_edge, polygon_points, vertex_index);
			helper = edge_helper[target_edge_index];
			if (check_vertex_type(helper, polygon_points) == VertexType_Merge)
				safe_insert_map(other_edge_map, helper, vertex_index); // other_edge_map[helper] = vertex_index;
			edge_helper[target_edge_index] = vertex_index;
			break;
		case VertexType_End:
			last_vertex = vertex_index > 0 ? vertex_index - 1 : array_size - 1;
			helper = edge_helper[last_vertex];
			if (check_vertex_type(helper, polygon_points) == VertexType_Merge)
				safe_insert_map(other_edge_map, helper, vertex_index); // other_edge_map[helper] = vertex_index;
			polygon_simple_decompose_remove(last_vertex, adj_edge);
			break;
		case VertexType_Normal://对于普通顶点的处理要稍微有点复杂
			if (check_orientation_left(vertex_index))//左侧
			{
				int  last_edge = vertex_index > 0 ? vertex_index - 1 :array_size-1;
				int helper = edge_helper[last_edge];
				if (check_vertex_type(helper,polygon_points) == VertexType_Merge)
					safe_insert_map(other_edge_map, helper, vertex_index); //other_edge_map[helper] = vertex_index;
				polygon_simple_decompose_remove(last_edge,adj_edge);
				polygon_simple_decompose_insert(adj_edge, polygon_points, vertex_index);
				edge_helper[vertex_index] = vertex_index;
			}
			else//右侧
			{
				int  left_edge = polygon_simple_decompose_check_left(adj_edge, polygon_points, vertex_index);
				int helper = edge_helper[left_edge];
				if (check_vertex_type(helper, polygon_points) == VertexType_Merge)
					safe_insert_map(other_edge_map, helper, vertex_index); //other_edge_map[helper] = vertex_index;
				edge_helper[left_edge] = vertex_index;
			}
		}
	}
#undef safe_insert_map
#undef check_orientation_left
	addtional_edge_map = other_edge_map;
	//polygon_triangular_cycle_sequence(array_size, other_edge_map, points_sequence, boundary_index);
	return points_sequence.size() > 0;
}

void polygon_simple_cycle_sequence(int points_number, const std::map<int, int> &adj_edge_map, std::vector<int> &points_sequence, std::vector<int> &boundary_index)
{
	//使用栈式处理算法,其核心思想类似于quick hull算法
	struct MonotonePartion
	{
		std::vector<int>			points_sequence_vec;
		std::set<int>					points_set;
	};
	//第一步,对现有的点集进行二元划分
	if (!adj_edge_map.size())
	{
		points_sequence.reserve(points_number);
		for (int j = 0; j < points_number; ++j)points_sequence.push_back(j);
		boundary_index.push_back(points_number);
		return;
	}
	std::list<MonotonePartion>		binary_partion_list;
	MonotonePartion   partion;
	std::map<int, int>  adj_edge_map_copy = adj_edge_map;
	auto it = adj_edge_map_copy.begin();
	//逆时针排序
	int  start_vertex = min_f(it->first,it->second);
	int  end_vertex = max_f(it->first,it->second);

	binary_partion_list.push_back(partion);
	binary_partion_list.push_back(partion);

	auto &top_partion = binary_partion_list.front();
	auto &bottom_partion = binary_partion_list.back();
	//上侧划分,注意,划分的过程中
	for (int base_j = start_vertex; base_j != end_vertex; base_j = (base_j + 1) % points_number)
		top_partion.points_sequence_vec.push_back(base_j),top_partion.points_set.insert(base_j);
	top_partion.points_sequence_vec.push_back(end_vertex);
	top_partion.points_set.insert(end_vertex);
	//下侧划分
	for (int base_j = start_vertex; base_j != end_vertex; base_j = (base_j - 1 + points_number) % points_number)
		bottom_partion.points_sequence_vec.push_back(base_j),bottom_partion.points_set.insert(base_j);
	bottom_partion.points_sequence_vec.push_back(end_vertex);
	bottom_partion.points_set.insert(end_vertex);
	//移除掉第一个连通边
	adj_edge_map_copy.erase(it);
	//针对已经划分出的部分,再次进行遍历
	while (binary_partion_list.size())
	{
		auto now_partion = binary_partion_list.front();
		binary_partion_list.pop_front();

		auto &points_sequence_vec = now_partion.points_sequence_vec;
		int  base_l = 0,secondary_l = -1;//记录下分裂的位置
		for (base_l = 0; base_l < points_sequence_vec.size(); ++base_l)
		{
			int  now_vertex = points_sequence_vec[base_l];
			auto itk = adj_edge_map_copy.find(now_vertex);//如果在该集合中查找到有分裂边
			if (itk != adj_edge_map_copy.end() && now_partion.points_set.find(itk->second) != now_partion.points_set.end())
			{
				secondary_l = itk->second;
				it = itk;
				break;
			}
		}
		//如果没有找到,则意味着此顶点序列所形成的环中没有其他新引入的边,因此可以直接写入
		if (secondary_l == -1)
		{
			points_sequence.insert(points_sequence.end(), points_sequence_vec.begin(), points_sequence_vec.end());
			boundary_index.push_back(points_sequence.size());
		}
		else//否则再次分裂,只是分裂的过程要稍微有些复杂
		{
			binary_partion_list.push_front(partion);
			auto &bottom_partion = binary_partion_list.front();

			binary_partion_list.push_front(partion);
			auto &top_partion = binary_partion_list.front();
			int  array_size = points_sequence_vec.size();
			//上侧划分
			for (int index_l = base_l; points_sequence_vec[index_l] != secondary_l; index_l = (index_l + 1) % array_size)
				top_partion.points_sequence_vec.push_back(points_sequence_vec[index_l]), top_partion.points_set.insert(points_sequence_vec[index_l]);
			top_partion.points_sequence_vec.push_back(secondary_l);
			top_partion.points_set.insert(secondary_l);
			//下册划分
			for (int index_l = base_l; points_sequence_vec[index_l] != secondary_l; index_l = (index_l - 1 + array_size) % array_size)
				bottom_partion.points_sequence_vec.push_back(points_sequence_vec[index_l]), bottom_partion.points_set.insert(points_sequence_vec[index_l]);
			bottom_partion.points_sequence_vec.push_back(secondary_l);
			bottom_partion.points_set.insert(secondary_l);
			//删除掉该连通边映射关系
			adj_edge_map_copy.erase(it);
		}
	}
}

void polygon_simple_generate(std::vector<cocos2d::Vec2> &polygon_points, std::vector<cocos2d::Vec2> &simple_polygon)
{
	if (&polygon_points != &simple_polygon)
		simple_polygon = polygon_points;
	//首先对顶点进行遍历,找出y坐标最小的点
	int   base_j = -1;
	for (int index_l = 0; index_l < simple_polygon.size(); ++index_l)
	{
		if (base_j == -1 || simple_polygon[index_l].y < simple_polygon[base_j].y || (simple_polygon[index_l].y == simple_polygon[base_j].y && simple_polygon[index_l].x < simple_polygon[base_j].x))
			base_j = index_l;
	}
	Vec2  compare_point = simple_polygon[base_j];
	std::function<bool(const Vec2 &a, const Vec2 &b)> compare_func = [compare_point](const Vec2 &a,const Vec2 &b)->bool {
		float a_x = a.x - compare_point.x;
		float a_y = a.y - compare_point.y;
		float f1 = atan2f(a_y,a_x);

		float b_x = b.x - compare_point.x;
		float b_y = b.y - compare_point.y;
		float f2 = atan2f(b_y,b_x);

		return f1 < f2 || f1 == f2 && a_x * a_x + a_y *a_y <= b_x * b_x + b_y * b_y;
	};
	quick_sort<Vec2>(simple_polygon.data(), (int)simple_polygon.size(), compare_func);
}

void polygon_monotone_triangulate(const std::vector<cocos2d::Vec2> &points_array, const int *sequence_array, int array_size, std::vector<int> &triangle_sequence, std::map<int, int> &addtional_edge_map)
{
	//首先对当前的顶点进行排序,其排序算法比较简单,因为已经假设输入的多边形为y单调多边形
	int   *sorted_points_array = new int[array_size];
	int     base_j = 0,secondary_l =0;
	//首先遍历数组,查找最高与最低的顶点,并计算多边形的CCW走向
	float polygon_sign_area = cross(points_array[sequence_array[array_size-1]], points_array[sequence_array[0]]);
	for (int index_l = 1; index_l < array_size; ++index_l)
	{
		int select_y = sequence_array[index_l];
		int  min_y = sequence_array[base_j];
		if (points_array[min_y].y < points_array[select_y].y || points_array[min_y].y == points_array[select_y].y && points_array[min_y].x > points_array[select_y].x)
			base_j = index_l;

		int max_y = sequence_array[secondary_l];
		if (points_array[max_y].y > points_array[select_y].y || points_array[max_y].y == points_array[select_y].y && points_array[max_y].x < points_array[select_y].x)
			secondary_l = index_l;
		polygon_sign_area += cross(points_array[sequence_array[index_l-1]], points_array[select_y]);
	}
	//对于y单调多边形,其顶点排序算法只需要一次遍历,即可完成
	int index_y = 0;
	sorted_points_array[index_y++] = base_j;
	for (int left = (base_j + 1) % array_size, right = (base_j - 1 + array_size) % array_size;left != secondary_l || right != secondary_l;)
	{
		//比较y值
		int translate_l = sequence_array[left];
		int translate_r = sequence_array[right];
		if (left != secondary_l && (points_array[translate_l].y > points_array[translate_r].y || points_array[translate_l].y == points_array[translate_r].y && points_array[translate_l].x < points_array[translate_r].x))
		{
			sorted_points_array[index_y] = left;
			left = (left + 1) % array_size;
		}
		else
		{
			assert(right != secondary_l);
			sorted_points_array[index_y] = right;
			right = (right - 1 + array_size) % array_size;
		}
		++index_y;
	}
	sorted_points_array[index_y++] = secondary_l;
	std::list<int>	sweep_stack;
	sweep_stack.push_back(0);
	sweep_stack.push_back(1);
	triangle_sequence.reserve(array_size * 2);
	//左侧值为1,右侧为-1,注意判断顺时针/逆时针的顺序并非那么的直观
	int ccw_type = polygon_sign_area > 0? 1 : -1;
#define check_vertex_boundary_type(secondary_index)  (points_array[sequence_array[secondary_index]].y <= points_array[sequence_array[(secondary_index - ccw_type + array_size)%array_size]].y && points_array[sequence_array[secondary_index]].y >= points_array[sequence_array[(secondary_index + ccw_type + array_size)%array_size]].y?1:-1)
#define check_secondary_index(idx)	sequence_array[sorted_points_array[idx]]
#define safe_insert_map(m,s,t) {triangle_sequence.push_back(s);triangle_sequence.push_back(t);}//auto i = m.find(s);if(i == m.end())m[s] = t;else m[t] =s;}
	for (int index_l = 2; index_l < index_y - 1; ++index_l)
	{
		int  now_index = sorted_points_array[index_l];
		int real_index = sequence_array[now_index];

		int  now_type = check_vertex_boundary_type(now_index);
		//检测栈顶元素的类型
		int top_index = sweep_stack.back();
		int top_type = check_vertex_boundary_type(sorted_points_array[top_index]);
		//如果在异侧
		if (now_type * top_type < 0)
		{
			//除了最后一个元素之外的其他元素,分别插入对角线
			int bottom_index = sweep_stack.front();
			while (sweep_stack.size())
			{
				int perform_index = sweep_stack.back();
				sweep_stack.pop_back();
				if (perform_index != bottom_index)
					safe_insert_map(addtional_edge_map,check_secondary_index(perform_index),real_index);// addtional_edge_map[check_secondary_index(perform_index)] = real_index;
			}
			sweep_stack.push_back(top_index);//top_index == (index_l -1 + array_size)%array_size
			sweep_stack.push_back(index_l);
		}
		else//如果在同侧,则处理算法要稍微复杂一些
		{
			sweep_stack.pop_back();//第一个元素除外
			int  last_perform_index = top_index;
			while (sweep_stack.size() && cross(points_array[real_index], points_array[check_secondary_index(last_perform_index)], points_array[check_secondary_index(sweep_stack.back())]) * now_type < 0)
			{
				last_perform_index = sweep_stack.back();
				sweep_stack.pop_back();
				safe_insert_map(addtional_edge_map, check_secondary_index(last_perform_index), real_index); // addtional_edge_map[check_secondary_index(last_perform_index)] = real_index;
			}
			sweep_stack.push_back(last_perform_index);
			sweep_stack.push_back(index_l);
		}
	}
	//针对最后一个元素,需要特殊处理
	if (sweep_stack.size())
		sweep_stack.pop_back();
	if (sweep_stack.size())
		sweep_stack.pop_front();
	for (auto it = sweep_stack.begin(); it != sweep_stack.end(); ++it)
		safe_insert_map(addtional_edge_map, check_secondary_index(*it), check_secondary_index(array_size - 1));//addtional_edge_map[check_secondary_index(*it)] = check_secondary_index(array_size - 1);

	delete[] sorted_points_array;
	sorted_points_array = nullptr;

#undef safe_insert_map
#undef check_secondary_index
#undef check_vertex_boundary_type
}
NS_GT_END