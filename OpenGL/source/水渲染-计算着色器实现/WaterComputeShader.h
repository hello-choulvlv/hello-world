/*
  *ʵ��ˮ��Ⱦ�ļ�����ɫ��
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
	//������ɫ��
	glk::ComputeShader   *_glProgram;
	//ˮ����
	int                                   _waterParamLoc;
	//ˮ������ĳߴ��һ��
	int                                   _waterResolutionHalfLoc;
	//ˮ������Ŀ��
	int                                   _waterMeshLoc;
private:
	WaterComputeShader();
	WaterComputeShader(const WaterComputeShader &);
	void    initWithFile(const char *csFile);
public:
	~WaterComputeShader();
	static WaterComputeShader *create(const char *csFile);
	/*
	  *����ˮ��Ⱦ����
	 */
	void   setWaterParam(const glk::GLVector4 &param)const;
	/*
	  *����ˮ������ߴ��һ��
	 */
	void   setWaterResolutionHalf(const glk::GLVector2 &halfResolution)const;
	/*
	  *����ˮ������Ŀ��
	 */
	void   setWaterMeshSize(int meshSize)const;
	/*
	  *��
	 */
	void   perform()const;
	/*
	  *�ɷ��߳���
	 */
	void   dispatch(int xCount,int yCount);
};
#endif