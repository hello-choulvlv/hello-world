/*
  *VSM场景实现
  *2018年8月4日
  *author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "VSM.h"
#include "engine/GLContext.h"
#include "engine/event/EventManager.h"
using namespace glk;
VSM::VSM():
	_shadowVSM(nullptr),
	_camera(nullptr),
	_lightProgram(nullptr),
	_pyramidMesh(nullptr),
	_groundMesh(nullptr),
	_viewProjMatrixLoc(-1),
	_modelMatrixLoc(-1),
	_normalMatrixLoc(-1),
	_lightPositionLoc(-1),
	_lightAmbientColorLoc(-1),
	_lightColorLoc(-1),
	_lightColor(1.0f,0.72f,0.72f,1.0f),
	_lightAmbientColor(0.25f,0.25f,0.25f),
	_lightBleeding(0.18f),
	_touchListener(nullptr),
	_keyListener(nullptr),
	_keyMask(0),
	_analysisScene(0),
	_changeViewMode(0)
{
}

VSM::~VSM()
{
	_camera->release();
	_lightProgram->release();
	_pyramidMesh->release();
	_groundMesh->release();
}

void  VSM::init()
{
	_lightProgram = GLProgram::createWithFile("shader/vsm/light_v.glsl", "shader/vsm/light_f.glsl");
	_modelMatrixLoc = _lightProgram->getUniformLocation("g_ModelMatrix");
	_viewProjMatrixLoc = _lightProgram->getUniformLocation("g_ViewProjMatrix");
	_normalMatrixLoc = _lightProgram->getUniformLocation("g_NormalMatrix");
	_shadowMapLoc = _lightProgram->getUniformLocation("g_ShadowMap");
	//_lightMVMatrixLoc = _lightProgram->getUniformLocation("g_LightMVMatrix");
	_lightViewProjMatrixLoc = _lightProgram->getUniformLocation("g_LightViewProjMatrix");
	_lightPositionLoc = _lightProgram->getUniformLocation("g_LightPosition");
	_lightColorLoc = _lightProgram->getUniformLocation("g_Color");
	_lightAmbientColorLoc = _lightProgram->getUniformLocation("g_AmbientColor");
	_lightBleedingLoc = _lightProgram->getUniformLocation("g_LightBleeding");

	_shadowProgram = GLProgram::createWithFile("shader/vsm/ShadowVSM_v.glsl","shader/vsm/ShadowVSM_f.glsl");
	_shadowViewProjMatrixLoc = _shadowProgram->getUniformLocation("g_ViewProjMatrix");
	_shadowModelMatrixLoc = _shadowProgram->getUniformLocation("g_ModelMatrix");

	auto &winSize = GLContext::getInstance()->getWinSize();
	GLVector3    eyePosition(4,4,4);
	GLVector3    targetPosition(0);
	//_camera = Camera2::createCamera(eyePosition,targetPosition,GLVector3(0,1,0));
	_camera = Camera2::createWithPerspective(45.0f, winSize.width / winSize.height, 0.1f, 20.0f);
	_camera->lookAt(eyePosition, targetPosition);
	//_camera->setPerspective(45.0f, winSize.width / winSize.height, 0.1f, 20.0f);

	_pyramidMesh = Pyramid::create(2.0f);
	_groundMesh = Mesh::createWithIntensity(4, 4,4,1.0f);
	_sphereMesh = Sphere::createWithSlice(64, 1.5f);

	_pyramidModelMatrix.translate(0, 1.0f, 0);
	_groundModelMatrix.rotate(-90,1.0f,0.0f,0.0f);
	_groundModelMatrix.trunk(_groundNormalMatrix);
	_sphereModelMatrix.translate(2.5f,1.5f,-1.0f);
	_sphereModelMatrix.trunk(_sphereNormalMatrix);

	_lightDirection = GLVector3(1,1,0.25f).normalize();

	_touchListener = TouchEventListener::createTouchListener(this,glk_touch_selector(VSM::onTouchBegan),glk_move_selector(VSM::onTouchMoved),glk_release_selector(VSM::onTouchEnded));
	EventManager::getInstance()->addTouchEventListener(_touchListener, 0);

	_keyListener = KeyEventListener::createKeyEventListener(this, glk_key_press_selector(VSM::onKeyPressed),glk_key_release_selector(VSM::onKeyReleased));
	EventManager::getInstance()->addKeyEventListener(_keyListener,0);

	_shadowVSM = ShadowMapVSM::create(Size(512,512));
	updateLightMatrix();
}

void VSM::updateLightMatrix()
{
	//计算视锥体远近平面
	const GLVector4 &nearFarVec = _camera->getNearFarFovRatio();
	float   z = (nearFarVec.x + powf(nearFarVec.y - nearFarVec.x, 0.5f))*0.5 + (nearFarVec.x + (nearFarVec.y - nearFarVec.x)*0.5f)*0.5f;
	//计算光源的视图矩阵
	GLVector3    lightTargetPosition(0,0,0);
	_lightPosition =  _lightDirection * 10 + lightTargetPosition;
	_lightViewMatrix.identity();
	_lightViewMatrix.lookAt(_lightPosition, lightTargetPosition, GLVector3(0,1,0));
	//光源的投影矩阵,此矩阵为正交投影矩阵,并且使用了LiSM中的一些技巧
	Matrix view_inverse,proj_inverse, matrix_inverse;
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
	_lightProjMatrix.identity();
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
	if(!_analysisScene)
		_lightProjMatrix.orthoProject(box_min.x,box_max.x,box_min.y,box_max.y,0,-box_min.z);
	else
	{
		Shape   *object[3] = { _groundMesh,_pyramidMesh,_sphereMesh };
		Matrix   *modelTransform[3] = { &_groundModelMatrix,&_pyramidModelMatrix,&_sphereModelMatrix };
		GLVector3   min_bb, max_bb;
		bool               first_union = true;
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
				min_bb.x = min_f(min_bb.x, box_min_2.x);
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
		min_bb.x = max_f(min_bb.x, box_min.x);
		min_bb.y = max_f(min_bb.y, box_min.y);
		min_bb.z = max_f(min_bb.z, box_min.z);

		max_bb.x = min_f(max_bb.x, box_max.x);
		max_bb.y = min_f(max_bb.y, box_max.y);
		max_bb.z = min_f(max_bb.z, box_max.z);

		_lightProjMatrix.orthoProject(min_bb.x, max_bb.x, min_bb.y, max_bb.y, 0, -min_bb.z);
	}
#undef min_f
#undef max_f
	_lightViewProjMatrix = _lightViewMatrix * _lightProjMatrix;
}

void  VSM::update(float deltaTime,float delayTime)
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
	}
	//_pyramidModelMatrix.identity();
	//_pyramidModelMatrix.rotate(delayTime, GLVector3(0.0f, 1.0f, 0.0f));
	//_pyramidModelMatrix.translate(0, 1.0f, 0.0f);
	//_pyramidModelMatrix.trunk(_pyramidNormalMatrix);
	updateLightMatrix();
}

void  VSM::render()
{
	auto &winSize = GLContext::getInstance()->getWinSize();
	glViewport(0, 0,512,512);
	glEnable(GL_POLYGON_OFFSET_FILL);
	//渲染阴影
	_shadowVSM->save();
	_shadowProgram->perform();
	//ground
	_groundMesh->bindVertexObject(0);
	glUniformMatrix4fv(_shadowModelMatrixLoc, 1, GL_FALSE,_groundModelMatrix.pointer());
	glUniformMatrix4fv(_shadowViewProjMatrixLoc, 1, GL_FALSE, _lightViewProjMatrix.pointer());
	glUniform3fv(_shadowLightPositionLoc,1,&_lightPosition.x);
	_groundMesh->drawShape();
	//Pyramid
	_pyramidMesh->bindVertexObject(0);
	glUniformMatrix4fv(_shadowModelMatrixLoc, 1, GL_FALSE, _pyramidModelMatrix.pointer());
	_pyramidMesh->drawShape();
	//Sphere Mesh
	_sphereMesh->bindVertexObject(0);
	glUniformMatrix4fv(_shadowModelMatrixLoc,1,GL_FALSE,_sphereModelMatrix.pointer());
	_sphereMesh->drawShape();

	glDisable(GL_DEPTH_TEST);
	_shadowVSM->fuzzy();
	_shadowVSM->restore();
	glViewport(0, 0, winSize.width, winSize.height);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);
	//
	//Matrix3    normal_matrix;
	//ground
	_lightProgram->perform();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_shadowVSM->getColorTexture());
	glUniform1i(_shadowMapLoc,0);
	glUniformMatrix4fv(_lightViewProjMatrixLoc,1,GL_FALSE,_lightViewProjMatrix.pointer());
	glUniform1f(_lightBleedingLoc,_lightBleeding);

	glUniformMatrix4fv(_modelMatrixLoc,1,GL_FALSE,_groundModelMatrix.pointer());
	if (!_changeViewMode)
		glUniformMatrix4fv(_viewProjMatrixLoc, 1, GL_FALSE, _camera->getViewProjMatrix().pointer());
	else
		glUniformMatrix4fv(_viewProjMatrixLoc,1,GL_FALSE,_lightViewProjMatrix.pointer());
	glUniformMatrix3fv(_normalMatrixLoc,1,GL_FALSE, _groundNormalMatrix.pointer());
	//glUniformMatrix4fv(_lightMVMatrixLoc,1,GL_FALSE,(_groundModelMatrix*_lightViewMatrix).pointer());

	glUniform3fv(_lightPositionLoc,1,&_lightPosition.x);
	glUniform4fv(_lightColorLoc,1,&_lightColor.x);
	glUniform3fv(_lightAmbientColorLoc,1,&_lightAmbientColor.x);

	_groundMesh->bindVertexObject(0);
	_groundMesh->bindNormalObject(1);

	_groundMesh->drawShape();

	//pyramid
	GLVector4    color(0.6, 0.6, 1.0f, 1.0f);
	glUniformMatrix4fv(_modelMatrixLoc, 1, GL_FALSE, _pyramidModelMatrix.pointer());
	glUniformMatrix3fv(_normalMatrixLoc,1,GL_FALSE, _pyramidNormalMatrix.pointer());
	//glUniformMatrix4fv(_lightMVMatrixLoc,1,GL_FALSE,(_pyramidModelMatrix*_lightViewMatrix).pointer());
	glUniform4fv(_lightColorLoc, 1, &color.x);
	_pyramidMesh->bindVertexObject(0);
	_pyramidMesh->bindNormalObject(1);
	_pyramidMesh->drawShape();

	color = GLVector4(1.0f,1.0f,1.0f,1.0f);
	glUniform4fv(_lightColorLoc, 1, &color.x);
	glUniformMatrix4fv(_modelMatrixLoc,1,GL_FALSE,_sphereModelMatrix.pointer());
	glUniformMatrix3fv(_normalMatrixLoc,1,GL_FALSE,_sphereNormalMatrix.pointer());
	//glUniformMatrix4fv(_lightMVMatrixLoc,1,GL_FALSE,(_sphereModelMatrix*_lightViewMatrix).pointer());
	_sphereMesh->bindVertexObject(0);
	_sphereMesh->bindNormalObject(1);
	_sphereMesh->drawShape();
}

bool		VSM::onTouchBegan(const glk::GLVector2 &touchPoint)
{
	_touchPoint = touchPoint;
	return true;
}

void  VSM::onTouchMoved(const glk::GLVector2 &touchPoint)
{
	GLVector2 offset = touchPoint - _touchPoint;
	auto &winSize = GLContext::getInstance()->getWinSize();

	//_camera->updateRotateMatrix(-offset.y,offset.x);
	_camera->rotate(-offset.x * 0.5,offset.y * 0.5);

	_touchPoint = touchPoint;
}
void  VSM::onTouchEnded(const glk::GLVector2 &touchPoint)
{

}

bool VSM::onKeyPressed(glk::KeyCodeType keyCode)
{
	if (keyCode == KeyCode_W)
		_keyMask |= 1;
	if (keyCode == KeyCode_S)
		_keyMask |= 2;
	if (keyCode == KeyCode_A)
		_keyMask |= 4;
	if (keyCode == KeyCode_D)
		_keyMask |= 8;
	if (keyCode == KeyCode_F5)
		_analysisScene = !_analysisScene;
	if (keyCode == KeyCode_F4)
		_changeViewMode = !_changeViewMode;
	return true;
}

void VSM::onKeyReleased(glk::KeyCodeType keyCode)
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