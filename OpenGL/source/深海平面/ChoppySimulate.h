/*
  *海平面的实现
  *2016-12-9 19:42:27
  *@Author:小花熊
 */
#ifndef __CHOPPY_SIMULATE_H__
#define __CHOPPY_SIMULATE_H__
#include<engine/GLProgram.h>
#include<engine/GLTexture.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#define		WAVE_GRID		130
#define		WAVE_SIZE		(WAVE_GRID-2)
//复数类
struct	  Complex 
{
		float		real;
		float		imag;
};
  class   ChoppySimulate:public GLObject
  {
  private:
//海平面网格
	  Mesh					 *_meshWave;
//天空盒
	  GLCubeMap		*_skyboxTex;
//海平面shader
	  GLProgram		*_choppyProgram;
//法线
	  unsigned   		  _choppyNormalId;
//偏移
	  unsigned             _choppyOffsetId;
//法线网格
	  GLVector3					 _choppyNormal[WAVE_SIZE][WAVE_SIZE];
//波数
	  GLVector3		_waveNum[WAVE_SIZE][WAVE_SIZE];
//海平面网格,FFT变换使用
	  Complex			_heightField[WAVE_SIZE][WAVE_SIZE];//高度梯度,用于波浪幅度的计算,这个数值一旦生成就不会改变
//动态生成的高度场
	  Complex			_choppyField[WAVE_SIZE][WAVE_SIZE];
//X-Y 方向的偏移场
	  Complex			_deltaXField[WAVE_SIZE][WAVE_SIZE];
	  Complex			_deltaYField[WAVE_SIZE][WAVE_SIZE];
//着色器内部统一变量的位置
	  unsigned			_modelMatrixLoc;
	  unsigned			_viewProjMatrixLoc;
	  unsigned			_normalMatrixLoc;
	  unsigned			_skyboxTexLoc;
	  unsigned			_eyePositionLoc;
	  unsigned			_freshnelParamLoc;
	  unsigned			_refractRatioLoc;//空气/水面 折射系数
	  unsigned			_waterColorLoc;
//
	  Matrix				_modelMatrix;
	  Matrix				_viewProjMatrix;
	  Matrix3				_normalMatrix;
	  GLVector3		_eyePosition;
	  GLVector3		_freshnelParam;
	  GLVector4		_waterColor;
	  float					_refractRatio;
	  float					_deltaTime;
//生成shader,以及设置shader变量
	  void            initProgram();
	  void            initNormal();
//计算波数,高度场
	  void            genWaveNumHeightField();
//变量的设置
  public:
	  ChoppySimulate();
	  ~ChoppySimulate();
	  void            init();
//计算高度场
	  void             updateHeightField(float _deltaTime);
//计算偏移场
	  void             updateOffsetField(float _deltaTime);
//计算偏移场

	  void             update(float		_deltaTime);
	  void             draw();
//设置各个数值
	  void             setModelMatrix(Matrix  &);
	  void             setViewProjMatrix(Matrix &);
	  void             setEyePosition(GLVector3 &);
	  void             setFreshnelParam(GLVector3  &);
	  void            setWaterColor(GLVector4  &);
	  void            setRefractRatio(float  );
  };
#endif