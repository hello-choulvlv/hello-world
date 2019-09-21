/*
 *��ɢ�㼯�����δ���
 *2019/6/16
 *Author:С����
 */
#include "point_polygon/point_polygon.h"
#include "matrix/matrix.h"
#include "line/line.h"
#include "cycle_sphere/cycle_sphere.h"
#include<unordered_map>
//#include<map>
#include<stack>
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
	//ע�⼴ʹ��2d�ռ���,����Ҳ����ʹ�ó�ƽ�����,��������ϵ�任��Ҳ������������ϵ
	polygon.plane_array.reserve(points.size());
	Plane2D  plane;
	for (int index_l = 0; index_l < points.size(); ++index_l)
	{
		const Vec2 &a = points[index_l];
		const Vec2 &b = points[index_l +1 >= points.size()?0:index_l +1];
		const Vec2 direction = normalize(b - a);
		//��Ҫ����һ�ξ���任,�������
		plane.normal.x = -direction.y;
		plane.normal.y = direction.x;
		plane.distance = dot(plane.normal,a);
		polygon.plane_array.push_back(plane);
	}
	polygon.point_array = points;
}
/*
  *ֱ��/�߶�/���������ε��ཻ����
 */
static bool polygon_unkonwn_intersect_test(const Polygon &polygon, const Vec2 &a,const Vec2 &direction,float min_f,float max_f)
{
	//��Զ���ε��ཻ����
	for (int index_l = 0; index_l < polygon.plane_array.size(); ++index_l)
	{
		const Plane2D  &plane = polygon.plane_array[index_l];
		float f = dot(plane.normal, direction);
		float d = dot(plane.normal,a) - plane.distance;
		//���ֱ�������ε�ĳһ���ߴ�ֱ
		if (fabsf(f) < 0.001f)
		{
			//���ֱ������Զ���˶���ε�ĳһ����,��ʱ�ض�����
			if (d < 0)
				return false;
		}
		else
		{
			//���ֱ�������εĽ���
			float t = -d / f;
			if (f < 0.0f)//��ʱ���ߵķ������˹��ߵķ���
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
	return polygon_unkonwn_intersect_test(polygon,*(Vec2*)&ray.origin,*(Vec2*)&ray.direction,0,FLT_MAX);
}

bool polygon_contains_point(const Polygon &polygon, const cocos2d::Vec2 &point)
{
	int l = 0, h = polygon.point_array.size();
	const Vec2 &v0 = polygon.point_array[0];
	const Vec2 interpolation = point - v0;
	do 
	{
		int m = (l+h) >> 1;
		if (sign_area(polygon.point_array[m] - v0, interpolation) >= 0)//���
			l = m;
		else
			h = m;
	} while (l +1 < h);
	//�������������Ҳ�
	if (l == 0 || h == polygon.point_array.size())
		return 0;
	return sign_area(polygon.point_array[h] - polygon.point_array[l],point - polygon.point_array[l]) >=0.0f;
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
	//����ÿһ����,�����������Ĺ�ϵ
	float r2 = cycle.radius * cycle.radius;
	for (int index_l = 0; index_l < polygon.plane_array.size(); ++index_l)
	{
		const Plane2D &plane = polygon.plane_array[index_l];
		//���Բ���ڳ�ƽ��֮��
		float d = dot(plane.normal,cycle.center) - plane.distance;
		if (d < -cycle.radius)
			return false;
		//���Բ�ľ��볬ƽ���ڰ뾶֮��,����Ҫ�����߶�������֮��ľ�����
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
	//����ڸ�����ռ�Ľ���֮��,��ִ�б�׼�Ķ����/���������
	//����ֱ�ӷ���true,��һ�������,����Ĵ����޷��ų���,����Բͬʱ���������ڵ�ƽ���ཻ,����ȴû���������ཻ
	return polygon_contains_point(polygon, cycle.center);
}
//not Optimal
template<typename TK>
void quick_sort(TK *source, int tk_num, std::function<bool(const TK &a, const TK &b)> &compare_func)
{
	//�����㷨Ŀǰ�Ȳ��ò�������,�������ǽ���ʹ�ù鲢����
	TK   *bubble = new TK[tk_num];
	int     step = 1, half = tk_num / 2;
	TK  *t1 = source, *t2 = bubble;
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
		TK *t = t1;
		t1 = t2; t2 = t;
	}
	if (t1 != source)
		memcpy(source, t1, sizeof(TK) * tk_num);

	delete[] bubble;
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
	//��d_pa��ʾΪd_ab,d_ac���������
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
	//��d_pa��ʾΪd_ab,d_ac���������
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

bool polygon_compute_minimum(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon_points)
{
	if (points.size() <= 3) return false;
	//��һ��,�Զ����������
	std::vector<Vec2> points_tmp = points;
	//����y������С,����x�����С�ĵ�
	int target_j = 0;
	for (int index_j = 1; index_j < points_tmp.size(); ++index_j)
	{
		if (points_tmp[index_j].y < points_tmp[target_j].y || (points_tmp[index_j].y == points_tmp[target_j].y && points_tmp[index_j].x > points_tmp[target_j].x))
			target_j = index_j;
	}

	Vec2 f_point = points_tmp[target_j];
	points_tmp.erase(points_tmp.begin() + target_j);
	std::function<bool(const Vec2 &a, const Vec2 &b)>  compare_func = [f_point](const Vec2 &a, const Vec2 &b)->bool {
		const float d_x = a.x - f_point.x;
		const float d_y = a.y - f_point.y;

		const float f_x = b.x - f_point.x;
		const float f_y = b.y - f_point.y;

		float angle_a = atan2f(d_y, d_x);
		float angle_b = atan2f(f_y, f_x);
		//����Ƕ���ͬ,����ο���������������λ�ø���ǰ
		return angle_a < angle_b || (angle_a == angle_b && d_x * d_x + d_y * d_y < f_x * f_x + f_y * f_y);//���⼫����ͬ�ĵ㼯
	};
	quick_sort<Vec2>(points_tmp.data(), (int)points_tmp.size(), compare_func);

	polygon_points.reserve(points.size());
	polygon_points.push_back(f_point);
	polygon_points.push_back(points_tmp[0]);

	for (int index_j = 1; index_j < points_tmp.size(); ++index_j)
	{
		//�Ե�ǰ�ĵ�������
		Vec2 &target_point = points_tmp.at(index_j);
		//����Ƿ�����ǰ��������������һ�µ�
		while (polygon_points.size() > 1 && sign_area(polygon_points.back() - polygon_points[polygon_points.size() - 2], target_point - polygon_points.back()) <= 0)
		{
			polygon_points.pop_back();
		}
		polygon_points.push_back(target_point);
	}

	return polygon_points.size() >= 3;
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
	//ע���㷨Ϊ�˼��׶�,��˲�û�о����Ż�
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
//���������㷨ʵ��
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
	//����
	std::function<bool(const Vec2Type &a, const Vec2Type &b)> compare_func = [](const Vec2Type &a, const Vec2Type &b)->bool {
		return a->x < b->x;
	};
	quick_sort<Vec2Type>(sorted_points_array.data(), (int)sorted_points_array.size(),compare_func);
	//���Ѿ��������ɢ�㼯���д���
	const int  boundary = points.size();
	Vec2  **buffer_array = new Vec2  *[boundary >> 1];
	int      buffer_size = 0;
	float distance = FLT_MAX;
	float *sub_distance_array = new float[(boundary >> 1) + 1];
	for (int base_j = 0; base_j < boundary; base_j += 2)
	{
		float x = FLT_MAX;
		float s = sorted_points_array[base_j]->x;//�ָ���
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
				break;//��ʱѭ�����ؽ�����ȥ��
			}
			float s = sorted_points_array[other_j -1]->x;
			float d = fminf(sub_distance_array[base_j * 2/step], sub_distance_array[other_j*2/step]);
			//�ֱ�ӷָ�������ȡ��d��Χ�ڵĶ���,���Ұ���Y�����������
			buffer_size = 0;
			//�������
			int index_l = other_j-1;
			for (; index_l >= base_j && sorted_points_array[index_l]->x + d > s; --index_l)
				static_sort_insert<Vec2>(buffer_array,buffer_size++, sorted_points_array[index_l]);
			//�Ҳ�����
			index_l = other_j;
			int boundary_r = other_j + step / 2 >= boundary ? boundary : other_j + step / 2;
			for (; index_l < boundary_r && sorted_points_array[index_l]->x - d < s;++index_l)
				static_sort_insert<Vec2>(buffer_array, buffer_size++, sorted_points_array[index_l]);
			//���������,��Ҫ�ȽϵĶ�����Ŀ���ᳬ���߸�
			float f = static_two_space_min_distance<Vec2,8>(buffer_array, buffer_size,distance, a, b);
			//���������д�뵽��������
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
	//ע�������㷨
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
//�ռ仮��
static void static_yz_space_partion(std::unordered_map<int64_t, PartionVec3> &partion_map,Vec3 &ref_location,Vec3 **buffer_array,int buffer_size,int extent,float interval)
{
	float t2 = interval * 2.0f;
	for (int index_l = 0; index_l < buffer_size; ++index_l)
	{
		Vec3 *location = buffer_array[index_l];
		int64_t y = (location->y - ref_location.y) / t2;
		int64_t z = (location->z - ref_location.z) / t2;
		//�����������ʵ�ʵ���������Ҫ��̬����
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
	//����Ƿ��������ڵĿռ����ڵĿռ�,��Ҫ���4��
	int64_t y = (location->y - ref_location.y)/ t2;
	int64_t z = (location->z - ref_location.z)/ t2;

	float fract_y = location->y - y *t2;
	float fract_z = location->z - z * t2;
	//
	PartionVec3   *compare_array[4];
	int compare_size = 0;
	
	int64_t base_key = y * extent + z;
	//���ѡ��4��,��һ����Ƚ�����
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
	//����
	std::function<bool(const Vec3Type &a, const Vec3Type &b)> compare_func = [](const Vec3Type &a, const Vec3Type &b)->bool {
		return a->x < b->x;
	};
	quick_sort<Vec3Type>(sorted_points_array.data(), (int)sorted_points_array.size(), compare_func);
	//���Ѿ��������ɢ�㼯���д���
	const int  boundary = points.size();
	Vec3  **buffer_array = new Vec3  *[boundary >> 1];
	int      buffer_size = 0;
	float distance = FLT_MAX;
	float *sub_distance_array = new float[(boundary >> 1) + 1];
	for (int base_j = 0; base_j < boundary; base_j += 2)
	{
		float x = FLT_MAX;
		float s = sorted_points_array[base_j]->x;//�ָ���
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
				break;//��ʱѭ�����ؽ�����ȥ��
			}
			float s = sorted_points_array[other_j - 1]->x;
			float d = fminf(sub_distance_array[base_j * 2 / step], sub_distance_array[other_j * 2 / step]);
			//�ֱ�ӷָ�������ȡ��d��Χ�ڵĶ���,���Ұ���Y�����������
			buffer_size = 0;
			//---------------------------�Ҳ�����-----------------------------------
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
			//���y������С�ĵ�
			float f = FLT_MAX;
			if (buffer_size > 0)
			{
				extent = ceilf((max_location.z - ref_location.z) * 0.5f/d);
				static_yz_space_partion(partion_map, ref_location, buffer_array,buffer_size,extent,d);
				//�������
				index_l = other_j - 1;
				for (; index_l >= base_j && sorted_points_array[index_l]->x + d > s; --index_l)
				f = fminf(f,static_two_space_min_distance(partion_map, ref_location, sorted_points_array[index_l], extent,d, distance, a, b));
				//���㷨������Ҫ���Ż�����partion_map���ڴ��ط���
				partion_map.clear();
			}
			//���������д�뵽��������
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
  *�������εļȶ������֧�ŵ�
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
	//������ݵ����ε�ά������������������
	if (simplex_count == 2)//��ʱ��Ҫ�����µķ���
	{
		Vec2  normal = simplex_array[1] - simplex_array[0];
		//����ԭ�����߶�֮�����Թ�ϵ
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
	//�����ԭ��������,����Ҳ�������Ϊ����ԭ���Voronoi��
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
	//��ʱ���������������εĶ���˳����˳ʱ��ĵĻ�����ʱ���
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
	//��ʱԭ��һ����������֮��
	return true;
}

static void static_compute_prefer_direction(const Vec2 *simplex_array,int simplex_count,Vec2 &direction)
{
	if (simplex_count == 1)
		direction = -simplex_array[0];
	else if (simplex_count == 2)
	{
		Vec2 normal = simplex_array[1] - simplex_array[0];
		//�����߶εķ�����ԭ��֮��Ĺ�ϵ
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
	//��ʼ����һ������,�����������һ��
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
		//����֧�ŵ�
		s = static_compute_support_point(pa, direction) - static_compute_support_point(pb, -direction);
		//�Ƿ��֪�жϳ�����͹������ཻ
		if (dot(s, direction) < 0)
			break;
		//���뵽��������
		simplex_array[simplex_count++] = s;
		assert(simplex_count <= 3);
		//�Ƿ�ԭ�����������γɵĵ�����֮��,���������ζ���������ཻ,���������һ����������
		bool intersect = static_compute_match_simplex(simplex_array,simplex_count);
		if (intersect)
			return true;
		static_compute_prefer_direction(simplex_array, simplex_count, direction);
	}
	return false;
}
//quick hull�㷨ʵ��
bool quick_hull_algorithm2d(const std::vector<cocos2d::Vec2> &points, std::vector<cocos2d::Vec2> &polygon)
{
	//��һ���������½������ϽǵĶ���,�����������Ȼ����͹����ε�����֧�ŵ�
	int base_l = 0, base_t = 0;
	for (int index_l = 1; index_l < points.size(); ++index_l)
	{
		const Vec2 &compare_point = points.at(base_l);
		const Vec2 &secondary_point = points.at(base_t);
		const Vec2 &target_point = points.at(index_l);
		if (target_point.x < compare_point.x || target_point.x == compare_point.x && target_point.y < compare_point.y)
			base_l = index_l;
		if (target_point.x > secondary_point.x || target_point.x == secondary_point.x && target_point.y > secondary_point.y)
			base_t = index_l;
	}
	struct QuickHull
	{
		int start_l, final_l;
		std::vector<int>      operate_points;
	};
	//���ζ����еĶ�����л���,��������������
	std::list<QuickHull>   stack_polygon;
	const Vec2 &base_point = points[base_t];
	Vec2  direction = base_point - points[base_l];
	//��Ҫ��������
	QuickHull   quick_vertex;
	stack_polygon.push_back(quick_vertex);
	stack_polygon.push_back(quick_vertex);
	
	QuickHull  &top = stack_polygon.back();
	top.start_l = base_l;
	top.final_l = base_t;
	top.operate_points.reserve(points.size()-2);

	QuickHull &back = stack_polygon.front();
	back.start_l = base_t;
	back.final_l = base_l;
	back.operate_points.reserve(points.size() - 2);
	//������еĶ���,���л���
	for (int index_l = 0; index_l < points.size(); ++index_l)
	{
		if (index_l != base_l && index_l != base_t)
		{
			const Vec2 &target_point = points.at(index_l);
			float f = -direction.y * (target_point.x - base_point.x) + direction.x * (target_point.y - base_point.y);
			if (f < 0)
				top.operate_points.push_back(index_l);
			else if (f > 0)
				back.operate_points.push_back(index_l);
		}
	}
	polygon.reserve(64);
	//������еĶ�ջ���д���
	while (stack_polygon.size() > 0)
	{
		QuickHull &quick_hull = stack_polygon.back();
		//��ʱ�ñ��Ѿ�û�й���������,��Ҫ�Ƴ���,���ѸñߵĵͶ˵Ķ˵�д�뵽����εĵ㼯����
		if (!quick_hull.operate_points.size())
		{
			polygon.push_back(points.at(quick_hull.start_l));
			stack_polygon.pop_back();
		}
		else//������Ҫ����ñ����������еĺ�ѡ����,ע�������Ҫ���ѱ�,����Ѻ��˳����������ʱ���,�������ó��Ķ���ζ��㽫�Ǵ��ҵ�
		{
			const Vec2 &base_point = points.at(quick_hull.start_l);
			direction.x = points.at(quick_hull.final_l).y - base_point.y;
			direction.y = base_point.x - points.at(quick_hull.final_l).x;
			
			int target_l = -1;
			float max_f = -FLT_MAX;
			for (int index_l = 0; index_l < quick_hull.operate_points.size(); ++index_l)
			{
				int select_l = quick_hull.operate_points[index_l];
				float f = dot(direction,points[select_l] - base_point);
				if (f > max_f)
				{
					max_f = f;
					target_l = select_l;
				}
			}
			assert(target_l != -1);
			//ѡ����ĳһ���,����Ҫ���ѱ�,ע����ѵĴ���
			QuickHull  secondary_hull;
			secondary_hull.start_l = target_l;
			secondary_hull.final_l = quick_hull.final_l;
			secondary_hull.operate_points.reserve(quick_hull.operate_points.size()-1);

			QuickHull last_hull;
			last_hull.start_l = quick_hull.start_l;
			last_hull.final_l = target_l;
			last_hull.operate_points.reserve(quick_hull.operate_points.size() - 1);

			const Vec2 &other_point = points.at(target_l);
			direction.x = other_point.y - points.at(quick_hull.start_l).y;
			direction.y = points.at(quick_hull.start_l).x - other_point.x;
			//��һ����
			const Vec2  d2 = Vec2(points.at(quick_hull.final_l).y - other_point.y, other_point.x - points.at(quick_hull.final_l).x);
			//�Զ�����л���
			for (int index_l = 0; index_l < quick_hull.operate_points.size(); ++index_l)
			{
				int  target_index = quick_hull.operate_points[index_l];
				if (target_index != target_l)
				{
					const Vec2 interpolation = points.at(target_index) - other_point;
					//�������������ͬʱ����
					if (dot(direction, interpolation) > 0)
						last_hull.operate_points.push_back(target_index);
					else if (dot(d2, interpolation) > 0)
						secondary_hull.operate_points.push_back(target_index);
				}
			}
			//�Ƴ���ԭ����
			stack_polygon.pop_back();
			stack_polygon.push_back(secondary_hull);
			stack_polygon.push_back(last_hull);
		}
	}
	return polygon.size() >= 3;
}
NS_GT_END