/*
  *碰撞检测器实现
  *@date:2017/12/3
  *@Author:xiaohuaxiong
 */
#include "CollisionDetector.h"

__US_GLK__;
CollisionDetector::CollisionDetector():
	_triangles(nullptr)
	,_triangleCount(0)
	,_eyeHeight(0)
	,_eyeKneeHeight(0)
	,_halfEyeHeight(0)
	,_halfEyeKneeHeight(0)
	,_backupEyeHeight(0)
	,_backupEyeKneeHeight(0)
	,_crouchType(CrouchType::CrouchType_Stand)
	,_fallSpeed(0)
{

}

CollisionDetector::~CollisionDetector()
{
	delete[] _triangles;
	_triangles = nullptr;
}

void CollisionDetector::init(glk::GLVector3 *vertex, int count, float eyeHeight, float eyeKneeHeight, float closetDistance)
{
	//创建三角形
	_triangleCount = count / 3;
	_triangles = new Triangle[_triangleCount];
	for (int i = 0,k=0; i < count; i += 3,++k)
	{
		_triangles[k].init(vertex[i], vertex[i + 1], vertex[i + 2]);
	}
	_eyeHeight = eyeHeight;
	_eyeKneeHeight = eyeKneeHeight;
	_closetDistance = closetDistance;
	//
	_halfEyeHeight = eyeHeight / 2;
	_halfEyeKneeHeight = eyeKneeHeight / 2;
	//
	_backupEyeHeight = eyeHeight;
	_backupEyeKneeHeight = eyeKneeHeight;
}

bool CollisionDetector::getHeightAbove(const glk::GLVector3 &eyePosition, float &minDistance, float &height)
{
	bool heightTest = false;
	for (int k = 0; k < _triangleCount; ++k)
		heightTest |= _triangles[k].rayIntersectAbove(eyePosition, minDistance, height);
	return heightTest;
}

bool CollisionDetector::getHeightUnder(const glk::GLVector3 &eyePosition, float &minDistance, float &height)
{
	bool heightTest = false;
	for (int k = 0; k < _triangleCount; ++k)
		heightTest |= _triangles[k].rayIntersectUnder(eyePosition, _eyeKneeHeight, minDistance, height);
	return heightTest;
}

void CollisionDetector::changeJump()
{
	if (_crouchType == CrouchType::CrouchType_Stand)
	{
		//如果还没有停止跳跃
		if (_fallSpeed == 0)
			_fallSpeed = -9.81f / 3;
	}
	else//此时角色处于蹲伏状态或者正在从蹲伏状态切换到站立形态
	{
		_crouchType = CrouchType::CrouchType_WouldStand;
	}
}

void CollisionDetector::changeCrouch()
{
if (_crouchType == CrouchType::CrouchType_Stand)
{
	//注意,虽然这个时候设置了眼睛的高度,但是眼睛的实际坐标并没有发生改变
	//眼睛的位置发生变化的地方在checkVertical函数里面
	_eyeHeight = _halfEyeHeight;
	_eyeKneeHeight = _halfEyeKneeHeight;
	_crouchType = CrouchType::CrouchType_Crouch;
}
else if (_fallSpeed < 0)//正在处于跳跃状态
{
	if (_crouchType == CrouchType::CrouchType_Crouch)//此时角色在空中也正处于站立形态
	{
		_crouchType = CrouchType::CrouchType_Stand;
		_eyeHeight = _backupEyeHeight;
		_eyeKneeHeight = _backupEyeKneeHeight;
	}
}
else
{
	if (_crouchType == CrouchType::CrouchType_Crouch)//如果是蹲伏状态,切换到正在前往站立形态
		_crouchType = CrouchType::CrouchType_WouldStand;
	else if (_crouchType == CrouchType::CrouchType_WouldStand)//如果是该状态,则迅速切换到蹲伏状态
	{
		_crouchType = CrouchType::CrouchType_Crouch;
		_eyeHeight = _halfEyeHeight;
		_eyeKneeHeight = _halfEyeKneeHeight;
	}
}
}

bool CollisionDetector::distanceTest(const glk::GLVector3 &eyePosition, float &minDistance,glk::GLVector3 &compensation)
{
	bool intersectTest = false;
	for (int k = 0; k < _triangleCount; ++k)
		intersectTest |= _triangles[k].rayDistanceTest(eyePosition, _eyeKneeHeight, _closetDistance, minDistance, compensation);
	return	intersectTest;
}

void CollisionDetector::checkVertical(const glk::GLVector3 &eyePosition, float deltaTime, glk::GLVector3 &movement)
{
	//如果角色正处于从蹲伏状态到站立形态的过渡
	if (_crouchType == CrouchType::CrouchType_WouldStand)
	{
		//检测角色的上方的最近距离
		float minDistance = _backupEyeHeight - _eyeHeight + _closetDistance;
		float height = 0;
		//此时上方minDistance距离范围内是没有障碍物的
		if (!getHeightAbove(eyePosition, minDistance, height))
		{
			_eyeHeight += _backupEyeHeight * deltaTime;
			_eyeKneeHeight = _eyeHeight * _backupEyeKneeHeight / _backupEyeHeight;
			if (_eyeHeight > _backupEyeHeight)
			{
				_eyeHeight = _backupEyeHeight;
				_eyeKneeHeight = _backupEyeKneeHeight;
				_crouchType = CrouchType::CrouchType_Stand;
			}
		}
	}
	//一般形态的测试
	float  groundHeight = 0;
	float  footHeight = eyePosition.y - _eyeHeight;
	float minDistance = 0x7FFFFFFF;
	getHeightUnder(eyePosition, minDistance, groundHeight);
	//测试是否角色有从站立形态过渡到蹲伏状态
	if (groundHeight < footHeight || _fallSpeed < 0)//此时说明角色要么在处于跳跃状态,要么处于从其他状态正在过渡到蹲伏状态
	{
		_fallSpeed += 9.81 * deltaTime;
		float distance = _fallSpeed * deltaTime;
		//测试是否是正在向上运动
		if (_fallSpeed < 0)
		{
			//上方所能允许的障碍物的最大高度
			float obstacleHeight = _closetDistance - distance;
			float  needHeight = 0;
			//此时上方有满足条件的障碍物,因此不能完全站立,需要对计算出来的位移做限制
			if (getHeightAbove(eyePosition, obstacleHeight, needHeight))
			{
				distance = _closetDistance - obstacleHeight;
			}
		}
		//测试,是否需要在此修正计算出来的位移量
		float intervalDistance = footHeight - groundHeight;
		//当出现这个条件判断的时候,只有在角色处于下蹲状态时才会出现
		if (distance > intervalDistance)
			distance = intervalDistance;
		movement = GLVector3(0, -distance, 0);
	}
	else//此时预示着角色的行走楼梯,或者从蹲伏状态过渡到站立形态
	{
		_fallSpeed = 0;
		//预示着角色正在行走楼梯,或者正在处于从蹲伏状态正在切换到站立形态
		float distance = groundHeight - footHeight;
		if(distance<_eyeHeight-_eyeKneeHeight)//不能超过膝盖,这一行代码就决定着角色不能直接跳过窗户
			movement = GLVector3(0, distance, 0);
	}
	if (movement.y != 0)
	{
		//距离测试
		GLVector3 compensation;
		float minDistance = _closetDistance;
		if( distanceTest(eyePosition + movement, minDistance,compensation) )
		{
			//累加上补偿偏移
			movement += compensation;
		}
	}
}

void CollisionDetector::checkHorizontal(const glk::GLVector3 &eyePosition,glk::GLVector3 &movement)
{
	if (_crouchType != CrouchType::CrouchType_Stand)
		movement *= 0.5f;
	//距离测试
	GLVector3 compensation;
	float            minDistance = _closetDistance;
	int   times = 0;
	GLVector3    newEyePosition = eyePosition + movement;
	while (distanceTest(newEyePosition, minDistance,compensation) && times<4)
	{
		movement += compensation;
		++times;
		minDistance = _closetDistance;
		compensation = GLVector3(0);
		newEyePosition = eyePosition + movement;
	}
}