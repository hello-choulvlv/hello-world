/*
  *DirectX摄像机
  *2018年3月16日
  *@version: 1.0
  *@author:xiaohuaxiong
 */
#ifndef __CAMERA_H__
#define __CAMERA_H__
#include<d3dx10math.h>
class Camera
{
	//摄像机眼睛的位置
	D3DXVECTOR3   _eyePosition;
	D3DXVECTOR3   _translateVec;
	//
	D3DXMATRIX     _viewMatrix,_projMatrix,_viewProjMatrix;
	D3DXMATRIX     _inverseViewMatrix;
	//分别绕+X轴/+Y轴的旋转角度
	float                         _roll,_pitch;
	bool                         _isViewChanged,_isViewProjChanged;
	bool                         _isInverseViewChanged;
public:
	explicit Camera();
	void                        initPerspective(float fov,float aspect,float nearZ,float farZ);
	void                        initOrtho(float l,float r,float t,float b,float n,float f);
public:
	//fov:视角的角度
	static Camera *createPerspective(float fov, float aspect, float nearZ, float farZ);
	static Camera *createOrtho(float l, float r, float t, float b, float n, float f);
	//
	//设置视图矩阵
	void                    lookAt(const D3DXVECTOR3 &eyePosition,const D3DXVECTOR3	&targetPosition);
	//更新视图矩阵中的旋转矩阵部分,沿X轴方向/Y轴方向的旋转角度
	void                    rotate(float angleOfX,float angleOfY);
	//移动摄像机
	void                    translate(float  dx,float dz);
	//获取视图矩阵
	const D3DXMATRIX &getViewMatrix();
	//获取投影矩阵
	const D3DXMATRIX &getProjMatrix();
	//获取试图投影矩阵
	const D3DXMATRIX &getViewProjMatrix();
	//获取视图矩阵的逆矩阵
	const D3DXMATRIX  &getInverseViewMatrix();
	//
	const D3DXVECTOR3& getEyePosition()const { return _eyePosition; };
};
#endif