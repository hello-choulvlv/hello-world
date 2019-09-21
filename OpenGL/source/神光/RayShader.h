/*
  *产生颜色的Shader
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
	//关于模型矩阵的位置
	int                          _modelMatrixLoc;
	//视图投影矩阵的位置
	int                          _viewProjMatrixLoc;
	//法线矩阵
	int                          _normalMatrixLoc;
	//眼睛的坐标
	int                          _eyePositionLoc;
	//物体的颜色
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
	//设置模型矩阵
	void    setModelMatrix(const glk::Matrix &modelMatrix);
	//设置法线矩阵
	void    setNormalMatrix(const glk::Matrix3 &normalMatrix);
	//设置视图投影矩阵
	void    setViewProjMatrix(const glk::Matrix &viewProjMatrix);
	//设置眼睛的位置
	void    setEyePosition(const glk::GLVector3 &eyePosition);
	//模型的颜色
	void    setColor(const glk::GLVector4 &color);
	//zhixing
	void    perform()const;
	int      getPositionLoc()const;
	int      getNormalLoc()const;
};
#endif