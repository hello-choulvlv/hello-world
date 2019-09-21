/*
  *DirectX�����
  *2018��3��16��
  *@version: 1.0
  *@author:xiaohuaxiong
 */
#ifndef __CAMERA_H__
#define __CAMERA_H__
#include<d3dx10math.h>
class Camera
{
	//������۾���λ��
	D3DXVECTOR3   _eyePosition;
	D3DXVECTOR3   _translateVec;
	//
	D3DXMATRIX     _viewMatrix,_projMatrix,_viewProjMatrix;
	D3DXMATRIX     _inverseViewMatrix;
	//�ֱ���+X��/+Y�����ת�Ƕ�
	float                         _roll,_pitch;
	bool                         _isViewChanged,_isViewProjChanged;
	bool                         _isInverseViewChanged;
public:
	explicit Camera();
	void                        initPerspective(float fov,float aspect,float nearZ,float farZ);
	void                        initOrtho(float l,float r,float t,float b,float n,float f);
public:
	//fov:�ӽǵĽǶ�
	static Camera *createPerspective(float fov, float aspect, float nearZ, float farZ);
	static Camera *createOrtho(float l, float r, float t, float b, float n, float f);
	//
	//������ͼ����
	void                    lookAt(const D3DXVECTOR3 &eyePosition,const D3DXVECTOR3	&targetPosition);
	//������ͼ�����е���ת���󲿷�,��X�᷽��/Y�᷽�����ת�Ƕ�
	void                    rotate(float angleOfX,float angleOfY);
	//�ƶ������
	void                    translate(float  dx,float dz);
	//��ȡ��ͼ����
	const D3DXMATRIX &getViewMatrix();
	//��ȡͶӰ����
	const D3DXMATRIX &getProjMatrix();
	//��ȡ��ͼͶӰ����
	const D3DXMATRIX &getViewProjMatrix();
	//��ȡ��ͼ����������
	const D3DXMATRIX  &getInverseViewMatrix();
	//
	const D3DXVECTOR3& getEyePosition()const { return _eyePosition; };
};
#endif