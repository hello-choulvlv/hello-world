/*
  *二维,三维圆球体基本函数实现
  *2019/6/16
  *@author:xiaohuaxiong
 */
#include "cycle_sphere.h"
#include "matrix/matrix.h"
#include "line/line.h"
#include<string.h>
using namespace cocos2d;
NS_GT_BEGIN
bool check_point_insideof_cycle(const Cycle &cycle, const cocos2d::Vec2 &point)
{
	float d_x = point.x - cycle.center.x;
	float d_y = point.y - cycle.center.y;

	return d_x * d_x + d_y * d_y < cycle.radius * cycle.radius;
}
///////////////////////////////////base////////////////////////////
void sphere_create(Sphere &sphere, const cocos2d::Vec3 &center, float radius)
{
	sphere.center = center;
	sphere.radius = radius;
}

void  compute_points_cycle_normal(const std::vector<cocos2d::Vec2> &points, Cycle &cycle)
{
	cycle.center = (points[0] + points[1]) * 0.5f;
	float d_x = points[0].x - cycle.center.x;
	float d_y = points[0].y - cycle.center.y;
	cycle.radius = sqrtf(d_x * d_x + d_y * d_y);
	for (int index_j = 2; index_j < points.size(); ++index_j)
	{
		const Vec2 &point = points.at(index_j);
		d_x = point.x - cycle.center.x;
		d_y = point.y - cycle.center.y;
		float  new_radius = sqrtf(d_x * d_x + d_y * d_y);
		if (new_radius > cycle.radius)
		{
			float trunk_radius = (new_radius + cycle.radius) * 0.5f;
			float f = (trunk_radius - cycle.radius) / new_radius;

			cycle.center.x += d_x * f;
			cycle.center.y += d_y * f;
			cycle.radius = trunk_radius;
		}
	}
}

void cycle_create(Cycle &cycle, const cocos2d::Vec2 &center, float radius)
{
	cycle.center = center;
	cycle.radius = radius;
}

void  cycle_create(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b, Cycle &cycle)
{
	cycle.center.x = (a.x + b.x) * 0.5f;
	cycle.center.y = (a.y + b.y) * 0.5f;

	float d_x = a.x - cycle.center.x;
	float d_y = a.y - cycle.center.y;

	cycle.radius = sqrtf(d_x * d_x + d_y * d_y);
}

void  cycle_create(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b, const cocos2d::Vec2 &c, Cycle &cycle)
{
	//需要用到行列式
	//https://zh.wikipedia.org/wiki/%E5%A4%96%E6%8E%A5%E5%9C%93
	//求半径
	Vec2  pb = b - a;
	Vec2  pc = c - a;
	Vec2  bc = c - b;

	float	length_ab = pb.length();
	float   length_ac = pc.length();
	float   length_bc = bc.length();
	//三点重合
	if (fabs(cross(pb, pc)) <= 0.0001f)
	{
		float dot_f = dot(pb, pc);
		cycle.center = dot_f >= 0.0f ? (length_ab >= length_ac ? (a + b) * 0.5 : (a + c) * 0.5f) : (b + c) * 0.5f;
		cycle.radius = dot_f >= 0.0f ? (cycle.center - a).length() : (cycle.center - b).length();
		return;
	}

	float  cos_v = pb.dot(pc) / (length_ab * length_ac);
	float  sin_v = sqrtf(1.0f - cos_v * cos_v);
	cycle.radius = length_bc / (2.0f * sin_v);
	//圆心
	float   det_f = 2.0f * ((a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x));
	float   x = ((a.x - c.x) * (a.x + c.x) + (a.y - c.y) * (a.y + c.y)) * (b.y - c.y) - ((b.x - c.x) * (b.x + c.x) + (b.y - c.y) * (b.y + c.y)) * (a.y - c.y);
	float  y = (a.x - c.x) * ((b.x - c.x) * (b.x + c.x) + (b.y - c.y) * (b.y + c.y)) - (b.x - c.x) * ((a.x - c.x) * (a.x + c.x) + (a.y - c.y) * (a.y + c.y));

	cycle.center.x = x / det_f;
	cycle.center.y = y / det_f;
}

void  create_mininum_cycle3(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b, const cocos2d::Vec2 &c, Cycle &cycle)
{
	Vec2  c_a = a - c;
	Vec2  a_b = b - a;
	Vec2  b_c = c - b;
	const Vec2  *edge_array[5] = { &a_b,&b_c,&c_a,&a_b,&b_c };
	const Vec2  *vertex_array[5] = { &a,&b,&c,&a,&b };
	float   dot_array[3] = { -dot(a_b,c_a),-dot(a_b,b_c),-dot(c_a,b_c) };
	float   triangle_area = fabs(cross(a_b, c_a));

	for (int index_l = 0; index_l < 3; ++index_l)
	{
		//判断是否有钝角/直角
		if (dot_array[index_l] <= 0.0f && triangle_area > 0.001f)
		{
			cycle.center = (*vertex_array[index_l + 1] + *vertex_array[index_l + 2]) * 0.5f;
			float d_x = cycle.center.x - vertex_array[index_l + 1]->x;
			float d_y = cycle.center.y - vertex_array[index_l + 1]->y;
			cycle.radius = sqrtf(d_x * d_x + d_y * d_y);
			return;
		}
		else if (triangle_area <= 0.001f)//如果有三点共线
		{
			if (dot_array[index_l] >= 0)
				cycle.center = (edge_array[index_l]->length() >= edge_array[index_l + 2]->length()) ? (*vertex_array[index_l] + *vertex_array[index_l + 1])*0.5f : (*vertex_array[index_l] + *vertex_array[index_l + 2]) * 0.5f;
			else
				cycle.center = (*vertex_array[index_l + 2] + *vertex_array[index_l + 2]) *0.5f;

			float d_x = cycle.center.x - vertex_array[index_l]->x;
			float d_y = cycle.center.y - vertex_array[index_l]->y;
			cycle.radius = sqrtf(d_x * d_x + d_y * d_y);
			return;
		}
	}

	//标准的外接圆
	float  cos_v = dot_array[0] / (a_b.length() * c_a.length());
	float  sin_v = sqrtf(1.0f - cos_v * cos_v);
	cycle.radius = b_c.length() / (2.0f * sin_v);
	//圆心
	float   det_f = 2.0f * ((a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x));
	float   x = ((a.x - c.x) * (a.x + c.x) + (a.y - c.y) * (a.y + c.y)) * (b.y - c.y) - ((b.x - c.x) * (b.x + c.x) + (b.y - c.y) * (b.y + c.y)) * (a.y - c.y);
	float  y = (a.x - c.x) * ((b.x - c.x) * (b.x + c.x) + (b.y - c.y) * (b.y + c.y)) - (b.x - c.x) * ((a.x - c.x) * (a.x + c.x) + (a.y - c.y) * (a.y + c.y));

	cycle.center.x = x / det_f;
	cycle.center.y = y / det_f;
}

void  compute_points_minimum_cycle(const std::vector<cocos2d::Vec2> &points, Cycle &cycle)
{
	float  d_x = points[1].x - points[0].x;
	float  d_y = points[1].y - points[0].y;
	cycle.center = (points[1] + points[0]) * 0.5f;
	cycle.radius = sqrtf(d_x * d_x + d_y * d_y) * 0.5f;
	//Welzl算法
	for (int index_l = 2; index_l < points.size(); ++index_l)
	{
		const Vec2 &point = points.at(index_l);
		//如果后续的点在圆内,则保持不变,否则,使用前面的点更新
		if (!check_point_insideof_cycle(cycle, point))
		{
			//新的圆以[0,index_l]顶点的连线为半径,中心为圆心
			cycle_create(point, points.front(), cycle);
			for (int index_j = 1; index_j < index_l; ++index_j)
			{
				const Vec2 &secondary_point = points.at(index_j);
				if (!check_point_insideof_cycle(cycle, secondary_point))
				{
					cycle_create(point, secondary_point, cycle);
					for (int index_k = 0; index_k < index_j; ++index_k)
					{
						const Vec2 &triple_point = points.at(index_k);
						if (!check_point_insideof_cycle(cycle, triple_point))
							create_mininum_cycle3(point, secondary_point, triple_point, cycle);
					}
				}
			}
		}
	}
}
////////////////////////////Sphere-Begin//////////////////////////////
void  create_mininum_cycle3(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, Cycle3 &cycle)
{
	Vec3  c_a = a - c;
	Vec3  a_b = b - a;
	Vec3  b_c = c - b;
	cycle.normal = cross_normalize(a_b, -c_a);

	const Vec3  *edge_array[5] = { &a_b,&b_c,&c_a,&a_b,&b_c };
	const Vec3  *vertex_array[5] = { &a,&b,&c,&a,&b };
	float   dot_array[3] = { -dot(a_b,c_a),-dot(a_b,b_c),-dot(c_a,b_c) };
	float   triangle_area = cross(a_b, c_a).length();

	for (int index_l = 0; index_l < 3; ++index_l)
	{
		//判断是否有钝角/直角
		if (dot_array[index_l] <= 0.0f && triangle_area > 0.001f)
		{
			cycle.center = (*vertex_array[index_l + 1] + *vertex_array[index_l + 2]) * 0.5f;
			float d_x = cycle.center.x - vertex_array[index_l + 1]->x;
			float d_y = cycle.center.y - vertex_array[index_l + 1]->y;
			float d_z = cycle.center.z - vertex_array[index_l + 1]->z;
			cycle.radius = sqrtf(d_x * d_x + d_y * d_y + d_z * d_z);
			return;
		}
		else if (triangle_area <= 0.001f)//如果有三点共线
		{
			if (dot_array[index_l] >= 0)
				cycle.center = (edge_array[index_l]->length() >= edge_array[index_l + 2]->length()) ? (*vertex_array[index_l] + *vertex_array[index_l + 1])*0.5f : (*vertex_array[index_l] + *vertex_array[index_l + 2]) * 0.5f;
			else
				cycle.center = (*vertex_array[index_l + 2] + *vertex_array[index_l + 2]) *0.5f;

			float d_x = cycle.center.x - vertex_array[index_l]->x;
			float d_y = cycle.center.y - vertex_array[index_l]->y;
			float d_z = cycle.center.z - vertex_array[index_l]->z;
			cycle.radius = sqrtf(d_x * d_x + d_y * d_y + d_z * d_z);
			return;
		}
	}
	//标准的外接圆
	float  cos_v = dot_array[0] / (a_b.length() * c_a.length());
	float  sin_v = sqrtf(1.0f - cos_v * cos_v);
	cycle.radius = b_c.length() / (2.0f * sin_v);

	//Vec3  c_a = a - c;
	//Vec3  a_b = b - a;
	//Vec3  b_c = c - b;

	float d1 = -dot(c_a, a_b);
	float d2 = -dot(a_b, b_c);
	float d3 = -dot(b_c, c_a);
	float c1 = d2 * d3;
	float c2 = d1 * d3;
	float c3 = d1 * d2;
	float cc = 2.0f * (c1 + c2 + c3);
	Vec3  v = a * (c2 + c3) + b * (c1 + c3) + c * (c1 + c2);
	cycle.center.x = v.x / cc;
	cycle.center.y = v.y / cc;
	cycle.center.z = v.z / cc;
}

void  create_sphere2(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, Sphere &sphere)
{
	sphere.center = (a + b) * 0.5f;
	float	d_x = sphere.center.x - a.x;
	float	d_y = sphere.center.y - a.y;
	float	d_z = sphere.center.z - a.z;
	sphere.radius = sqrtf(d_x * d_x + d_y * d_y + d_z * d_z);
}
/*
*此算法的思想与create_cycle3类似
*/
void  create_sphere3(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, Sphere &sphere)
{
	Vec3  c_a = a - c;
	Vec3  a_b = b - a;
	Vec3  b_c = c - b;
	const Vec3  *edge_array[5] = { &a_b,&b_c,&c_a,&a_b,&b_c };
	const Vec3  *vertex_array[5] = { &a,&b,&c,&a,&b };
	float   dot_array[3] = { -dot(a_b,c_a),-dot(a_b,b_c),-dot(c_a,b_c) };
	float   triangle_area = cross(a_b, c_a).length();

	if (triangle_area <= 0.001f)
	{
		sphere.center = a;
		if (dot_array[0] >= 0)
		{
			sphere.center += length2(a_b) > length2(c_a) ? b : c;
			sphere.radius = (sphere.center - a).length();
		}
		else
		{
			sphere.center = (b + c) * 0.5f;
			sphere.radius = b_c.length() * 0.5f;
		}
	}

	for (int index_l = 0; index_l < 3; ++index_l)
	{
		//判断是否有钝角/直角
		if (dot_array[index_l] <= 0.0f)
		{
			sphere.center = (*vertex_array[index_l + 1] + *vertex_array[index_l + 2]) * 0.5f;
			float d_x = sphere.center.x - vertex_array[index_l + 1]->x;
			float d_y = sphere.center.y - vertex_array[index_l + 1]->y;
			float d_z = sphere.center.z - vertex_array[index_l + 1]->z;
			sphere.radius = sqrtf(d_x * d_x + d_y * d_y + d_z * d_z);
			return;
		}
	}
	//标准的外接圆
	float  cos_v = dot_array[0] / (a_b.length() * c_a.length());
	float  sin_v = sqrtf(1.0f - cos_v * cos_v);
	sphere.radius = b_c.length() / (2.0f * sin_v);
	//或者可以直接从二维几何的角度直接计算,而不是使用上面的代数算法
	float d1 = -dot(c_a, a_b);
	float d2 = -dot(a_b, b_c);
	float d3 = -dot(b_c, c_a);
	float c1 = d2 * d3;
	float c2 = d1 * d3;
	float c3 = d1 * d2;
	float cc = 2.0f * (c1 + c2 + c3);
	Vec3  v = a * (c2 + c3) + b * (c1 + c3) + c * (c1 + c2);
	sphere.center.x = v.x / cc;
	sphere.center.y = v.y / cc;
	sphere.center.z = v.z / cc;
}
//求空间四个点的最小包围球,如果存在退化的点,则也会给出相关的方程式
void  create_sphere4(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &d, Sphere &sphere)
{
	//对顶点重新排序
	const Vec3 ab = b - a;
	const Vec3 ca = a - c;
	const Vec3 ad = d - a;
	//三棱锥的有向体积
	Vec3 normal = cross_normalize(ab, -ca);
	float dot_f = dot(ad, normal);
	float volum = fabs(dot_f);
	//如果是退化的空间四个顶点
	if (volum < 0.001f)
	{
		create_minimum_sphere4_plane(a, b, c, d, sphere);
		return;
	}
	//否则执行标准的外接球算法,不再使用4阶行列式,运算量太大
	//计算外接圆的圆心
	if (dot_f < 0)
	{
		dot_f = -dot_f;
		normal = -normal;
	}
	//下面的代码参见3d数学基础第12张,第6节
	Vec3  c_a = a - c;
	Vec3  a_b = b - a;
	Vec3  b_c = c - b;

	float d1 = -dot(c_a, a_b);
	float d2 = -dot(a_b, b_c);
	float d3 = -dot(b_c, c_a);
	float c1 = d2 * d3;
	float c2 = d1 * d3;
	float c3 = d1 * d2;
	float cc = 2.0f * (c1 + c2 + c3);
	Vec3  v = a * (c2 + c3) + b * (c1 + c3) + c * (c1 + c2);
	Vec3 center(v.x / cc, v.y / cc, v.z / cc);

	float distance = (length2(center - d) - length2(center - a)) / (2.0f * dot(ad, normal));
	sphere.center = center + normal * distance;
	sphere.radius = (sphere.center - a).length();
}

void  create_minimum_sphere4_plane(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &d, Sphere &sphere)
{
	//假设前三个顶点构成了一个三角形
	create_sphere3(a, b, c, sphere);
	//判断最后一个顶点的相对位置
	if (!check_point_insideof_sphere(sphere, d))
	{
		//选取离顶点d最远的两个顶点
		float   length_array[3] = { (a - d).length() ,(b - d).length() ,(c - d).length() };
		const  Vec3  *vertex_array[3] = { &a,&b,&c };

		int  index_l = length_array[0] >= length_array[1] && length_array[0] >= length_array[2] ? 0 : (length_array[1] >= length_array[0] && length_array[1] >= length_array[2] ? 1 : 2);
		int  secondary_l = (index_l + 1) % 3;
		int  tripple_l = (index_l + 2) % 3;

		if (length_array[secondary_l] < length_array[tripple_l])
		{
			int t = secondary_l;
			secondary_l = tripple_l;
			tripple_l = t;
		}
		//再次创建最小包围圆
		create_sphere3(d, *vertex_array[index_l], *vertex_array[secondary_l], sphere);
		//检测是否剩余的一个点在圆的范围之内,分支走到这里的概率已经非常小了
		if (!check_point_insideof_sphere(sphere, *vertex_array[tripple_l]))
			create_sphere3(d, *vertex_array[index_l], *vertex_array[tripple_l], sphere);
	}
}
/*
*求四个点的最小包围球
*此算法比较复杂,因为所牵涉到的情况比较多
*此算法的核心为,四个离散空间点的最小包围球一定落在三棱锥的内部,或者边界上
*/
void  create_minimum_sphere4(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &d, Sphere &sphere)
{
	//对顶点重新排序
	const Vec3 ab = b - a;
	const Vec3 bc = c - b;
	const Vec3 ca = a - c;
	const Vec3 ad = d - a;
	const Vec3 cd = d - c;
	const Vec3 bd = d - b;
	struct TriangleVertex
	{
		const  Vec3  *target_vertex;
		const  Vec3  *bottom_vertex_array[3];
	};
	const struct TriangleVertex  vertex_pair_array[4] = {
		{ &d,{ &b,&c,&a } },//d
		{ &a,{ &c,&b,&d } },//a
		{ &b,{ &a,&c,&d } },//b
		{ &c,{ &d,&a,&b } },//c
	};
	//三棱锥的有向体积
	Vec3 normal = cross_normalize(ab, -ca);
	float  volum = fabs(normal.dot(ad));
	//如果是退化的空间四个顶点
	if (volum < 0.001f)
	{
		create_minimum_sphere4_plane(a, b, c, d, sphere);
		return;
	}
	for (int index_j = 0; index_j < 4; ++index_j)
	{
		const TriangleVertex  &vertex_gather = vertex_pair_array[index_j];
		const Vec3 &target_vertex = *vertex_gather.target_vertex;
		const Vec3 **vertex_array = (const Vec3 **)vertex_gather.bottom_vertex_array;
		//判断是否有一个夹角为钝角,或者是直角
		float dot_f1 = dot(*vertex_array[0] - target_vertex, *vertex_array[1] - target_vertex);
		float dot_f2 = dot(*vertex_array[1] - target_vertex, *vertex_array[2] - target_vertex);
		float dot_f3 = dot(*vertex_array[2] - target_vertex, *vertex_array[0] - target_vertex);
		//计算对角点
		const int v1_index = dot_f1 <= 0 ? 2 : (dot_f2 <= 0 ? 0 : (dot_f3 <= 0 ? 1 : -1));
		const Vec3  *v1 = v1_index != -1 ? vertex_array[v1_index] : nullptr;
		const Vec3  *v2 = v1_index != -1 ? vertex_array[(v1_index + 1) % 3] : nullptr;
		const Vec3  *v3 = v1_index != -1 ? vertex_array[(v1_index + 2) % 3] : nullptr;
		//如果满足了相关的条件,则求出球心坐标,以及半径
		if (v1_index != -1 && dot(*v2 - *v1, *v3 - *v1) <= 0)//此时上三角形为钝角,且下三角形的对角也为钝角/直角
		{
			sphere.center = (*v2 + *v3) * 0.5f;
			sphere.radius = (*v2 - *v3).length() * 0.5f;
			return;
		}
		//否则求下三角形的最小包围圆的圆心与半径
		if (v1_index != -1)
		{
			Cycle3 cycle3;
			create_mininum_cycle3(*v1, *v2, *v3, cycle3);
			//检测圆心与对角三角形的上方顶点的距离
			Vec3  skew_edge = target_vertex - cycle3.center;
			float length_f = skew_edge.length();
			if (length_f <= cycle3.radius)
			{
				sphere.center = cycle3.center;
				sphere.radius = cycle3.radius;
				return;
			}
			//否则需要做一些几何变换,可以断定,求新的坐标一定在下三角形的圆心与上三角形的最高点的连线在下三角形的投影上
			//因此,首先计算连线在下三角形的投影
			Vec3  normal = cross_normalize(*v2 - *v1, *v3 - *v1);//法线有可能是反向的,需要矫正
			if (normal.dot(skew_edge) < 0)
				normal = -normal;
			//计算投影向量
			Vec3  direction = normalize(skew_edge - normal * skew_edge.dot(normal));
			float  d = ((length2(*v1) - length2(target_vertex)) - dot(*v1 - target_vertex, cycle3.center)) / (dot((*v1 - target_vertex) * 2.0f, normal));
			sphere.center = cycle3.center + direction * d;
			sphere.radius = (sphere.center - target_vertex).length();
			return;
		}
	}
	//否则执行标准的外接球算法,不再使用4阶行列式,运算量太大
	create_sphere4(a, b, c, d, sphere);
}

void  compute_minimum_sphere_normal(const std::vector<cocos2d::Vec3> &points, Sphere  &sphere)
{
	sphere.center = (points[0] + points[1]) * 0.5f;
	float d_x = points[0].x - sphere.center.x;
	float d_y = points[0].y - sphere.center.y;
	float d_z = points[0].z - sphere.center.z;
	sphere.radius = sqrtf(d_x * d_x + d_y * d_y + d_z * d_z);
	for (int index_j = 2; index_j < points.size(); ++index_j)
	{
		const Vec3 &point = points.at(index_j);
		d_x = point.x - sphere.center.x;
		d_y = point.y - sphere.center.y;
		d_z = point.z - sphere.center.z;
		float  new_radius = sqrtf(d_x * d_x + d_y * d_y + d_z * d_z);
		if (new_radius > sphere.radius)
		{
			float trunk_radius = (new_radius + sphere.radius) * 0.5f;
			float f = (trunk_radius - sphere.radius) / new_radius;

			sphere.center.x += d_x * f;
			sphere.center.y += d_y * f;
			sphere.center.z += d_z * f;
			sphere.radius = trunk_radius;
		}
	}
}
/*
*判断三维点是否在球内部,或者边界上
*/
bool check_point_insideof_sphere(const Sphere &sphere, const cocos2d::Vec3 &point)
{
	float d_x = point.x - sphere.center.x;
	float d_y = point.y - sphere.center.y;
	float d_z = point.z - sphere.center.z;

	return d_x * d_x + d_y * d_y + d_z * d_z <= sphere.radius * sphere.radius + 0.1f;
}

bool check_point_onsurfaceof_sphere(const Sphere &sphere, const cocos2d::Vec3 &point)
{
	float d_x = point.x - sphere.center.x;
	float d_y = point.y - sphere.center.y;
	float d_z = point.z - sphere.center.z;

	return fabs(d_x * d_x + d_y * d_y + d_z * d_z - sphere.radius * sphere.radius) <= 0.1f;
}

float sphere_plane_minimum_distance(const Sphere &sphere, const Plane &plane)
{
	float d = plane.distanceTo(sphere.center);
	//区分是否在平面的那一侧
	if (fabsf(d) <= sphere.radius)
		return 0;
	if (d > sphere.radius)
		return d - sphere.radius;
	return d + sphere.radius;
}
///////////////////////////Sphere-End////////////////////////////////
NS_GT_END