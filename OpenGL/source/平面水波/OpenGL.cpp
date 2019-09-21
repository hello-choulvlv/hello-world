#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/GLTexture.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<engine/Tools.h>
#include<engine/Sprite.h>
#include<math.h>
#include<string.h>
#include<assert.h>
#include<stdlib.h>
#define        GRID_SIZE                            130
#define        WAVE_SIZE                          (GRID_SIZE-2)
//对水波能量衰减系数
#define        WAVE_DAMPING                0.90f
typedef        float                  (*WaveWeightType)[GRID_SIZE];
typedef        GLVector3       (*WaterMeshType)[GRID_SIZE-2];
void   drawWaterMask(GLContext *_content);

struct        WaveShader
{
	GLProgram    *_waterProgram;
	unsigned         _modelMatrixLoc;
	unsigned         _viewProjMatrixLoc;
	unsigned         _normalMatrixLoc;
	unsigned         _skyTexMapLoc;
	unsigned         _eyePositionLoc;
	unsigned         _freshnelParamLoc;
	unsigned         _refractRatioLoc;
	unsigned         _waterColorLoc;
	unsigned			_skyboxMapLoc;
//程序数据
	WaveShader();
	~WaveShader();
};
WaveShader::WaveShader()
{
	_waterProgram = GLProgram::createWithFile("shader/wave/wave_copy.vsh", "shader/wave/wave_other.fsh");
	_modelMatrixLoc = _waterProgram->getUniformLocation("u_modelMatrix");
	_viewProjMatrixLoc = _waterProgram->getUniformLocation("u_viewProjMatrix");
	_normalMatrixLoc = _waterProgram->getUniformLocation("u_normalMatrix");
	_skyTexMapLoc = _waterProgram->getUniformLocation("u_skyTexCube");
	_freshnelParamLoc = _waterProgram->getUniformLocation("u_freshnelParam");
	_refractRatioLoc = _waterProgram->getUniformLocation("u_refractRatio");
	_eyePositionLoc = _waterProgram->getUniformLocation("u_eyePosition");
	_waterColorLoc = _waterProgram->getUniformLocation("u_waterColor");

	_skyboxMapLoc = _waterProgram->getUniformLocation("u_skyboxMap");
}
WaveShader::~WaveShader()
{
	_waterProgram->release();
}
//水面波纹
struct       UserData
{
	WaveShader    _waterShader;
	GLCubeMap   *_skyboxMap;
	GLTexture		*_skybox;
	unsigned           _waveNormalId;
//纹理
	float                _meshHeight[GRID_SIZE][GRID_SIZE];
	float                _meshOutHeight[GRID_SIZE][GRID_SIZE];
	float                _meshVelocity[GRID_SIZE][GRID_SIZE];
	WaveWeightType    _cycleMesh[2];
	int                   _srcMeshIndex;
	Mesh              *_meshObject;
	float                 _deltaTime;

	Matrix             _modelMatrix;
	Matrix3           _normalMatrix;
	Matrix             _viewProjMatrix;
	GLVector3       _eyePosition;
	GLVector3       _freshnelParam;
	GLVector4       _waterColor;
	float                  _refractRatio;
	char                  *_pixelBuffer;
	unsigned            _texBufferId;
};
//

void        Init(GLContext    *_context)
{ 
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	Size    _size = _context->getWinSize();
//
	memset(_user->_meshHeight, 0, sizeof(_user->_meshHeight));
	memset(_user->_meshVelocity,0,sizeof(_user->_meshVelocity));
	memset(_user->_meshOutHeight,0,sizeof(_user->_meshOutHeight));
	_user->_cycleMesh[0] = _user->_meshHeight;
	_user->_cycleMesh[1] = _user->_meshVelocity;
	_user->_srcMeshIndex = 0;// 0src ,1:dst


	glGenBuffers(1, &_user->_waveNormalId);
	glBindBuffer(GL_ARRAY_BUFFER, _user->_waveNormalId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLVector3)*WAVE_SIZE*WAVE_SIZE , NULL, GL_DYNAMIC_DRAW);
//写入数据
	const     char      *vcubeMapFiles[6] = {
		"tga/water/sky/zpos.bmp",//+Z
		"tga/water/sky/zneg.bmp",//-Z
		"tga/water/sky/xpos.bmp",//+X
		"tga/water/sky/xneg.bmp",//-X
										   "tga/water/sky/ypos.bmp",//+Y
										   "tga/water/sky/yneg.bmp",//-Y
	                                     };
	_user->_skybox = GLTexture::createWithFile("tga/water/sky/ypos.bmp");
	_user->_skyboxMap = GLCubeMap::createWithFiles(vcubeMapFiles);
	_user->_meshObject = Mesh::createWithIntensity(GRID_SIZE - 2, _size.width/2.0f, _size.height/2.0f, 1.0f);
//	_user->_modelMatrix.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
	const   float   zeye = _size.height / 1.1566f;
	_user->_modelMatrix.translate(0.0f, 0.0f, 0.0f);
	_user->_normalMatrix = _user->_modelMatrix.normalMatrix();

	_user->_eyePosition = GLVector3(0.0f,0.0f, zeye);

	_user->_viewProjMatrix.lookAt(_user->_eyePosition, GLVector3(0.0f, 0.0f, 0.0f), GLVector3(0.0f, 1.0f, 0.0f));
	_user->_viewProjMatrix.perspective(60.0f, _size.width / _size.height, 1.0f, zeye+_size.height/2.0f);

	_user->_freshnelParam = GLVector3(0.12f,0.88f,2.0f);
	_user->_refractRatio = 1.0f / 1.33f;
	_user->_waterColor = GLVector4(0.7f, 0.78f, 0.91f, 1.0f);//海水的颜色

	_user->_deltaTime = 0.0f;

//
	//glGenTextures(1, &_user->_texBufferId);
	//glBindTexture(GL_TEXTURE_2D,_user->_texBufferId);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _size.width, _size.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//_user->_pixelBuffer = new char[(int)(_size.width*_size.height)*sizeof(char)*4];
	_user->_pixelBuffer = NULL;
//	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	//
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Sprite    *sprite = Sprite::createWithFile("tga/water/sky/ypos.bmp");
	//Matrix   identityM;
	//sprite->render(&identityM);
	//sprite->release();

	//glReadBuffer(GL_BACK);
	//glReadPixels(0, 0, _size.width, _size.height, GL_RGBA, GL_UNSIGNED_BYTE, _user->_pixelBuffer);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_deltaTime += _deltaTime;
//计算水面的高度,同时将数据写入缓存中
	WaterMeshType    _waterMesh;
	WaveWeightType  _srcMesh = _user->_cycleMesh[_user->_srcMeshIndex];
	WaveWeightType  _dstMesh = _user->_cycleMesh[(_user->_srcMeshIndex+1) & 0x1];
	_user->_srcMeshIndex = (_user->_srcMeshIndex + 1) & 0x1;
	for (int i = 1; i < GRID_SIZE-1; ++i)
	{
		for (int j = 1;j < GRID_SIZE-1; ++j)
		{
//八个方向的数值总和 
			float          _totalValue = //_srcMesh[i - 1][j] + _srcMesh[i][j + 1] + _srcMesh[i + 1][j] + _srcMesh[i][j - 1];
				_srcMesh[i - 1][j - 1] + _srcMesh[i - 1][j] + _srcMesh[i - 1][j + 1] + _srcMesh[i][j - 1] +
				_srcMesh[i][j + 1] + _srcMesh[i + 1][j - 1] + _srcMesh[i + 1][j] + _srcMesh[i + 1][j + 1];
			_dstMesh[i][j] = _totalValue * 0.25f - _dstMesh[i][j];
			_dstMesh[i][j] *= WAVE_DAMPING;
		}
	}
//计算法线,注意,在下面的代码中,不会严格按照物理模型计算水面网格的法线,而是一种近似的求X-Z方向的偏导数
	glBindBuffer(GL_ARRAY_BUFFER, _user->_waveNormalId);
	_waterMesh = (WaterMeshType)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(GLVector3)*WAVE_SIZE*WAVE_SIZE, GL_MAP_WRITE_BIT);
	for (int i = 1; i < GRID_SIZE-1; ++i)
	{
		for (int j = 1; j < GRID_SIZE - 1; ++j)
		{
			_waterMesh[i - 1][j - 1] = GLVector3(_dstMesh[i + 1][j] - _dstMesh[i][j],
				_dstMesh[i][j + 1] - _dstMesh[i][j],
				_dstMesh[i][j]);
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
//	memcpy(_nowHeight,_outHeight,sizeof(_user->_meshOutHeight));
//随机填充数据,模拟雨点
#ifndef __TEST_TEMPLATE__
	if (_user->_deltaTime > 0.3f)
	{
		_user->_deltaTime = 0.0f;
		float     idx = 1.0f*rand()/RAND_MAX* WAVE_SIZE;
		float     idy=1.0f*rand()/RAND_MAX* WAVE_SIZE;
		const    float  radius = 16.0f;
	//	_nowHeight[(int)idx][(int)idy] += -4.0f;
		for (int i = 1; i < GRID_SIZE - 1; ++i)
		{
			for (int j = 1; j < GRID_SIZE - 1; ++j)
			{
				const  float   real_radius = sqrt((idx - i)*(idx - i) + (idy - j)*(idy - j));
				_dstMesh[i][j] -= 128.0f/ (real_radius*real_radius + 1.0);// *exp(-real_radius / radius*1.0f);
			}
		}
	}
#endif
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawWaterMask(_context);

}

void   drawWaterMask(GLContext  *_content)
{
	UserData *_user = (UserData *)_content->userObject;
	Size            _winSize = _content->getWinSize();
//	glBindTexture(GL_TEXTURE_2D, _user->_texBufferId);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _winSize.width, _winSize.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _user->_pixelBuffer);

	WaveShader   *_wave = &_user->_waterShader;
	_wave->_waterProgram->perform();

	_user->_meshObject->bindVertexObject(0);
	_user->_meshObject->bindTexCoordObject(1);

	glBindBuffer(GL_ARRAY_BUFFER, _user->_waveNormalId);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	//uniforms
	glUniformMatrix4fv(_wave->_modelMatrixLoc, 1, GL_FALSE, _user->_modelMatrix.pointer());
	glUniformMatrix4fv(_wave->_viewProjMatrixLoc, 1, GL_FALSE, _user->_viewProjMatrix.pointer());
	glUniformMatrix3fv(_wave->_normalMatrixLoc, 1, GL_FALSE, _user->_normalMatrix.pointer());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->_skybox->name());
	glUniform1i(_wave->_skyboxMapLoc, 0);

	glUniform3fv(_wave->_freshnelParamLoc, 1, &_user->_freshnelParam.x);

	_user->_meshObject->drawShape();
	//
//	glReadBuffer(GL_BACK);
//	glReadPixels(0, 0, _winSize.width, _winSize.height, GL_RGBA, GL_UNSIGNED_BYTE, _user->_pixelBuffer);
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_skyboxMap->release();
	_user->_meshObject->release();
	_user->_skybox->release();
	glDeleteBuffers(1, &_user->_waveNormalId);
	delete _user->_pixelBuffer;
	_user->_pixelBuffer = NULL;
}
///////////////////////////don not modify below function////////////////////////////////////
