/*
  *摄像机,与Camera.h/Camera.cpp的实现不同,该实现适合地形上的漫游
  *建筑物内部的寻址,因此相对常规的摄像机实现,Camera2的实现有着更多的限制
  *最典型的就是对摄像机的角度的限制
  *2017年11月26日
  *@Author:xiaohuaxiong
*/
#ifndef __CAMERA2_H__
#define __CAMERA2_H__
#include "Geometry.h"
#include "glmath.h"
class Camera2
{
private:
	//视图矩阵的三个轴
	float_3	  _X, _Y, _Z;
	float_3    _forwardZ, _upVector;
	//眼睛的位置,以及目标位置的位置
	float_3   _eyePosition, _targetPosition;
	//视图矩阵/视图投影矩阵
	mat4x4          _viewMatrix, _projMatrix,_viewProjMatrix;
	//视图投影矩阵的逆矩阵
	mat4x4          _viewProjMatrixInverse;
	mat4x4          _rayMatrix,_viewportMatrix,_rayProjMatrix,_rayViewMatrix;
	//是否视图投影矩阵需要重新计算
	bool              _isViewProjDirty;
	//是否试图投影矩阵的逆矩阵需要重新计算
	bool              _isViewProjInverseDirty;
private:
	Camera2();
	/*
	  *更新视图矩阵
	 */
	void   updateViewMatrix();
public:
	/*
	  *参数的意义与透视投影矩阵的参数意义相同
	 */
	void             initWithPerspective(float fov,float whrate,float nearZ,float farZ);
	/*
	  *参数的意义与正交矩阵的参数的意义相同
	 */
	void             initWithOrtho(float lx,float rx,float by,float ty,float nearZ,float farZ);
	/*
	  *创建透视矩阵
	 */
	static   Camera2   *createWithPerspective(float fov,float whrate,float nearZ,float farZ);
	/*
	  *创建正交投影矩阵
	 */
	static   Camera2   *createWithOrtho(float lx,float rx,float by,float ty,float nearZ,float farZ);
	/*
	  *视图矩阵,与常规的创建透视矩阵的方案不同,该函数没有upVector参数,
	  upVector参数的数值需要动态计算
	 */
	void    lookAt(const float_3 &eyePosition,const float_3 &targetPosition);
	/*
	  *计算偏移后的摄像机
	 */
	void    translate(const float_3 &offset);
	/*
	  *旋转,注意计算旋转的过程与Camera类的不同之处
	  *xOffset为摄像机在X轴方向的偏移量
	  *yOffset为摄像机在Y轴方向的偏移量
	 */
	//void   rotate(float xOffset,float yOffset);
	void   rotate2(float xOffset,float yOffset);
	/*
	  *获取眼睛的位置
	 */
	const float_3& getEyePosition()const { return _eyePosition; };
	/*
	  *获取目标位置
	 */
	const float_3& getTargetPosition()const {return _targetPosition;};
	/*
	  *获取视图矩阵
	 */
	const mat4x4&       getViewMatrix()const { return _viewMatrix; };
	/*
	  *获取投影矩阵
	 */
	const mat4x4&      getProjMatrix()const { return _projMatrix; };
	/*
	  *获取视图投影矩阵
	 */
	const mat4x4&       getViewProjMatrix();
	/*
	  *获取视图投影矩阵的逆矩阵
	 */
	const mat4x4&      getViewProjReverseMatrix();
	//ray matrix
	const mat4x4&      getRayMatrix()const;
	void   updateRayMatrix();
	void   updateViewportMatrix(float width,float height);
	void   updateRayProjMatrix(float fov,float aspect);
	/*
	  *获取方向向量X/Y/Z
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