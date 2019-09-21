/*
  *水渲染Shader封装
  *@2017-8-3
  *@Author:xiaohuaxiong
*/
#ifndef __WATER_SHADER_H__
#define __WATER_SHADER_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"

class WaterShader :public glk::Object
{
	glk::GLProgram   *_glProgram;
	//Model Matrix
	int							_modelMatrixLoc;
	int                           _viewProjMatrixLoc;
	int                           _normalMatrixLoc;
	int                           _heightMapLoc;
	int                           _normalMapLoc;
	int                           _texCubeMapLoc;
	int                           _cubeMapNormalLoc;
	int                           _cameraPositionLoc;
	int                           _lightPositionLoc;
	int                           _halfCubeHeightLoc;
	int                           _waterHeightLoc;
	int                           _positionLoc;
	int                           _fragCoordLoc;
private:
	WaterShader();
	WaterShader(const WaterShader &);
	void		initWithFile(const char *vsFile,const char *fsFile);
public:
	~WaterShader();
	static WaterShader *create(const char *vsFile,const char *fsFile);
	/*
	  *设置模型矩阵
	 */
	void		setModelMatrix(const glk::Matrix &modelMatrix);
	/*
	  *视图投影矩阵
	  */
	void     setViewProjMatrix(const glk::Matrix &viewProjMatrix);
	/*
	  *法线矩阵
	*/
	//void     setNormalMatrix(const glk::Matrix3 &normalMatrix);
	/*
	  *顶点纹理,高度贴图
	 */
	void     setHeightMap(int heightMapId,int unit);
	/*
	  *法线贴图
	*/
	void     setNormalMap(int normalMapId,int unit);
	/*
	  *立方体贴图
	 */
	void    setTexCubeMap(int texCubeMapId,int	unit);
	/*
	  *立方体法线
	 */
	void    setCubeMapNormal(const glk::GLVector3 *cubeMapNormal,int size);
	/*
	  *摄像机的位置
	 */
	void   setCameraPosition(const glk::GLVector3 &cameraPosition);
	/*
	  *光源的位置
	*/
	void   setLightDirection(const glk::GLVector3 &lightDirection);
	/*
	  *水面的高度
	 */
	void   setWaterHeight(float waterHeight);
	/*
	  *半立方体的高度
	 */
	void   setHalfCubeHeight(float halfCubeHeight);
	/*
	  *perform
	 */
	void  perform();
	/*
	  *position
	 */
	int	getPositionLoc();
	/*
	  *fragCoord
	 */
	int  getFragCoordLoc();
};
#endif