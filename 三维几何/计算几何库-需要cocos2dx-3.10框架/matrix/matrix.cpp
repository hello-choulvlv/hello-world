/*
  *基本几何函数库
  */
#include "matrix.h"
#include<math.h>
using namespace cocos2d;
NS_GT_BEGIN
//////////////////////////////base-begin////////////////////
float dot(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float dot(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b)
{
	return a.x * b.x + a.y * b.y;
}

float dot(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b,const cocos2d::Vec2 &c)
{
	return (b.x - a.x) * (c.x - a.x) + (b.y - a.y) * (c.y - a.y);
}

float  dot(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c)
{
	return (b.x - a.x) * (c.x - a.x) + (b.y - a.y) * (c.y - a.y) + (b.z - a.z) * (c.z - a.z);
}

float dot_abs(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b)
{
	return fabsf(a.x * b.x) + fabsf(a.y * b.y) + fabsf(a.z * b.z);
}

float length2(const cocos2d::Vec3 &a)
{
	return a.x * a.x + a.y * a.y + a.z * a.z;
}

float length2(const Vec2 &a)
{
	return a.x * a.x + a.y * a.y;
}

float  length2(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b)
{
	float d_x = b.x - a.x;
	float d_y = b.y - a.y;
	return d_x * d_x + d_y * d_y;
}

float  length2(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b)
{
	float d_x = b.x - a.x;
	float d_y = b.y - a.y;
	float d_z = b.z - a.z;
	return d_x * d_x + d_y * d_y + d_z * d_z;
}

float length(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b)
{
	float d_x = b.x - a.x;
	float d_y = b.y - a.y;
	return sqrtf(d_x * d_x + d_y * d_y);
}

float length(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b)
{
	float d_x = b.x - a.x;
	float d_y = b.y - a.y;
	float d_z = b.z - a.z;
	return sqrtf(d_x * d_x + d_y * d_y + d_z * d_z);
}

float length(const cocos2d::Vec2 &a)
{
	return sqrtf(a.x * a.x + a.y * a.y);
}

float length(const cocos2d::Vec3 &a)
{
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

cocos2d::Vec3 cross(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b)
{
	return Vec3(a.y * b.z - a.z * b.y, -a.x *b.z + a.z * b.x, a.x * b.y - a.y * b.x);
}

cocos2d::Vec3  cross(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c)
{
	float a_x = b.x - a.x;
	float a_y = b.y - a.y;
	float a_z = b.z - a.z;

	float b_x = c.x - a.x;
	float b_y = c.y - a.y;
	float b_z = c.z - a.z;

	return Vec3(a_y * b_z - a_z * b_y, -a_x *b_z + a_z * b_x, a_x * b_y - a_y * b_x);
}

cocos2d::Vec3 cross_normalize(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b)
{
	float x = a.y * b.z - a.z * b.y;
	float y = -a.x *b.z + a.z * b.x;
	float z = a.x * b.y - a.y * b.x;

	float d = sqrtf(x*x + y*y + z*z);
	d = d != 0 ? 1.0f / d : 0.0f;
	return Vec3(x*d, y*d, z*d);
}

cocos2d::Vec3  cross_normalize(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c)
{
	float a_x = b.x - a.x;
	float a_y = b.y - a.y;
	float a_z = b.z - a.z;

	float b_x = c.x - a.x;
	float b_y = c.y - a.y;
	float b_z = c.z - a.z;

	float x = a_y * b_z - a_z * b_y;
	float y = -a_x *b_z + a_z * b_x;
	float z = a_x * b_y - a_y * b_x;

	float d = sqrtf(x*x + y*y + z*z);
	d = d != 0 ? 1.0f / d : 0.0f;
	return Vec3(x*d, y*d, z*d);
}

cocos2d::Vec3 normalize(const cocos2d::Vec3 &v)
{
	float d = v.x * v.x + v.y * v.y + v.z*v.z;
	d = d != 0 ? sqrtf(1.0f / d) : 0.0f;

	return Vec3(v.x*d, v.y*d, v.z*d);
}

cocos2d::Vec3 normalize(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b) {
	float x = b.x - a.x;
	float y = b.y - a.y;
	float z = b.z - a.z;

	float d = x * x + y * y + z*z;
	d = d != 0 ? sqrtf(1.0f / d) : 0.0f;

	return Vec3(x*d, y*d, z*d);
}

cocos2d::Vec3 normalize(float x,float y,float z)
{
	float d = x * x + y * y + z*z;
	d = d != 0 ? sqrtf(1.0f / d) : 0.0f;

	return Vec3(x*d, y*d, z*d);
}

cocos2d::Vec2 normalize(float x, float y)
{
	float d = x * x + y * y;
	d = d != 0 ? 1.0f / sqrtf(d):0.0f;

	return Vec2(x*d,y *d);
}

cocos2d::Vec2 normalize(const Vec2 &v)
{
	float d = v.x * v.x + v.y *v.y;
	d = d != 0 ? 1.0f / sqrtf(d):0.0f;

	return Vec2(v.x * d,v.y *d);
}

cocos2d::Vec2 normalize(const cocos2d::Vec2 &v, const cocos2d::Vec2 &u)
{
	float x = u.x - v.x;
	float y = u.y - v.y;

	float d = x * x + y *y;
	d = d != 0 ? 1.0f / sqrtf(d) : 0.0f;

	return Vec2(x * d, y *d);
}

float radian_from(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b)
{
	float angle_a = atan2f(a.y,a.x);
	float angle_b = atan2f(b.y,b.x);

	if (angle_a < 0.0f) angle_a += 2.0f * M_PI;
	if (angle_b < 0.0f)angle_b += 2.0f * M_PI;

	return angle_b - angle_a;
}

float randian_fromy(const cocos2d::Vec2 &axis) {
	float d = sqrtf(axis.x * axis.x + axis.y * axis.y);
	d = d != 0.0f ? 1.0f / d : 0.0f;

	float asin_f = asinf(axis.x * d);
	return asin_f >=0.0f?M_PI_2 - asin_f:asin_f+M_PI_2;
}

float randian_fromy(float x, float y) {
	float d = sqrtf(x * x + y * y);
	d = d != 0.0f ? 1.0f / d : 0.0f;

	float asin_f = asinf(x * d);
	return asin_f >= 0.0f ? M_PI_2 - asin_f : asin_f + M_PI_2;
}

float radians(float angle) {
	return angle / 180.0f * M_PI;
}

float angles(float r) {
	return r * 180.0f /M_PI;
}

float fx_center_point3(cocos2d::Vec2 &center,const cocos2d::Vec2 &a, const cocos2d::Vec2 &b, const cocos2d::Vec2 &c) {
	Vec2  pb = b - a;
	Vec2  pc = c - a;
	Vec2  bc = c - b;

	float	length_ab = length(pb);
	float   length_ac = length(pc);
	float   length_bc = length(bc);

	float  cos_v = pb.dot(pc) / (length_ab * length_ac);
	float  sin_v = sqrtf(1.0f - cos_v * cos_v);
	//圆心
	float ax_cx = a.x - c.x;
	float bx_cx = b.x - c.x;
	float by_cy = b.y - c.y;
	float ay_cy = a.y - c.y;

	float ax_a_cx = a.x + c.x;
	float ay_a_cy = a.y + c.y;
	float bx_a_cx = b.x + c.x;
	float by_a_cy = b.y + c.y;

	float   det_f = 2.0f * (ax_cx * by_cy - ay_cy * bx_cx);
	float   x = (ax_cx * ax_a_cx + ay_cy * ay_a_cy) * by_cy - (bx_cx * bx_a_cx + by_cy * by_a_cy) * ay_cy;
	float  y = ax_cx * (bx_cx * bx_a_cx + by_cy * by_a_cy) - bx_cx * (ax_cx * ax_a_cx + ay_cy * ay_a_cy);

	center.x = x / det_f;
	center.y = y / det_f;

	return length_bc / (2.0f * sin_v);
}

float cross(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b)
{
	return a.x * b.y - a.y * b.x;
}

float cross(const cocos2d::Vec2 &base, const cocos2d::Vec2 &fc1, const cocos2d::Vec2 &fc2)
{
	return	(fc1.x - base.x) * (fc2.y - base.y) - (fc1.y - base.y) * (fc2.x - base.x);
}

float cross_normalize(const cocos2d::Vec2 &fc1, const cocos2d::Vec2 &fc2) {
	float d1 = sqrtf(fc1.x * fc1.x + fc1.y *fc1.y);
	float d2 = sqrtf(fc2.x * fc2.x + fc2.y *fc2.y);

	d1 = d1 != 0.0f ? 1.0f / d1 : 0.0f;
	d2 = d2 != 0.0f ? 1.0f / d2 : 0.0f;

	return d1 * d2 *(fc1.x  * fc2.y - fc1.y * fc2.x);
}

float cross_normalize(const cocos2d::Vec2 &base, const cocos2d::Vec2 &fc1, const cocos2d::Vec2 &fc2) {
	float a_x = fc1.x - base.x;
	float a_y = fc1.y - base.y;

	float b_x = fc2.x - base.x;
	float b_y = fc2.y - base.y;

	float d1 = sqrtf(a_x * a_x+ a_y * a_y);
	float d2 = sqrtf(b_x * b_x + b_y * b_y);

	d1 = d1 != 0.0f ? 1.0f / d1 : 0.0f;
	d2 = d2 != 0.0f ? 1.0f / d2 : 0.0f;

	return d1 * d2 *(a_x  * b_y - a_y * b_x);
}

float sign_area(const cocos2d::Vec2 &db, const cocos2d::Vec2 &dc)
{
	return db.x * dc.y - db.y * dc.x;
}

float clampf(float min, float max, float v)
{
	return fmin(fmax(min,v),max);
}

float randomf10()
{
	return 2.0f * rand() / RAND_MAX - 1.0f;
}

float random()
{
	return 1.0f * rand() / RAND_MAX;
}

float line_point_distance(const cocos2d::Vec2 &start_point, const cocos2d::Vec2 &direction, const cocos2d::Vec2 &point)
{
	float d_x = point.x - start_point.x;
	float d_y = point.y - start_point.y;

	return -direction.y * d_x + direction.x * d_y;
}
bool segment_vertical_intersent(const cocos2d::Vec2 &start_point, const cocos2d::Vec2 &final_point, float x0, cocos2d::Vec2 &intersect_point) {
	float d_x = final_point.x - start_point.x;
	float d_y = final_point.y - start_point.y;

	d_x = d_x != 0.0f ? 1.0f / d_x : 0.0f;
	intersect_point.x = x0;
	intersect_point.y = start_point.y + (x0 - start_point.x) * d_x * d_y;
	return true;
}
//////////////////////////矩阵变换///////////////////////////////////
void vec3_transfrom_mat3(const cocos2d::Vec3 &t, const mat3x3 &mat, cocos2d::Vec3 &dst)
{
	float x = t.x * mat.m[0] + t.y * mat.m[3] + t.z * mat.m[6];
	float y = t.x * mat.m[1] + t.y * mat.m[4] + t.z * mat.m[7];
	float z = t.x * mat.m[2] + t.y * mat.m[5] + t.z * mat.m[8];

	dst.x = x; dst.y = y; dst.z = z;
}

void mat3_transform_vec3(const mat3x3 &mat, const cocos2d::Vec3 &t, cocos2d::Vec3 &dst)
{
	float x = mat.m[0] * t.x + mat.m[1] * t.y + mat.m[2] * t.z;
	float y = mat.m[3] * t.x + mat.m[4] * t.y + mat.m[5] * t.z;
	float z = mat.m[6] * t.x + mat.m[7] * t.y + mat.m[8] * t.z;

	dst.x = x, dst.y = y,dst.z = z;
}

void mat3_load_identity(mat3x3 &mat)
{
	mat.m[0] = 1;
	mat.m[1] = 0;
	mat.m[2] = 0;

	mat.m[3] = 0;
	mat.m[4] = 1;
	mat.m[5] = 0;

	mat.m[6] = 0;
	mat.m[7] = 0;
	mat.m[8] = 1;
}

void  mat3_create_rotate(mat3x3 &mat, const cocos2d::Vec3 &axis, float angle)
{
	float c = cosf(angle * M_PI / 180.0f);
	float s = sinf(angle * M_PI / 180.0f);

	float t = 1.0f - c;
	float tx = t * axis.x;
	float ty = t * axis.y;
	float tz = t * axis.z;
	float txy = tx * axis.y;
	float txz = tx * axis.z;
	float tyz = ty * axis.z;
	float sx = s * axis.x;
	float sy = s * axis.y;
	float sz = s * axis.z;

	mat.m[0] = c + tx*axis.x;
	mat.m[1] = txy + sz;
	mat.m[2] = txz - sy;

	mat.m[3] = txy - sz;
	mat.m[4] = c + ty*axis.y;
	mat.m[5] = tyz + sx;

	mat.m[6] = txz + sy;
	mat.m[7] = tyz - sx;
	mat.m[8] = c + tz*axis.z;
}

void mat3_mutiply(const mat3x3 &a, const mat3x3 &b, mat3x3 &dst)
{
	float m[9];
	float *matrix_array = dst.m != a.m && dst.m != b.m ?dst.m:m;
	matrix_array[0] = a.m[0] * b.m[0] + a.m[1] * b.m[3] + a.m[2] * b.m[6];
	matrix_array[1] = a.m[0] * b.m[1] + a.m[1] * b.m[4] + a.m[2] * b.m[7];
	matrix_array[2] = a.m[0] * b.m[2] + a.m[1] * b.m[5] + a.m[2] * b.m[8];

	matrix_array[3] = a.m[3] * b.m[0] + a.m[4] * b.m[3] + a.m[5] * b.m[6];
	matrix_array[4] = a.m[3] * b.m[1] + a.m[4] * b.m[4] + a.m[5] * b.m[7];
	matrix_array[5] = a.m[3] * b.m[2] + a.m[4] * b.m[5] + a.m[5] * b.m[8];

	matrix_array[3] = a.m[6] * b.m[0] + a.m[7] * b.m[3] + a.m[8] * b.m[6];
	matrix_array[4] = a.m[6] * b.m[1] + a.m[7] * b.m[4] + a.m[8] * b.m[7];
	matrix_array[5] = a.m[6] * b.m[2] + a.m[7] * b.m[5] + a.m[8] * b.m[8];

	if (matrix_array != dst.m)
		memcpy(dst.m,matrix_array,sizeof(float) * 9);
}
///////////////////mat4x4/////////////////////////////////////
void mat4_load_identity(mat4x4 &mat)
{
	mat.m[0] = 1;
	mat.m[1] = 0;
	mat.m[2] = 0;
	mat.m[3] = 0;

	mat.m[4] = 0;
	mat.m[5] = 1;
	mat.m[6] = 0;
	mat.m[7] = 0;

	mat.m[8] = 0;
	mat.m[9] = 0;
	mat.m[10] = 1;
	mat.m[11] = 0;

	mat.m[12] = 0;
	mat.m[13] = 0;
	mat.m[14] = 0;
	mat.m[15] = 1;
}

void mat4_multiply(const mat4x4 &a, const mat4x4 &b, mat4x4 &dst)
{
	float m[16];
	float * matrix_array = dst.m != a.m && dst.m != b.m ? dst.m : m;

	matrix_array[0] = b.m[0] * a.m[0] + b.m[4] * a.m[1] + b.m[8] * a.m[2] + b.m[12] * a.m[3];
	matrix_array[1] = b.m[1] * a.m[0] + b.m[5] * a.m[1] + b.m[9] * a.m[2] + b.m[13] * a.m[3];
	matrix_array[2] = b.m[2] * a.m[0] + b.m[6] * a.m[1] + b.m[10] * a.m[2] + b.m[14] * a.m[3];
	matrix_array[3] = b.m[3] * a.m[0] + b.m[7] * a.m[1] + b.m[11] * a.m[2] + b.m[15] * a.m[3];

	matrix_array[4] = b.m[0] * a.m[4] + b.m[4] * a.m[5] + b.m[8] * a.m[6] + b.m[12] * a.m[7];
	matrix_array[5] = b.m[1] * a.m[4] + b.m[5] * a.m[5] + b.m[9] * a.m[6] + b.m[13] * a.m[7];
	matrix_array[6] = b.m[2] * a.m[4] + b.m[6] * a.m[5] + b.m[10] * a.m[6] + b.m[14] * a.m[7];
	matrix_array[7] = b.m[3] * a.m[4] + b.m[7] * a.m[5] + b.m[11] * a.m[6] + b.m[15] * a.m[7];

	matrix_array[8] = b.m[0] * a.m[8] + b.m[4] * a.m[9] + b.m[8] * a.m[10] + b.m[12] * a.m[11];
	matrix_array[9] = b.m[1] * a.m[8] + b.m[5] * a.m[9] + b.m[9] * a.m[10] + b.m[13] * a.m[11];
	matrix_array[10] = b.m[2] * a.m[8] + b.m[6] * a.m[9] + b.m[10] * a.m[10] + b.m[14] * a.m[11];
	matrix_array[11] = b.m[3] * a.m[8] + b.m[7] * a.m[9] + b.m[11] * a.m[10] + b.m[15] * a.m[11];

	matrix_array[12] = b.m[0] * a.m[12] + b.m[4] * a.m[13] + b.m[8] * a.m[14] + b.m[12] * a.m[15];
	matrix_array[13] = b.m[1] * a.m[12] + b.m[5] * a.m[13] + b.m[9] * a.m[14] + b.m[13] * a.m[15];
	matrix_array[14] = b.m[2] * a.m[12] + b.m[6] * a.m[13] + b.m[10] * a.m[14] + b.m[14] * a.m[15];
	matrix_array[15] = b.m[3] * a.m[12] + b.m[7] * a.m[13] + b.m[11] * a.m[14] + b.m[15] * a.m[15];

	if (matrix_array != dst.m)
		memcpy(dst.m,matrix_array,sizeof(float) * 16);
}
NS_GT_END