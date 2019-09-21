/*
  *建筑物场景
  *2017/12/5
  *@Authox:xiaohuaxiong
 */
#include "GL/glew.h"
#include "engine/GLContext.h"
#include"SceneBuilding.h"
#include "engine/event/EventManager.h"
#include<assert.h>

__US_GLK__;
//关于一些按键的掩码
#define _KEY_MASK_W_ 0x1
#define _KEY_MASK_S_  0x2
#define _KEY_MASK_A_ 0x4
#define _KEY_MASK_D_ 0x8
#define _KEY_MASK_SHIFT_ 0x10
#define _KEY_MASK_VALIDE_  0x1F

SceneBuilding::SceneBuilding() :
	_camera(nullptr)
	,_touchListener(nullptr)
	,_keyListener(nullptr)
	, _buildingVertexId(0)
	, _vertexCount(0)
	, _glProgram(nullptr)
	,_lightPosition(5,10,7)
{

}

SceneBuilding::~SceneBuilding()
{
	_camera->release();
	_touchListener->release();
	EventManager::getInstance()->removeListener(_touchListener);
	_touchListener = nullptr;
	glDeleteBuffers(1,&_buildingVertexId);
}

void SceneBuilding::loadSceneBuilding(const char *filename)
{
	//加载数据
	FILE  *file = fopen(filename,"rb");
	assert(file != nullptr);
	int     vertexCount=0;
	fread(&vertexCount, sizeof(int), 1, file);
	GLVector3 *vertexData = nullptr;
	_vertexCount = vertexCount;
	if (vertexCount > 0)
	{
		vertexData = new GLVector3[vertexCount];
		fread(vertexData,sizeof(GLVector3),vertexCount,file);
		//对于每个顶点,需要有法线/纹理坐标/模型坐标
		int                 groupSize = vertexCount * 3;
		GLVector3   *VertexGroup = new GLVector3[groupSize];
		//TBN矩阵的逆矩阵
		Matrix3         tbnReverse;
		int    index = 0;
		for (int k = 0; k < vertexCount; k += 3)
		{
			//求TBN矩阵的逆矩阵
			GLVector3    abVec = vertexData[k + 1] - vertexData[k];
			GLVector3    acVec = vertexData[k+2]-vertexData[k];
			GLVector3    normal = abVec.cross(acVec).normalize();
			Matrix3::reverseTBN(normal,tbnReverse);
			GLVector3    fragCoordA = vertexData[k] * tbnReverse;
			GLVector3    fragCoordB = vertexData[k + 1] * tbnReverse;
			GLVector3    fragCoordC = vertexData[k + 2] * tbnReverse;
			
			VertexGroup[index] = vertexData[k];
			VertexGroup[index + 1] = normal;
			VertexGroup[index + 2] = fragCoordA;

			VertexGroup[index + 3] = vertexData[k+1];
			VertexGroup[index + 4] = normal;
			VertexGroup[index + 5] = fragCoordB;

			VertexGroup[index + 6] = vertexData[k+2];
			VertexGroup[index + 7] = normal;
			VertexGroup[index + 8] = fragCoordC;

			index += 9;
		}
		glGenBuffers(1,&_buildingVertexId);
		glBindBuffer(GL_ARRAY_BUFFER, _buildingVertexId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLVector3)*groupSize,VertexGroup,GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER,0);
		//
		delete[] VertexGroup;
		VertexGroup = nullptr;
	}
	fclose(file);
	//设置摄像机
	auto &winSize = GLContext::getInstance()->getWinSize();
	_camera = Camera2::createWithPerspective(45.0, winSize.width/winSize.height,0.125f,500);
	_camera->lookAt(GLVector3(0,1.75f,7.0f),GLVector3(0,1.75f,0));
	_collisionDetector.init(vertexData,vertexCount,1.75,1.25f,0.125f);
	//加载纹理
	_texture = GLTexture::createWithFile("tga/terrain/concrete.tga");
	struct TexParam  texParam = {0,0,GL_REPEAT,GL_REPEAT};
	_texture->setTexParam(texParam);
	//
	_glProgram = GLProgram::createWithFile("shader/terrain/SceneBuilding_VS.glsl","shader/terrain/SceneBuilding_FS.glsl");
	//事件派发
	_touchListener = TouchEventListener::createTouchListener(this,glk_touch_selector(SceneBuilding::onTouchPressed),glk_move_selector(SceneBuilding::onTouchMoved),glk_release_selector(SceneBuilding::onTouchReleased));
	EventManager::getInstance()->addTouchEventListener(_touchListener,0);
	//
	_keyListener = KeyEventListener::createKeyEventListener(this, glk_key_press_selector(SceneBuilding::onKeyPressed),glk_key_release_selector(SceneBuilding::onKeyRelease));
	EventManager::getInstance()->addKeyEventListener(_keyListener,0);

	delete[]vertexData;
	vertexData = nullptr;
}

void SceneBuilding::render()
{
	_glProgram->perform();
	glBindBuffer(GL_ARRAY_BUFFER,_buildingVertexId);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(GLVector3)*3,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(GLVector3)*3,(void*)(sizeof(GLVector3)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(GLVector3)*3,(void*)(sizeof(GLVector3)*2));

	int	mvpLoc			= _glProgram->getUniformLocation("g_MVPMatrix");
	int   textureLoc   = _glProgram->getUniformLocation("g_Texture");
	int	lightPositionLoc = _glProgram->getUniformLocation("g_lightPosition");
	//
	glUniformMatrix4fv(mvpLoc,1,GL_FALSE, _camera->getViewProjMatrix().pointer());
	glUniform3fv(lightPositionLoc,1,&_lightPosition.x);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_texture->getName());
	glUniform1i(textureLoc,0);

	glDrawArrays(GL_TRIANGLES, 0, _vertexCount);

}

void SceneBuilding::update(float deltaTime)
{
	//如果检测到当前键盘掩码是否有效的
	if (_keyMask & _KEY_MASK_VALIDE_)
	{
		//向左
		float               speed = 2.0f;
		float              distance = speed*deltaTime;
		//shift按键将会产生加速
		if (_keyMask & _KEY_MASK_SHIFT_)
			distance *= 1.5f;
		GLVector3    stepVec;
		if (_keyMask & _KEY_MASK_W_)
			stepVec += _camera->getForwardVector()*distance;
		if (_keyMask & _KEY_MASK_S_)
			stepVec -= _camera->getForwardVector()*distance;
		if (_keyMask & _KEY_MASK_A_)
			stepVec -= _camera->getXVector() * distance;
		if (_keyMask & _KEY_MASK_D_)
			stepVec += _camera->getXVector() * distance;
		//水平碰撞检测
		_collisionDetector.checkHorizontal(_camera->getTargetPosition(), stepVec);
		if (stepVec.x*stepVec.x + stepVec.y*stepVec.y + stepVec.z*stepVec.z != 0)
		{
			_camera->translate(stepVec);
		}
	}
	//竖直方向的状态监测
	GLVector3  movVec;
	_collisionDetector.checkVertical(_camera->getTargetPosition(), deltaTime, movVec);
	if (movVec.x*movVec.x + movVec.y*movVec.y + movVec.z*movVec.z != 0)
	{
		_camera->translate(movVec);
	}
}

bool SceneBuilding::onTouchPressed(const glk::GLVector2 &touchPoint)
{
	_offsetVec = touchPoint;
	return true;
}

void SceneBuilding::onTouchMoved(const glk::GLVector2 &touchPoint)
{
	//移动摄像机
	GLVector2 offsetVec =  _offsetVec - touchPoint;
	_camera->rotate(offsetVec.x *0.25f,- offsetVec.y*0.25f);
	_offsetVec = touchPoint;
}

void SceneBuilding::onTouchReleased(const glk::GLVector2 &touchPoint)
{

}

bool SceneBuilding::onKeyPressed(glk::KeyCodeType keyCode)
{
	switch (keyCode)
	{
		case KeyCodeType::KeyCode_W:
			_keyMask |= _KEY_MASK_W_;
			break;
		case KeyCodeType::KeyCode_S:
			_keyMask |= _KEY_MASK_S_;
			break;
		case KeyCodeType::KeyCode_A:
			_keyMask |= _KEY_MASK_A_;
			break;
		case KeyCodeType::KeyCode_D:
			_keyMask |= _KEY_MASK_D_;
			break;
		case KeyCodeType::KeyCode_SHIFT://按下此键,角色会加速
			_keyMask |= _KEY_MASK_SHIFT_;
			break;
		case KeyCodeType::KeyCode_C://蹲下
			_collisionDetector.changeCrouch();
			break;
		case KeyCodeType::KeyCode_SPACE:
			_collisionDetector.changeJump();
			break;
	}
	return true;
}

void SceneBuilding::onKeyRelease(glk::KeyCodeType keyCode)
{
	if (keyCode == KeyCodeType::KeyCode_W)
		_keyMask &= ~_KEY_MASK_W_;
	else if (keyCode == KeyCodeType::KeyCode_S)
		_keyMask &= ~_KEY_MASK_S_;
	else if (keyCode == KeyCodeType::KeyCode_A)
		_keyMask &= ~_KEY_MASK_A_;
	else if (keyCode == KeyCodeType::KeyCode_D)
		_keyMask &= ~_KEY_MASK_D_;
	else if (keyCode == KeyCodeType::KeyCode_SHIFT)
		_keyMask &= ~_KEY_MASK_SHIFT_;
}