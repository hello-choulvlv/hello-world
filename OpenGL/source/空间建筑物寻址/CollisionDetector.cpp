/*
  *��ײ�����ʵ��
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
	//����������
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
		//�����û��ֹͣ��Ծ
		if (_fallSpeed == 0)
			_fallSpeed = -9.81f / 3;
	}
	else//��ʱ��ɫ���ڶ׷�״̬�������ڴӶ׷�״̬�л���վ����̬
	{
		_crouchType = CrouchType::CrouchType_WouldStand;
	}
}

void CollisionDetector::changeCrouch()
{
if (_crouchType == CrouchType::CrouchType_Stand)
{
	//ע��,��Ȼ���ʱ���������۾��ĸ߶�,�����۾���ʵ�����겢û�з����ı�
	//�۾���λ�÷����仯�ĵط���checkVertical��������
	_eyeHeight = _halfEyeHeight;
	_eyeKneeHeight = _halfEyeKneeHeight;
	_crouchType = CrouchType::CrouchType_Crouch;
}
else if (_fallSpeed < 0)//���ڴ�����Ծ״̬
{
	if (_crouchType == CrouchType::CrouchType_Crouch)//��ʱ��ɫ�ڿ���Ҳ������վ����̬
	{
		_crouchType = CrouchType::CrouchType_Stand;
		_eyeHeight = _backupEyeHeight;
		_eyeKneeHeight = _backupEyeKneeHeight;
	}
}
else
{
	if (_crouchType == CrouchType::CrouchType_Crouch)//����Ƕ׷�״̬,�л�������ǰ��վ����̬
		_crouchType = CrouchType::CrouchType_WouldStand;
	else if (_crouchType == CrouchType::CrouchType_WouldStand)//����Ǹ�״̬,��Ѹ���л����׷�״̬
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
	//�����ɫ�����ڴӶ׷�״̬��վ����̬�Ĺ���
	if (_crouchType == CrouchType::CrouchType_WouldStand)
	{
		//����ɫ���Ϸ����������
		float minDistance = _backupEyeHeight - _eyeHeight + _closetDistance;
		float height = 0;
		//��ʱ�Ϸ�minDistance���뷶Χ����û���ϰ����
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
	//һ����̬�Ĳ���
	float  groundHeight = 0;
	float  footHeight = eyePosition.y - _eyeHeight;
	float minDistance = 0x7FFFFFFF;
	getHeightUnder(eyePosition, minDistance, groundHeight);
	//�����Ƿ��ɫ�д�վ����̬���ɵ��׷�״̬
	if (groundHeight < footHeight || _fallSpeed < 0)//��ʱ˵����ɫҪô�ڴ�����Ծ״̬,Ҫô���ڴ�����״̬���ڹ��ɵ��׷�״̬
	{
		_fallSpeed += 9.81 * deltaTime;
		float distance = _fallSpeed * deltaTime;
		//�����Ƿ������������˶�
		if (_fallSpeed < 0)
		{
			//�Ϸ�����������ϰ�������߶�
			float obstacleHeight = _closetDistance - distance;
			float  needHeight = 0;
			//��ʱ�Ϸ��������������ϰ���,��˲�����ȫվ��,��Ҫ�Լ��������λ��������
			if (getHeightAbove(eyePosition, obstacleHeight, needHeight))
			{
				distance = _closetDistance - obstacleHeight;
			}
		}
		//����,�Ƿ���Ҫ�ڴ��������������λ����
		float intervalDistance = footHeight - groundHeight;
		//��������������жϵ�ʱ��,ֻ���ڽ�ɫ�����¶�״̬ʱ�Ż����
		if (distance > intervalDistance)
			distance = intervalDistance;
		movement = GLVector3(0, -distance, 0);
	}
	else//��ʱԤʾ�Ž�ɫ������¥��,���ߴӶ׷�״̬���ɵ�վ����̬
	{
		_fallSpeed = 0;
		//Ԥʾ�Ž�ɫ��������¥��,�������ڴ��ڴӶ׷�״̬�����л���վ����̬
		float distance = groundHeight - footHeight;
		if(distance<_eyeHeight-_eyeKneeHeight)//���ܳ���ϥ��,��һ�д���;����Ž�ɫ����ֱ����������
			movement = GLVector3(0, distance, 0);
	}
	if (movement.y != 0)
	{
		//�������
		GLVector3 compensation;
		float minDistance = _closetDistance;
		if( distanceTest(eyePosition + movement, minDistance,compensation) )
		{
			//�ۼ��ϲ���ƫ��
			movement += compensation;
		}
	}
}

void CollisionDetector::checkHorizontal(const glk::GLVector3 &eyePosition,glk::GLVector3 &movement)
{
	if (_crouchType != CrouchType::CrouchType_Stand)
		movement *= 0.5f;
	//�������
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