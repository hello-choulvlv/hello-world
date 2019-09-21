/*
  *噪声纹理
  *@Version 1.0:实现一维噪声纹理,二维噪声纹理,三维噪声纹理
  *@Author:小花熊
  *2016-8-24 19:48:29
*/
#ifndef   __NOISE_TEXTURE_H__
#define  __NOISE_TEXTURE_H__
#include<engine/Object.h>
#include<stdlib.h>
#include<stdio.h>
__NS_GLK_BEGIN
//frequency表示噪声的频率,频率越大,展现的细节就越丰富,当然外形就越不明显
//一维纹理
class      NoiseTexture:public Object
{
private:
	unsigned      _noiseTextureId;
	float              _width;
private:
	NoiseTexture(NoiseTexture &);
	NoiseTexture();
	void              initWithSeed(unsigned seed,float   frequency,int  width);
public:
	~NoiseTexture();
	static  NoiseTexture		*createWithSeed(unsigned seed,int  frequency,int   width);
	unsigned      name();//返回纹理的OpenGL名字
	float                 getWidth(){ return  _width; };
};
//二维噪声纹理
class   NoiseTexture2 :public  Object
{
private:
	unsigned         _noiseTextureId;
	float                 _width;
private:
	NoiseTexture2();
	NoiseTexture2(NoiseTexture2 &);
	void                 initWithSeed(unsigned seed, float   frequency,int  width);
public:
	~NoiseTexture2();
	static  NoiseTexture2	*createWithSeed(unsigned  seed, float   frequency,int  width);
	unsigned         name();
	float                 getWidth(){ return _width; };
};
//三维噪声纹理
class    NoiseTexture3 :public Object
{
private:
	unsigned      _noiseTextureId;
	float              _width;
private:
	NoiseTexture3();
	NoiseTexture3(NoiseTexture3 &);
	void            initWithSeed(unsigned seed, float   frequency,int width);
public:
	~NoiseTexture3();
	static  NoiseTexture3   *createWithSeed(unsigned  seed,float  frequency,int  width);
	unsigned      name();
	float             getWidth(){ return  _width; };
};
//连续的倍频噪声,注意这是一个立方体
class   NoiseConsistency :public  Object
{
private:
	unsigned     _noiseTextureId;
	float             _frequency;
	float             _width;
private:
	NoiseConsistency();
	NoiseConsistency(NoiseConsistency &);
	void          initWithSeed(unsigned   seed,float   frequency,int  width);
public:
	~NoiseConsistency();
	static  NoiseConsistency   *createWithSeed(unsigned seed,float      frequency,int  width);
	unsigned                 name();
	float                         getWidth();
};
__NS_GLK_END
#endif