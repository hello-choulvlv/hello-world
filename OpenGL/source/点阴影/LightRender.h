/*
 *从光线的角度渲染场景,并获得深度纹理
 *2016-10-21 19:41:57
 *@Author:小花熊
 */
#include<engine/GLObject.h>
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
class  LightRender :public  GLObject
{
public:
	GLProgram        *_glProgram;
	Matrix                _viewMatrix;
	unsigned             _framebufferId;
	unsigned             _depthTextureId;
	unsigned             _mvpMatrixLoc;
public:
	LightRender();
	~LightRender();
};