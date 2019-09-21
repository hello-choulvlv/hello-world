/*
  *作为一种光源的箱子
  2016-9-22 08:43:49
  @Author:小花熊
  */
#ifndef  __LIGHT_CHEST_H__
#define  __LIGHT_CHEST_H__
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#include<engine/GLObject.h>
//在大规模使用这个类的时候,最好在GLState.h里面打开程序对象缓存的功能
class    LightChest :public GLObject
{
private:
	GLProgram            *_glProgram;//光源程序对象
	GLChest                 *_lightChestShape;
	GLVector3               _lightColor;//光源的颜色
	GLVector3               _position;//光源的位置
	float                         _scaleX, _scaleY, _scaleZ;//光源被缩放的比例
	Matrix                     _rotateMatrix;//光源的实际旋转情况记录
//model矩阵
	Matrix                     _modelMatrix;
//程序对象中统一变量的位置
	unsigned                 _mvpMatrixLoc;
	unsigned                 _lightColorLoc;
	float                         _angle;
private:
	LightChest();
	LightChest(LightChest &);
	void                       initLightChest(GLVector3   &lightColor);
public:
	~LightChest();
	static        LightChest          *createLightChest(GLVector3       &lightColor);
//设置光源的颜色
	void          setColor(GLVector3  &lightColor);
//设置光源的位置
	void          setPosition(GLVector3  &);
//设置光源的缩放比例
	void          setScale(float   scaleX,float  scaleY,float  scaleZ);
	void          setScale(float  scaleXYZ);
//绕某一个轴旋转
	void          rotate(float  angle, GLVector3 &);
//update函数
	void           update(float      _deltaTime);
//draw函数
	void           draw(Matrix   &projectMatrix,unsigned   flag);
};
#endif