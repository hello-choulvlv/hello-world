/*
 *��Ӱ
 *2016-6-16 20:17:09
*/
#ifndef  __GLSHADOWMAP_H__
#define __GLSHADOWMAP_H__
#include<engine/GLObject.h>
class        GLShadowMap :public  GLObject
{
private:
	unsigned     int        _shadowMapId;//��Ӱ�������
	unsigned     int        _shadowFramebufferId;//��Ӱ֡����������
private:
	GLShadowMap(GLShadowMap  &);
	GLShadowMap();
	void       initWithSize(int  width, int height);
public:
	~GLShadowMap();
	static    GLShadowMap         *createWithSize(int width, int  height);
	//��ȡframebuffer Id
	unsigned    int         framebuffer();
	//��ȡ��Ӱ�������
	unsigned  int          shadowMap();
	//��֡����������
	void            bindFramebuffer();
};
#endif