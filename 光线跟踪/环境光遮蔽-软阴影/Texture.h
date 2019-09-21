/*
  *����ʵ��,Ŀǰֻʵ���˾ͽ�����,˫���Բ���,���Ʒ�ʽΪrepeat
  *2018��5��18��
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
	//�ͽ�����,ʹ�õ�λ��������
	float_3 getColorNearest(float s,float t)const;
	//˫���Բ���
	float_3  getColorBilinear(float s,float t)const;
};
#endif
