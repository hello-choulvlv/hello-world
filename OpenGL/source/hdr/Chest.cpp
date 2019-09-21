/**
 *箱子
 *2016-9-20 18:25:28
  *@Author:小花熊
 */
#include "Chest.h"
Chest::Chest()
{
	_glProgram = NULL;
	_chestShape = NULL;
	_scaleX = 1.0f;
	_scaleY = 1.0f;
	_scaleZ = 1.0f;
	_position = GLVector3(0.0f,0.0f,0.0f);
	_lightColor = GLVector3(1.0f,1.0f,1.0f);
	_specularFactor = 1.0f;
	_angle = 0.0f;
}
Chest::~Chest()
{
	_glProgram->release();
	_chestShape->release();
	_texture->release();
}

void         Chest::initChest(const  char   *file_name)
{
	_glProgram = GLProgram::createWithFile("shader/hdr/box.vsh", "shader/hdr/box.fsh");
	_glProgram->retain();
	_chestShape = GLChest::createWithScale(1.0f);
	_texture = GLTexture::createWithFile(file_name);
	_texture->retain();
//获取统一变量的位置
	_mvpMatrixLoc = _glProgram->getUniformLocation("u_mvpMatrix");
	_modelViewMatrixLoc = _glProgram->getUniformLocation("u_modelViewMatrix");
	_normalMatrixLoc = _glProgram->getUniformLocation("u_normalMatrix");
	_lightPositionLoc = _glProgram->getUniformLocation("u_lightPosition");
	_baseMapLoc = _glProgram->getUniformLocation("u_baseMap");
	_lightColorLoc = _glProgram->getUniformLocation("u_lightColor");
	_ambientColorLoc = _glProgram->getUniformLocation("u_ambientColor");
	_eyePositionLoc = _glProgram->getUniformLocation("u_eyePosition");
	_specularFactorLoc = _glProgram->getUniformLocation("u_specularFactor");
}
Chest            *Chest::createChest(const char *file_name)
{
	Chest		*_chest = new   Chest();
	_chest->initChest(file_name);
	return  _chest;
}

//设置位置
void       Chest::setPosition(GLVector3 &_newPosition)
{
	_position = _newPosition;
}

void      Chest::setScale(float   _new_scaleXYZ)
{
	_scaleX = _scaleY = _scaleZ = _new_scaleXYZ;
}

void      Chest::setScale(float scaleX, float scaleY, float scaleZ)
{
	_scaleX = scaleX;
	_scaleY = scaleY;
	_scaleZ = scaleZ;
}

void     Chest::setRotateAngle(float angle, GLVector3 &axis)
{
	_rotateMatrix.identity();
	_rotateMatrix.rotate(angle, axis.x, axis.y, axis.z);
}
//关于光线的函数
void     Chest::setLightPosition(GLVector3 &lightPosition)
{
	_lightPosition = lightPosition;
}

void     Chest::setLightColor(GLVector3 &color)
{
	_lightColor = color;
}

void     Chest::setAmbientColor(GLVector3 &ambientColor)
{
	_ambientColor = ambientColor;
}

void     Chest::setEyePosition(GLVector3 &eyePosition)
{
	_eyePosition = eyePosition;
}

void     Chest::setSpecularCoeff(float _coeff)
{
	_specularFactor = _coeff;
}
void      Chest::update(float _delta)
{
	_angle += 16.0f*_delta;
	if (_angle > 360.0f)
		_angle -= 360.0f;
	_rotateMatrix.identity();
	_rotateMatrix.rotate(_angle, 1.0f, 0.5f, 0.5f);
}
void      Chest::draw(Matrix &projectMatrix, unsigned drawFlag)
{
//逐步合成 平移,旋转,缩放矩阵
	Matrix        translMatrix;
	Matrix        scaleMatrix;

	translMatrix.translate(_position.x, _position.y, _position.z);
	scaleMatrix.scale(_scaleX,_scaleY,_scaleZ);

	_modelViewMatrix = scaleMatrix * _rotateMatrix * translMatrix;
//导出法线矩阵
	_normalMatrix = _modelViewMatrix.normalMatrix();

	_mvpMatrix = _modelViewMatrix * projectMatrix;
//向OpenGL程序对象传递数据
	_glProgram->enableObject();
	_chestShape->bindVertexObject(0);
	_chestShape->bindNormalObject(1);
	_chestShape->bindTexCoordObject(2);
//矩阵
	glUniformMatrix4fv(_mvpMatrixLoc, 1, GL_FALSE, _mvpMatrix.pointer());
	glUniformMatrix4fv(_modelViewMatrixLoc, 1, GL_FALSE, _modelViewMatrix.pointer());
	glUniformMatrix3fv(_normalMatrixLoc, 1, GL_FALSE, _normalMatrix.pointer());
//光线
	glUniform3fv(_lightPositionLoc, 1, &_lightPosition.x);
	glUniform3fv(_lightColorLoc, 1, &_lightColor.x);
	glUniform3fv(_ambientColorLoc, 1, &_ambientColor.x);
	glUniform3fv(_eyePositionLoc, 1, &_eyePosition.x);
	glUniform1f(_specularFactorLoc, _specularFactor);
//
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture->name());
	glUniform1i(_baseMapLoc,0);

	_chestShape->drawShape();
}