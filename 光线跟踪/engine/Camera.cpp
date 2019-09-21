/*
  *�����ʵ��
  *2017-7-19
  *@Author:xiaohuaxiong
  */
#include "Camera.h"
#include<math.h>
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

void   Camera::initCamera(const float_3 &eyePosition, const float_3 &targetPosition, const float_3 &upVec)
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
	float yaw = GLK_RADIUS_TO_ANGLE(atan2f(array[2][0], float_2(-array[2][1],array[2][2]).length()));
	//������ת����
	_rotateVec = float_3(pitch,yaw,0.0f);
	//����ƽ������ϵ,������Ҫ�����ͼ�����е���ת����������
	mat4x4 roatateM;
	roatateM.rotateX(-pitch);
	roatateM.rotateY(-yaw);
	//ע��ƽ�����������������ʵ�ʵ�����
	_translateVec = *(float_3*)&(float_4(array[3][0],array[3][1],array[3][2],0.0f) * roatateM);
	_isViewDirty = true;
	_isViewProjDirty = true;
	_isInverseViewProjDirty = true;
}

Camera *Camera::createCamera(const float_3 &eyePosition, const float_3 &targetPosition, const float_3 &upVec)
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

const mat4x4& Camera::getViewProjMatrix()
{
	if (_isViewProjDirty)
	{
		_viewProjMatrix = _viewMatrix * _projMatrix;
		_isViewProjDirty = false;
	}
	return _viewProjMatrix;
}

const mat4x4& Camera::getViewMatrix()const
{
	return _viewMatrix;
}

const mat4x4& Camera::getProjMatrix()const
{
	return _projMatrix;
}

const mat4x4& Camera::getInverseViewMatrix()
{
	if (_isViewDirty)
	{
		_inverseViewMatrix = _viewMatrix.reverse();
		_isViewDirty = false;
	}
	return _inverseViewMatrix;
}

const mat4x4& Camera::getInverseViewProjMatrix()
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
		 _viewProjMatrix.reverse(_inverseViewProjMatrix);
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
		float_4 afterRotateVec = float_4(-dx,0,-dz,0.0f)* getInverseViewMatrix();
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

const float_3& Camera::getCameraPosition()const
{
	return _eyePosition;
}
