/*
  *光栅化渲染器
  *2018年4月26日
  *@author:xiaohuaxiong
 */
#include "Raster.h"
#include<stdlib.h>
#include<assert.h>
Raster::Raster(int width,int height):
	_colorBuffer(nullptr),
	_width(width),
	_height(height)
{
}

Raster::~Raster()
{
	delete[] _colorBuffer;
}

Raster *Raster::create(int w, int h)
{
	Raster *r = new Raster(w,h);
	r->init(w,h);
	return r;
}

void Raster::init(int w, int h)
{
	//w,h必须是4的倍数
	assert(!(w&0x3) && !(h&0x3));
	memset(&_bitmap,0,sizeof(_bitmap));
	_bitmap.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	_bitmap.bmiHeader.biPlanes = 1;
	_bitmap.bmiHeader.biBitCount = 24;
	_bitmap.bmiHeader.biCompression = BI_RGB;
	_bitmap.bmiHeader.biWidth = w;
	_bitmap.bmiHeader.biHeight = h;
	//
	_colorBuffer = new byte[w*h*3];
}

void Raster::swapBuffers(HDC hdc)
{
	StretchDIBits(hdc, 0, 0, _width, _height, 0, 0, _width, _height, _colorBuffer, &_bitmap, DIB_RGB_COLORS, SRCCOPY);
}