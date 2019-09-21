/*
  *普通模糊特效
  *2016-10-14 19:10:01
  *@Author:小花熊
  */
#ifndef  __NORMAL_BLUR_H__
#define __NORMAL_BLUR_H__
#include<engine/GLObject.h>
#include<engine/Geometry.h>
#include<engine/GLProgram.h>
struct       NormalBlur:public  GLObject
{
	GLProgram      *_glProgram;
	unsigned            _framebufferId;
	unsigned            _textureId;
	unsigned            _vertexbufferId;
	Matrix               _mvpMatrix;
//位置
	unsigned          _mvpMatrixLoc;
	unsigned           _baseMapLoc;
	unsigned           _screenPixelLoc;
	unsigned           _kernelCountLoc;
	NormalBlur();
	~NormalBlur();
	unsigned          loadBlurTexture(unsigned    targetTextureId);
};
#endif