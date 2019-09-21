/*
  *大型地形的渲染
  *@date:2017-6-8
  *@Author:xiaohuaxiong
  *@Version 1.0:实现了最基本的地形渲染
 */
#include "GL/glew.h"
#include "engine/GLContext.h"
#include "engine/event/EventManager.h"
#include "engine/GLContext.h"
#include "Terrain.h"
#include<stdio.h>
#include<assert.h>
//W Key Code
#define _KEY_MASK_W 0x1
//S Key COde
#define _KEY_MASK_S 0x02
//A Key Code
#define _KEY_MASK_A 0x04
//D Key Code
#define _KEY_MASK_D 0x08
//所有按键的掩码集合
#define _KEY_MASK_ALL 0x0F

Terrain::Terrain()
{
	_terrainShader = nullptr;
	_keyListener = nullptr;
	_touchListener = nullptr;
	_keyMask = 0;
	//
	_terrainVertexId = 0;
	_terrainIndexId = 0;
	//
	_heightField = nullptr;
	_terrainSize = 0;
}

Terrain::~Terrain()
{
	delete _terrainShader;
	_terrainShader = nullptr;
	glDeleteBuffers(1, &_terrainVertexId);
	glDeleteBuffers(1, &_terrainIndexId);
	_terrainVertexId = 0;
	_terrainIndexId = 0;
	glk::EventManager::getInstance()->removeListener(_touchListener);
	_touchListener->release();
	_touchListener = nullptr;
}

Terrain  *Terrain::createTerrainWithFile(const std::string &filename)
{
	Terrain  *terrain = new Terrain();
	terrain->initWithFile(filename);
	return terrain;
}

Terrain *Terrain::createTerrainWithTexture(glk::GLTexture *hightTexture)
{
	return nullptr;
}

void Terrain::initWithFile(const std::string &filename)
{
	//打开并测试文件
	FILE   *fp = nullptr;
	const int errorCode = fopen_s(&fp, filename.c_str(), "rb");
	if (errorCode)
	{
		printf("Open binary terrain file '%s' error!\n",filename.c_str());
		assert(!errorCode);
	}
	//检测文件内容
	if (fread(&_terrainSize, sizeof(int), 1, fp) != 1 || _terrainSize<=0 )
	{
		printf("Binary file '%s' has broken format!\n",filename.c_str());
		fclose(fp);
		fp = nullptr;
		assert(0);
	}
	const int sizePlusOne = _terrainSize + 1;
	const float halfWidth = _terrainSize/2.0f;
	const int    totalSize = sizePlusOne * sizePlusOne;
	_heightField = new float[totalSize];
	if (fread(_heightField, sizeof(float), totalSize,fp) != totalSize)
	{
		fclose(fp);
		fp = nullptr;
		printf("Binary file '%s' lose data.\n",filename.c_str());
		assert(0);
	}
	fclose(fp);
	fp = nullptr;
	//计算最大/最小高程值
	const float d2d = _terrainSize / 2.0f;
	_boundaryMax.x = d2d;
	_boundaryMin.x = -d2d;
	_boundaryMax.z = d2d;
	_boundaryMin.z = -d2d;
	_boundaryMin.y = _boundaryMax.y = _heightField[0];
	for (int i = 1; i < totalSize; ++i)
	{
		if (_boundaryMax.y < _heightField[i])
			_boundaryMax.y = _heightField[i];
		if (_boundaryMin.y > _heightField[i])
			_boundaryMin.y = _heightField[i];
	}
	//计算平面网格
	int index = 0;
	float   *VertexData = new float[totalSize*6];
	int idx = 0;
	for (int z = 0; z < sizePlusOne; ++z)
	{
		for (int x = 0; x < sizePlusOne; ++x)
		{
			//position
			glk::GLVector3  *position = (glk::GLVector3 *)(VertexData+index);
			position->x = x - halfWidth;
			position->y =  _heightField[idx++];// this->getHeightValue(x, z);
			position->z =  halfWidth - z;
			glk::GLVector3 *normal = (glk::GLVector3 *)(VertexData+index+3);
			*normal =  glk::GLVector3(this->getHeightValue(x + 1, z) - this->getHeightValue(x - 1, z), 2.0f, this->getHeightValue(x, z + 1) - this->getHeightValue(x, z - 1)).normalize();
			index += 6;
		}
	}
	//生成顶点缓冲区对象
	glGenBuffers(1, &_terrainVertexId);
	glBindBuffer(GL_ARRAY_BUFFER, _terrainVertexId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*totalSize*6,VertexData,GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	delete VertexData;
	VertexData = nullptr;
	//产生三角形扇序列
	int *indexVertex = new int[_terrainSize*_terrainSize * 6];
	index = 0;
	for (int z = 0; z < _terrainSize; ++z)
	{
		for (int x = 0; x < _terrainSize; ++x)
		{
			//上三角
			indexVertex[index] = (z+1) *sizePlusOne+x;
			indexVertex[index + 1] = z*sizePlusOne+x;
			indexVertex[index + 2] = (z+1)*sizePlusOne + x + 1;
			//下三角
			indexVertex[index + 3] = (z + 1)*sizePlusOne + x + 1;
			indexVertex[index + 4] = z*sizePlusOne + x;
			indexVertex[index + 5] = z*sizePlusOne + x + 1;
			index += 6;
		}
	}
	glGenBuffers(1, &_terrainIndexId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _terrainIndexId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*_terrainSize*_terrainSize * 6,indexVertex,GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	delete indexVertex;
	indexVertex = nullptr;
	/*
	  *Shader
	*/
	_terrainShader = TerrainShader::createTerrainShader("shader/terrain/VS_Terrain.glsl", "shader/terrain/FS_Terrain.glsl");
	/*
	  *摄像机
	 */
	const float height = this->getHeightValueSmooth(0.0f, 0.0f);
	initCamera(glk::GLVector3(0.0f,height+1.75f,0.0f),glk::GLVector3(0.0f, height+1.75f ,-1.0f));
	/*
	  *光线与颜色
	*/
	initLightParam();
	/*
	  *触屏事件监听器注册
	 */
	_touchListener = glk::TouchEventListener::createTouchListener(this, glk_touch_selector(Terrain::onTouchBegan),glk_move_selector(Terrain::onTouchMoved),glk_release_selector(Terrain::onTouchEnded));
	glk::EventManager::getInstance()->addTouchEventListener(_touchListener,0);
	/*
	  *键盘事件注册
	 */
	_keyListener = glk::KeyEventListener::createKeyEventListener(this, glk_key_press_selector(Terrain::onKeyPressed), glk_key_release_selector(Terrain::onKeyReleased));
	glk::EventManager::getInstance()->addKeyEventListener(_keyListener,0);
}
inline float Terrain::getHeightValue(int x, int z)const
{
	const int nx = x < 0 ? 0 : (x>_terrainSize?_terrainSize:x);
	const int nz = z < 0 ? 0 : (z>_terrainSize?_terrainSize:z);
	return _heightField[nz*(_terrainSize+1)+x];
}

inline int Terrain::getRealIndex(int x, int z)const
{
	const int nx = x < 0 ? 0 : (x > _terrainSize ? _terrainSize : x);
	const int nz = z < 0 ? 0 : (z > _terrainSize ? _terrainSize : z);
	return nz *(_terrainSize+1) + nx;
}

float Terrain::getHeightValueSmooth(float x, float z)const
{
	const float halfWidth = _terrainSize / 2.0f;
	//注意Z值的反转
	z = -z;
	x += halfWidth;
	z +=  halfWidth;
	const float nx = x < 0 ? 0:(x>_terrainSize?_terrainSize:x);
	const float nz = z < 0 ? 0 : (z>_terrainSize?_terrainSize:z);

	const int lx = (int)nx;
	const int lz = (int)nz;
//计算中心位置离左下角位置的偏移
	const float fx = nx - lx;
	const float fz = nz - lz;

	float a = this->getHeightValue(lx, lz);
	float b = this->getHeightValue(lx+1,lz);
	float c = this->getHeightValue(lx, lz + 1);
	float d = this->getHeightValue(lx + 1, lz + 1);

	float ab = a + (b - a)*fx;
	float cd = c + (d-c)*fx;
	return ab + (cd-ab)*fz;
}

void   Terrain::initCamera(const glk::GLVector3 &eyePosition, const glk::GLVector3 &targetPosition)
{
	_eyePosition = eyePosition;
	_targetPosition = targetPosition;
	_viewMatrix.identity();
	_viewMatrix.lookAt(eyePosition, targetPosition, glk::GLVector3(0.0f,1.0f,0.0f));
	//投影矩阵
	const glk::Size &winSize = glk::GLContext::getInstance()->getWinSize();
	_projMatrix.identity();
	_projMatrix.perspective(45.0f, winSize.width/winSize.height, 0.1f, 1024.0f);
	//视图投影矩阵
	_viewProjMatrix = _viewMatrix * _projMatrix;
	//分解矩阵
	//const float *array = _viewMatrix.pointer();
	//模型矩阵
	_modelMatrix.identity();
	//以三个坐标轴的形式计算视图矩阵
	_zAxis = (_eyePosition - _targetPosition).normalize();
	_xAxis = glk::GLVector3(0.0f, 1.0f, 0.0f).cross(_zAxis);
	_yAxis = _zAxis.cross(_xAxis);
	buildViewMatrix();
}

void  Terrain::initLightParam()
{
	_lightColor = glk::GLVector4(1.0f,1.0f,1.0f,1.0f);
	_lightDirection = glk::GLVector3(1.0f,1.0f,1.6f).normalize();
	_terrainColor = glk::GLVector4(1.0f,1.0f,1.0f,1.0f);
}

void  Terrain::update(const float deltaTime)
{
	//检查是否有键盘事件
	if (_keyMask & _KEY_MASK_ALL)
	{
		const float speed = 5.0f;
		const float stepDistance = speed * deltaTime;
		const glk::GLVector3 upVec(0.0f,1.0f,0.0f);
		const glk::GLVector3 xVec = _xAxis;
		//前向Z轴
		const glk::GLVector3 zVec = upVec.cross(xVec);
		glk::GLVector3 stepVec;
		//检测各个按键
		if (_keyMask & _KEY_MASK_W)
			stepVec = stepVec + zVec * stepDistance;
		if (_keyMask & _KEY_MASK_S)
			stepVec = stepVec - zVec *stepDistance;
		if (_keyMask & _KEY_MASK_A)
			stepVec = stepVec - xVec * stepDistance;
		if (_keyMask & _KEY_MASK_D)
			stepVec = stepVec + xVec	 * stepDistance;
		//将视点累加
		glk::GLVector3 targetPosition = _targetPosition + stepVec;
		//依据各个方向对目标位置进行截断
		if (targetPosition.x < _boundaryMin.x)
			stepVec.x = _boundaryMin.x - _targetPosition.x;
		if (targetPosition.x > _boundaryMax.x)
			stepVec.x = _boundaryMax.x-_targetPosition.x;
		if(targetPosition.z<_boundaryMin.z)
			stepVec.z = _boundaryMin.z - _targetPosition.z;
		if(targetPosition.z>_boundaryMax.z)
			stepVec.z = _boundaryMax.z - _targetPosition.z;
		//
		targetPosition = _targetPosition + stepVec;
		const float height = getHeightValueSmooth(targetPosition.x, targetPosition.z);
		stepVec = stepVec + glk::GLVector3(0.0f,height+1.75f-_targetPosition.y,0.0f);
		_eyePosition = _eyePosition + stepVec;
		_targetPosition = _targetPosition + stepVec;
		buildViewMatrix();
	}
}

void Terrain::render()
{
	_terrainShader->perform();
	//绑定顶点数组
	const int positionLoc = _terrainShader->getPositionLoc();
	glBindBuffer(GL_ARRAY_BUFFER,_terrainVertexId);
	if (positionLoc >= 0)
	{
		glEnableVertexAttribArray(positionLoc);
		glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, nullptr);
	}
	const int normalLoc = _terrainShader->getNormalLoc();
	if (normalLoc >= 0)
	{
		glEnableVertexAttribArray(normalLoc);
		glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));
	}
	//绑定索引缓冲区对象
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _terrainIndexId);
	//设置Uniform Variable
	_terrainShader->setModelMatrix(_modelMatrix);
	_terrainShader->setNormalMatrix(_modelMatrix.normalMatrix());
	_terrainShader->setViewProjMatrix(_viewProjMatrix);
	//光线,颜色
	_terrainShader->setEyePosition(_eyePosition);
	_terrainShader->setLightColor(_lightColor);
	_terrainShader->setLightDirection(_lightDirection);
	_terrainShader->setTerrainColor(_terrainColor);
	//渲染
	glDrawElements(GL_TRIANGLES,_terrainSize*_terrainSize*6,GL_UNSIGNED_INT,nullptr);
}

bool    Terrain::onTouchBegan(const glk::GLVector2 *touchPoint)
{
	_touchOffset = *touchPoint;
	//接下来计算每一步的偏移
	return true;
}

void    Terrain::onTouchMoved(const glk::GLVector2 *touchPoint)
{
	const glk::GLVector2 	offsetVec =*touchPoint - _touchOffset;
	//绕Y轴旋转
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	glk::GLVector3 Zdirection = _eyePosition - _targetPosition;
	glk::Matrix rotateYMatrix;
	rotateYMatrix.rotateY(-offsetVec.x*__MATH_PI__*_ANGLE_FACTOR_/winSize.width);
	_xAxis = (_xAxis.xyzw0() * rotateYMatrix).xyz();
	_yAxis = (_yAxis.xyzw0()*rotateYMatrix).xyz();
	_zAxis = (_zAxis.xyzw0()*rotateYMatrix).xyz();
	//绕X轴旋转,注意旋转的角度会有所限制,不会让场景处于角色的上方
	if (offsetVec.y != 0.0f)
	{
		float angleX =offsetVec.y * __MATH_PI__ * _ANGLE_FACTOR_ / winSize.height;
		glk::Matrix rotateXMatrix;
		rotateXMatrix.rotate(angleX,_xAxis.x,_xAxis.y,_xAxis.z);
		_yAxis = (_yAxis.xyzw0()*rotateXMatrix).xyz();
		_zAxis = (_zAxis.xyzw0()*rotateXMatrix).xyz();
		//限制
		if (_yAxis.y < 0.0f)
		{
			_zAxis = glk::GLVector3(0.0f,_zAxis.y>0.0f?1.0f:-1.0f,0.0f);
			_yAxis = _zAxis.cross(_xAxis);
		}
	}
	_eyePosition = _targetPosition + _zAxis * Zdirection.length();
	buildViewMatrix();
	_touchOffset = *touchPoint;
}

void   Terrain::onTouchEnded(const glk::GLVector2 *touchPoint)
{
	_touchOffset = *touchPoint;
}

bool  Terrain::onKeyPressed(const glk::KeyCodeType keyCode)
{
	if (keyCode == glk::KeyCodeType::KeyCode_W)
		_keyMask |= _KEY_MASK_W;
	if (keyCode == glk::KeyCodeType::KeyCode_S)
		_keyMask |= _KEY_MASK_S;
	if (keyCode == glk::KeyCodeType::KeyCode_A)
		_keyMask |= _KEY_MASK_A;
	if (keyCode == glk::KeyCodeType::KeyCode_D)
		_keyMask |= _KEY_MASK_D;
	return true;
}

void Terrain::onKeyReleased(const glk::KeyCodeType keyCode)
{
	if (keyCode == glk::KeyCodeType::KeyCode_W)
		_keyMask  &= ~_KEY_MASK_W;
	if (keyCode == glk::KeyCodeType::KeyCode_S)
		_keyMask &= ~ _KEY_MASK_S;
	if (keyCode == glk::KeyCodeType::KeyCode_A)
		_keyMask &= ~_KEY_MASK_A;
	if (keyCode == glk::KeyCodeType::KeyCode_D)
		_keyMask &= ~_KEY_MASK_D;
}
void Terrain::buildViewMatrix()
{
	//合成视图矩阵
	typedef float (*MatrixArray)[4];
	MatrixArray array = (MatrixArray)_viewMatrix.pointer();
	//由三个坐标轴以及移动的偏移量计算视图矩阵
	array[0][0] = _xAxis.x;
	array[1][0] = _xAxis.y;
	array[2][0] = _xAxis.z;

	array[0][1] = _yAxis.x;
	array[1][1] = _yAxis.y;
	array[2][1] = _yAxis.z;

	array[0][2] = _zAxis.x;
	array[1][2] = _zAxis.y;
	array[2][2] = _zAxis.z;

	array[0][3] = 0.0f;
	array[1][3] = 0.0f;
	array[2][3] = 0.0f;

	array[3][0] = -_xAxis.dot(_eyePosition);
	array[3][1] = -_yAxis.dot(_eyePosition);
	array[3][2] = -_zAxis.dot(_eyePosition);

	array[3][3] = 1.0f;
	//计算视图投影矩阵
	_viewProjMatrix.multiply(_viewMatrix, _projMatrix);
}