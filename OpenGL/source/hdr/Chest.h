/*
  *����hdr������
  *2016-9-20 17:58:43
  *@Author:С����
 */
//ע���ڴ���ʹ��������ʱ��һ��Ҫ��GLState.h���濪��������󻺴湦��
#ifndef  __CHEST_H__
#define __CHEST_H__
#include<GL/glew.h>
#include<engine/GLProgram.h>
#include<engine/GLTexture.h>
#include<engine/Shape.h>
#include<engine/Geometry.h>
class    Chest :public  GLObject
{
private:
	GLProgram         *_glProgram;
	GLChest	             *_chestShape;
	GLTexture           *_texture;
//��������
	float                       _scaleX, _scaleY, _scaleZ;
	GLVector3            _position;
//�Ƹ���������ת
	Matrix                  _rotateMatrix;
//model view ����
	Matrix                 _modelViewMatrix;
	Matrix                 _mvpMatrix;
//���߾���
	Matrix3               _normalMatrix;
//���ߵ�λ��
	GLVector3           _lightPosition;
//���ߵ���ɫ
	GLVector3          _lightColor;
//�۾���λ��
	GLVector3          _eyePosition;
//������ǿ��
	GLVector3          _ambientColor;
//����ϵ��
	float                    _specularFactor;
//�������ͳһ������λ��
	unsigned            _mvpMatrixLoc;
	unsigned            _modelViewMatrixLoc;
	unsigned            _normalMatrixLoc;
	unsigned            _baseMapLoc;
	unsigned            _lightPositionLoc;
	unsigned            _lightColorLoc;
	unsigned            _eyePositionLoc;
	unsigned            _ambientColorLoc;
	unsigned            _specularFactorLoc;
	float                   _angle;
private:
	Chest();
	Chest(Chest &);
	void                   initChest(const  char   *file_name);
public:
	~Chest();
	static          Chest            *createChest(const  char   *file_name);
//�������ӵ�λ��
	void             setPosition(GLVector3   &);
//�������ű���
	void             setScale(float      scaleX,float   scaleY,float    scaleZ);
	void             setScale(float      scaleXYZ);
//������ĳһ������ת�Ƕ�
	void             setRotateAngle(float   angle,GLVector3  &);
//���ù�Դ��λ��
	void             setLightPosition(GLVector3  &);
//���ù�Դ����ɫ
	void             setLightColor(GLVector3   &);
//���û��������ɫ
	void             setAmbientColor(GLVector3  &);
//�����۾���λ��
	void             setEyePosition(GLVector3   &);
//���÷���ϵ��
	void             setSpecularCoeff(float  _coff);
//update����
	void             update(float      _delta);
	void             draw(Matrix        &projectMatrix,unsigned    drawFlag);
};
#endif