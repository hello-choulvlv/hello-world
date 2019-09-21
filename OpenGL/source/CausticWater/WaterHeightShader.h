/*
  *水面高度计算GPU实现
  *2017-8-2
  *@Author:xiaohuaxiong
 */
#ifndef __WATER_HEIGHT_SHADER_H__
#define __WATER_HEIGHT_SHADER_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/GLProgram.h"

class WaterHeightShader :public glk::Object
{
private:
	glk::GLProgram  *_glProgram;
	int                          _baseMapLoc;
	int                          _waterParamLoc;
	int                          _meshSizeLoc;
	int                          _positionLoc;
	int                          _fragCoordLoc;
private:
	WaterHeightShader();
	WaterHeightShader(const WaterHeightShader &);
	void		initWithFile(const char *vsFile,const char *fsFile);
public:
	~WaterHeightShader();
	static WaterHeightShader *create(const char *vsFile,const char *fsFile);
	/*
	  *设置基本贴图
	 */
	void    setBaseMap(int textureId,int unit);
	/*
	  *设置关于水的参数
	*/
	void   setWaterParam(const glk::GLVector4 &waterParam);
	/*
	  *设置网格单元的总宽度
	*/
	void   setMeshSize(glk::GLVector2 &meshSize);
	/*
	  *perform
	 */
	void  perform()const;
	/*
	  *position location
	 */
	int  getPositionLoc();
	/*
	  *get fragCoord loction
	*/
	int    getFragCoordLoc();
};


#endif