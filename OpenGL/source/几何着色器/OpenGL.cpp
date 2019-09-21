//几何着色器示例
#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/TGAImage.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<engine/Sprite.h>
#include<engine/GLShadowMap.h>
#include<string.h>
#include<stdlib.h>
//几何着色器使用
#include"vbm.h"
struct       UserData
{
	GLProgram           *object;
	GLuint                  u_baseMapId;
	GLuint                  u_baseMapLoc;
	GLuint                  u_mvpMatrixLoc;
	Matrix                  u_mvpMatrix;
//球
	GLSphere			*vSphere;
	VBObject            vbObject;
};
//

void        Init(GLContext    *_context)
{
	UserData	*_user = new   UserData();
	_context->userObject = _user;

	_user->object = GLProgram::createWithFile("shader/geometry/geometry.vsh", "shader/geometry/geometry.gsh", "shader/geometry/geometry.fsh");
	_user->u_mvpMatrixLoc = _user->object->getUniformLocation("u_mvpMatrix");
	_user->u_baseMapLoc = _user->object->getUniformLocation("u_baseMap");

	_user->vbObject.LoadFromVBM("model/ninja.vbm",0,2,1);
	//TGAImage   baseMap("tga/Earth512x256.tga");
	//_user->u_baseMapId = baseMap.genTextureMap();
	glGenTextures(1, &_user->u_baseMapId);
	unsigned char * tex = (unsigned char *)new char[1024 * 1024 * 4];
	memset(tex, 0, 1024 * 1024 * 4);

	int n, m;

	for (n = 0; n < 256; n++)
	{
		for (m = 0; m < 1270; m++)
		{
			int x = rand() & 0x3FF;
			int y = rand() & 0x3FF;
			tex[(y * 1024 + x) * 4 + 0] = (rand() & 0x3F) + 0xC0;
			tex[(y * 1024 + x) * 4 + 1] = (rand() & 0x3F) + 0xC0;
			tex[(y * 1024 + x) * 4 + 2] = (rand() & 0x3F) + 0xC0;
			tex[(y * 1024 + x) * 4 + 3] = n;
		}
	}

	glBindTexture(GL_TEXTURE_2D, _user->u_baseMapId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	delete tex;

	_user->vSphere = GLSphere::createWithSlice(128, 0.6f);

//矩阵
	Size  _size = _context->getWinSize();
	_user->u_mvpMatrix.identity();
	_user->u_mvpMatrix.translate(0.0f, -80.0f, -130.0f);
//	_user->u_mvpMatrix.translate(0.0f, 0.0f, -4.0f);
//	_user->u_mvpMatrix.frustum(-1.0, 1.0, -_size.height / _size.width, _size.height / _size.width, 1.0f, 100.0f);
	_user->u_mvpMatrix.frustum(-1.0, 1.0, -_size.height / _size.width, _size.height / _size.width, 1.0f, 5000.0f);
//
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
//想要使用毛发效果需要开启颜色混溶
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	_user->object->enableObject();
	//_user->vSphere->bindVertexObject(0);
	//_user->vSphere->bindTexCoordObject(1);
	//_user->vSphere->bindNormalObject(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->u_baseMapId);
	glUniform1i(_user->u_baseMapLoc, 0);

	glUniformMatrix4fv(_user->u_mvpMatrixLoc, 1, GL_FALSE, _user->u_mvpMatrix.pointer());
//	_user->vSphere->drawShape();
   _user->vbObject.Render();
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->object->release();
	glDeleteTextures(1, &_user->u_baseMapId);
	_user->vSphere->release();
}
///////////////////////////don not modify below function////////////////////////////////////
