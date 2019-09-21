/*
  *光线跟踪纹理采样实现
  *2018年5月18日
  *@author:xiaohuaxiong
 */
#include "Texture.h"
#include<assert.h>

Texture::Texture():
	_colorBuffer(nullptr),
	_width(0),
	_height(0)
{
}

Texture::~Texture()
{
	delete _colorBuffer;
	_colorBuffer = nullptr;
}

bool Texture::loadTexture(const std::string &filename)
{
#if defined(_DEBUG) || defined(DEBUG)
	_filename = filename;
#endif
	delete _colorBuffer;
	_colorBuffer = nullptr;
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename.c_str());
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(filename.c_str());

	assert(fif != FIF_UNKNOWN);
	FIBITMAP *dib = nullptr;
	if (FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, filename.c_str());
	assert(dib != nullptr);

	_width= FreeImage_GetWidth(dib);
	_height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	int bpp = FreeImage_GetBPP(dib);

	assert(_width != 0 && _height != 0);

	BYTE *Bits = FreeImage_GetBits(dib);
	assert(Bits != nullptr);
	assert(bpp == 24 || bpp == 32);

	_colorBuffer = new unsigned char[_width * _height * 3];
	bpp = bpp / 8;

	BYTE *data = _colorBuffer, *line = Bits;

	for (int y = 0; y < _height; y++)
	{
		BYTE *pixel = line;

		for (int x = 0; x < _width; x++)
		{
			data[0] = pixel[2];
			data[1] = pixel[1];
			data[2] = pixel[0];

			pixel += bpp;
			data += 3;
		}

		line += pitch;
	}
	FreeImage_Unload(dib);
	return true;
}

float_3   Texture::getColorNearest(float s, float t)const
{
	//取整
	s -= (int)s;
	t -= (int)t;

	if (s < 0) s += 1.0f;
	if (t < 0)s += 1.0f;

	int    d_x = _width * s + 0.5f;
	int    d_y = _height * t + 0.5f;

	d_x %= _width;
	d_y %= _height;
	unsigned char *buffer = _colorBuffer + (d_y *_width	+d_x) * 3;

	return float_3(buffer[0]/255.0f,buffer[1]/255.0f,buffer[2]/255.0f);
}

float_3 Texture::getColorBilinear(float s, float t)const
{
	//取整
	s -= (int)s;
	t -= (int)t;

	if (s < 0)s += 1.0f;
	if (t < 0)t += 1.0f;

	int d_x = s * _width + 0.5f;
	int d_y = t*_height + 0.5f;

	d_x %= _width;
	d_y %= _height;

	int  d_x_1 = (d_x -1)>=0?(d_x-1):(d_x-1 + _width);
	int  d_y_1 = (d_y-1)>=0?(d_y-1):(d_y-1+_height);

	float   u = 1-s,v = 1-t;
	float   uv = u*v;
	float   st = s*t;
	float   ut = u*t;
	float   sv = s*v;

	unsigned char *buffer_22 = _colorBuffer + (d_y*_width + d_x)*3;
	unsigned char *buffer_11 = _colorBuffer + (d_y_1*_width + d_x_1)*3;
	unsigned char *buffer_21 = _colorBuffer + (d_y_1 * _width + d_x)*3;
	unsigned char *buffer_12 = _colorBuffer + (d_y*_width+d_x_1)*3;

	float r = buffer_11[0] * uv + buffer_22[0] * st + buffer_12[0] * ut + buffer_21[0] * sv;
	float g = buffer_11[1] * uv + buffer_22[1]  * st + buffer_12[1] * ut + buffer_21[1]  * sv;
	float b = buffer_11[2] * uv + buffer_22[2] * st + buffer_12[2] * ut + buffer_21[2] * sv;

	return float_3(r/255.0f,g/255.0f,b/255.f);
}