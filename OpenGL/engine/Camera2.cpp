/*
  *另一种摄像机的实现,此实现与常规的摄像机实现相比有着更为严格的约束
  *@date:2017年11月26日
  *@Author:xiaohuaxiong
 */
#include "engine/Camera2.h"
__NS_GLK_BEGIN
Camera2::Camera2() :
	_upVector(0,1,0)
	,_forwardZ(0,0,-1)
	,_isViewProjDirty(false)
	,_isViewProjInverseDirty(false)
{

}

void Camera2::initWithPerspective(float fov, float whrate, float nearZ, float farZ)
{
	Matrix::createPerspective(fov, whrate, nearZ, farZ, _projMatrix);
	_nearFarFovRatioVec.x = nearZ, _nearFarFovRatioVec.y = farZ, _nearFarFovRatioVec.z = fov,_nearFarFovRatioVec.w = whrate;
	_isViewProjDirty = true;
	_isViewProjInverseDirty = true;
}

void Camera2::initWithOrtho(float lx, float rx, float by, float ty, float nearZ, float farZ)
{
	Matrix::createOrtho(lx, rx, by, ty, nearZ, farZ, _projMatrix);
	_nearFarFovRatioVec.x = nearZ, _nearFarFovRatioVec.y = farZ;
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

void Camera2::lookAt(const GLVector3 &eyePosition, const GLVector3 &targetPosition)
{
	_eyePosition = eyePosition;
	_Z = (eyePosition - targetPosition).normalize();
	GLVector3::generateViewXY(_Z,_X,_Y);
	updateViewMatrix();

	_targetPosition = _eyePosition - _Z*0.05f;
	_isViewProjDirty = true;
	_isViewProjInverseDirty = true;
	_forwardZ = _upVector.cross(_X);
}

const Matrix& Camera2::getViewProjMatrix()
{
	if (_isViewProjDirty)
	{
		_viewProjMatrix.multiply(_viewMatrix, _projMatrix);
		_isViewProjDirty = false;
	}
	return _viewProjMatrix;
}

const Matrix& Camera2::getViewProjReversematrix()
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

void Camera2::translate(const GLVector3 &offset)
{
	_eyePosition += offset;
	_targetPosition += offset;
	updateViewMatrix();
	_isViewProjDirty = true;
	_isViewProjInverseDirty = true;
}
/*
 *重量级函数
 */
void Camera2::rotate(float xOffset, float yOffset)
{
	bool cameraChanged = false;
	GLVector3   z_axis = _eyePosition - _targetPosition;
	if (xOffset != 0)
	{
		//旋转的时候直接绕+Y轴旋转
		Matrix3 rotateMatrix;
		rotateMatrix.rotate(xOffset,GLVector3(0,1,0));
		//_Y轴不一定是向上垂直的
		_X = _X*rotateMatrix;
		_Y = _Y*rotateMatrix;
		_Z = _Z*rotateMatrix;
		cameraChanged = true;
	}
	if (yOffset != 0)
	{
		Matrix3 rotateM;
		rotateM.rotate(yOffset, _X);
		_Y = _Y*rotateM;
		_Z = _Z*rotateM;
		//修正
		if (_Y.y < 0 )
		{
			_Z = GLVector3(0,_Z.y>0?1:-1,0);
			_Y = _Z.cross(_X);
		}
		cameraChanged = true;
	}
	if (cameraChanged)
	{
		//_eyePosition = _targetPosition + _Z * z_axis.length();
		updateViewMatrix();
		_isViewProjDirty = true;
		_isViewProjInverseDirty = true;
		_targetPosition = _eyePosition - _Z*0.05f;
		//
		_forwardZ = _upVector.cross(_X);
	}
}

__NS_GLK_END