/*
  *实现水渲染的计算着色器
  *2017-7-15
  *@Author:xiaohuaxiong
 */
#ifndef __WATER_COMPUTE_SHADER_H__
#define __WATER_COMPUTE_SHADER_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"

class WaterComputeShader :public glk::Object
{
	//计算着色器
	glk::ComputeShader   *_glProgram;
	//水参数
	int                                   _waterParamLoc;
	//水面网格的尺寸的一半
	int                                   _waterResolutionHalfLoc;
	//水面网格的宽度
	int                                   _waterMeshLoc;
private:
	WaterComputeShader();
	WaterComputeShader(const WaterComputeShader &);
	void    initWithFile(const char *csFile);
public:
	~WaterComputeShader();
	static WaterComputeShader *create(const char *csFile);
	/*
	  *设置水渲染参数
	 */
	void   setWaterParam(const glk::GLVector4 &param)const;
	/*
	  *设置水面网格尺寸的一半
	 */
	void   setWaterResolutionHalf(const glk::GLVector2 &halfResolution)const;
	/*
	  *设置水面网格的跨度
	 */
	void   setWaterMeshSize(int meshSize)const;
	/*
	  *绑定
	 */
	void   perform()const;
	/*
	  *派发线程组
	 */
	void   dispatch(int xCount,int yCount);
};
#endif