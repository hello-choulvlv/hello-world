/*
  *@aim:TGA image data
  *&2016-3-23 14:46:12
  */
#ifndef    __TGA_IMAGE_H__
#define   __TGA_IMAGE_H__
//��Ӱ
#include<engine/Object.h>
__NS_GLK_BEGIN
//Ŀǰ�Ѿ�������,����Ĵ���ֻ��Ϊ�˼�����ǰ�Ĵ���
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
	//������ĸ�ʽ��������,Ŀǰֻ֧��24λ��32λ����
	unsigned      genTextureMap();
	unsigned      genTextureMap(int    _wrapAttrib);
	unsigned  genVertexBuffer();//���ɶ��㻺��������
//������������
	void          setImageWrapAttrib(int    _wrapAttrib);
	//����3D��������
	static        unsigned   int    gen3DNoiseTextureMap(int textureSize, float frequency);
};
//����������
class   TGAImageCubeMap
{
private:
	int                      _cubeMapId;
	const     char     *_imageName[6];
private:
	TGAImageCubeMap(TGAImageCubeMap &);
public:
	TGAImageCubeMap(const char *imageName[6]);//����-x,x,-y,y,-z,z���ͼƬ��·��
	~TGAImageCubeMap();
	int        genTextureCubeMap();
};

__NS_GLK_END
#endif