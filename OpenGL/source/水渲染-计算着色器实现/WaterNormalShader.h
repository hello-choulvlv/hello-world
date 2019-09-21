/*
  *计算水面网格的法线
  *2017-7-26
  *@Author:xiaohuaxiong
 */
#ifndef __WATER_NORMAL_SHADER_H__
#define __WATER_NORMAL_SHADER_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/GLProgram.h"

class WaterNormalShader :public glk::Object
{
	glk::ComputeShader  *_glProgram;
	//网格的每单元长度
	int                                 _meshUnitSizeLoc;
private:
	WaterNormalShader();
	WaterNormalShader(const WaterNormalShader &);
	void   initWithFile(const char *csFile);
public:
	~WaterNormalShader();
	static WaterNormalShader *create(const char *csFile);
	/*
	  *设置网格的每单元的长度
	 */
	void   setMeshUnitSize(float unitSize)const;
	/*
	  *绑定
	 */
	void   perform()const;
	/*
	  *派发线程组
	 */
	void   dispatch(int xCount,int yCount)const;
};
#endif
