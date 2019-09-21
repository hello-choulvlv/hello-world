/*
  *水渲染,计算着色器实现
  *2017-7-27
  *@Author:xiaohuaxiong
*/
#include "GL/glew.h"
#include "engine/event/EventManager.h"
#include "engine/GLContext.h"
#include "ComputeWater.h"
#include<stdlib.h>

#define _MESH_SIZE_    128
//键盘掩码
#define _KEY_MASK_W_    0x1
#define _kEY_MASK_S_      0x2
#define _KEY_MASK_A_     0x4
#define _KEY_MASK_D_     0x8

ComputeWater::ComputeWater() :
	_waterMesh(nullptr)
	,_meshSize(_MESH_SIZE_)
	,_meshUnitSize(1.0f)
	,_texCubeMap(nullptr)
	,_computeWaterShader(nullptr)
	,_computeNormalShader(nullptr)
	,_renderShader(nullptr)
	,_heightFieldBuffer(0)
	, _velocityFieldBuffer(0)
	,_outHeightFieldBuffer(0)
	,_normalFieldBuffer(0)
	,_camera(nullptr)
	,_touchEventListener(nullptr)
	,_keyEventListener(nullptr)
	,_keyMask(0)
	,_deltaTime(0.0f)
{

}

ComputeWater::~ComputeWater()
{
	_waterMesh->release();
	_texCubeMap->release();
	_computeWaterShader->release();
	_computeNormalShader->release();
	_renderShader->release();
	glDeleteBuffers(1, &_heightFieldBuffer);
	glDeleteBuffers(1, &_velocityFieldBuffer);
	glDeleteBuffers(1, &_outHeightFieldBuffer);
	glDeleteBuffers(1, &_normalFieldBuffer);

	_camera->release();

	glk::EventManager::getInstance()->removeListener(_touchEventListener);
	_touchEventListener->release();

	glk::EventManager::getInstance()->removeListener(_keyEventListener);
	_keyEventListener->release();

	_waterMesh = nullptr;
	_texCubeMap = nullptr;
	_computeWaterShader = nullptr;
	_computeNormalShader = nullptr;
	_renderShader = nullptr;
	_heightFieldBuffer = 0;
	_velocityFieldBuffer = 0;
	_normalFieldBuffer = 0;
	_camera = nullptr;
	_touchEventListener = nullptr;
	_keyEventListener = nullptr;
}

ComputeWater *ComputeWater::create()
{
	ComputeWater *water = new ComputeWater();
	water->init();
	return water;
}

void   ComputeWater::init()
{
	_waterMesh = glk::Mesh::createWithIntensity(_meshSize, _meshSize, _meshSize, 1.0f);
	_computeWaterShader = WaterComputeShader::create("shader/ComputeWater/ComputeWaterHeight_CS.glsl");
	_computeNormalShader = WaterNormalShader::create("shader/ComputeWater/ComputeWaterNormal_CS.glsl");
	_renderShader = RenderShader::create("shader/ComputeWater/Render_VS.glsl", "shader/ComputeWater/Render_FS.glsl");
	_camera = glk::Camera::createCamera(glk::GLVector3(0.0f, _MESH_SIZE_ / 1.5f, 0.0f), glk::GLVector3(0.0f, 0.0f, -_MESH_SIZE_ / 1.2f),glk::GLVector3(0.0f,1.0f,0.0f));
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	_camera->setPerspective(60.0f, winSize.width / winSize.height, 0.1f, 1000.0f);
	const char *fileList[6] = {
		"tga/water/sky/zpos.bmp",//+Z
		"tga/water/sky/zneg.bmp",//-Z
		"tga/water/sky/xpos.bmp",//+X
		"tga/water/sky/xneg.bmp",//-X
		"tga/water/sky/ypos.bmp",//+Y
		"tga/water/sky/yneg.bmp",//-Y
	};
	_texCubeMap = glk::GLCubeMap::createWithFiles(fileList);
	//初始化Shader Buffer对象
	initShaderBuffer();
	initWaterParam();
	//触屏事件
	_touchEventListener = glk::TouchEventListener::createTouchListener(this, glk_touch_selector(ComputeWater::onTouchBegan),
												glk_move_selector(ComputeWater::onTouchMoved),glk_release_selector(ComputeWater::onTouchEnded));
	glk::EventManager::getInstance()->addTouchEventListener(_touchEventListener,0);
	//键盘事件
	_keyEventListener = glk::KeyEventListener::createKeyEventListener(this, glk_key_press_selector(ComputeWater::onKeyPressed),glk_key_release_selector(ComputeWater::onKeyReleased));
	glk::EventManager::getInstance()->addKeyEventListener(_keyEventListener,0);
}

void ComputeWater::initShaderBuffer()
{
	/*
	  *高度场buffer
	 */
	glGenBuffers(1, &_heightFieldBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _heightFieldBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float)*_meshSize*_meshSize,nullptr,GL_DYNAMIC_DRAW);
	/*
	  *速率场buffer
	 */
	glGenBuffers(1, &_velocityFieldBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _velocityFieldBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float)*_meshSize*_meshSize,nullptr,GL_DYNAMIC_DRAW);
	/*
	  *输出的高度
	 */
	glGenBuffers(1, &_outHeightFieldBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _outHeightFieldBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float)*_meshSize*_meshSize, nullptr, GL_DYNAMIC_DRAW);
	/*
	  *法线
	*/
	glGenBuffers(1, &_normalFieldBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _normalFieldBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*_meshSize*_meshSize*3,nullptr,GL_DYNAMIC_DRAW);
}

void ComputeWater::initWaterParam()
{
	_mMatrix.identity();
	_mMatrix.rotateX(-90.0f);
	_mMatrix.translate(0.0f,  0.0f,-_meshSize);
	_waterColor=glk::GLVector4(0.7f, 0.78f, 0.91f, 1.0f);//海水的颜色
	_freshnelParam = glk::GLVector3(0.12f, 0.88f, 2.0f);
}

void ComputeWater::updateWaterComputeShader()
{
	_computeWaterShader->perform();
	//buffer base
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _heightFieldBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _velocityFieldBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _outHeightFieldBuffer);
	//设置uniform参数
	_computeWaterShader->setWaterMeshSize(_meshSize);
	_computeWaterShader->setWaterResolutionHalf(glk::GLVector2(_meshSize,_meshSize));
	//暂时不设置任何的扰动参数
	_computeWaterShader->setWaterParam(_waterParam);
	//dispatch
	_computeWaterShader->dispatch(126, 126);
}

void  ComputeWater::updateWaterNormal()
{
	_computeNormalShader->perform();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _outHeightFieldBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _normalFieldBuffer);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _heightFieldBuffer);
	_computeNormalShader->setMeshUnitSize(_meshUnitSize);
	_computeNormalShader->dispatch(127, 127);
}

void ComputeWater::drawWater()
{
	_renderShader->perform();
	_renderShader->setViewProjMatrix( _camera->getViewProjMatrix());
	_renderShader->setModelMatrix(_mMatrix);
	_renderShader->setCameraPosition(_camera->getCameraPosition());
	_renderShader->setTexCubeMap(_texCubeMap->name(), 0);
	_renderShader->setWaterColor(_waterColor);
	_renderShader->setFreshnelPatram(_freshnelParam);
	//
	_waterMesh->bindVertexObject(0);
	if (_renderShader->getNormalLoc() >= 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, _normalFieldBuffer);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	_waterMesh->drawShape();
	//将系数还原
	_waterParam.w = 0.0f;
}

void ComputeWater::draw()
{
	drawWater();
}

void ComputeWater::update(float deltaTime)
{
	_deltaTime += deltaTime;
	/*
	  *计算摄像机的移动旋转
	  *
	*/
	if (_keyMask & _KEY_MASK_W_)
	{
		_camera->updateTranslateMatrix(0.0f, -15.0f * deltaTime);
	}
	if (_keyMask & _kEY_MASK_S_)
	{
		_camera->updateTranslateMatrix(0.0f, 15.0f*deltaTime);
	}
	if (_keyMask & _KEY_MASK_A_)
	{
		_camera->updateTranslateMatrix(-15.0f*deltaTime, 0.0f);
	}
	if (_keyMask & _KEY_MASK_D_)
	{
		_camera->updateTranslateMatrix(15.0f*deltaTime, 0.0f);
	}
	//计算水面的高度场
	updateWaterComputeShader();
	//计算法线
	updateWaterNormal();
	int t = _heightFieldBuffer;
	_heightFieldBuffer = _outHeightFieldBuffer;
	_outHeightFieldBuffer = t;
}

bool ComputeWater::onTouchBegan(const glk::GLVector2 *touchPoint)
{
	_offsetVec = *touchPoint;
	onTouchWaterWave(touchPoint);
	return true;
}

void ComputeWater::onTouchMoved(const glk::GLVector2 *touchPoint)
{
	onTouchWaterWave(touchPoint);
}

void ComputeWater::onTouchEnded(const glk::GLVector2 *touchPoint)
{

}

void ComputeWater::onTouchWaterWave(const glk::GLVector2 *touchPoint)
{
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	float dx = touchPoint->x / winSize.width * 2.0f - 1.0f;
	float dy = touchPoint->y / winSize.height * 2.0f - 1.0f;
	//还原成OpenGL世界坐标
	glk::GLVector4 OpenGLCoord = glk::GLVector4(dx, dy, 0.0f, 1.0f) * _camera->getInverseViewProjMatrix();
	OpenGLCoord = OpenGLCoord / OpenGLCoord.w;
	//摄像机的位置
	auto &cameraPosition = _camera->getCameraPosition();
	//水面的法线
	const glk::GLVector3		normal(0.0f, 1.0f, 0.0f);
	//从眼睛向目标点的射线
	const glk::GLVector3    ray = (OpenGLCoord.xyz() - cameraPosition).normalize();
	//平面方程使用 Ax+By+Cz - D = 0表示,求出水平面离远点的距离D
	float  D = normal.dot(glk::GLVector3(0.0f, 0.0f, 0.0f));
	//目标点鱼视点的连线与水平线所成的夹角的正弦值
	float  sinValue = -normal.dot(ray);
	//此时包括了两重含义,1视线不与水面平行,2视点不与目标点重合
	if (sinValue != 0.0f)
	{
		//目标点与水平面之间的距离
		float  distance = normal.dot(cameraPosition) - D;
		//斜边的长度
		float  slopDistance = distance / sinValue;
		//射线最终指向的目标点
		glk::GLVector3 targetPosition = cameraPosition + ray * slopDistance;
		//判断目标点是否经过了水面
		if (targetPosition.x >= -_meshSize && targetPosition.x <= _meshSize && targetPosition.z >= -_meshSize*2.0f && targetPosition.z <= 0)
		{
			_waterParam.x = (-targetPosition.z * 0.5f);//网格的纵向
			_waterParam.y = (targetPosition.x + _meshSize) * 0.5f;//网格的横向
			_waterParam.z = 8.0f;//扩散半径
			_waterParam.w = 1.0f *rand()/RAND_MAX ;//扩散强度系数
		}
	}
}

bool ComputeWater::onKeyPressed(const glk::KeyCodeType keyCode)
{
	if (keyCode == glk::KeyCodeType::KeyCode_W)
		_keyMask |= _KEY_MASK_W_;
	else if (keyCode == glk::KeyCodeType::KeyCode_S)
		_keyMask |= _kEY_MASK_S_;
	else if (keyCode == glk::KeyCodeType::KeyCode_A)
		_keyMask |= _KEY_MASK_A_;
	else if (keyCode == glk::KeyCodeType::KeyCode_D)
		_keyMask |= _KEY_MASK_D_;
	return true;
}

void ComputeWater::onKeyReleased(const glk::KeyCodeType keyCode)
{
	if (keyCode == glk::KeyCodeType::KeyCode_W)
		_keyMask &= ~_KEY_MASK_W_;
	else if (keyCode == glk::KeyCodeType::KeyCode_S)
		_keyMask &= ~_kEY_MASK_S_;
	else if (keyCode == glk::KeyCodeType::KeyCode_A)
		_keyMask &= ~_KEY_MASK_A_;
	else if (keyCode == glk::KeyCodeType::KeyCode_D)
		_keyMask &= ~_KEY_MASK_D_;
}