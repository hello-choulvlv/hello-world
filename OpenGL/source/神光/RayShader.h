/*
  *������ɫ��Shader
  *2017-06-24
  *@Author:xiaohuaxiong
 */
#ifndef __RAY_SHADER_H__
#define __RAY_SHADER_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"
class RayShader :public glk::Object
{
private:
	glk::GLProgram  *_glProgram;
	//����ģ�;����λ��
	int                          _modelMatrixLoc;
	//��ͼͶӰ�����λ��
	int                          _viewProjMatrixLoc;
	//���߾���
	int                          _normalMatrixLoc;
	//�۾�������
	int                          _eyePositionLoc;
	//�������ɫ
	int                          _colorLoc;
	//
	int                         _positionLoc;
	int                         _normalLoc;
private:
	RayShader();
	RayShader(RayShader &);
	void      initWithFileName(const char *vsFile, const char *fsFile);
public:
	~RayShader();
	static RayShader *createRayShader(const char *vsFile,const char *fsFile);
	//����ģ�;���
	void    setModelMatrix(const glk::Matrix &modelMatrix);
	//���÷��߾���
	void    setNormalMatrix(const glk::Matrix3 &normalMatrix);
	//������ͼͶӰ����
	void    setViewProjMatrix(const glk::Matrix &viewProjMatrix);
	//�����۾���λ��
	void    setEyePosition(const glk::GLVector3 &eyePosition);
	//ģ�͵���ɫ
	void    setColor(const glk::GLVector4 &color);
	//zhixing
	void    perform()const;
	int      getPositionLoc()const;
	int      getNormalLoc()const;
};
#endif