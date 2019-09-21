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
#define  __HEIGHT_SCALE__			0.057f
#define  __GRAVITY_ACCEL             981.0f
#define  __neg2pow(a)                        ((((  (a)+1) & 0x1)<<1) -1)
//类型系统
typedef  GLVector3		(*WaveType)[WAVE_SIZE];

int	FFT(int, int, float *, float *);
int	FFT2D(Complex	c[WAVE_SIZE][WAVE_SIZE], int, int, int);
int	DFT(int, int, float *, float *);
int Powerof2(int n, int *m, int *twopm);
//在后面会对齐标准化
static     const    GLVector2   __windDirection= GLVector2(10.0f,14.0f);
static     const    float   __windSpeed = 120.0f;

// Generating gaussian random number with mean 0 and standard deviation 1.
float gauss()
{
	float u1 = rand() / (float)RAND_MAX;
	float u2 = rand() / (float)RAND_MAX;
	if (u1 < 1e-6f)
		u1 = 1e-6f;
	return sqrtf(-2 * logf(u1)) * cosf(2 * __MATH_PI__ * u2);
}
//每一步的含义,请参见Tessendorf的Simulating Ocean Water,Page 9
// phillips Spectrum
// K: normalized wave vector, W: wind direction, v: wind velocity, a: amplitude constant
float phillips(GLVector2 K, GLVector2 W, float v, float a, float dir_depend)
{
	// largest possible wave from constant wind of velocity v
	float l = v * v / __GRAVITY_ACCEL;
	// damp out waves with very small length w << l
	float w = l / 1000;

	float Ksqr = K.x * K.x + K.y * K.y;
	float Kcos = K.x * W.x + K.y * W.y;
	float phillips = a * expf(-1 / (l * l * Ksqr)) / (Ksqr * Ksqr * Ksqr) * (Kcos * Kcos);

	// filter out waves moving opposite to wind
	if (Kcos < 0)
		phillips *= dir_depend;

	// damp out waves with very small length w << l
	return phillips *expf(-Ksqr * w * w);
}

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
	_eyePosition = GLVector3(0.0f,WAVE_SIZE/1.2f,0.0f);
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
	const     float       waveWorldWidth = WORLD_SIZE;// GLContext::getInstance()->getWinSize();
	const     float       waveWorldHeight = WORLD_SIZE;
	const     float       otherConstant = sqrt(0.5f);
	GLVector2         wind = __windDirection;
	wind.normalize();
	GLVector2        K;
	const      float       a = 1.0f * 1e-7;//这个数字要非常小
	for (int i = 0; i < WAVE_SIZE; ++i)
	{
		K.x =  2.0f *__MATH_PI__ * (i - middleValue) / waveWorldWidth;
		for (int j = 0; j < WAVE_SIZE; ++j)
		{
			K.y = 2.0f *__MATH_PI__ *(j - middleValue) / waveWorldHeight;
			float                  sqr_v = sqrt(K.x*K.x+K.y*K.y);
//计算高度场
			const float     phillipsValue = sqr_v <= 1e-7 ? sqr_v : phillips(K, wind, __windSpeed,a, 0.07f);
			const  float    root = sqrt(phillipsValue);
			_heightField[i][j].real = otherConstant * gauss() * root;
			_heightField[i][j].imag = otherConstant * gauss() * root;

			_waveNum[i][j] = GLVector3(K.x, K.y, sqrt(__GRAVITY_ACCEL*sqr_v));
		}
	}
}

void		ChoppySimulate::updateHeightField(float _deltaTime)
{
	const   float     _actual_dim = WAVE_SIZE *0.5f;
	for (int i = 0; i < WAVE_SIZE; ++i)
	{
		for (int j = 0; j < WAVE_SIZE; ++j)
		{
			const float wkt = _waveNum[i][j].z   * this->_deltaTime;
			const float cosValue = cos(wkt);
			const float sinValue = sin(wkt);
			const int    otherX = WAVE_SIZE - i - 1;
			const int    otherY = WAVE_SIZE - j - 1;
//计算真实的高度场
			_choppyField[i][j].real =_heightField[i][j].real * cosValue  - _heightField[i][j].imag*sinValue
											+_heightField[otherX][otherY].real*cosValue -_heightField[otherX][otherY].imag *sinValue;
			_choppyField[i][j].imag = _heightField[i][j].real *sinValue + _heightField[i][j].imag*cosValue
											-_heightField[otherX][otherY].real*sinValue - _heightField[otherX][otherY].imag*cosValue;
			GLVector2   K = GLVector2(i - _actual_dim,j - _actual_dim);
			float     sqr_k = K.x*K.x + K.y* K.y;
			if (sqr_k > 12 - 12)
				sqr_k = 1.0f / sqrt(sqr_k);
			K.x *= sqr_k;
			K.y *= sqr_k;
//计算偏移场
			_deltaXField[i][j].real = _choppyField[i][j].imag*K.x ;
			_deltaXField[i][j].imag=- _choppyField[i][j].real*K.x;

			_deltaYField[i][j].real = _choppyField[i][j].imag * K.y;
			_deltaYField[i][j].imag = -_choppyField[i][j].real*K.y;
		}
	}
	FFT2D(_choppyField, WAVE_SIZE, WAVE_SIZE, -1);
	FFT2D(_deltaXField, WAVE_SIZE, WAVE_SIZE, -1);
	FFT2D(_deltaYField, WAVE_SIZE, WAVE_SIZE, -1);
//修正
	for (int i = 0; i < WAVE_SIZE;++i)
	{
		for (int j = 0; j < WAVE_SIZE; ++j)
		{
			const    float     _lamda =  __HEIGHT_SCALE__*__neg2pow(i+j);
			_choppyField[i][j].imag *= _lamda;
			_deltaXField[i][j].imag *= _lamda;
			_deltaYField[i][j].imag *= _lamda;
		}
	}
//生成法线
	const   float fix_x = 1.0f * WORLD_SIZE / WAVE_SIZE;
	const   float fix_y = 1.0f * WORLD_SIZE / WAVE_SIZE;
	for (int i = 0; i < WAVE_SIZE; ++i)
	{
		const   int       otherX = i < WAVE_SIZE - 1 ? i + 1 : 0;
		for (int j = 0; j < WAVE_SIZE; ++j)
		{
			const   int       otherY = j < WAVE_SIZE - 1 ? j +1: 0;
			_choppyNormal[i][j] = GLVector3(-(_choppyField[otherX][j].imag - _choppyField[i][j].imag),
				-(_choppyField[i][otherY].imag - _choppyField[i][j].imag),
				fix_x);
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
			nowWave->z = _choppyField[i][j].imag ;
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
static       float        __real_buffer[WAVE_SIZE];
static       float        __imag_buffer[WAVE_SIZE];
int FFT(int dir, int m, float *x, float *y)
{
	int nn, i, i1, j, k, i2, l, l1, l2;
	float c1, c2, tx, ty, t1, t2, u1, u2, z;

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
			x[i] /= (float)nn;
			y[i] /= (float)nn;
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
	if (!Powerof2(nx, &m, &twopm) || twopm != nx)
		return  0;
	for (j = 0; j<ny; j++) {
		for (i = 0; i<nx; i++) {
			__real_buffer[i] = c[i][j].real;
			__imag_buffer[i] = c[i][j].imag;
		}
		FFT(dir, m, __real_buffer, __imag_buffer);
		for (i = 0; i<nx; i++) {
			c[i][j].real = __real_buffer[i];
			c[i][j].imag = __imag_buffer[i];
		}
	}

	/* Transform the columns */

	if (!Powerof2(ny, &m, &twopm) || twopm != ny)
		return  0;
	for (i = 0; i<nx; i++) {
		for (j = 0; j<ny; j++) {
			__real_buffer[j] = c[i][j].real;
			__imag_buffer[j] = c[i][j].imag;
		}
		FFT(dir, m, __real_buffer, __imag_buffer);
		for (j = 0; j<ny; j++) {
			c[i][j].real = __real_buffer[j];
			c[i][j].imag = __imag_buffer[j];
		}
	}
	return		1;
}

/*-------------------------------------------------------------------------
Direct fourier transform
*/
int DFT(int dir, int m, float *x1, float *y1)
{
	long i, k;
	float arg;
	float cosarg, sinarg;
	float *x2 = NULL, *y2 = NULL;

	x2 = (float *)malloc(m*sizeof(float));
	y2 = (float *)malloc(m*sizeof(float));

	for (i = 0; i<m; i++) {
		x2[i] = 0;
		y2[i] = 0;
		arg = -dir * 2.0 * 3.141592654 * (float)i / (float)m;
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
