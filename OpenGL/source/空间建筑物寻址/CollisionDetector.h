/*
  *��ײ�����
  *2Date:2017��12��1��
  *@Author:xiaohuaxiong
 */
#ifndef __COLLISION_DETECTOR_H__
#define __COLLISION_DETECTOR_H__
#include "Triangle.h"
/*
  *���ڽ�ɫ�Ķ׷�״̬
 */
enum CrouchType
{
	CrouchType_Stand =0,//��̬,վ��
	CrouchType_Crouch = 1,//�׷�״̬
	CrouchType_WouldStand = 2,//�Ӷ׷�״̬������վ��״̬����
};
class CollisionDetector
{
	//�����μ���
	Triangle		*_triangles;
	int                 _triangleCount;
	//�۾��ĸ߶�
	float              _eyeHeight;
	//�۾���ϥ�ǵĸ߶�
	float              _eyeKneeHeight;
	//�۾��ĸ߶ȵ�һ��,����ֵ�����������¶׵�ʱ���ɫ��λ��
	float              _halfEyeHeight;
	float              _halfEyeKneeHeight;
	//��������
	float              _backupEyeHeight;
	float              _backupEyeKneeHeight;
	//�ڽӾ���,��ɫ���κ���������ϰ��ﶼҪ�����������,����ᱻ�ü���
	float              _closetDistance;
	//��ɫ��״̬
	CrouchType   _crouchType;
	//�����ٶ�
	float                 _fallSpeed;
public:
	CollisionDetector();
	~CollisionDetector();
	void                init(glk::GLVector3 *vertex,int count,float eyeHeight,float eyeKneeHeight,float closetDistance);
	/*
	  *��ȡ�뵱ǰ��ɫ�ĵ����������
	  *�����������С������Ҫ��,�򷵻�false
	 */
	bool               getHeightUnder(const glk::GLVector3 &eyePosition, float &minDistance, float &height);
	/*
	  *��ȡ���ɫ�۾����Ϸ���̾���
	  *���������Ҫ��,�򷵻�false
	 */
	bool               getHeightAbove(const glk::GLVector3 &eyePosition,float &minDistance,float &height);
	/*
	  *ˮƽ���,�ڸ������۾���������,��ɫ����Ӧ����ε����Լ���λ��
	 */
	void              checkHorizontal(const glk::GLVector3 &eyePosition,glk::GLVector3 &movement);
	/*
	  *��ֱ���,�ڸ������۾���λ�õ������,��ɫ��Ҫ��ε����Լ���λ��
	  *��������Ҫ�����Լ�����������,��Ϊ�漰����ɫ�����������˶�
	 */
	void              checkVertical(const glk::GLVector3 &eyePosition,float deltaTime,glk::GLVector3 &movement);
	/*
	  *�������,�����ɫ����ͨ�������Լ���λ��ǰ�еĻ�,�򷵻�true,ͬʱ������Ҫ������ƫ����
	  *���򷵻�false
	 */
	bool              distanceTest(const glk::GLVector3 &eyePosition,float &minDistance,glk::GLVector3 &compensation);
	/*
	  *״̬�л�
	  *��Ծ,ע��Ҳ���ܲ������ɹ�,ȡ���ڽ�ɫ�Ŀռ�λ��
	 */
	void             changeJump();
	/*
	  *״̬�л�
	  *�׷�,Ҳ�п����ǿ��ٶ׷�
	 */
	void            changeCrouch();
 };
#endif