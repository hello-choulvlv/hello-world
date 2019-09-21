/*
  *精灵的实现
  *2016-6-17 20:11:40
  *version:1.0
*/
#include<engine/Sprite.h>
#include<engine/Geometry.h>
#include<engine/GLCacheManager.h>
#include<engine/GLContext.h>
#include<engine/GLState.h>
#include<GL/glew.h>
#include<stdio.h>
#include<string.h>
__NS_GLK_BEGIN

Sprite::Sprite()
{
	_glTexture = NULL;
	_glProgram = NULL;
	_vertexVBO = 0;
	_baseMapLoc = 0;
	_mvMatrixLoc = 0;
//
	_renderColor = GLVector4(1.0f,1.0f,1.0f,1.0f);
}
Sprite::~Sprite()
{
	glDeleteBuffers(1 ,& _vertexVBO);
	_glTexture->release();
	_glProgram->release();
	_vertexVBO = 0;
	_glTexture = NULL;
	_glProgram = NULL;
}
//创建Sprite对象
void     Sprite::initWithFile(const char *file_name)
{
	_glTexture = GLTexture::createWithFile(file_name);//GLCacheManager::getInstance()->findGLTexture(file_name);
//创建顶点缓冲区对象
	Size     _size = GLContext::getInstance()->getWinSize();
	float    _width = _glTexture->getWidth() / _size.width;
	float    _height = _glTexture->getHeight() / _size.height;
	float    _vertex[] = {
		       -_width,_height,0.0f, 0.0f,1.0f,
			   -_width,-_height,0.0f,0.0f,0.0f,
			   _width,_height,0.0f,1.0f,1.0f,
			   _width,-_height,0.0f,1.0f,0.0f
	};
	glGenBuffers(1, &_vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_vertex), _vertex, GL_STATIC_DRAW);
//着色器
	_glProgram = GLCacheManager::getInstance()->findGLProgram(GLCacheManager::GLProgramType_TextureColor);
	_glProgram->retain();
	_mvMatrixLoc = _glProgram->getUniformLocation("u_mvMatrix");
	_baseMapLoc = _glProgram->getUniformLocation("u_baseMap");
	_renderColorLoc = _glProgram->getUniformLocation("u_renderColor");
	glGenVertexArrays(1, &_vertexArrayBufferId);
	glBindVertexArray( _vertexArrayBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);

	glEnableVertexAttribArray(GLAttribPosition);
	glVertexAttribPointer(GLAttribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);

	glEnableVertexAttribArray(GLAttribTexCoord);
	glVertexAttribPointer(GLAttribTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
	glBindVertexArray(0);

}

Sprite             *Sprite::createWithFile(const char *file_name)
{
	Sprite    *_sprite = new   Sprite();
	_sprite->initWithFile(file_name);
	return  _sprite;
}
//
void     Sprite::setRenderColor(GLVector4 *renderColor)
{
	_renderColor = *renderColor;
}
//设置变换矩阵
void      Sprite::setAffineMatrix(Matrix *affineMatrix)
{
	_mvMatrix = *affineMatrix;
}

Matrix   *Sprite::getAffineMattix()
{
	return &_mvMatrix;
}

//渲染精灵
void     Sprite::render()
{
	_glProgram->perform();
	glBindVertexArray(_vertexArrayBufferId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _glTexture->getName());
	glUniform1i(_baseMapLoc,0);
//颜色
	glUniform4fv(_renderColorLoc, 1, (float*)&_renderColor);
//变换矩阵
	glUniformMatrix4fv(_mvMatrixLoc, 1, GL_FALSE, _mvMatrix.pointer());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
}

void  Sprite::render(Matrix *_affineMatrix,unsigned   otherTextureId)
{
	_glProgram->enableObject();
	glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);

	glEnableVertexAttribArray(GLAttribPosition);
	glVertexAttribPointer(GLAttribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);

	glEnableVertexAttribArray(GLAttribTexCoord);
	glVertexAttribPointer(GLAttribTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, otherTextureId?otherTextureId:_glTexture->getName());
	glUniform1i(_baseMapLoc, 0);
//颜色
	glUniform4fv(_renderColorLoc, 1, (float*)&_renderColor);
//变换矩阵
	glUniformMatrix4fv(_mvMatrixLoc, 1, GL_FALSE, _affineMatrix->pointer());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
//获取精灵的大小
Size        &Sprite::getContentSize()
{
	return  _glTexture->getContentSize();
}
//设置Mipmap
void     Sprite::setMipmap()
{
	_glTexture->genMipmap();
}

__NS_GLK_END