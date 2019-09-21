/*
  *数学专用函数实现
  *@date:2017-04-11
  *@Author:xiaohuaxiong
 */
#include<engine/Math.h>
#include<stdlib.h>
#include<math.h>
__NS_GLK_BEGIN
static unsigned _static_rand_seed;

void		initSeed(unsigned seed)
{
	_static_rand_seed = seed;
}
float		randomValue()
{
	//unsigned之内的所有的素数    
	static       unsigned       _prim_table_index = 0;
	const       unsigned        _prim_table_size = 17;
	const       unsigned        _max_unsigned_value = 0xFFFF;
	static       unsigned       _prim_table_[_prim_table_size] = { 344209, 353767, 1738621, 1747289, 1938863, 2145547, 2238571, 2360173, 2414947, 2512249,
		9983977, 9999397, 9998179, 9359281, 9363533, 9369557, 9375791 };
	unsigned         _new_seed = 9363533 * _static_rand_seed*_static_rand_seed+ _static_rand_seed * 1738621 + 2145547;
	_static_rand_seed = _new_seed;
	return       _new_seed%_max_unsigned_value / ((float)_max_unsigned_value);
}

void		gauss(float work[2])
{
	float      x, y;
	float     w;
	const	int			__maxValue = 0x7FFF;
	do
	{
		x = rand() / (float)RAND_MAX;// this->randomValue();
		y = rand() / (float)RAND_MAX;
		x = 2.0f* x - 1.0f;
		y = 2.0f * y - 1.0f;
		w = x*x + y*y;
	} while (w >= 1.0f);
	w = sqrt(-2.0f * log(w) / w);
	work[0] = w * x;
	work[1] = w*y;
	//若期望值为E,方差为V的话,可以加上
	//	work[0] = V*w*x1 + E;
	//	work[1] = V*w*x2 + E;
}



float     phillips(const float a, const float k[2], const float wind[2])
{
	float k2 = k[0] * k[0] + k[1] * k[1];
	if (k2 == 0)
		return 0;
	float v2 = wind[0] * wind[0] + wind[1] * wind[1];//风的方向
	float EL = v2 / GLK_GRAVITY_CONSTANT;//重力加速度
	// the factor *exp(-sqrt(k2)*1.0) can get rid of small waves by increasing 1.0
	float    w = k[0] * wind[0] + k[1] * wind[1];
	//具体的公式,请参见simulating-ocean-water-01.pdf Page 9
	float ret = a*(exp(-1.0 / (k2*EL*EL)) / (k2*k2))*
		(w*w / (k2*v2))*exp(-sqrt(k2)*1.0);
	// this value must be positive since we take the square root later
	return ret;
}

//a是一个固定的系数,名称为Phillips系数
//一般取 0.0081
//   float      phillips(const float a, const float k[2], const float wind[2])
//   {
//	   float		K2 = k[0] * k[0] + k[1] * k[1];
//	   if (K2 == 0.0f)		return 0.0f;
//
//	   float		V2 = wind[0] * wind[0] + wind[1] * wind[1];
//
//	   float		EL = V2 / __GRAVITY_CONSTANT;
////点乘
//	   float		dotValue = k[0] * wind[0] + k[1] * wind[1];
////返回的值必须是正数,因为后面需要用它作为平方因子
//	   const float		expValue = exp(-1.0f/(K2 * EL *EL));
//	   const float		unitValue = dotValue * dotValue/(K2 * V2);
//	   return  a*expValue / (K2*K2) *unitValue * exp(- sqrt(K2));
//   }

//   float phillips(float a, const float K[2], const float  W[2])
//   {
////	   a = 0.35f* 1e-7f;
//	   float dir_depend = 0.07f;
//	   float    v = 600.0f;
//	   // largest possible wave from constant wind of velocity v
//	   float l = v * v / 981.0f;
//	   // damp out waves with very small length w << l
//	   float w = l / 1000;
//
//	   float Ksqr = K[0] * K[0] + K[1] * K[1];
//	   float Kcos = K[0] * W[0] + K[1] * W[1];
//	   float phillips = a * expf(-1 / (l * l * Ksqr)) / (Ksqr * Ksqr * Ksqr) * (Kcos * Kcos);
//
//	   // filter out waves moving opposite to wind
//	   if (Kcos < 0)
//		   phillips *= dir_depend;
//
//	   // damp out waves with very small length w << l
//	   return phillips * expf(-Ksqr * w * w);
//   }

float gauss_distribution(float x, float y, float want_x,float want_y, float variance)
{
	float d_x = x - want_x;
	float d_y = y - want_y;

	float d_2x = d_x * d_x;
	float d_2y = d_y * d_y;

	float d_t = d_2x + d_2y;
	float d_v = variance * variance;
	float d_2v = 2.0f * d_v;

	float gauss = 1.0f / sqrtf(2.0f * M_PI * d_v);

	gauss *= expf(-d_t / d_2v);

	return gauss;
}

__NS_GLK_END