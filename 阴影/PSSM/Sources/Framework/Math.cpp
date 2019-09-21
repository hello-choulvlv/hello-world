#include "Math.h"


static float dot(const Vec3 &a,const Vec3 &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

//Äæ¾ØÕó
inline void  Matrix::reverse(Matrix  &reverse_matrix)
{
	float  *m_array = (float *)m;
	float a0 = m_array[0] * m_array[5] - m_array[1] * m_array[4];
	float a1 = m_array[0] * m_array[6] - m_array[2] * m_array[4];
	float a2 = m_array[0] * m_array[7] - m_array[3] * m_array[4];
	float a3 = m_array[1] * m_array[6] - m_array[2] * m_array[5];
	float a4 = m_array[1] * m_array[7] - m_array[3] * m_array[5];
	float a5 = m_array[2] * m_array[7] - m_array[3] * m_array[6];
	float b0 = m_array[8] * m_array[13] - m_array[9] * m_array[12];
	float b1 = m_array[8] * m_array[14] - m_array[10] * m_array[12];
	float b2 = m_array[8] * m_array[15] - m_array[11] * m_array[12];
	float b3 = m_array[9] * m_array[14] - m_array[10] * m_array[13];
	float b4 = m_array[9] * m_array[15] - m_array[11] * m_array[13];
	float b5 = m_array[10] * m_array[15] - m_array[11] * m_array[14];

	// Calculate the determinant.
	float det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;

	// Close to zero, can't invert.
	assert((det > 0 ? det : -det) >= 0.00001);
	// Support the case where m == dst.
	float  *out_array = (float *)reverse_matrix.m;
	float    f = 1.0f / det;
	out_array[0] = f * (m_array[5] * b5 - m_array[6] * b4 + m_array[7] * b3);
	out_array[1] = f * (-m_array[1] * b5 + m_array[2] * b4 - m_array[3] * b3);
	out_array[2] = f *(m_array[13] * a5 - m_array[14] * a4 + m_array[15] * a3);
	out_array[3] = f * (-m_array[9] * a5 + m_array[10] * a4 - m_array[11] * a3);

	out_array[4] = f * (-m_array[4] * b5 + m_array[6] * b2 - m_array[7] * b1);
	out_array[5] = f *(m_array[0] * b5 - m_array[2] * b2 + m_array[3] * b1);
	out_array[6] = f *(-m_array[12] * a5 + m_array[14] * a2 - m_array[15] * a1);
	out_array[7] = f * (m_array[8] * a5 - m_array[10] * a2 + m_array[11] * a1);

	out_array[8] = f * (m_array[4] * b4 - m_array[5] * b2 + m_array[7] * b0);
	out_array[9] = f * (-m_array[0] * b4 + m_array[1] * b2 - m_array[3] * b0);
	out_array[10] = f * (m_array[12] * a4 - m_array[13] * a2 + m_array[15] * a0);
	out_array[11] = f * (-m_array[8] * a4 + m_array[9] * a2 - m_array[11] * a0);

	out_array[12] = f * (-m_array[4] * b3 + m_array[5] * b1 - m_array[6] * b0);
	out_array[13] = f * (m_array[0] * b3 - m_array[1] * b1 + m_array[2] * b0);
	out_array[14] = f * (-m_array[12] * a3 + m_array[13] * a1 - m_array[14] * a0);
	out_array[15] = f * (m_array[8] * a3 - m_array[9] * a1 + m_array[10] * a0);
}

void  vec3_multiply_matrix(const Vector3 &src, const Matrix &mat, Vector3 *dst)
{
	float x = src.x * mat.m[0][0] + src.y * mat.m[1][0] + src.z * mat.m[2][0] + mat.m[3][0];
	float y = src.x * mat.m[0][1] + src.y * mat.m[1][1] + src.z * mat.m[2][1] + mat.m[3][1];
	float z = src.x * mat.m[0][2] + src.y * mat.m[1][2] + src.z * mat.m[2][2] + mat.m[3][2];

	dst->x = x, dst->y = y,dst->z = z;
}

void  create_ortho_matrix(Matrix &ortho, float  left, float right, float  bottom, float  top, float  nearZ, float  farZ)
{
	float       deltaX = right - left;
	float       deltaY = top - bottom;
	float       deltaZ = farZ - nearZ;
	assert(deltaX > 0.0f && deltaY > 0.0f && deltaZ > 0.0f);

	ortho.m[0][0] = 2.0f / deltaX;
	ortho.m[0][1] = 0.0f;
	ortho.m[0][2] = 0.0f;
	ortho.m[0][3] = 0.0f;

	ortho.m[1][0] = 0.0f;
	ortho.m[1][1] = 2.0f / deltaY;
	ortho.m[1][2] = 0.0f;
	ortho.m[1][3] = 0.0f;

	ortho.m[2][0] = 0.0f;
	ortho.m[2][1] = 0.0f;
	ortho.m[2][2] = 1.0f / deltaZ;
	ortho.m[2][3] = 0.0f;

	ortho.m[3][0] = -(right + left) / deltaX;
	ortho.m[3][1] = -(top + bottom) / deltaY;
	ortho.m[3][2] = nearZ/ deltaZ;
	ortho.m[3][3] = 1.0f;
}

void  create_frustum_matrix(Matrix &frust, float left, float right, float bottom, float top, float nearZ, float farZ)
{
	float       deltaX = right - left;
	float       deltaY = top - bottom;
	float       deltaZ = farZ - nearZ;

	assert(deltaX > 0.0f && deltaY > 0.0f && deltaZ > 0.0f && nearZ > 0.0f);

	frust.m[0][0] = 2.0f * nearZ / deltaX;
	frust.m[0][1] = frust.m[0][2] = frust.m[0][3] = 0.0f;

	frust.m[1][1] = 2.0f * nearZ / deltaY;
	frust.m[1][0] = frust.m[1][2] = frust.m[1][3] = 0.0f;

	frust.m[2][0] = (right + left) / deltaX;
	frust.m[2][1] = (top + bottom) / deltaY;
	frust.m[2][2] = farZ/ deltaZ;
	frust.m[2][3] = 1.0f;

	frust.m[3][2] = -nearZ * farZ / deltaZ;
	frust.m[3][0] = frust.m[3][1] = frust.m[3][3] = 0.0f;
}

Plane::Plane():
	_normal(0.f, 0.f, 1.f),
	_dist(0.f)
{
}

// create plane from tree point
Plane::Plane(const Vec3& p1, const Vec3& p2, const Vec3& p3)
{
	initPlane(p1, p2, p3);
}

// create plane from normal and dist
Plane::Plane(const Vec3& normal, float dist)
{
	initPlane(normal, dist);
}

// create plane from normal and a point on plane
Plane::Plane(const Vec3& normal, const Vec3& point)
{
	initPlane(normal, point);
}

void Plane::initPlane(const Vec3& p1, const Vec3& p2, const Vec3& p3)
{
	Vec3 p21 = p2 - p1;
	Vec3 p32 = p3 - p2;
	//Vec3::Cross(p21, p32, &_normal);
	_normal = Cross(p21,p32);
	//_normal.normalize();
	_normal = Normalize(_normal);
	//_dist = _normal.Dot(p1);
	_dist = dot(_normal,p1);
}

void Plane::initPlane(const Vec3& normal, float dist)
{
	float oneOverLength = 1 / normal.Length();// normal.length();
	_normal = normal * oneOverLength;
	_dist = dist * oneOverLength;
}

void Plane::initPlane(const Vec3& normal, const Vec3& point)
{
	_normal = normal;
	//_normal.normalize();
	_normal = Normalize(_normal);
	//_dist = _normal.dot(point);
	_dist = dot(_normal,point);
}

float Plane::dist2Plane(const Vec3& p) const
{
	//return _normal.dot(p) - _dist;
	return dot(_normal,p) - _dist;
}


PointSide Plane::getSide(const Vec3& point) const
{
	float dist = dist2Plane(point);
	if (dist > 0)
		return PointSide::FRONT_PLANE;
	else if (dist < 0)
		return PointSide::BEHIND_PLANE;
	else
		return PointSide::IN_PLANE;
}