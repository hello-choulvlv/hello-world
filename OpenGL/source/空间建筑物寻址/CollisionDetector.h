/*
  *碰撞检测器
  *2Date:2017年12月1日
  *@Author:xiaohuaxiong
 */
#ifndef __COLLISION_DETECTOR_H__
#define __COLLISION_DETECTOR_H__
#include "Triangle.h"
/*
  *关于角色的蹲伏状态
 */
enum CrouchType
{
	CrouchType_Stand =0,//常态,站立
	CrouchType_Crouch = 1,//蹲伏状态
	CrouchType_WouldStand = 2,//从蹲伏状态正在向站立状态过渡
};
class CollisionDetector
{
	//三角形集合
	Triangle		*_triangles;
	int                 _triangleCount;
	//眼睛的高度
	float              _eyeHeight;
	//眼睛到膝盖的高度
	float              _eyeKneeHeight;
	//眼睛的高度的一半,此数值用来计算在下蹲的时候角色的位置
	float              _halfEyeHeight;
	float              _halfEyeKneeHeight;
	//备份数据
	float              _backupEyeHeight;
	float              _backupEyeKneeHeight;
	//邻接距离,角色在任何情况下离障碍物都要大于这个距离,否则会被裁剪掉
	float              _closetDistance;
	//角色的状态
	CrouchType   _crouchType;
	//下落速度
	float                 _fallSpeed;
public:
	CollisionDetector();
	~CollisionDetector();
	void                init(glk::GLVector3 *vertex,int count,float eyeHeight,float eyeKneeHeight,float closetDistance);
	/*
	  *获取离当前角色的地面最近距离
	  *如果不满足最小化距离要求,则返回false
	 */
	bool               getHeightUnder(const glk::GLVector3 &eyePosition, float &minDistance, float &height);
	/*
	  *获取离角色眼睛的上方最短距离
	  *如果不满足要求,则返回false
	 */
	bool               getHeightAbove(const glk::GLVector3 &eyePosition,float &minDistance,float &height);
	/*
	  *水平检测,在给定的眼睛的坐标下,角色究竟应该如何调整自己的位置
	 */
	void              checkHorizontal(const glk::GLVector3 &eyePosition,glk::GLVector3 &movement);
	/*
	  *垂直检测,在给定的眼睛的位置的情况下,角色需要如何调整自己的位置
	  *本函数需要处理自己的坐标问题,因为涉及到角色的自由落体运动
	 */
	void              checkVertical(const glk::GLVector3 &eyePosition,float deltaTime,glk::GLVector3 &movement);
	/*
	  *距离测试,如果角色可以通过调整自己的位置前行的话,则返回true,同时返回需要修正的偏移量
	  *否则返回false
	 */
	bool              distanceTest(const glk::GLVector3 &eyePosition,float &minDistance,glk::GLVector3 &compensation);
	/*
	  *状态切换
	  *跳跃,注意也可能操作不成功,取决于角色的空间位置
	 */
	void             changeJump();
	/*
	  *状态切换
	  *蹲伏,也有可能是快速蹲伏
	 */
	void            changeCrouch();
 };
#endif