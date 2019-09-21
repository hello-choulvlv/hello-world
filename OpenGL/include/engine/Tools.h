/*
  *@aim:系统函数库,常用的各种辅助函数
  &2016-3-7 16:50:13
  */
#ifndef    __HELP_H__
#define   __HELP_H__
#include<stdio.h>
#include<engine/GLState.h>
__NS_GLK_BEGIN
//// tga header
#ifndef __APPLE__
#pragma pack(push,x1)                            // Byte alignment (8-bit)
#pragma pack(1)
#endif
typedef struct
#ifdef __APPLE__
__attribute__((packed))
#endif
{
	unsigned char  IdSize,
		MapType,
		ImageType;
	unsigned short PaletteStart,
		PaletteSize;
	unsigned char  PaletteEntryDepth;
	unsigned short X,
		Y,
		Width,
		Height;
	unsigned char  ColorDepth,
		Descriptor;

} TGA_HEADER;

#ifndef __APPLE__
#pragma pack(pop,x1)
#endif
////////////////////////////BMP文件结构//////////////////
#pragma pack(1)
struct   BMPRGB {
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char alpha;
};
struct BMPInfoHeader {
	unsigned int	size;
	unsigned int	width;
	unsigned int	height;
	unsigned short  planes;
	unsigned short  bits;
	unsigned int	compression;
	unsigned int	imageSize;
	unsigned int	xScale;
	unsigned int	yScale;
	unsigned int	colors;
	unsigned int	importantColors;
};
struct BMPHeader {
	unsigned short	type;
	unsigned int    	size;
	unsigned short	unused;
	unsigned short	unused2;
	unsigned int  	    offset;
};
struct BMPInfo {
	BMPInfoHeader		header;
	BMPRGB				colors[1];
};
#pragma pack(8)
///////////////////
class     Tools
{
//禁止创建类的对象
private:
	Tools();
	Tools(Tools &);
	~Tools();
public:
//是否是小尾端序
static    bool        littleEndin();
//从给定的文件中读取文件内容,如果打开文件失败,返回NULL
static    const      char     *getFileContent(const  char *_file_name);
//
static int readFile(FILE *pFile, int bytesToRead, void *buffer);
//
static FILE *openFile(void *ioContext, const char *fileName);
//加载TGA文件
static char *loadTGA( const char *fileName, int *width, int *height, int   *bitdepth);

//加载BMP文件
static  char *loadBMP(const char *file_name,int *width,int *height,int *bitDepth);

//加载图像文件,目前只支持BMP,TGA两种
static  char  *loadImage(const char *file_name,int *width,int *height,int *bitDepth);
};
__NS_GLK_END
#endif