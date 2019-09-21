/*
  *纹理实现,目前只实现了就近采样,双线性采样,环绕方式为repeat
  *2018年5月18日
  *@author:xiaohuaxiong
 */
#ifndef  __TEXTURE_H__
#define __TEXTURE_H__
#include "FreeImage.h"
#include "engine/Geometry.h"
#include<string>
class Texture
{
#if defined(_DEBUG) || defined(DEBUG)
	std::string _filename;
#endif
	unsigned char *_colorBuffer;
	int             _width, _height;
public:
	Texture();
	~Texture();
	bool     loadTexture(const std::string &filename);
	//就近采样,使用单位纹理坐标
	float_3 getColorNearest(float s,float t)const;
	//双线性采样
	float_3  getColorBilinear(float s,float t)const;
};
#endif
