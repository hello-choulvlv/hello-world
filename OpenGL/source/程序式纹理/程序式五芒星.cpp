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
//	unsigned          baseMapId;
//位置
	unsigned         u_mvpMatrixLoc;
	unsigned         u_modelMatrixLoc;
	unsigned         u_normalMatrixLoc;
//光线
	unsigned         u_lightColorLoc;
	unsigned         u_lightPositionLoc;
	unsigned         u_eyePositionLoc;
	unsigned         u_ambientColorLoc;
	unsigned         u_specularFactorLoc;
//条纹
	unsigned         u_halfSpaceLoc;
	unsigned         u_pentacleColorLoc;
	unsigned         u_sphereColorLoc;
	unsigned         u_stripeWidthLoc;
	unsigned         u_borderWidthLoc;
//实际的数值
	Matrix            u_mvpMatrix;
	Matrix            u_modelMatrix;
	Matrix            u_projectMatrix;
	Matrix3          u_normalMatrix;
//光线
	GLVector3     u_lightColor;
	GLVector3     u_lightPosition;
	GLVector3     u_eyePosition;
	GLVector3     u_ambientColor;
//条纹
	GLVector3     u_halfSpace[5];
	GLVector3     u_pentacleColor;
	GLVector3     u_sphereColor;
	GLVector3     u_stripeColor;
	float                u_stripeWidth;
	float                u_borderWidth;
//光线
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
	_user->object = GLProgram::createWithFile("shader/program_texture/pentacle.vsh", "shader/program_texture/pentacle.fsh");
//Vertex Shader
	_user->u_mvpMatrixLoc = _user->object->getUniformLocation("u_mvpMatrix");
	_user->u_modelMatrixLoc = _user->object->getUniformLocation("u_modelMatrix");
	_user->u_normalMatrixLoc = _user->object->getUniformLocation("u_normalMatrix");
//光线
	_user->u_lightColorLoc = _user->object->getUniformLocation("u_lightColor");
	_user->u_lightPositionLoc = _user->object->getUniformLocation("u_lightPosition");
	_user->u_ambientColorLoc = _user->object->getUniformLocation("u_ambientColor");
	_user->u_eyePositionLoc = _user->object->getUniformLocation("u_eyePosition");
	_user->u_specularFactorLoc = _user->object->getUniformLocation("u_specularFactor");
//条纹
	_user->u_pentacleColorLoc = _user->object->getUniformLocation("u_pentacleColor");
	_user->u_halfSpaceLoc = _user->object->getUniformLocation("u_halfSpace");
	_user->u_sphereColorLoc = _user->object->getUniformLocation("u_sphereColor");
//	_user->u_stripeWidthLoc = _user->object->getUniformLocation("u_stripeWidth");
	_user->u_borderWidthLoc = _user->object->getUniformLocation("u_borderWidth");
//设置投影矩阵
	_user->u_projectMatrix.orthoProject(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 10.0f);
//
	_user->vSphere = GLSphere::createWithSlice(128, 0.68f);
//光线
	_user->u_lightColor = GLVector3(0.8f,0.8f,1.0f);
	_user->u_lightPosition = GLVector3(2.5f,0.5f,0.0f);
	_user->u_eyePosition = GLVector3(0.0f,0.0f,1.0f);
	_user->u_ambientColor = GLVector3(0.1f,0.1f,0.1f);
	_user->u_specularFactor = 0.36f;
//条纹
	_user->u_stripeColor = GLVector3(0.0f,1.0f,0.0f);
	_user->u_sphereColor = GLVector3(1.0f,0.0f,1.0f);
	_user->u_pentacleColor = GLVector3(0.0f,1.0f,0.0f);
//半空间
	_user->u_halfSpace[0] = GLVector3(1.0f,0.0f,0.0f);
	_user->u_halfSpace[1] = GLVector3(0.309016994f,0.951056516f,0.0f);
	_user->u_halfSpace[2] = GLVector3(-0.809016994f,0.587785252f,0.0f);
	_user->u_halfSpace[3] = GLVector3(-0.809016994f,-0.587785252f,0.0f);
	_user->u_halfSpace[4] = GLVector3(0.309016994f,-0.951056516f,0.0f);
	_user->u_borderWidth = 0.05f;
	_user->u_stripeWidth = 0.1f;
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
	_user->vSphere->bindNormalObject(1);
//
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
	glUniform1f(_user->u_specularFactorLoc, _user->u_specularFactor);
//条纹
	glUniform3fv(_user->u_pentacleColorLoc, 1, &_user->u_pentacleColor.x);
	glUniform3fv(_user->u_sphereColorLoc, 1, &_user->u_sphereColor.x);
	glUniform1f(_user->u_borderWidthLoc, _user->u_borderWidth);
//	glUniform1f(_user->u_stripeWidthLoc,_user->u_stripeWidth);
//半空间
	GLVector3    halfSpace[5];
	for (int i = 0; i < 5; ++i)
		halfSpace[i] = _user->u_halfSpace[i]* _user->u_normalMatrix;
	glUniform3fv(_user->u_halfSpaceLoc, 5, &halfSpace[0].x);
//画图
	_user->vSphere->drawShape();
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->object->release();
	_user->vSphere->release();
}
///////////////////////////don not modify below function////////////////////////////////////
