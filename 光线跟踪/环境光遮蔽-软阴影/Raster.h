/*
  *光栅化渲渲染器,在以后的版本中,我们将实现深度缓冲区对象
  *@version:1.0
  *2018年4月26日
  *@author:xiaohuaxiong
 */
#ifndef __RASTER_H__
#define __RASTER_H__
#include <windows.h>
class Raster
{
public:
	unsigned char   *_colorBuffer;
	BITMAPINFO   _bitmap;
	HWND                _windowHandler;
	int                        _width, _height,_lineWidth;
public:
	Raster(int windowWidth,int windowHeight);
	static    Raster    *create(int w,int h);
	void          init(int w,int h);
	~Raster();
	//交换缓冲区对象
	void        swapBuffers(HDC  hdc);
};
#endif