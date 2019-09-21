/*
  *水光交互/焦散线Caustic
  *@2017-8-23
  *@Author:xiaohuaxiong
 */
#ifndef __CAUSTIC_SHADER_H__
#define __CAUSTIC_SHADER_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"

class CausticShader :public glk::Object
{
	glk::GLProgram			*_glProgram;
	int								_waterHeightMapLoc;
	int								_waterNormalMapLoc;
	int								_modeMatrixLoc;
	int                               _groundHeightLoc;
	int                               _waterHeightLoc;
	int                               _lightDirectionLoc;
	int                               _resolutionLoc;
	int                               _groundMapLoc;
private:
	CausticShader();
	CausticShader(const CausticShader &);
	void             init(const char *vsFile,const char *fsFile);
public:
	~CausticShader();
	static  CausticShader *create(const char *vsFile,const char *fsFile);
	/*
	  *Model Matrix
	 */
	void         setModelMatrix(const glk::Matrix &modelMatrix);
	/*
	  *设置水面高度场纹理
	 */
	void          setWaterHeightMap(int textureId,int unit);
	/*
	  *设置水面法线纹理
	 */
	void          setWaterNormalMap(int textureId,int unit);
	/*
	  *设置地面高度
	 */
	void         setGroundHeight(float groundHeight);
	/*
	  *设置水面的高度
	 */
	void         setWaterHeight(float waterHeight);
	/*
	  *设置光线的方向
	 */
	void        setLightDirection(const glk::GLVector3 &lightDirection);
	/*
	  *设置地面的分辨率
	 */
	void        setResolution(const glk::GLVector2 &resolution);
	/*
	  *设置地面纹理
	 */
	void       setGroundMap(int textureId,int unit);
	/*
	  *perform
	 */
	void       perform()const;
};

#endif