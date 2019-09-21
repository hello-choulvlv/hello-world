/*
  *�����ʵ��
  *2017-7-19
  *@Author:xiaohuaxiong
  */
#include "engine/Camera.h"
#include<math.h>
__NS_GLK_BEGIN
Camera::Camera():
_cameraType(CameraType::CameraType_None)
,_isViewDirty(true)
,_isProjMatrixDirty(true)
, _isViewProjDirty(true)
, _isInverseViewProjDirty(true)
{

}

Camera::~Camera()
{

}

void   Camera::initCamera(const GLVector3 &eyePosition, const GLVector3 &targetPosition, const GLVector3 &upVec)
{
	//������ͼ����
	_viewMatrix.lookAt(eyePosition, targetPosition, upVec);
	_eyePosition = eyePosition;
	_targetPosition = targetPosition;
	//�ֽ���ͼ����,������㷨��μ�https://www.physicsforums.com/threads/decomposition-of-a-rotation-matrix.623740/
	typedef float(*MatrixArray)[4];
	MatrixArray array = (MatrixArray)_viewMatrix.pointer();
	//��X����ת�ĽǶ�
	float pitch = GLK_RADIUS_TO_ANGLE(atan2f(-array[2][1],array[2][2])) ;
	float yaw = GLK_RADIUS_TO_ANGLE(atan2f(array[2][0],GLVector2(-array[2][1],array[2][2]).length()));
	//������ת����
	_rotateVec = GLVector3(pitch,yaw,0.0f);
	//����ƽ������ϵ,������Ҫ�����ͼ�����е���ת����������
	Matrix roatateM;
	roatateM.rotateX(-pitch);
	roatateM.rotateY(-yaw);
	//ע��ƽ�����������������ʵ�ʵ�����
	_translateVec = (GLVector4(array[3][0],array[3][1],array[3][2],0.0f) * roatateM).xyz();
	_isViewDirty = true;
	_isViewProjDirty = true;
	_isInverseViewProjDirty = true;
}

Camera *Camera::createCamera(const GLVector3 &eyePosition, const GLVector3 &targetPosition, const GLVector3 &upVec)
{
	Camera *camera = new Camera();
	camera->initCamera(eyePosition, targetPosition, upVec);
	return camera;
}

void  Camera::setPerspective(const float angle, const float widthRate, const float nearZ, const float farZ)
{
	_projMatrix.identity();
	_projMatrix.perspective(angle, widthRate,nearZ,farZ);
	_cameraType = CameraType::CameraType_Perspertive;
	_isProjMatrixDirty = true;
	_isViewProjDirty = true;
	_isInverseViewProjDirty = true;
}

void Camera::setOrtho(float left, float right, float bottom, float top, float nearZ, float farZ)
{
	_projMatrix.identity();
	_projMatrix.orthoProject(left,right,bottom,top	,nearZ,farZ);
	_cameraType = CameraType::CameraType_Ortho;
	_isProjMatrixDirty = true;
	_isViewProjDirty = true;
	_isInverseViewProjDirty = true;
}

const Matrix& Camera::getViewProjMatrix()
{
	if (_isViewProjDirty)
	{
		_viewProjMatrix = _viewMatrix * _projMatrix;
		_isViewProjDirty = false;
	}
	return _viewProjMatrix;
}

const Matrix& Camera::getViewMatrix()const
{
	return _viewMatrix;
}

const Matrix& Camera::getProjMatrix()const
{
	return _projMatrix;
}

const Matrix& Camera::getInverseViewMatrix()
{
	if (_isViewDirty)
	{
		_inverseViewMatrix = _viewMatrix.reverse();
		_isViewDirty = false;
	}
	return _inverseViewMatrix;
}

const Matrix& Camera::getInverseViewProjMatrix()
{
	//�ж��Ƿ��������Ҫ����
	if (_isInverseViewProjDirty)
	{
		_isInverseViewProjDirty = false;
		if (_isViewProjDirty)//�����Ҫ��ǰ����,����Ƿ���ͼ����/ͶӰ�������˱仯
		{
			_isViewProjDirty = false;
			_viewProjMatrix = _viewMatrix * _projMatrix;
		}
		_inverseViewProjMatrix = _viewProjMatrix.reverse();
	}
	return _inverseViewProjMatrix;
}

void Camera::updateRotateMatrix(float dx, float dy)
{
	if (dx != 0 || dy != 0)
	{
		_rotateVec.x += dx;
		_rotateVec.y += dy;
		_viewMatrix.identity();
		_viewMatrix.translate(_translateVec);
		_viewMatrix.rotateY(_rotateVec.y);
		_viewMatrix.rotateX(_rotateVec.x);
		//����
		_isViewDirty = true;
		_isViewProjDirty = true;
		_isInverseViewProjDirty = true;
	}
}

void Camera::updateTranslateMatrix(float dx, float dz)
{
	if (dx != 0 || dz != 0)
	{
		GLVector4 afterRotateVec = GLVector4(-dx,0,-dz,0.0f)* getInverseViewMatrix();
		_translateVec.x += afterRotateVec.x;
		_translateVec.y += afterRotateVec.y;
		_translateVec.z += afterRotateVec.z;
		//���¹�����ͼ����,ͬʱ�����۾�������
		_viewMatrix.identity();
		_viewMatrix.translate(_translateVec);
		_viewMatrix.rotateY(_rotateVec.y);
		_viewMatrix.rotateX(_rotateVec.x);
		//
		_eyePosition.x = - _translateVec.x;
		_eyePosition.y = -_translateVec.y;
		_eyePosition.z = -_translateVec.z;
		//������������������
		_isViewDirty = true;
		_isViewProjDirty = true;
		_isInverseViewProjDirty = true;
	}
}

const GLVector3& Camera::getCameraPosition()const
{
	return _eyePosition;
}
__NS_GLK_END