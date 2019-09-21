/*
  *点阴影实现
  *2017年12月11日
  *@Author:xiaohuaxiong
 */
#include"GL/glew.h"
#include"PointShadow.h"
#include"engine/GLContext.h"
#include"engine/event/EventManager.h"
//
#define _KEY_MASK_W_  0x01
#define _KEY_MASK_S_   0x02
#define _KEY_MASK_A_   0x04
#define _KEY_MASK_D_   0x08
__US_GLK__;

PointShadow::PointShadow() :
	_shadowProgram(nullptr)
	,_renderProgram(nullptr)
	,_lightProgram(nullptr)
	,_camera(nullptr)
	,_shadowMap(nullptr)
	,_roomMesh(nullptr)
	,_sphereMesh(nullptr)
	,_texture(nullptr)
	,_touchListener(nullptr)
	,_keyListener(nullptr)
	,_lightPosition(0,4,0)
	,_keyMask(0)
{
}

PointShadow::~PointShadow()
{
	_shadowProgram->release();
	_renderProgram->release();
	_lightProgram->release();
	_camera->release();
	_shadowMap->release();
	_roomMesh->release();
	_sphereMesh->release();
	_texture->release();
	//
	_touchListener->release();
	_keyListener->release();
	//
	EventManager::getInstance()->removeListener(_touchListener);
	EventManager::getInstance()->removeListener(_keyListener);
}

void PointShadow::initShadow()
{
	_lightProgram = GLProgram::createWithFile("shader/point_shadow/Light_VS.glsl", "shader/point_shadow/Light_FS.glsl");
	_shadowProgram = GLProgram::createWithFile("shader/point_shadow/Shadow_VS.glsl", "shader/point_shadow/Shadow_GS.glsl","shader/point_shadow/Shadow_FS.glsl");
	_renderProgram = GLProgram::createWithFile("shader/point_shadow/Render_VS.glsl", "shader/point_shadow/Render_FS.glsl");
	//Camera
	auto &winSize = GLContext::getInstance()->getWinSize();
	_camera = Camera2::createWithPerspective(60.0f, winSize.width/winSize.height,0.125f,500.0f);
	_camera->lookAt(Vec3(),Vec3(0,0,-1));
	//shadow map
	Size  shadowSize(512,512);
	_shadowMap = ShadowMap::createWithMapLayer(shadowSize, 6);
	//mesh
	_roomMesh = Skybox::createWithScale(32);
	_sphereMesh = Sphere::createWithSlice(64, 6);
	_texture = GLTexture::createWithFile("tga/Earth512x256.tga");
	//event
	_touchListener = TouchEventListener::createTouchListener(this,glk_touch_selector(PointShadow::onTouchPressed),glk_move_selector(PointShadow::onTouchMoved),glk_release_selector(PointShadow::onTouchReleased)); 
	EventManager::getInstance()->addTouchEventListener(_touchListener, 0);
	//
	_keyListener = KeyEventListener::createKeyEventListener(this,glk_key_press_selector(PointShadow::onKeyPressed),glk_key_release_selector(PointShadow::onKeyReleased));
	EventManager::getInstance()->addKeyEventListener(_keyListener, 0);
	//计算六个面矩阵
	Mat4 proj;
	Mat4::createPerspective(90.0f, 1.0f, 1.0f, 512, proj);
	_viewProjMatrix[0] = Mat4(Vec3(0,0,1),Vec3(0,1,0),Vec3(-1,0,0), _lightPosition)*proj;//+X
	_viewProjMatrix[1] = Mat4(Vec3(0,0,-1),Vec3(0,1,0),Vec3(1,0,0), _lightPosition)*proj;//-X
	_viewProjMatrix[2] = Mat4(Vec3(1,0,0),Vec3(0,0,1),Vec3(0,-1,0), _lightPosition)*proj;//+y
	_viewProjMatrix[3] = Mat4(Vec3(1,0,0),Vec3(0,0,-1),Vec3(0,1,0), _lightPosition)*proj;//-Y
	_viewProjMatrix[4] = Mat4(Vec3(-1,0,0),Vec3(0,1,0),Vec3(0,0,-1), _lightPosition)*proj;//+Z
	_viewProjMatrix[5] = Mat4(Vec3(1,0,0),Vec3(0,1,0),Vec3(0,0,1), _lightPosition)*proj;//-Z
	//偏置矩阵
	Mat4 offset;
	offset.offset();
	for (int k = 0; k < 6; ++k)
		_viewProjOffsetMat4[k].multiply(_viewProjMatrix[k], offset);
}

void PointShadow::update(float deltaTime)
{
	if (_keyMask)
	{
		float speed = 3.0f;
		float distance = speed*deltaTime;
		//
		Vec3 stepVec;
		if (_keyMask & _KEY_MASK_W_)
			stepVec += _camera->getForwardVector() *distance;
		if (_keyMask & _KEY_MASK_S_)
			stepVec -= _camera->getForwardVector() * distance;
		if (_keyMask & _KEY_MASK_A_)
			stepVec -= _camera->getXVector() * distance;
		if (_keyMask & _KEY_MASK_D_)
			stepVec += _camera->getXVector()*distance;
		_camera->translate(stepVec);
	}
}

void PointShadow::renderShadow()
{
	//改变视口
	_shadowMap->activeShadowFramebuffer();
	//
	_shadowProgram->perform();
	int  viewProjMatrixLoc = _shadowProgram->getUniformLocation("g_ViewProjMatrix");
	glUniformMatrix4fv(viewProjMatrixLoc,6,GL_FALSE,_viewProjMatrix[0].pointer());
	//
	int  modelMatrixLoc = _shadowProgram->getUniformLocation("g_ModelMatrix");
	//房间
	Mat4 identity;
	_roomMesh->bindVertexObject(0);
	glUniformMatrix4fv(modelMatrixLoc,1,GL_FALSE,identity.pointer());
	_roomMesh->drawShape();
	//球体
	Vec3   translateVec[6] = {
		Vec3(24,0,0),
		Vec3(0,0,-24),
		Vec3(-24,0,0),
		Vec3(0,0,24),
		Vec3(0,24,0),
		Vec3(0,-24,0),
	};
	for (int k = 0; k < 6; ++k)
	{
		_sphereMesh->bindVertexObject(0);
		Mat4 translate;
		translate.translate(translateVec[k]);
		//
		glUniformMatrix4fv(modelMatrixLoc,1,GL_FALSE,translate.pointer());
		_sphereMesh->drawShape();
	}
	_shadowMap->restoreFramebuffer();
}

void PointShadow::render()
{
	//球体
	_renderProgram->perform();
	int    mvpMatrixLoc2 = _renderProgram->getUniformLocation("g_MVPMatrix");
	int    modelMatrixLoc2 = _renderProgram->getUniformLocation("g_ModelMatrix");
	int    baseMapLoc2 = _renderProgram->getUniformLocation("g_BaseMap");
	int    lightPositionLoc2 = _renderProgram->getUniformLocation("g_LightPosition");
	int    lightColorLoc2 = _renderProgram->getUniformLocation("g_LightColor");
	//四个
	_sphereMesh->bindVertexObject(0);//position
	_sphereMesh->bindTexCoordObject(1);
	_sphereMesh->bindNormalObject(2);
	//texture
	glUniform1i(baseMapLoc2,0);
	//光源的位置与颜色
	Vec3 lightColor(1);
	glUniform3fv(lightPositionLoc2,1,&_lightPosition.x);
	glUniform3fv(lightColorLoc2,1,&lightColor.x);
	//
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_texture->getName());
	glUniform1i(baseMapLoc2,0);
	////
	Vec3   translateVec[6] = {
		Vec3(24,0,0),
		Vec3(0,0,-24),
		Vec3(-24,0,0),
		Vec3(0,0,24),
		Vec3(0,24,0),
		Vec3(0,-24,0),
	};
	glUniformMatrix4fv(mvpMatrixLoc2, 1, GL_FALSE, _camera->getViewProjMatrix().pointer());
	for (int k = 0; k < 6; ++k)
	{
		Mat4  translate;
		translate.translate(translateVec[k]);

		glUniformMatrix4fv(modelMatrixLoc2,1,GL_FALSE,translate.pointer());
		_sphereMesh->drawShape();
	}
	//房间
	_lightProgram->perform();
	_roomMesh->bindVertexObject(0);
	_roomMesh->bindNormalObject(1);
	int  mvpMatrixLoc = _lightProgram->getUniformLocation("g_MVPMatrix");
	int  modelMatrixLoc = _lightProgram->getUniformLocation("g_ModelMatrix");
	int  lightPositionLoc = _lightProgram->getUniformLocation("g_LightPosition");
	int  lightColorLoc = _lightProgram->getUniformLocation("g_LightColor");
	int  colorLoc = _lightProgram->getUniformLocation("g_Color");
	int  cascadeMapLoc = _lightProgram->getUniformLocation("g_CascadeShadowMap");
	int  textureMatrixLoc = _lightProgram->getUniformLocation("g_TextureMatrix");
	//
	Vec4 color(0.9,0.9,0.9,1);
	Mat4 identity;
	glUniformMatrix4fv(mvpMatrixLoc,1,GL_FALSE,_camera->getViewProjMatrix().pointer());
	glUniformMatrix4fv(modelMatrixLoc,1,GL_FALSE,identity.pointer());
	//
	glUniform3fv(lightPositionLoc,1,&_lightPosition.x);
	glUniform3fv(lightColorLoc,1,&lightColor.x);
	glUniform4fv(colorLoc,1,&color.x);
	//
	glUniformMatrix4fv(textureMatrixLoc,6,GL_FALSE,_viewProjOffsetMat4[0].pointer());
	//
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY,_shadowMap->getDepthTexture());
	glUniform1i(cascadeMapLoc,1);
	//
	_roomMesh->drawShape();
}

bool PointShadow::onTouchPressed(const glk::Vec2 &touchPoint)
{
	_offsetVec2 = touchPoint;
	return true;
}

void PointShadow::onTouchMoved(const glk::Vec2 &touchPoint)
{
	_camera->rotate((_offsetVec2.x - touchPoint.x)*0.25f,(touchPoint.y - _offsetVec2.y)*0.25f );
	_offsetVec2 = touchPoint;
}

void PointShadow::onTouchReleased(const glk::Vec2 &touchPoint)
{

}

bool PointShadow::onKeyPressed(glk::KeyCodeType keyCode)
{
	if (keyCode == glk::KeyCodeType::KeyCode_W)
		_keyMask |= _KEY_MASK_W_;
	else if (keyCode == KeyCodeType::KeyCode_S)
		_keyMask |= _KEY_MASK_S_;
	else if (keyCode == KeyCodeType::KeyCode_A)
		_keyMask |= _KEY_MASK_A_;
	else if (keyCode == KeyCodeType::KeyCode_D)
		_keyMask |= _KEY_MASK_D_;
	return true;
}

void PointShadow::onKeyReleased(glk::KeyCodeType keyCode)
{
	if (keyCode == glk::KeyCodeType::KeyCode_W)
		_keyMask &= ~_KEY_MASK_W_;
	else if (keyCode == KeyCodeType::KeyCode_S)
		_keyMask &= ~ _KEY_MASK_S_;
	else if (keyCode == KeyCodeType::KeyCode_A)
		_keyMask &=~ _KEY_MASK_A_;
	else if (keyCode == KeyCodeType::KeyCode_D)
		_keyMask &= ~_KEY_MASK_D_;
}