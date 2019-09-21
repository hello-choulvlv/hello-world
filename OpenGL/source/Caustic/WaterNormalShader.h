/*
  *计算水面网格法线
  *@2017-8-2
  @Author:xiaohuaxiong
*/
#ifndef __WATER_NORMAL_SHADER_H__
#define __WATER_NORMAL_SHADER_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/GLProgram.h"

class WaterNormalShader :public glk::Object
{
	glk::GLProgram *_glProgram;
	int                         _baseMapLoc;
	int                         _meshIntervalLoc;
	int                         _positionLoc;
	int                         _fragCoordLoc;
private:
	WaterNormalShader();
	WaterNormalShader(const WaterNormalShader &);
	void   initWithFile(const char *vsFile,const char *fsFile);
public:
	~WaterNormalShader();
	static WaterNormalShader *create(const char *vsFile,const char *fsFile);
	/*
	  *设置基本纹理
	*/
	void  setBaseMap(int textureId,int unit);
	/*
	  *设置网格间隔
	*/
	void  setMeshInterval(float meshInterval);
	/*
	  *get position location
	 */
	int    getPositionLoc()const;
	/*
	  *get fragCoord location
	*/
	int    getFragCoordLoc()const;
	/*
	  *perform
	 */
	void  perform()const;

};
#endif