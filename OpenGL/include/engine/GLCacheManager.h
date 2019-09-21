/*
*程序对象,纹理对象缓存管理
*2016-6-17 18:38:16
*version:1.0
*小花熊
  */
//Version 2.0:修正了程序对象,纹理对象的引用计数bug,以及不能再程序退出时调用析构函数的bug
#ifndef  __GL_CACHE_MANAGER_H__
#define __GL_CACHE_MANAGER_H__
#include<engine/GLState.h>
#include<engine/GLProgram.h>
#include<engine/GLTexture.h>
#include<map>
#include<string>
//全局单子
__NS_GLK_BEGIN

class  GLCacheManager
{
public:
	enum GLProgramType
	{
		GLProgramType_TextureColor = 0,//通用着色器
		GLProgramType_LightParallel = 1,//平行光光照着色器
		GLProgramType_ShadowMap = 2,//常规SM阴影着色器
		GLProgramType_LightSpaceShadowMap = 3,//光空间透视阴影着色器
		GLProgramType_DebugNormalTexture = 4,//调试常规的纹理
		GLProgramType_DebugDepthTexture = 5,//调试深度纹理
		GLProgramType_FuzzyBoxTexture = 6,//box模糊纹理
		GLProgramType_FuzzyBoxTextureVSM = 7,//针对VSM而存在的纹理模糊
		GLProgramType_Number = 8,
	};
private:
	std::map<GLProgramType, GLProgram *>  _glProgramCache;
	std::map<std::string, GLTexture *>   _glTextureCache;
	unsigned		_bufferIdentity;//单位顶点缓冲区对象
private:
	GLCacheManager(GLCacheManager &);
	GLCacheManager();
private:
	static    GLCacheManager              _glCacheManager;
//向程序对象缓存中加入程序对象,注意,引擎的使用者不要调用这个函数,这个函数只能在GLProgram中调用
	friend   class    GLProgram;
	void      inserGLProgram(GLProgramType type, GLProgram *);
public:
	static    GLCacheManager  *getInstance();
	~GLCacheManager();
//给定名字查找程序对象,如过没有找到,返回NULL
	GLProgram            *findGLProgram(GLProgramType type);
//给定名字返回纹理对象,如过没有找到,返回NULL
	GLTexture              *findGLTexture(std::string  &);
//插入纹理
	void                          insertGLTexture(std::string &,GLTexture  *);
//返回单位数组缓冲区
	unsigned                 loadBufferIdentity();
};
__NS_GLK_END
#endif