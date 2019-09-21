/*
  *aabb-obb相交测试算法实现
  *2019/6/16
  *@author:xiaohuaxiong
  */
#include "aabb_obb.h"
#include "line/line.h"
#include "cycle_sphere/cycle_sphere.h"
#include "triangle/triangle.h"
using namespace cocos2d;
NS_GT_BEGIN

void aabb_create(AABB &aabb, const cocos2d::Vec3 &min_bb, const cocos2d::Vec3 &max_bb)
{
	aabb.bb_min = min_bb;
	aabb.bb_max = max_bb;
}

void obb_create(OBB &obb, const AABB &aabb)
{
	obb.center = (aabb.bb_max + aabb.bb_min) * 0.5f;
	obb.extent = (aabb.bb_max - aabb.bb_min) * 0.5f;

	obb.xaxis.x = 1.0f, obb.xaxis.y = 0, obb.xaxis.z = 0;
	obb.yaxis.x = 0, obb.yaxis.y = 1, obb.yaxis.z = 0;
	obb.zaxis.x = 0, obb.zaxis.y = 0, obb.zaxis.z = 1;
}

void obb_create(OBB  &obb, const AABB &aabb, const mat3x3 &mat)
{
	obb.center = (aabb.bb_max + aabb.bb_min) * 0.5f;
	obb.extent = (aabb.bb_max - aabb.bb_min) * 0.5f;

	obb.xaxis.x = mat.m[0], obb.xaxis.y = mat.m[1], obb.xaxis.z = mat.m[2];
	obb.yaxis.x = mat.m[3], obb.yaxis.y = mat.m[4], obb.yaxis.z = mat.m[5];
	obb.zaxis.x = mat.m[6], obb.zaxis.y = mat.m[7], obb.zaxis.z = mat.m[8];
}
void obb_create(OBB  &obb, const AABB &aabb, const cocos2d::Vec3 &rotate_axis, float angle)
{
	mat3x3 mat;
	mat3_create_rotate(mat, rotate_axis, angle);

	obb.center = (aabb.bb_max + aabb.bb_min) * 0.5f;
	obb.extent = (aabb.bb_max - aabb.bb_min) * 0.5f;

	obb.xaxis.x = mat.m[0], obb.xaxis.y = mat.m[1], obb.xaxis.z = mat.m[2];
	obb.yaxis.x = mat.m[3], obb.yaxis.y = mat.m[4], obb.yaxis.z = mat.m[5];
	obb.zaxis.x = mat.m[6], obb.zaxis.y = mat.m[7], obb.zaxis.z = mat.m[8];
}

void obb_create(OBB &obb, const cocos2d::Vec3 &min_bb, const cocos2d::Vec3 &max_bb)
{
	obb.center = (max_bb + min_bb) * 0.5f;
	obb.extent = (max_bb - min_bb) * 0.5f;

	obb.xaxis.x = 1.0f, obb.xaxis.y = 0, obb.xaxis.z = 0;
	obb.yaxis.x = 0, obb.yaxis.y = 1, obb.yaxis.z = 0;
	obb.zaxis.x = 0, obb.zaxis.y = 0, obb.zaxis.z = 1;
}

void obb_create(OBB  &obb, const cocos2d::Vec3 &min_bb, const cocos2d::Vec3 &max_bb, const mat3x3 &mat)
{
	obb.center = (max_bb + min_bb) * 0.5f;
	obb.extent = (max_bb - min_bb) * 0.5f;

	obb.xaxis.x = mat.m[0], obb.xaxis.y = mat.m[1], obb.xaxis.z = mat.m[2];
	obb.yaxis.x = mat.m[3], obb.yaxis.y = mat.m[4], obb.yaxis.z = mat.m[5];
	obb.zaxis.x = mat.m[6], obb.zaxis.y = mat.m[7], obb.zaxis.z = mat.m[8];
}

void obb_create(OBB  &obb, const cocos2d::Vec3 &min_bb, const cocos2d::Vec3 &max_bb, const cocos2d::Vec3 &rotate_axis, float angle)
{
	mat3x3 mat;
	mat3_create_rotate(mat, rotate_axis, angle);

	obb.center = (max_bb + min_bb) * 0.5f;
	obb.extent = (max_bb - min_bb) * 0.5f;

	obb.xaxis.x = mat.m[0], obb.xaxis.y = mat.m[1], obb.xaxis.z = mat.m[2];
	obb.yaxis.x = mat.m[3], obb.yaxis.y = mat.m[4], obb.yaxis.z = mat.m[5];
	obb.zaxis.x = mat.m[6], obb.zaxis.y = mat.m[7], obb.zaxis.z = mat.m[8];
}

bool aabb_aabb_intersect_test(const AABB &a, const AABB &b)
{
	if (a.bb_max.x < b.bb_min.x || a.bb_min.x > b.bb_max.x)
		return false;
	if (a.bb_max.y < b.bb_min.y || a.bb_min.y > b.bb_max.y)
		return false;
	if (a.bb_max.z < b.bb_min.z || a.bb_min.z > b.bb_max.z)
		return false;
	return true;
}

//将b空间的坐标系变换到a中,b x a
void transform_obb_matrix(mat3x3 &mat, const OBB &a, const OBB &b)
{
	mat.m[0] = b.xaxis.x * a.xaxis.x + b.xaxis.y * a.xaxis.y+ b.xaxis.z * a.xaxis.z;
	mat.m[1] = b.xaxis.x * a.yaxis.x+ b.xaxis.y * a.yaxis.y + b.xaxis.z * a.yaxis.z;
	mat.m[2] = b.xaxis.x * a.zaxis.x + b.xaxis.y * a.zaxis.y + b.xaxis.z * a.zaxis.z;

	mat.m[3] = b.yaxis.x * a.xaxis.x + b.yaxis.y * a.xaxis.y + b.yaxis.z * a.xaxis.z;
	mat.m[4] = b.yaxis.x * a.yaxis.x + b.yaxis.y * a.yaxis.y + b.yaxis.z * a.yaxis.z;
	mat.m[5] = b.yaxis.x * a.zaxis.x + b.yaxis.y * a.zaxis.y + b.yaxis.z * a.zaxis.z;

	mat.m[6] = b.zaxis.x * a.xaxis.x + b.zaxis.y * a.xaxis.y + b.zaxis.z * a.xaxis.z;
	mat.m[7] = b.zaxis.x * a.yaxis.x + b.zaxis.y * a.yaxis.y + b.zaxis.z * a.yaxis.z;
	mat.m[8] = b.zaxis.x * a.zaxis.x + b.zaxis.y * a.zaxis.y + b.zaxis.z * a.zaxis.z;
}

void  transform_obb_vec3(const OBB &src,const Vec3 &v,Vec3 &dst)
{
	float x = src.xaxis.x * v.x + src.xaxis.y * v.y + src.xaxis.z * v.z;
	float y = src.yaxis.x * v.x + src.yaxis.y * v.y + src.yaxis.z * v.z;
	float z = src.zaxis.x * v.x + src.zaxis.y * v.y + src.zaxis.z * v.z;

	dst.x = x; dst.y = y; dst.z = z;
}

bool obb_obb_intersect_test(const OBB &a, const OBB &b)
{
	mat3x3  mat,abs_mat;
	transform_obb_matrix(mat, a, b);
	//获取绝对值矩阵
	for (int index_l = 0; index_l < 9; ++index_l)
		abs_mat.m[index_l] = fabs(mat.m[index_l]);
	Vec3  t = b.center - a.center,rotate_vec3;
	transform_obb_vec3(a, t,t);
	vec3_transfrom_mat3(b.extent, abs_mat, rotate_vec3);
	//分别对OBB的六个轴进行分离轴测试
	const float *vec_array = &t.x,*v1 = &a.extent.x,*v2 = &rotate_vec3.x;
	for (int index_j = 0; index_j < 3; ++index_j)
	{
		if (fabs(vec_array[index_j]) > v1[index_j] + v2[index_j])
			return false;
	}
	//变换到b空间下
	Vec3  c;
	const float *c_array = &c.x;
	const float *v3 = &b.extent.x;
	mat3_transform_vec3(mat, t, c);
	mat3_transform_vec3(abs_mat, a.extent, rotate_vec3);
	for (int index_j = 0; index_j < 3; ++index_j)
	{
		if (fabs(c_array[index_j]) > v3[index_j] + v2[index_j])
			return false;
	}
	//剩下的9个分离轴测试,代码为优化过后的
	const float eps = 0.0001f;
	//L = a.xaxis x b.xaxis
	float  ra = a.extent.y * abs_mat.m[2] + a.extent.z * abs_mat.m[1];
	float  rb = b.extent.y * abs_mat.m[6] + b.extent.z * abs_mat.m[3];
	if (ra > eps && fabs(t.z * mat.m[1] - t.y * mat.m[2]) > ra + rb)
		return false;
	//L = Ax x By
	ra = a.extent.y * abs_mat.m[5] + a.extent.z * abs_mat.m[4];
	rb = b.extent.x * abs_mat.m[6] +  b.extent.z * abs_mat.m[0];
	if (ra > eps && fabs(t.z * mat.m[4] - t.y * mat.m[5]) > ra + rb)
		return false;
	//L = Ax x Bz
	ra = a.extent.y * abs_mat.m[8] + a.extent.z * abs_mat.m[7];
	rb = b.extent.x * abs_mat.m[3] + b.extent.y * abs_mat.m[0];
	if (ra > eps && fabs(t.z * mat.m[7] - t.y * mat.m[8]) > ra + rb)
		return false;
	//L = Ay x Bx
	ra = a.extent.x * abs_mat.m[2] + a.extent.z * abs_mat.m[0];
	rb = b.extent.y * abs_mat.m[7] + b.extent.z * abs_mat.m[4];
	if (ra > eps && fabs(t.x * mat.m[2] - t.z * mat.m[0]) > ra + rb)
		return false;
	//L = Ay x By
	ra = a.extent.x * abs_mat.m[5] + a.extent.z * abs_mat.m[3];
	rb = b.extent.x * abs_mat.m[7] + b.extent.z * abs_mat.m[1];
	if (ra > eps && fabs(t.x * mat.m[5] - t.z * mat.m[3]) > ra + rb)
		return false;
	//L = Ay x Bz
	ra = a.extent.x * abs_mat.m[8] + a.extent.z * abs_mat.m[6];
	rb = b.extent.x * abs_mat.m[4] + b.extent.y * abs_mat.m[1];
	if (ra > eps && fabs(t.x * mat.m[8] - t.z * mat.m[6]) > ra + rb)
		return false;
	//L = Az x Bx
	ra = a.extent.x * abs_mat.m[1] + a.extent.y * abs_mat.m[0];
	rb = b.extent.y * abs_mat.m[8] + b.extent.z * abs_mat.m[5];
	if (ra > eps && fabs(-t.x * mat.m[1] + t.y * mat.m[0]) > ra + rb)
		return false;
	//L = Az x By
	ra = a.extent.x * abs_mat.m[4] + a.extent.y * abs_mat.m[3];
	rb = b.extent.x * abs_mat.m[8] + b.extent.z * abs_mat.m[2];
	if (ra > eps && fabs(-t.x * mat.m[4] + t.y * mat.m[3]) > ra + rb)
		return false;
	//L = Az x Bz
	ra = a.extent.x * abs_mat.m[7] + a.extent.y * abs_mat.m[6];
	rb = b.extent.x * abs_mat.m[5] + b.extent.y * abs_mat.m[2];
	if (ra > eps && fabs(-t.x * mat.m[7] + t.y * mat.m[6]) > ra + rb)
		return false;
	return true;
}

void  obb_create_obb_vertex8(const OBB &obb, cocos2d::Vec3 *vertex)
{
	const Vec3 x_vec = obb.xaxis * obb.extent.x;
	const Vec3 y_vec = obb.yaxis * obb.extent.y;
	const Vec3 z_vec = obb.zaxis * obb.extent.z;

	vertex[0] = obb.center - x_vec - y_vec - z_vec;
	vertex[1] = obb.center + x_vec - y_vec - z_vec;
	vertex[2] = obb.center + x_vec + y_vec - z_vec;
	vertex[3] = obb.center - x_vec + y_vec - z_vec;

	vertex[4] = obb.center - x_vec - y_vec + z_vec;
	vertex[5] = obb.center + x_vec - y_vec + z_vec;
	vertex[6] = obb.center + x_vec + y_vec + z_vec;
	vertex[7] = obb.center - x_vec + y_vec + z_vec;
}

float  aabb_point_minimum_distance(const AABB &aabb,const cocos2d::Vec3 &point,Vec3 &intersect_point)
{
	intersect_point.x = clampf(aabb.bb_min.x, aabb.bb_max.x, point.x);
	intersect_point.y = clampf(aabb.bb_min.y,aabb.bb_max.y,point.y);
	intersect_point.z = clampf(aabb.bb_min.z,aabb.bb_max.z,point.z);

	return (point - intersect_point).length();
}

float obb_point_minimum_distance(const OBB &obb,const cocos2d::Vec3 &point, cocos2d::Vec3 &intersect_point)
{
	Vec3 local_point = point - obb.center;
	float x = clampf(-obb.extent.x,obb.extent.x,gt::dot(local_point,obb.xaxis));
	float y = clampf(-obb.extent.y,obb.extent.y,gt::dot(local_point,obb.yaxis));
	float z = clampf(-obb.extent.z,obb.extent.z,gt::dot(local_point,obb.zaxis));
	//变换到世界空间中
	intersect_point.x = x * obb.xaxis.x + y * obb.yaxis.x + z * obb.zaxis.x + obb.center.x;
	intersect_point.y = x * obb.xaxis.y + y * obb.yaxis.y + z * obb.zaxis.y + obb.center.y;
	intersect_point.z = x * obb.xaxis.z + y * obb.yaxis.z + z * obb.zaxis.z + obb.center.z;

	float d_x = local_point.x - x;
	float d_y = local_point.y - y;
	float d_z = local_point.z - z;
	return sqrtf(d_x * d_x + d_y * d_y + d_z * d_z);
}

bool aabb_plane_intersect_test(const AABB &aabb, const Plane &plane)
{
	const Vec3 center = (aabb.bb_max + aabb.bb_min) * 0.5f;
	const Vec3 extent = aabb.bb_max - center;

	float r = extent.x * fabsf(plane.normal.x) + extent.y * fabsf(plane.normal.y) + extent.z * fabsf(plane.normal.z);
	return fabsf(plane.normal.dot(center) - plane.distance) <= r;
}

bool obb_plane_intersect_test(const OBB &obb, const Plane &plane)
{
	const Vec3 &normal = plane.normal;
	float r = obb.extent.x * fabsf(normal.dot(obb.xaxis)) + obb.extent.y * fabsf(normal.dot(obb.yaxis)) + obb.extent.z * fabsf(normal.dot(obb.zaxis));

	return fabsf(normal.dot(obb.center) - plane.distance) <= r;
}

bool aabb_sphere_intersect_test(const AABB &aabb, const Sphere &sphere)
{
	//AABB与球体的相交测试不能采用分离面测试算法,因为该测试是非充分的
	//计算最近点
	float x = clampf(aabb.bb_min.x,aabb.bb_max.x,sphere.center.x);
	float y = clampf(aabb.bb_min.y,aabb.bb_max.y,sphere.center.y);
	float z = clampf(aabb.bb_min.z,aabb.bb_max.z,sphere.center.z);

	float dx = sphere.center.x - x;
	float dy = sphere.center.y - y;
	float dz = sphere.center.z - z;
	return dx * dx + dy * dy + dz *dz <= sphere.radius * sphere.radius;
}

bool obb_sphere_intersect_test(const OBB &obb, const Sphere &sphere)
{
	const Vec3 direction = sphere.center - obb.center;
	float x = clampf(-obb.extent.x,obb.extent.x,dot(direction,obb.xaxis));
	float y = clampf(-obb.extent.y,obb.extent.y,dot(direction,obb.yaxis));
	float z = clampf(-obb.extent.z,obb.extent.z,dot(direction,obb.zaxis));

	float l = sqrtf(x*x + y*y +z*z);

	return  direction.length() - l <= sphere.radius;
}

bool aabb_triangle_intersect_test(const AABB &aabb, const Triangle &triangle)
{
	//计算分离面
	Vec3 ab = triangle.b - triangle.a;
	Vec3 ca = triangle.a - triangle.c;
	Vec3 normal = cross_normalize(ca, ab);

	Vec3 center = (aabb.bb_max + aabb.bb_min) * 0.5f;
	Vec3 extent = aabb.bb_max - center;
	//此时AABB完全在分离面的一侧,因此必定不相交
	if (fabsf(normal.dot(center - triangle.a)) > extent.x * fabsf(normal.x) + extent.y * fabsf(normal.y) + extent.z * fabsf(normal.z))
		return false;
	//以AABB的中心点为原点,进行分离轴测试.一共是9个
	Vec3  na = triangle.a - center;
	Vec3  nb = triangle.b - center;
	Vec3  nc = triangle.c - center;
	Vec3 bc = nc - nb;
	//Vx x ab==>(0,-ab.z,ab.y)
#define vx_compare_func(n1,n2,n,extent){\
	float r =extent.y * fabsf(n.z) + extent.z * fabsf(n.y); \
	float l = -n1.y * n.z + n1.z * n.y;\
	float f = -n2.y * n.z + n2.z * n.y;\
	if(fabsf(r) >= 0.001f && fmaxf(-fmaxf(l,f),fminf(l,f)) > r)\
		return false;\
}
	vx_compare_func(na, nc, ab, extent);
	vx_compare_func(nb, na, bc, extent);
	vx_compare_func(nc, nb, ca, extent);
#undef vx_compare_func
	//Vy x ab==>(z,0,-x)
#define vy_compare_func(n1,n2,n,extent){\
	float r = extent.x * fabsf(n.z) + extent.z * fabsf(n.x);\
	float l = n1.x * n.z - n1.z * n.x;\
	float f = n2.x * n.z - n2.z * n.x;\
	if(fabsf(r) >= 0.001f && fmaxf(-fmaxf(l,f),fminf(l,f)) > r)\
		return false;\
}
	vy_compare_func(na, nc, ab, extent);
	vy_compare_func(nb, na, bc, extent);
	vy_compare_func(nc, nb, ca, extent);
#undef vy_compare_func
	//Vz x ab==>(-y,x,0)
#define vz_compare_func(n1,n2,n,extent){\
	float r = extent.x * fabsf(n.y) + extent.y * fabsf(n.x);\
	float l = -n1.x * n.y + n1.y * n.x;\
	float f = -n2.x * n.y + n2.y * n.x;\
	if(fabsf(r) >= 0.001f && fmaxf(-fmaxf(l,f),fminf(l,f)) >r)\
	return false;\
}
	vz_compare_func(na, nc, ab, extent);
	vz_compare_func(nb, na, bc, extent);
	vz_compare_func(nc, nb, ca, extent);
#undef vz_compare_func
	//沿着AABB的三个面法线的分离轴做测试
	//VX
	float l = fminf(na.x,fminf(nb.x,nc.x));
	float f = fmaxf(na.x,fmaxf(nb.x,nc.x));
	if (fmaxf(-fmaxf(l, f), fminf(l, f)) > extent.x)
		return false;
	//Vy
	l = fminf(na.y,fminf(nb.y,nc.y));
	f = fmaxf(na.y,fmaxf(nb.y,nc.y));
	if (fmaxf(-fmaxf(l, f), fminf(l, f)) > extent.y)
		return false;
	//VZ
	l = fminf(na.z, fminf(nb.z, nc.z));
	f = fmaxf(na.z, fmaxf(nb.z, nc.z));
	if (fmaxf(-fmaxf(l, f), fminf(l, f)) > extent.z)
		return false;
	return true;
}

bool obb_triangle_intersect_test(const OBB &obb, const Triangle &triangle)
{
	//首先判断OBB与三角形所在的平面的相交测试
	Vec3 ab = triangle.b - triangle.a;
	Vec3 bc = triangle.c - triangle.b;
	Vec3 normal = cross_normalize(ab,bc);
	//以平面的法线作为分离轴
	const Vec3 &extent = obb.extent;
	if (dot(normal, obb.center - triangle.b) > extent.x * fabsf(dot(normal, obb.xaxis)) + extent.y * fabsf(dot(normal, obb.yaxis)) + extent.z * fabsf(dot(normal, obb.zaxis)))
		return false;
	//测试9个分离轴
	const Vec3 na = triangle.a - obb.center;
	const Vec3 nb = triangle.b - obb.center;
	const Vec3 nc = triangle.c - obb.center;
	const Vec3 ca = na - nc;
#define vxyz_compare_func(n1,n2,n,axis,extent){\
	const Vec3 normal_l = cross_normalize(axis,n);\
	if(length2(normal_l) > 0)\
	{\
		float r = dot_abs(extent,normal_l);\
		float l = dot(n1,normal_l);\
		float f = dot(n2,normal_l);\
		if(fmaxf(-fmaxf(l,f),fminf(l,f)) >r)\
			return false;\
	}\
}
	//Vx x ab
	vxyz_compare_func(na, nc, ab, obb.xaxis, extent);
	//Vx x bc
	vxyz_compare_func(nb, na, bc, obb.xaxis, extent);
	//Vx x ca
	vxyz_compare_func(nc, nb, ca, obb.xaxis, extent);

	//Vy x ab
	vxyz_compare_func(na, nc, ab, obb.yaxis, extent);
	//Vy x bc
	vxyz_compare_func(nb, na, bc, obb.yaxis, extent);
	//Vy x ca
	vxyz_compare_func(nc, nb, ca, obb.yaxis, extent);

	//Vz x ab
	vxyz_compare_func(na, nc, ab, obb.zaxis, extent);
	//Vz x bc
	vxyz_compare_func(nb, na, bc, obb.zaxis, extent);
	//Vz x ca
	vxyz_compare_func(nc, nb, ca, obb.zaxis, extent);
#undef vxyz_compare_func
	//测试三个OBB标准分离面
	//将na,nb,nc变换到OBB的空间内
	const Vec3 ta = {na.dot(obb.xaxis) , na.dot(obb.yaxis) , na.dot(obb.zaxis)};
	const Vec3 tb = { nb.dot(obb.xaxis) , nb.dot(obb.yaxis) , nb.dot(obb.zaxis) };
	const Vec3 tc = { nc.dot(obb.xaxis) , nc.dot(obb.yaxis) , nc.dot(obb.zaxis) };

	float l = fminf(ta.x,fminf(tb.x,tc.x));
	float f = fmaxf(ta.x,fmaxf(tb.x,tc.x));
	if (fmaxf(-fmaxf(l, f), fminf(l, f)) > extent.x)
		return false;

	l = fminf(ta.y, fminf(tb.y, tc.y));
	f = fmaxf(ta.y, fmaxf(tb.y, tc.y));
	if (fmaxf(-fmaxf(l, f), fminf(l, f)) > extent.y)
		return false;

	l = fminf(ta.z, fminf(tb.z, tc.z));
	f = fmaxf(ta.z, fmaxf(tb.z, tc.z));
	if (fmaxf(-fmaxf(l, f), fminf(l, f)) > extent.z)
		return false;

	return true;
}

bool aabb_segment_intersect_test(const AABB &aabb, const Segment &segment)
{
	//对AABB的表达方式进行变形
	const Vec3 center = (aabb.bb_min + aabb.bb_max) * 0.5f;
	const Vec3 extent = aabb.bb_max - center;
	//求出直线的另一种表达方式
	const Vec3 seg_center = (segment.start_point + segment.final_point) * 0.5f;
	const Vec3 direction = seg_center - segment.start_point;
	//对三个标准分离轴进行测试
	const Vec3 d = seg_center - center;
	//x
	if (fabsf(d.x) > extent.x + fabsf(direction.x))
		return false;
	//y
	if (fabsf(d.y) > extent.y + fabsf(direction.y))
		return false;
	//z
	if (fabsf(d.z) > extent.z + fabsf(direction.z))
		return false;
	//另外三个分离轴测试
	//Vx x direction ==>(0,-z,y)
	float r = fabsf(-d.y * direction.z + d.z * direction.y);
	float l = extent.y * fabsf(direction.z) + extent.z * fabsf(direction.y);
	if (r > 0 && r > l)
		return false;
	//Vy x direction=>(z,0,-x)
	r = fabsf(d.x * direction.z - d.z * direction.x);
	l = extent.x * fabsf(direction.z) + extent.z * fabsf(direction.x);
	if (r > 0 && r > l)
		return false;
	//Vz x direction=>(-y,x,0)
	r = fabsf(-d.x * direction.y + d.y * direction.x);
	l = extent.x * fabsf(direction.y) + extent.y * fabsf(direction.x);
	if (r > 0 && r > l)
		return false;
	return true;
}

bool obb_segment_intersect_test(const OBB &obb, const Segment &segment)
{
	//变换到OBB的局部坐标空间
	Vec3 center = (segment.final_point + segment.start_point) * 0.5f;
	Vec3 direction = segment.final_point - center;

	center -= obb.center;
	const Vec3 departure = Vec3(dot(center, obb.xaxis), dot(center, obb.yaxis), dot(center, obb.zaxis));
	direction = Vec3(dot(direction, obb.xaxis), dot(direction, obb.yaxis), dot(direction, obb.zaxis));
	//x
	if (fabsf(departure.x) > obb.extent.x + fabsf(direction.x))
		return false;
	//y
	if (fabsf(departure.y) > obb.extent.y + fabsf(direction.y))
		return false;
	//z
	if (fabsf(departure.z) > obb.extent.z + fabsf(direction.z))
		return false;
	//Vx x direction ==>(0,-z,y)
	float r = fabsf(-departure.y * direction.z + departure.z * direction.y);
	float l = obb.extent.y * fabsf(direction.z) + obb.extent.z * fabsf(direction.y);
	if (r > 0 && r > l)
		return false;
	//Vy x direction=>(z,0,-x)
	r = fabsf(departure.x * direction.z - departure.z * direction.x);
	l = obb.extent.x * fabsf(direction.z) + obb.extent.z * fabsf(direction.x);
	if (r > 0 && r > l)
		return false;
	//Vz x direction=>(-y,x,0)
	r = fabsf(-departure.x * direction.y + departure.y * direction.x);
	l = obb.extent.x * fabsf(direction.y) + obb.extent.y * fabsf(direction.x);
	if (r > 0 && r > l)
		return false;
	return true;
}
NS_GT_END