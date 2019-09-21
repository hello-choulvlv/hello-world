/*
  *��ƽ���ʵ��
  *2016-12-9 19:42:27
  *@Author:С����
 */
#ifndef __CHOPPY_SIMULATE_H__
#define __CHOPPY_SIMULATE_H__
#include<engine/GLProgram.h>
#include<engine/GLTexture.h>
#include<engine/Geometry.h>
#include<engine/Shape.h>
#define		WAVE_GRID		130
#define		WAVE_SIZE		(WAVE_GRID-2)
//������
struct	  Complex 
{
		float		real;
		float		imag;
};
  class   ChoppySimulate:public GLObject
  {
  private:
//��ƽ������
	  Mesh					 *_meshWave;
//��պ�
	  GLCubeMap		*_skyboxTex;
//��ƽ��shader
	  GLProgram		*_choppyProgram;
//����
	  unsigned   		  _choppyNormalId;
//ƫ��
	  unsigned             _choppyOffsetId;
//��������
	  GLVector3					 _choppyNormal[WAVE_SIZE][WAVE_SIZE];
//����
	  GLVector3		_waveNum[WAVE_SIZE][WAVE_SIZE];
//��ƽ������,FFT�任ʹ��
	  Complex			_heightField[WAVE_SIZE][WAVE_SIZE];//�߶��ݶ�,���ڲ��˷��ȵļ���,�����ֵһ�����ɾͲ���ı�
//��̬���ɵĸ߶ȳ�
	  Complex			_choppyField[WAVE_SIZE][WAVE_SIZE];
//X-Y �����ƫ�Ƴ�
	  Complex			_deltaXField[WAVE_SIZE][WAVE_SIZE];
	  Complex			_deltaYField[WAVE_SIZE][WAVE_SIZE];
//��ɫ���ڲ�ͳһ������λ��
	  unsigned			_modelMatrixLoc;
	  unsigned			_viewProjMatrixLoc;
	  unsigned			_normalMatrixLoc;
	  unsigned			_skyboxTexLoc;
	  unsigned			_eyePositionLoc;
	  unsigned			_freshnelParamLoc;
	  unsigned			_refractRatioLoc;//����/ˮ�� ����ϵ��
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
//����shader,�Լ�����shader����
	  void            initProgram();
	  void            initNormal();
//���㲨��,�߶ȳ�
	  void            genWaveNumHeightField();
//����������
  public:
	  ChoppySimulate();
	  ~ChoppySimulate();
	  void            init();
//����߶ȳ�
	  void             updateHeightField(float _deltaTime);
//����ƫ�Ƴ�
	  void             updateOffsetField(float _deltaTime);
//����ƫ�Ƴ�

	  void             update(float		_deltaTime);
	  void             draw();
//���ø�����ֵ
	  void             setModelMatrix(Matrix  &);
	  void             setViewProjMatrix(Matrix &);
	  void             setEyePosition(GLVector3 &);
	  void             setFreshnelParam(GLVector3  &);
	  void            setWaterColor(GLVector4  &);
	  void            setRefractRatio(float  );
  };
#endif