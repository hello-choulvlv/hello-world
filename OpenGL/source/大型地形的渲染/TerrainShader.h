/*
  *渲染地形的OpenGL Shader
  *@date:2017-6-8
  *@Author:xiaohuaxiong
  */
#ifndef __TERRAIN_SHADER_H__
#define __TERRAIN_SHADER_H__
#include "engine/GLProgram.h"
#include "engine/Geometry.h"
class TerrainShader
{
	glk::GLProgram *_terrainProgram;
	//模型矩阵的shader位置
	int                         _modelMatrixLoc;
	//视图投影矩阵的位置
	int                         _viewProjMatrixLoc;
	//法线矩阵
	int                         _normalMatrixLoc;
	//眼睛的位置
	int                         _eyePositionLoc;
	//颜色
	int                         _colorLoc;
	//光线的颜色
	int                         _lightColorLoc;
	//光线的方向
	int                         _lightDirectionLoc;
	//顶点的位置
	int                         _positionLoc;
	//顶点的法线
	int                         _normalLoc;
private:
	TerrainShader();
	TerrainShader(TerrainShader &);
	void    initWithFile(const char *vsFile,const char *fsFile);
public:
	~TerrainShader();

	static TerrainShader *createTerrainShader(const char *vsFile,const char *fsFile);
	/*
	  *设置模型矩阵
	 */
	void   setModelMatrix(const glk::Matrix &modelMatrix);
	/*
	  *设置视图投影矩阵
	 */
	void   setViewProjMatrix(const glk::Matrix &viewProjMatrix);
	/*
	  *设置法线矩阵
	 */
	void   setNormalMatrix(const glk::Matrix3 &normalMatrix);
	/*
	  *设置观察者的位置
	 */
	void   setEyePosition(const glk::GLVector3 &eyePosition);
	/*
	  *设置光线的方向
	*/
	void   setLightDirection(const glk::GLVector3 &lightDirection);
	/*
	  *设置光线的颜色
	 */
	void   setLightColor(const glk::GLVector4 &lightColor);
	/*
	  *设置地形的基本颜色
	 */
	void   setTerrainColor(const glk::GLVector4 &color);
	/*
	  *顶点的位置
	 */
	int     getPositionLoc()const;
	/*
	  *法线的位置
	 */
	int     getNormalLoc()const;
	/*
	  *使用Shader
	 */
	void   perform()const;
};
#endif