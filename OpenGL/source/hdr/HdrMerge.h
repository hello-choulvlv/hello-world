/*
  *Hdr纹理融合
  *2016-9-22 15:34:15
  *@Author:小花熊
*/
#ifndef __HDR_MERGE_H__
#define __HDR_MERGE_H__
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
class        HdrMerge :public  GLObject
{
private:
	GLProgram		*_glProgram;
	Matrix                  _mvpMatrix;
	unsigned              _vertex_bufferId;
//hdr高光指数
	float                     _exposure;
//
	unsigned              _mvpMatrixLoc;
	unsigned              _baseMapLoc;
	unsigned              _hdrMapLoc;
	unsigned              _exposureLoc;
private:
	HdrMerge();
	HdrMerge(HdrMerge &);
	void                       initHdrMerge(float   exposure);
public:
	~HdrMerge();
//高光指数
	static      HdrMerge		*createHdrMerge(float  exposure);
//设置高光指数
	void                 setExposure(float);
//drawMerge
	void                       drawMerge(unsigned     baseMap,unsigned      hdrMap);
};
#endif