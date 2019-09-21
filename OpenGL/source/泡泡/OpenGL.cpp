//
#include <GL/glew.h>
#include<engine/GLContext.h>
#include<engine/GLProgram.h>
#include<engine/TGAImage.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<engine/Sprite.h>
#include<engine/GLState.h>
#include<time.h>
#include<stdlib.h>
//
//Common  Data  Struct
#define    SPRITE_COUNT   20
struct       UserData
{
	GLProgram     *object;
	GLuint	         u_baseMapLoc;
	GLuint             u_pointSizeLoc;
	GLuint             baseMapId;
	GLuint             u_radiusLoc;
	GLuint             u_timeIntervalLoc;

	Sprite              *vSprite;
	GLuint            feedbackVBO[2];
	int                   vSrcIndex;
	float                timeInterval;
};
struct   SpritePoint
{
	GLVector2     vPosition;
	GLVector2     vVelocity;
};
void        Init(GLContext    *_context)
{
	_context->userObject = new   UserData();
	UserData      *_user = (UserData *)_context->userObject;
//
	_user->object = GLProgram::createWithFile("shader/bubble/bubble.vsh", "shader/bubble/bubble.fsh");
	_user->u_radiusLoc = _user->object->getUniformLocation("u_radius");
	_user->u_baseMapLoc = _user->object->getUniformLocation("u_baseMap");
	_user->u_pointSizeLoc = _user->object->getUniformLocation("u_pointSize");
	_user->u_timeIntervalLoc = _user->object->getUniformLocation("u_timeInterval");

	TGAImage   _baseMap("tga/global_img_heng4.tga");
	_user->baseMapId = _baseMap.genTextureMap();

	_user->vSprite = Sprite::createWithFile("tga/global_img_heng4.tga");

//点的初始坐标,以及速度
	SpritePoint       vSprite[SPRITE_COUNT];
	srand((int)time(0));
	for (int i = 0; i < SPRITE_COUNT; ++i)
	{
		vSprite[i].vPosition = GLVector2();
		vSprite[i].vVelocity = GLVector2(rand() % 1000 / 500.0f - 1.0f,rand()%10000/5000.0f-1.0f);
	}
//gen feedback buffer
	glGenBuffers(2, _user->feedbackVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _user->feedbackVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vSprite), vSprite, GL_DYNAMIC_COPY);

	glBindBuffer(GL_ARRAY_BUFFER, _user->feedbackVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vSprite), NULL, GL_DYNAMIC_COPY);

	const char   *feedbackVaryings[2] = {"v_position","v_velocity"};
	_user->object->feedbackVaryingsWith(feedbackVaryings, 2, GL_INTERLEAVED_ATTRIBS);
	_user->vSrcIndex = 0;
	_user->timeInterval = 0.0f;
//很坑,尼玛被这个坑了半天,OpenGL与OpenGLES的区别,切记切记
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}
//
void         Update(GLContext   *_context, float   _deltaTime)
{
	UserData    *_user = (UserData *)_context->userObject;
	_user->timeInterval = _deltaTime;
}

void         Draw(GLContext	*_context)
{
	UserData      *_user = (UserData *)_context->userObject;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//very important

	Size  _size = _context->getWinSize();
	_user->object->enableObject();
	
	int    srcVBO = _user->feedbackVBO[_user->vSrcIndex];
	int    dstVBO = _user->feedbackVBO[(_user->vSrcIndex+1) %2];
	glBindBuffer(GL_ARRAY_BUFFER, srcVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SpritePoint), 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpritePoint),__offsetof(SpritePoint,vVelocity));

	glUniform1f(_user->u_pointSizeLoc, 140.0f);
	glUniform1f(_user->u_radiusLoc,140.f/_size.width);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _user->baseMapId);
	glUniform1i(_user->u_baseMapLoc, 0);

	glUniform1f(_user->u_timeIntervalLoc,_user->timeInterval);
//开始变换反馈
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,dstVBO);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, dstVBO);

	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, SPRITE_COUNT);
	glEndTransformFeedback();

	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
//
	_user->vSrcIndex = (_user->vSrcIndex + 1) %2;
//	_user->vSprite->render();
}
void         ShutDown(GLContext  *_context)
{
	UserData    *_user = (UserData *)_context->userObject;
}
 