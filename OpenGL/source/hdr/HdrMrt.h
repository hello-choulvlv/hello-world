/*
 *hdr + mrt :两个纹理附着
 *2016-9-22 10:33:47
 *@Author:小花熊
  */
#ifndef  __HDR_MRT_H__
#define __HDR_MRT_H__
#include<engine/GLObject.h>

class    HdrMrt :public  GLObject
{
private:
//帧缓冲区对象
	unsigned         _hdr_mrt_framebufferId;
	unsigned         _origin_framebufferId;
//location=0的纹理附着
	unsigned         _normal_textureId;
//location=1的纹理附着
	unsigned         _hdr_textureId;
//深度纹理附着
	unsigned         _depth_textureId;
	int                   _framebuffer_width;
	int                   _framebuffer_height;
private:
	HdrMrt();
	HdrMrt(HdrMrt &);
	void                 initHdrMrt(int    width,int   height);
public:
	~HdrMrt();
	static         HdrMrt			*createHdrMrt(int  width,int  height);
//获取location=0的纹理对象
	unsigned        normalTexture();
//获取location=1的纹理对象
	unsigned        hdrTexture();
//绑定帧缓冲区对象
	void                activateFramebuffer();
//恢复原来的帧缓冲区对象绑定
	void                resumeFramebuffer();
};
#endif
