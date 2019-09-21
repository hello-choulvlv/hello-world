//��ΪGodRayӦ�ó���ĺ�벿����ɫ����
//2017-6-28
//@Author:xiaohuaxiong
#ifndef __RENDER_SHADER_H__
#define __RENDER_SHADER_H__
#include "engine/GLProgram.h"
#include "engine/Geometry.h"

class RenderShader :public glk::Object
{
private:
	glk::GLProgram       *_glProgram;
	int  _modelViewProjMatrixLoc;//MVP�����λ��
	int  _baseMapLoc;//��Ϊ���������λ��
	int  _positionLoc;//position
	int  _fragCoordLoc;//frag coord
private:
	RenderShader(RenderShader &);
	RenderShader();
	void   initWithFile(const char *vsFile,const char *fsFile);
public:
	~RenderShader();
	static RenderShader *createRenderShader(const char *vsFile,const char *fsFile);
	//����ģ��ͶӰ����
	void  setMVPMatrix(const glk::Matrix &mvp);
	//������������
	void  setBaseMap(int texture,int unit);
	//��ȡ�����λ��
	int     getPositionLoc()const;
	//��ȡ���������λ��
	int     getFragCoordLoc()const;
	void   perform()const;
};
#endif