/*
  *实时辉光
  *2017-7-18
  *@Author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "engine/GLContext.h"
#include "engine/GLCacheManager.h"
#include "engine/event/EventManager.h"
#include "SunHalo.h"
#include<assert.h>
#include<math.h>

SunHalo::SunHalo():
_drawSunTexture(nullptr)
,_drawThickTexture0(nullptr)
,_drawThickTexture1(nullptr)
,_drawPreciseTexture0(nullptr)
,_drawPreciseTexture1(nullptr)
,_drawMixTexture(nullptr)
,_dirtyTexture(nullptr)
,_blurShader(nullptr)
,_haloShader(nullptr)
,_renderShader(nullptr)
,_normalShader(nullptr)
, _touchEventListener(nullptr)
{

}

SunHalo::~SunHalo()
{
	_drawSunTexture->release();
	_drawThickTexture0->release();
	_drawThickTexture1->release();
	_drawPreciseTexture0->release();
	_drawPreciseTexture1->release();
	_drawMixTexture->release();
	//
	_dirtyTexture->release();
	_sphereMesh->release();
	//
	_blurShader->release();
	_haloShader->release();
	_renderShader->release();
	_normalShader->release();
	//
	glk::EventManager::getInstance()->removeListener(_touchEventListener);
	_touchEventListener->release();
}

SunHalo *SunHalo::create()
{
	SunHalo  *halo = new SunHalo();
	halo->init();
	return halo;
}

void SunHalo::init()
{
	const glk::Size &winSize = glk::GLContext::getInstance()->getWinSize();
	const glk::Size halfWinSize(winSize.width/2.0f,winSize.height/2.0f);
	_drawSunTexture = glk::RenderTexture::createRenderTexture(halfWinSize,glk::RenderTexture::RenderType::ColorBuffer|glk::RenderTexture::RenderType::DepthBuffer);

	_drawThickTexture0 = glk::RenderTexture::createRenderTexture(halfWinSize,glk::RenderTexture::RenderType::ColorBuffer | glk::RenderTexture::RenderType::DepthBuffer);
	_drawThickTexture1 = glk::RenderTexture::createRenderTexture(halfWinSize, glk::RenderTexture::RenderType::ColorBuffer | glk::RenderTexture::RenderType::DepthBuffer);

	_drawPreciseTexture0 = glk::RenderTexture::createRenderTexture(halfWinSize, glk::RenderTexture::RenderType::ColorBuffer | glk::RenderTexture::RenderType::DepthBuffer);
	_drawPreciseTexture1 = glk::RenderTexture::createRenderTexture(halfWinSize, glk::RenderTexture::RenderType::ColorBuffer | glk::RenderTexture::RenderType::DepthBuffer);

	_drawMixTexture = glk::RenderTexture::createRenderTexture(halfWinSize, glk::RenderTexture::RenderType::ColorBuffer | glk::RenderTexture::RenderType::DepthBuffer);
	_dirtyTexture = glk::GLTexture::createWithFile("tga/halo/lensdirt_lowc.tga");

	_sphereMesh = glk::Sphere::createWithSlice( 16.0f, 3.75f);

	_blurShader = BlurShader::create("shader/SunHalo/Blur_VS.glsl", "shader/SunHalo/Blur_FS.glsl");
	_haloShader = HaloShader::create("shader/SunHalo/Halo_VS.glsl", "shader/SunHalo/Halo_FS.glsl");
	_renderShader = RenderShader::create("shader/SunHalo/Render_VS.glsl", "shader/SunHalo/Render_FS.glsl");
	_normalShader = NormalShader::create("shader/SunHalo/Normal_VS.glsl","shader/SunHalo/Normal_FS.glsl");
	//视图矩阵
	initCamera(glk::GLVector3(0.5f*2.5f, 0.25f*2.5f, 2.5f*2.5f),glk::GLVector3());
	/*
	  *事件的分发
	 */
	_touchEventListener = glk::TouchEventListener::createTouchListener(this,glk_touch_selector(SunHalo::onTouchBegan),
																					glk_move_selector(SunHalo::onTouchMoved),glk_release_selector(SunHalo::onTouchEnded));
	glk::EventManager::getInstance()->addTouchEventListener(_touchEventListener,0);
}

void SunHalo::initCamera(const glk::GLVector3 &eyePosition, const glk::GLVector3 &targetPosition)
{
	_viewMatrix.lookAt(eyePosition, targetPosition, glk::GLVector3(0.0f,1.0f,0.0f));
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	_projMatrix.perspective(60.0f, winSize.width/winSize.height,0.1f,512.0f);
	_viewProjMatrix = _viewMatrix * _projMatrix;
	/*
	  *分解视图矩阵
	 */
	typedef float(*MatrixArray)[4];
	MatrixArray array = (MatrixArray)_viewMatrix.pointer();
	//求欧拉角
	float pitch = atan2f(-array[2][1],array[2][2]);
	float yaw = atan2f(array[2][0],glk::GLVector2(-array[2][1], array[2][2]).length());
	//求平移向量,逆向求旋转矩阵
	glk::Matrix rotateM;
	rotateM.rotateX(-pitch * _ANGLE_FACTOR_);
	rotateM.rotateY(-yaw * _ANGLE_FACTOR_);
	_translateVec = (glk::GLVector4(array[3][0],array[3][1],array[3][2],0.0f) * rotateM).xyz();
	_rotateVec = glk::GLVector3(pitch * _ANGLE_FACTOR_,yaw * _ANGLE_FACTOR_,0.0f);
}

void SunHalo::update(float dt)
{

}

void SunHalo::draw()
{
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	glViewport(0, 0, winSize.width / 2, winSize.height / 2);
	//绑定帧缓冲区对象
	drawSunTexture();
	//
	//glViewport(0, 0, winSize.width, winSize.height);
	//drawTexture(_drawSunTexture->getColorBuffer());
	//
	drawThickTexture();
	//
	drawPreciseTexture();
	//
	drawMixTexture();
	//
	glViewport(0, 0, winSize.width, winSize.height);
	drawNormalTexture();
}

void  SunHalo::drawSunTexture()
{
	_drawSunTexture->activeFramebuffer();
	_renderShader->perform();
	//
	_sphereMesh->bindVertexObject(0);

	glk::Matrix modelMatrix;
	modelMatrix.translate(glk::GLVector3(0.0f, 0.0f, -100.0f));

	glk::Matrix mvp = modelMatrix * _viewProjMatrix;
	_renderShader->setMVPMatrix(mvp);
	_renderShader->setColor(glk::GLVector4(1.0f, 0.90f, 0.80f,1.0f));

	_sphereMesh->drawShape();
	_drawSunTexture->disableFramebuffer();
}

void SunHalo::drawThickTexture()
{
	//跨度
	float pixelWidth = 1.0f/glk::GLContext::getInstance()->getWinSize().width;
	//绑定帧缓冲区对象
	_drawThickTexture0->activeFramebuffer();
	//
	_blurShader->perform();
	int identityVertex = glk::GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER,identityVertex);
	//
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float)*3));
	// 输入的纹理是渲染太阳的之后的纹理
	_blurShader->setBaseMap(_drawSunTexture->getColorBuffer(),0);
	// 目前的是水平方向
	_blurShader->setDirection(glk::GLVector2(pixelWidth,0.0f));
	_blurShader->setStepWidth(1.0f);
	glk::Matrix identity;
	_blurShader->setMVPMatrix(identity);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//垂直方向
	_drawThickTexture0->disableFramebuffer();
	_drawThickTexture1->activeFramebuffer();
	_blurShader->setDirection(glk::GLVector2(0.0f, pixelWidth));
	_blurShader->setBaseMap(_drawThickTexture0->getColorBuffer(), 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	//还原帧缓冲区对象
	_drawThickTexture1->disableFramebuffer();
}

void SunHalo::drawPreciseTexture()
{
	//跨度
	float pixelWidth = 1.0f / glk::GLContext::getInstance()->getWinSize().width;
	//绑定帧缓冲区对象
	_drawPreciseTexture0->activeFramebuffer();
	//
	_blurShader->perform();
	int identityVertex = glk::GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER, identityVertex);
	//
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	_blurShader->setBaseMap(_drawSunTexture->getColorBuffer(), 0);// 输入的纹理是渲染太阳的之后的纹理
	// 目前的是水平方向
	_blurShader->setDirection(glk::GLVector2(pixelWidth, 0.0f));
	_blurShader->setStepWidth(10.0f);
	glk::Matrix identity;
	_blurShader->setMVPMatrix(identity);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//垂直方向
	_drawPreciseTexture0->disableFramebuffer();
	_drawPreciseTexture1->activeFramebuffer();
	_blurShader->setDirection(glk::GLVector2(0.0f, pixelWidth));
	_blurShader->setBaseMap(_drawPreciseTexture0->getColorBuffer(), 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	//还原帧缓冲区对象
	_drawPreciseTexture1->disableFramebuffer();
}
//混合
void SunHalo::drawMixTexture()
{
	_drawMixTexture->activeFramebuffer();
	_haloShader->perform();
	//
	int identityVertex = glk::GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER, identityVertex);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float)*3));

	glk::Matrix identity;
	_haloShader->setMVPMatrix(identity);
	_haloShader->setBaseTexture(_dirtyTexture->name(),0);
	_haloShader->setHighTexture(_drawPreciseTexture1->getColorBuffer(), 1);
	_haloShader->setLowTexture(_drawThickTexture1->getColorBuffer(), 2);
	_haloShader->setDispersal(3.0f / 16.0f);
	_haloShader->setDistortion(glk::GLVector3(0.94f,0.97f,1.0f));
	_haloShader->setHaloIntensity(1.5f);
	_haloShader->setHaloWidth(0.45f);

	glk::GLVector4 sunProjPosition = glk::GLVector4(0.0f, 0.0f, -100.0f,1.0f) * _viewProjMatrix;
	_haloShader->setSunProjPosition(glk::GLVector2(sunProjPosition.x/ sunProjPosition.w * 0.5f+0.5f, sunProjPosition.y/ sunProjPosition.w*0.5f+0.5f));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	_drawMixTexture->disableFramebuffer();
}

void SunHalo::drawNormalTexture()
{
	_normalShader->perform();
	int identityVertex = glk::GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER, identityVertex);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	_normalShader->setMVPMatrix(glk::Matrix());
	_normalShader->setBaseMap(_drawMixTexture->getColorBuffer(), 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void SunHalo::drawTexture(int textureId)
{
	int defaultFramebuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFramebuffer);
	assert(!defaultFramebuffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	_normalShader->perform();
	int identityVertex = glk::GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER, identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float)*3));

	glk::Matrix identity;
	_normalShader->setMVPMatrix(identity);
	_normalShader->setBaseMap(textureId, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

bool    SunHalo::onTouchBegan(const glk::GLVector2 *touchPoint)
{
	_offsetVec = *touchPoint;
	return true;
}

void SunHalo::onTouchMoved(const glk::GLVector2 *touchPoint)
{
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	//计算旋转矩阵
	glk::GLVector2 interpolation = *touchPoint - _offsetVec;
	_rotateVec.x += -interpolation.y/winSize.height * _ANGLE_FACTOR_ * 0.3f;
	_rotateVec.y += interpolation.x/winSize.width * _ANGLE_FACTOR_ * 0.3f;
	//平移向量需要在键盘事件中计算
	_viewMatrix.identity();
	_viewMatrix.translate(_translateVec);
	_viewMatrix.rotateY(_rotateVec.y);
	_viewMatrix.rotateX(_rotateVec.x);
	_viewProjMatrix = _viewMatrix * _projMatrix;
	_offsetVec = *touchPoint;
}

void SunHalo::onTouchEnded(const glk::GLVector2 *touchPoint)
{

}
