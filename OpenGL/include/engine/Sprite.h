/*
  *精灵,2D图片绘制的工具
  *2016-6-16 20:50:42
  *小花熊
  */
#ifndef  __SPRITE_H__
#define __SPRITE_H__
#include<engine/Object.h>
#include<engine/GLTexture.h>
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
__NS_GLK_BEGIN
//这里我们假设使用者已经完全掌握了OpenGL的矩阵变换,所以精灵的表现完全按照每一步操作所对应的矩阵内容来表现的
//这样会使得它的使用更灵活,但是却比较难以掌控,使用者需要多加注意
class  Sprite :public Object
{
//纹理
private:
	GLTexture	    *_glTexture;
	GLProgram    *_glProgram;
	unsigned         _vertexVBO;//顶点缓冲区对象
	unsigned         _vertexArrayBufferId;
	//uniform variable index
	int                   _baseMapLoc;//纹理的uniform索引
	int                   _mvMatrixLoc;//模型视图矩阵的索引
	int                   _renderColorLoc;//精灵的颜色索引
//	GLVector3      _position;//位置
//	GLVector3      _scale;
	GLVector4      _renderColor;//精灵的颜色
	Matrix        _mvMatrix;//变换矩阵
private:
	Sprite(Sprite &);
	Sprite();
	void         initWithFile(const  char  *file_name);
public:
	~Sprite();
	static   Sprite            *createWithFile(const char  *file_name);
//设置渲染颜色
	void         setRenderColor(GLVector4  *);
//设置精灵的变换矩阵
	void        setAffineMatrix(Matrix   *);
//获取变换矩阵
	Matrix        *getAffineMattix();
//绘制精灵,注意,如果使用了不带输入参数的函数,则使用对象自带的矩阵,否则使用输入矩阵
	//note:此函数不能在绘制其他图像的中间步骤中调用,否则有可能出现不确定的行为
	void        render();
	void        render(Matrix   *,unsigned otherTextureId=0);
//获取精灵的大小
	Size        &getContentSize();
//设置Mipmap
	void       setMipmap();
	GLProgram		*getGLProgram(){return  _glProgram;};
};
__NS_GLK_END
#endif