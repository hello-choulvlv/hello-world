/*
 *延迟着色前阶段处理
 *2016-10-9 20:55:49
 */
#include<GL/glew.h>
#include<engine/GLContext.h>
#include "RenderSphere.h"
RenderSphere::RenderSphere()
{
	_glProgram = NULL;
	_texture = NULL;
	_sphere = NULL;
_deltaTime=	0.0f;
}

RenderSphere::~RenderSphere()
{
	_glProgram->release();
	_texture->release();
	_sphere->release();
	_glProgram = NULL;
	_texture = NULL;
	_sphere = NULL;
}

void              RenderSphere::initWithFile(const char *file_name)
{
	_texture = GLTexture::createWithFile(file_name);
	_glProgram = GLProgram::createWithFile("shader/ssao/ssao_normal.vsh", "shader/ssao/ssao_normal.fsh");
	_sphere = GLSphere::createWithSlice(128,1.0f );
//location of uniforms
	_mvpMatrixLoc = _glProgram->getUniformLocation("u_mvpMatrix");
	_modelMatrixLoc = _glProgram->getUniformLocation("u_modelMatrix");
	_normalMatrixLoc = _glProgram->getUniformLocation("u_normalMatrix");
	_baseMapLoc = _glProgram->getUniformLocation("u_baseMap");
	_nearfarLoc = _glProgram->getUniformLocation("u_nearFarPlane");
}

RenderSphere	*RenderSphere::create(const char *file_name)
{
	RenderSphere  *_render = new   RenderSphere();
	_render->initWithFile(file_name);
	return _render;
}

void       RenderSphere::update(float deltaTime)
{
	_deltaTime += deltaTime*12.0f;
	if (_deltaTime >= 360.0f)
		_deltaTime -= 360.0f;
}

void       RenderSphere::draw(Matrix &projMatrix, unsigned   flag)
{
//开启着色器
	_glProgram->enableObject();
//传输顶点数据
	_sphere->bindVertexObject(0);
	_sphere->bindTexCoordObject(1);
	_sphere->bindNormalObject(2);
//传递矩阵
	_modelMatrix.identity();
	_modelMatrix.rotate(17.0f, 0.0f, 1.0f, -0.15f);
	_modelMatrix.rotate(_deltaTime, 0.0f, 1.0f, 0.0f);
	_modelMatrix.translate(_position.x,_position.y,_position.z);

	_normalMatrix = _modelMatrix.normalMatrix();

	_mvpMatrix = _modelMatrix *projMatrix;

	glUniformMatrix4fv(_mvpMatrixLoc, 1, GL_FALSE, _mvpMatrix.pointer());
	glUniformMatrix4fv(_modelMatrixLoc, 1, GL_FALSE, _modelMatrix.pointer());
	glUniformMatrix3fv(_normalMatrixLoc, 1, GL_FALSE, _normalMatrix.pointer());
	glUniform2fv(_nearfarLoc, 1, &GLContext::getInstance()->getNearFarPlane().x);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture->name());
	glUniform1i(_baseMapLoc, 0);

	_sphere->drawShape();
}

void          RenderSphere::setPosition(GLVector3 &newPosition)
{
	_position = newPosition;
}