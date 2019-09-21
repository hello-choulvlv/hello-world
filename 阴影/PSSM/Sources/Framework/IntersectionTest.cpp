#include "Math.h"
#include "Frustum.h"
#include "IntersectionTests.h"
/*
  *�ཻ���Ե��㷨�ĺ���˼�붼�ǻ���������һ������
  *���������ĳһ����Χ�в��ཻ,��ô�����Χ�е�6���������� ���·��һ��������С ���·��
  *���߸����㷨�����ĵ�ʮ���¶�������������ϸ��˵��
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
//����ͶӰ�¼���������׶����ཻ����
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

	//��ʵ�ʵ�ʹ����,���ж��ָ�Ϊ��Ч,���ٵ��㷨,���Ƕ���������Ҫ��,��������,����[A,B]-[C,D]
	//bool overlap = (unsigned)(B - C) <= B - A + D - C
	//���л�����һ��AABB ���������߳���ʾ����ײ���
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
	//��׶�������
	Vector3 vFrustumCenter = (vFrustumMin + vFrustumMax) * 0.5f;
	//��׶��ĳߴ��һ��
	Vector3 vFrustumHalfSize = (vFrustumMax - vFrustumMin) * 0.5f;
	//��׶��������������߷����ͶӰ
	float fFrustumCenterProj = Dot(vFrustumCenter, vSweepDir);
	// project AABB half-size
	//�ô��������Ϊ:������׶��İ�ߴ������߷�������ͶӰ����(���߷���İ˸�����)
	float fFrustumHalfSizeProj = vFrustumHalfSize.x * fabs(vSweepDir.x) + vFrustumHalfSize.y * fabs(vSweepDir.y) + vFrustumHalfSize.z * fabs(vSweepDir.z);
	//���Ź��߷���,��׶�����Դ�����,�����
	float fFrustumProjMin = fFrustumCenterProj - fFrustumHalfSizeProj;
	float fFrustumProjMax = fFrustumCenterProj + fFrustumHalfSizeProj;

	// project AABB center point
	Vector3 vObjectCenter = (vObjectMin + vObjectMax) * 0.5f;
	Vector3 vObjectHalfSize = (vObjectMax - vObjectMin) * 0.5f;
	float fObjectCenterProj = Dot(vObjectCenter, vSweepDir);
	// project AABB half-size
	//��ʵ�ʵ�ʹ����,Ҳ���Ա�ʾΪ,ͬ����������˷�,ÿ�����Ľ����Ϊ�Ǹ���
	float fObjectHalfSizeProj = vObjectHalfSize.x * fabs(vSweepDir.x) + vObjectHalfSize.y * fabs(vSweepDir.y) + vObjectHalfSize.z * fabs(vSweepDir.z);
	//�伸������ͬ������
	float fObjectProjMin = fObjectCenterProj - fObjectHalfSizeProj;
	float fObjectProjMax = fObjectCenterProj + fObjectHalfSizeProj;

	// find the distance in sweep direction
	// where intersection occurs on all axis.
	// sweep direction intersection
	// starts: fObjectProjMax + fDist = fFrustumProjMin
	// ends: fObjectProjMin + fDist = fFrustumProjMax
	//�����������ݵ���ʵ����Ϊ:��׶����ĳһ�㵽��������������,�������һ�����,��С���뷶Χ
	float fDistMin = fFrustumProjMin - fObjectProjMax;//��С�����
	float fDistMax = fFrustumProjMax - fObjectProjMin;//��� ��̾���
	//�������Ϊ������:Ŀǰ��û�з���
	if (fDistMin > fDistMax)
		Swap(fDistMin, fDistMax);

	// only intersects in opposite of sweep direction
	if (fDistMax < 0) return false;//��������׶�����Զ��֮��

	// intersection on an axis:
	// starts: vObjectMax.x + fDist*vSweepDir.x = vFrustumMin.x
	//   ends: vObjectMin.x + fDist*vSweepDir.x = vFrustumMax.x
	//�������Χ���ཻ����
	// test x-axis:
	if (vSweepDir.x == 0)
	{
		// there is never an intersection on this axis
		if (vFrustumMin.x > vObjectMax.x || vObjectMin.x > vFrustumMax.x) return false;
	}
	else//������Χ��֮������ĳһ�����򷴸����������Χ�е��ཻ����
	{
		float fDistMinNew = (vFrustumMin.x - vObjectMax.x) / vSweepDir.x;//����������׶������
		float fDistMaxNew = (vFrustumMax.x - vObjectMin.x) / vSweepDir.x;//����������׶����Ҳ�
		if (fDistMinNew > fDistMaxNew) //�������ò��û�д���,����������,vSweepDir��صķ���Ϊ��ֵ
			Swap(fDistMinNew, fDistMaxNew);

		// distance ranges don't overlap
		//��������ʱ,��Ӧ��ʱ������ֱ�����׶�����������
		if (fDistMin > fDistMaxNew || fDistMinNew > fDistMax) return false;
		// otherwise merge ranges
		fDistMin = Max(fDistMin, fDistMinNew);//ʲô����������,��ֵ�ᷢ���ı�,��������ȫͶӰ����׶��,���������(��󼸺���x����С����С��׶��x����)бͶӰ
		fDistMax = Min(fDistMax, fDistMaxNew);//����������x�����ڹ���yu��׶������x�����γɵ�ֱ�ߵ����,�Ҽ��������Сx������ֱ�ߵ��ڲ�
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
//����ͶӰ����,����������׶���Ƿ����ص�
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
	//�������������׶���ǰ�����ߺ�,����ζ��û���ص�
	// test for overlap
	if (fMin1 > fMax2 || fMin2 > fMax1) return false;

	return true;
}

// AABB vs Frustum test, returns true if objects intersect
//��׼����׶��ü�
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
	//����ʱ��׼����׶��,Ҳ����ƽ��ͷ��Ĳü��㷨
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
	//������õı�������ӳ�䵽vSweepDir������
	if (fSweepDir != 0)
	{
		// intersection starts when fMax1 + fSweepDir * t >= fMin2
		float fIntersectionStart = (fMin2 - fMax1) / fSweepDir;//fSweepDir�������� 

		// intersection ends when fMin1 + fSweepDir * t >= fMax2
		float fIntersectionEnd = (fMax2 - fMin1) / fSweepDir;//fSweepDir������Ҳ�

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
	//����׶�嵽���������Զ+�������
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