#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/TGAImage.h>
#include<engine/Geometry.h>
#include<engine/NoiseTexture.h>
//以下三个宏只能在同时间开启一个
//#define  __TEST__
//#define     __NOISE3D__
//连续噪声
#define     __CONSISTENCY__
struct       UserData
{
	GLProgram	*object;
#ifdef  __TEST__
	NoiseTexture2   *noiseTexture;
#endif
#ifdef  __CONSISTENCY__
	NoiseConsistency    *noiseConsistency;
#endif
//纹理
#ifdef  __NOISE3D__
	unsigned          noiseTextureId;
#endif
	unsigned          vertexBufferId;
	unsigned          u_baseMapLoc;
	unsigned          u_mvpMatrixLoc;
	unsigned          u_timeLoc;
	float                  u_time;
	Matrix              u_mvpMatrix;
};
//

void        Init(GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	_user->object = GLProgram::createWithFile("shader/noise/noise.vsh", "shader/noise/noise.fsh");
#ifdef __TEST__
	_user->noiseTexture = NoiseTexture2::createWithSeed(0x7C8B9F7, 32.0f, 1024);
#endif
	_user->u_mvpMatrixLoc = _user->object->getUniformLocation("u_mvpMatrix");
	_user->u_baseMapLoc = _user->object->getUniformLocation("u_baseMap");
	_user->u_timeLoc=_user->object->getUniformLocation("u_time");
#ifdef  __TEST__
	float          _factor = 1.0;// 64.0f / _context->getWinSize().width*2.0f;
	float          VertexData[] = {
		-_factor, _factor, 0.0f, 0.0f, 1.0f,
		-_factor ,- _factor, 0.0f, 0.0f, 0.0f,
		_factor, _factor, 0.0f, 1.0f, 1.0f,
		_factor, -_factor, 0.0f, 1.0f, 0.0f,
	};
#endif
#ifdef  __NOISE3D__  
	float          factor = 0.5f;
	float          VertexData[] = {
		-factor, factor, 0.0f, 0.0f, 1.0f,
		-factor, -factor, 0.0f, 0.0f, 0.0f,
		factor, factor, 0.0f, 1.0f, 1.0f,
		factor, -factor, 0.0f, 1.0f, 0.0f,
	};
#endif
#ifdef __CONSISTENCY__
	float          factor = 0.5f;
	float          VertexData[] = {
		-factor, factor, 0.0f, 0.0f, 1.0f,
		-factor, -factor, 0.0f, 0.0f, 0.0f,
		factor, factor, 0.0f, 1.0f, 1.0f,
		factor, -factor, 0.0f, 1.0f, 0.0f,
	};
#endif
	glGenBuffers(1, &_user->vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _user->vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#ifdef __NOISE3D__
	_user->noiseTextureId = TGAImage::gen3DNoiseTextureMap(128, 16.0f);
#endif
#ifdef  __CONSISTENCY__
	_user->noiseConsistency = NoiseConsistency::createWithSeed(0x7C809B77,16.0f, 128);
#endif
	_user->u_mvpMatrix.identity();
	_user->u_time = 0.0f;
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->u_time +=  _deltaTime*0.1;
	if (_user->u_time >= 360.0f)
		_user->u_time -= 360.0f;
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	_user->object->enableObject();
	glBindBuffer(GL_ARRAY_BUFFER, _user->vertexBufferId);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float)*3));
//
#ifdef  __TEST__
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->noiseTexture->name());
#endif
#ifdef  __NOISE3D__
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, _user->noiseTextureId);// _user->noiseTexture->name());
#endif
#ifdef  __CONSISTENCY__
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D,  _user->noiseConsistency->name());
#endif
	glUniform1i(_user->u_baseMapLoc,0);
	glUniform1f(_user->u_timeLoc,_user->u_time);
	glUniformMatrix4fv(_user->u_mvpMatrixLoc, 1, GL_FALSE, _user->u_mvpMatrix.pointer());
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->object->release();
#ifdef  __TEST__
	_user->noiseTexture->release();
#endif
#ifdef  __NOISE3D__
	glDeleteTextures(1,&_user->noiseTexture);
#endif
#ifdef  __CONSISTENCY__
	_user->noiseConsistency->release();
#endif
}
///////////////////////////don not modify below function////////////////////////////////////
