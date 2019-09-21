#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/TGAImage.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<math.h>
struct       UserData
{
	GLProgram	*object;
//纹理
	unsigned          baseMapId;
//位置
	unsigned         u_mvpMatrixLoc;
	unsigned         u_modelMatrixLoc;
	unsigned         u_normalMatrixLoc;
	unsigned         u_baseMapLoc;
	unsigned         u_lightColorLoc;
	unsigned         u_lightPositionLoc;
	unsigned         u_lightDirectionLoc;
	unsigned         u_eyePositionLoc;
	unsigned         u_ambientColorLoc;
	unsigned         u_specularFactorLoc;
	unsigned         u_cosThetaLoc;
//实际的数值
	Matrix            u_mvpMatrix;
	Matrix            u_modelMatrix;
	Matrix            u_projectMatrix;
	Matrix3          u_normalMatrix;
	GLVector3     u_lightColor;
	GLVector3     u_lightPosition;
	GLVector3    u_lightDirection;
	GLVector3     u_eyePosition;
	GLVector3     u_ambientColor;
	float                u_cosTheta;
	float                u_specularFactor;
//
	GLSphere		*vSphere;
	float                deltaAngle;
};
//

void        Init(GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	_user->object = GLProgram::createWithFile("shader/light/light_spot.vsh", "shader/light/light_spot.fsh");
	_user->u_mvpMatrixLoc = _user->object->getUniformLocation("u_mvpMatrix");
	_user->u_modelMatrixLoc = _user->object->getUniformLocation("u_modelMatrix");
	_user->u_normalMatrixLoc = _user->object->getUniformLocation("u_normalMatrix");
	_user->u_baseMapLoc = _user->object->getUniformLocation("u_baseMap");
	_user->u_lightColorLoc = _user->object->getUniformLocation("u_lightColor");
	_user->u_lightPositionLoc = _user->object->getUniformLocation("u_lightPosition");
	_user->u_lightDirectionLoc = _user->object->getUniformLocation("u_lightDirection");
	_user->u_ambientColorLoc = _user->object->getUniformLocation("u_ambientColor");
	_user->u_eyePositionLoc = _user->object->getUniformLocation("u_eyePosition");
	_user->u_specularFactorLoc = _user->object->getUniformLocation("u_specularFactor");
	_user->u_cosThetaLoc = _user->object->getUniformLocation("u_cosTheta");
//
	TGAImage		_baseMap("tga/Earth512x256.tga");
	_user->baseMapId = _baseMap.genTextureMap();
//矩阵
//	_user->u_mvpMatrix.identity();
//	_user->u_mvpMatrix.translate(0.0f, 0.0f, -4.0f);

	_user->u_projectMatrix.orthoProject(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 10.0f);
//
	_user->vSphere = GLSphere::createWithSlice(128, 0.6f);
//光线
	_user->u_lightColor = GLVector3(0.8f,0.8f,1.0f);
	_user->u_lightPosition = GLVector3(0.5f,0.0f,-3.0f);
	_user->u_eyePosition = GLVector3(1.0f,0.0f,0.0f);
	_user->u_lightDirection = normalize(&GLVector3(_user->u_lightPosition.x, _user->u_lightPosition.y, _user->u_lightPosition.z+4.0f));
	_user->u_ambientColor = GLVector3(0.1,0.1,0.1);
	_user->u_specularFactor = 0.36f;
	_user->u_cosTheta = cos(15.0f / __MATH_PI__);
//
	_user->deltaAngle = 0.0f;
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->deltaAngle += _deltaTime*12.0f;
	if (_user->deltaAngle >= 360.0f)
		_user->deltaAngle -= 360.0f;
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	_user->object->enableObject();
	_user->vSphere->bindVertexObject(0);
	_user->vSphere->bindTexCoordObject(1);
	_user->vSphere->bindNormalObject(2);
//
	glActiveTexture(GL_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, _user->baseMapId);
	glUniform1i(_user->u_baseMapLoc, 0);
//Matrix
	_user->u_mvpMatrix.identity();
	_user->u_mvpMatrix.rotate(17.0f,0.0f,1.0f,-0.2f);
	_user->u_mvpMatrix.rotate(_user->deltaAngle, 0.0f, 1.0f, 0.0f);
	_user->u_mvpMatrix.translate(0.0f, 0.0f, -4.0f);

	_user->u_normalMatrix = _user->u_mvpMatrix.normalMatrix();
//传递模型矩阵
	glUniformMatrix4fv(_user->u_modelMatrixLoc, 1, GL_FALSE, _user->u_mvpMatrix.pointer());
	_user->u_mvpMatrix.multiply(_user->u_projectMatrix);
//MVP矩阵,法线矩阵
	glUniformMatrix4fv(_user->u_mvpMatrixLoc, 1, GL_FALSE, _user->u_mvpMatrix.pointer());
	glUniformMatrix3fv(_user->u_normalMatrixLoc,1,GL_FALSE,_user->u_normalMatrix.pointer());
//光线
	glUniform3fv(_user->u_lightColorLoc, 1, &_user->u_lightColor.x);
	glUniform3fv(_user->u_lightPositionLoc, 1, &_user->u_lightPosition.x);
	glUniform3fv(_user->u_eyePositionLoc, 1, &_user->u_eyePosition.x);
	glUniform3fv(_user->u_ambientColorLoc,1,&_user->u_ambientColor.x);
	glUniform3fv(_user->u_lightDirectionLoc, 1, &_user->u_lightDirection.x);
	glUniform1f(_user->u_specularFactorLoc, _user->u_specularFactor);
	glUniform1f(_user->u_cosThetaLoc, _user->u_cosTheta);

	_user->vSphere->drawShape();
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->object->release();
	_user->vSphere->release();
	glDeleteTextures(1, &_user->baseMapId);

}
///////////////////////////don not modify below function////////////////////////////////////
