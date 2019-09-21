/*
  *@aim:ϵͳ������
  &2016-3-7 16:56:00
  *version:2.0,�����ӵ�֧��,֧��8λ��ȵ�ͼƬ
  *version:3.0���Ӷ�BMPͼƬ��֧��
  *version:4.0��ǿ�˶�ͼƬ��֧�ֵĿ���չ��
  */
#include<engine/Tools.h>
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<map>
__NS_GLK_BEGIN

static  char  *vLoadBMP(FILE *fp, int *width, int *height, int *bitDepth);
static  char  *vLoadTGA(FILE *fp, int *width, int *height, int *bitDepth);
static  char  *vLoadTGA_NO_Change(FILE *fp, int *width, int *height, int *bitDepth);
/////////////////////////////////////////////////////////////////////////////////////
/*
  *����ͼƬ�ĺ�����
 */
struct  VImageFuncTable
{
public:
	std::map<int, char* (*)(FILE *, int *, int *, int *)>  vfuncTable;
private:
	VImageFuncTable();
public:
	static   VImageFuncTable        VFunTable;
	static   VImageFuncTable        *getInstance();
};
VImageFuncTable           VImageFuncTable::VFunTable;

VImageFuncTable::VImageFuncTable()
{
	vfuncTable[19778] = vLoadBMP;//���Լ���BMPͼƬ�ĺ���
	vfuncTable[0x20000] = vLoadTGA;//TGA����
	vfuncTable[65792]=vLoadTGA_NO_Change;
}
VImageFuncTable   *VImageFuncTable::getInstance()
{
	return &VFunTable;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool      Tools::littleEndin()
{
	      int     _endin=1;
		  return     *((char   *)(&_endin)) & 0x1;
}
//���ļ��ж�ȡ����
const   char  *Tools::getFileContent(const char *_file_name)
{
	          char      *_buff=NULL;
			  FILE      *_file=fopen(_file_name,"rb");

			  if( !_file   )
				        return    NULL;
			  fseek(_file,0,SEEK_END);
			  int      _size=(int)ftell(_file);

			  fseek(_file,0,SEEK_SET);
			  _buff=new   char[_size+2];
			  fread(_buff,1,_size,_file);
			  _buff[_size]='\0';

			  fclose(_file);
			  _file=NULL;
			  return   _buff;
}
int Tools::readFile(FILE *pFile, int bytesToRead, void *buffer)
{
	int bytesRead = 0;

#ifdef ANDROID
	bytesRead = AAsset_read(pFile, buffer, bytesToRead);
#else
	bytesRead = fread(buffer, bytesToRead, 1, pFile);
#endif

	return bytesRead;
}
FILE    *Tools::openFile(void *ioContext, const char *fileName)
{
	FILE *pFile = NULL;

#ifdef ANDROID

	if (ioContext != NULL)
	{
		AAssetManager *assetManager = (AAssetManager *)ioContext;
		pFile = AAssetManager_open(assetManager, fileName, AASSET_MODE_BUFFER);
	}

#else
#ifdef __APPLE__
	// iOS: Remap the filename to a path that can be opened from the bundle.
	fileName = GetBundleFileName(fileName);
#endif

	pFile = fopen(fileName, "rb");
#endif

	return pFile;
}
char  *vLoadTGA_NO_Change(FILE *fp, int *width, int *height, int *bitDepth)
{
	char  *buffer = NULL;
	TGA_HEADER   Header;
	int          bytesRead;

	bytesRead = Tools::readFile(fp, sizeof(TGA_HEADER), &Header);

	*width = Header.Width;
	*height = Header.Height;
	*bitDepth = Header.ColorDepth;
	if (Header.ColorDepth == 8 || Header.ColorDepth == 24 || Header.ColorDepth == 32)
	{
		int bytesToRead = sizeof(char) * (*width) * (*height) * Header.ColorDepth / 8;
		// Allocate the image data buffer
		buffer = new char[bytesToRead];
		bytesRead = Tools::readFile(fp, bytesToRead, buffer);
	}
	return buffer;
}
char * Tools::loadTGA( const char *fileName, int *width, int *height, int   *bitdepth)
{
	char        *buffer;
	FILE      *fp;
	// Open the file for reading
	fp = fopen(fileName, "rb");

	if (fp == NULL)
	{
		// Log error as 'error in opening the input file from apk'
		printf("esLoadTGA FAILED to load : { %s }\n", fileName);
		return NULL;
	}
	buffer=vLoadTGA(fp, width, height, bitdepth);
	fclose(fp);
	return  buffer;
}
//����BMP�ļ�
char    *Tools::loadBMP(const char *file_name, int *width, int *height, int *bitDepth)
{
	char   *buffer = NULL;
	FILE   *pFile;
	unsigned long lInfoSize = 0;
	unsigned long lBitSize = 0;
	int                   byteRead = 0;
// Attempt to open the file
	pFile = fopen(file_name, "rb");
	if (!pFile)
	{
		printf("file '%s' is not exist\n  ",file_name);
		fclose(pFile);
		assert(false);
	}
	buffer=vLoadBMP(pFile, width, height, bitDepth);
	fclose(pFile);
	return  buffer;
}
//BMP��ʽ
char  *vLoadBMP(FILE *fp,int *width,int *height,int *bitDepth)
{
	char   *buffer = NULL;
	BMPInfo     bitmapInfo;
	unsigned long lInfoSize = 0;
	unsigned long lBitSize = 0;
	BMPHeader	bitmapHeader;
	int                   byteRead = 0;
	// Attempt to open the file
	// File is Open. Read in bitmap header information
	byteRead = fread(&bitmapHeader, sizeof(BMPHeader), 1, fp);
	assert(byteRead == 1);
	// Read in bitmap information structure
	lInfoSize = bitmapHeader.offset - sizeof(BMPHeader);
	assert(fread(&bitmapInfo, lInfoSize, 1, fp) == 1);

	// Save the size and dimensions of the bitmap
	*width = bitmapInfo.header.width;
	*height = abs((int)bitmapInfo.header.height);
	*bitDepth = bitmapInfo.header.bits;
	lBitSize = bitmapInfo.header.imageSize;

	// If the size isn't specified, calculate it anyway	
	// Allocate space for the actual bitmap
	if (lBitSize == 0)  lBitSize = (*width *bitmapInfo.header.bits + 7) / 8 * abs(*height);
	buffer = new char[lBitSize];

	// Read in the bitmap bits, check for corruption
	byteRead = fread(buffer, lBitSize, 1, fp);
	assert (byteRead == 1);
//�����ֽ�����
	if (bitmapInfo.header.bits == 32 || bitmapInfo.header.bits == 24)
	{
		const int step = bitmapInfo.header.bits >> 3;
		int                         i;
		for (i = 0; i < lBitSize; i += step)
		{
			const      char        b = buffer[i];
			buffer[i] = buffer[i + 2];
			buffer[i + 2] = b;
		}
	}
	// Close the bitmap file now that we have all the data we need
	return buffer;
}

 char  *vLoadTGA(FILE *fp, int *width, int *height, int *bitDepth)
{
	char  *buffer = NULL;
	TGA_HEADER   Header;
	int          bytesRead;

	bytesRead = Tools::readFile(fp, sizeof(TGA_HEADER), &Header);

	*width = Header.Width;
	*height = Header.Height;
	*bitDepth = Header.ColorDepth;
	if (Header.ColorDepth == 8 || Header.ColorDepth == 24 || Header.ColorDepth == 32)
	{
		   int bytesToRead = sizeof(char) * (*width) * (*height) * Header.ColorDepth/8;
		// Allocate the image data buffer
		   buffer = new char[bytesToRead];
			bytesRead = Tools::readFile(fp, bytesToRead, buffer);
			if (Header.ColorDepth >= 24)
			{
				const      int     _step = Header.ColorDepth >> 3;
				int                         i;
				for (i = 0; i < bytesToRead; i += _step)
				{
					const      char        b = buffer[i];
					buffer[i] = buffer[i + 2];
					buffer[i + 2] = b;
				}
			}
	}
	return buffer;
}
char  *Tools::loadImage(const char *file_name, int *width, int *height, int *bitDepth)
{
	//���ļ�
	FILE          *fp = NULL;
	int             _size;
	int             byteRead = 0;
	int             fileType[4];
	unsigned   char  image_header[6];
	//���ȼ���ΪBMP�ļ�
	fp = fopen(file_name, "rb");
	if (!fp)
	{
		printf("file '%s' is not exist.\n", file_name);
		assert(false);
	}
	//��ȡͷ�ĸ��ֽ�
	byteRead = fread(image_header, 1, 6, fp);
	if (byteRead != 6)
	{
		printf("file '%s' is not a legal image file.\n", file_name);
		fclose(fp);
		assert(false);
	}
	fileType[0] = image_header[0] + (image_header[1] << 8);//BMP
	fileType[1] = image_header[0] + (image_header[1] << 8) + (image_header[2] << 16) + (image_header[3] << 24);//TGA
	//��֤ͼƬ��ʽ
	
	char  *(*vfunc)(FILE *, int *, int *, int *) = NULL;
	for (int index = 0; index < 2; ++index)
	{
		std::map<int, char *(*)(FILE *, int *, int *, int *)>::iterator   it = VImageFuncTable::VFunTable.vfuncTable.find(fileType[index]);
		if (it != VImageFuncTable::VFunTable.vfuncTable.end())
		{
			vfunc = it->second;
			break;
		}
	}
	if (! vfunc)//���û�в��ҵ���Ӧ�ĺ���ָ��,����
	{
		printf("File '%s' is not a image file that could be supported by engine\n", file_name);
		fclose(fp);
		assert(false);
	}
//�ļ�ָ�����
	fseek(fp, 0, SEEK_SET);
	char  *buffer = vfunc(fp, width, height, bitDepth);
//����TGA�ļ�����
	fclose(fp);
	return buffer;
}

__NS_GLK_END