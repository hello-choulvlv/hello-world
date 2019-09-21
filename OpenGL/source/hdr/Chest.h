/*
  *用于hdr的物体
  *2016-9-20 17:58:43
  *@Author:小花熊
 */
//注意在大量使用这个类的时候一定要在GLState.h里面开启程序对象缓存功能
#ifndef  __CHEST_H__
#define __CHEST_H__
#include<GL/glew.h>
#include<engine/GLProgram.h>
#include<engine/GLTexture.h>
#include<engine/Shape.h>
#include<engine/Geometry.h>
class    Chest :public  GLObject
{
private:
	GLProgram         *_glProgram;
	GLChest	             *_chestShape;
	GLTexture           *_texture;
//基本属性
	float                       _scaleX, _scaleY, _scaleZ;
	GLVector3            _position;
//绕给定的轴旋转
	Matrix                  _rotateMatrix;
//model view 矩阵
	Matrix                 _modelViewMatrix;
	Matrix                 _mvpMatrix;
//法线矩阵
	Matrix3               _normalMatrix;
//光线的位置
	GLVector3           _lightPosition;
//光线的颜色
	GLVector3          _lightColor;
//眼睛的位置
	GLVector3          _eyePosition;
//环境光强度
	GLVector3          _ambientColor;
//反射系数
	float                    _specularFactor;
//程序对象统一变量的位置
	unsigned            _mvpMatrixLoc;
	unsigned            _modelViewMatrixLoc;
	unsigned            _normalMatrixLoc;
	unsigned            _baseMapLoc;
	unsigned            _lightPositionLoc;
	unsigned            _lightColorLoc;
	unsigned            _eyePositionLoc;
	unsigned            _ambientColorLoc;
	unsigned            _specularFactorLoc;
	float                   _angle;
private:
	Chest();
	Chest(Chest &);
	void                   initChest(const  char   *file_name);
public:
	~Chest();
	static          Chest            *createChest(const  char   *file_name);
//设置箱子的位置
	void             setPosition(GLVector3   &);
//设置缩放比例
	void             setScale(float      scaleX,float   scaleY,float    scaleZ);
	void             setScale(float      scaleXYZ);
//设置绕某一向量旋转角度
	void             setRotateAngle(float   angle,GLVector3  &);
//设置光源的位置
	void             setLightPosition(GLVector3  &);
//设置光源的颜色
	void             setLightColor(GLVector3   &);
//设置环境光的颜色
	void             setAmbientColor(GLVector3  &);
//设置眼睛的位置
	void             setEyePosition(GLVector3   &);
//设置反射系数
	void             setSpecularCoeff(float  _coff);
//update函数
	void             update(float      _delta);
	void             draw(Matrix        &projectMatrix,unsigned    drawFlag);
};
#endif