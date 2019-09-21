//
#include <GL/glew.h>
#include<GL/freeglut.h>
#include<engine/GLContext.h>
#include<engine/TGAImage.h>
#include<engine/GLProgram.h>
#include<engine/glState.h>
#include<engine/ESMatrix.h>
//#include"sphere.h"
//define   constant
#define      ATTR_POSITION            0
#define      ATTR_TEXCOORD          1
#define      ATTR_NORMAL              2
//
//Common  Data  Struct
struct       UserData
{
	GLProgram         *cubeMapObject;
	GLProgram         *reflectObject;//
	GLuint                 cubeMapId;
	GLuint                 cubeMapLoc;
//
	GLuint                 baseMapLoc;
	GLuint                 positionVBO;
	GLuint                 normalVBO;
	GLuint                 indiceVBO;
	int                        numberOfIndice2;
//
	GLuint                 vertexBufferId;
	GLuint                 texCoordBufferId;
	GLuint                 normalBufferId;
	GLuint                 indiceBufferId;
	int                        numberOfIndice;
};

void        Init(GLContext    *_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	_user->cubeMapObject = GLProgram::createWithFile("shader/chapter-7/CubeMap.vsh", "shader/chapter-7/CubeMap.fsh");
	_user->reflectObject=GLProgram::createWithFile("shader/chapter-7/reflect.vsh","shader/chapter-7/reflect.fsh");
	_user->baseMapLoc=_user->reflectObject->getUniformLocation("u_baseMap");
	const char *imageName[6] = {
		"tga/neg_x.tga", "tga/pos_x.tga",
		"tga/neg_y.tga","tga/pos_y.tga",
		"tga/neg_z.tga", "tga/pos_z.tga"
	            };
	TGAImageCubeMap    cubeMap(imageName);
	_user->cubeMapId = cubeMap.genTextureCubeMap();
	_user->cubeMapLoc = _user->cubeMapObject->getUniformLocation("u_cubeMap");

	float        *position,*texCoord,*normal;
	int            numberOfVertex, numberOfIndice,*indice;
	numberOfIndice=esGenCube(2.0f, &position, &normal, &texCoord, &indice, &numberOfVertex);
	_user->numberOfIndice = numberOfIndice;

	glGenBuffers(1, &_user->vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _user->vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*numberOfVertex * 3, position, GL_STATIC_DRAW);

	glGenBuffers(1, &_user->texCoordBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _user->texCoordBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * numberOfVertex, texCoord,GL_STATIC_DRAW);

	glGenBuffers(1,&_user->normalBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _user->texCoordBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numberOfVertex, normal, GL_STATIC_DRAW);

	glGenBuffers(1, &_user->indiceBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _user->indiceBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*numberOfIndice, indice,GL_STATIC_DRAW);

	free(position);
	free(normal);
	free(texCoord);
	free(indice);
	position = normal = texCoord = NULL,indice=NULL;
//
	_user->numberOfIndice2 = esGenSphere(124,0.8f,&position,&normal,NULL,&indice,&numberOfVertex);
	glGenBuffers(1,&_user->positionVBO);
	glBindBuffer(GL_ARRAY_BUFFER,_user->positionVBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*3*numberOfVertex,position,GL_STATIC_DRAW);

	glGenBuffers(1,&_user->normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER,_user->normalVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numberOfVertex, normal, GL_STATIC_DRAW);

	glGenBuffers(1, &_user->indiceVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _user->indiceVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*_user->numberOfIndice2, indice, GL_STATIC_DRAW);

	free(position);
	free(normal);
	free(indice);
	position = NULL, normal = NULL, indice = NULL;

	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);//

}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
}
void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//_user->cubeMapObject->enableObject();
	//glBindBuffer(GL_ARRAY_BUFFER, _user->vertexBufferId);
	//glEnableVertexAttribArray(0);//position
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(float)*3,NULL);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_CUBE_MAP,_user->cubeMapId);
	//glUniform1i(_user->cubeMapLoc,0);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _user->indiceBufferId);
	//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
//
	_user->reflectObject->enableObject();
	glBindBuffer(GL_ARRAY_BUFFER, _user->positionVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, _user->normalVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _user->cubeMapId);
	glUniform1i(_user->baseMapLoc, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _user->indiceVBO);
	glDrawElements(GL_TRIANGLES, _user->numberOfIndice2, GL_UNSIGNED_INT, NULL);
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->cubeMapObject->release();
	_user->reflectObject->release();
	glDeleteTextures(1, &_user->cubeMapId);
	glDeleteBuffers(1,&_user->positionVBO);
	glDeleteBuffers(1,&_user->normalVBO);
	_user->cubeMapObject = NULL;
	_user->cubeMapId = 0;
}

void      glMain(GLContext    *_context)
{
			_context->userObject = new   UserData();
			_context->setDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
			_context->setWinSize(Size(640,480));
			_context->setWinPosition(GLVector2(220,140));
			_context->registerInitFunc(Init);
			_context->registerUpdateFunc(Update);
			_context->registerDrawFunc(Draw);
			_context->registerShutDownFunc(ShutDown);
}
