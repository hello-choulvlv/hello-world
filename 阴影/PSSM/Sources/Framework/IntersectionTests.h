#pragma once
//新版视锥体裁剪算法
bool   intersect_test_new(const Frustum &frustum, const BoundingBox &aabb);
//光空间对摄像机空间视锥体的裁剪
bool   intersect_test_light(const Plane *light_planes, int length_l, const BoundingBox &aabb);
//生成视锥体的六个平面方程
void   generate_frustum_plane(const Frustum  &frustum,Plane  *planes);
//判断两个相交于一条线的平面相对光线方向的位置,0全部面向光源或者全部背离光源,1平面1面向光源,2平面b面向光源
int      judge_relative_position(const Plane &pa,const Plane &pb,const Vec3  &light_direction);
// AABB vs AABB test, returns true if objects intersect
bool IntersectionTest(const BoundingBox &objectBB, const BoundingBox &frustumBB);


//在世界坐标系中,查找投影物,如果某一个几何对象会沿着某一个方向投影到视锥体上,则测试是成功的
// AABB vs AABB sweep test, returns true if intersection can occur if object is translated along given direction
//vSweepDir:光源到几何体的方向,在世界坐标系中
bool SweepIntersectionTest(const BoundingBox &objectBB, const BoundingBox &frustumBB, const Vector3 &vSweepDir);

// Helper function for AABB vs frustum test
//沿着投影方向,几何体与视锥体是否有重叠
//
bool ProjectedIntersection(const Vector3 &vHalfSize, const Vector3 &vCenter, const Vector3 *pFrustumPoints, const Vector3 &vDir);

// AABB vs Frustum test, returns true if objects intersect
//标准的视锥体裁剪
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