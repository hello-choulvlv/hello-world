#pragma once

#include "BoundingBox.h"

class Frustum
{
public:
  // 8 corner points of frustum
  Vector3 m_pPoints[8];

  // computes AABB from corner points
  inline void CalculateAABB(void);

  // world space bounding box
  BoundingBox m_AABB;

  Plane _plane[6];// clip plane, left, right, top, bottom, near, far
};

// computes AABB vectors from corner points
inline void Frustum::CalculateAABB(void)
{
  m_AABB.Set(m_pPoints, 8, sizeof(Vector3));
}
