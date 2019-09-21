/*
  *SSAO实现
  *2018年1月12日
  *@author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "SSAO.h"
#include "engine/GLContext.h"
#include "engine/event/EventManager.h"
#include "engine/GLCacheManager.h"
#include<random>
#define _KEY_MASK_W_  0x1
#define _KEY_MASK_S_    0x2
#define _KEY_MASK_A_   0x4
#define _KEY_MASK_D_   0x8
using namespace glk;
SSAO::SSAO():
	_skybox(nullptr)
	,_pillar(nullptr)
	,_lightProgram(nullptr)
	,_geometryProgram(nullptr)
	, _ssaoProgram(nullptr)
	,_fuzzyProgram(nullptr)
	,_fuzzyTexture0(nullptr)
	,_fuzzyTexture1(nullptr)
	,_lightPosition(0)
	,_lightColor(0.9f,0.9f,0.9f,1.0f)
	,_touchListener(nullptr)
	,_keyListener(nullptr)
	,_keyMask(0)
{

}

SSAO::~SSAO()
{
	_skybox->release();
	_pillar->release();
	_lightProgram->release();
	_geometryProgram->release();
	_ssaoProgram->release();
	_fuzzyProgram->release();
	_fuzzyTexture0->release();
	_fuzzyTexture1->release();
	_defferedShader->release();
	_camera->release();
	glDeleteTextures(1, &_tangentTextureId);

	EventManager::getInstance()->removeListener(_touchListener);
	_touchListener->release();

	EventManager::getInstance()->removeListener(_keyListener);
	_keyListener = nullptr;
}

SSAO *SSAO::create()
{
	SSAO *ssao = new SSAO();
	ssao->init();
	return ssao;
}

void SSAO::init()
{
	_skybox = Skybox::createWithScale(64);
	_pillar = Chest::createWithScale(4,64,8);
	//柱子的模型变换矩阵
	_pillarModelMatrix[0].translate(60,0,0);
	_pillarModelMatrix[1].translate(0, 0, -60);
	_pillarModelMatrix[2].translate(-60, 0, 0);
	_pillarModelMatrix[3].translate(0, 0, 60);
	//法线矩阵不用设置

	//Shader
	_lightProgram = GLProgram::createWithFile("shader/ssao/LightRender_VS.glsl", "shader/ssao/LightRender_FS.glsl");
	_geometryProgram = GLProgram::createWithFile("shader/ssao/Geometry_VS.glsl", "shader/ssao/Geometry_FS.glsl");
	_ssaoProgram = GLProgram::createWithFile("shader/ssao/SSAO_VS.glsl", "shader/ssao/SSAO_FS.glsl");
	_fuzzyProgram = GLProgram::createWithFile("shader/ssao/Fuzzy_VS.glsl", "shader/ssao/Fuzzy_FS.glsl");
	//摄像机
	auto &winSize = GLContext::getInstance()->getWinSize();
	_camera = Camera2::createWithPerspective(60.0f, winSize.width/winSize.height,0.5f,400.0f);
	_camera->lookAt(Vec3(), Vec3(0,0,-1));
	//event
	_touchListener = TouchEventListener::createTouchListener(this, glk_touch_selector(SSAO::onTouchBegan),
																														glk_move_selector(SSAO::onTouchMoved),
																														glk_release_selector(SSAO::onTouchReleased));
	EventManager::getInstance()->addTouchEventListener(_touchListener,0);

	_keyListener = KeyEventListener::createKeyEventListener(this, glk_key_press_selector(SSAO::onKeyPressed),glk_key_release_selector(SSAO::onKeyReleased));
	EventManager::getInstance()->addKeyEventListener(_keyListener, 0);
	//Deffered Shader,只有三个分量
	_defferedShader = DefferedShader::create(winSize,3);
	initKernelTangent();

	_ssaoTexture = RenderTexture::createRenderTexture(winSize, RenderTexture::RenderType::ColorBuffer,
																										RenderTexture::ColorFormatType::ColorFormatType_R_HALF,
																										RenderTexture::DepthFormatType::DepthFormatType_Normal);
	_fuzzyTexture0 = RenderTexture::createRenderTexture(winSize,RenderTexture::RenderType::ColorBuffer,
																										RenderTexture::ColorFormatType::ColorFormatType_R_HALF,
																										RenderTexture::DepthFormatType::DepthFormatType_Normal);
	_fuzzyTexture1 = RenderTexture::createRenderTexture(winSize, RenderTexture::RenderType::ColorBuffer,
																										RenderTexture::ColorFormatType::ColorFormatType_R_HALF,
																										RenderTexture::DepthFormatType::DepthFormatType_Normal);
}

void SSAO::initKernelTangent()
{
	//转动核心
	float start = 0.25;
	float end = 1.0f;
	float interpolation = end - start;
	std::uniform_real_distribution<float> random_distribute(0,1.0f);
	std::default_random_engine  rand_engine;
	for (int k = 0; k < 32; ++k)
	{
		Vec3  kernel(2.0f*random_distribute(rand_engine) -1.0f,
								2.0f*random_distribute(rand_engine)-1.0f, 
								random_distribute(rand_engine));
		float scale = k / 31.0f;
		scale =  start + interpolation * scale*scale;
		_kernelVec3[k] = kernel.normalize()*scale;// random_distribute(rand_engine);
	}
	//
	auto &winSize = GLContext::getInstance()->getWinSize();
	//生成切线向量
	int        totalSize = winSize.width*winSize.height*3;
	float   *tangentVec = new float[totalSize];
	for (int index = 0; index < totalSize; index +=3)
	{
		tangentVec[index] =  2.0f*random_distribute(rand_engine) - 1.0f;
		tangentVec[index + 1] =  2.0f*random_distribute(rand_engine) - 1.0f;
		tangentVec[index + 2] = 0.0f;
	}
	//切线向量
	glGenTextures(1, &_tangentTextureId);
	glBindTexture(GL_TEXTURE_2D,_tangentTextureId);
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, winSize.width, winSize.height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, winSize.width, winSize.height,GL_RGB,GL_FLOAT, tangentVec);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB32F,winSize.width,winSize.height,0,GL_RGB,GL_FLOAT,tangentVec);
	glBindTexture(GL_TEXTURE_2D,0);

	delete[] tangentVec;
	tangentVec = nullptr;
}

void SSAO::update(float dt)
{
	if (_keyMask)
	{
		Vec3 stepVec;
		float speed = 1.5f;
		if (_keyMask & _KEY_MASK_W_)
			stepVec+=_camera->getForwardVector()*speed;
		if (_keyMask & _KEY_MASK_S_)
			stepVec -= _camera->getForwardVector()*speed;
		if (_keyMask &_KEY_MASK_A_)
			stepVec -= _camera->getXVector()*speed;
		if (_keyMask & _KEY_MASK_D_)
			stepVec += _camera->getXVector()*speed;
		_camera->translate(stepVec);
	}
}

void SSAO::defferedRender()
{
	_defferedShader->active();
	//
	_geometryProgram->perform();
	int  modelViewMatrixLoc = _geometryProgram->getUniformLocation("g_ModelViewMatrix");
	int  projMatrixLoc = _geometryProgram->getUniformLocation("g_ProjMatrix");
	int  normalViewMatrixLoc = _geometryProgram->getUniformLocation("g_NormalViewMatrix");
	int  lightColorLoc = _geometryProgram->getUniformLocation("g_LightColor");
	//
	auto &viewMatrix = _camera->getViewMatrix();
	Mat3   viewTruckMatrix = viewMatrix;
	glUniform4fv(lightColorLoc,1,&_lightColor.x);
	glUniformMatrix4fv(projMatrixLoc,1,GL_FALSE,_camera->getProjMatrix().pointer());
	//房间
	_skybox->bindVertexObject(0);
	_skybox->bindNormalObject(1);
	glUniformMatrix4fv(modelViewMatrixLoc, 1, GL_FALSE, (_skyboxModelMatrix * viewMatrix).pointer() );
	glUniformMatrix3fv(normalViewMatrixLoc, 1, GL_FALSE, (_skyboxNormalMatrix * viewTruckMatrix).pointer());
	_skybox->drawShape();
	//柱子
	_pillar->bindVertexObject(0);
	_pillar->bindNormalObject(1);
	for (int k = 0; k < 4; ++k)
	{
		Vec4 lightColor(0.1+ 0.9*k/4.0f, 0.6, 0.5, 1.0f);
		glUniform4fv(lightColorLoc, 1, &lightColor.x);
		glUniformMatrix4fv(modelViewMatrixLoc, 1, GL_FALSE, (_pillarModelMatrix[k]* viewMatrix).pointer());
		glUniformMatrix3fv(normalViewMatrixLoc, 1, GL_FALSE, (_pillarNormalMatrix[k]* viewTruckMatrix).pointer());
		_pillar->drawShape();
	}
	_defferedShader->restore();
}
void SSAO::updateOcclusion()
{
	int  positionTextureLoc = _ssaoProgram->getUniformLocation("g_PositionTexture");
	int  normalTextureLoc = _ssaoProgram->getUniformLocation("g_NormalTexture");
	int  tangentTextureLoc = _ssaoProgram->getUniformLocation("g_TangentTexture");
	int  projMatrixLoc = _ssaoProgram->getUniformLocation("g_ProjMatrix");
	int  normalViewMatrixLoc = _ssaoProgram->getUniformLocation("g_NormalViewMatrix");
	int  kernelLoc = _ssaoProgram->getUniformLocation("g_Kernel");
	//
	_ssaoTexture->activeFramebuffer();
	_ssaoProgram->perform();
	//使用通过延迟着色获得的纹理数据计算遮蔽值
	int               textureCount = 3;
	unsigned   textureId[3];
	_defferedShader->getColorBuffer(textureId, &textureCount);
	//
	auto &viewMatrix = _camera->getViewMatrix();
	Mat3  viewNormalMatrix = viewMatrix;
	glUniformMatrix4fv(projMatrixLoc,1,GL_FALSE,_camera->getProjMatrix().pointer());
	glUniformMatrix3fv(normalViewMatrixLoc,1,GL_FALSE, viewNormalMatrix.pointer());
	//position
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,textureId[1]);
	glUniform1i(positionTextureLoc,0);
	//normal
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,textureId[2]);
	glUniform1i(normalTextureLoc,1);
	//tangent
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,_tangentTextureId);
	glUniform1i(tangentTextureLoc,2);
	//kernel
	glUniform3fv(kernelLoc,32,&_kernelVec3[0].x);
	//Vertex Buffer
	int identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER,identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)(sizeof(float)*3));

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	//
	_ssaoTexture->disableFramebuffer();
}
void SSAO::fuzzyOcclusion()
{
	_fuzzyTexture0->activeFramebuffer();
	_fuzzyProgram->perform();
	//
	int   baseTextureLoc = _fuzzyProgram->getUniformLocation("g_BaseMap");
	int   fuzzyStepVecLoc = _fuzzyProgram->getUniformLocation("g_FuzzyVec");
	//水平模糊
	Vec2 fuzzyStepVec(1.0f,0.0f);
	auto &winSize = GLContext::getInstance()->getWinSize();
	glUniform2f(fuzzyStepVecLoc,1.0f/ winSize.width,0.0f);
	//使用上一个函数计算出来的纹理
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _ssaoTexture->getColorBuffer());
	glUniform1i(baseTextureLoc,0);
	//使用单位顶点缓冲区对象
	int identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER,identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)(sizeof(float)*3));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	_fuzzyTexture0->disableFramebuffer();
	//垂直模糊
	glUniform2f(fuzzyStepVecLoc,0.0f,1.0f/ winSize.height);
	_fuzzyTexture1->activeFramebuffer();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_fuzzyTexture0->getColorBuffer());
	glUniform1i(baseTextureLoc,0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	_fuzzyTexture1->disableFramebuffer();
}
void SSAO::render()
{
	_lightProgram->perform();
	//uniforms
	int occlusionTextureLoc = _lightProgram->getUniformLocation("g_OcclusionTexture");
	int colorTextureLoc = _lightProgram->getUniformLocation("g_ColorTexture");
	int positionTextureLoc = _lightProgram->getUniformLocation("g_PositionTexture");
	int normalTextureLoc = _lightProgram->getUniformLocation("g_NormalTexture");
	int lightPositionLoc = _lightProgram->getUniformLocation("g_LightPosition");
	int lightColorLoc = _lightProgram->getUniformLocation("g_LightColor");
	int textureCount = 3;
	unsigned colorBuffer[4];
	_defferedShader->getColorBuffer(colorBuffer, &textureCount);
	//环境光遮蔽纹理
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_fuzzyTexture1->getColorBuffer());
	glUniform1i(occlusionTextureLoc,0);
	//颜色
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,colorBuffer[0]);
	glUniform1i(colorTextureLoc,1);
	//位置
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,colorBuffer[1]);
	glUniform1i(positionTextureLoc,2);
	//法线
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D,colorBuffer[2]);
	glUniform1i(normalTextureLoc,3);
	//光照的位置以及颜色
	glUniform3fv(lightPositionLoc,1,&_lightPosition.x);
	glUniform3fv(lightColorLoc,1,&_lightColor.x);
	//
	int identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER,identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)(sizeof(float)*3));

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, _fuzzyTexture1->getColorBuffer());
	//glUniform1i(occlusionTextureLoc,0);

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

bool SSAO::onTouchBegan(const Vec2 &touchPoint)
{
	_offsetVec2 = touchPoint;
	return true;
}

void SSAO::onTouchMoved(const Vec2 &touchPoint)
{
	Vec2 offsetVec = touchPoint - _offsetVec2;

	_camera->rotate(-offsetVec.x*0.5f, offsetVec.y*0.5f);
	_offsetVec2 = touchPoint;
}

void SSAO::onTouchReleased(const Vec2 &touchPoint)
{

}

bool SSAO::onKeyPressed(KeyCodeType keyCode)
{
	if (keyCode == KeyCodeType::KeyCode_W)
		_keyMask |= _KEY_MASK_W_;
	else if (keyCode == KeyCodeType::KeyCode_S)
		_keyMask |= _KEY_MASK_S_;
	else if (keyCode == KeyCodeType::KeyCode_A)
		_keyMask |= _KEY_MASK_A_;
	else if (keyCode == KeyCodeType::KeyCode_D)
		_keyMask |= _KEY_MASK_D_;
	return true;
}

void SSAO::onKeyReleased(KeyCodeType keyCode)
{
	if (keyCode == KeyCodeType::KeyCode_W)
		_keyMask &= ~_KEY_MASK_W_;
	else if (keyCode == KeyCodeType::KeyCode_S)
		_keyMask &= ~_KEY_MASK_S_;
	else if (keyCode == KeyCodeType::KeyCode_A)
		_keyMask &= ~_KEY_MASK_A_;
	else if (keyCode == KeyCodeType::KeyCode_D)
		_keyMask &= ~_KEY_MASK_D_;
}