#pragma once
//�°���׶��ü��㷨
bool   intersect_test_new(const Frustum &frustum, const BoundingBox &aabb);
//��ռ��������ռ���׶��Ĳü�
bool   intersect_test_light(const Plane *light_planes, int length_l, const BoundingBox &aabb);
//������׶�������ƽ�淽��
void   generate_frustum_plane(const Frustum  &frustum,Plane  *planes);
//�ж������ཻ��һ���ߵ�ƽ����Թ��߷����λ��,0ȫ�������Դ����ȫ�������Դ,1ƽ��1�����Դ,2ƽ��b�����Դ
int      judge_relative_position(const Plane &pa,const Plane &pb,const Vec3  &light_direction);
// AABB vs AABB test, returns true if objects intersect
bool IntersectionTest(const BoundingBox &objectBB, const BoundingBox &frustumBB);


//����������ϵ��,����ͶӰ��,���ĳһ�����ζ��������ĳһ������ͶӰ����׶����,������ǳɹ���
// AABB vs AABB sweep test, returns true if intersection can occur if object is translated along given direction
//vSweepDir:��Դ��������ķ���,����������ϵ��
bool SweepIntersectionTest(const BoundingBox &objectBB, const BoundingBox &frustumBB, const Vector3 &vSweepDir);

// Helper function for AABB vs frustum test
//����ͶӰ����,����������׶���Ƿ����ص�
//
bool ProjectedIntersection(const Vector3 &vHalfSize, const Vector3 &vCenter, const Vector3 *pFrustumPoints, const Vector3 &vDir);

// AABB vs Frustum test, returns true if objects intersect
//��׼����׶��ü�
bool IntersectionTest(const BoundingBox &objectBB, const Frustum &frustum);



// Helper function for AABB vs Frustum sweep test
//
//
inline bool ProjectedSweepIntersection(const Vector3 &vHalfSize, const Vector3 &vCenter,
	const Vector3 *pFrustumPoints,
	const Vector3 &vDir,
	float &fDistMin, float &fDistMax, const Vector3 &vSweepDir);

// AABB vs Frustum sweep test, returns true if intersection can occur if object is translated along given direction
bool SweepIntersectionTest(const BoundingBox &objectBB, const Frustum &frustum, const Vector3 &vSweepDir);