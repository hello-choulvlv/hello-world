/*
  *��˹ͼ��ģ��
  *2016-9-22 11:11:38
  *@Author:С����
  */
#ifndef  __GAUSSIAN_PERFORM_H__
#define  __GAUSSIAN_PERFORM_H__
#include<engine/GLObject.h>
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
//ʹ��������ʱ����Ҫһ�� ��������
class    GaussianPerform :public  GLObject
{
private:
	GLProgram    *_glProgram;
//֡����������
	unsigned          _pp_framebufferId[2];
//֡��������������
	unsigned          _pp_textureId[2];
//�洢���յı�����õ����������
	unsigned          _pp_texture_index;
//���㻺��������
	unsigned         _vertex_bufferId;
	Matrix            _mvpMatrix;
	GLVector2     _hvstep;
//��������ͳһ������λ��
	unsigned           _mvpMatrixLoc;
	unsigned           _baseMapLoc;
//��ˮƽ�ʹ�ֱ�����ϵĲ������,���ֵ�������Ƶ�ǰ�ǽ���ˮƽ�����Ǵ�ֱ����
	unsigned           _hvstepLoc;
	int                      _performTimes;
private:
	GaussianPerform();
	GaussianPerform(GaussianPerform &);
	void         initGaussianPerform(int  width,int  height,int   performTimes);
public:
	~GaussianPerform();
	static       GaussianPerform         *createGaussianPerform(int  width,int  height,int     performTimes);
//���ô���Ĵ���
	void          setPerformTimes(int   performTimes);
//��˹ģ������,base_textureId:��������������
	void         perform(unsigned      base_textureId);
//��ȡ������õ����������
	unsigned      performedTexture();
};
#endif
