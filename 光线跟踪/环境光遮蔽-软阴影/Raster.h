/*
  *��դ������Ⱦ��,���Ժ�İ汾��,���ǽ�ʵ����Ȼ���������
  *@version:1.0
  *2018��4��26��
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
	//��������������
	void        swapBuffers(HDC  hdc);
};
#endif