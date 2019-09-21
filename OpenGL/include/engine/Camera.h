/*
  *摄像机类,主要用于大型地形的渲染,空间场景追踪等一系列的3d操作
  *date:2017-6-14
  *@Author:xiaohuaxiong
 */
#ifndef __CAMERA_H__
#define __CAMERA_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/Types.h"
__NS_GLK_BEGIN
/*
  *场景跟踪,在大型地形上漫游
 */
class Camera :public Object
{
	CameraType       _cameraType;
	//视图矩阵
	Matrix    _viewMatrix;
	//视图矩阵的逆
	Matrix    _inverseViewMatrix;
	//投影矩阵
	Matrix    _projMatrix;
	//投影矩阵的逆
	Matrix   _inverseProjMatrix;
	//视图投影矩阵
	Matrix   _viewProjMatrix;
	//视图投影矩阵的逆
	Matrix   _inverseViewProjMatrix;
	//对于三个离散分量的坐标轴,用于合成视图矩阵,同时也方便对视图矩阵加以约束
	GLVector3    _xAxis;
	GLVector3    _yAxis;
	GLVector3    _zAxis;
	//摄像机的视点和目光朝向的点
	GLVector3   _eyePosition;
	GLVector3   _targetPosition;
	//旋转向量
	GLVector3   _rotateVec;
	//平移向量
	GLVector3   _translateVec;
	//是否视图矩阵有所变动
	bool             _isViewDirty;
	//是否投影矩阵发生了变动
	bool            _isProjMatrixDirty;
	//是否视图投影矩阵发生了变化
	bool            _isViewProjDirty;
	//是否视图投影矩阵的逆矩阵发生了变化
	bool            _isInverseViewProjDirty;
private:
	Camera();
	Camera(Camera &);
	void   initCamera(const GLVector3 &eyePosition,const GLVector3 &targetPosition,const GLVector3 &upVec);
public:
	~Camera();
	/*
	  *创建摄像机类,给定眼睛所在的坐标,目光朝向的坐标,以及向上的方向向量
	  *注意,正常情况下方向向量应该朝上,并且在场景跟踪,地形漫游的时候始终会对这个条件加以约束
	 */
	static Camera    *createCamera(const GLVector3 &eyePosition,const GLVector3 &targetPosition,const GLVector3 &upVec);
	/*
	  *返回摄像机的类型
	 */
	inline CameraType   getType()const { return _cameraType; };
	/*
	  *设置透视投影矩阵
	  *@param:angle为透视角度
	  *@param:widthRate屏幕的横纵比
	  *@param:screenHeight屏幕的高度
	  *@param:nearZ,farZ近远平面
	 */
	void    setPerspective(const float angle,const float widthRate,const float nearZ,const float farZ);
	/*
	  *设置正交投影
	  *@param:left,right屏幕的左右宽度
	  *@param:bottom,top屏幕的底部,上部的跨度
	  *@param:near,far近远平面
	*/
	void   setOrtho(float left,float right,float bottom,float top,float nearZ,float farZ);
	/*
	  *获取视图投影矩阵
	 */
	const Matrix &getViewProjMatrix();
	/*
	  *获取视图矩阵
	 */
	const Matrix& getViewMatrix()const;
	/*
	  *获取投影矩阵
	 */
	const Matrix &getProjMatrix()const;
	/*
	  *获取视图矩阵的逆
	 */
	const Matrix &getInverseViewMatrix();
	/*
	  *获取视图投影矩阵的逆
	 */
	const Matrix &getInverseViewProjMatrix();
	/*
	  *更新视图矩阵的旋转矩阵,dx,dy表示视图矩阵的旋转矩阵在X轴Y轴的变化量,以角度为准
	  *常见的计算方式为 dx = -offsetY/winSize.height * _ANGLE_FACTOR_ * 0.5f;
	  *dy = offsetX /winSize.width *_ANGLE__FACTOR_ *0.5f;
	  *其中winSize代表窗口的大小,_ANGLE_FACTOR_可以查看影青中的数值
	 */
	void                  updateRotateMatrix(float dx,float dy);
	/*
	  *更新视图矩阵的平移矩阵
	  *dx:表示向X轴移动的距离
	  *dz:表示向Z轴方向移动的距离
	  *此二者都是以标准OpenGL坐标系
	  *正值表示向正方向,负值表示负方向
	 */
	void                  updateTranslateMatrix(float dx,float dz);
	/*
	  *获取摄像机的坐标
	 */
	const GLVector3&  getCameraPosition()const;
};

__NS_GLK_END
#endif