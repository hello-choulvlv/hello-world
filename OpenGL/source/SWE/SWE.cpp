/*
  *Shallow Water Equation
  *2018Äê12ÔÂ28ÈÕ
  *@author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "SWE.h"
#include "engine/GLCacheManager.h"
#include "engine/GLContext.h"

#include "engine/event/EventManager.h"
using namespace glk;
static const int s_MeshWidth = 64;
SWE::SWE():
	_glProgram(nullptr)
	,_normalTexture(0)
{
}

SWE::~SWE()
{
	glDeleteTextures(1, &_normalTexture);
	_normalTexture = 0;
	_glProgram->release();
	_glProgram = nullptr;

	EventManager::getInstance()->removeListener(_touchEventListener);
	_touchEventListener->release();
	_touchEventListener = nullptr;
}

void  SWE::initSWE()
{
	//generate swe liquid
	_liquid.init(s_MeshWidth + 2, s_MeshWidth + 2);
	glGenTextures(1, &_normalTexture);
	glBindTexture(GL_TEXTURE_2D,_normalTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, s_MeshWidth, s_MeshWidth);
	glBindTexture(GL_TEXTURE_2D,0);

	_touchEventListener = TouchEventListener::createTouchListener(this,glk_touch_selector(SWE::onTouchBegan),
		glk_move_selector(SWE::onTouchMoved),
		glk_release_selector(SWE::onTouchEnded));
	EventManager::getInstance()->addTouchEventListener(_touchEventListener, 0);

	_glProgram = GLProgram::createWithFile("shader/swe/swe_vs.glsl", "shader/swe/swe_fs.glsl");
	_mainTextureLoc = _glProgram->getUniformLocation("g_MainTexture");
	_secondaryTextureLoc = _glProgram->getUniformLocation("g_SecondaryTexture");
	_normalTextureLoc = _glProgram->getUniformLocation("g_NormalTexture");

	_mainTexture = GLTexture::createWithFile("tga/swe/marble.tga");
	_secondaryTexture = GLTexture::createWithFile("tga/swe/frac2.tga");
}

void  SWE::update(float t)
{
	_liquid.update(0.1);
}

void  SWE::render()
{
	_glProgram->perform();
	int identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER,identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void *)(sizeof(float)*3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _mainTexture->getName());
	glUniform1i(_secondaryTextureLoc,0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,_secondaryTexture->getName());
	glUniform1i(_mainTextureLoc,1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,_normalTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, s_MeshWidth, s_MeshWidth, GL_RGBA, GL_UNSIGNED_BYTE, _liquid.getNormal());
	glUniform1i(_normalTextureLoc,2);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

bool SWE::onTouchBegan(const glk::GLVector2 &touchPoint)
{
	auto &winSize = GLContext::getInstance()->getWinSize();
	_liquid.touchAt(touchPoint.x / winSize.width, touchPoint.y / winSize.height);
	return true;
}

void  SWE::onTouchMoved(const glk::GLVector2 &touchPoint)
{
	auto &winSize = GLContext::getInstance()->getWinSize();
	_liquid.touchAt(touchPoint.x / winSize.width, touchPoint.y / winSize.height);
}

void  SWE::onTouchEnded(const glk::GLVector2 &touchPoint)
{

}