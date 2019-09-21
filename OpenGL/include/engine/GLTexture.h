/*
  *纹理
  *2016-6-14 20:31:10
 */
#ifndef  __GLTEXTURE_H__
#define __GLTEXTURE_H__
#include<engine/Object.h>
#include<engine/Geometry.h>
__NS_GLK_BEGIN
struct TexParam
{
	int     minFilter;
	int     magFilter;
	int     wrapS;
	int     wrapT;
};
class  GLTexture:public Object
{
private:
	unsigned int      _textureId;
	Size       _contentSize;//图片的大小
private:
	GLTexture(GLTexture &);
	GLTexture();
	void      initWithFile(const char *_file_name);
public:
	~GLTexture();
	static   GLTexture    *createWithFile(const char  *_file_name);
	void         setWrapParam(unsigned wrap_param,unsigned _param);
	void         setTexParam(const TexParam &texParam);
//获取纹理的尺寸
	Size&      getContentSize();
	float     getWidth();
	float     getHeight();
//纹理的名字
	unsigned int  getName();
//设置Mipmap
	void        genMipmap();
};
//立方体贴图
/*
  *
  */
class   GLCubeMap :public Object
{
	unsigned         _cubeMapId;
	Size                  _contentSize;
private:
	GLCubeMap(GLCubeMap &);
	GLCubeMap();
//文件的排布依次为+X,-X,+Y,-Y,+Z,-Z
	void                initWithFiles(const  char   **file_list);
	void                initWithEmpty(Size    &contentSize);
public:
	~GLCubeMap();
	static     GLCubeMap	      *createWithFiles(const char **file_list);
	static     GLCubeMap      *createEmptyMap(Size    &contentSize);
	unsigned       getName();
};
__NS_GLK_END
#endif