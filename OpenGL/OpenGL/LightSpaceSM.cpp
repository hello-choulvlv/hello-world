/*
  *光空间透视阴影
  *2018年8月8日
  *@author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "LightSpaceSM.h"
#include "engine/GLCacheManager.h"
#include "engine/GLContext.h"
#include "engine/event/EventManager.h"
using namespace glk;

LightSpaceSM::LightSpaceSM() :
	_shadowMap(nullptr),
	_shadowProgram(nullptr),
	_camera(nullptr),
	_pyramidMesh(nullptr),
	_groundMesh(nullptr),
	_sphereMesh(nullptr),
	_lightProgram(nullptr),
	_touchListener(nullptr),
	_keyListener(nullptr),
	_lightAmbientColor(0.25f, 0.25f, 0.25f),
	_lightColor(1.0f),
	_keyMask(0),
	_viewMode(0),
	_showDebug(0),
	_analysisScene(0)
{
}

LightSpaceSM::~LightSpaceSM()
{
	_shadowMap->release();
	_shadowProgram->release();
	_camera->release();
	_lightProgram->release();

	EventManager::getInstance()->removeListener(_touchListener);
	EventManager::getInstance()->removeListener(_keyListener);

	_touchListener->release();
	_keyListener->release();
}

LightSpaceSM *LightSpaceSM::create()
{
	LightSpaceSM *lism = new LightSpaceSM();
	lism->init();
	return lism;
}

bool LightSpaceSM::init()
{
	int    mapSize = 512;
	_shadowMap = ShadowMap::createWithMapSize(Size(mapSize,mapSize));
	_shadowProgram = GLCacheManager::getInstance()->findGLProgram(GLCacheManager::GLProgramType_ShadowMap);
	_shadowProgram->retain();
	_shadowMVPMatrixLoc = _shadowProgram->getUniformLocation("g_MVPMatrix");

	_lightProgram = GLProgram::createWithFile("shader/lism/light_v.glsl", "shader/lism/light_f.glsl");
	_lightModelMatrixLoc = _lightProgram->getUniformLocation("g_ModelMatrix");
	_lightViewProjMatrixLoc = _lightProgram->getUniformLocation("g_ViewProjMatrix");
	_lightNormalMatrixLoc = _lightProgram->getUniformLocation("g_NormalMatrix");
	_lightShadowMapLoc = _lightProgram->getUniformLocation("g_ShadowMap");
	_lightLightViewProjMatrixLoc = _lightProgram->getUniformLocation("g_LightViewProjMatrix");
	_lightLightDirectionLoc = _lightProgram->getUniformLocation("g_LightDirection");
	_lightAmbientColorLoc = _lightProgram->getUniformLocation("g_AmbientColor");
	_lightColorLoc = _lightProgram->getUniformLocation("g_Color");

	_debugProgram = GLCacheManager::getInstance()->findGLProgram(GLCacheManager::GLProgramType_DebugDepthTexture);
	_debugMVPMatrixLoc = _debugProgram->getUniformLocation("g_MVPMatrix");
	_debugBaseMapLoc = _debugProgram->getUniformLocation("g_BaseMap");

	_pyramidMesh = Pyramid::create(2.0f);
	_groundMesh = Mesh::createWithIntensity(4, 4, 4, 1.0f);
	_sphereMesh = Sphere::createWithSlice(64, 1.5f);

	_pyramidModelMatrix.translate(0, 1.0f, 0);
	_groundModelMatrix.rotate(-90, 1.0f, 0.0f, 0.0f);
	_groundModelMatrix.trunk(_groundNormalMatrix);
	_sphereModelMatrix.translate(2.5f, 1.5f, -1.0f);
	_sphereModelMatrix.trunk(_sphereNormalMatrix);

	_touchListener = TouchEventListener::createTouchListener(this, glk_touch_selector(LightSpaceSM::onTouchBegan),glk_move_selector(LightSpaceSM::onTouchMoved),glk_release_selector(LightSpaceSM::onTouchEnded));
	EventManager::getInstance()->addTouchEventListener(_touchListener,0);

	_keyListener = KeyEventListener::createKeyEventListener(this, glk_key_press_selector(LightSpaceSM::onKeyPressed),glk_key_release_selector(LightSpaceSM::onKeyReleased));
	EventManager::getInstance()->addKeyEventListener(_keyListener, 0);

	auto &winSize = GLContext::getInstance()->getWinSize();
	GLVector3    eyePosition(4, 4, 4);
	GLVector3    targetPosition(0);
	_camera = Camera2::createWithPerspective(45.0f, winSize.width / winSize.height, 0.1f, 12.0f);
	_camera->lookAt(eyePosition, targetPosition);

	_lightDirection = GLVector3(1.0f,1.0f,0.25f).normalize();
	updateLightSpaceFrustum();
	return true;
}

void LightSpaceSM::updateLightSpaceFrustum()
{
	//计算视锥体远近平面
	const GLVector4 &nearFarVec = _camera->getNearFarFovRatio();
	float   z = (nearFarVec.x + powf(nearFarVec.y - nearFarVec.x, 0.5f))*0.5 + (nearFarVec.x + (nearFarVec.y - nearFarVec.x)*0.5f)*0.5f;
	//计算光源的视图矩阵
	GLVector3    lightTargetPosition(0, 0, 0);
	GLVector3    lightPosition = _lightDirection * 10 + lightTargetPosition;
	_lightViewMatrix.identity();
	_lightViewMatrix.lookAt(lightPosition, lightTargetPosition, GLVector3(0, 1, 0));
	//光源的投影矩阵,此矩阵为正交投影矩阵,并且使用了LiSM中的一些技巧
	Matrix view_inverse, proj_inverse, matrix_inverse;
	_camera->getViewProjMatrix().reverse(matrix_inverse);

	GLVector4	    boundbox[8] = {
		GLVector4(-1,-1,-1,1),
		GLVector4(1,-1,-1,1),
		GLVector4(1,1,-1,1),
		GLVector4(-1,1,-1,1),

		GLVector4(-1,-1,1,1),
		GLVector4(1,-1,1,1),
		GLVector4(1,1,1,1),
		GLVector4(-1,1,1,1),
	};
	//计算包围盒
	GLVector3   box_min(FLT_MAX);
	GLVector3   box_max(-FLT_MAX);
#define min_f(x,y)   x<=y?x:y;
#define max_f(x,y)   x>=y?x:y;
	for (int k = 0; k < 8; ++k)
	{
		GLVector4   corner = boundbox[k] * matrix_inverse;
		corner /= corner.w;
		GLVector4   vertex = corner * _lightViewMatrix;

		box_min.x = min_f(box_min.x, vertex.x);
		box_min.y = min_f(box_min.y, vertex.y);
		box_min.z = min_f(box_min.z, vertex.z);

		box_max.x = max_f(box_max.x, vertex.x);
		box_max.y = max_f(box_max.y, vertex.y);
		box_max.z = max_f(box_max.z, vertex.z);
	}

	_lightProjMatrix.identity();
	//是否启用了场景分析
	if(!_analysisScene)
		_lightProjMatrix.orthoProject(box_min.x, box_max.x, box_min.y, box_max.y,0, -box_min.z);
	else
	{
		Shape   *object[3] = {_groundMesh,_pyramidMesh,_sphereMesh};
		Matrix   *modelTransform[3] = {&_groundModelMatrix,&_pyramidModelMatrix,&_sphereModelMatrix};
		GLVector3   min_bb, max_bb;
		bool               first_union=true;
		for (int k = 0; k < 3; ++k)
		{
			const AABB &aabb = object[k]->getAABB();
			const GLVector4 min_b = aabb._minBox.xyzw1() * *modelTransform[k];
			const GLVector4 max_b = aabb._maxBox.xyzw1() * *modelTransform[k];

			GLVector3   bounding[8] = {
				min_b.xyz(),//1
				GLVector3(max_b.x,min_b.y,min_b.z),
				GLVector3(max_b.x,max_b.y,min_b.z),
				GLVector3(min_b.x,max_b.y,min_b.z),

				GLVector3(min_b.x,min_b.y,max_b.z),
				GLVector3(max_b.x,min_b.y,max_b.z),
				max_b.xyz(),
				GLVector3(min_b.x,max_b.y,max_b.z),
			};
			GLVector3   box_min_2(FLT_MAX);
			GLVector3   box_max_2(-FLT_MAX);
			for (int j = 0; j < 8; ++j)
			{
				GLVector4 vertex = GLVector4(bounding[j], 1.0) * _lightViewMatrix;
				box_min_2.x = min_f(box_min_2.x, vertex.x);
				box_min_2.y = min_f(box_min_2.y, vertex.y);
				box_min_2.z = min_f(box_min_2.z, vertex.z);

				box_max_2.x = max_f(box_max_2.x, vertex.x);
				box_max_2.y = max_f(box_max_2.y, vertex.y);
				box_max_2.z = max_f(box_max_2.z, vertex.z);
			}
			if (!first_union)
			{
				min_bb.x = min_f(min_bb.x,box_min_2.x);
				min_bb.y = min_f(min_bb.y, box_min_2.y);
				min_bb.z = min_f(min_bb.z, box_min_2.z);

				max_bb.x = max_f(max_bb.x, box_max_2.x);
				max_bb.y = max_f(max_bb.y, box_max_2.y);
				max_bb.z = max_f(max_bb.z, box_max_2.z);
			}
			else
			{
				first_union = false;
				min_bb = box_min_2;
				max_bb = box_max_2;
			}
		}
		//将对象包围盒与视锥体包围盒融合
		min_bb.x = max_f(min_bb.x,box_min.x);
		min_bb.y = max_f(min_bb.y,box_min.y);
		min_bb.z = max_f(min_bb.z,box_min.z);

		max_bb.x = min_f(max_bb.x,box_max.x);
		max_bb.y = min_f(max_bb.y,box_max.y);
		max_bb.z = min_f(max_bb.z,box_max.z);

		_lightProjMatrix.orthoProject(min_bb.x, max_bb.x, min_bb.y, max_bb.y, 0, -min_bb.z);
	}
#undef min_f
#undef max_f
	_lightViewProjMatrix = _lightViewMatrix * _lightProjMatrix;
}

void  LightSpaceSM::update(float deltaTime, float delayTime)
{
	if (_keyMask)
	{
		GLVector3 move;
		if (_keyMask & 0x1)
			move += _camera->getForwardVector();
		if (_keyMask & 0x2)
			move -= _camera->getForwardVector();
		if (_keyMask & 0x4)
			move -= _camera->getXVector();
		if (_keyMask & 0x8)
			move += _camera->getXVector();
		_camera->translate(move * 0.05);
		//updateLightSpaceFrustum();
	}
	updateLightSpaceFrustum();
}

void  LightSpaceSM::render()
{
	int mapSize = 512;
	auto &winSize = GLContext::getInstance()->getWinSize();
	glViewport(0, 0, mapSize, mapSize);
	glColorMask(0, 0, 0, 0);
	_shadowMap->activeShadowFramebuffer();

	_shadowProgram->perform();

	glUniformMatrix4fv(_shadowMVPMatrixLoc,1,GL_FALSE,(_groundModelMatrix*_lightViewProjMatrix).pointer());
	_groundMesh->bindVertexObject(0);
	_groundMesh->drawShape();

	glUniformMatrix4fv(_shadowMVPMatrixLoc,1,GL_FALSE,(_pyramidModelMatrix*_lightViewProjMatrix).pointer());
	_pyramidMesh->bindVertexObject(0);
	_pyramidMesh->drawShape();

	glUniformMatrix4fv(_shadowMVPMatrixLoc,1,GL_FALSE,(_sphereModelMatrix*_lightViewProjMatrix).pointer());
	_sphereMesh->bindVertexObject(0);
	_sphereMesh->drawShape();

	_shadowMap->restoreFramebuffer();
	glViewport(0, 0,winSize.width,winSize.height );
	glColorMask(1,1,1,1);
	//Shading
	_lightProgram->perform();
	//场景的视角
	if (!_viewMode)
		glUniformMatrix4fv(_lightViewProjMatrixLoc, 1, GL_FALSE, _camera->getViewProjMatrix().pointer());
	else
		glUniformMatrix4fv(_lightViewProjMatrixLoc, 1, GL_FALSE, _lightViewProjMatrix.pointer());
	glUniformMatrix4fv(_lightLightViewProjMatrixLoc,1,GL_FALSE,_lightViewProjMatrix.pointer());
	glUniform3f(_lightLightDirectionLoc,_lightDirection.x,_lightDirection.y,_lightDirection.z);
	glUniform3fv(_lightAmbientColorLoc,1,&_lightAmbientColor.x);
	glUniform4fv(_lightColorLoc,1,&_lightColor.x);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_shadowMap->getDepthTexture());
	glUniform1i(_lightShadowMapLoc,0);

	//ground mesh
	GLVector4 color(1.0f, 0.72f, 0.72f, 1.0f);
	glUniform4fv(_lightColorLoc,1,&color.x);
	glUniformMatrix4fv(_lightModelMatrixLoc,1,GL_FALSE,_groundModelMatrix.pointer());
	glUniformMatrix3fv(_lightNormalMatrixLoc,1,GL_FALSE,_groundNormalMatrix.pointer());
	_groundMesh->bindVertexObject(0);
	_groundMesh->bindNormalObject(1);
	_groundMesh->drawShape();

	//pyramid
	color = GLVector4(0.6, 0.6, 1.0f, 1.0f);
	glUniform4fv(_lightColorLoc,1,&color.x);
	glUniformMatrix4fv(_lightModelMatrixLoc, 1, GL_FALSE, _pyramidModelMatrix.pointer());
	glUniformMatrix3fv(_lightNormalMatrixLoc, 1, GL_FALSE, _pyramidNormalMatrix.pointer());
	_pyramidMesh->bindVertexObject(0);
	_pyramidMesh->bindNormalObject(1);
	_pyramidMesh->drawShape();

	//Sphere
	color = GLVector4(1);
	glUniform4fv(_lightColorLoc, 1, &color.x);
	glUniformMatrix4fv(_lightModelMatrixLoc, 1, GL_FALSE, _sphereModelMatrix.pointer());
	glUniformMatrix3fv(_lightNormalMatrixLoc, 1, GL_FALSE, _sphereNormalMatrix.pointer());
	_sphereMesh->bindVertexObject(0);
	_sphereMesh->bindNormalObject(1);
	_sphereMesh->drawShape();

	if(_showDebug)
		debugRender();
}

void    LightSpaceSM::debugRender()
{
	int    viewSize = 256;
	auto &winSize = GLContext::getInstance()->getWinSize();
	glViewport(0,0,viewSize,viewSize);
	glDisable(GL_DEPTH_TEST);

	_debugProgram->perform();
	glUniformMatrix4fv(_debugMVPMatrixLoc, 1, GL_FALSE, _debugMVPMatrix.pointer());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_shadowMap->getDepthTexture());
	glUniform1i(_debugBaseMapLoc,0);

	int identity_vertex_id = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER,identity_vertex_id);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)(sizeof(float)*3));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glEnable(GL_DEPTH_TEST);
	glViewport(0,0,winSize.width,winSize.height);
}

bool		LightSpaceSM::onTouchBegan(const glk::GLVector2 &touchPoint)
{
	_touchOffset = touchPoint;
	return true;
}

void  LightSpaceSM::onTouchMoved(const glk::GLVector2 &touchPoint)
{
	GLVector2 offset = touchPoint - _touchOffset;
	auto &winSize = GLContext::getInstance()->getWinSize();

	//_camera->updateRotateMatrix(-offset.y,offset.x);
	_camera->rotate(-offset.x * 0.5, offset.y * 0.5);
	//updateLightSpaceFrustum();

	_touchOffset = touchPoint;
}
void  LightSpaceSM::onTouchEnded(const glk::GLVector2 &touchPoint)
{

}

bool LightSpaceSM::onKeyPressed(glk::KeyCodeType keyCode)
{
	if (keyCode == KeyCode_W)
		_keyMask |= 1;
	if (keyCode == KeyCode_S)
		_keyMask |= 2;
	if (keyCode == KeyCode_A)
		_keyMask |= 4;
	if (keyCode == KeyCode_D)
		_keyMask |= 8;
	if (keyCode == KeyCode_F4)//切换视角
		_viewMode = (_viewMode + 1) & 0x1;
	if (keyCode == KeyCode_F3)//深度纹理调试
		_showDebug = !_showDebug;
	if (keyCode == KeyCode_F5)//场景分析
		_analysisScene = !_analysisScene;
	return true;
}

void LightSpaceSM::onKeyReleased(glk::KeyCodeType keyCode)
{
	if (keyCode == KeyCode_W)
		_keyMask &= ~1;
	if (keyCode == KeyCode_S)
		_keyMask &= ~2;
	if (keyCode == KeyCode_A)
		_keyMask &= ~4;
	if (keyCode == KeyCode_D)
		_keyMask &= ~8;
}