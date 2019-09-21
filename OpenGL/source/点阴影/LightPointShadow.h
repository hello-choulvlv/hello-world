/*
  *���Դ��Ӱ,ע����ʹ��������ʱ��,��Ҫ��GLState.h���ֶ��򿪼�����ɫ����
  *2016-10-25 20:21:33
  *@Author:С����
 */
#include<engine/GLObject.h>
#include<engine/Geometry.h>
#include<engine/GLProgram.h>

class    LightPointShadow :public GLObject
{
public:
	GLProgram           *_shadowProgram;
	unsigned                 _framebufferId;
#ifdef  __GEOMETRY_SHADOW__
	unsigned                 _depthTextureId;
	unsigned                 _modelMatrixLoc;
	unsigned                 _viewProjMatrixLoc[6];
	unsigned                 _lightPositionLoc;
	unsigned                 _maxDistanceLoc;
#else
	unsigned                 _depthTextureId;
	unsigned                 _cubeMapId;
	unsigned                 _mvpMatrixLoc;
	unsigned                 _modelMatrixLoc;
	unsigned                 _lightPositionLoc;
	Matrix                    _viewMatrix;
#endif
public:
	LightPointShadow();
	~LightPointShadow();
#ifndef __GEOMETRY_SHADOW__
//�������Ϣд�뵽�ڼ�����
	void              writeFace(int   index, GLVector3  & _lightPosition);
//������������󶨵�ĳ����Ԫ
	void              bindTextureUnit(int  _texture_unit);
//������ͼ����
	Matrix         &loadViewMatrix();
#endif
};