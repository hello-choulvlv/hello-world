/*
  *实时阴影实现
  *在实现的过程中,我们不再使用Cocos2d-x官方自带的空间几何变换函数,
  *因为考虑到矩阵变换/矩阵-向量乘法在实时阴影的计算过程中,使用的非常频繁,但是
  *官方自带的库最大问题就是调用链太长,冗余的计算太多,
  *因此有必要使用高效率的设计方案.
  *在使用OpenGL实现的过程中,我们将会引入一部分3.0的函数实现,当然我们也会做相关的判断.
  *LiSM相对而言对硬件的要求比较低,VSM可以用小尺寸的纹理就能实现高质量的阴影,
  *但是对硬件的要求比较高,目前测试的结果,也只有win32平台能完全的支持
  *@version:1.0
  *@date:2018年8月23日
  *@author:xiaoxiong
 */
#include "platform/CCGL.h"
#include "base/CCConfiguration.h"
#include "base/CCDirector.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventListenerCustom.h"
#include "base/ccMacros.h"
#include "base/CCEventType.h"

#include "3d/CCMesh.h"
#include "3d/CCMeshVertexIndexData.h"
#include "renderer/CCShadows.h"
#include "renderer/ccShaders.h"
#include "renderer/ccGLStateCache.h"
#include "renderer/CCGLProgram.h"
#include "renderer/CCMeshCommand.h"

#define min_f(x,y)  x<y?x:y;
#define max_f(x,y) x>y?x:y;
NS_CC_BEGIN
/*******************几何变换函数****************************/
/*NDC坐标系,注意他是一个左手坐标系*/
static  const  float static_ndcCoords[4*8] = {
	-1,1,-1,1,//0
	-1,-1,-1,1,//1
	1,-1,-1,1,//2
	1,1,-1,1,//3

	-1,1,1,1,//4
	-1,-1,1,1,//5
	1,-1,1,1,//6
	1,1,1,1,//7
};
//求逆矩阵
//向量与矩阵之间的乘法
static void   shadow_vec3_multiply_mat4(Vec3  &src,const Mat4 &mat,Vec3 &dst)
{
	float  x = src.x * mat.m[0] + src.y * mat.m[4] + src.z * mat.m[8] + mat.m[12];
	float  y = src.x * mat.m[1] + src.y * mat.m[5] + src.z * mat.m[9] + mat.m[13];
	float  z = src.x * mat.m[2] + src.y * mat.m[6] + src.z * mat.m[10] + mat.m[14];

	dst.x = x;
	dst.y = y;
	dst.z = z;
}

static void shadow_vec4_multiply_mat4(const Vec4 &src,const Mat4 &mat,Vec4 &dst)
{
	float  x = src.x * mat.m[0] + src.y * mat.m[4] + src.z * mat.m[8] + src.w * mat.m[12];
	float  y = src.x * mat.m[1] + src.y * mat.m[5] + src.z * mat.m[9] + src.w * mat.m[13];
	float  z = src.x * mat.m[2] + src.y * mat.m[6] + src.z * mat.m[10] + src.w * mat.m[14];
	float  w = src.x * mat.m[3] + src.y * mat.m[7] + src.z * mat.m[11] + src.w * mat.m[15];

	dst.x = x;
	dst.y = y;
	dst.z = z;
	dst.w = w;
}
//矩阵乘法
static void shadow_mat4_multiply_mat4(const Mat4  &src1,const Mat4 &src2,Mat4 &dst)
{
	float  m[16];
	const float  *m_1 = src1.m;
	const float  *m_2 = src2.m;
	float  *m_3 = m;
	if(dst.m != m_1 && dst.m != m_2)
		m_3 = dst.m;

	m_3[0] = m_1[0] * m_2[0] + m_1[1] * m_2[4] + m_1[2] * m_2[8] + m_1[3]*m_2[12];
	m_3[1] = m_1[0] * m_2[1] + m_1[1] * m_2[5] + m_1[2] * m_2[9] + m_1[3] * m_2[13];
	m_3[2] = m_1[0] * m_2[2] + m_1[1] * m_2[6] + m_1[2] * m_2[10] + m_1[3] * m_2[14];
	m_3[3] = m_1[0] * m_2[3] + m_1[1] * m_2[7] + m_1[2] * m_2[11] + m_1[3] * m_2[15];

	m_3[4] = m_1[4] * m_2[0] + m_1[5] * m_2[4] + m_1[6] * m_2[8] + m_1[7] * m_2[12];
	m_3[5] = m_1[4] * m_2[1] + m_1[5] * m_2[5] + m_1[6] * m_2[9] + m_1[7] * m_2[13];
	m_3[6] = m_1[4] * m_2[2] + m_1[5] * m_2[6] + m_1[6] * m_2[10] + m_1[7] * m_2[14];
	m_3[7] = m_1[4] * m_2[3] + m_1[5] * m_2[7] + m_1[6] * m_2[11] + m_1[7] * m_2[15];

	m_3[8] = m_1[8] * m_2[0] + m_1[9] * m_2[4] + m_1[10] * m_2[8] + m_1[11] * m_2[12];
	m_3[9] = m_1[8] * m_2[1] + m_1[9] * m_2[5] + m_1[10] * m_2[9] + m_1[11] * m_2[13];
	m_3[10] = m_1[8] * m_2[2] + m_1[9] * m_2[6] + m_1[10] * m_2[10] + m_1[11] * m_2[14];
	m_3[11] = m_1[8] * m_2[3] + m_1[9] * m_2[7] + m_1[10] * m_2[11] + m_1[11] * m_2[15];

	m_3[12] = m_1[12] * m_2[0] + m_1[13] * m_2[4] + m_1[14] * m_2[8] + m_1[15] * m_2[12];
	m_3[13] = m_1[12] * m_2[1] + m_1[13] * m_2[5] + m_1[14] * m_2[9] + m_1[15] * m_2[13];
	m_3[14] = m_1[12] * m_2[2] + m_1[13] * m_2[6] + m_1[14] * m_2[10] + m_1[15] * m_2[14];
	m_3[15] = m_1[12] * m_2[3] + m_1[13] * m_2[7] + m_1[14] * m_2[11] + m_1[15] * m_2[15];

	if (m_3 != dst.m)
		memcpy(dst.m,m_3,sizeof(float)*16);
}
/************************************************************/
ShadowMap::ShadowMap(const Size &framebufferSize, ShadowType type):
	_framebufferId(0),
	_depthbufferId(0),
	_framebufferSize(framebufferSize),
	_shadowType(type)
{
}

ShadowMap::~ShadowMap()
{
	if (_framebufferId)
		glDeleteFramebuffers(1,&_framebufferId);
	if (_depthbufferId)
		glDeleteFramebuffers(1,&_depthbufferId);
	_framebufferId = 0;
	_depthbufferId = 0;
}

bool ShadowMap::initFramebuffer()
{
	/*创建帧缓冲去对象*/
	int default_framebuffer,default_texture;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING,&default_framebuffer);
	glGetIntegerv(GL_TEXTURE_BINDING_2D,&default_texture);

	glGenFramebuffers(1,&_framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER,_framebufferId);
	glGenTextures(1, &_depthbufferId);
	glBindTexture(GL_TEXTURE_2D,_depthbufferId);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//注意以下的设置与常规的纹理之间的区别
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE,GL_COMPARE_R_TO_TEXTURE);
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_FUNC,GL_LEQUAL);
	//如果平台受支持尽可能地使用不可变更纹理
#if defined(GL_ARB_texture_storage) || defined(GL_EXT_texture_storage)
	const Configuration *config = Configuration::getInstance();
	if (config->supportImmutableStorage())
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, _framebufferSize.width, _framebufferSize.height);
	else
		glTexImage2D(GL_TEXTURE_2D,0, GL_DEPTH_COMPONENT24,_framebufferSize.width,_framebufferSize.height,0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,nullptr);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _framebufferSize.width, _framebufferSize.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
#endif
	//绑定帧缓冲去对象
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,_depthbufferId,0);
	//glDrawBuffer(GL_NONE);
	int result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(result == GL_FRAMEBUFFER_COMPLETE);
	//
	glBindTexture(GL_TEXTURE_2D,default_texture);
	glBindFramebuffer(GL_FRAMEBUFFER,default_framebuffer);

	return result;
}

void  ShadowMap::recreate()
{
	initFramebuffer();
}

unsigned ShadowMap::getFramebufferId()const
{
	return _framebufferId;
}

unsigned ShadowMap::getDepthbufferId()const
{
	return _depthbufferId;
}

ShadowType ShadowMap::getShadowType()const
{
	return _shadowType;
}

///////////////ShadowMapLiSM/////////////////////
ShadowMapLiSM::ShadowMapLiSM(const Size &framebufferSize) :ShadowMap(framebufferSize, ShadowType::ShadowType_LiSM)
{
}

bool ShadowMapLiSM::initFramebuffer()
{
	if (!ShadowMap::initFramebuffer())
		return false;
	initShadowProgram();
	return true;
}

void ShadowMapLiSM::initShadowProgram()
{
}
/////////////ShadowMapVSM//////////////////
ShadowMapVSM::ShadowMapVSM(const Size &framebufferSize) :ShadowMap(framebufferSize, ShadowType::ShadowType_VSM)
,_identityVertexBuffer(0)
,_fuzzyProgram(nullptr)
,_fuzzyAttribPositionLoc(-1)
,_fuzzyAttribFragCoordLoc(-1)
,_fuzzyBaseMapLoc(-1)
,_fuzzyPixelStepLoc(-1)
,_fuzzyFuzzyCountLoc(-1)
,_fuzzyCount(5)
{
	_framebufferIds[0] = _framebufferIds[1] = _framebufferIds[2] = 0;
	_textureIds[0] = _textureIds[1] = 0;
}

ShadowMapVSM::~ShadowMapVSM()
{
	if(_fuzzyProgram)
		_fuzzyProgram->release();
	if (_depthbufferId)
		glDeleteRenderbuffers(1,&_depthbufferId);
	glDeleteFramebuffers(3,_framebufferIds);
	glDeleteTextures(2, _textureIds);
	_framebufferIds[0] = _framebufferIds[1] = _framebufferIds[2] = 0;
	_textureIds[0] = _textureIds[1] = 0;
	_framebufferId = 0;
	//
	_identityVertexBuffer = 0;
	_fuzzyProgram = nullptr;
	_depthbufferId = 0;
}

bool ShadowMapVSM::initFramebuffer()
{
	CHECK_GL_ERROR_DEBUG();
	int  default_framebuffer, default_texture,default_renderbuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING,&default_framebuffer);
	glGetIntegerv(GL_TEXTURE_BINDING_2D,&default_texture);
	glGetIntegerv(GL_RENDERBUFFER_BINDING,&default_renderbuffer);

	const Configuration *config = Configuration::getInstance();
	//平台的差异性证实了,MAC-iOS平台只能使用半浮点数,这样会带来一些问题
	//由于浮点精度不足,带来的深度冲突干扰.
	glGenTextures(2,_textureIds);
	CHECK_GL_ERROR_DEBUG();
	for (int k = 0; k < 2; ++k)
	{
		glBindTexture(GL_TEXTURE_2D,_textureIds[k]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		//如果有可能,开启各向异性
		if (config->supportAnisortropic())
		{
			float anisotropy = config->getMaxAnisotropicValue();
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);// >= 2 ? anisotropy * 0.5f : 1.0f);
		}
		CHECK_GL_ERROR_DEBUG();
#if defined(GL_ARB_texture_storage) || defined(GL_EXT_texture_storage)
		if (config->supportImmutableStorage())
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, _framebufferSize.width, _framebufferSize.height);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _framebufferSize.width, _framebufferSize.height, 0, 0x8227, 0x140B, nullptr);
#else
		glTexImage2D(GL_TEXTURE_2D,0, 0x822F,_framebufferSize.width,_framebufferSize.height,0, 0x8227,0x140B,nullptr);
#endif
	}
	CHECK_GL_ERROR_DEBUG();
	//创建深度缓冲区对象
	glGenRenderbuffers(1, &_depthbufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, _depthbufferId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _framebufferSize.width, _framebufferSize.height);
	CHECK_GL_ERROR_DEBUG();

	glGenFramebuffers(3, _framebufferIds);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferIds[0]);
	
	//将两个纹理对象绑定到三个帧缓冲去对象上
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,_textureIds[0],0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,_depthbufferId);
	int    result = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	assert(result);
	//
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferIds[1]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureIds[1], 0);
	result &= glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	assert(result);

	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferIds[2]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureIds[0], 0);
	result &= glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	assert(result);

	glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
	glBindRenderbuffer(GL_RENDERBUFFER,default_renderbuffer);
	glBindTexture(GL_TEXTURE_2D,default_texture);

	CHECK_GL_ERROR_DEBUG();
	initShadowProgram();
	return result;
}

void  ShadowMapVSM::recreate()
{
	initFramebuffer();
}

void ShadowMapVSM::initShadowProgram()
{
	//作为后置处理的箱式模糊处理Shader
	if (!_fuzzyProgram)
	{
		_fuzzyProgram = GLProgram::createWithByteArrays(cc3d_BoxFuzzy_Vert, cc3d_BoxFuzzy_Frag);
		_fuzzyProgram->retain();
	}
	else
	{
		_fuzzyProgram->reset();
		_fuzzyProgram->initWithByteArrays(cc3d_BoxFuzzy_Vert, cc3d_BoxFuzzy_Frag);
	}

	_fuzzyAttribPositionLoc = glGetAttribLocation(_fuzzyProgram->getProgram(),"a_position");
	_fuzzyAttribFragCoordLoc = glGetAttribLocation(_fuzzyProgram->getProgram(),"a_fragCoord");
	_fuzzyBaseMapLoc = glGetUniformLocation(_fuzzyProgram->getProgram(),"u_BaseMap");
	_fuzzyPixelStepLoc = glGetUniformLocation(_fuzzyProgram->getProgram(),"u_PixelStep");
	_fuzzyFuzzyCountLoc = glGetUniformLocation(_fuzzyProgram->getProgram(),"u_FuzzyCount");

	CHECK_GL_ERROR_DEBUG();
}

void  ShadowMapVSM::setVertexBufferIdentity(unsigned vertex_buffer)
{
	_identityVertexBuffer = vertex_buffer;
}

void ShadowMapVSM::setSampleCount(int fuzzyCount)
{
	_fuzzyCount = fuzzyCount;
}

int ShadowMapVSM::getSampleCount()const
{
	return _fuzzyCount;
}

unsigned ShadowMapVSM::getDepthbufferId()const
{
	return _textureIds[0];
}

unsigned ShadowMapVSM::getFramebufferId()const
{
	return _framebufferIds[0];
}

void ShadowMapVSM::process()
{
	//切换帧缓冲区绑定
	glBindFramebuffer(GL_FRAMEBUFFER,_framebufferIds[1]);
	//关闭深度测试
	GL::setDepthTest(false);
	_fuzzyProgram->use();
	glUniform2f(_fuzzyPixelStepLoc,1.0f/_framebufferSize.width,0.0f);
	glUniform1i(_fuzzyFuzzyCountLoc,_fuzzyCount);

	GL::bindTexture2DN(0,_textureIds[0]);
	glUniform1i(_fuzzyBaseMapLoc,0);

	glBindBuffer(GL_ARRAY_BUFFER, _identityVertexBuffer);

	glEnableVertexAttribArray(_fuzzyAttribPositionLoc);
	glVertexAttribPointer(_fuzzyAttribPositionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);

	glEnableVertexAttribArray(_fuzzyAttribFragCoordLoc);
	glVertexAttribPointer(_fuzzyAttribFragCoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	/*纵向的模糊*/
	glBindFramebuffer(GL_FRAMEBUFFER,_framebufferIds[2]);
	GL::bindTexture2DN(0, _textureIds[1]);
	glUniform2f(_fuzzyPixelStepLoc,0.0f,1.0f/_framebufferSize.height);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}
/////////////////////////////////Shadows//////////////
Shadows::Shadows():
	_shadowMap(nullptr)
	,_sceneCamera(nullptr)
	,_lightColor(1.0f,1.0f,1.0f)
	,_lightAmbientColor(0.3f,0.3f,0.3f)
	,_lightDirection(0.57725f, 0.57725f, 0.57725f)
	,_lightPosition(_lightDirection * 400)
	,_analysisScene(false)
	, _useLightMode(0)
	,_visibilityTest(false)
	, _lightBleedingVec3(0.12f,0.7f,0.0f)
	, _shadowSize(512)
	, _lastFramebufferId(0)
	,_shadowMapName("CSVK_ShadowMap")
	,_lightViewMatrixName("u_LightViewMatrix")
	,_lightProjMatrixName("u_LightProjMatrix")
	,_lightDirectionName("u_LightDirection")
	,_lightBleedingName("u_LightBleeding")
	,_lightColorName("u_LightColor")
	,_lightAmbientColorName("u_AmbientLightSourceColor")
	,_useLightModeName("u_UseLightMode")
	, _lightViewProjMatrixName("u_LightViewProjMatrix")
	,_lightPixelStepName("u_PixelStep")
	,_clearColor(1000,10000,10000,10000)
	, _backgroundListener(nullptr)
{
	_polygonOffset[0] = 2.0f;
	_polygonOffset[1] = 2.0f;
}

Shadows::~Shadows()
{
	if (_skinProgram)
		_skinProgram->release();
	if (_lismProgram)
		_lismProgram->release();
	if (_clearProgram)
		_clearProgram->release();
	if (_debugProgram)
		_debugProgram->release();

	delete _shadowMap;
	glDeleteBuffers(1,&_identityArrayBuffer);

	if (_backgroundListener)
		Director::getInstance()->getEventDispatcher()->removeEventListener(_backgroundListener);
	_backgroundListener = nullptr;
}

bool   Shadows::initWithShadowParam(const ShadowParam &param)
{
	float sacleFactor = 1;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	GLView *glView = Director::getInstance()->getOpenGLView();
	sacleFactor *= glView->getFrameZoomFactor() * glView->getRetinaFactor();
#endif
	_sceneCamera = param.camera;
	_shadowSize = param.shadowSize * sacleFactor;
	//初始化摄像机
	_lightDirection = param.lightDirection;
	_lightPosition = param.lightPosition;

	_lightPixelStepVec3.x = _lightPixelStepVec3.y =  1.0f / _shadowSize;

	Mat4::createLookAt(param.lightPosition,param.lightPosition - param.lightDirection * 2,Vec3(0,1,0),&_lightViewMatrix);

	_skinProgram = GLProgram::createWithByteArrays(cc3d_SkinShadowMap_VSM_Vert,cc3d_SkinShadowMap_VSM_Frag);
	_skinProgram->retain();

	_lismProgram = GLProgram::createWithByteArrays(cc3d_SkinShadowMap_LiSM_Vert, cc3d_SkinShadowMap_LiSM_Frag);
	_lismProgram->retain();

	/*整理数据结构*/
	_lightAttribUniformParam.lightViewMatrix = &_lightViewMatrix;
	_lightAttribUniformParam.lightProjMatrix = &_lightProjMatrix;
	_lightAttribUniformParam.lightViewProjMatrix = &_lightViewProjMatrix;

	GL::bindVAO(0);
	float vertex_data[] = {
		-1.0f,1.0f,0.0f,0.0f,1.0f,//0
		-1.0f,-1.0f,0.0f,0.0f,0.0f,//1
		1.0f,1.0f,0.0f,1.0f,1.0f,//2
		1.0f,-1.0f,0.0f,1.0f,0.0f,
	};
	glGenBuffers(1,&_identityArrayBuffer);
	glBindBuffer(GL_ARRAY_BUFFER,_identityArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER,sizeof(vertex_data),vertex_data,GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);

	_debugProgram = GLProgram::createWithByteArrays(cc3d_DebugTexture_Vert, cc3d_DebugTexture_Frag);
	_debugProgram->retain();

	_clearProgram = GLProgram::createWithByteArrays(cc3d_FramebufferDepthClear_Vert, cc3d_FramebufferDepthClear_Frag);
	_clearProgram->retain();

	_clearPositionLoc = _clearProgram->getAttribLocation("a_position");
	_clearColorLoc = _clearProgram->getUniformLocation("u_color");
	
	glPolygonOffset(_polygonOffset[0], _polygonOffset[1]);
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	_backgroundListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED, CC_CALLBACK_1(Shadows::recreate, this));
	Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_backgroundListener,-1);
#endif
	return setShadowType(param.shadowType);
}

Shadows *Shadows::create(const ShadowParam &param)
{
	Shadows *realTimeShadow = new Shadows();
	if (realTimeShadow->initWithShadowParam(param))
	{
		realTimeShadow->autorelease();
		return realTimeShadow;
	}
	realTimeShadow->release();
	return nullptr;
}

bool Shadows::checkMachineSupport(ShadowType shadowType)
{
	bool support = false;
	Configuration   *config = Configuration::getInstance();
	//是否支持浮点纹理
	if (shadowType == ShadowType::ShadowType_VSM)
	{
		support = config->supportGLES3() || 
			((config->supportHalfColorBuffer() || (config->supportHalfFloatTexture() && config->supportHalfFloatTextureLinearSample()))
			&& config->supportRGTexture() && config->supportsOESDepth24());
	}
	else if (shadowType == ShadowType::ShadowType_LiSM)
	{
		//需要支持深度纹理
		support = config->supportGLES3() || config->supportsOESDepth24();
	}
	return support;
}

int  Shadows::getVertion()
{
	return 2;
}

void  Shadows::recreate(EventCustom *recreateEvent)
{
	_skinProgram->reset();
	_lismProgram->reset();
	_skinProgram->initWithByteArrays(cc3d_SkinShadowMap_VSM_Vert, cc3d_SkinShadowMap_VSM_Frag);
	_lismProgram->initWithByteArrays(cc3d_SkinShadowMap_LiSM_Vert, cc3d_SkinShadowMap_LiSM_Frag);

	GL::bindVAO(0);
	float vertex_data[] = {
		-1.0f,1.0f,0.0f,0.0f,1.0f,//0
		-1.0f,-1.0f,0.0f,0.0f,0.0f,//1
		1.0f,1.0f,0.0f,1.0f,1.0f,//2
		1.0f,-1.0f,0.0f,1.0f,0.0f,
	};
	glGenBuffers(1, &_identityArrayBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _identityArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	_debugProgram->reset();
	_clearProgram->reset();

	_debugProgram->initWithByteArrays(cc3d_DebugTexture_Vert, cc3d_DebugTexture_Frag);
	_clearProgram->initWithByteArrays(cc3d_FramebufferDepthClear_Vert, cc3d_FramebufferDepthClear_Frag);

	_clearPositionLoc = _clearProgram->getAttribLocation("a_position");
	_clearColorLoc = _clearProgram->getUniformLocation("u_color");

	_shadowMap->recreate();
	if (_shadowMap->getShadowType() == ShadowType_VSM)
	{
		ShadowMapVSM *vsm = (ShadowMapVSM*)_shadowMap;
		vsm->setVertexBufferIdentity(_identityArrayBuffer);
	}
}

void  Shadows::setLightBleeding(float light_bleeding)
{
	_lightBleedingVec3.x = light_bleeding;
}

float  Shadows::getLightBleeding()const
{
	return _lightBleedingVec3.x;
}

void  Shadows::setAmbientColor(const Vec3 &ambientColor)
{
	_lightAmbientColor = ambientColor;
}

const Vec3&  Shadows::getAmbientColor()const
{
	return _lightAmbientColor;
}

void  Shadows::setLightColor(const Vec3 &lightColor)
{
	_lightColor = lightColor;
}

const Vec3& Shadows::getLightColor()const
{
	return _lightColor;
}

void  Shadows::setShadowWeight(float f)
{
	_lightBleedingVec3.y = f;
}

float  Shadows::getShadowWeight()const
{
	return _lightBleedingVec3.y;
}

void  Shadows::setVSMSampleCount(int sample_count)
{
	CCASSERT(sample_count > 1, "VSM Sample count should > 1.");
	if (_shadowMap->getShadowType() == ShadowType_VSM)
	{
		ShadowMapVSM *vsm = (ShadowMapVSM*)_shadowMap;
		vsm->setSampleCount(sample_count);
	}
}

int  Shadows::getVSMSampleCount()const
{
	if (_shadowMap->getShadowType() == ShadowType_VSM)
		return ((ShadowMapVSM*)_shadowMap)->getSampleCount();
	return 0;
}

void  Shadows::setLightMode(bool lightMode)
{
	_useLightMode = lightMode;
}

bool  Shadows::isLightMode()const
{
	return _useLightMode;
}

void  Shadows::setAnalysisScene(bool analysisScene)
{
	_analysisScene = analysisScene;
}

bool Shadows::isAnalysisScene()const
{
	return _analysisScene;
}

void Shadows::setCasterVisibilityTest(bool visibilityTest)
{
	_visibilityTest = visibilityTest;
}

bool Shadows::isCasterVisibilityTest()const
{
	return _visibilityTest;
}

void Shadows::setShadowMapName(const std::string &name)
{
	_shadowMapName = name;
}

const std::string &Shadows::getShadowMapName()const
{
	return _shadowMapName;
}

void Shadows::setPolygonOffset(float polygonOffset[2])
{
	_polygonOffset[0] = polygonOffset[0];
	_polygonOffset[1] = polygonOffset[1];
	glPolygonOffset(polygonOffset[0], polygonOffset[1]);
}

void  Shadows::setPolygonEnabled(bool b)
{
	if (b)
		glEnable(GL_POLYGON_OFFSET_FILL);
	else
		glDisable(GL_POLYGON_OFFSET_FILL);
}

bool Shadows::setShadowType(ShadowType  shadowType)
{
	if (!_shadowMap || _shadowMap->getShadowType() != shadowType)
	{
		delete _shadowMap;
		_shadowMap = nullptr;
		if (shadowType == ShadowType_LiSM)
			_shadowMap = new ShadowMapLiSM(Size(_shadowSize, _shadowSize));
		else if (shadowType == ShadowType_VSM)
		{
			ShadowMapVSM *vsm = new ShadowMapVSM(Size(_shadowSize, _shadowSize));
			vsm->setVertexBufferIdentity(_identityArrayBuffer);
			_shadowMap = vsm;
		}
		CCASSERT(_shadowMap!=nullptr,"error,invalid shadow type.");
		if (!_shadowMap->initFramebuffer())
			return false;

		GLProgram  *object_array[4] = { nullptr, _lismProgram,_skinProgram,nullptr };
		GLProgram  *target_object = object_array[shadowType];

		_lightAttribUniformParam.positionLoc = target_object->getAttribLocation("a_position");
		_lightAttribUniformParam.blendWeightLoc = target_object->getAttribLocation("a_blendWeight");
		_lightAttribUniformParam.blendIndexLoc = target_object->getAttribLocation("a_blendIndex");
		_lightAttribUniformParam.modelViewLoc = target_object->getUniformLocation("CC_MVMatrix");
		_lightAttribUniformParam.projLoc = target_object->getUniformLocation("CC_PMatrix");
		_lightAttribUniformParam.matrixPaletteLoc = target_object->getUniformLocation("u_matrixPalette");
		return true;
	}
	return false;
}

ShadowType Shadows::getShadowType()const
{
	return _shadowMap!=nullptr?_shadowMap->getShadowType(): ShadowType_None;
}

//重新绑定摄像机,但是此摄像机也有可能位空
void Shadows::setCamera(Camera *camera)
{
	_sceneCamera = camera;
}

Camera  *Shadows::getCamera()const
{
	return _sceneCamera;
}

void  Shadows::setLightViewMatrix(const Vec3 &lightDirection, const Vec3 &lightPosition)
{
	//初始化摄像机
	_lightDirection = lightDirection;
	_lightPosition = lightPosition;

	Mat4::createLookAt(lightPosition, lightPosition - lightDirection * 2, Vec3(0, 1, 0), &_lightViewMatrix);
}

void Shadows::beforeRender()
{
	/**计算视锥体*/
	float  mat[16];
	Mat4  &reverse_matrix = *reinterpret_cast<Mat4*>(mat);
	shadow_mat4_multiply_mat4(_sceneCamera->getViewProjectionMatrix().getInversed(), _lightViewMatrix,reverse_matrix);
	//计算光空间下的原场景空间的八个顶点的坐标
	float   light_coordVec4[8*4];
	Vec4   *light_coords = reinterpret_cast<Vec4*>(light_coordVec4);
	const Vec4   *ndc_coord = reinterpret_cast<const Vec4*>(static_ndcCoords);
	Vec3 &box_min = _frustumAABB._min;
	Vec3 &box_max = _frustumAABB._max;
	box_min = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	box_max = Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (int k = 0; k < 8; ++k)
	{
		Vec4 &lightCoord = light_coords[k];
		shadow_vec4_multiply_mat4(ndc_coord[k], reverse_matrix, lightCoord);

		lightCoord.x /= lightCoord.w;
		lightCoord.y /= lightCoord.w;
		lightCoord.z /= lightCoord.w;

		box_min.x = min_f(box_min.x, lightCoord.x);
		box_min.y = min_f(box_min.y, lightCoord.y);
		box_min.z = min_f(box_min.z, lightCoord.z);

		box_max.x = max_f(box_max.x, lightCoord.x);
		box_max.y = max_f(box_max.y, lightCoord.y);
		box_max.z = max_f(box_max.z, lightCoord.z);
	}
	if (!_analysisScene)
		Mat4::createOrthographicOffCenter(box_min.x, box_max.x, box_min.y, box_max.y, 0.0f, -box_min.z, &_lightProjMatrix);
	_shadowCasters.reserve(16);
	_casterAABB._min = Vec3(FLT_MAX,FLT_MAX,FLT_MAX);
	_casterAABB._max = Vec3(-FLT_MAX,-FLT_MAX,-FLT_MAX);
}

void Shadows::pushShadowCaster(Mesh **meshes, int meshCount, const AABB &aabb,bool useShadow)
{
	//重新检测Mesh的可视化,在Mesh的实际使用中,发现Mesh的数量很少有大于3的,为了避免繁琐的小内存分配与释放
	//使用了堆栈上的数组
	Mesh  *check_meshes[64];
	Mesh  **check_p = check_meshes;
	if (meshCount > 64)
		check_p = new Mesh *[meshCount];
	int         mesh_count = 0;
	for (int k = 0; k < meshCount; ++k)
	{
		meshes[k]->setUseShadow(useShadow);
		if (meshes[k]->isVisible())
			check_p[mesh_count++] = meshes[k];
	}
	//如果没有产生任何有效的Mesh,直接返回.
	if (!mesh_count)
	{
		if (check_p != check_meshes)
			delete check_p;
		return;
	}
	/*
	  *如果开启了场景分析,步骤如下
	 */
	const  Vec3 &box_min = aabb._min;
	const Vec3  &box_max = aabb._max;
	float  min_x = FLT_MAX, min_y = FLT_MAX, min_z = FLT_MAX;
	float  max_x = -FLT_MAX, max_y = -FLT_MAX, max_z = -FLT_MAX;
	//只要有任何的一项开启,则必须进行光空间转换
	if (_analysisScene || _visibilityTest)
	{
		//将包围盒展开成8个顶点
		float   light_aabbVec3[8 * 3] = {
			box_min.x,box_min.y,box_min.z,//0
			box_max.x,box_min.y,box_min.z,//1
			box_max.x,box_max.y,box_min.z,//2
			box_min.x,box_max.y,box_min.z,//3

			box_min.x,box_min.y,box_max.z,//4
			box_max.x,box_min.y,box_max.z,//5
			box_max.x,box_max.y,box_max.z,//6
			box_min.x,box_max.y,box_max.z,//7
		};
		Vec3  *light_aabb = reinterpret_cast<Vec3 *>(light_aabbVec3);
		for (int k = 0; k < 8; ++k)
		{
			Vec3 &light_coord = light_aabb[k];
			shadow_vec3_multiply_mat4(light_coord, _lightViewMatrix, light_coord);

			min_x = min_f(min_x, light_coord.x);
			min_y = min_f(min_y, light_coord.y);
			min_z = min_f(min_z, light_coord.z);

			max_x = max_f(max_x, light_coord.x);
			max_y = max_f(max_y, light_coord.y);
			max_z = max_f(max_z, light_coord.z);
		}
	}
	if (_analysisScene)
	{
		//变换到光空间,并扩张几何体集合的包围盒
		Vec3  &geometry_min = _casterAABB._min;
		Vec3  &geometry_max = _casterAABB._max;

		geometry_max.x = max_f(max_x, geometry_max.x);
		geometry_max.y = max_f(max_y, geometry_max.y);
		geometry_max.z = max_f(max_z, geometry_max.z);

		geometry_min.x = min_f(min_x, geometry_min.x);
		geometry_min.y = min_f(min_y, geometry_min.y);
		geometry_min.z = min_f(min_z, geometry_min.z);
	}
	/*
	  *是否启用光空间裁剪
	 */
	bool  visibilityN = false;
	if (_visibilityTest)
	{
		const Vec3 &box_min = _frustumAABB._min;
		const Vec3 &box_max = _frustumAABB._max;
		//只要有任何一个条件满足,则就处于光空间之外,此时几何体一定不会产生阴影
		visibilityN = min_x > box_max.x || max_x <box_min.x || min_y > box_max.y || max_y <box_min.y || min_z >0 || max_z <box_min.z;
	}
	/*
	  *这里也同样附带了一个问题,假如在光空间中将几何体裁剪,那么会不会阴影几何体的最终的渲染,
	  *实际上不会的,因为光空间是视锥体空间的完全空间.
	 */
	if(!visibilityN)
		addMeshes(check_p, mesh_count);
	if (check_p != check_meshes)
		delete check_p;
}

void  Shadows::afterVisitGeometry()
{
	//与光空间本身形成的视锥体进行比较
	if (_analysisScene)
	{
		float  min_x = max_f(_frustumAABB._min.x, _casterAABB._min.x);
		float  min_y = max_f(_frustumAABB._min.y, _casterAABB._min.y);
		float  min_z = max_f(_frustumAABB._min.z, _casterAABB._min.z);

		float  max_x = min_f(_frustumAABB._max.x, _casterAABB._max.x);
		float  max_y = min_f(_frustumAABB._max.y, _casterAABB._max.y);
		float  max_z = min_f(_frustumAABB._max.z, _casterAABB._max.z);

		Mat4::createOrthographicOffCenter(min_x, max_x, min_y, max_y,0, -min_z, &_lightProjMatrix);
	}
	shadow_mat4_multiply_mat4(_lightViewMatrix,_lightProjMatrix,_lightViewProjMatrix);
	//_lightViewProjMatrix = _lightProjMatrix * _lightViewMatrix;
}

void Shadows::addMeshes(Mesh **meshes, int meshCount)
{
	if (_shadowCasters.size() + meshCount >= _shadowCasters.capacity())
		_shadowCasters.reserve(_shadowCasters.size()*2);
	for (int k = 0; k < meshCount; ++ k)
		_shadowCasters.push_back(meshes[k]);
}

void Shadows::addMesh(Mesh *mesh)
{
	addMeshes(&mesh, 1);
}

void  Shadows::debugTexture(int texture)
{
	_debugProgram->use();
	glBindBuffer(GL_ARRAY_BUFFER,_identityArrayBuffer);

	int position_loc = _debugProgram->getAttribLocation("a_position");
	int fragCoord_loc = _debugProgram->getAttribLocation("a_fragCoord");
	int  texture_loc = _debugProgram->getUniformLocation("CC_Texture0");

	glEnableVertexAttribArray(position_loc);
	glVertexAttribPointer(position_loc,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(fragCoord_loc);
	glVertexAttribPointer(fragCoord_loc,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)(sizeof(float)*3));

	GL::bindTexture2DN(0, texture);
	glUniform1i(texture_loc,0);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	glBindBuffer(GL_ARRAY_BUFFER,0);
}

void Shadows::renderShadowVSM()
{
	_skinProgram->use();
	glUniformMatrix4fv(_lightAttribUniformParam.projLoc,1,GL_FALSE,_lightProjMatrix.m);
	//输入的顶点属性需要三个,位置,blend-weight,blend-index
	for (auto it = _shadowCasters.begin(); it != _shadowCasters.end(); ++it)
	{
		Mesh  *mesh = *it;
		const MeshVertexData  *vertexData = mesh->getMeshIndexData()->getMeshVertexData();
		_lightAttribUniformParam.vertexStride = vertexData->getVertexBuffer()->getSizePerVertex();
		//必须拿到关于顶点数据的信息
		const MeshVertexAttrib *attrib[3] = {
			vertexData->getMeshVertexAttrib(GLProgram::VERTEX_ATTRIB_POSITION),//position
			vertexData->getMeshVertexAttrib(GLProgram::VERTEX_ATTRIB_BLEND_WEIGHT),//blend-weight
			vertexData->getMeshVertexAttrib(GLProgram::VERTEX_ATTRIB_BLEND_INDEX),//blend-index
		};
		//*将顶点数据中关于位置的信息写入到结构体中,并传递到相关的函数中
		mesh->getMeshCommand().drawShadow(_lightAttribUniformParam,mesh->getSkin(),attrib, LightParamMask_Position | LightParamMask_BlendWeight | LightParamMask_BlendIndex|LightParamMask_LightViewMatrix);
		//将阴影纹理的阴影设置到Mesh中
		if (mesh->isUseShadow())//检测是否Mesh使用Shadow
		{
			mesh->setShadowMap(_shadowMapName, _shadowMap->getDepthbufferId());
			mesh->setLightMatrix(_lightViewMatrixName, _lightViewMatrix);
			mesh->setLightMatrix(_lightProjMatrixName, _lightProjMatrix);
			mesh->setLightVec3(_lightAmbientColorName, _lightAmbientColor);
			mesh->setLightVec3(_lightColorName, _lightColor);
			mesh->setLightVec3(_lightBleedingName, _lightBleedingVec3);
			mesh->setLightVec3(_lightDirectionName, _lightDirection);
			mesh->setLight1i(_useLightModeName, _useLightMode);
		}
	}
	_shadowMap->process();
}

void  Shadows::renderShadowLiSM()
{
	_lismProgram->use();

	glUniformMatrix4fv(_lightAttribUniformParam.projLoc, 1, GL_FALSE, _lightViewProjMatrix.m);
	//输入的顶点属性需要三个,位置,blend-weight,blend-index
	for (auto it = _shadowCasters.begin(); it != _shadowCasters.end(); ++it)
	{
		Mesh  *mesh = *it;
		const MeshVertexData  *vertexData = mesh->getMeshIndexData()->getMeshVertexData();
		_lightAttribUniformParam.vertexStride = vertexData->getVertexBuffer()->getSizePerVertex();
		//必须拿到关于顶点数据的信息
		const MeshVertexAttrib *attrib[3] = {
			vertexData->getMeshVertexAttrib(GLProgram::VERTEX_ATTRIB_POSITION),//position
			vertexData->getMeshVertexAttrib(GLProgram::VERTEX_ATTRIB_BLEND_WEIGHT),//blend-weight
			vertexData->getMeshVertexAttrib(GLProgram::VERTEX_ATTRIB_BLEND_INDEX),//blend-index
		};
		//*将顶点数据中关于位置的信息写入到结构体中,并传递到相关的函数中
		mesh->getMeshCommand().drawShadow(_lightAttribUniformParam, mesh->getSkin(), attrib,LightParamMask_Position|LightParamMask_BlendWeight|LightParamMask_BlendIndex);
		//将阴影纹理的阴影设置到Mesh中
		if (mesh->isUseShadow())//检测是否Mesh使用Shadow
		{
			mesh->setShadowMap(_shadowMapName, _shadowMap->getDepthbufferId());
			mesh->setLightMatrix(_lightViewProjMatrixName, _lightViewProjMatrix);
			mesh->setLightVec3(_lightAmbientColorName, _lightAmbientColor);
			mesh->setLightVec3(_lightColorName, _lightColor);
			mesh->setLightVec3(_lightPixelStepName,_lightPixelStepVec3);
			mesh->setLightVec3(_lightBleedingName,_lightBleedingVec3);
			mesh->setLightVec3(_lightDirectionName, _lightDirection);
			mesh->setLight1i(_useLightModeName, _useLightMode);
		}
	}
}

void  Shadows::renderShadow()
{
	//如果没有任何的几何体,返回
	if (!_shadowCasters.size())
		return;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_lastFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _shadowMap->getFramebufferId());
	/*
	*获取原来的窗口维度
	*/
	int viewport[4];
	GL::queryViewport(viewport);
	GL::setViewPort(0, 0, _shadowSize, _shadowSize);
	bool depthTest = GL::isDepthTest();
	int depth_mask = GL::queryDepthMask();
	int  depth_func = GL::queryDepthFunc();
	bool  cull_face = GL::isCullFace();
	unsigned cullface_mode = GL::queryCullFaceMode();
	/*
	*清理帧缓冲区对象
	*/
	GL::bindVAO(0);
	_clearProgram->use();
	glBindBuffer(GL_ARRAY_BUFFER, _identityArrayBuffer);

	glEnableVertexAttribArray(_clearPositionLoc);
	glVertexAttribPointer(_clearPositionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);

	//OpenGL Context State
	GL::setDepthMask(true);
	GL::setDepthFunc(GL_ALWAYS);
	GL::setDepthTest(true);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	GL::setDepthFunc(GL_LEQUAL);
	GL::setCullFace(true);
	GL::setCullFaceMode(GL_BACK);

	typedef  void  (Shadows::*RenderShadowFunc)();
	RenderShadowFunc    func_sm[3] = {
		nullptr,
		&Shadows::renderShadowLiSM,
		&Shadows::renderShadowVSM,
	};
	(this->*func_sm[_shadowMap->getShadowType()])();

	//restore
	CHECK_GL_ERROR_DEBUG();
	GL::bindVAO(0);
	//恢复原来的帧缓冲去对象绑定
	glBindFramebuffer(GL_FRAMEBUFFER, _lastFramebufferId);
	GL::setDepthTest(depthTest);
	GL::setDepthFunc(depth_func);
	GL::setDepthMask(depth_mask);
	GL::setCullFace(cull_face);
	GL::setCullFaceMode(cullface_mode);
	GL::setViewPort(viewport[0], viewport[1], viewport[2], viewport[3]);
	//debugTexture(_shadowMap->_depthbufferId);
}

void Shadows::afterRender()
{
	_shadowCasters.clear();
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

NS_CC_END

#undef min_f
#undef max_f