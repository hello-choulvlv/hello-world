/*
  *��������
  *@Version 1.0:ʵ��һά��������,��ά��������,��ά��������
  *@Author:С����
  *2016-8-24 19:48:29
*/
#ifndef   __NOISE_TEXTURE_H__
#define  __NOISE_TEXTURE_H__
#include<engine/Object.h>
#include<stdlib.h>
#include<stdio.h>
__NS_GLK_BEGIN
//frequency��ʾ������Ƶ��,Ƶ��Խ��,չ�ֵ�ϸ�ھ�Խ�ḻ,��Ȼ���ξ�Խ������
//һά����
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
	unsigned      name();//���������OpenGL����
	float                 getWidth(){ return  _width; };
};
//��ά��������
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
//��ά��������
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
//�����ı�Ƶ����,ע������һ��������
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