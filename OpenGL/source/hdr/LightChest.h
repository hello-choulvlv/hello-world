/*
  *��Ϊһ�ֹ�Դ������
  2016-9-22 08:43:49
  @Author:С����
  */
#ifndef  __LIGHT_CHEST_H__
#define  __LIGHT_CHEST_H__
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<engine/GLObject.h>
//�ڴ��ģʹ��������ʱ��,�����GLState.h����򿪳�����󻺴�Ĺ���
class    LightChest :public GLObject
{
private:
	GLProgram            *_glProgram;//��Դ�������
	GLChest                 *_lightChestShape;
	GLVector3               _lightColor;//��Դ����ɫ
	GLVector3               _position;//��Դ��λ��
	float                         _scaleX, _scaleY, _scaleZ;//��Դ�����ŵı���
	Matrix                     _rotateMatrix;//��Դ��ʵ����ת�����¼
//model����
	Matrix                     _modelMatrix;
//���������ͳһ������λ��
	unsigned                 _mvpMatrixLoc;
	unsigned                 _lightColorLoc;
	float                         _angle;
private:
	LightChest();
	LightChest(LightChest &);
	void                       initLightChest(GLVector3   &lightColor);
public:
	~LightChest();
	static        LightChest          *createLightChest(GLVector3       &lightColor);
//���ù�Դ����ɫ
	void          setColor(GLVector3  &lightColor);
//���ù�Դ��λ��
	void          setPosition(GLVector3  &);
//���ù�Դ�����ű���
	void          setScale(float   scaleX,float  scaleY,float  scaleZ);
	void          setScale(float  scaleXYZ);
//��ĳһ������ת
	void          rotate(float  angle, GLVector3 &);
//update����
	void           update(float      _deltaTime);
//draw����
	void           draw(Matrix   &projectMatrix,unsigned   flag);
};
#endif