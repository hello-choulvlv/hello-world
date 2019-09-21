#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/TGAImage.h>
#include<engine/Sprite.h>
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
	Sprite              *sprite;
	unsigned          vertexBufferId;
	unsigned          u_abValueLoc;
	unsigned          u_mvpMatrixLoc;
	unsigned          u_maskColorLoc;
	unsigned          u_shapeColorLoc;
	GLVector2       u_abValue;
	GLVector4       u_shapeColor;
	GLVector4       u_maskColor;
	Matrix              u_mvpMatrix;
};
//

void        Init(GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	_user->object = GLProgram::createWithFile("shader/program_texture/ellipse.vsh", "shader/program_texture/ellipse.fsh");
	_user->sprite = Sprite::createWithFile("tga/global_bg_big.tga");
	_user->u_mvpMatrixLoc = _user->object->getUniformLocation("u_mvpMatrix");
	_user->u_shapeColorLoc = _user->object->getUniformLocation("u_shapeColor");
	_user->u_maskColorLoc= _user->object->getUniformLocation("u_maskColor");
	_user->u_abValueLoc = _user->object->getUniformLocation("u_abValue");
	float          factor = 0.5f;
	float          VertexData[] = {
		-factor, factor, 0.0f, 0.0f, 1.0f,
		-factor, -factor, 0.0f, 0.0f, 0.0f,
		factor, factor, 0.0f, 1.0f, 1.0f,
		factor, -factor, 0.0f, 1.0f, 0.0f,
	};

	glGenBuffers(1, &_user->vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _user->vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	Size _size = _context->getWinSize();
	_user->u_abValue = GLVector2(_size.width*factor/2,_size.height*factor/2);
	_user->u_shapeColor = GLVector4(0.6f,0.8f,0.0f,1.0f);
	_user->u_maskColor = GLVector4(.0f,0.0f,1.0f,1.0f);
	_user->u_mvpMatrix.identity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;

}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
//背景
//	_user->sprite->render();
	_user->object->enableObject();
	glBindBuffer(GL_ARRAY_BUFFER, _user->vertexBufferId);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float)*3));

	glUniform4fv(_user->u_shapeColorLoc, 1, &_user->u_shapeColor.x);
	glUniform4fv(_user->u_maskColorLoc, 1, &_user->u_maskColor.x);
	glUniform2fv(_user->u_abValueLoc, 1, &_user->u_abValue.x);
	glUniformMatrix4fv(_user->u_mvpMatrixLoc, 1, GL_FALSE, _user->u_mvpMatrix.pointer());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->object->release();
	_user->sprite->release();
	glDeleteBuffers(1, &_user->vertexBufferId);
}
///////////////////////////don not modify below function////////////////////////////////////
