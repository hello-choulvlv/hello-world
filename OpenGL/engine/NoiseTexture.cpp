/*
 *Version 1.0:实现最基本的噪声
 *Author 小花熊
 *2016-8-24 19:48:22
 */
#include<engine/NoiseTexture.h>
#include<GL/glew.h>
#include<math.h>
#include<assert.h>
__NS_GLK_BEGIN
#ifdef  _WIN32
#define   random       rand
#endif
/*
 * 程序需要一些基本的宏,以及一些辅助函数
 */
#define __B 0x100
#define __BM 0xff
#define __N 0x1000
#define __NP 12   /* 2^N */
#define __NM 0xfff

#define __s_curve(t) ( t * t * (3. - 2. * t) )
#define __lerp(t, a, b) ( a + t * (b - a) )
#define __setup(i,b0,b1,r0,r1)\
        t = vec[i] + __N;\
        b0 = ((int)t) & __BM;\
        b1 = (b0+1) & __BM;\
        r0 = t - (int)t;\
        r1 = r0 - 1.;
#define __at2(rx,ry) ( rx * q[0] + ry * q[1] )
#define __at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )
//以下为辅助函数

static int p[__B + __B + 2];
static double g3[__B + __B + 2][3];
static double g2[__B + __B + 2][2];
static double g1[__B + __B + 2];
static int start = 1;
////////////////////静态函数//////////////////
static   void     __noise_init(void);
static   double __noise_noise1(double);
static   double __noise_noise2(double *);
static   double __noise_noise3(double *);
static   void     __noise_normalize3(double *);
static   void     __noise_normalize2(double *);

static    double __noise_perlin_noise1D(double, double, double, int);
static    double __noise_perlin_noise2D(double, double, double, double, int);
static    double __noise_perlin_noise3D(double, double, double, double, double, int);
///////////////////////////////////////////////
double  __noise_noise1(double arg)
{
	int bx0, bx1;
	double rx0, rx1, sx, t, u, v, vec[1];

	vec[0] = arg;
	//if (start) {
	//	start = 0;
	//	__noise_init();
	//}

	__setup(0, bx0, bx1, rx0, rx1);

	sx = __s_curve(rx0);
	u = rx0 * g1[p[bx0]];
	v = rx1 * g1[p[bx1]];

	return(__lerp(sx, u, v));
}

double __noise_noise2(double vec[2])
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	double rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	int i, j;

	//if (start) {
	//	start = 0;
	//	__noise_init();
	//}

	__setup(0, bx0, bx1, rx0, rx1);
	__setup(1, by0, by1, ry0, ry1);

	i = p[bx0];
	j = p[bx1];

	b00 = p[i + by0];
	b10 = p[j + by0];
	b01 = p[i + by1];
	b11 = p[j + by1];

	sx = __s_curve(rx0);
	sy = __s_curve(ry0);

	q = g2[b00]; u = __at2(rx0, ry0);
	q = g2[b10]; v = __at2(rx1, ry0);
	a = __lerp(sx, u, v);

	q = g2[b01]; u = __at2(rx0, ry1);
	q = g2[b11]; v =__at2(rx1, ry1);
	b = __lerp(sx, u, v);

	return __lerp(sy, a, b);
}

double __noise_noise3(double vec[3])
{
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	double rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
	int i, j;

	//if (start) {
	//	start = 0;
	//	__noise_init();
	//}

	__setup(0, bx0, bx1, rx0, rx1);
	__setup(1, by0, by1, ry0, ry1);
	__setup(2, bz0, bz1, rz0, rz1);

	i = p[bx0];
	j = p[bx1];

	b00 = p[i + by0];
	b10 = p[j + by0];
	b01 = p[i + by1];
	b11 = p[j + by1];

	t = __s_curve(rx0);
	sy = __s_curve(ry0);
	sz = __s_curve(rz0);

	q = g3[b00 + bz0]; u = __at3(rx0, ry0, rz0);
	q = g3[b10 + bz0]; v = __at3(rx1, ry0, rz0);
	a = __lerp(t, u, v);

	q = g3[b01 + bz0]; u = __at3(rx0, ry1, rz0);
	q = g3[b11 + bz0]; v = __at3(rx1, ry1, rz0);
	b = __lerp(t, u, v);

	c = __lerp(sy, a, b);

	q = g3[b00 + bz1]; u = __at3(rx0, ry0, rz1);
	q = g3[b10 + bz1]; v = __at3(rx1, ry0, rz1);
	a = __lerp(t, u, v);

	q = g3[b01 + bz1]; u = __at3(rx0, ry1, rz1);
	q = g3[b11 + bz1]; v = __at3(rx1, ry1, rz1);
	b = __lerp(t, u, v);

	d = __lerp(sy, a, b);

	return __lerp(sz, c, d);
}

void __noise_normalize2(double v[2])
{
	double s;

	s = sqrt(v[0] * v[0] + v[1] * v[1]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
}

void __noise_normalize3(double v[3])
{
	double s;

	s = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
	v[2] = v[2] / s;
}

void __noise_init(void)
{
	int i, j, k;

	for (i = 0; i < __B; i++) {
		p[i] = i;
		g1[i] = (double)((random() % (__B + __B)) - __B) / __B;

		for (j = 0; j < 2; j++)
			g2[i][j] = (double)((random() % (__B + __B)) - __B) / __B;
		__noise_normalize2(g2[i]);

		for (j = 0; j < 3; j++)
			g3[i][j] = (double)((random() % (__B + __B)) - __B) / __B;
		__noise_normalize3(g3[i]);
	}

	while (--i) {
		k = p[i];
		p[i] = p[j = random() % __B];
		p[j] = k;
	}

	for (i = 0; i < __B + 2; i++) {
		p[__B + i] = p[i];
		g1[__B + i] = g1[i];
		for (j = 0; j < 2; j++)
			g2[__B + i][j] = g2[i][j];
		for (j = 0; j < 3; j++)
			g3[__B + i][j] = g3[i][j];
	}
}

/* --- My harmonic summing functions - PDB --------------------------*/

/*
In what follows "alpha" is the weight when the sum is formed.
Typically it is 2, As this approaches 1 the function is noisier.
"beta" is the harmonic scaling/spacing, typically 2.
*/

double __noise_perlin_noise1D(double x, double alpha, double beta, int n)
{
	int i;
	double val, sum = 0;
	double p, scale = 1;

	p = x;
	for (i = 0; i<n; i++) {
		val = __noise_noise1(p);
		sum += val / scale;
		scale *= alpha;
		p *= beta;
	}
	return(sum);
}

double __noise_perlin_noise2D(double x, double y, double alpha, double beta, int n)
{
	int i;
	double val, sum = 0;
	double p[2], scale = 1;

	p[0] = x;
	p[1] = y;
	for (i = 0; i<n; i++) {
		val = __noise_noise2(p);
		sum += val / scale;
		scale *= alpha;
		p[0] *= beta;
		p[1] *= beta;
	}
	return(sum);
}

double __noise_perlin_noise3D(double x, double y, double z, double alpha, double beta, int n)
{
	int i;
	double val, sum = 0;
	double p[3], scale = 1;

	p[0] = x;
	p[1] = y;
	p[2] = z;
	for (i = 0; i<n; i++) {
		val = __noise_noise3(p);
		sum += val / scale;
		scale *= alpha;
		p[0] *= beta;
		p[1] *= beta;
		p[2] *= beta;
	}
	return(sum);
}
//尽可能的防止宏定义的扩散,干扰其他的内容
#undef  __at3
#undef  __at2
#undef  __setup
#undef  __lerp
#undef  __s_curve
#undef  __NM
#undef  __NP
#undef  __N
#undef  __BM
#undef  __B
//////////////////////一维纹理/////////////////////////
NoiseTexture::NoiseTexture()
{
	_noiseTextureId = 0;
	_width = 0;
}
//
NoiseTexture::~NoiseTexture()
{
	glDeleteTextures(1, &_noiseTextureId);
	_noiseTextureId = 0;
}
//
void    NoiseTexture::initWithSeed(unsigned seed, float frequency, int width)
{
	assert(width>0);
//初始化随机数种子,并同时填写网格梯度
	srand(seed);
	__noise_init();
	_width = _width;
	unsigned char   *_noise = new  unsigned char[width];
	float                     deltaX = frequency / width;//实际上外面的形式应该为frequency*1.0/width
	float                     stepX = 0.0f;
	float                     scopValue = 256.0f;// / frequency;
	for (int i = 0; i < width; ++i)
	{
		float      noise_value = __noise_noise1(stepX)*scopValue;
		stepX += deltaX;
		_noise[i] = (unsigned char)noise_value;
	}
	glGenTextures(1,&_noiseTextureId);
	glBindTexture(GL_TEXTURE_1D,_noiseTextureId);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R, width, 0, GL_R, GL_UNSIGNED_BYTE,_noise);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glBindTexture(GL_TEXTURE_1D, 0);
	delete   _noise;
	_noise = NULL;
}
unsigned      NoiseTexture::name()
{
	return    _noiseTextureId;
}
NoiseTexture		*NoiseTexture::createWithSeed(unsigned seed, int  frequency, int width)
{
	NoiseTexture	*_noiseTexture = new    NoiseTexture();
	_noiseTexture->initWithSeed(seed, frequency, width);
	return   _noiseTexture;
}
///////////////////连续噪声纹理//////////////////////////////////////////////
NoiseConsistency::NoiseConsistency()
{
	_noiseTextureId = 0;
	_width = 0;
}
NoiseConsistency::~NoiseConsistency()
{
	glDeleteTextures(1, &_noiseTextureId);
	_noiseTextureId = 0;
}
void   NoiseConsistency::initWithSeed(unsigned seed, float   frequency, int width)
{
	assert(width>0);
	_width = width;
	srand(seed);
	__noise_init();
	unsigned   char    *_noise = new   unsigned char[width*width*width*4];
	double          position[3] = {0.0,0.0,0.0};
	unsigned char  *_sliceNoise;
	int            i,j, k;
	for (int step = 0; step < 4;++step,frequency*=2)
	{
		float          deltaXYZ = frequency / width;
		float         scopeValue = 255.0f;// / frequency;
		position[2] = 0.0;
		for (i = 0; i < width; ++i, position[2] += deltaXYZ)
		{
			_sliceNoise = _noise + width*width*i*4;
			position[1] = 0.0f;
			for (j = 0; j < width; ++j, position[1] += deltaXYZ)
			{
				unsigned   char    *_lineNoise = _sliceNoise + j*width * 4;
				position[0] = 0.0f;
				for (k = 0; k < width; ++k, position[0] += deltaXYZ)
				{
					_lineNoise[(k << 2) + step] = (unsigned char)( __noise_noise3(position)*scopeValue);
//					assert(_noiseValue<=255);
//					_lineNoise[(k<<2) + step] =  (unsigned char)_noiseValue;
				}
			}
		}
	}
	glGenTextures(1, &_noiseTextureId);
	glBindTexture(GL_TEXTURE_3D, _noiseTextureId);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, _noise);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glBindTexture(GL_TEXTURE_3D,0);
	delete  _noise;
	_noise = NULL;
}
unsigned NoiseConsistency::name()
{
	return  _noiseTextureId;
}
NoiseConsistency	*NoiseConsistency::createWithSeed(unsigned seed, float frequency, int width)
{
	NoiseConsistency	*_noiseTexture = new    NoiseConsistency();
	_noiseTexture->initWithSeed(seed, frequency, width);
	return   _noiseTexture;
}
/////////////二维噪声纹理////////////////////////////////
NoiseTexture2::NoiseTexture2()
{
	_noiseTextureId = 0;
	_width = 0;
}

NoiseTexture2::~NoiseTexture2()
{
	glDeleteTextures(1, &_noiseTextureId);
	_noiseTextureId = 0;
}

void       NoiseTexture2::initWithSeed(unsigned seed, float frequency, int width)
{
	assert(width>0);
	_width = width;
	srand(seed);
	__noise_init();
	typedef      unsigned   char     byte;
	const  int    totalNums = width*width;
	byte      *_noise = new   byte[totalNums];
	float      *floatNoiseValue = new  float[totalNums];
	float       deltaXY = frequency / width;
//	float       scopeValue = 256.0f / frequency;
	double    position[2] = {0.0,0.0};
	float       minValue = 10000;
	float       maxValue = -10000;
	int          i, k;
	for ( i = 0; i < width; ++i, position[0] += deltaXY)
	{
		float      *surfaceNoise = floatNoiseValue + i*width;
		position[1] = 0.0;
		for (k = 0; k < width; ++k, position[1] += deltaXY)
		{
			float     noiseValue=__noise_noise2(position);
			if (minValue>noiseValue)   minValue = noiseValue;
			if (maxValue < noiseValue)  maxValue = noiseValue;
			surfaceNoise[k] = noiseValue;
		}
	}
	float       range = maxValue - minValue;
	for (i = 0; i < totalNums; ++i)
	{
		float    noiseValue = floatNoiseValue[i];
		_noise[i] = (byte)((noiseValue - minValue) / range*255.0f);
	}
	glGenTextures(1, &_noiseTextureId);
	glBindTexture(GL_TEXTURE_2D,_noiseTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, width, 0, GL_RED, GL_UNSIGNED_BYTE, _noise);
//切记,一下的设置一个都能少,一个都不能错误,否则会产生颜色的异常
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
	delete   _noise;
	delete   floatNoiseValue;
}

unsigned    NoiseTexture2::name()
{
	return  _noiseTextureId;
}

NoiseTexture2	  *NoiseTexture2::createWithSeed(unsigned seed, float frequency, int width)
{
	NoiseTexture2	*_noiseTexture = new   NoiseTexture2();
	_noiseTexture->initWithSeed(seed, frequency, width);
	return  _noiseTexture;
}
//////////////三维噪声/////////////////////
NoiseTexture3::NoiseTexture3()
{
	_noiseTextureId = 0;
	_width = 0;
}

NoiseTexture3::~NoiseTexture3()
{
	glDeleteTextures(1, &_noiseTextureId);
	_noiseTextureId = 0;
}

void          NoiseTexture3::initWithSeed(unsigned seed, float frequency, int width)
{
	assert(width > 0);
	_width = width;
	srand(seed);
	__noise_init();
	unsigned   char    *_noise = new   unsigned char[width*width*width ];
	double          position[3];
	unsigned char  *_sliceNoise;
	int            i, j, k;
		float          deltaXYZ = frequency / width;
		float         scopeValue = 256.0f;// / frequency;
		position[0] = 0.0;
		for (i = 0; i < width; ++i, position[0] += deltaXYZ)
		{
			_sliceNoise = _noise + width*width*i;
			position[1] = 0.0f;
			for (j = 0; j < width; ++j, position[1] += deltaXYZ)
			{
				unsigned   char    *_lineNoise = _sliceNoise + j*width;
				position[2] = 0.0f;
				for (k = 0; k < width; ++k, position[2] += deltaXYZ)
					_lineNoise[k] = (unsigned char)( __noise_noise3(position)*scopeValue);
			}
		}
	glGenTextures(1, &_noiseTextureId);
	glBindTexture(GL_TEXTURE_3D, _noiseTextureId);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, width, width, width, 0, GL_RED, GL_UNSIGNED_BYTE, _noise);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glBindTexture(GL_TEXTURE_3D, 0);
	delete  _noise;
	_noise = NULL;
}

unsigned   NoiseTexture3::name()
{
	return  _noiseTextureId;
}

NoiseTexture3     *NoiseTexture3::createWithSeed(unsigned seed,float  frequency, int width)
{
	NoiseTexture3		*_noisetexture = new   NoiseTexture3();
	_noisetexture->initWithSeed(seed, frequency, width);
	return  _noisetexture;
}
__NS_GLK_END