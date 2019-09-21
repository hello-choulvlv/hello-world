/*
 *阴影
 *2016-6-16 20:17:09
*/
#ifndef  __GLSHADOWMAP_H__
#define __GLSHADOWMAP_H__
#include <engine/GLState.h>
#include<engine/GLObject.h>
__NS_GLK_BEGIN
class        GLShadowMap :public  GLObject
{
private:
	unsigned     int        _shadowMapId;//阴影纹理对象
	unsigned     int        _shadowFramebufferId;//阴影帧缓冲区对象
private:
	GLShadowMap(GLShadowMap  &);
	GLShadowMap();
	void       initWithSize(int  width, int height);
public:
	~GLShadowMap();
	static    GLShadowMap         *createWithSize(int width, int  height);
	//获取framebuffer Id
	unsigned    int         framebuffer();
	//获取阴影纹理对象
	unsigned  int          shadowMap();
	//绑定帧缓冲区对象
	void            bindFramebuffer();
};
__NS_GLK_END
#endif