/*
  *实时水渲染
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
	//MVP 矩阵
	int                         _mvpMatrixLoc;
	//model matrix
	int                         _modelMatrixLoc;
	//texCubeMap
	int                         _texCubeMapLoc;
	//freshnelParam
	int                         _freshnelParamLoc;
	//水的颜色
	int                         _waterColorLoc;
	//摄像机的位置
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
	  *设置MVP矩阵
	 */
	void   setViewProjMatrix(const glk::Matrix &mvpMatrix)const;
	/*
	  *设置模型矩阵
	 */
	void   setModelMatrix(const glk::Matrix &modelMatrix)const;
	/*
	  *设置立方体贴图的位置
	 */
	void   setTexCubeMap(int textureId,int unit)const;
	/*
	  *设置菲涅尔参数
	 */
	void  setFreshnelPatram(const glk::GLVector3 &freshnelParam)const;
	/*
	  *设置水的颜色
	 */
	void setWaterColor(const glk::GLVector4 &waterColor)const;
	/*
	  *设置摄像机的位置
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
