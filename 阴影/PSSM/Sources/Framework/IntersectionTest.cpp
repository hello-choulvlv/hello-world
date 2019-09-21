#include "Math.h"
#include "Frustum.h"
#include "IntersectionTests.h"
/*
  *相交测试的算法的核心思想都是基于这样的一个现象
  *如果射线与某一个包围盒不相交,那么其与包围盒的6个坐标轴的最长 最短路径一定大于最小 最长的路径
  *光线跟踪算法技术的第十九章对这个算办有着详细的说明
 */

bool   intersect_test_new(const Frustum &frustum, const BoundingBox &aabb)
{
	Vec3 point;
	const  Vec3  &aabb_min = aabb.m_vMin;
	const Vec3  &aabb_max = aabb.m_vMax;
	const  Plane  *planes = frustum._plane;

	for (int i = 0; i < 6; i++)
	{
		const Vec3& normal = planes[i].getNormal();
		point.x = normal.x < 0 ? aabb_max.x : aabb_min.x;
		point.y = normal.y < 0 ? aabb_max.y : aabb_min.y;
		point.z = normal.z < 0 ? aabb_max.z : aabb_min.z;

		if (planes[i].getSide(point) == PointSide::FRONT_PLANE)
			return false;
	}
	return true;
}

bool   intersect_test_light(const Plane *light_planes,int length_l,const BoundingBox &aabb)
{
	Vec3 point;
	const  Vec3  &aabb_min = aabb.m_vMin;
	const Vec3  &aabb_max = aabb.m_vMax;

	for (int i = 0; i < length_l; i++)
	{
		const Vec3& normal = light_planes[i].getNormal();
		point.x = normal.x < 0 ? aabb_max.x : aabb_min.x;
		point.y = normal.y < 0 ? aabb_max.y : aabb_min.y;
		point.z = normal.z < 0 ? aabb_max.z : aabb_min.z;

		if (light_planes[i].getSide(point) == PointSide::FRONT_PLANE)
			return false;
	}
	return true;
}

void   generate_frustum_plane(const Frustum  &frustum,Plane *plane)
{
	const Vec3  *points = frustum.m_pPoints;

	plane[0].initPlane(Cross(points[1] - points[0], points[3] - points[0]),points[0]);//0 near
	plane[1].initPlane(Cross(points[7] - points[4], points[5] - points[4]),points[4]);//1 far

	plane[2].initPlane(Cross(points[5] - points[4], points[0] - points[4]),points[4]);//2 left
	plane[3].initPlane(Cross(points[2] - points[3],points[7] - points[3]),points[3]);//3 right

	plane[4].initPlane(Cross(points[0] - points[4],points[7] - points[4]),points[4]);//4 bottom
	plane[5].initPlane(Cross(points[5] - points[1],points[2] - points[1]),points[1]);//5 top
}

int      judge_relative_position(const Plane &pa, const Plane &pb, const Vec3  &light_direction)
{
	float dot_a = Dot(pa.getNormal(),light_direction);
	float dot_b = Dot(pb.getNormal(),light_direction);

	if (dot_a * dot_b > 0)
		return 0;
	return dot_a > 0 ? 1: 2;
}
//正交投影下几何体与视锥体的相交测试
bool intersect_frustum_ortho()
{
	return true;
}

bool IntersectionTest(const BoundingBox &objectBB, const BoundingBox &frustumBB)
{
	// min and max vectors
	const Vector3 &vFrustumMin = frustumBB.m_vMin;
	const Vector3 &vFrustumMax = frustumBB.m_vMax;
	const Vector3 &vObjectMin = objectBB.m_vMin;
	const Vector3 &vObjectMax = objectBB.m_vMax;

	//在实际的使用中,还有多种更为高效,快速的算法,但是对数据有所要求,比如如下,区间[A,B]-[C,D]
	//bool overlap = (unsigned)(B - C) <= B - A + D - C
	//还有基于另一种AABB 的中心与半边长表示法碰撞检测
	// test all axes
	if (vObjectMin.x > vFrustumMax.x || vFrustumMin.x > vObjectMax.x) return false;
	if (vObjectMin.y > vFrustumMax.y || vFrustumMin.y > vObjectMax.y) return false;
	if (vObjectMin.z > vFrustumMax.z || vFrustumMin.z > vObjectMax.z) return false;

	// all tests passed - intersection occurs
	return true;
}

bool SweepIntersectionTest(const BoundingBox &objectBB, const BoundingBox &frustumBB, const Vector3 &vSweepDir)
{
	// min and max vectors of object
	const Vector3 &vFrustumMin = frustumBB.m_vMin;
	const Vector3 &vFrustumMax = frustumBB.m_vMax;
	const Vector3 &vObjectMin = objectBB.m_vMin;
	const Vector3 &vObjectMax = objectBB.m_vMax;

	// calculate projections along sweep direction
	// project AABB center point
	//视锥体的中心
	Vector3 vFrustumCenter = (vFrustumMin + vFrustumMax) * 0.5f;
	//视锥体的尺寸的一半
	Vector3 vFrustumHalfSize = (vFrustumMax - vFrustumMin) * 0.5f;
	//视锥体的中心沿着视线方向的投影
	float fFrustumCenterProj = Dot(vFrustumCenter, vSweepDir);
	// project AABB half-size
	//该代码的语义为:计算视锥体的半尺寸在视线方向的最大投影距离(视线方向的八个变种)
	float fFrustumHalfSizeProj = vFrustumHalfSize.x * fabs(vSweepDir.x) + vFrustumHalfSize.y * fabs(vSweepDir.y) + vFrustumHalfSize.z * fabs(vSweepDir.z);
	//沿着光线方向,视锥体离光源的最短,最长距离
	float fFrustumProjMin = fFrustumCenterProj - fFrustumHalfSizeProj;
	float fFrustumProjMax = fFrustumCenterProj + fFrustumHalfSizeProj;

	// project AABB center point
	Vector3 vObjectCenter = (vObjectMin + vObjectMax) * 0.5f;
	Vector3 vObjectHalfSize = (vObjectMax - vObjectMin) * 0.5f;
	float fObjectCenterProj = Dot(vObjectCenter, vSweepDir);
	// project AABB half-size
	//在实际的使用中,也可以表示为,同方向的向量乘法,每分量的结果都为非负数
	float fObjectHalfSizeProj = vObjectHalfSize.x * fabs(vSweepDir.x) + vObjectHalfSize.y * fabs(vSweepDir.y) + vObjectHalfSize.z * fabs(vSweepDir.z);
	//其几何意义同上所述
	float fObjectProjMin = fObjectCenterProj - fObjectHalfSizeProj;
	float fObjectProjMax = fObjectCenterProj + fObjectHalfSizeProj;

	// find the distance in sweep direction
	// where intersection occurs on all axis.
	// sweep direction intersection
	// starts: fObjectProjMax + fDist = fFrustumProjMin
	// ends: fObjectProjMin + fDist = fFrustumProjMax
	//以下两个数据的真实含义为:视锥体上某一点到几何体的最近距离,事先求出一个最大,最小距离范围
	float fDistMin = fFrustumProjMin - fObjectProjMax;//最小最长距离
	float fDistMax = fFrustumProjMax - fObjectProjMin;//最大 最短距离
	//条件语句为真的情况:目前还没有发现
	if (fDistMin > fDistMax)
		Swap(fDistMin, fDistMax);

	// only intersects in opposite of sweep direction
	if (fDistMax < 0) return false;//落在了视锥体的最远端之外

	// intersection on an axis:
	// starts: vObjectMax.x + fDist*vSweepDir.x = vFrustumMin.x
	//   ends: vObjectMin.x + fDist*vSweepDir.x = vFrustumMax.x
	//射线与包围盒相交测试
	// test x-axis:
	if (vSweepDir.x == 0)
	{
		// there is never an intersection on this axis
		if (vFrustumMin.x > vObjectMax.x || vObjectMin.x > vFrustumMax.x) return false;
	}
	else//两个包围盒之间沿着某一个方向反复做射线与包围盒的相交测试
	{
		float fDistMinNew = (vFrustumMin.x - vObjectMax.x) / vSweepDir.x;//几何体在视锥体的左侧
		float fDistMaxNew = (vFrustumMax.x - vObjectMin.x) / vSweepDir.x;//几何体在视锥体的右侧
		if (fDistMinNew > fDistMaxNew) //条件语句貌似没有触发,触发的条件,vSweepDir相关的分量为负值
			Swap(fDistMinNew, fDistMaxNew);

		// distance ranges don't overlap
		//条件成立时,对应的时几何体分别处于视锥体的左右两侧
		if (fDistMin > fDistMaxNew || fDistMinNew > fDistMax) return false;
		// otherwise merge ranges
		fDistMin = Max(fDistMin, fDistMinNew);//什么样的条件下,数值会发生改变,几何体完全投影到视锥体,并且是左侧(最大几何体x坐标小于最小视锥体x坐标)斜投影
		fDistMax = Min(fDistMax, fDistMaxNew);//几何体的最大x坐标在光线yu视锥体的最大x坐标形成的直线的外侧,且几何体的最小x坐标在直线的内侧
	}

	// test y-axis:
	if (vSweepDir.y == 0)
	{
		// there is never an intersection on this axis
		if (vFrustumMin.y > vObjectMax.y || vObjectMin.y > vFrustumMax.y) return false;
	}
	else
	{
		float fDistMinNew = (vFrustumMin.y - vObjectMax.y) / vSweepDir.y;
		float fDistMaxNew = (vFrustumMax.y - vObjectMin.y) / vSweepDir.y;
		if (fDistMinNew > fDistMaxNew)
			Swap(fDistMinNew, fDistMaxNew);

		// distance ranges don't overlap
		if (fDistMin > fDistMaxNew || fDistMinNew > fDistMax) return false;
		// otherwise merge ranges
		fDistMin = Max(fDistMin, fDistMinNew);
		fDistMax = Min(fDistMax, fDistMaxNew);
	}

	// test z-axis:
	if (vSweepDir.z == 0)
	{
		// there is never an intersection on this axis
		if (vFrustumMin.z > vObjectMax.z || vObjectMin.z > vFrustumMax.z) return false;
	}
	else
	{
		float fDistMinNew = (vFrustumMin.z - vObjectMax.z) / vSweepDir.z;
		float fDistMaxNew = (vFrustumMax.z - vObjectMin.z) / vSweepDir.z;
		if (fDistMinNew > fDistMaxNew)
			Swap(fDistMinNew, fDistMaxNew);

		// distance ranges don't overlap
		if (fDistMin > fDistMaxNew || fDistMinNew > fDistMax) return false;
	}

	// all tests passed - intersection occurs
	return true;
}

// Helper function for AABB vs frustum test
//沿着投影方向,几何体与视锥体是否有重叠
bool ProjectedIntersection(const Vector3 &vHalfSize, const Vector3 &vCenter, const Vector3 *pFrustumPoints, const Vector3 &vDir)
{
	// project AABB center point to vector
	float fCenter = Dot(vCenter, vDir);
	// project AABB half-size to vector
	float fHalfSize = vHalfSize.x * fabs(vDir.x) + vHalfSize.y * fabs(vDir.y) + vHalfSize.z * fabs(vDir.z);

	float fMin1 = fCenter - fHalfSize;
	float fMax1 = fCenter + fHalfSize;

	// project frustum points
	float fProj2 = Dot(pFrustumPoints[0], vDir);
	float fMin2 = fProj2;
	float fMax2 = fProj2;
	for (int i = 1; i < 8; i++)
	{
		fProj2 = Dot(pFrustumPoints[i], vDir);
		fMin2 = Min(fProj2, fMin2);
		fMax2 = Max(fProj2, fMax2);
	}
	//如果几何体在视锥体的前方或者后方,则意味着没有重叠
	// test for overlap
	if (fMin1 > fMax2 || fMin2 > fMax1) return false;

	return true;
}

// AABB vs Frustum test, returns true if objects intersect
//标准的视锥体裁剪
bool IntersectionTest(const BoundingBox &objectBB, const Frustum &frustum)
{
	// Note that this code is not very optimal
	Vector3 vHalfSize = (objectBB.m_vMax - objectBB.m_vMin) * 0.5f;
	Vector3 vCenter = (objectBB.m_vMin + objectBB.m_vMax) * 0.5f;

	// AABB face normals
	if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, Vector3(1, 0, 0))) return false;
	if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, Vector3(0, 1, 0))) return false;
	if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, Vector3(0, 0, 1))) return false;

	// frustum face normals
	//以下时标准的视锥体,也就是平截头体的裁剪算法
	// front and back faces:
	Vector3 vNorm1 = Normalize(Cross(frustum.m_pPoints[1] - frustum.m_pPoints[0],
		frustum.m_pPoints[3] - frustum.m_pPoints[0]));
	if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm1)) return false;

	// left face:
	Vector3 vNorm2 = Normalize(Cross(frustum.m_pPoints[1] - frustum.m_pPoints[0],
		frustum.m_pPoints[4] - frustum.m_pPoints[0]));
	if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm2)) return false;

	// right face:
	Vector3 vNorm3 = Normalize(Cross(frustum.m_pPoints[2] - frustum.m_pPoints[3],
		frustum.m_pPoints[7] - frustum.m_pPoints[3]));
	if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm3)) return false;

	// top face:
	Vector3 vNorm4 = Normalize(Cross(frustum.m_pPoints[2] - frustum.m_pPoints[1],
		frustum.m_pPoints[5] - frustum.m_pPoints[1]));
	if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm4)) return false;

	// bottom face:
	Vector3 vNorm5 = Normalize(Cross(frustum.m_pPoints[3] - frustum.m_pPoints[0],
		frustum.m_pPoints[4] - frustum.m_pPoints[0]));
	if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm5)) return false;


	// edge cross edge cases
	//
	Vector3 pBoxEdges[3] = { Vector3(1,0,0), Vector3(0,1,0), Vector3(0,0,1) };
	for (int i = 0; i < 3; i++)
	{
		// edge up-down
		Vector3 vNorm1 = Normalize(Cross(frustum.m_pPoints[1] - frustum.m_pPoints[0], pBoxEdges[i]));
		if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm1)) return false;

		// edge left-right
		Vector3 vNorm2 = Normalize(Cross(frustum.m_pPoints[3] - frustum.m_pPoints[0], pBoxEdges[i]));
		if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm2)) return false;

		// edge bottom left
		Vector3 vNorm3 = Normalize(Cross(frustum.m_pPoints[4] - frustum.m_pPoints[0], pBoxEdges[i]));
		if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm3)) return false;

		// edge top left
		Vector3 vNorm4 = Normalize(Cross(frustum.m_pPoints[5] - frustum.m_pPoints[1], pBoxEdges[i]));
		if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm4)) return false;

		// edge top right
		Vector3 vNorm5 = Normalize(Cross(frustum.m_pPoints[6] - frustum.m_pPoints[2], pBoxEdges[i]));
		if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm5)) return false;

		// edge bottom right
		Vector3 vNorm6 = Normalize(Cross(frustum.m_pPoints[7] - frustum.m_pPoints[3], pBoxEdges[i]));
		if (!ProjectedIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm6)) return false;
	}

	// all tests passed - intersection occurs
	return true;
}

bool ProjectedSweepIntersection(const Vector3 &vHalfSize, const Vector3 &vCenter,const Vector3 *pFrustumPoints,
	const Vector3 &vDir,float &fDistMin, float &fDistMax, const Vector3 &vSweepDir)
{
	// project sweep direction
	float fSweepDir = Dot(vSweepDir, vDir);

	// project AABB center point to vector
	float fCenter = Dot(vCenter, vDir);
	// project AABB half-size to vector
	float fHalfSize = vHalfSize.x * fabs(vDir.x) + vHalfSize.y * fabs(vDir.y) + vHalfSize.z * fabs(vDir.z);

	float fMin1 = fCenter - fHalfSize;
	float fMax1 = fCenter + fHalfSize;

	// project frustum points
	float fProj2 = Dot(pFrustumPoints[0], vDir);
	float fMin2 = fProj2;
	float fMax2 = fProj2;
	for (int i = 1; i < 8; i++)
	{
		fProj2 = Dot(pFrustumPoints[i], vDir);
		fMin2 = Min(fProj2, fMin2);
		fMax2 = Max(fProj2, fMax2);
	}

	// sweep can affect intersection
	//将所获得的标量长度映射到vSweepDir方向上
	if (fSweepDir != 0)
	{
		// intersection starts when fMax1 + fSweepDir * t >= fMin2
		float fIntersectionStart = (fMin2 - fMax1) / fSweepDir;//fSweepDir方向的左侧 

		// intersection ends when fMin1 + fSweepDir * t >= fMax2
		float fIntersectionEnd = (fMax2 - fMin1) / fSweepDir;//fSweepDir方向的右侧

		// ranges must be in right order
		if (fIntersectionStart > fIntersectionEnd) Swap(fIntersectionStart, fIntersectionEnd);

		// distance ranges don't overlap
		if (fDistMin > fIntersectionEnd || fIntersectionStart > fDistMax)
		{
			return false;
		}

		// otherwise merge ranges
		fDistMin = Max(fDistMin, fIntersectionStart);
		fDistMax = Min(fDistMax, fIntersectionEnd);
	}
	// sweep doesn't affect intersection
	else
	{
		// no intersection ever
		if (fMin1 > fMax2 || fMin2 > fMax1)
		{
			return false;
		}
	}

	return true;
}

// AABB vs Frustum sweep test, returns true if intersection can occur if object is translated along given direction
bool SweepIntersectionTest(const BoundingBox &objectBB, const Frustum &frustum, const Vector3 &vSweepDir)
{
	// Note that this code is not very optimal
	Vector3 vHalfSize = (objectBB.m_vMax - objectBB.m_vMin) * 0.5f;
	Vector3 vCenter = (objectBB.m_vMin + objectBB.m_vMax) * 0.5f;

	float fDistMin = 0.0f;
	float fDistMax = FLT_MAX;
	//求视锥体到几何体的最远+最近距离
	// find potential intersection range in sweep direction
	if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, vSweepDir, fDistMin, fDistMax, vSweepDir)) return false;

	// AABB face normals
	if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, Vector3(1, 0, 0), fDistMin, fDistMax, vSweepDir)) return false;
	if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, Vector3(0, 1, 0), fDistMin, fDistMax, vSweepDir)) return false;
	if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, Vector3(0, 0, 1), fDistMin, fDistMax, vSweepDir)) return false;

	// frustum face normals
	//

	// front and back faces:
	Vector3 vNorm1 = Normalize(Cross(frustum.m_pPoints[1] - frustum.m_pPoints[0],
		frustum.m_pPoints[3] - frustum.m_pPoints[0]));
	if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm1, fDistMin, fDistMax, vSweepDir)) return false;

	// left face:
	Vector3 vNorm2 = Normalize(Cross(frustum.m_pPoints[1] - frustum.m_pPoints[0],
		frustum.m_pPoints[4] - frustum.m_pPoints[0]));
	if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm2, fDistMin, fDistMax, vSweepDir)) return false;

	// right face:
	Vector3 vNorm3 = Normalize(Cross(frustum.m_pPoints[2] - frustum.m_pPoints[3],
		frustum.m_pPoints[7] - frustum.m_pPoints[3]));
	if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm3, fDistMin, fDistMax, vSweepDir)) return false;

	// top face:
	Vector3 vNorm4 = Normalize(Cross(frustum.m_pPoints[2] - frustum.m_pPoints[1],
		frustum.m_pPoints[5] - frustum.m_pPoints[1]));
	if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm4, fDistMin, fDistMax, vSweepDir)) return false;

	// bottom face:
	Vector3 vNorm5 = Normalize(Cross(frustum.m_pPoints[3] - frustum.m_pPoints[0],
		frustum.m_pPoints[4] - frustum.m_pPoints[0]));
	if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm5, fDistMin, fDistMax, vSweepDir)) return false;


	// edge cross edge cases
	//
	Vector3 pBoxEdges[3] = { Vector3(1,0,0), Vector3(0,1,0), Vector3(0,0,1) };
	for (int i = 0; i < 3; i++)
	{
		// edge up-down
		Vector3 vNorm1 = Normalize(Cross(frustum.m_pPoints[1] - frustum.m_pPoints[0], pBoxEdges[i]));
		if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm1, fDistMin, fDistMax, vSweepDir)) return false;

		// edge left-right
		Vector3 vNorm2 = Normalize(Cross(frustum.m_pPoints[3] - frustum.m_pPoints[0], pBoxEdges[i]));
		if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm2, fDistMin, fDistMax, vSweepDir)) return false;

		// edge bottom left
		Vector3 vNorm3 = Normalize(Cross(frustum.m_pPoints[4] - frustum.m_pPoints[0], pBoxEdges[i]));
		if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm3, fDistMin, fDistMax, vSweepDir)) return false;

		// edge top left
		Vector3 vNorm4 = Normalize(Cross(frustum.m_pPoints[5] - frustum.m_pPoints[1], pBoxEdges[i]));
		if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm4, fDistMin, fDistMax, vSweepDir)) return false;

		// edge top right
		Vector3 vNorm5 = Normalize(Cross(frustum.m_pPoints[6] - frustum.m_pPoints[2], pBoxEdges[i]));
		if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm5, fDistMin, fDistMax, vSweepDir)) return false;

		// edge bottom right
		Vector3 vNorm6 = Normalize(Cross(frustum.m_pPoints[7] - frustum.m_pPoints[3], pBoxEdges[i]));
		if (!ProjectedSweepIntersection(vHalfSize, vCenter, frustum.m_pPoints, vNorm6, fDistMin, fDistMax, vSweepDir)) return false;
	}

	// all tests passed - intersection occurs
	return true;
}