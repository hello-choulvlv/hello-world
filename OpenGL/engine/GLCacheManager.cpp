/*
 *OpenGL�������,������󻺴�ʵ��
 *2016-6-17 18:47:11
 *version:1.0
 *С����
  */
#include<GL/glew.h>
#include<engine/GLCacheManager.h>
#include<engine/GLState.h>
#include<assert.h>
__NS_GLK_BEGIN
GLCacheManager    GLCacheManager::_glCacheManager;
//��Ƕ�ĳ������Դ����
//��ͨ��ɫ��
static   const  char  *static_SahderTextureColor_Vert ="#version 330 core\n" \
"precision highp float;\
uniform  mat4   g_MVPMatrix;  \
layout(location=0)in   vec4    a_position;\
layout(location = 1)in   vec2    a_texCoord;\
out       vec2       v_texCoord;\
\
void  main()\
{\
	    gl_Position = g_MVPMatrix*a_position;\
	    v_texCoord = a_texCoord;\
}";
static   const  char   *static_ShaderTextureColor_Frag ="#version 330 core\n" \
"precision highp float;\
uniform   vec4      g_Color;                 \
uniform     sampler2D     g_BaseMap;\
layout(location = 0)out     vec4    outColor;\
in       vec2     v_texCoord;\
\
void    main()\
{\
	    outColor = texture(g_BaseMap, v_texCoord)*g_Color;\
}";
//һ�������ɫ��
 static   const   char      *static_ShaderParallelLightNormal_Vert ="#version 330 core\n" \
	 "precision  highp  float;\
    uniform                  mat4      g_MVPMatrix;\
	uniform                  mat3      g_NormalMatrix;\
    layout(location = 0)in     vec4      a_position;\
    layout(location = 1)in     vec2      a_fragCoord;\
    layout(location = 2)in     vec3      a_normal;\
	out     vec2    v_texCoord;\
	out     vec3    v_normal;\
	void    main()\
	{\
		v_normal=normalize(g_normalMatrix*a_normal);\
		\
		v_texCoord = a_fragCoord;\
		gl_Position = g_MVPMatrix*a_position;\
}";
 static   const   char      *static_ShaderParallelLightNormal_Frag = "#version 330 core\n" \
	"precision    highp    float;\
	uniform      sampler2D       g_BaseMap;\
    uniform      vec4                  g_LightColor;\
	uniform      vec3                  g_LightDir;\
    layout(location = 0)out      vec4     outColor;\
    in           vec2            v_texCoord;\
    in           vec3            v_normal;\
    \
    void     main()\
{\
	    vec4      baseColor = texture(g_BaseMap, v_texCoord);\
	    float     dotL = max(0.0, dot(normalize(v_normal), g_LightDir));\
	    \
	outColor = baseColor*vec4(0.10, 0.10, 0.10, 0.10) + dotL*baseColor*g_LightColor;\
}";
//ShadowMap��ɫ��
 static const char *static_ShaderShadowMap_Vert = "#version 330 core\n"
	 "layout(location=0)in	vec4	a_position;\n"
	 "uniform mat4	g_MVPMatrix;\n"
	 "void		main()\n"
	 "{\n"
	 "		gl_Position = g_MVPMatrix * a_position;\n"
	 "}";
 static const char *static_ShaderShadowMap_Frag = "#version 330 core\n"
	 "precision highp float;\n"
	 "void		main()\n"
	 "{\n"
	 "}";
 //��������
 static const char *static_ShaderDebugTexture_Vert = "#version 330 core\n"
	 "layout(location=0)in	vec4	a_position;\n"
	 "layout(location=1)in	vec2	a_fragCoord;\n"
	 "uniform mat4 g_MVPMatrix;\n"
	 "out	vec2	v_fragCoord;\n"
	 "void	main(){\n"
	 "	gl_Position = g_MVPMatrix * a_position;\n"
	 " v_fragCoord = a_fragCoord;\n"
	 "}\n";
 static const char *static_ShaderDebugTexture_Frag = "#version 330 core\n"
	 "layout(location=0)out		vec4    outColor;\n"
	 "uniform	sampler2D    g_BaseMap;\n"
	 "in		vec2	v_fragCoord;\n"
	 "void		main(){\n"
	 "  outColor = texture(g_BaseMap,v_fragCoord);\n"
	 "}\n";
 //�����������
 static const char *static_ShaderDebugDepthTexture_Vert = "#version 330 core\n"
	 "layout(location=0)in	vec4	a_position;\n"
	 "layout(location=1)in	vec2	a_fragCoord;\n"
	 "uniform mat4 g_MVPMatrix;\n"
	 "out	vec2	v_fragCoord;\n"
	 "void	main(){\n"
	 "	gl_Position = g_MVPMatrix * a_position;\n"
	 " v_fragCoord = a_fragCoord;\n"
	 "}\n";
 static const char *static_ShaderDebugDepthTexture_Frag = "#version 330 core\n"
	 "precision highp float;\n"
	 "layout(location=0)out		vec4    outColor;\n"
	 "uniform	sampler2D    g_BaseMap;\n"
	 "in		vec2	v_fragCoord;\n"
	 "void		main(){\n"
	 "  float r = texture(g_BaseMap,v_fragCoord).r;\n"
	 "	 outColor = vec4(r);\n"
	 "}\n";
 //box��ʽģ������
 static const char *static_ShaderFuzzyBoxTexture_Vert = "#version 330 core\n"
	 "layout(location=0)in	vec4    a_position;\n"
	 "layout(location=1)in	vec2	a_fragCoord;\n"
	 "out	vec2  v_fragCoord;\n"
	 "void  main()\n"
	 "{\n"
	 "		gl_Position = a_position;\n"
	 "		v_fragCoord = a_fragCoord;\n"
	 "}\n";
 static const char *static_ShaderFuzzyBoxTexture_Frag = "#version 330 core\n"
	 "precision highp float;\n"
	 "layout(location=0)out		vec4    outColor;\n"
	 "uniform    sampler2D    g_BaseTexture;\n"
	 "uniform    vec2                g_PixelStep;\n"
	 "uniform    int                   g_SampleCount;\n"
	 "in		vec2    v_fragCoord;\n"
	 "void		main()\n"
	 "{\n"
	 "		float    samples = float(g_SampleCount);\n"
	 "		vec2    offset = v_fragCoord  - g_PixelStep * (samples -1.0) * 0.5;\n"
	 "		outColor = vec2(0.0);\n"
	 "		for(int k=0;k<g_SampleCount;++k)\n"
	 "			outColor += texture(g_BaseTexture,offset + float(k) * g_PixelStep);\n"
	 "     outColor/=samples;\n"
	 "}\n";
 static const char *static_ShaderFuzzyBoxTextureVSM_Frag = "#version 330 core\n"
	 "precision highp float;\n"
	 "layout(location=0)out		vec2    outColor;\n"
	 "uniform    sampler2D    g_BaseTexture;\n"
	 "uniform    vec2                g_PixelStep;\n"
	 "uniform    int                   g_SampleCount;\n"
	 "in		vec2    v_fragCoord;\n"
	 "void		main()\n"
	 "{\n"
	 "		float    samples = float(g_SampleCount);\n"
	 "		vec2    offset = v_fragCoord - g_PixelStep * (samples -1.0) * 0.5;\n"
	 "		outColor = vec2(0.0);\n"
	 "		for(int k=0;k<g_SampleCount;++k)\n"
	 "			outColor += texture(g_BaseTexture,offset + float(k) * g_PixelStep).rg;\n"
	 "     outColor/=samples;\n"
	 "}\n";
//��¼�������Դ����ļ�¼��
struct   ProgramSourceRecord
{
	const  char  *vert_src;
	const  char  *frag_src;
};
static   ProgramSourceRecord      static_EmbedProgramTable[] = {
	{ static_SahderTextureColor_Vert,static_ShaderTextureColor_Frag },//0
	{ static_ShaderParallelLightNormal_Vert,static_ShaderParallelLightNormal_Frag },//1
	{static_ShaderShadowMap_Vert,static_ShaderShadowMap_Frag},//2
	{ static_ShaderShadowMap_Vert,static_ShaderShadowMap_Frag },//3
	{ static_ShaderDebugTexture_Vert ,static_ShaderDebugTexture_Frag},//4
	{ static_ShaderDebugDepthTexture_Vert ,static_ShaderDebugDepthTexture_Frag },//5
	{ static_ShaderFuzzyBoxTexture_Vert ,static_ShaderFuzzyBoxTexture_Frag },//6
	{ static_ShaderFuzzyBoxTexture_Vert, static_ShaderFuzzyBoxTextureVSM_Frag },//7
};
GLCacheManager::GLCacheManager()
{
	_bufferIdentity = 0;
}
GLCacheManager::~GLCacheManager()
{
//ɾ���������
	std::map<GLProgramType, GLProgram *>::iterator   _it = _glProgramCache.begin();
	while (_it != _glProgramCache.end())
	{
		_it->second->release(); 
		++_it;
	}
//ɾ���������
	std::map<std::string, GLTexture	*>::iterator  _texture_it = _glTextureCache.begin();
	while (_texture_it != _glTextureCache.end())
	{
		_texture_it->second->release();
		_texture_it++;
	}
	_glProgramCache.clear();
	_glTextureCache.clear();
	if (_bufferIdentity)
		glDeleteBuffers(1, &_bufferIdentity);
}
GLCacheManager    *GLCacheManager::getInstance()
{
	return &_glCacheManager;
}

//������ֵ���ҳ������
GLProgram	*GLCacheManager::findGLProgram(GLProgramType type)
{
	assert(type >= GLProgramType::GLProgramType_TextureColor && type <GLProgramType::GLProgramType_Number);
//���Ȳ����Ƿ���ڸó������
	std::map<GLProgramType, GLProgram *>::iterator   _it = _glProgramCache.find(type);
	if (_it != _glProgramCache.end())
		    return  _it->second;
//���û���ҵ�,�鿴�Ƿ���Ƕ�ĳ������
	GLProgram   *glProgram = GLProgram::createWithString(static_EmbedProgramTable[type].vert_src, static_EmbedProgramTable[type].frag_src);
	_glProgramCache[type] = glProgram;//��������ü�����1����
	//_glProgram->retain();
	return   glProgram;
}
//�򻺴��м���������
void           GLCacheManager::inserGLProgram(GLProgramType type, GLProgram *glProgram)
{
	std::map<GLProgramType, GLProgram *>::iterator    it = _glProgramCache.find(type);
	assert(it == _glProgramCache.end());
	_glProgramCache[type] = glProgram;
}
//��������ͬ,�������û����Ƕ��
GLTexture       *GLCacheManager::findGLTexture(std::string &key)
{
	std::map<std::string, GLTexture	*>::iterator it = _glTextureCache.find(key);
	if (it != _glTextureCache.end())
		return  it->second;
	return nullptr;
}
//��������
void        GLCacheManager::insertGLTexture(std::string &key,GLTexture *glTexture)
{
	std::map<std::string, GLTexture *>::iterator  it = _glTextureCache.find(key);
	assert(it == _glTextureCache.end());
	_glTextureCache[key] = glTexture;
}

unsigned      GLCacheManager::loadBufferIdentity()
{
	if (!_bufferIdentity)
	{
		int         _default_bufferId;
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &_default_bufferId);
		float     _VertexData[20] = {
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 
		};
		glGenBuffers(1, &_bufferIdentity);
		glBindBuffer(GL_ARRAY_BUFFER, _bufferIdentity);
		glBufferData(GL_ARRAY_BUFFER, sizeof(_VertexData), _VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, _default_bufferId);
	}
	return _bufferIdentity;
}
__NS_GLK_END