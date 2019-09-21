/*
  *海平面实现
  *2016-12-9 20:16:59
  *@Author:小花熊
 */
#include<GL/glew.h>
#include<engine/GLContext.h>
#include "ChoppySimulate.h"
//#include <iostream>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<stdio.h>
#include<assert.h>
#define  PHILLIPS_CONSTANT        0.00081
#define  __LAMDA_SCALE                0.23f
#define  __HEIGHT_SCALE__			0.1f
#define  __neg2pow(a)                        ((((  (a)+1) & 0x1)<<1) -1)
//类型系统
typedef  GLVector3		(*WaveType)[WAVE_SIZE];

int	FFT(int, int, double *, double *);
int	FFT2D(Complex	c[WAVE_SIZE][WAVE_SIZE], int, int, int);
int	DFT(int, int, double *, double *);
int Powerof2(int n, int *m, int *twopm);

static     const    float   __windSpeed[2] = {20.0f,30.0f};
ChoppySimulate::ChoppySimulate()
{
	_meshWave = NULL;
	_skyboxTex = NULL;
	_choppyProgram = NULL;
	_deltaTime = 0.0f;
}
ChoppySimulate::~ChoppySimulate()
{
	_meshWave->release();
	_skyboxTex->release();
	_choppyProgram->release();
}
void   ChoppySimulate::init()
{
	this->initProgram();
//高度场
	this->genWaveNumHeightField();
//法线
	this->initNormal();
}

void  ChoppySimulate::initProgram()
{
	_choppyProgram = GLProgram::createWithFile("shader/wave/choppy.vsh", "shader/wave/choppy.fsh");
	_modelMatrixLoc = _choppyProgram->getUniformLocation("u_modelMatrix");
	_viewProjMatrixLoc = _choppyProgram->getUniformLocation("u_viewProjMatrix");
	_normalMatrixLoc = _choppyProgram->getUniformLocation("u_normalMatrix");
	_skyboxTexLoc = _choppyProgram->getUniformLocation("u_skyboxTexCube");
	_eyePositionLoc = _choppyProgram->getUniformLocation("u_eyePosition");
	_freshnelParamLoc = _choppyProgram->getUniformLocation("u_freshnelParam");
	_refractRatioLoc = _choppyProgram->getUniformLocation("u_refractRatio");
	_waterColorLoc = _choppyProgram->getUniformLocation("u_waterColor");
//设置默认的值
	_modelMatrix.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
	_modelMatrix.translate(0.0f,0.0f,-WAVE_SIZE);
	_normalMatrix = _modelMatrix.normalMatrix();
//视图,透视矩阵
	_eyePosition = GLVector3(0.0f,WAVE_SIZE/1.5f,0.0f);
	_viewProjMatrix.lookAt(_eyePosition, GLVector3(0.0f,0.0f,-WAVE_SIZE/1.2f),GLVector3(0.0f,1.0f,0.0f));
	Size   _size = GLContext::getInstance()->getWinSize();
	_viewProjMatrix.perspective(60.0f, _size.width/_size.height,0.1f,500.0f);
//天空盒
	//const   char   *skyboxFile[6] = {
	//	"tga/water/choppy_sky/stormydays_rt.tga", //+X
	//	"tga/water/choppy_sky/stormydays_lf.tga",//-X
	//	"tga/water/choppy_sky/stormydays_up.tga",//+Y
	//	"tga/water/choppy_sky/stormydays_dn.tga",//-Y
	//	"tga/water/choppy_sky/stormydays_ft.tga",//+Z
	//	"tga/water/choppy_sky/stormydays_bk.tga",//-Z
	//};
	const   char   *skyboxFile[6] = {
		"tga/water/sky/xpos.bmp", //+X
		"tga/water/sky/xneg.bmp",//-X
		"tga/water/sky/ypos.bmp",//+Y
		"tga/water/sky/yneg.bmp",//-Y
		"tga/water/sky/zpos.bmp",//+Z
		"tga/water/sky/zneg.bmp",//-Z
	};
	_skyboxTex = GLCubeMap::createWithFiles(skyboxFile);
//freshnel参数
	_freshnelParam = GLVector3(0.12f, 0.88f, 2.0f);
	_waterColor = GLVector4(0.7f, 0.78f, 0.91f, 1.0f);
//光线的折射系数
	_refractRatio = 1.0f / 1.33f;
//Mesh
	_meshWave = Mesh::createWithIntensity(WAVE_SIZE, WAVE_SIZE, WAVE_SIZE, 1.0f);
}

void   ChoppySimulate::initNormal()
{
	glGenBuffers(1, &_choppyNormalId);
	glBindBuffer(GL_ARRAY_BUFFER, _choppyNormalId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLVector3)*WAVE_SIZE*WAVE_SIZE, NULL, GL_DYNAMIC_DRAW);
	
	glGenBuffers(1, &_choppyOffsetId);
	glBindBuffer(GL_ARRAY_BUFFER, _choppyOffsetId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLVector3)*WAVE_SIZE*WAVE_SIZE, NULL, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void  ChoppySimulate::genWaveNumHeightField()
{
	const     float       middleValue = WAVE_SIZE*0.5f;
//波平面的宽度,关于这个宽度的由来,可以查看 Mesh的实现
	const     float       waveWorldWidth = WAVE_SIZE*2;// GLContext::getInstance()->getWinSize();
	const     float       waveWorldHeight = WAVE_SIZE*2;
	const     float       otherConstant = sqrt(0.5f);
	for (int i = 0; i < WAVE_SIZE; ++i)
	{
		for (int j = 0; j < WAVE_SIZE; ++j)
		{
			GLVector3        *nowWave = &_waveNum[i][j];
			nowWave->x = 2.0f *__MATH_PI__ * (i - middleValue) / waveWorldWidth;
			nowWave->y = 2.0f *__MATH_PI__ *(j - middleValue) / waveWorldHeight;
			nowWave->z = sqrt(nowWave->x * nowWave->x + nowWave->y*nowWave->y);
//计算高度场
			float		work[2];
			float    k[2] = {nowWave->x,nowWave->y};
			GLContext::getInstance()->gauss(work);
			const float     phillipsValue = GLContext::getInstance()->phillips(PHILLIPS_CONSTANT,k,__windSpeed);
			const  float    root = sqrt(phillipsValue);
			_heightField[i][j].real = otherConstant * work[0] * root;
			_heightField[i][j].imag = otherConstant * work[1] * root;
		}
	}
}

void		ChoppySimulate::updateHeightField(float _deltaTime)
{
	const       int         halfWay = WAVE_SIZE/2 +1;
	for (int i = 0; i < halfWay; ++i)
	{
		for (int j = 0; j < WAVE_SIZE; ++j)
		{
			const float wkt = sqrt(_waveNum[i][j].z   *__GRAVITY_CONSTANT) * this->_deltaTime;
			const float cosValue = cos(wkt);
			const float sinValue = sin(wkt);
			const int    otherX = WAVE_SIZE - i - 1;
			const int    otherY = WAVE_SIZE - j - 1;
//计算真实的高度场
			_choppyField[i][j].real =_heightField[i][j].real * cosValue  - _heightField[i][j].imag*sinValue
											+_heightField[otherX][otherY].real*cosValue +_heightField[otherX][otherY].imag *sinValue;
			_choppyField[i][j].imag = _heightField[i][j].real *sinValue + _heightField[i][j].imag*cosValue
											-_heightField[otherX][otherY].real*sinValue + _heightField[otherX][otherY].imag*cosValue;
//计算 X-Y对称的高度场
			if (i < halfWay - 1)
			{
				_choppyField[otherX][otherY].real = _heightField[otherX][otherY].real * cosValue - _heightField[otherX][otherY].imag*sinValue
																				+ _heightField[i][j].real*cosValue + _heightField[i][j].imag *sinValue;
				_choppyField[otherX][otherY].imag = _heightField[otherX][otherY].real *sinValue + _heightField[otherX][otherY].imag*cosValue
															- _heightField[i][j].real*sinValue + _heightField[i][j].imag*cosValue;
			}
		}
	}
	this->updateOffsetField(_deltaTime);
	if (! FFT2D(_choppyField,WAVE_SIZE,WAVE_SIZE,-1))
	{
		printf("_choppyField FFT2D failed,please check.\n");
		assert(0);
	}
//修正相关的值
	for (int i = 0; i < WAVE_SIZE; ++i)
	{
		for (int j = 0; j < WAVE_SIZE; ++j)
		{
			_choppyField[i][j].real *= __neg2pow(i + j);
		}
	}
//生成法线
	const    float	fixX=2.0f;
	const	float     fixY = 2.0f;
	for (int i = 0; i < WAVE_SIZE; ++i)
	{
		const   int       otherX = i < WAVE_SIZE - 1 ? i + 1 : 0;
		for (int j = 0; j < WAVE_SIZE; ++j)
		{
			const   int       otherY = j < WAVE_SIZE - 1 ? j +1: 0;
			//GLVector3        xVec = GLVector3(fixX + _deltaXField[otherX][j].imag - _deltaXField[i][j].imag,
			//	 _deltaYField[otherX][j].imag - _deltaYField[i][j].imag,
			//	 (_choppyField[otherX][j].imag - _choppyField[i][j].imag)*__HEIGHT_SCALE__
			//	);
			//GLVector3       yVec = GLVector3(_deltaXField[i][otherY].imag - _deltaXField[i][j].imag,
			//	fixY + _deltaYField[i][otherY].imag - _deltaYField[i][j].imag,
			//	(_choppyField[i][otherY].imag - _choppyField[i][j].imag) * __HEIGHT_SCALE__
			//	);
			//GLVector3  xVec = GLVector3(fixX, _deltaYField[i][otherY].imag - _deltaYField[i][j].imag, (_choppyField[otherX][j].real - _choppyField[i][j].real) *__HEIGHT_SCALE__);
			//GLVector3  yVec = GLVector3(_deltaXField[otherX][j].imag - _deltaXField[i][j].imag, fixY, (_choppyField[i][otherY].real - _choppyField[i][j].real)*__HEIGHT_SCALE__);
			GLVector3  xVec = GLVector3(fixX, 0.0f, (_choppyField[otherX][j].real - _choppyField[i][j].real) *__HEIGHT_SCALE__);
			GLVector3  yVec = GLVector3(0.0f, fixY, (_choppyField[i][otherY].real - _choppyField[i][j].real)*__HEIGHT_SCALE__);
			_choppyNormal[i][j] = xVec.cross(yVec);
		}
	}
}

void      ChoppySimulate::updateOffsetField(float _deltaTime)
{
	//计算偏移场
	//#define    __USE_FULL_WAVE_OFFSET__
	for (int i = 0; i < WAVE_SIZE; ++i)
	{
		for (int j = 0; j < WAVE_SIZE; ++j)
		{
			if (_waveNum[i][j].z == 0.0f)
			{
				_deltaXField[i][j].real = 0.0f;
				_deltaXField[i][j].imag = 0.0f;
				_deltaYField[i][j].real = 0.0f;
				_deltaYField[i][j].imag = 0.0f;
			}
			else
			{
				//一下可以选择的方式
				const      float   klen = _waveNum[i][j].z;
				const      float   xscale = -_waveNum[i][j].x / klen;
				const      float   yscale = -_waveNum[i][j].y / klen;
#ifndef __USE_FULL_WAVE_OFFSET__
				_deltaXField[i][j].real = 0.0f;
				_deltaXField[i][j].imag = _choppyField[i][j].imag*xscale;

				_deltaYField[i][j].real = 0.0f;
				_deltaYField[i][j].imag = _choppyField[i][j].imag * yscale;
#endif
			}
		}
	}
	//#undef __USE_FULL_WAVE_OFFSET__
	//逆快速傅里叶变换
	if (!FFT2D(_deltaXField, WAVE_SIZE, WAVE_SIZE, -1))
	{
		printf("_deltaXField FFT2D error,please check\n");
		assert(0);
	}
	if (!FFT2D(_deltaYField, WAVE_SIZE, WAVE_SIZE, -1))
	{
		printf("_deltaYField FFT2D error,please check.\n");
		assert(0);
	}
	//对偏移场进行缩放
	for (int i = 0; i < WAVE_SIZE; ++i)
	{
		for (int j = 0; j < WAVE_SIZE; ++j)
		{
			const   float   scaleXY = __LAMDA_SCALE * __neg2pow(i + j);
			_deltaXField[i][j].real *= scaleXY;
			_deltaXField[i][j].imag *= scaleXY;
			_deltaYField[i][j].real *= scaleXY;
			_deltaYField[i][j].imag *= scaleXY;
		}
	}
}

void       ChoppySimulate::update(float _deltaTime)
{
	this->_deltaTime += _deltaTime;
	this->updateHeightField(_deltaTime);
//顶点偏移
	glBindBuffer(GL_ARRAY_BUFFER, _choppyOffsetId);
	WaveType			_waterVertex = (WaveType)glMapBufferRange(GL_ARRAY_BUFFER,0,sizeof(GLVector3)*WAVE_SIZE*WAVE_SIZE,GL_MAP_WRITE_BIT	);
	for (int i = 0; i < WAVE_SIZE; ++i)
	{
		for (int j = 0; j < WAVE_SIZE; ++j)
		{
			GLVector3     *nowWave = &_waterVertex[i][j];
			nowWave->x = _deltaXField[i][j].imag;
			nowWave->y = _deltaYField[i][j].imag;
			nowWave->z = _choppyField[i][j].real * __HEIGHT_SCALE__;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
//法线
	glBindBuffer(GL_ARRAY_BUFFER, _choppyNormalId);
	WaveType   _waterNormal = (WaveType)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(GLVector3)*WAVE_SIZE*WAVE_SIZE, GL_MAP_WRITE_BIT);
	memcpy(_waterNormal, _choppyNormal, sizeof(GLVector3)*WAVE_SIZE*WAVE_SIZE);
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void       ChoppySimulate::draw()
{
	_choppyProgram->perform();
//Vertex Attribute
	_meshWave->bindVertexObject(0);
//Offset
	glBindBuffer(GL_ARRAY_BUFFER, _choppyOffsetId);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
//Normal
	glBindBuffer(GL_ARRAY_BUFFER, _choppyNormalId);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
//Uniform Attribute
	glUniformMatrix4fv(_modelMatrixLoc, 1, GL_FALSE, _modelMatrix.pointer());
	glUniformMatrix4fv(_viewProjMatrixLoc, 1, GL_FALSE, _viewProjMatrix.pointer());
	glUniformMatrix3fv(_normalMatrixLoc, 1, GL_FALSE, _normalMatrix.pointer());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _skyboxTex->name());
	glUniform1i(_skyboxTexLoc, 0);

	glUniform3fv(_eyePositionLoc, 1, &_eyePosition.x);
	glUniform3fv(_freshnelParamLoc, 1, &_freshnelParam.x);
	glUniform4fv(_waterColorLoc, 1, &_waterColor.x);
	glUniform1f(_refractRatioLoc, _refractRatio);

	_meshWave->drawShape();
}

/*-------------------------------------------------------------------------
This computes an in-place complex-to-complex FFT
x and y are the real and imaginary arrays of 2^m points.
dir =  1 gives forward transform
dir = -1 gives reverse transform

Formula: forward
N-1
---
1   \          - j k 2 pi n / N
X(n) = ---   >   x(k) e                    = forward transform
N   /                                n=0..N-1
---
k=0

Formula: reverse
N-1
---
\          j k 2 pi n / N
x(n) =       >   X(k) e                    = reverse transform?
/                                n=0..N-1
---
k=0
*/
int FFT(int dir, int m, double *x, double *y)
{
	long nn, i, i1, j, k, i2, l, l1, l2;
	double c1, c2, tx, ty, t1, t2, u1, u2, z;

	/* Calculate the number of points
	nn = 1;
	for (i=0;i<m;i++)
	nn *= 2;
	*/
	nn = 1 << m;

	/* Do the bit reversal */
	i2 = nn >> 1;
	j = 0;
	for (i = 0; i<nn - 1; i++) {
		if (i < j) {
			tx = x[i];
			ty = y[i];
			x[i] = x[j];
			y[i] = y[j];
			x[j] = tx;
			y[j] = ty;
		}
		k = i2;
		while (k <= j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}

	/* Compute the FFT */
	c1 = -1.0;
	c2 = 0.0;
	l2 = 1;
	for (l = 0; l<m; l++) {
		l1 = l2;
		l2 <<= 1;
		u1 = 1.0;
		u2 = 0.0;
		for (j = 0; j<l1; j++) {
			for (i = j; i<nn; i += l2) {
				i1 = i + l1;
				t1 = u1 * x[i1] - u2 * y[i1];
				t2 = u1 * y[i1] + u2 * x[i1];
				x[i1] = x[i] - t1;
				y[i1] = y[i] - t2;
				x[i] += t1;
				y[i] += t2;
			}
			z = u1 * c1 - u2 * c2;
			u2 = u1 * c2 + u2 * c1;
			u1 = z;
		}
		c2 = sqrt((1.0 - c1) / 2.0);
		if (dir == 1)
			c2 = -c2;
		c1 = sqrt((1.0 + c1) / 2.0);
	}

	/* Scaling for forward transform */
	if (dir == 1) {
		for (i = 0; i<nn; i++) {
			x[i] /= (double)nn;
			y[i] /= (double)nn;
		}
	}

	return 1;
}

/*-------------------------------------------------------------------------
Perform a 2D FFT inplace given a complex 2D array
The direction dir, 1 for forward, -1 for reverse
The size of the array (nx,ny)
Return false if there are memory problems or
the dimensions are not powers of 2
*/
//int FFT2D(COMPLEX **c,int nx,int ny,int dir)
int FFT2D(Complex	 c[WAVE_SIZE][WAVE_SIZE], int nx, int ny, int dir)
{
	int i, j;
	int m, twopm;
	double *real, *imag;

	/* Transform the rows */
	real = (double *)malloc(nx * sizeof(double));
	imag = (double *)malloc(nx * sizeof(double));
	if (real == NULL || imag == NULL)
		return  0;
	if (!Powerof2(nx, &m, &twopm) || twopm != nx)
		return  0;
	for (j = 0; j<ny; j++) {
		for (i = 0; i<nx; i++) {
			real[i] = c[i][j].real;
			imag[i] = c[i][j].imag;
		}
		FFT(dir, m, real, imag);
		for (i = 0; i<nx; i++) {
			c[i][j].real = real[i];
			c[i][j].imag = imag[i];
		}
	}
	free(real);
	free(imag);

	/* Transform the columns */
	real = (double *)malloc(ny * sizeof(double));
	imag = (double *)malloc(ny * sizeof(double));
	if (real == NULL || imag == NULL)
		return 0;
	if (!Powerof2(ny, &m, &twopm) || twopm != ny)
		return  0;
	for (i = 0; i<nx; i++) {
		for (j = 0; j<ny; j++) {
			real[j] = c[i][j].real;
			imag[j] = c[i][j].imag;
		}
		FFT(dir, m, real, imag);
		for (j = 0; j<ny; j++) {
			c[i][j].real = real[j];
			c[i][j].imag = imag[j];
		}
	}
	free(real);
	free(imag);

	return		1;
}

/*-------------------------------------------------------------------------
Direct fourier transform
*/
int DFT(int dir, int m, double *x1, double *y1)
{
	long i, k;
	double arg;
	double cosarg, sinarg;
	double *x2 = NULL, *y2 = NULL;

	x2 = (double *)malloc(m*sizeof(double));
	y2 = (double *)malloc(m*sizeof(double));
	if (x2 == NULL || y2 == NULL)
		return  0;

	for (i = 0; i<m; i++) {
		x2[i] = 0;
		y2[i] = 0;
		arg = -dir * 2.0 * 3.141592654 * (double)i / (double)m;
		for (k = 0; k<m; k++) {
			cosarg = cos(k * arg);
			sinarg = sin(k * arg);
			x2[i] += (x1[k] * cosarg - y1[k] * sinarg);
			y2[i] += (x1[k] * sinarg + y1[k] * cosarg);
		}
	}

	/* Copy the data back */
	if (dir == 1) {
		for (i = 0; i<m; i++) {
			x1[i] = x2[i] / (double)m;
			y1[i] = y2[i] / (double)m;
		}
	}
	else {
		for (i = 0; i<m; i++) {
			x1[i] = x2[i];
			y1[i] = y2[i];
		}
	}

	free(x2);
	free(y2);
	return	1;
}

/*-------------------------------------------------------------------------
Calculate the closest but lower power of two of a number
twopm = 2**m <= n
Return TRUE if 2**m == n
*/
int Powerof2(int n, int *m, int *twopm)
{
	if (n <= 1) {
		*m = 0;
		*twopm = 1;
		return 0;
	}

	*m = 1;
	*twopm = 2;
	do {
		(*m)++;
		(*twopm) *= 2;
	} while (2 * (*twopm) <= n);

	if (*twopm != n)
		return 0;
	else
		return 1;
}
