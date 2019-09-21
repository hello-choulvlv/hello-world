//
#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/TGAImage.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<engine/Sprite.h>
#include<engine/GLShadowMap.h>

//Common  Data  Struct

struct       UserData
{
	GLProgram           *object;
	GLuint                   baseMapId;
	GLuint                   normalMapId;
	GLTexture             *baseTexture;
	GLTexture             *normalTexture;
//
	GLuint                  u_baseMapLoc;
	GLuint                  u_normalMapLoc;
	GLuint                  u_normalMatrixLoc;
	GLuint                  u_mvpMatrixLoc;
	GLuint                  u_lightVectorLoc;//光线的方向
//光线的方向
	  GLVector3           lightVector;
//旋转的角度
	  float                     angleArc;
	  Matrix                 mvpMatrix;
	  Matrix3               normalMatrix;
//数据
	  GLSphere			  *vSphere;
};
//

void        Init(GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;
//
	_user->object = GLProgram::createWithFile("shader/normal/normal.vsh", "shader/normal/normal.fsh");
	_user->u_mvpMatrixLoc = _user->object->getUniformLocation("u_mvpMatrix");
	_user->u_baseMapLoc = _user->object->getUniformLocation("u_baseMap");
	_user->u_normalMapLoc = _user->object->getUniformLocation("u_normalMap");
	_user->u_lightVectorLoc = _user->object->getUniformLocation("u_lightVector");
	_user->u_normalMatrixLoc = _user->object->getUniformLocation("u_normalMatrix");
//TGAImage
//	TGAImage		_baseMap("tga/normal/IceMoon.tga");
//	_user->baseMapId = _baseMap.genTextureMap();
	_user->baseMapId = 0;
	_user->baseTexture = GLTexture::createWithFile("tga/normal/IceMoon.tga");
	_user->baseTexture->genMipmap();
//Normal Texture
//	TGAImage	    _normalMap("tga/normal/IceMoonBump.tga");
//	_user->normalMapId = _normalMap.genTextureMap();
	_user->normalMapId = 0;
	_user->normalTexture = GLTexture::createWithFile("tga/normal/IceMoonBump.tga");
	_user->normalTexture->genMipmap();
//Sphere
	_user->vSphere = GLSphere::createWithSlice(256, 0.6f);
//
	_user->lightVector = normalize(&GLVector3(1.0f,1.0f,1.0f));
//
	_user->angleArc = 0.0f;
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
//	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->angleArc += _deltaTime*6.0f;
	if (_user->angleArc >= 360.0f)
		_user->angleArc -= 360.0f;
	_user->mvpMatrix.identity();
	_user->mvpMatrix.rotate(_user->angleArc,0.0f,1.0f,0.0f);
	_user->normalMatrix = _user->mvpMatrix.normalMatrix();
	_user->mvpMatrix.translate(0.0f, 0.0f, -4.0f);
	_user->mvpMatrix.orthoProject(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
	_user->object->enableObject();
	_user->vSphere->bindVertexObject(0);
	_user->vSphere->bindTexCoordObject(1);
	_user->vSphere->bindNormalObject(2);
	_user->vSphere->bindTangentObject(3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_user->baseTexture->name() );// _user->baseMapId);
	glUniform1i(_user->u_baseMapLoc,0);
// 
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _user->normalTexture->name());// _user->normalMapId);
	glUniform1i(_user->u_normalMapLoc,1);
//Matrix
	glUniformMatrix4fv(_user->u_mvpMatrixLoc, 1,GL_FALSE,_user->mvpMatrix.pointer());
//normal matrix
	glUniformMatrix3fv(_user->u_normalMatrixLoc, 1, GL_FALSE, _user->normalMatrix.pointer());
//
	glUniform3fv(_user->u_lightVectorLoc, 1, &_user->lightVector.x);
	_user->vSphere->drawShape();
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->object->release();
	_user->object = NULL;
	glDeleteTextures(1, &_user->baseMapId);
	_user->baseMapId = 0;
	glDeleteTextures(1, &_user->normalMapId);
	_user->normalMapId = 0;
	_user->normalTexture->release();
	_user->normalTexture = NULL;
	_user->baseTexture->release();
	_user->baseTexture = NULL;
	_user->vSphere->release();
	_user->vSphere = NULL;
}
///////////////////////////don not modify below function////////////////////////////////////
