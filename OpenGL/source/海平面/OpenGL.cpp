#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/TGAImage.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<engine/NoiseTexture.h>
#include<math.h>
//是否添加波光
#define   __USE_WAVE_LIGHT__
struct       UserData
{
	GLProgram	*object;
//纹理
	unsigned          baseMapId;
	Matrix             u_mvpMatrix;
	Matrix             u_modelViewMatrix;
//波传导的方向,最好是单位化
	GLVector4       u_DParams;
//控制波峰的陡峭程度
	GLVector2        u_QParams;
//波的振幅
	GLVector2        u_AParams;
//波的角速度,注意这里是一个伪角速度,并没有严格遵循数学公式 Ω=2π/λ,(λ为波长),而在正常的数学函数中,分母应该为波的传播周期
	GLVector2        u_omegaWave;
//波经过的时间
	GLVector2         u_deltaTime;
//fi = v * 2π/λ,也可以理解成伪传播速度
	GLVector2         u_waveFi;
#ifdef __USE_WAVE_LIGHT__
	GLVector4          u_fragColor;
	GLVector2          u_resolution;
	float                     u_time;
#endif
//Loc
	unsigned           u_mvpMatrixLoc;
	unsigned           u_modelViewMatrixLoc;
	unsigned           u_baseMapLoc;
	unsigned           u_noiseMapLoc;
	unsigned           u_DParamsLoc;
	unsigned           u_QParamsLoc;
	unsigned           u_AParamsLoc;
	unsigned           u_omegaWaveLoc;
	unsigned           u_deltaTimeLoc;
	unsigned           u_waveFiLoc;
#ifdef __USE_WAVE_LIGHT__
	unsigned           u_resolutionLoc;
	unsigned           u_timeLoc;
	unsigned           u_fragColorLoc;
#endif
	Mesh              *_meshObject;
	NoiseTexture2    *_noiseMap;
};
//

void        Init(GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;
#ifdef  __USE_WAVE_LIGHT__
	_user->object = GLProgram::createWithFile("shader/water/water_gernster.vsh", "shader/water/water_wave.fsh");
#else
	_user->object = GLProgram::createWithFile("shader/water/water_gernster.vsh", "shader/water/water_gernster.fsh");
#endif
	_user->u_mvpMatrixLoc = _user->object->getUniformLocation("u_mvpMatrix");
	_user->u_modelViewMatrixLoc = _user->object->getUniformLocation("u_modelViewMatrix");
	_user->u_baseMapLoc = _user->object->getUniformLocation("u_baseMap");
	_user->u_noiseMapLoc = _user->object->getUniformLocation("u_noiseMap");
	_user->u_DParamsLoc = _user->object->getUniformLocation("u_DParams");
	_user->u_QParamsLoc = _user->object->getUniformLocation("u_QParams");
	_user->u_AParamsLoc = _user->object->getUniformLocation("u_AParams");
	_user->u_omegaWaveLoc = _user->object->getUniformLocation("u_omegaWave");
	_user->u_deltaTimeLoc = _user->object->getUniformLocation("u_deltaTime");
	_user->u_waveFiLoc = _user->object->getUniformLocation("u_waveFi");
#ifdef __USE_WAVE_LIGHT__
	_user->u_timeLoc = _user->object->getUniformLocation("u_time");
	_user->u_fragColorLoc = _user->object->getUniformLocation("u_fragColor");
	_user->u_resolutionLoc = _user->object->getUniformLocation("u_resolution");
#endif
//写入数据
	_user->u_mvpMatrix.identity();
	_user->u_mvpMatrix.rotate(-90.0f,1.0f,0.0f,0.0f);
_user->u_mvpMatrix.translate(0.0f, -0.9f, 0.0f);
//视图矩阵
	Matrix        viewMatrix;
	viewMatrix.lookAt(GLVector3(0,20.0f,1.0f),GLVector3(0.0f,0.0f,-6.0f),GLVector3(0.0f,1.0f,0.0f));
	_user->u_mvpMatrix.multiply(viewMatrix);
	_user->u_modelViewMatrix = _user->u_mvpMatrix;

	_user->u_mvpMatrix.identity();
	Size     _size = _context->getWinSize();
	_user->u_mvpMatrix.perspective(60.0f, _size.width/_size.height,0.1f,300.0f);
//假定波长
	const   float     kAmpOverLen = 0.207f;
	float     _lamda = 2.83f;
	float     _lamda2 = 1.73f;
	float     _speedx = 0.37f;
	float     _speedx2 = 0.57f;
	GLVector2    _nVector1 = normalize(&GLVector2(1.0f, 1.6f)), _nVector2=GLVector2(normalize(&GLVector2(0.5f,-0.3f)));
	_user->u_DParams = GLVector4(_nVector1.x,_nVector1.y,_nVector2.x,_nVector2.y);
	_user->u_AParams = GLVector2(_lamda2*kAmpOverLen, _lamda2*kAmpOverLen);
	_user->u_QParams = GLVector2(0.12f,0.18);
	_user->u_omegaWave = GLVector2(sqrt(9.8f * 2 * __MATH_PI__ / _lamda), sqrt(9.8f * 2 * __MATH_PI__ / _lamda2));
	_user->u_waveFi =GLVector2( _speedx * 2 * __MATH_PI__ / _lamda,_speedx2*2*__MATH_PI__/_lamda2);
	_user->u_deltaTime = GLVector2(0.0f,0.73f);
#ifdef __USE_WAVE_LIGHT__
	_user->u_time = 0.0f;
	_user->u_fragColor = GLVector4(1.0f,1.0f,1.0f,1.0f);
	_user->u_resolution = GLVector2(_size.width,_size.height);
#endif
//网格对象
	_user->_meshObject = Mesh::createWithIntensity(40, 20.0f, 20.0f, 20.0f);
//添加噪声
	_user->_noiseMap = NoiseTexture2::createWithSeed(0x7C8D9AF7, 17.0f, 1024);
	TGAImage		_baseMap("tga/water/water.bmp");
	_user->baseMapId = _baseMap.genTextureMap(GL_REPEAT);
//
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->u_deltaTime.x += _deltaTime*0.787f;
	_user->u_deltaTime.y += _deltaTime*0.8749f;
	//if (_user->u_frameAngle >= 360.0f)
	//	_user->u_frameAngle -= 360.0f;
#ifdef __USE_WAVE_LIGHT__
	_user->u_time += _deltaTime;
#endif
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_user->object->enableObject();
	_user->_meshObject->bindVertexObject(0);
	_user->_meshObject->bindTexCoordObject(1);
//矩阵,向量,标量
	glUniformMatrix4fv(_user->u_mvpMatrixLoc, 1, GL_FALSE, _user->u_mvpMatrix.pointer());
	glUniformMatrix4fv(_user->u_modelViewMatrixLoc,1,GL_FALSE,_user->u_modelViewMatrix.pointer());
	glUniform4fv(_user->u_DParamsLoc, 1, &_user->u_DParams.x);
	glUniform2fv(_user->u_QParamsLoc, 1,&_user->u_QParams.x);
	glUniform2fv(_user->u_AParamsLoc,1,& _user->u_AParams.x);
	glUniform2fv(_user->u_deltaTimeLoc, 1,&_user->u_deltaTime.x);
	glUniform2fv(_user->u_waveFiLoc, 1,&_user->u_waveFi.x);
	glUniform2fv(_user->u_omegaWaveLoc,1, &_user->u_omegaWave.x);
#ifdef __USE_WAVE_LIGHT__
	glUniform4fv(_user->u_fragColorLoc, 1, &_user->u_fragColor.x);
	glUniform2fv(_user->u_resolutionLoc, 1, &_user->u_resolution.x);
	glUniform1f(_user->u_timeLoc, _user->u_time);
#endif
//纹理
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->baseMapId);
	glUniform1i(_user->u_baseMapLoc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _user->_noiseMap->name());
	glUniform1i(_user->u_noiseMapLoc,1);

	_user->_meshObject->drawShape();
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->object->release();
	glDeleteTextures(1, &_user->baseMapId);
	_user->_meshObject->release();
	_user->_noiseMap->release();
}
///////////////////////////don not modify below function////////////////////////////////////
