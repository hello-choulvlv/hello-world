/*
  *�����,��Camera.h/Camera.cpp��ʵ�ֲ�ͬ,��ʵ���ʺϵ����ϵ�����
  *�������ڲ���Ѱַ,�����Գ���������ʵ��,Camera2��ʵ�����Ÿ��������
  *����͵ľ��Ƕ�������ĽǶȵ�����
  *2017��11��26��
  *@Author:xiaohuaxiong
*/
#ifndef __CAMERA2_H__
#define __CAMERA2_H__
#include "Geometry.h"
#include "glmath.h"
class Camera2
{
private:
	//��ͼ�����������
	float_3	  _X, _Y, _Z;
	float_3    _forwardZ, _upVector;
	//�۾���λ��,�Լ�Ŀ��λ�õ�λ��
	float_3   _eyePosition, _targetPosition;
	//��ͼ����/��ͼͶӰ����
	mat4x4          _viewMatrix, _projMatrix,_viewProjMatrix;
	//��ͼͶӰ����������
	mat4x4          _viewProjMatrixInverse;
	mat4x4          _rayMatrix,_viewportMatrix,_rayProjMatrix,_rayViewMatrix;
	//�Ƿ���ͼͶӰ������Ҫ���¼���
	bool              _isViewProjDirty;
	//�Ƿ���ͼͶӰ������������Ҫ���¼���
	bool              _isViewProjInverseDirty;
private:
	Camera2();
	/*
	  *������ͼ����
	 */
	void   updateViewMatrix();
public:
	/*
	  *������������͸��ͶӰ����Ĳ���������ͬ
	 */
	void             initWithPerspective(float fov,float whrate,float nearZ,float farZ);
	/*
	  *��������������������Ĳ�����������ͬ
	 */
	void             initWithOrtho(float lx,float rx,float by,float ty,float nearZ,float farZ);
	/*
	  *����͸�Ӿ���
	 */
	static   Camera2   *createWithPerspective(float fov,float whrate,float nearZ,float farZ);
	/*
	  *��������ͶӰ����
	 */
	static   Camera2   *createWithOrtho(float lx,float rx,float by,float ty,float nearZ,float farZ);
	/*
	  *��ͼ����,�볣��Ĵ���͸�Ӿ���ķ�����ͬ,�ú���û��upVector����,
	  upVector��������ֵ��Ҫ��̬����
	 */
	void    lookAt(const float_3 &eyePosition,const float_3 &targetPosition);
	/*
	  *����ƫ�ƺ�������
	 */
	void    translate(const float_3 &offset);
	/*
	  *��ת,ע�������ת�Ĺ�����Camera��Ĳ�֮ͬ��
	  *xOffsetΪ�������X�᷽���ƫ����
	  *yOffsetΪ�������Y�᷽���ƫ����
	 */
	//void   rotate(float xOffset,float yOffset);
	void   rotate2(float xOffset,float yOffset);
	/*
	  *��ȡ�۾���λ��
	 */
	const float_3& getEyePosition()const { return _eyePosition; };
	/*
	  *��ȡĿ��λ��
	 */
	const float_3& getTargetPosition()const {return _targetPosition;};
	/*
	  *��ȡ��ͼ����
	 */
	const mat4x4&       getViewMatrix()const { return _viewMatrix; };
	/*
	  *��ȡͶӰ����
	 */
	const mat4x4&      getProjMatrix()const { return _projMatrix; };
	/*
	  *��ȡ��ͼͶӰ����
	 */
	const mat4x4&       getViewProjMatrix();
	/*
	  *��ȡ��ͼͶӰ����������
	 */
	const mat4x4&      getViewProjReverseMatrix();
	//ray matrix
	const mat4x4&      getRayMatrix()const;
	void   updateRayMatrix();
	void   updateViewportMatrix(float width,float height);
	void   updateRayProjMatrix(float fov,float aspect);
	/*
	  *��ȡ��������X/Y/Z
	*/
	const float_3& getXVector()const { return _X; };
	const float_3& getUpVector()const { return _upVector; };
	const float_3& getForwardVector()const { return _forwardZ; };
};

class Camera
{
public:
	vec3 X, Y, Z, Reference, Position;
	mat4x4f Vin, Pin, VPin, RayMatrix;

public:
	Camera();
	~Camera();

	void CalculateRayMatrix();
	void Look(const vec3 &Position, const vec3 &Reference, bool RotateAroundReference = false);
	bool OnKeyDown(unsigned nChar);
	void OnMouseMove(int dx, int dy);
	void OnMouseWheel(short zDelta);
};
#endif