/*
  *高斯图像模糊
  *2016-9-22 11:11:38
  *@Author:小花熊
  */
#ifndef  __GAUSSIAN_PERFORM_H__
#define  __GAUSSIAN_PERFORM_H__
#include<engine/GLObject.h>
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
//使用这个类的时候需要一个 基本纹理
class    GaussianPerform :public  GLObject
{
private:
	GLProgram    *_glProgram;
//帧缓冲区对象
	unsigned          _pp_framebufferId[2];
//帧缓冲区的纹理附着
	unsigned          _pp_textureId[2];
//存储最终的被处理好的纹理的索引
	unsigned          _pp_texture_index;
//顶点缓冲区对象
	unsigned         _vertex_bufferId;
	Matrix            _mvpMatrix;
	GLVector2     _hvstep;
//程序对象的统一变量的位置
	unsigned           _mvpMatrixLoc;
	unsigned           _baseMapLoc;
//在水平和垂直方向上的步进情况,这个值用来控制当前是进行水平处理还是垂直处理
	unsigned           _hvstepLoc;
	int                      _performTimes;
private:
	GaussianPerform();
	GaussianPerform(GaussianPerform &);
	void         initGaussianPerform(int  width,int  height,int   performTimes);
public:
	~GaussianPerform();
	static       GaussianPerform         *createGaussianPerform(int  width,int  height,int     performTimes);
//设置处理的次数
	void          setPerformTimes(int   performTimes);
//高斯模糊处理,base_textureId:待处理的纹理对象
	void         perform(unsigned      base_textureId);
//获取被处理好的纹理的索引
	unsigned      performedTexture();
};
#endif
