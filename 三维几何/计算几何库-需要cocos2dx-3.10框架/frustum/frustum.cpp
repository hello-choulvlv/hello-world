/*
  *视锥体裁剪 + 阴影体裁剪算法实现
  *2020年4月17日
  *@author:xiaohuaxiong
 */
#include "frustum.h"
#include "matrix/matrix.h"
#include <math.h>
using namespace cocos2d;
NS_GT_BEGIN
enum PlaneType {
	PlaneType_left = 0,
	PlaneType_Right = 1,
	PlaneType_Low = 2,
	PlaneType_Up = 3,
	PlaneType_Near = 4,
	PlaneType_Far = 5,
};

struct PlaneEdge {
	short start_idx, over_idx;
	PlaneType		left_type,right_type;
};

static void normalize_planes(cocos2d::Vec4 &plane_equation) {
	float d = sqrtf(plane_equation.x * plane_equation.x + plane_equation.y * plane_equation.y + plane_equation.z * plane_equation.z);
	d = d != 0.0f ? 1.0f/d:0.0f;
	plane_equation.x *= d;
	plane_equation.y *= d;
	plane_equation.z *= d;
	plane_equation.w *= d;
}

void Frustum2::initGeometryPlanes(const cocos2d::Mat4 &view_proj_mat) {
	////https://blog.csdn.net/qq_31709249/article/details/80175119
	const float *matrix_array = view_proj_mat.m;
#define check_pos_plane(target_plane,ix){\
	target_plane.x = -(matrix_array[ix] + matrix_array[3]);\
	target_plane.y = -(matrix_array[ix +4] + matrix_array[7]);\
	target_plane.z = -(matrix_array[ix+8] + matrix_array[11]);\
	target_plane.w = -(matrix_array[ix+12] + matrix_array[15]);\
}

#define check_neg_plane(target_plane,ix){\
	target_plane.x = matrix_array[ix] - matrix_array[3];\
	target_plane.y = matrix_array[ix+4] - matrix_array[7];\
	target_plane.z = matrix_array[ix +8] - matrix_array[11];\
	target_plane.w = matrix_array[ix +12] - matrix_array[15];\
	}

	Vec4 &left_plane = _planes[0];//左平面
	Vec4 &right_plane = _planes[1];//
	check_pos_plane(left_plane,0);
	check_neg_plane(right_plane,0);
	normalize_planes(left_plane);
	normalize_planes(right_plane);

	Vec4 &low_plane = _planes[2];
	Vec4 &up_plane = _planes[3];
	check_pos_plane(low_plane,1);
	check_neg_plane(up_plane,1);
	normalize_planes(low_plane);
	normalize_planes(up_plane);

	Vec4 &far_plane = _planes[4];
	Vec4 &near_plane = _planes[5];
	check_pos_plane(far_plane,2);
	check_neg_plane(near_plane,2);
	normalize_planes(far_plane);
	normalize_planes(near_plane);

#undef  check_neg_plane
#undef check_pos_plane
}

void Frustum2::initShadowPlanes(const cocos2d::Vec3 frustum_vertex[8], const cocos2d::Vec3 &light_direction) {
	//首先需要对视锥体的各个边/平面作上编号.从平面外侧的法线观察平面,其边的走向始终时逆时针的
	PlaneEdge  edge_array[12] = {
		{0,1,PlaneType_Near ,PlaneType_Low},//0,棱的左侧平面/右侧平面
		{1,2,PlaneType_Near,PlaneType_Right},//1
		{2,3,PlaneType_Near,PlaneType_Up},//2
		{3,0,PlaneType_Near,PlaneType_left},//3

		{5,4,PlaneType_Far,PlaneType_Low},//4
		{4,7,PlaneType_Far,PlaneType_left},//5
		{7,6,PlaneType_Far,PlaneType_Up},//6
		{6,5,PlaneType_left,PlaneType_Right},//7

		{7,3,PlaneType_Up,PlaneType_left},//8
		{2,6,PlaneType_Up,PlaneType_Right},//9
		{5,1,PlaneType_Low,PlaneType_Right},//10
		{0,4,PlaneType_Low,PlaneType_left},//11
	};
	//各个平面的法线
	const Vec3 normal_array[6] = {
		cross_normalize(frustum_vertex[0],frustum_vertex[7],frustum_vertex[4]),//0 left
		cross_normalize(frustum_vertex[1],frustum_vertex[5],frustum_vertex[2]),//1 right
		cross_normalize(frustum_vertex[0],frustum_vertex[4],frustum_vertex[5]),//2 low
		cross_normalize(frustum_vertex[3],frustum_vertex[6],frustum_vertex[7]),//3 up
		cross_normalize(frustum_vertex[0],frustum_vertex[1],frustum_vertex[3]),//4 near
		cross_normalize(frustum_vertex[5],frustum_vertex[4],frustum_vertex[6]),//5 far
	};
	//针对所有的棱进行检测
	_shadowNum = 0;
	for (int j = 0; j < 12; ++j) {
		const PlaneEdge  &edge = edge_array[j];
		float f1 = dot(light_direction,normal_array[edge.left_type]);
		float f2 = dot(light_direction,normal_array[edge.right_type]);
		//此时,视锥体平面一个平面朝向光线,另一个背离光线,这种情况下才可以形成新的裁剪平面
		if (f1 * f2 < 0.0f) {
			//针对f1/f2的符号不同,将使用不同的算法
			Vec4 &shadow_plane = _shaodwPlanes[_shadowNum++];
			Vec3 &normal = *(Vec3*)&shadow_plane;
			if (f1 > 0.0f) 
				normal = cross(frustum_vertex[edge.over_idx] - frustum_vertex[edge.start_idx],light_direction);
			else 
				normal = cross(light_direction,frustum_vertex[edge.over_idx] - frustum_vertex[edge.start_idx]);
			shadow_plane.w = -dot(normal, frustum_vertex[edge.start_idx]);
			normalize_planes(shadow_plane);
		}
	}
	//另外,有些平面始终是背离光线的,此时这些平面也可以作为裁剪平面出现
	short vertex_idx_array[6] = {4,5,4,6,0,7};
	for (int j = 0; j < 6; ++j) {
		float f = dot(normal_array[j],light_direction);
		if (f < 0.0f){
			Vec4 &shadow_plane = _shaodwPlanes[_shadowNum++];
			Vec3 &normal = *(Vec3*)&shadow_plane;
			normal = normal_array[j];
			shadow_plane.w = -dot(normal, frustum_vertex[vertex_idx_array[j]]);
			normalize_planes(shadow_plane);
		}
	}
	assert(_shadowNum <= 12);
}

bool Frustum2::isLocateInFrustum(const AABB &aabb)const {
	Vec3 point;
	for (int j = 0; j < 6; ++j) {
		const Vec4 &plane = _planes[j];
		point.x = plane.x > 0.0f ? aabb.bb_min.x:aabb.bb_max.x;
		point.y = plane.y > 0.0f ? aabb.bb_min.y:aabb.bb_max.y;
		point.z = plane.z > 0.0f ? aabb.bb_min.z:aabb.bb_max.z;
		if (dot(*(Vec3*)&plane, point) + plane.w > 0.0f)
			return false;
	}
	return true;
}

bool Frustum2::isLocateInFrustum(const OBB &obb)const {
	Vec3 x_axis = obb.xaxis * obb.extent.x, y_axis = obb.yaxis * obb.extent.y, z_axis = obb.zaxis * obb.extent.z;
	for (int j = 0; j < 6; ++j) {
		const Vec4 &plane = _planes[j];
		Vec3 point = obb.center;
		point += dot(*(Vec3*)&plane,obb.xaxis) > 0.0f?-x_axis : x_axis;
		point += dot(*(Vec3*)&plane,obb.yaxis) > 0.0f?-y_axis : y_axis;
		point += dot(*(Vec3*)&plane,obb.zaxis) > 0.0f?-z_axis : z_axis;

		if (dot(*(Vec3*)&plane, point) + plane.w > 0.0f)
			return false;
	}
	return true;
}

bool Frustum2::isLocateInShadowVolumn(const AABB &aabb)const {
	Vec3 point;
	for (int j = 0; j < _shadowNum; ++j) {
		const Vec4 &plane = _shaodwPlanes[j];
		point.x = plane.x > 0.0f ? aabb.bb_min.x : aabb.bb_max.x;
		point.y = plane.y > 0.0f ? aabb.bb_min.y : aabb.bb_max.y;
		point.z = plane.z > 0.0f ? aabb.bb_min.z : aabb.bb_max.z;
		if (dot(*(Vec3*)&plane, point) + plane.w > 0.0f)
			return false;
	}
	return true;
}

bool Frustum2::isLocateInShadowVolumn(const OBB &obb)const {
	Vec3 x_axis = obb.xaxis * obb.extent.x, y_axis = obb.yaxis * obb.extent.y, z_axis = obb.zaxis * obb.extent.z;
	for (int j = 0; j < _shadowNum; ++j) {
		const Vec4 &plane = _shaodwPlanes[j];
		Vec3 point = obb.center;
		point += dot(*(Vec3*)&plane, obb.xaxis) > 0.0f ? -x_axis : x_axis;
		point += dot(*(Vec3*)&plane, obb.yaxis) > 0.0f ? -y_axis : y_axis;
		point += dot(*(Vec3*)&plane, obb.zaxis) > 0.0f ? -z_axis : z_axis;

		if (dot(*(Vec3*)&plane, point) + plane.w > 0.0f)
			return false;
	}
	return true;
}

NS_GT_END