/*
  *作为光源的箱子的实现
  *2016-9-22 08:56:46
  *@Author:小花熊
  */
#include<GL/glew.h>
#include "LightChest.h"

LightChest::LightChest()
{
	_glProgram = NULL;
	_lightChestShape = NULL;
	_angle = 0.0f;
}

LightChest::~LightChest()
{
	_glProgram->release();
	_lightChestShape->release();
}
LightChest		  *LightChest::createLightChest(GLVector3 &lightColor)
{
	LightChest	*_lightChest = new   LightChest();
	_lightChest->initLightChest(lightColor);
	return  _lightChest;
}

void          LightChest::initLightChest(GLVector3 &lightColor)
{
	_lightColor = lightColor;
//创建程序对象
	_glProgram = GLProgram::createWithFile("shader/hdr/light_box.vsh", "shader/hdr/light_box.fsh");
	_glProgram->retain();
	_lightChestShape = GLChest::createWithScale(1.0f);
//Uniform variable location
	_mvpMatrixLoc = _glProgram->getUniformLocation("u_mvpMatrix");
	_lightColorLoc = _glProgram->getUniformLocation("u_lightColor");
}

void          LightChest::setPosition(GLVector3 &position)
{
	_position = position;
}

void         LightChest::setScale(float scaleX, float scaleY, float scaleZ)
{
	_scaleX = scaleX;
	_scaleY = scaleY;
	_scaleZ = scaleZ;
}

void      LightChest::setScale(float	scaleXYZ)
{
	_scaleX = _scaleY = _scaleZ = scaleXYZ;
}

void        LightChest::setColor(GLVector3 &lightColor)
{
	_lightColor = lightColor;
}

void         LightChest::rotate(float   angle,GLVector3   &axis)
{
	_rotateMatrix.identity();
	_rotateMatrix.rotate(angle, axis.x, axis.y, axis.z);
}
void         LightChest::update(float _deltaTime)
{
	_angle += 12.0f*_deltaTime;
	if (_angle > 360.0f)
		_angle -= 360.0f;
	_rotateMatrix.identity();
	_rotateMatrix.rotate(_angle, 1.0f,0.5f,-0.5f );
}

void         LightChest::draw(Matrix &projectMatrix, unsigned flag)
{
	_glProgram->enableObject();
	_lightChestShape->bindVertexObject(0);
	
	Matrix     scaleMatrix, translateMatrix;
	scaleMatrix.scale(_scaleX,_scaleY,_scaleZ);
	translateMatrix.translate(_position.x, _position.y, _position.z);
	_modelMatrix = scaleMatrix *_rotateMatrix * translateMatrix;
	Matrix     mvpMatrix = _modelMatrix *projectMatrix;
	glUniformMatrix4fv(_mvpMatrixLoc, 1, GL_FALSE,mvpMatrix.pointer() );

	glUniform3fv(_lightColorLoc, 1, &_lightColor.x);

	_lightChestShape->drawShape();
}