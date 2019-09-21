#include "Common.h"
#include "Light.h"
#include "SceneObject.h"
#include "IntersectionTests.h"
#include "Application.h"
#include "DemoSetup.h"
#include<assert.h>

#define min_f(x,y) (x) <= (y) ?(x):(y)
#define max_f(x,y) (x ) >= (y)?(x):(y)

Light::Light():
	_isViewDirty(true)
	,_isViewProjDirty(true)
	,_isLightParamChanged(true)
{
	m_Type = TYPE_PERSPECTIVE;// TYPE_ORTHOGRAPHIC;
  m_vUpVector = Vector3(0, 1, 0);
  m_fAspectRatio = 1.0f;
  m_vSource = Vector3(0, 200, 0);
  m_vTarget = Vector3(0, 0, 0);
  m_fNear = 50.0f;
  m_fFar = 400.0f;
  m_fFOV = DegreeToRadian(90.0f);
  m_vLightDiffuse = Vector3(0.7f,0.7f,0.7f);
  m_vLightAmbient = Vector3(0.25f,0.25f,0.25f);

  ZeroMemory(&m_ControlState, sizeof(ControlState));
  m_ControlState.m_vRotation = Vector3(-DegreeToRadian(130.0f), -DegreeToRadian(35.0f),0);
}

// processes light controls
void Light::DoControls(void)
{
  float fDeltaTime = DeltaTimeUpdate(m_ControlState.m_fLastUpdate);

  // Rotate light
  if (GetKeyDown(VK_LEFT))
  {
	  m_ControlState.m_vRotation.x += 0.02f * fDeltaTime;
	  _isLightParamChanged = true;
  }
  else if (GetKeyDown(VK_RIGHT)) 
  {
	  m_ControlState.m_vRotation.x -= 0.02f * fDeltaTime;
	  _isLightParamChanged = true;
  }
  if(GetKeyDown(VK_UP))
 {
	  m_ControlState.m_vRotation.y += 0.01f * fDeltaTime;
	  _isLightParamChanged = true;
  }
  else if (GetKeyDown(VK_DOWN))
  {
	  m_ControlState.m_vRotation.y -= 0.01f * fDeltaTime;
	  _isLightParamChanged = true;
  }
  if (_isLightParamChanged)
  {
	  m_ControlState.m_vRotation.y = Clamp(m_ControlState.m_vRotation.y, DegreeToRadian(-89.9f), DegreeToRadian(0.0f));
	  //与+Z轴的夹角
	  float ch = cosf(m_ControlState.m_vRotation.x);
	  float sh = sinf(m_ControlState.m_vRotation.x);
	  //绕+X轴旋转,角度的测量规则为与-Y轴的夹角
	  float cp = cosf(m_ControlState.m_vRotation.y);
	  float sp = sinf(m_ControlState.m_vRotation.y);
	  Vector3 vDist = m_vTarget - m_vSource;
	  //球面坐标系,注意,光源使用的观察算法与摄像机截然不同,摄像机只改变目标的位置,而光源只改变自身的位置,目标不会变化
	  m_vSource = m_vTarget + Vector3(sh*cp, -sp, cp*ch) * vDist.Length();
  }
  // Switch light type
  //
  if(GetKeyDown('T'))
  {
    if(!m_ControlState.m_bSwitchingType)
    {
		m_Type = (LightType)(1 - m_Type);// (m_Type == Light::TYPE_ORTHOGRAPHIC) ? Light::TYPE_PERSPECTIVE : Light::TYPE_ORTHOGRAPHIC;
      m_ControlState.m_bSwitchingType = true;
    }
	_isLightParamChanged = true;
  }
  else
  {
    m_ControlState.m_bSwitchingType = false;
  }
  if (_isLightParamChanged)
  {
	  CalculateMatrices();

	  _isLightParamChanged = false;
	  _isViewDirty = false;
	  _isViewProjDirty = false;
  }
}

// calculates default light matrices
void Light::CalculateMatrices(void)
{
  // view matrix
  m_mView = MatrixLookAtLH(m_vSource, m_vTarget, m_vUpVector);

  // projection matrix
  if(m_Type == TYPE_PERSPECTIVE)
  {
    m_mProj = MatrixPerspectiveFovLH(m_fFOV, m_fAspectRatio, m_fNear, m_fFar);
  }
  else
  {
    // this is just a funny way to calculate a size for the light using FOV
    float fFarPlaneSize = 2 * tanf(m_fFOV * 0.5f) * m_fFar;
    m_mProj = MatrixOrthoLH(fFarPlaneSize * m_fAspectRatio, fFarPlaneSize, m_fNear, m_fFar);
  }

  _viewProjMatrix = m_mView * m_mProj;
}

void  Light::calculate_light_space_frustum(const Frustum  &world_frustum, Frustum &light_frustum,LightType  light_type)
{
	const Vector3  *points = world_frustum.m_pPoints;
	//将所有的顶点转换到光空间中
	float   vertex_array[8 * 3];
	Vec3  *vertex = (Vec3 *)vertex_array;
	Vec3   aabb_min, aabb_max;
	for (int idx_j = 0; idx_j < 8; ++idx_j)
	{
		vec3_multiply_matrix(points[idx_j], m_mView, vertex + idx_j);
		if (!idx_j)
		{
			aabb_min = vertex[idx_j];
			aabb_max = vertex[idx_j];
		}
		else
		{
			aabb_min.x = min_f(aabb_min.x, vertex[idx_j].x);
			aabb_min.y = min_f(aabb_min.y,vertex[idx_j].y);
			aabb_min.z = min_f(aabb_min.z, vertex[idx_j].z);

			aabb_max.x = max_f(aabb_max.x,vertex[idx_j].x);
			aabb_max.y = max_f(aabb_max.y, vertex[idx_j].y);
			aabb_max.z = max_f(aabb_max.z, vertex[idx_j].z);
		}
	}
	//计算最佳正交矩阵
	Matrix  ortho_matrix;
	if (light_type == TYPE_ORTHOGRAPHIC)//正交投影
		create_ortho_matrix(ortho_matrix, aabb_min.x, aabb_max.x, aabb_min.y, aabb_max.y, 0, aabb_max.z);
	else//透视投影,注意计算平截头体的过程要稍微复杂一点
	{
		//float s = 1.0f;// m_fNear / aabb_min.z;
		//create_frustum_matrix(ortho_matrix, aabb_min.x * s, aabb_max.x * s, aabb_min.y * s, aabb_max.y * s, m_fNear, aabb_max.z);
	}
	Matrix view_ortho_matrix = m_mView * ortho_matrix;

	Plane  *plane = light_frustum._plane;
	const float *matrix_array = (float *)view_ortho_matrix.m;
	plane[0].initPlane(-Vec3(matrix_array[3] + matrix_array[0], matrix_array[7] + matrix_array[4], matrix_array[11] + matrix_array[8]), (matrix_array[15] + matrix_array[12]));//left
	plane[1].initPlane(-Vec3(matrix_array[3] - matrix_array[0], matrix_array[7] - matrix_array[4], matrix_array[11] - matrix_array[8]), (matrix_array[15] - matrix_array[12]));//right
	plane[2].initPlane(-Vec3(matrix_array[3] + matrix_array[1], matrix_array[7] + matrix_array[5], matrix_array[11] + matrix_array[9]), (matrix_array[15] + matrix_array[13]));//bottom
	plane[3].initPlane(-Vec3(matrix_array[3] - matrix_array[1], matrix_array[7] - matrix_array[5], matrix_array[11] - matrix_array[9]), (matrix_array[15] - matrix_array[13]));//top
	plane[4].initPlane(-Vec3(matrix_array[2], matrix_array[6], matrix_array[10]), matrix_array[14]);//near
	plane[5].initPlane(-Vec3(matrix_array[3] - matrix_array[2], matrix_array[7] - matrix_array[6], matrix_array[11] - matrix_array[10]), (matrix_array[15] - matrix_array[14]));//far
}

void  Light::calculate_light_space_frustum_plane(const Frustum &world_frustum,const Vec3 &light_direction,Plane *planes, int &plane_length)
{
	const Vec3  *points = world_frustum.m_pPoints;
	int     index_l = 0;
	//生成视锥体的平面方程
	generate_frustum_plane(world_frustum, _frustum_planes);
	//判断由视锥体的各个边与光线的方向形成的半空间约束投影体
	static const short edge_index_array[24] = {//视锥体的12条边,按照所在的视锥体平面的顺时针方向设定
		1,2,2,3,3,0,0,1,//near
		5,6,6,7,7,4,4,5,//far
		1,5,2,6,3,7,0,4,//interval
	};
	//两个平面的交集形成的边,边对于第一个平面是顺时针,对于第二个平面是逆时针
	//0 near
	//1 far
	//2 left
	//3 right
	//4 bottom
	//5 top
	static const short plane_index_array[24] = {
		0,5,		0,3,		0,4,		0,2,//near
		5,1,		3,1,		4,1,		2,1,//far
		5,2,		3,5,		4,3,		2,4,//interval
	};
	//judge
	for (int j = 0; j < 12; ++j)
	{
		int  base_j = j * 2;
		int  relative_position = judge_relative_position(_frustum_planes[plane_index_array[base_j]],_frustum_planes[plane_index_array[base_j + 1]],light_direction);
		if (relative_position > 0)
		{
			int   start_idx = relative_position == 1 ? edge_index_array[base_j]: edge_index_array[base_j + 1];
			int  final_idx = relative_position == 1 ? edge_index_array[base_j + 1]: edge_index_array[base_j];

			planes[index_l++].initPlane(Cross(points[final_idx] - points[start_idx], light_direction),points[start_idx]);
		}
	}
	//另外加上,如果视锥体的某一个平面的法线与光线的方向相反,则其是一个半空间约束
	for (int j = 0; j < 6; ++j)
	{
		if (Dot(_frustum_planes[j].getNormal(), light_direction) < 0)
			planes[index_l ++] = _frustum_planes[j];
	}
	plane_length = index_l;
}

// finds scene objects that overlap given frustum from light's view
std::vector<SceneObject *> Light::FindCasters(const Frustum &frustum)
{
  Vector3 vDir = Normalize(m_vTarget - m_vSource);

  std::vector<SceneObject *> casters;
  casters.reserve(g_SceneObjects.size());
  int  plane_length = 0;
  //对于正交投影矩阵,计算光空间下的视图投影矩阵
  //calculate_light_space_frustum(frustum, _light_frustum, TYPE_ORTHOGRAPHIC);
  calculate_light_space_frustum_plane(frustum, -vDir, _light_planes, plane_length);
  for(unsigned int i=0; i<g_SceneObjects.size(); i++)
  {
    SceneObject *pObject = g_SceneObjects[i];
    if(pObject->m_bOnlyReceiveShadows) continue;

	if (i == 24)
	{
		int x = 0;
		int y = 0;
	}
    // do intersection test
    // orthogonal light
    if(m_Type == TYPE_ORTHOGRAPHIC)
    {
      // use sweep intersection
      if(g_iVisibilityTest == VISTEST_ACCURATE) {
        // test accurately
        //if(!SweepIntersectionTest(pObject->m_AABB, frustum, vDir)) continue;
		  //if (!intersect_test_new(_light_frustum, pObject->m_AABB))
			//  continue;
		  if (!intersect_test_light(_light_planes, plane_length, pObject->m_AABB))continue;
      } else if(g_iVisibilityTest == VISTEST_CHEAP) {
        // test only with AABB of frustum
        if(!SweepIntersectionTest(pObject->m_AABB, frustum.m_AABB, vDir)) continue;
      }
    }
    // perspective light
    else if(m_Type == TYPE_PERSPECTIVE)
    {
      // the same kind of sweep intersection doesn't really work here, but we can
      // approximate it by using the direction to center of AABB as the sweep direction
      // (note that sometimes this will fail)
      Vector3 vDirToCenter = Normalize(((pObject->m_AABB.m_vMax + pObject->m_AABB.m_vMin) * 0.5f) - m_vSource);
      if(g_iVisibilityTest == VISTEST_ACCURATE)
      {
        // test accurately
        //if(!SweepIntersectionTest(pObject->m_AABB, frustum, vDirToCenter)) continue;
		  //if (!intersect_test_new(_light_frustum, pObject->m_AABB)) continue;
		  if (!intersect_test_light(_light_planes, plane_length, pObject->m_AABB))continue;
      } else if(g_iVisibilityTest == VISTEST_CHEAP) {
        // test only with AABB of frustum
        if(!SweepIntersectionTest(pObject->m_AABB, frustum.m_AABB, vDirToCenter)) continue;
      }
    }

    casters.push_back(pObject);
  }
  return casters;
}

// build a matrix for cropping light's projection
// given vectors are in light's clip space
inline Matrix Light::BuildCropMatrix(const Vector3 &vMin, const Vector3 &vMax)
{
  float fScaleX, fScaleY, fScaleZ;
  float fOffsetX, fOffsetY, fOffsetZ;

  fScaleX = 2.0f / (vMax.x - vMin.x);
  fScaleY = 2.0f / (vMax.y - vMin.y);

  fOffsetX = -0.5f * (vMax.x + vMin.x) * fScaleX;
  fOffsetY = -0.5f * (vMax.y + vMin.y) * fScaleY;
  //DX引擎,NDC坐标系中Z分量的范围为[0-1]
  fScaleZ = 1.0f / (vMax.z - vMin.z);//近平面与原点对齐
  fOffsetZ = -vMin.z * fScaleZ;
  //fOffsetZ = -0.5f*(vMin.z+vMax.z)*fScaleZ;
  // crop volume matrix
  //将物体的边缘与视锥体对齐
  return Matrix(   fScaleX,     0.0f,     0.0f,   0.0f,
                          0.0f,  fScaleY,     0.0f,   0.0f,
                          0.0f,     0.0f,  fScaleZ,   0.0f,
                      fOffsetX, fOffsetY, fOffsetZ,   1.0f  );
}

// helper function for computing AABB in clip space
inline BoundingBox CreateClipSpaceAABB(const BoundingBox &bb, const Matrix &mViewProj)
{
  Vector4 vTransformed[8];
  // for each point
  for(int i=0;i<8;i++)
  {
    // transform to projection space
    vTransformed[i] = Transform(bb.m_pPoints[i], mViewProj);

    // compute clip-space coordinates
    vTransformed[i].x /= vTransformed[i].w;
    vTransformed[i].y /= vTransformed[i].w;
    vTransformed[i].z /= vTransformed[i].w;
  }

  return BoundingBox(vTransformed, 8, sizeof(Vector4));
}

// crops the light volume on given frustum (scene-independent projection)
Matrix Light::CalculateCropMatrix(const Frustum &frustum)
{
	//Matrix mViewProj = m_mView * m_mProj;
	// find boundaries in light clip space
	BoundingBox cropBB = CreateClipSpaceAABB(frustum.m_AABB, _viewProjMatrix);

	// use default near plane
	//cropBB.m_vMin.z = 0.0f;

	// finally, create matrix
	return BuildCropMatrix(cropBB.m_vMin, cropBB.m_vMax);
}

// crops the light volume on given objects, constrained by given frustum
Matrix Light::CalculateCropMatrix(const std::vector<SceneObject *> &casters, const std::vector<SceneObject *> &receivers, const Frustum &frustum)
{
  if(!g_bUseSceneDependentProjection) return CalculateCropMatrix(frustum);

  const Matrix& mViewProj = _viewProjMatrix;// m_mView * m_mProj;//使用了光空间的视图,投影矩阵.

  // bounding box limits
  BoundingBox receiversBB, splitBB, castersBB;

  // for each caster
  // find boundaries in light clip space
  for(unsigned int i = 0; i < casters.size(); i++)
    castersBB.Union(CreateClipSpaceAABB(casters[i]->m_AABB, mViewProj));

  // for each receiver
  // find boundaries in light clip space
  for(unsigned int i = 0; i < receivers.size(); i++)
  {
    receiversBB.Union(CreateClipSpaceAABB(receivers[i]->m_AABB, mViewProj));
  }
  //assert
  assert(castersBB.m_vMin.x >= receiversBB.m_vMin.x && castersBB.m_vMin.y >= receiversBB.m_vMin.y && castersBB.m_vMax.x <= receiversBB.m_vMax.x && castersBB.m_vMax.y <= receiversBB.m_vMax.y);
  // find frustum boundaries in light clip space
  splitBB = CreateClipSpaceAABB(frustum.m_AABB, mViewProj);

  // next we will merge the bounding boxes
  //
  BoundingBox cropBB;
  cropBB.m_vMin.x = Max(Max(castersBB.m_vMin.x, receiversBB.m_vMin.x), splitBB.m_vMin.x);
  cropBB.m_vMax.x = Min(Min(castersBB.m_vMax.x, receiversBB.m_vMax.x), splitBB.m_vMax.x);
  cropBB.m_vMin.y = Max(Max(castersBB.m_vMin.y, receiversBB.m_vMin.y), splitBB.m_vMin.y);
  cropBB.m_vMax.y = Min(Min(castersBB.m_vMax.y, receiversBB.m_vMax.y), splitBB.m_vMax.y);
  cropBB.m_vMin.z = castersBB.m_vMin.z;
  cropBB.m_vMax.z = Min(receiversBB.m_vMax.z, splitBB.m_vMax.z);

  // when there are no casters, the merged
  // bounding box will be infinitely small
  if(casters.size() == 0)
  {
    // it will cause artifacts when rendering receivers,
    // so just use the frustum bounding box instead
    cropBB.m_vMin = splitBB.m_vMin;
    cropBB.m_vMax = splitBB.m_vMax;
  }

  // finally, create matrix
  return BuildCropMatrix(cropBB.m_vMin, cropBB.m_vMax);
}
//计算Crop矩阵
void  Light::CalculateReceiverBoundingBox(const std::vector<SceneObject *> &receivers, BoundingBox &receiversBB)
{
	//Matrix mViewProj = m_mView * m_mProj;//使用了光空间的视图,投影矩阵.
	for (unsigned int i = 0; i < receivers.size(); i++)
	{
		receiversBB.Union(CreateClipSpaceAABB(receivers[i]->m_AABB, _viewProjMatrix));
	}
}
// crops the light volume on given objects, constrained by given frustum
Matrix Light::CalculateCropMatrix(const std::vector<SceneObject *> &casters, const BoundingBox &receiversBB,const Frustum &frustum)
{
	if (!g_bUseSceneDependentProjection) return CalculateCropMatrix(frustum);

	const Matrix& mViewProj = _viewProjMatrix;// m_mView * m_mProj;//使用了光空间的视图,投影矩阵.

										 // bounding box limits
	BoundingBox splitBB, castersBB;

	// for each caster
	// find boundaries in light clip space
	for (unsigned int i = 0; i < casters.size(); i++)
		castersBB.Union(CreateClipSpaceAABB(casters[i]->m_AABB, mViewProj));

	//assert
	//assert(castersBB.m_vMin.x >= receiversBB.m_vMin.x && castersBB.m_vMin.y >= receiversBB.m_vMin.y && castersBB.m_vMax.x <= receiversBB.m_vMax.x && castersBB.m_vMax.y <= receiversBB.m_vMax.y);
	// find frustum boundaries in light clip space
	splitBB = CreateClipSpaceAABB(frustum.m_AABB, mViewProj);

	// next we will merge the bounding boxes
	//
	BoundingBox cropBB;
	cropBB.m_vMin.x = Max(Max(castersBB.m_vMin.x, receiversBB.m_vMin.x), splitBB.m_vMin.x);
	cropBB.m_vMax.x = Min(Min(castersBB.m_vMax.x, receiversBB.m_vMax.x), splitBB.m_vMax.x);
	cropBB.m_vMin.y = Max(Max(castersBB.m_vMin.y, receiversBB.m_vMin.y), splitBB.m_vMin.y);
	cropBB.m_vMax.y = Min(Min(castersBB.m_vMax.y, receiversBB.m_vMax.y), splitBB.m_vMax.y);
	cropBB.m_vMin.z = castersBB.m_vMin.z;
	cropBB.m_vMax.z = Min(receiversBB.m_vMax.z, splitBB.m_vMax.z);

	// when there are no casters, the merged
	// bounding box will be infinitely small
	if (casters.size() == 0)
	{
		// it will cause artifacts when rendering receivers,
		// so just use the frustum bounding box instead
		cropBB.m_vMin = splitBB.m_vMin;
		cropBB.m_vMax = splitBB.m_vMax;
	}

	// finally, create matrix
	return BuildCropMatrix(cropBB.m_vMin, cropBB.m_vMax);
}
// returns direction of light
Vector3 Light::GetDir(void)
{
  return Normalize(m_vTarget - m_vSource);
}
