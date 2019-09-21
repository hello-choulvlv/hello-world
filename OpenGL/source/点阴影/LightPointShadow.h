/*
  *点光源阴影,注意再使用这个类的时候,需要在GLState.h中手动打开几何着色器宏
  *2016-10-25 20:21:33
  *@Author:小花熊
 */
#include<engine/GLObject.h>
#include<engine/Geometry.h>
#include<engine/GLProgram.h>

class    LightPointShadow :public GLObject
{
public:
	GLProgram           *_shadowProgram;
	unsigned                 _framebufferId;
#ifdef  __GEOMETRY_SHADOW__
	unsigned                 _depthTextureId;
	unsigned                 _modelMatrixLoc;
	unsigned                 _viewProjMatrixLoc[6];
	unsigned                 _lightPositionLoc;
	unsigned                 _maxDistanceLoc;
#else
	unsigned                 _depthTextureId;
	unsigned                 _cubeMapId;
	unsigned                 _mvpMatrixLoc;
	unsigned                 _modelMatrixLoc;
	unsigned                 _lightPositionLoc;
	Matrix                    _viewMatrix;
#endif
public:
	LightPointShadow();
	~LightPointShadow();
#ifndef __GEOMETRY_SHADOW__
//将深度信息写入到第几个面
	void              writeFace(int   index, GLVector3  & _lightPosition);
//将立方体纹理绑定到某个单元
	void              bindTextureUnit(int  _texture_unit);
//加载视图矩阵
	Matrix         &loadViewMatrix();
#endif
};