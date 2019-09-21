/*
  *ģ������ͼ��Shader
   *2017-07-14
   *@Author:xiaohuaxiong
 */
#ifndef __BLUR_SHADER_H__
#define __BLUR_SHADER_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"

class BlurShader :public glk::Object
{
	glk::GLProgram   *_glProgram;
	//MVP����
	int                           _mvpMatrixLoc;
	//Ŀ������
	int                           _baseMapLoc;
	//����
	int                           _directionLoc;
	//����
	int                          _stepWidthLoc;
	//λ��
	int                          _positionLoc;
	//��������
	int                          _fragCoordLoc;
private:
	BlurShader();
	BlurShader(BlurShader &);
	void              initWithFile(const char *vsFile,const char *fsFile);
public:
	~BlurShader();
	static BlurShader *create(const char *vsFile, const char *fsFile);
	/*
	  *����MVP����
	 */
	void         setMVPMatrix(const glk::Matrix &mvpMatrix);
	/*
	  *��������Ԫ
	 */
	void         setBaseMap(int textureId,int unit);
	/*
	  *����ģ���ķ���
	 */
	void         setDirection(const glk::GLVector2 &direction);
	/*
	  *����ģ���Ĳ���
	 */
	void         setStepWidth(float width);
	/*
	  *λ������
	 */
	int           getPositionLoc()const;
	/*
	  *��������
	 */
	int           getFragCoord()const;
	//
	void        perform()const;
};
#endif