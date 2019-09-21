#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/GLTexture.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<math.h>
#include<string.h>
#define        GRID_SIZE                            130
#define        WAVE_SIZE                          (GRID_SIZE-2)
//对水波能量衰减系数
#define        WAVE_DAMPING                0.99f
typedef        float                  (*WaveWeightType)[GRID_SIZE];
typedef        GLVector3       (*WaterMeshType)[GRID_SIZE-2];
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
//程序数据
	WaveShader();
	~WaveShader();
};
WaveShader::WaveShader()
{
	_waterProgram = GLProgram::createWithFile("shader/wave/wave.vsh", "shader/wave/wave.fsh");
	_modelMatrixLoc = _waterProgram->getUniformLocation("u_modelMatrix");
	_viewProjMatrixLoc = _waterProgram->getUniformLocation("u_viewProjMatrix");
	_normalMatrixLoc = _waterProgram->getUniformLocation("u_normalMatrix");
	_skyTexMapLoc = _waterProgram->getUniformLocation("u_skyTexCube");
	_freshnelParamLoc = _waterProgram->getUniformLocation("u_freshnelParam");
	_refractRatioLoc = _waterProgram->getUniformLocation("u_refractRatio");
	_eyePositionLoc = _waterProgram->getUniformLocation("u_eyePosition");
	_waterColorLoc = _waterProgram->getUniformLocation("u_waterColor");
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
	unsigned           _waveNormalId;
//纹理
	float                _meshHeight[GRID_SIZE][GRID_SIZE];
	float                _meshOutHeight[GRID_SIZE][GRID_SIZE];
	float                _meshVelocity[GRID_SIZE][GRID_SIZE];
	Mesh              *_meshObject;
	float                 _deltaTime;

	Matrix             _modelMatrix;
	Matrix3           _normalMatrix;
	Matrix             _viewProjMatrix;
	GLVector3       _eyePosition;
	GLVector3       _freshnelParam;
	GLVector4       _waterColor;
	float                  _refractRatio;
};
//

void        Init(GLContext    *_context)
{ 
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	Size    _size = _context->getWinSize();
	_user->_meshObject = Mesh::createWithIntensity(GRID_SIZE - 2, GRID_SIZE - 2, GRID_SIZE - 2, 1.0f);
//
	memset(_user->_meshHeight, 0, sizeof(_user->_meshHeight));
	memset(_user->_meshVelocity,0,sizeof(_user->_meshVelocity));
	memset(_user->_meshOutHeight,0,sizeof(_user->_meshOutHeight));

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
	_user->_skyboxMap = GLCubeMap::createWithFiles(vcubeMapFiles);
	_user->_modelMatrix.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
	_user->_modelMatrix.translate(0.0f, 0.0f, -WAVE_SIZE);
	_user->_normalMatrix = _user->_modelMatrix.normalMatrix();

	_user->_eyePosition = GLVector3(0.0f, WAVE_SIZE / 1.5f, 0.0f);

	_user->_viewProjMatrix.lookAt(_user->_eyePosition, GLVector3(0.0f, 0.0f, -WAVE_SIZE / 1.2f), GLVector3(0.0f, 1.0f, 0.0f));
	_user->_viewProjMatrix.perspective(60.0f,_size.width/_size.height,1.0f,1000.0f);

	_user->_freshnelParam = GLVector3(0.12f,0.88f,2.0f);
	_user->_refractRatio = 1.0f / 1.33f;
	_user->_waterColor = GLVector4(0.7f, 0.78f, 0.91f, 1.0f);//海水的颜色

	_user->_deltaTime = 0.0f;

	//float     idx = 64.5f;
	//float     idy = 64.5f;
	//WaveWeightType   _nowHeight = _user->_meshHeight;
	//const    float  radius = 16.0f;
	//for (int i = 1; i < GRID_SIZE-1; ++i)
	//{
	//	for (int j = 1; j < GRID_SIZE-1; ++j)
	//	{
	//		const  float   real_radius = sqrt((idx - i)*(idx - i) + (idy - j)*(idy - j));
	//		_nowHeight[i][j] -=  exp(-real_radius / radius*3.0)*12.356f;
	//	}
	//}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_deltaTime += _deltaTime;
//一下是处理水面网格
	WaveWeightType   _nowHeight = _user->_meshHeight;
	WaveWeightType   _nowVelocity = _user->_meshVelocity;
	WaveWeightType   _outHeight = _user->_meshOutHeight;
//计算水面的高度,同时将数据写入缓存中
	glBindBuffer(GL_ARRAY_BUFFER, _user->_meshObject->getVertexBufferId());
	WaterMeshType    _waterMesh = (WaterMeshType)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(GLVector3)*WAVE_SIZE*WAVE_SIZE, GL_MAP_WRITE_BIT);
	for (int i = 1; i < GRID_SIZE-1; ++i)
	{
		for (int j = 1;j < GRID_SIZE-1; ++j)
		{
//八个方向的数值总和 
			float          _totalValue =  //_nowHeight[i - 1][j]  + _nowHeight[i][j - 1] +_nowHeight[i][j + 1]  + _nowHeight[i + 1][j] ;
			 _nowHeight[i - 1][j - 1] + _nowHeight[i - 1][j] + _nowHeight[i - 1][j + 1] + _nowHeight[i][j - 1] +
				_nowHeight[i][j + 1] + _nowHeight[i + 1][j - 1] + _nowHeight[i + 1][j] + _nowHeight[i + 1][j + 1];
//注意下面的代码是计算的速率,注意,能量一定要保持平衡,否则不能保持波平面的和谐
			_nowVelocity[i][j] += _totalValue * 0.125  - _nowHeight[i][j];
			_nowVelocity[i][j] *= WAVE_DAMPING;
			_outHeight[i][j] = _nowHeight[i][j] + _nowVelocity[i][j];
		}
	}
//计算实际的高度场
	for (int i = 1; i < GRID_SIZE - 1; ++i)
	{
		for (int j = 1; j < GRID_SIZE - 1; ++j)
		{
			//const   float    velocity = _nowVelocity[i][j] * WAVE_DAMPING;
			//_nowVelocity[i][j] = velocity;
			//_nowHeight[i][j] += _deltaTime*velocity;
			_waterMesh[i - 1][j - 1].z = _outHeight[i][j];// _nowHeight[i][j];
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
//	memcpy(_nowHeight, _nowVelocity, sizeof(_user->_meshHeight));
//计算法线,注意,在下面的代码中,不会严格按照物理模型计算水面网格的法线,而是一种近似的求X-Z方向的偏导数
	glBindBuffer(GL_ARRAY_BUFFER, _user->_waveNormalId);
	_waterMesh = (WaterMeshType)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(GLVector3)*WAVE_SIZE*WAVE_SIZE, GL_MAP_WRITE_BIT);
	for (int i = 1; i < GRID_SIZE-1; ++i)
	{
		for (int j = 1; j < GRID_SIZE - 1; ++j)
		{
			_waterMesh[i - 1][j - 1] = GLVector3(_outHeight[i + 1][j] - _outHeight[i][j],
				_outHeight[i][j + 1] - _outHeight[i][j],
				1.0f    	);
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	memcpy(_nowHeight,_outHeight,sizeof(_user->_meshOutHeight));
//随机填充数据,模拟雨点
#ifndef __TEST_TEMPLATE__
	if (_user->_deltaTime > 0.3f)
	{
		_user->_deltaTime = 0.0f;
		float     idx = _context->randomValue()* WAVE_SIZE;
		float     idy=_context->randomValue()* WAVE_SIZE;
		const    float  radius = 16.0f;
	//	_nowHeight[(int)idx][(int)idy] += -4.0f;
		for (int i = 1; i < GRID_SIZE - 1; ++i)
		{
			for (int j = 1; j < GRID_SIZE - 1; ++j)
			{
				const  float   real_radius = sqrt((idx - i)*(idx - i) + (idy - j)*(idy - j));
				_nowHeight[i][j] -= exp(-real_radius / radius*6.0f-1.5f);
			}
		}
	}
#endif
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	WaveShader   *_wave = &_user->_waterShader;
	_wave->_waterProgram->perform();

	_user->_meshObject->bindVertexObject(0);
	_user->_meshObject->bindTexCoordObject(1);

	glBindBuffer(GL_ARRAY_BUFFER, _user->_waveNormalId);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3,GL_FLOAT,GL_FALSE,0,NULL);
//uniforms
	glUniformMatrix4fv(_wave->_modelMatrixLoc, 1, GL_FALSE, _user->_modelMatrix.pointer());
	glUniformMatrix4fv(_wave->_viewProjMatrixLoc, 1, GL_FALSE, _user->_viewProjMatrix.pointer());
	glUniformMatrix3fv(_wave->_normalMatrixLoc, 1, GL_FALSE, _user->_normalMatrix.pointer());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _user->_skyboxMap->name());
	glUniform1i(_wave->_skyTexMapLoc, 0);

	glUniform3fv(_wave->_freshnelParamLoc,1,&_user->_freshnelParam.x);
	glUniform3fv(_wave->_eyePositionLoc, 1, &_user->_eyePosition.x);
	glUniform1f(_wave->_refractRatioLoc, _user->_refractRatio); 
	glUniform4fv(_wave->_waterColorLoc, 1, &_user->_waterColor.x);

	_user->_meshObject->drawShape();
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->_skyboxMap->release();
	_user->_meshObject->release();
	glDeleteBuffers(1, &_user->_waveNormalId);
}
///////////////////////////don not modify below function////////////////////////////////////
