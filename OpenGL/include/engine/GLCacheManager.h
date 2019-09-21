/*
*�������,������󻺴����
*2016-6-17 18:38:16
*version:1.0
*С����
  */
//Version 2.0:�����˳������,�����������ü���bug,�Լ������ٳ����˳�ʱ��������������bug
#ifndef  __GL_CACHE_MANAGER_H__
#define __GL_CACHE_MANAGER_H__
#include<engine/GLState.h>
#include<engine/GLProgram.h>
#include<engine/GLTexture.h>
#include<map>
#include<string>
//ȫ�ֵ���
__NS_GLK_BEGIN

class  GLCacheManager
{
public:
	enum GLProgramType
	{
		GLProgramType_TextureColor = 0,//ͨ����ɫ��
		GLProgramType_LightParallel = 1,//ƽ�й������ɫ��
		GLProgramType_ShadowMap = 2,//����SM��Ӱ��ɫ��
		GLProgramType_LightSpaceShadowMap = 3,//��ռ�͸����Ӱ��ɫ��
		GLProgramType_DebugNormalTexture = 4,//���Գ��������
		GLProgramType_DebugDepthTexture = 5,//�����������
		GLProgramType_FuzzyBoxTexture = 6,//boxģ������
		GLProgramType_FuzzyBoxTextureVSM = 7,//���VSM�����ڵ�����ģ��
		GLProgramType_Number = 8,
	};
private:
	std::map<GLProgramType, GLProgram *>  _glProgramCache;
	std::map<std::string, GLTexture *>   _glTextureCache;
	unsigned		_bufferIdentity;//��λ���㻺��������
private:
	GLCacheManager(GLCacheManager &);
	GLCacheManager();
private:
	static    GLCacheManager              _glCacheManager;
//�������󻺴��м���������,ע��,�����ʹ���߲�Ҫ�����������,�������ֻ����GLProgram�е���
	friend   class    GLProgram;
	void      inserGLProgram(GLProgramType type, GLProgram *);
public:
	static    GLCacheManager  *getInstance();
	~GLCacheManager();
//�������ֲ��ҳ������,���û���ҵ�,����NULL
	GLProgram            *findGLProgram(GLProgramType type);
//�������ַ����������,���û���ҵ�,����NULL
	GLTexture              *findGLTexture(std::string  &);
//��������
	void                          insertGLTexture(std::string &,GLTexture  *);
//���ص�λ���黺����
	unsigned                 loadBufferIdentity();
};
__NS_GLK_END
#endif