/*
 *�ӹ��ߵĽǶ���Ⱦ����,������������
 *2016-10-21 19:41:57
 *@Author:С����
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