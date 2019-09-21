/*
  *另一种摄像机的实现,此实现与常规的摄像机实现相比有着更为严格的约束
  *@date:2017年11月26日
  *@Author:xiaohuaxiong
 */
#include "Camera2.h"
#include<math.h>
#include<windows.h>
static mat4x4 s_BiaseMatrix(float_4( 2.0f, 0.0f, 0.0f, 0.0f),
													 float_4(0.0f, 2.0f, 0.0f, 0.0f),
													float_4(0.0f, 0.0f, 2.0f, 0.0f),
													float_4( -1.0f, -1.0f, -1.0f, 1.0f ));
static mat4x4f s_BiaseMatrixf(vec4(2.0f, 0.0f, 0.0f, 0.0f),
	vec4(0.0f, 2.0f, 0.0f, 0.0f),
	vec4(0.0f, 0.0f, 2.0f, 0.0f),
	vec4(-1.0f, -1.0f, -1.0f, 1.0f));
Camera2::Camera2() :
	_upVector(0,1,0)
	,_forwardZ(0,0,-1)
	,_isViewProjDirty(false)
	,_isViewProjInverseDirty(false)
{

}

void Camera2::initWithPerspective(float fov, float whrate, float nearZ, float farZ)
{
	mat4x4::createPerspective(fov, whrate, nearZ, farZ, _projMatrix);
	_isViewProjDirty = true;
	_isViewProjInverseDirty = true;
}

void Camera2::initWithOrtho(float lx, float rx, float by, float ty, float nearZ, float farZ)
{
	mat4x4::createOrtho(lx, rx, by, ty, nearZ, farZ, _projMatrix);
	_isViewProjDirty = true;
	_isViewProjInverseDirty = true;
}

Camera2 *Camera2::createWithPerspective(float fov, float whrate, float nearZ, float farZ)
{
	Camera2 *camera = new Camera2();
	camera->initWithPerspective(fov, whrate, nearZ, farZ);
	return camera;
}

Camera2 *Camera2::createWithOrtho(float lx, float rx, float by, float ty, float nearZ, float farZ)
{
	Camera2 *camera = new Camera2();
	camera->initWithOrtho(lx, rx, by, ty, nearZ, farZ);
	return camera;
}

void Camera2::lookAt(const float_3 &eyePosition, const float_3 &targetPosition)
{
	_eyePosition = eyePosition;
	_Z = (eyePosition - targetPosition).normalize();
	float_3::generateViewXY(_Z,_X,_Y);
	updateViewMatrix();

	_targetPosition = targetPosition;
	_isViewProjDirty = true;
	_isViewProjInverseDirty = true;
	_forwardZ = _upVector.cross(_X);
}

const mat4x4& Camera2::getViewProjMatrix()
{
	if (_isViewProjDirty)
	{
		_viewProjMatrix.multiply(_viewMatrix, _projMatrix);
		_isViewProjDirty = false;
	}
	return _viewProjMatrix;
}

const mat4x4& Camera2::getViewProjReverseMatrix()
{
	if (_isViewProjInverseDirty)
	{
		_isViewProjInverseDirty = false;
		_isViewProjDirty = false;
		_viewProjMatrix.multiply(_viewMatrix, _projMatrix);
		_viewProjMatrix.reverse(_viewProjMatrixInverse);
	}
	return _viewProjMatrixInverse;
}

/*
  *构造视图矩阵,
  *为了能快速计算矩阵数据,这里直接操纵了矩阵中的数据单元
 */
void Camera2::updateViewMatrix()
{
	typedef float (*marray)[4];
	marray m = (marray)_viewMatrix.pointer();

	m[0][0] = _X.x; m[0][1] = _Y.x; m[0][2] = _Z.x;
	m[1][0] = _X.y;m[1][1] = _Y.y;m[1][2] = _Z.y;
	m[2][0] = _X.z;m[2][1] = _Y.z;m[2][2] = _Z.z;
	m[0][3] = 0; m[1][3] = 0; m[2][3] = 0;

	m[3][0] = -_X.dot(_eyePosition);
	m[3][1] = -_Y.dot(_eyePosition);
	m[3][2] = -_Z.dot(_eyePosition);
	m[3][3] = 1;
}

const mat4x4& Camera2::getRayMatrix()const
{
	return _rayMatrix;
}

void Camera2::translate(const float_3 &offset)
{
	_eyePosition += offset;
	_targetPosition += offset;
	updateViewMatrix();
	_isViewProjDirty = true;
	_isViewProjInverseDirty = true;
}

void Camera2::rotate2(float xOffset, float yOffset)
{
	float sensitivity = 0.25f;

	float hangle = xOffset * sensitivity;
	float vangle = yOffset * sensitivity;

	_eyePosition -= _targetPosition;

	_Y = rotate(_Y, vangle, _X);
	_Z = rotate(_Z, vangle, _X);

	if (_Y.y < 0.0f)
	{
		_Z = float_3(0.0f, _Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
		_Y = _Z.cross(_X);
	}

	_X = rotate(_X, hangle, float_3(0.0f, 1.0f, 0.0f));
	_Y = rotate(_Y, hangle, float_3(0.0f, 1.0f, 0.0f));
	_Z = rotate(_Z, hangle, float_3(0.0f, 1.0f, 0.0f));

	_eyePosition = _targetPosition + _Z * _eyePosition.length();
	updateRayMatrix();
}
/*
 *重量级函数
 */
//void Camera2::rotate(float xOffset, float yOffset)
//{
//	bool cameraChanged = false;
//	_eyePosition -= _targetPosition;
//	float sensitivity = 1.5f;
//	float hangle = xOffset * sensitivity;
//	float vangle = yOffset * sensitivity;
//	bool camera_changed = false;
//	if (xOffset != 0)
//	{
//		mat3x3    rotate_matrix(vangle, _X);
//		_Y *= rotate_matrix;
//		_Z *= rotate_matrix;
//
//		if (_Y.y < 0.0f)
//		{
//			_Z = float_3(0.0f, _Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
//			_Y = _Z.cross(_X);
//		}
//		camera_changed = true;
//	}
//	if (yOffset != 0)
//	{
//		mat3x3  rotate_matrix2(hangle, float_3(0, 1, 0));
//		_X *= rotate_matrix2;
//		_Y *= rotate_matrix2;
//		_Z *= rotate_matrix2;
//		camera_changed = true;
//	}
//	_eyePosition = _targetPosition + _Z * _eyePosition.length();
//
//	if (camera_changed)
//	{
//		updateViewMatrix();
//		_isViewProjDirty = true;
//		_isViewProjInverseDirty = true;
//		//
//		_forwardZ = _upVector.cross(_X);
//	}
//}

void Camera2::updateViewportMatrix(float width, float height)
{
	float *m = (float*)&_viewportMatrix;
	m[0] = 1.0f / width;
	m[5] = 1.0f / height;
}

void Camera2::updateRayProjMatrix(float fov, float aspect)
{
	float tany = tanf(fov/360.0f*3.141592653f);
	float *m = (float*)&_rayProjMatrix;
	m[0] = tany * aspect;
	m[5] = tany;
	m[10] = 0.0f;
	m[14] = -1.0f;
}

void Camera2::updateRayMatrix()
{
	float *m = (float*)&_rayViewMatrix;
	m[0] = _X.x; m[1] = _X.y;m[2] = _X.z; 
	m[4] = _Y.x;m[5] = _Y.y;m[6] = _Y.z;
	m[8] = _Z.x;m[9] = _Z.y;m[10] = _Z.z;
	//RayMatrix = Vin * Pin * BiasMatrixInverse * VPin;
	_rayMatrix = _viewportMatrix * s_BiaseMatrix * _rayProjMatrix * _rayViewMatrix;
}

Camera::Camera()
{
	X = vec3(1.0, 0.0, 0.0);
	Y = vec3(0.0, 1.0, 0.0);
	Z = vec3(0.0, 0.0, 1.0);

	Reference = vec3(0.0, 0.0, 0.0);
	Position = vec3(0.0, 0.0, 5.0);
}

Camera::~Camera()
{
}

void Camera::CalculateRayMatrix()
{
	float *V = (float*)&Vin;
	V[0] = X.x; V[4] = Y.x; V[8] = Z.x;
	V[1] = X.y; V[5] = Y.y; V[9] = Z.y;
	V[2] = X.z; V[6] = Y.z; V[10] = Z.z;

	RayMatrix = Vin * Pin * s_BiaseMatrixf * VPin;
	//RayMatrix = VPin * s_BiaseMatrix * Pin*Vin;
}

void Camera::Look(const vec3 &Position, const vec3 &Reference, bool RotateAroundReference)
{
	this->Reference = Reference;
	this->Position = Position;

	Z = normalize(Position - Reference);
	X = normalize(cross(vec3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);

	if (!RotateAroundReference)
	{
		this->Reference = this->Position;
		this->Position += Z * 0.05f;
	}

	CalculateRayMatrix();
}

bool Camera::OnKeyDown(UINT nChar)
{
	float Distance = 0.125f;

	if (GetKeyState(VK_CONTROL) & 0x80) Distance *= 0.5f;
	if (GetKeyState(VK_SHIFT) & 0x80) Distance *= 2.0f;

	vec3 Up(0.0f, 1.0f, 0.0f);
	vec3 Right = X;
	vec3 Forward = cross(Up,Right);

	Up *= Distance;
	Right *= Distance;
	Forward *= Distance;

	vec3 Movement;

	if (nChar == 'W') Movement += Forward;
	if (nChar == 'S') Movement -= Forward;
	if (nChar == 'A') Movement -= Right;
	if (nChar == 'D') Movement += Right;
	if (nChar == 'R') Movement += Up;
	if (nChar == 'F') Movement -= Up;

	Reference += Movement;
	Position += Movement;

	return Movement.x != 0.0f || Movement.y != 0.0f || Movement.z != 0.0f;
}

void Camera::OnMouseMove(int dx, int dy)
{
	float sensitivity = 0.25f;

	float hangle = (float)dx * sensitivity;
	float vangle = (float)dy * sensitivity;

	Position -= Reference;

	Y = rotate(Y, vangle, X);
	Z = rotate(Z, vangle, X);

	if (Y.y < 0.0f)
	{
		Z = vec3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
		Y = cross(Z,X);
	}

	X = rotate(X, hangle, vec3(0.0f, 1.0f, 0.0f));
	Y = rotate(Y, hangle, vec3(0.0f, 1.0f, 0.0f));
	Z = rotate(Z, hangle, vec3(0.0f, 1.0f, 0.0f));

	Position = Reference + Z * length(Position);

	CalculateRayMatrix();
}

void Camera::OnMouseWheel(short zDelta)
{
	Position -= Reference;

	if (zDelta < 0 && length(Position) < 500.0f)
	{
		Position += Position * 0.1f;
	}

	if (zDelta > 0 && length(Position) > 0.05f)
	{
		Position -= Position * 0.1f;
	}

	Position += Reference;
}