/*
  *����ˮ������ķ���
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
	//�����ÿ��Ԫ����
	int                                 _meshUnitSizeLoc;
private:
	WaterNormalShader();
	WaterNormalShader(const WaterNormalShader &);
	void   initWithFile(const char *csFile);
public:
	~WaterNormalShader();
	static WaterNormalShader *create(const char *csFile);
	/*
	  *���������ÿ��Ԫ�ĳ���
	 */
	void   setMeshUnitSize(float unitSize)const;
	/*
	  *��
	 */
	void   perform()const;
	/*
	  *�ɷ��߳���
	 */
	void   dispatch(int xCount,int yCount)const;
};
#endif
