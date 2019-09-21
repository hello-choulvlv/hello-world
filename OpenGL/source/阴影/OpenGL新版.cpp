//使用新的矩阵算法实现
#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/TGAImage.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<engine/Sprite.h>
#include<engine/GLShadowMap.h>
#define _SHADOW_MAP_ 1
#define _USE_SHADOW_TEXTURE_ 1
//Common  Data  Struct
//引擎测试
struct       ShadowScene
{
	 GLProgram    *object;
	 GLuint            u_mvpMatrixLoc;
	 GLuint            u_renderColorLoc;
};
//场景
struct       RealScene
{
      GLProgram    *object;
	  GLuint            u_mvpLightMatrixLoc;
	  GLuint            u_mvpMatrixLoc;
	  GLuint            u_normalMatrixLoc;
	  GLuint            u_baseMapLoc;
	  GLuint            u_shadowMapLoc;
	  GLuint            u_lightVectorLoc;
};
struct       UserData
{
      ShadowScene	shadowScene;
	  RealScene         realScene;
//地面纹理
	  GLuint              textureGroundId;
//球面纹理Id
	  GLuint              textureSphereId;
//地面
	  GLGround        *vGround;
//球
	  GLSphere         *vSphere;
//阴影纹理
	  GLShadowMap     *shadowMap;
	  Sprite               *vSprite;
	  Matrix           viewMatrix;
	  Matrix           projMatrix;
//
	  Matrix               lightSphereMatrix;
//	  ESMatrix          lightSphereMatrix;
	  Matrix           lightGroundMatrix;

	  Matrix           eyeViewMatrix;
	  Matrix           eyeSphereMatrix;
	  Matrix           eyeGroundMatrix;
//法线矩阵
	  Matrix3           normalGroundMatrix;
	  Matrix3           normalSphereMatrix;
//光线的方向
	  GLVector3           lightVector;
//旋转的角度
	  float                     angleArc;
};
//
void        SetSphereMVP(GLContext    *_context)
{
	UserData    *_user = (UserData*)_context->userObject;
////////////////////////球////////////////
		_user->lightSphereMatrix.identity();
		_user->lightSphereMatrix.translate(2.0f, 0.0f, 0.0f);
		_user->lightSphereMatrix.rotate(_user->angleArc, 0.0f, 1.0f, 0.0f);
		_user->lightSphereMatrix.translate(0.0f, 0.5f, -5.0f);
		_user->lightSphereMatrix.multiply(_user->viewMatrix);
//获取法线矩阵
	_user->normalSphereMatrix=_user->lightSphereMatrix.normalMatrix();
	_user->lightSphereMatrix.multiply(_user->projMatrix);
	////////////////眼睛的的视野//////////////////
	_user->eyeSphereMatrix.identity();
	_user->eyeSphereMatrix.translate(2.0f, 0.0f, 0.0f);
	_user->eyeSphereMatrix.rotate( _user->angleArc, 0.0f, 1.0f, 0.0f);
	_user->eyeSphereMatrix.translate( 0.0f, 0.5f, -5.0f);

	_user->eyeSphereMatrix.multiply(_user->eyeViewMatrix);
	_user->eyeSphereMatrix.multiply(_user->projMatrix);
//球
}
//固定不变的矩阵
void        SetFixedMatrix(GLContext *_context)
{
	UserData    *_user = (UserData*)_context->userObject;
	Size   _size = _context->getWinSize();
	_user->projMatrix.perspective( 60.0f, _size.width / _size.height, 1.0f, 100.0f);
	//	esOrtho(&_user->projMatrix, -4.0f, 4.0f, -4.0f, 4.0f, 0.0f, 8.0f);
	//光源的视野
	//	esMatrixLookAt(&_user->viewMatrix, &GLVector3(4.0f, 2.5f, -4.0f), &GLVector3(0.0f, 0.0f, -4.0f), &GLVector3(0.0f, 1.0f, 0.0f));
	GLVector3   eyeVector(2.5f, 2.5f, -5.5f);
	GLVector3   targetVector(2.0f, 0.0f, -5.0f);
	_user->viewMatrix.lookAt(eyeVector, targetVector, GLVector3(0.0f, 1.0f, 0.0f));
	//光线的方向
	Matrix3      trunkMatrix = _user->viewMatrix.trunk();
	_user->lightVector = GLVector3(eyeVector.x-targetVector.x,eyeVector.y-targetVector.y,eyeVector.z-targetVector.z)*trunkMatrix;
	_user->lightVector = normalize(&_user->lightVector);

///////////////////地面//////////////////
	//旋转
	_user->lightGroundMatrix.rotate( -90.0f, 1.0f, 0.0f, 0.0f);
	//平移
	_user->lightGroundMatrix.translate( 0.0f, -0.9f, -5.0f);
	//视图矩阵
	_user->lightGroundMatrix.multiply(_user->viewMatrix);
	//获取法线矩阵
	_user->normalGroundMatrix=_user->lightGroundMatrix.normalMatrix();
	GLVector3    rotatevector = GLVector3(0.0f, 0.0f, 1.0f)*_user->normalGroundMatrix;
	_user->lightGroundMatrix.multiply(_user->projMatrix);
///////////眼睛方向/////////
	//旋转
	_user->eyeGroundMatrix.rotate( -90.0f, 1.0f, 0.0f, 0.0f);
	//平移
	_user->eyeGroundMatrix.translate( 0.0f, -0.9f, -5.0f);
	//视图矩阵
	_user->eyeViewMatrix.lookAt(GLVector3(0.0f, 0.0f, 1.0f), GLVector3(0.0f, 0.0f, -5.0f), GLVector3(0.0f, 1.0f, 0.0f));
	_user->eyeGroundMatrix.multiply(_user->eyeViewMatrix);
	//MVP矩阵
	_user->eyeGroundMatrix.multiply(_user->projMatrix);
}
void        Init(GLContext    *_context)
{
	_context->userObject = new   UserData();
	UserData      *_user = (UserData *)_context->userObject;
	ShadowScene   *_shadow=&_user->shadowScene;
	_shadow->object=GLProgram::createWithFile("shader/shadow/shadow.vsh","shader/shadow/shadow.fsh");
	_shadow->u_mvpMatrixLoc=_shadow->object->getUniformLocation("u_lightMatrix");
	_shadow->u_renderColorLoc=_shadow->object->getUniformLocation("u_renderColor");
//世界的场景
	RealScene    *_real=&_user->realScene;
	_real->object=GLProgram::createWithFile("shader/shadow/shadow_scene.vsh","shader/shadow/shadow_scene.fsh");
	_real->u_mvpMatrixLoc=_real->object->getUniformLocation("u_mvpMatrix");
	_real->u_mvpLightMatrixLoc=_real->object->getUniformLocation("u_lightMatrix");
	_real->u_normalMatrixLoc=_real->object->getUniformLocation("u_normalMatrix");
	_real->u_baseMapLoc=_real->object->getUniformLocation("u_baseMap");
	_real->u_shadowMapLoc=_real->object->getUniformLocation("u_shadowMap");
	_real->u_lightVectorLoc = _real->object->getUniformLocation("u_lightVector");
//阴影纹理
	Size    _size=_context->getWinSize();
	_user->shadowMap=GLShadowMap::createWithSize(_size.width,_size.height);
//物体
	_user->vGround=GLGround::createWithGrid(16,10.0f);
	_user->vSphere=GLSphere::createWithSlice(128,1.0f);
	_user->vSprite=Sprite::createWithFile("tga/Earth512x256-b.tga");
//纹理
	TGAImage    _baseMap("tga/map/ground.tga");
	_user->textureGroundId=_baseMap.genTextureMap();

	TGAImage    _otherMap("tga/Earth512x256-b.tga");
	_user->textureSphereId=_otherMap.genTextureMap();

	_user->angleArc = 0.0f;
/////////////////////update matrix///////////////////

/////////////////////////////////////////////
	SetFixedMatrix(_context);
	SetSphereMVP(_context);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->angleArc += _deltaTime*12.0f;
	if (_user->angleArc >= 360.0f)
		_user->angleArc -= 360.0f;
	SetSphereMVP(_context);
}
//渲染深度纹理
void         DrawShadow(GLContext   *_context)
{
        UserData   *_user=(UserData *)_context->userObject;
		ShadowScene  *_shadow=&_user->shadowScene;
#ifdef _SHADOW_MAP_
		glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
#endif
		_shadow->object->enableObject();
		_user->vGround->bindVertexObject(0);

		glUniformMatrix4fv(_shadow->u_mvpMatrixLoc,1,GL_FALSE,_user->lightGroundMatrix.pointer());
		glUniform4fv(_shadow->u_renderColorLoc,1,(float*)&GLVector4(0.5f,0.8f,0.5f,1.0f));
		_user->vGround->drawShape();
//球
		_shadow->object->enableObject();
		_user->vSphere->bindVertexObject(0);


		glUniformMatrix4fv(_shadow->u_mvpMatrixLoc,1,GL_FALSE,_user->lightSphereMatrix.pointer());
		glUniform4fv(_shadow->u_renderColorLoc,1,(float*)&GLVector4(1.0f,0.0f,0.0f,1.0f));
		_user->vSphere->drawShape();

#ifdef  _SHADOW_MAP_
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
#endif
}
//渲染场景
void         DrawScene(GLContext  *_context)
{
	UserData  *_user=(UserData*)_context->userObject;
//地面
	RealScene	*_real=&_user->realScene;
	_real->object->enableObject();
	_user->vGround->bindVertexObject(0);
	_user->vGround->bindNormalObject(1);
	_user->vGround->bindTexCoordObject(2);
//光线
	glUniform3fv(_real->u_lightVectorLoc,1,(float*)&_user->lightVector );
//法线矩阵
	glUniformMatrix3fv(_real->u_normalMatrixLoc,1,GL_FALSE,_user->normalGroundMatrix.pointer());
//MVP矩阵
	glUniformMatrix4fv(_real->u_mvpMatrixLoc,1,GL_FALSE,_user->eyeGroundMatrix.pointer());
//阴影矩阵
	glUniformMatrix4fv(_real->u_mvpLightMatrixLoc,1,GL_FALSE,_user->lightGroundMatrix.pointer());
//纹理
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_user->textureGroundId);
	glUniform1i(_real->u_baseMapLoc,0);
//阴影
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,_user->shadowMap->shadowMap());
	glUniform1i(_real->u_shadowMapLoc,1);
	_user->vGround->drawShape();
//球体
	_real->object->enableObject();
	_user->vSphere->bindVertexObject(0);
	_user->vSphere->bindNormalObject(1);
	_user->vSphere->bindTexCoordObject(2);
//光线
	glUniform3fv(_real->u_lightVectorLoc, 1, &_user->lightVector.x);
//法线矩阵
	glUniformMatrix3fv(_real->u_normalMatrixLoc, 1, GL_FALSE, _user->normalSphereMatrix.pointer());
//矩阵传递
	glUniformMatrix4fv(_real->u_mvpMatrixLoc,1,GL_FALSE,_user->eyeSphereMatrix.pointer());
	glUniformMatrix4fv(_real->u_mvpLightMatrixLoc,1,GL_FALSE,_user->lightSphereMatrix.pointer());
//纹理
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_user->textureSphereId);
	glUniform1i(_real->u_baseMapLoc,0);
//阴影纹理
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,_user->shadowMap->shadowMap());
	glUniform1i(_real->u_shadowMapLoc,1);
	_user->vSphere->drawShape();
}
void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	Size  _size = _context->getWinSize();
#ifdef  _SHADOW_MAP_
    _user->shadowMap->bindFramebuffer();
//	glViewport(0, 0, _size.width, _size.height);
#endif
	glClear(GL_DEPTH_BUFFER_BIT);//very important
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0f,4.0f);
	DrawShadow(_context);
	glDisable(GL_POLYGON_OFFSET_FILL);
//
#ifdef  _SHADOW_MAP_
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
//	glViewport(0, 0, _size.width, _size.height);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	DrawScene(_context);
#endif
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
}
///////////////////////////don not modify below function////////////////////////////////////
