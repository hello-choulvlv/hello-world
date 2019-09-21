/*
  *@aim:TGA image data
  *&2016-3-23 14:46:12
  */
#ifndef    __TGA_IMAGE_H__
#define   __TGA_IMAGE_H__
//阴影
#include<engine/Object.h>
__NS_GLK_BEGIN
//目前已经废弃了,该类的存在只是为了兼容以前的代码
__declspec(deprecated)
class       TGAImage
{
	//container
	unsigned     char      *_buffer;
	float                               _width;
	float                               _height;
	int                                  _bitDepth;
	unsigned    int               _textureId;
private:
	TGAImage(TGAImage   &);
public:
	TGAImage(const   char   *image_name);
	~TGAImage();
	inline    float           getWidth(){ return  _width; };
	inline    float           getHeight(){ return   _height; };
	void      *getImageSource(){ return   _buffer; };
	//由纹理的格式创建纹理,目前只支持24位与32位纹理
	unsigned      genTextureMap();
	unsigned      genTextureMap(int    _wrapAttrib);
	unsigned  genVertexBuffer();//生成顶点缓冲区对象
//设置纹理属性
	void          setImageWrapAttrib(int    _wrapAttrib);
	//创建3D噪声纹理
	static        unsigned   int    gen3DNoiseTextureMap(int textureSize, float frequency);
};
//立方体纹理
class   TGAImageCubeMap
{
private:
	int                      _cubeMapId;
	const     char     *_imageName[6];
private:
	TGAImageCubeMap(TGAImageCubeMap &);
public:
	TGAImageCubeMap(const char *imageName[6]);//输入-x,x,-y,y,-z,z轴的图片的路径
	~TGAImageCubeMap();
	int        genTextureCubeMap();
};

__NS_GLK_END
#endif