//
#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/TGAImage.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
//
//Common  Data  Struct
//ÒýÇæ²âÊÔ
//define   constant
//
struct    ReflectSkybox
{
	GLProgram   *reflectObject;
	GLuint             u_mvpMatrixLoc;
	GLuint             u_modelViewMatrixLoc;
	GLuint             u_eyePositionLoc;
	GLuint             u_normalMatrixLoc;
	GLuint             u_cubeMapLoc;
	float                  rotateAngle;
};
//Common  Data  Struct
struct       UserData
{
	GLProgram     *object;
	GLProgram     *skyObject;
	GLuint             skyMapId;
	GLuint             skyMapLoc;
	GLuint             baseMapLoc;
	GLuint             mvpMatrixLoc;
	GLuint             texture2DId;
	GLCube           *vCube;
	GLSkybox       *vSkybox;
	GLSphere        *vSphere;
	float                 roateAngle;
	ReflectSkybox   vReflect;
};

void        Init(GLContext    *_context)
{
	_context->userObject = new   UserData();
	UserData      *_user = (UserData *)_context->userObject;
	const   char   *skyImage[6] = { "tga/skybox/left.bmp", "tga/skybox/right.tga", "tga/skybox/top.tga",
		"tga/skybox/bottom.tga", "tga/skybox/front.tga", "tga/skybox/back.tga" };
	TGAImageCubeMap      _cubeMap(skyImage);
	_user->skyMapId = _cubeMap.genTextureCubeMap();

	TGAImage     _texture("tga/skybox/left.tga");
	_user->texture2DId = _texture.genTextureMap();

	_user->skyObject = GLProgram::createWithFile("shader/skybox/Skybox.vsh", "shader/skybox/Skybox.fsh");
	_user->skyMapLoc=_user->skyObject->getUniformLocation("u_cubeMap");
	_user->mvpMatrixLoc = _user->skyObject->getUniformLocation("u_mvpMatrix");
	_user->baseMapLoc = _user->skyObject->getUniformLocation("u_baseMap");
	_user->vCube = GLCube::createWithScale(0.5f);
	_user->vSkybox = GLSkybox::createWithScale(1.0f);
	_user->vSphere=GLSphere::createWithSlice(128,0.5f);
	_user->roateAngle=0.0f;
//
	ReflectSkybox    *_reflect=&_user->vReflect;
	_reflect->reflectObject=GLProgram::createWithFile("shader/skybox/Reflect.vsh","shader/skybox/Reflect.fsh");
	_reflect->u_mvpMatrixLoc=_reflect->reflectObject->getUniformLocation("u_mvpMatrix");
	_reflect->u_modelViewMatrixLoc=_reflect->reflectObject->getUniformLocation("u_modelViewMatrix");
	_reflect->u_normalMatrixLoc=_reflect->reflectObject->getUniformLocation("u_normalMatrix");
	_reflect->u_eyePositionLoc=_reflect->reflectObject->getUniformLocation("u_eyePosition");
	_reflect->u_cubeMapLoc=_reflect->reflectObject->getUniformLocation("u_cubeMap");
	_reflect->rotateAngle=0.0f;
//
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->roateAngle+=_deltaTime*6.0f;
	if(_user->roateAngle>=360.0f)
		_user->roateAngle-=360.0f;
	_user->vReflect.rotateAngle+=16.0f*_deltaTime;
	if(_user->vReflect.rotateAngle>=360.0f)
		_user->vReflect.rotateAngle-=360.0f;
}
void         DrawCube(GLContext   *_context)
{
	UserData  *_user=(UserData *)_context->userObject;
	ReflectSkybox		*_reflect=&_user->vReflect;
	_reflect->reflectObject->enableObject();
//	_user->vCube->bindVertexObject(0);
//	_user->vCube->bindNormalObject(1);
	_user->vSphere->bindVertexObject(0);
	_user->vSphere->bindNormalObject(1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP,_user->skyMapId);
	glUniform1i(_reflect->u_cubeMapLoc,1);
	Size  _size=_context->getWinSize();
//Matrix
	ESMatrix    identity,viewMatrix;
	ESMatrix3  normalMatrix;  
	esRotate(&identity,_reflect->rotateAngle,0.0f,1.0f,0.0f);

	esMatrixLookAt(&viewMatrix,&GLVector3(0.0f,0.0f,4.0f),&GLVector3(0.0f,0.0f,0.0f),&GLVector3(0.0f,1.0f,0.0f));
	esMatrixMultiply(&identity,&identity,&viewMatrix);

	esMatrixNormal(&normalMatrix,&identity);
	glUniformMatrix3fv(_reflect->u_normalMatrixLoc,1,GL_FALSE,(float*)normalMatrix.mat3);
	glUniformMatrix4fv(_reflect->u_modelViewMatrixLoc,1,GL_FALSE,(float*)identity.m);

	esPerspective(&identity,35.0f,_size.width/_size.height,0.1f,100.0f);
	glUniformMatrix4fv(_reflect->u_mvpMatrixLoc,1,GL_FALSE,(float*)identity.m);
//ÑÛ¾¦×ø±ê
	glUniform3fv(_reflect->u_eyePositionLoc,1,(float*)&GLVector3(0.0f,0.0f,4.0f));

//	_user->vCube->drawShape();
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	_user->vSphere->drawShape();
}
void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//very important
	_user->skyObject->enableObject();
	_user->vSkybox->bindVertexObject(0);
	_user->vSkybox->bindTexCoordObject(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _user->skyMapId);
//	glBindTexture(GL_TEXTURE_2D,_user->texture2DId);
	glUniform1i(_user->skyMapLoc,0);
//	glUniform1i(_user->baseMapLoc, 0);

	ESMatrix      identity;
	ESMatrix      view;
//	esRotate(&identity,_user->roateAngle,0.0f,1.0f,0.0f);
	esMatrixLookAt(&view, &GLVector3(0.0f, 0.0f, 0.0f), &GLVector3(0.0f,0.0f,-1.0f),&GLVector3(0.0f,1.0f,0.0f));
	esMatrixMultiply(&identity, &identity, &view);
	Size   _size=_context->getWinSize();
	esPerspective(&identity,35.0f, _size.width/_size.height, 0.1f, 100.0f);
	glUniformMatrix4fv(_user->mvpMatrixLoc, 1, GL_FALSE, (float*)identity.m);
	glDepthMask(GL_FALSE);
	_user->vSkybox->drawShape();
	glDepthMask(GL_TRUE);
	DrawCube(_context);
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
}
///////////////////////////don not modify below function////////////////////////////////////
