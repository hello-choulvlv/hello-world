#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/TGAImage.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<engine/TGAImage.h>
//环境映射
struct       Reflect
{
	GLProgram         *object;
	unsigned              u_mvpMatrixLoc;
	unsigned              u_modelMatrixLoc;
	unsigned              u_normalMatrixLoc;
	unsigned              u_cubeMapLoc;
	unsigned              u_eyePositionLoc;
	unsigned              u_lightColorLoc;
	unsigned              u_lightPositionLoc;
	unsigned              u_ambientColorLoc;
	unsigned              u_specularFactorLoc;
//
	Matrix                 u_mvpMatrix;
	Matrix                 u_modelMatrix;
	Matrix                 projectMatrix;
	Matrix3               u_normalMatrix;
	GLVector3          u_lightColor;
	GLVector3          u_lightPosition;
	GLVector3          u_eyePosition;
	GLVector3          u_ambientColor;
	float                    u_specularFactor;
	float                    _deltaTime;
	GLSphere		   *vSphere;
	unsigned            _cubeMapId;
public:
	Reflect(unsigned );
	~Reflect();
	void                update(float   deltaTime);
	void                draw();
};
Reflect::Reflect(unsigned   cubeMapId)
{
	object = GLProgram::createWithFile("shader/skybox/Reflect.vsh", "shader/skybox/Reflect.fsh");
	u_mvpMatrixLoc = object->getUniformLocation("u_mvpMatrix");
	u_modelMatrixLoc = object->getUniformLocation("u_modelMatrix");
	u_normalMatrixLoc = object->getUniformLocation("u_normalMatrix");
	u_cubeMapLoc = object->getUniformLocation("u_cubeMap");
	u_eyePositionLoc = object->getUniformLocation("u_eyePosition");
	u_lightPositionLoc = object->getUniformLocation("u_lightPosition");
	u_lightColorLoc = object->getUniformLocation("u_lightColor");
	u_ambientColorLoc = object->getUniformLocation("u_ambientColor");
	u_specularFactorLoc = object->getUniformLocation("u_specularFactor");
//
	projectMatrix.identity();
	projectMatrix.orthoProject(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 10.0f);
	u_lightColor = GLVector3(0.8f,0.8f,1.0f);
	u_eyePosition = GLVector3(0.0f,0.0f,0.0f);
	u_lightPosition = GLVector3(1.0f,0.0f,0.0f);
	u_ambientColor = GLVector3(0.1f,0.1f,0.1f);
	u_specularFactor = 0.36f;
	_deltaTime = 0.0f;
	vSphere = GLSphere::createWithSlice(128, 0.6f);
	_cubeMapId = cubeMapId;
}
Reflect::~Reflect()
{
	object->release();
	vSphere->release();
}
void          Reflect::update(float  deltaTime)
{
	_deltaTime += deltaTime*12.0f;
	if (_deltaTime > 360.0f)
		_deltaTime -= 360.0f;
	u_mvpMatrix.identity();
	u_mvpMatrix.rotate(-17.0f, 1.0f, 0.0f, 0.0f); 
	u_mvpMatrix.rotate(_deltaTime, 0.0f, 1.0f, 0.0f);
	u_mvpMatrix.translate(0.0f, 0.0f, -4.0f);
//从以上矩阵中导出模型矩阵和法线矩阵
	u_modelMatrix = u_mvpMatrix;
	u_normalMatrix = u_modelMatrix.normalMatrix();
	u_mvpMatrix.multiply(projectMatrix);
}
void           Reflect::draw()
{
	object->enableObject();
	vSphere->bindVertexObject(0);
	vSphere->bindNormalObject(1);

	glUniformMatrix4fv(u_mvpMatrixLoc, 1, GL_FALSE, u_mvpMatrix.pointer());
	glUniformMatrix4fv(u_modelMatrixLoc, 1, GL_FALSE, u_modelMatrix.pointer());
	glUniformMatrix3fv(u_normalMatrixLoc, 1, GL_FALSE, u_normalMatrix.pointer());
//环境纹理
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _cubeMapId);
	glUniform1i(u_cubeMapLoc,0);
//关于光线
	glUniform3fv(u_lightPositionLoc, 1, &u_lightPosition.x);
	glUniform3fv(u_lightColorLoc, 1, &u_lightColor.x);
	glUniform3fv(u_ambientColorLoc, 1, &u_ambientColor.x);
	glUniform3fv(u_eyePositionLoc, 1, &u_eyePosition.x);
	glUniform1f(u_specularFactorLoc,u_specularFactor);
//画图
	vSphere->drawShape();
}
struct       UserData
{
	GLProgram	*object;
//纹理
	unsigned          u_cubeMapId;
	unsigned          u_cubeMapLoc;
	unsigned          u_mvpMatrixLoc;
	unsigned          u_modelMatrix;
	unsigned          u_normalMatrix;
//光线
	float                u_specularFactor;
	Matrix            u_mvpMatrix;
	Matrix            projectionMatrix;
	Matrix            viewMatrix;
//
	GLSphere		*vSphere;
	GLSkybox		*vSkybox;
	float                deltaAngle;
	Reflect            *_reflect;
};
//

void        Init(GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;
	_user->object = GLProgram::createWithFile("shader/skybox/Skybox.vsh", "shader/skybox/Skybox.fsh");
//Vertex Shader
	_user->u_mvpMatrixLoc = _user->object->getUniformLocation("u_mvpMatrix");
	_user->u_cubeMapLoc = _user->object->getUniformLocation("u_cubeMap");
//
	const    char    *vSkybox_name[6] = { "tga/skybox/skybox_left.tga", //-X
		"tga/skybox/skybox_right.tga",//+X
		"tga/skybox/skybox_bottom.tga",//-Y
		"tga/skybox/skybox_top.tga",//+Y
		"tga/skybox/skybox_front.tga",//-Z
		"tga/skybox/skybox_back.tga",//+Z
	};
	TGAImageCubeMap    _cubeMap(vSkybox_name);
	_user->u_cubeMapId = _cubeMap.genTextureCubeMap();
//
	_user->vSkybox = GLSkybox::createWithScale(1.0f);
	_user->u_mvpMatrix.identity();
//	_user->u_mvpMatrix.translate(0.0f, 0.0f, -4.0f);
	_user->deltaAngle = 0.0f;

	Size    _size = _context->getWinSize();
	_user->projectionMatrix.identity();
	_user->projectionMatrix.perspective(45.0f, _size.width/_size.height,0.1f,100.0f);
	_user->viewMatrix.identity();
	_user->viewMatrix.lookAt(GLVector3(0.0f,0.0f,0.0f),GLVector3(0.0f,0.0f,-1.0f),GLVector3(0.0f,1.0f,0.0f));
//
	_user->_reflect = new   Reflect(_user->u_cubeMapId);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->deltaAngle += _deltaTime*12.0f;
	if (_user->deltaAngle >= 360.0f)
		_user->deltaAngle -= 360.0f;
	_user->u_mvpMatrix.identity();
	_user->u_mvpMatrix.rotate(_user->deltaAngle, 0.0f, 1.0f, 0.0f);
//	_user->u_mvpMatrix.translate(0.0f, 0.0f, -4.0f);
	_user->_reflect->update(_deltaTime);
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	_user->object->enableObject();
	_user->vSkybox->bindVertexObject(0);
	_user->vSkybox->bindTexCoordObject(1);
//
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _user->u_cubeMapId);
	glUniform1i(_user->u_cubeMapLoc,0);
//
	_user->u_mvpMatrix.multiply(_user->viewMatrix);
	_user->u_mvpMatrix.multiply(_user->projectionMatrix);
	glUniformMatrix4fv(_user->u_mvpMatrixLoc, 1, GL_FALSE, _user->u_mvpMatrix.pointer());
//
//画图
	_user->vSkybox->drawShape();
	_user->_reflect->draw();
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->object->release();
	_user->vSkybox->release();
	delete     _user->_reflect;
//	_user->vSphere->release();
}
///////////////////////////don not modify below function////////////////////////////////////
