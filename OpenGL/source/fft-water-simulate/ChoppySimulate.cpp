/*
  *ChoppySimulate
  *2018年2月2日
  *@Author:xiaohuaxiong
*/
#include "GL/glew.h"
#include "ChoppySimulate.h"
#include "engine/FFT.h"
#include "engine/GLCacheManager.h"
#include "engine/GLContext.h"
#include "engine/Math.h"

#include "engine/event/EventManager.h"
//世界的范围
#define _WORLD_SIZE_  128
#define _KEY_MASK_W_   0x01
#define _KEY_MASK_S_     0x02
#define _KEY_MASK_A_    0x04
#define _KEY_MASK_D_    0x08
//phillips系数
const float phillips_coefficient = 0.0008f;
//风的方向
const float windSpeed[2] = {40,20};
//高度的缩放系数
const float height_scale = 0.1f;
using namespace glk;
ChoppySimulate::ChoppySimulate() :
	_choppyProgram(nullptr)
	, _skyboxProgram(nullptr)
	, _camera(nullptr)
	, _normalCubeMap(nullptr)
	, _envCubeMap(nullptr)
	, _freshnelParam(0.6f, 0.5f, 0.7f)
	, _lightPosition(-600, 600, 194)
	//,_eyePosition(127, 130, -135)
	, _touchListener(nullptr)
	, _keyListener(nullptr)
	, _time(0)
	, _keyMask(0)
{

}

ChoppySimulate::~ChoppySimulate()
{
	_skyboxProgram->release();
}

void ChoppySimulate::initChoppySimulate()
{
	_choppyProgram = GLProgram::createWithFile("shader/choppy/Choppy_VS.glsl", "shader/choppy/Choppy_FS.glsl");
	//_transform
	_transform.rotate(-90,1,0,0);
	_transform.translate(0, 0, -_CHOPPY_MESH_SIZE_ / 2.0f);
	_transform.trunk(_normalMatrix);
	//_transform.translate(0,0,-_WORLD_SIZE_/2.0f);
	//
	_skyboxProgram = GLProgram::createWithFile("shader/choppy/Skybox_VS.glsl", "shader/choppy/Skybox_FS.glsl");
	_skyboxTextures[0] = GLTexture::createWithFile("tga/skybox/xpos.bmp");//jajlands1_right
	_skyboxTextures[1] = GLTexture::createWithFile("tga/skybox/xneg.bmp");//jajlands1_left
	_skyboxTextures[2] = GLTexture::createWithFile("tga/skybox/ypos.bmp");//jajlands1_top
	_skyboxTextures[3] = GLTexture::createWithFile("tga/skybox/yneg.bmp");//jajlands1_bottom
	_skyboxTextures[4] = GLTexture::createWithFile("tga/skybox/zpos.bmp");//jajlands1_back
	_skyboxTextures[5] = GLTexture::createWithFile("tga/skybox/zneg.bmp");//jajlands1_front
	//天空盒的六个面对应的六个矩阵变换
	_skyboxTransform[0].rotate(-90,0,1,0);
	_skyboxTransform[0].translate(1, 0, 0);//+X

	_skyboxTransform[1].rotate(90,0,1,0);
	_skyboxTransform[1].translate(-1, 0, 0);//-X

	_skyboxTransform[2].rotate(90,1,0,0);//+Y
	_skyboxTransform[2].translate(0, 1, 0);//+Y

	_skyboxTransform[3].rotate(-90,1,0,0);//-Y
	_skyboxTransform[3].translate(0,-1,0);

	_skyboxTransform[4].rotate(180,0,1,0);
	_skyboxTransform[4].translate(0, 0, 1);

	_skyboxTransform[5].translate(0, 0, -1);
	//环境贴图
	const char *filename[6] = {
		"tga/skybox/ixpos.bmp",
		"tga/skybox/ixneg.bmp",
		"tga/skybox/iypos.bmp",
		"tga/skybox/iyneg.bmp",
		"tga/skybox/izpos.bmp",
		"tga/skybox/izneg.bmp",
	};
	_envCubeMap = GLCubeMap::createWithFiles(filename);
	//折射贴图
	const char *bottomFilenames[6] = {
		"tga/skybox/bottom.bmp",
		"tga/skybox/bottom.bmp",
		"tga/skybox/bottom.bmp",
		"tga/skybox/bottom.bmp",
		"tga/skybox/bottom.bmp",
		"tga/skybox/bottom.bmp",
	};
	_bottomCubeMap = GLCubeMap::createWithFiles(bottomFilenames);
	//Camera
	auto &winSize = GLContext::getInstance()->getWinSize();
	_camera = Camera2::createWithPerspective(60, winSize.width/winSize.height,0.5f,500.0f);
	_camera->lookAt(Vec3(0,32,14),Vec3(0,0,-32));
	//event
	_touchListener = TouchEventListener::createTouchListener(this, 
		glk_touch_selector(ChoppySimulate::onTouchBegan),
		glk_move_selector(ChoppySimulate::onTouchMoved),
		glk_release_selector(ChoppySimulate::onTouchReleased)
		);
	EventManager::getInstance()->addTouchEventListener(_touchListener, 0);

	_keyListener = KeyEventListener::createKeyEventListener(this, 
		glk_key_press_selector(ChoppySimulate::onKeyPressed),
		glk_key_release_selector(ChoppySimulate::onKeyReleased)
		);
	EventManager::getInstance()->addKeyEventListener(_keyListener, 0);

	initHeightField();
	_time = 1.0f*rand()/RAND_MAX;
	//生成索引缓冲区对象
	initBuffer();
}

void ChoppySimulate::initBuffer()
{
	int *indexdata = new int[(_CHOPPY_MESH_SIZE_ - 1)*(_CHOPPY_MESH_SIZE_ - 1) * 6];
	int index = 0;
	for (int i = 0; i < _CHOPPY_MESH_SIZE_ - 1; ++i)
	{
		for (int j = 0; j < _CHOPPY_MESH_SIZE_ - 1; ++j)
		{
			indexdata[index] = (i + 1)*_CHOPPY_MESH_SIZE_ + j;
			indexdata[index + 1] = i*_CHOPPY_MESH_SIZE_ + j;
			indexdata[index + 2] = (i + 1)*_CHOPPY_MESH_SIZE_ + j + 1;

			indexdata[index + 3] = (i + 1)*_CHOPPY_MESH_SIZE_ + j + 1;
			indexdata[index + 4] = i*_CHOPPY_MESH_SIZE_ + j;
			indexdata[index + 5] = i*_CHOPPY_MESH_SIZE_ + j + 1;

			index += 6;
		}
	}
	glGenBuffers(1, &_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 6 * (_CHOPPY_MESH_SIZE_ - 1)*(_CHOPPY_MESH_SIZE_ - 1), indexdata, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	delete[] indexdata;
	//
	//纹理坐标
	float *fragCoord = new float[_CHOPPY_MESH_SIZE_*_CHOPPY_MESH_SIZE_*2];
	//法线
	float *normal = new float[_CHOPPY_MESH_SIZE_*_CHOPPY_MESH_SIZE_*3];
	int index1 = 0, index2 = 0;
	float stepWidth = _CHOPPY_MESH_SIZE_ - 1;
	float scale = 1.0f;
	for (int i = 0; i < _CHOPPY_MESH_SIZE_; ++i)
	{
		for (int j = 0; j < _CHOPPY_MESH_SIZE_; ++j)
		{
			_positions[i][j] = Vec3(
				1.0f*i / stepWidth * _WORLD_SIZE_ - _WORLD_SIZE_ / 2.0f,
				1.0f*j / stepWidth * _WORLD_SIZE_ - _WORLD_SIZE_ / 2.0f,
				0.0f
			);
			//法线
			normal[index1] = 0;
			normal[index1 + 1] = 0;
			normal[index1 + 2] = 1;
			//纹理坐标
			fragCoord[index2] = scale * i / stepWidth;
			fragCoord[index2] = scale*j / stepWidth;

			index1 += 3;
			index2 += 2;
		}
	}
	//生成纹理缓冲区对象
	glGenBuffers(1, &_fragCoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER,_fragCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2* _CHOPPY_MESH_SIZE_*_CHOPPY_MESH_SIZE_,fragCoord,GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	delete[] fragCoord;
	//生成法线纹理
	glGenTextures(1, &_normalTexture);
	glBindTexture(GL_TEXTURE_2D,_normalTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB16F, _CHOPPY_MESH_SIZE_, _CHOPPY_MESH_SIZE_,0,GL_RGB,GL_FLOAT,normal);
	glBindTexture(GL_TEXTURE_2D,0);
	delete[] normal;
}

void ChoppySimulate::initHeightField()
{
	float center = _CHOPPY_MESH_SIZE_ / 2.0f;
	float random[2],k[2];
	float coefficient = 1.0f / sqrtf(2);
	for (int i = 0; i < _CHOPPY_MESH_SIZE_; ++i)
	{
		for (int j = 0; j < _CHOPPY_MESH_SIZE_; ++j)
		{
			k[0] = 2.0f*MATH_PI*(i-center)/_WORLD_SIZE_;
			k[1] = 2.0f*MATH_PI*(j - center) / _WORLD_SIZE_;
			float kpower2 = k[0]*k[0] + k[1]*k[1];
			float ksqrt = sqrtf(kpower2);
			_lambad[i][j] = Vec4(k[0],k[1],ksqrt, kpower2);
			gauss(random);
			float root = phillips(phillips_coefficient,k, windSpeed);
			float sqrtr = sqrtf(root);
			_basicHeightField[i][j] = Complex(coefficient*random[0]*sqrtr,coefficient*random[1]*sqrtr);
		}
	}
}

void ChoppySimulate::update(float dt)
{
	_time += dt;
	//Camera
	if (_keyMask)
	{
		Vec3  stepVec;
		float   speed = 1.0f;
		if (_keyMask & _KEY_MASK_W_)
			stepVec += _camera->getForwardVector()*speed;
		if (_keyMask & _KEY_MASK_S_)
			stepVec -= _camera->getForwardVector()*speed;
		if (_keyMask & _KEY_MASK_A_)
			stepVec -= _camera->getXVector()*speed;
		if (_keyMask & _KEY_MASK_D_)
			stepVec += _camera->getXVector()*speed;
		if (stepVec.x*stepVec.x + stepVec.y*stepVec.y + stepVec.z*stepVec.z != 0)
			_camera->translate(stepVec);
	}
	int  half = _CHOPPY_MESH_SIZE_ / 2;
	for (int i = 0; i < half; ++i)
	{
		for (int j = 0; j < _CHOPPY_MESH_SIZE_; ++j)
		{
			float rootFactor = sqrtf(_lambad[i][j].z * GLK_GRAVITY_CONSTANT)*_time;
			float sinValue = sinf(rootFactor);
			float cosValue = cosf(rootFactor);

			auto &result = _heightField[i][j];
			result.real = _basicHeightField[i][j].real*cosValue
				+ _basicHeightField[i][j].imag*sinValue
				+ _basicHeightField[_CHOPPY_MESH_SIZE_ - i - 1][_CHOPPY_MESH_SIZE_ - j - 1].real*cosValue
				- _basicHeightField[_CHOPPY_MESH_SIZE_ - i - 1][_CHOPPY_MESH_SIZE_ - j - 1].imag*sinValue;

			result.imag = _basicHeightField[i][j].imag*cosValue
				+ _basicHeightField[i][j].real*sinValue
				- _basicHeightField[_CHOPPY_MESH_SIZE_ - i - 1][_CHOPPY_MESH_SIZE_ - j - 1].imag*cosValue
				- _basicHeightField[_CHOPPY_MESH_SIZE_ - i - 1][_CHOPPY_MESH_SIZE_ - j - 1].real*sinValue;
			//镜像位置的值
			auto &relativeResult = _heightField[_CHOPPY_MESH_SIZE_-i-1][_CHOPPY_MESH_SIZE_-j-1];
			relativeResult.real = _basicHeightField[i][j].real*cosValue
				+ _basicHeightField[i][j].imag*sinValue
				+ _basicHeightField[_CHOPPY_MESH_SIZE_ - i - 1][_CHOPPY_MESH_SIZE_ - j - 1].real*cosValue
				- _basicHeightField[_CHOPPY_MESH_SIZE_ - i - 1][_CHOPPY_MESH_SIZE_ - j - 1].imag*sinValue;

			relativeResult.imag = _basicHeightField[i][j].imag*cosValue
				+ _basicHeightField[i][j].real*sinValue
				- _basicHeightField[_CHOPPY_MESH_SIZE_ - i - 1][_CHOPPY_MESH_SIZE_ - j - 1].imag*cosValue
				- _basicHeightField[_CHOPPY_MESH_SIZE_ - i - 1][_CHOPPY_MESH_SIZE_ - j - 1].real*sinValue;
		}
	}
	//变换偏移场
	updateOffsetField();
	//对第二阶段生成的高度场进行FFT变换
	FFT2D(_heightField, _CHOPPY_MESH_SIZE_, _CHOPPY_MESH_SIZE_,-1);
	//调整
	for (int i = 0; i < _CHOPPY_MESH_SIZE_; ++i)
	{
		for (int j = 0; j < _CHOPPY_MESH_SIZE_; ++j)
		{
			_heightField[i][j].real *= __SIGNFloat(i+j);
		}
	}
	//计算法线
	updateNormals();
	//计算位置
	//将网格置于坐标系的中央
	float center = _WORLD_SIZE_ / 2.0f;
	float stepWidth = _CHOPPY_MESH_SIZE_ - 1;
	for (int i = 0; i < _CHOPPY_MESH_SIZE_; ++i)
	{
		for (int j = 0; j < _CHOPPY_MESH_SIZE_; ++j)
		{
			_positions[i][j] = Vec3(
				1.0f*i / stepWidth*_WORLD_SIZE_ - center + _offsetXField[i][j].imag,
				1.0f*j / stepWidth*_WORLD_SIZE_ - center + _offsetYField[i][j].imag,
				_heightField[i][j].real*height_scale
			);
		}
	}
}
//计算偏移场变化
void ChoppySimulate::updateOffsetField()
{
	for (int i = 0; i < _CHOPPY_MESH_SIZE_; ++i)
	{
		for (int j = 0; j < _CHOPPY_MESH_SIZE_; ++j)
		{
			float k = _lambad[i][j].z;
			if (k != 0)
			{
				_offsetXField[i][j].real = 0;
				_offsetXField[i][j].imag = -_heightField[i][j].imag *_lambad[i][j].x / k;

				_offsetYField[i][j].real = 0;
				_offsetYField[i][j].imag = -_heightField[i][j].imag*_lambad[i][j].y / k;
			}
			else
			{
				_offsetXField[i][j].real = 0;
				_offsetXField[i][j].imag = 0;

				_offsetYField[i][j].real = 0;
				_offsetYField[i][j].imag = 0;
			}
		}
	}
	//计算X方向的偏移场FFT
	FFT2D(_offsetXField,_CHOPPY_MESH_SIZE_,_CHOPPY_MESH_SIZE_,-1);
	//计算Y方向的偏移场FFT
	FFT2D(_offsetYField,_CHOPPY_MESH_SIZE_,_CHOPPY_MESH_SIZE_,-1);
	//调整
	for (int i = 0; i < _CHOPPY_MESH_SIZE_; ++i)
	{
		for (int j = 0; j < _CHOPPY_MESH_SIZE_; ++j)
		{
			float symbol = __SIGNFloat(i+j)*0.29f;

			_offsetXField[i][j].real *= symbol;
			_offsetXField[i][j].imag *= symbol;

			_offsetYField[i][j].real *= symbol;
			_offsetYField[i][j].imag *= symbol;
		}
	}
}

void ChoppySimulate::updateNormals()
{
	float xl = 1.0f*_WORLD_SIZE_ / _CHOPPY_MESH_SIZE_;
	float yl = 1.0f*_WORLD_SIZE_ / _CHOPPY_MESH_SIZE_;
	for (int i = 0; i < _CHOPPY_MESH_SIZE_-1; ++i)
	{
		for (int j = 0; j < _CHOPPY_MESH_SIZE_-1; ++j)
		{
			float targetHeight = _heightField[i][j].real;
			//X方向上的偏导
			Vec3 dx(xl,0,(_heightField[i+1][j].real - targetHeight)*height_scale);
			//Y方向上的偏导
			Vec3 dy(0,yl,(_heightField[i][j+1].real- targetHeight)*height_scale);

			_normals[i][j] = dx.cross(dy).normalize();
		}
	}
	//将剩余的地方补充完整
	for (int k = 0; k < _CHOPPY_MESH_SIZE_-1; ++k)
	{
		_normals[k][_CHOPPY_MESH_SIZE_ - 1] = _normals[k][0];
		_normals[_CHOPPY_MESH_SIZE_ - 1][k] = _normals[0][k];
	}
	_normals[_CHOPPY_MESH_SIZE_ - 1][_CHOPPY_MESH_SIZE_ - 1] = _normals[0][_CHOPPY_MESH_SIZE_-1];
	//将法线写入到纹理缓冲区中
	glBindTexture(GL_TEXTURE_2D,_normalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _CHOPPY_MESH_SIZE_, _CHOPPY_MESH_SIZE_, 0, GL_RGB, GL_FLOAT, _normals);
}

void ChoppySimulate::render()
{
	renderSkybox();
	//渲染海浪
	_choppyProgram->perform();
	int modelMatrixLoc = _choppyProgram->getUniformLocation("g_ModelMatrix");
	int viewProjMatrixLoc = _choppyProgram->getUniformLocation("g_ViewProjMatrix");
	int normalMatrixLoc = _choppyProgram->getUniformLocation("g_NormalMatrix");
	int freshnelParamLoc = _choppyProgram->getUniformLocation("g_FreshnelParam");
	int lightPositionLoc = _choppyProgram->getUniformLocation("g_LightPosition");
	int eyePositionLoc = _choppyProgram->getUniformLocation("g_EyePosition");
	int ratioLoc = _choppyProgram->getUniformLocation("g_Ratio");
	int textureLoc = _choppyProgram->getUniformLocation("g_Texture");
	int envCubeMapLoc = _choppyProgram->getUniformLocation("g_EnvCubeMap");
	int bottomCubeMapLoc = _choppyProgram->getUniformLocation("g_BottomCubeMap");
	//加载Uniform数据
	glUniformMatrix4fv(modelMatrixLoc,1,GL_FALSE,_transform.pointer());
	glUniformMatrix4fv(viewProjMatrixLoc, 1, GL_FALSE, _camera->getViewProjMatrix().pointer());
	glUniformMatrix3fv(normalMatrixLoc,1,GL_FALSE,_normalMatrix.pointer());
	glUniform3fv(freshnelParamLoc, 1, &_freshnelParam.x);
	glUniform3fv(lightPositionLoc, 1, &_lightPosition.x);
	glUniform3fv(eyePositionLoc,1,&_camera->getEyePosition().x); 
	glUniform1f(ratioLoc, 1.0f / 1.33333f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _envCubeMap->getName());
	glUniform1i(envCubeMapLoc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _bottomCubeMap->getName());
	glUniform1i(bottomCubeMapLoc, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,_normalTexture);
	glUniform1i(textureLoc,2);

	int normalLoc = _choppyProgram->getAttribLocation("a_normal");
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,_positions);

	if (normalLoc >= 0)
	{
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, _normals);
	}
	glBindBuffer(GL_ARRAY_BUFFER,_fragCoordBuffer);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,0,nullptr);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_indexBuffer);
	glDrawElements(GL_TRIANGLES,(_CHOPPY_MESH_SIZE_-1)*(_CHOPPY_MESH_SIZE_-1)*6,GL_UNSIGNED_INT,nullptr);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

void ChoppySimulate::renderSkybox()
{
	int identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	_skyboxProgram->perform();
	int modelMatrixLoc = _skyboxProgram->getUniformLocation("g_ModelMatrix");
	int viewProjMatrixLoc = _skyboxProgram->getUniformLocation("g_ViewProjMatrix");
	int cameraPositionLoc = _skyboxProgram->getUniformLocation("g_CameraPosition");
	int skyboxMapLoc = _skyboxProgram->getUniformLocation("g_SkyboxMap");
	//
	glBindBuffer(GL_ARRAY_BUFFER,identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)(sizeof(float)*3));

	glUniform3fv(cameraPositionLoc,1,&_camera->getEyePosition().x);
	glUniformMatrix4fv(viewProjMatrixLoc,1,GL_FALSE,_camera->getViewProjMatrix().pointer());
	for (int k = 0; k < 6; ++k)
	{
		glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, _skyboxTransform[k].pointer());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,_skyboxTextures[k]->getName());
		glUniform1i(skyboxMapLoc,0);
		glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	}
}
//event
bool ChoppySimulate::onTouchBegan(const Vec2 &touchPoint)
{
	_touchOffset = touchPoint;
	return true;
}

void ChoppySimulate::onTouchMoved(const Vec2 &touchPoint)
{
	_camera->rotate(-(touchPoint.x-_touchOffset.x)*0.5f,(touchPoint.y-_touchOffset.y)*0.5f);
	_touchOffset = touchPoint;
}

void ChoppySimulate::onTouchReleased(const Vec2 &touchPoint)
{

}

bool ChoppySimulate::onKeyPressed(KeyCodeType keyCode)
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

void ChoppySimulate::onKeyReleased(KeyCodeType keyCode)
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