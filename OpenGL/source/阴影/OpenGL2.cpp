//
#include <GL/glew.h>
#include<GL/freeglut.h>
#include<engine/GLContext.h>
#include<engine/TGAImage.h>
#include<engine/GLProgram.h>
#include<engine/glState.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<engine/GLShadowMap.h>
#include<assert.h>
#include<math.h>
//#include"sphere.h"
//define   constant
#define      ATTR_POSITION            0
#define      ATTR_TEXCOORD          1
#define      ATTR_NORMAL              2
//
//Common  Data  Struct
struct       ShadowMap
{
	GLProgram     *shadowObject;
	GLuint              lightMatrixLoc;
	ESMatrix         lightGroundMatrix;
	ESMatrix         lightSphereMatrix;
};
//场景
struct       SceneMap
{
	GLProgram     *object;
	GLuint             eyeMatrixLoc;
	GLuint             lightMatrixLoc;
	GLuint              baseMapId;
	GLuint              baseMapLoc;
	GLuint              sphereMapId;

	GLuint              shadowMapId;
	GLuint              shadowMapLoc;
	ESMatrix          sceneGroundMVPMatrix;
	ESMatrix          sceneSphereMVPMatrix;
};
struct       UserData
{
//场景
	SceneMap         mSceneMap;
	ShadowMap     mShadowMap;
	GLuint               sphereMapId;
//
	GLGround        *ground;
	GLSphere         *sphere;
//Shadow
	GLShadowMap         *shadowMap;
};
//设置模型视图投影矩阵,分别从光源的角度,和眼睛的角度观察
void         InitMVP(GLContext   *_context)
{
	UserData   *_user = (UserData *)_context->userObject;
	SceneMap    *_sceneMap = &_user->mSceneMap;
	ShadowMap  *_shadowMap = &_user->mShadowMap;
	Size                 _size = _context->getWinSize();

	ESMatrix      modelMatrix, viewMatrix, translateMatrix,projMatrix, modeViewMatrix;
	ESMatrix      translateMatrix2,modelMatrix2,modelViewMatrix2;
//创建眼睛中视野 MVP矩阵
	esScale(&modelMatrix, 10.0f, 10.0f, 10.0f);
	esRotate(&modelMatrix, -90.0f, 1.0f, 0.0f, 0.0f);
	esTranslate(&translateMatrix, 0.0f, -0.9f, -5.0f);
	esTranslate(&translateMatrix2, 2.5f, 0.5f, -5.0f);
	esMatrixMultiply(&modelMatrix, &modelMatrix, &translateMatrix);
	esMatrixMultiply(&modelMatrix2, &modelMatrix2, &translateMatrix2);
	//
	esMatrixLookAt(&viewMatrix, &GLVector3(0.0, 0.0, 1.0f), &GLVector3(0.0f, 0.0f, -5.0f), &GLVector3(0.0f, 1.0f, 0.0f));

	esPerspective(&projMatrix, 60.0f, _size.width / _size.height, 1.0f, 100.0f);
	esMatrixMultiply(&modeViewMatrix, &modelMatrix, &viewMatrix);
	esMatrixMultiply(&modelViewMatrix2, &modelMatrix2, &viewMatrix);
	esMatrixMultiply(&_sceneMap->sceneGroundMVPMatrix, &modeViewMatrix, &projMatrix);
	esMatrixMultiply(&_sceneMap->sceneSphereMVPMatrix,&modelViewMatrix2,&projMatrix);
//创建光源视野的MVP矩阵
	esMatrixLookAt(&viewMatrix, &GLVector3(4.0f, 2.0f, -5.5f), &GLVector3(2.5f, 0.5f, -5.0f), &GLVector3(0.0f, 1.0f, 0.0f));
	esMatrixMultiply(&modeViewMatrix,&modelMatrix,&viewMatrix);
	esMatrixMultiply(&_shadowMap->lightGroundMatrix, &modeViewMatrix, &projMatrix);

	esMatrixMultiply(&modelViewMatrix2, &modelMatrix2, &viewMatrix);
	esMatrixMultiply(&_shadowMap->lightSphereMatrix, &modelViewMatrix2, &projMatrix);
}
void        Init(GLContext    *_context)
{
	_context->userObject = new  UserData();
	UserData      *_user = (UserData *)_context->userObject;
//Shadow program object
	_user->mShadowMap.shadowObject = GLProgram::createWithFile("shader/Shdow/shadow.vsh", "shader/Shdow/shadow.fsh");
	_user->mShadowMap.lightMatrixLoc = _user->mShadowMap.shadowObject->getUniformLocation("u_lightMatrix");
//Scene program object
	_user->mSceneMap.object = GLProgram::createWithFile("shader/Shdow/shadow_scene.vsh", "shader/Shdow/shadow_scene.fsh");
	_user->mSceneMap.eyeMatrixLoc = _user->mSceneMap.object->getUniformLocation("u_mMatrix");
	_user->mSceneMap.lightMatrixLoc = _user->mSceneMap.object->getUniformLocation("u_lightMatrix");
	_user->mSceneMap.baseMapLoc = _user->mSceneMap.object->getUniformLocation("u_baseMap");
	_user->mSceneMap.shadowMapLoc = _user->mSceneMap.object->getUniformLocation("u_shadowMap");

	TGAImage      baseMap("tga/map/ground.tga");
	_user->mSceneMap.baseMapId = baseMap.genTextureMap();

	TGAImage      sphereMap("tga/Earth512x256.tga");
	_user->mSceneMap.sphereMapId = sphereMap.genTextureMap();

	_user->ground = GLGround::createWithGrid(8,1.0f);
	_user->sphere = GLSphere::createWithSlice(32, 1.0f);
//
	Size   _size = _context->getWinSize();
	_user->shadowMap = GLShadowMap::createWithSize(_size.width, _size.height);
	InitMVP(_context);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
}
//
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;

}
//阴影
void         DrawShadow(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	ShadowMap    *_shadowMap = &_user->mShadowMap;

	int     _defaultFramebufferId;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &_defaultFramebufferId);
	_user->shadowMap->bindFramebuffer();
	glClear(GL_DEPTH_BUFFER_BIT);
	_shadowMap->shadowObject->enableObject();
	glEnable(GL_POLYGON_OFFSET_FILL);
//	glPolygonOffset(5.0f, 100.0f);
//disable color render
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	_user->ground->bindVertexObject(0);
	glUniformMatrix4fv(_shadowMap->lightMatrixLoc, 1, GL_FALSE, (float*)_shadowMap->lightGroundMatrix.m);
	_user->ground->drawShape();
//draw sphere
	_shadowMap->shadowObject->enableObject();
	_user->sphere->bindVertexObject(0);
	glUniformMatrix4fv(_shadowMap->lightMatrixLoc, 1, GL_FALSE, (float*)_shadowMap->lightSphereMatrix.m);
	_user->sphere->drawShape();
//	glDisable(GL_POLYGON_OFFSET_FILL);
//resume
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _defaultFramebufferId);
}
//画场景
void        DrawScene(GLContext   *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	SceneMap   *_sceneMap = &_user->mSceneMap;
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//
	_sceneMap->object->enableObject();
	_user->ground->bindVertexObject(0);
	_user->ground->bindTexCoordObject(1);
	glUniformMatrix4fv(_sceneMap->eyeMatrixLoc, 1, GL_FALSE, (float*)_sceneMap->sceneGroundMVPMatrix.m);
	glUniformMatrix4fv(_sceneMap->lightMatrixLoc, 1, GL_FALSE, (float*)_user->mShadowMap.lightGroundMatrix.m);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _sceneMap->baseMapId);
	glUniform1i(_sceneMap->baseMapLoc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _user->shadowMap->shadowMap());
	glUniform1i(_sceneMap->shadowMapLoc, 1);

	_user->ground->drawShape();
//draw sphere
	_sceneMap->object->enableObject();
	_user->sphere->bindVertexObject(0);
	_user->sphere->bindTexCoordObject(1);
	glUniformMatrix4fv(_sceneMap->eyeMatrixLoc, 1, GL_FALSE, (float*)_sceneMap->sceneSphereMVPMatrix.m);
	glUniformMatrix4fv(_sceneMap->lightMatrixLoc, 1, GL_FALSE, (float*)_user->mShadowMap.lightSphereMatrix.m);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _sceneMap->sphereMapId);
	glUniform1i(_sceneMap->baseMapLoc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _user->shadowMap->shadowMap());
	glUniform1i(_sceneMap->shadowMapLoc, 1);

	_user->sphere->drawShape();
}
void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	DrawShadow(_context);
	DrawScene(_context);
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->mSceneMap.object->release();
	_user->mShadowMap.shadowObject->release();
	_user->ground->release();
	_user->sphere->release();
	_user->shadowMap->release();
}
