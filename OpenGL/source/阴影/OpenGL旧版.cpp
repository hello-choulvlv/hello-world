//
#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/TGAImage.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<engine/Sprite.h>
#include<engine/GLShadowMap.h>
#include<string.h>
#define _SHADOW_MAP_ 1
#define _USE_SHADOW_TEXTURE_ 1
//Common  Data  Struct
//�������
struct       ShadowScene
{
	 GLProgram    *object;
	 GLuint            u_mvpMatrixLoc;
	 GLuint            u_renderColorLoc;
};
//����
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
//��������
	  GLuint              textureGroundId;
//��������Id
	  GLuint              textureSphereId;
//����
	  GLGround        *vGround;
//��
	  GLSphere         *vSphere;
//��Ӱ����
	  GLShadowMap     *shadowMap;
	  Sprite               *vSprite;
	  ESMatrix           viewMatrix;
	  ESMatrix           projMatrix;
//
	  ESMatrix           lightSphereMatrix;
	  ESMatrix           lightGroundMatrix;

	  ESMatrix           eyeViewMatrix;
	  ESMatrix           eyeSphereMatrix;
	  ESMatrix           eyeGroundMatrix;
//���߾���
	  ESMatrix3           normalGroundMatrix;
	  ESMatrix3           normalSphereMatrix;
//���ߵķ���
	  GLVector3           lightVector;
//��ת�ĽǶ�
	  float                     angleArc;
};
//
void        SetSphereMVP(GLContext    *_context)
{
	UserData    *_user = (UserData*)_context->userObject;
	ESMatrix    identity;
////////////////////////��////////////////
	esMatrixLoadIdentity(&_user->lightSphereMatrix);
	esTranslate(&_user->lightSphereMatrix, 2.0f, 0.0f, 0.0f);
	esRotate(&_user->lightSphereMatrix, _user->angleArc, 0.0f, 1.0f, 0.0f);
	esTranslate(&_user->lightSphereMatrix, 0.0f, 0.0f, -4.0f);
	esMatrixMultiply(&_user->lightSphereMatrix, &_user->lightSphereMatrix, &_user->viewMatrix);
//��ȡ���߾���
	esMatrixNormal(&_user->normalSphereMatrix, &_user->lightSphereMatrix);
	esMatrixMultiply(&_user->lightSphereMatrix, &_user->lightSphereMatrix, &_user->projMatrix);
	////////////////�۾��ĵ���Ұ//////////////////
	esMatrixLoadIdentity(&_user->eyeSphereMatrix);
	esTranslate(&_user->eyeSphereMatrix, 2.0f, 0.0f, 0.0f);
	esRotate(&_user->eyeSphereMatrix, _user->angleArc, 0.0f, 1.0f, 0.0f);
	esTranslate(&_user->eyeSphereMatrix, 0.0f, 0.0f, -4.0f);

	esMatrixMultiply(&_user->eyeSphereMatrix, &_user->eyeSphereMatrix, &_user->eyeViewMatrix);
	esMatrixMultiply(&_user->eyeSphereMatrix, &_user->eyeSphereMatrix, &_user->projMatrix);
//��
}
//�̶�����ľ���
void        SetFixedMatrix(GLContext *_context)
{
	UserData    *_user = (UserData*)_context->userObject;
	Size   _size = _context->getWinSize();
	esPerspective(&_user->projMatrix, 50.0f, _size.width / _size.height, 1.0f, 100.0f);
	//	esOrtho(&_user->projMatrix, -4.0f, 4.0f, -4.0f, 4.0f, 0.0f, 8.0f);
	//��Դ����Ұ
	//	esMatrixLookAt(&_user->viewMatrix, &GLVector3(4.0f, 2.5f, -4.0f), &GLVector3(0.0f, 0.0f, -4.0f), &GLVector3(0.0f, 1.0f, 0.0f));
	esMatrixLookAt(&_user->viewMatrix, &GLVector3(2.3f, 2.5f, -4.0f), &GLVector3(2.0f, 0.0f, -4.0f), &GLVector3(0.0f, 1.0f, 0.0f));
	//���ߵķ���
	ESMatrix3      trunkMatrix;
	esMatrixTrunk(&trunkMatrix, &_user->viewMatrix);
	_user->lightVector = esMatrixMultiplyVector3(&GLVector3(2.3f - 2.0f, 2.5f - 0.0f, -4.0f + 4.0f), &trunkMatrix);// normalize(&GLVector3(2.3f - 2.0f, 2.5f - 0.0f, -4.0f + 4.0f));
	_user->lightVector = normalize(&_user->lightVector);
	ESMatrix    identity;
	///////////////////����//////////////////
	//��ת
	esRotate(&_user->lightGroundMatrix, -90.0f, 1.0f, 0.0f, 0.0f);
	//ƽ��
	esTranslate(&_user->lightGroundMatrix, 0.0f, -1.0f, -4.0f);
	//��ͼ����
	esMatrixMultiply(&_user->lightGroundMatrix, &_user->lightGroundMatrix, &_user->viewMatrix);
	//��ȡ���߾���
	esMatrixTrunk(&_user->normalGroundMatrix, &_user->lightGroundMatrix);
	GLVector3    rotatevector = esMatrixMultiplyVector3(&GLVector3(0.0f, 0.0f, 1.0f), &_user->normalGroundMatrix);
	esMatrixMultiply(&_user->lightGroundMatrix, &_user->lightGroundMatrix, &_user->projMatrix);
////////////////////
	//��ת
	esRotate(&_user->eyeGroundMatrix, -90.0f, 1.0f, 0.0f, 0.0f);
	//ƽ��
	esTranslate(&_user->eyeGroundMatrix, 0.0f, -1.0f, -4.0f);
	//��ͼ����
	esMatrixLookAt(&_user->eyeViewMatrix, &GLVector3(0.0f, 1.0f, 0.0f), &GLVector3(0.0f, 0.0f, -8.0f), &GLVector3(0.0f, 1.0f, 0.0f));
	esMatrixMultiply(&_user->eyeGroundMatrix, &_user->eyeGroundMatrix, &_user->eyeViewMatrix);
	//MVP����
	esMatrixMultiply(&_user->eyeGroundMatrix, &_user->eyeGroundMatrix, &_user->projMatrix);
}
void        Init(GLContext    *_context)
{
	_context->userObject = new   UserData();
	UserData      *_user = (UserData *)_context->userObject;
	ShadowScene   *_shadow=&_user->shadowScene;
	_shadow->object=GLProgram::createWithFile("shader/shadow/shadow.vsh","shader/shadow/shadow.fsh");
	_shadow->u_mvpMatrixLoc=_shadow->object->getUniformLocation("u_lightMatrix");
	_shadow->u_renderColorLoc=_shadow->object->getUniformLocation("u_renderColor");
//����ĳ���
	RealScene    *_real=&_user->realScene;
	_real->object=GLProgram::createWithFile("shader/shadow/shadow_scene.vsh","shader/shadow/shadow_scene.fsh");
	_real->u_mvpMatrixLoc=_real->object->getUniformLocation("u_mvpMatrix");
	_real->u_mvpLightMatrixLoc=_real->object->getUniformLocation("u_lightMatrix");
	_real->u_normalMatrixLoc=_real->object->getUniformLocation("u_normalMatrix");
	_real->u_baseMapLoc=_real->object->getUniformLocation("u_baseMap");
	_real->u_shadowMapLoc=_real->object->getUniformLocation("u_shadowMap");
	_real->u_lightVectorLoc = _real->object->getUniformLocation("u_lightVector");
//��Ӱ����
	Size    _size=_context->getWinSize();
	_user->shadowMap=GLShadowMap::createWithSize(_size.width,_size.height);
//����
	_user->vGround=GLGround::createWithGrid(16,5.0f);
	_user->vSphere=GLSphere::createWithSlice(128,0.5f);
	_user->vSprite=Sprite::createWithFile("tga/Earth512x256-b.tga");
//����
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
//	glEnable(GL_CULL_FACE);
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
//��Ⱦ�������
void         DrawShadow(GLContext   *_context)
{
        UserData   *_user=(UserData *)_context->userObject;
		ShadowScene  *_shadow=&_user->shadowScene;
#ifdef _SHADOW_MAP_
		glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
#endif
		_shadow->object->enableObject();
		_user->vGround->bindVertexObject(0);

		glUniformMatrix4fv(_shadow->u_mvpMatrixLoc,1,GL_FALSE,(float*)&_user->lightGroundMatrix.m);
		glUniform4fv(_shadow->u_renderColorLoc,1,(float*)&GLVector4(0.5f,0.8f,0.5f,1.0f));
		_user->vGround->drawShape();
//��
		_shadow->object->enableObject();
		_user->vSphere->bindVertexObject(0);


		glUniformMatrix4fv(_shadow->u_mvpMatrixLoc,1,GL_FALSE,(float*)_user->lightSphereMatrix.m);
		glUniform4fv(_shadow->u_renderColorLoc,1,(float*)&GLVector4(1.0f,0.0f,0.0f,1.0f));
		_user->vSphere->drawShape();

#ifdef  _SHADOW_MAP_
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
#endif
}
//��Ⱦ����
void         DrawScene(GLContext  *_context)
{
	UserData  *_user=(UserData*)_context->userObject;
//����
	RealScene	*_real=&_user->realScene;
	_real->object->enableObject();
	_user->vGround->bindVertexObject(0);
	_user->vGround->bindNormalObject(1);
	_user->vGround->bindTexCoordObject(2);
//����
	glUniform3fv(_real->u_lightVectorLoc,1,(float*)&_user->lightVector );
//���߾���
	glUniformMatrix3fv(_real->u_normalMatrixLoc,1,GL_FALSE,(float*)_user->normalGroundMatrix.mat3);
//MVP����
	glUniformMatrix4fv(_real->u_mvpMatrixLoc,1,GL_FALSE,(float*)_user->eyeGroundMatrix.m);
//��Ӱ����
	glUniformMatrix4fv(_real->u_mvpLightMatrixLoc,1,GL_FALSE,(float*)_user->lightGroundMatrix.m);
//����
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_user->textureGroundId);
	glUniform1i(_real->u_baseMapLoc,0);
//��Ӱ
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,_user->shadowMap->shadowMap());
	glUniform1i(_real->u_shadowMapLoc,1);
	_user->vGround->drawShape();
//����
	_real->object->enableObject();
	_user->vSphere->bindVertexObject(0);
	_user->vSphere->bindNormalObject(1);
	_user->vSphere->bindTexCoordObject(2);
//����
	glUniform3fv(_real->u_lightVectorLoc, 1, (float*)&_user->lightVector);
//���߾���
	glUniformMatrix3fv(_real->u_normalMatrixLoc, 1, GL_FALSE, (float*)_user->normalSphereMatrix.mat3);
//���󴫵�
	glUniformMatrix4fv(_real->u_mvpMatrixLoc,1,GL_FALSE,(float*)_user->eyeSphereMatrix.m);
	glUniformMatrix4fv(_real->u_mvpLightMatrixLoc,1,GL_FALSE,(float*)_user->lightSphereMatrix.m);
//����
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_user->textureSphereId);
	glUniform1i(_real->u_baseMapLoc,0);
//��Ӱ����
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
