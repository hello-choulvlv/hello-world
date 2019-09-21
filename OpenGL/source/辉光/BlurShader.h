/*
  *模糊纹理图像Shader
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
	//MVP矩阵
	int                           _mvpMatrixLoc;
	//目标纹理
	int                           _baseMapLoc;
	//方向
	int                           _directionLoc;
	//步长
	int                          _stepWidthLoc;
	//位置
	int                          _positionLoc;
	//纹理坐标
	int                          _fragCoordLoc;
private:
	BlurShader();
	BlurShader(BlurShader &);
	void              initWithFile(const char *vsFile,const char *fsFile);
public:
	~BlurShader();
	static BlurShader *create(const char *vsFile, const char *fsFile);
	/*
	  *设置MVP矩阵
	 */
	void         setMVPMatrix(const glk::Matrix &mvpMatrix);
	/*
	  *设置纹理单元
	 */
	void         setBaseMap(int textureId,int unit);
	/*
	  *设置模糊的方向
	 */
	void         setDirection(const glk::GLVector2 &direction);
	/*
	  *设置模糊的步长
	 */
	void         setStepWidth(float width);
	/*
	  *位置坐标
	 */
	int           getPositionLoc()const;
	/*
	  *纹理坐标
	 */
	int           getFragCoord()const;
	//
	void        perform()const;
};
#endif