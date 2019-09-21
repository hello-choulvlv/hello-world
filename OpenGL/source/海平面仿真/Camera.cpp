/*
  *摄像机实现
  *@2018年3月16日
  *@author:xiaohuaxiong
*/
#include "Camera.h"
#define RADIANS_FACTOR (180.0f/M_PI)
Camera::Camera():
	_isViewChanged(false)
	, _isViewProjChanged(false)
	,_isInverseViewChanged(false)
{
	D3DXMatrixIdentity(&_viewMatrix);
	D3DXMatrixIdentity(&_viewProjMatrix);
}

void Camera::initPerspective(float fov, float aspect, float nearZ, float farZ)
{
	D3DXMatrixPerspectiveFovLH(&_projMatrix, fov/RADIANS_FACTOR, aspect, nearZ, farZ);
	_isViewProjChanged = true;
}

void Camera::initOrtho(float l, float r, float t, float b, float n, float f)
{
	D3DXMatrixOrthoOffCenterLH(&_projMatrix, l, r, b, t, n, f);
	_isViewProjChanged = true;
}

void Camera::lookAt(const D3DXVECTOR3 &eyePosition, const D3DXVECTOR3 &targetPosition)
{
	D3DXVECTOR3  upperVec(0,1,0);
	D3DXMatrixLookAtLH(&_viewMatrix, &eyePosition, &targetPosition,&upperVec);
	//_isViewChanged = true;
	_isViewProjChanged = true;
	_isInverseViewChanged = true;
	//计算roll + pitch
	_roll = atan2f(_viewMatrix.m[1][2], sqrtf(_viewMatrix.m[0][2] * _viewMatrix.m[0][2] +_viewMatrix.m[2][2]*_viewMatrix.m[2][2])) * RADIANS_FACTOR;
	_pitch = atan2f(-_viewMatrix.m[0][2],_viewMatrix.m[2][2]) * RADIANS_FACTOR;

	_eyePosition = eyePosition;
	_translateVec = -eyePosition;
}

Camera  *Camera::createPerspective(float fov, float aspect, float nearZ, float farZ)
{
	Camera *camera = new Camera();
	camera->initPerspective(fov, aspect, nearZ, farZ);
	return camera;
}

Camera *Camera::createOrtho(float l, float r, float t, float b, float n, float f)
{
	Camera *camera = new Camera();
	camera->initOrtho(l, r, t, b, n, f);
	return camera;
}

void Camera::rotate(float angleOfX, float angleOfY)
{
	if (angleOfX != 0 || angleOfY != 0)
	{
		_roll += angleOfX;
		_pitch += angleOfY;

		_isViewChanged = true;
		_isInverseViewChanged = true;
		_isViewProjChanged = true;
	}
}

void Camera::translate(float dx, float dz)
{
	if (dx != 0 || dz != 0)
	{
		//计算旋转矩阵
		auto &inverseMatrix = getInverseViewMatrix();
		float  x = -dx * inverseMatrix.m[0][0] - dz * inverseMatrix.m[2][0];
		float  y = -dx * inverseMatrix.m[0][1] - dz *  inverseMatrix.m[2][1];
		float  z = -dx * inverseMatrix.m[0][2] - dz *  inverseMatrix.m[2][2];
		//
		_translateVec.x += x;
		_translateVec.y += y;
		_translateVec.z += z;

		_eyePosition = -_translateVec;

		_isViewChanged = true;
		_isViewProjChanged = true;
		_isInverseViewChanged = true;
	}
}

const D3DXMATRIX& Camera::getViewMatrix()
{
	if (_isViewChanged)
	{
		D3DXMATRIX  xm, ym,tm;
		D3DXMatrixTranslation(&tm, _translateVec.x, _translateVec.y, _translateVec.z);
		D3DXMatrixRotationY(&ym, _pitch / RADIANS_FACTOR);
		D3DXMatrixRotationX(&xm, _roll/ RADIANS_FACTOR);
		_viewMatrix = tm   * ym * xm ;
		_isViewChanged = false;
	}
	return _viewMatrix;
}

const D3DXMATRIX& Camera::getProjMatrix()
{
	return _projMatrix;
}

const D3DXMATRIX& Camera::getViewProjMatrix()
{
	if (_isViewProjChanged)
	{
		auto &vm = getViewMatrix();
		_viewProjMatrix = vm * _projMatrix;
		_isViewProjChanged = false;
	}
	return _viewProjMatrix;
}

const D3DXMATRIX& Camera::getInverseViewMatrix()
{
	if (_isInverseViewChanged)
	{
		auto &vm = getViewMatrix();
		float det = 0;
		D3DXMatrixInverse(&_inverseViewMatrix, &det,&vm);
		_isInverseViewChanged = false;
	}
	return _inverseViewMatrix;
}
