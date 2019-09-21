/*
  *ʵʱˮ��Ⱦ
  *2017-7-26
  *@Author:xiaohuaxiong
 */
#ifndef __RENDER_SHADER_H__
#define __RENDER_SHADER_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"

class RenderShader :public glk::Object
{
	glk::GLProgram *_glProgram;
	//MVP ����
	int                         _mvpMatrixLoc;
	//model matrix
	int                         _modelMatrixLoc;
	//texCubeMap
	int                         _texCubeMapLoc;
	//freshnelParam
	int                         _freshnelParamLoc;
	//ˮ����ɫ
	int                         _waterColorLoc;
	//�������λ��
	int                         _cameraPositionLoc;
	//
	int                         _positionLoc;
	//
	int                         _normalLoc;
private:
	RenderShader();
	RenderShader(const RenderShader &);
	void     initWithFile(const char *vsFile,const char *fsFile);
public:
	~RenderShader();
	static RenderShader *create(const char *vsFile,const char *fsFile);
	/*
	  *����MVP����
	 */
	void   setViewProjMatrix(const glk::Matrix &mvpMatrix)const;
	/*
	  *����ģ�;���
	 */
	void   setModelMatrix(const glk::Matrix &modelMatrix)const;
	/*
	  *������������ͼ��λ��
	 */
	void   setTexCubeMap(int textureId,int unit)const;
	/*
	  *���÷���������
	 */
	void  setFreshnelPatram(const glk::GLVector3 &freshnelParam)const;
	/*
	  *����ˮ����ɫ
	 */
	void setWaterColor(const glk::GLVector4 &waterColor)const;
	/*
	  *�����������λ��
	 */
	void setCameraPosition(const glk::GLVector3 &cameraPosition)const;
	/*
	  *position
	 */
	int   getPositionLoc()const;
	/*
	  *normal
	 */
	int   getNormalLoc()const;
	/*
	  *perform
	 */
	void  perform()const;
};

#endif
