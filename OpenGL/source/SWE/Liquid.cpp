/*
  *浅水方程,流体实现
  *2018年12月19日
  *@author:xiaohuaxiong
 */
#include "Liquid.h"
#include<math.h>
#include<string.h>
Liquid::Liquid() :
	_resolutionX(0)
	, _resolutionY(0)
	, _heightArray(nullptr)
	, _velocityXArray(nullptr)
	, _velocityYArray(nullptr)
	, _nextHeightArray(nullptr)
	,_nowHeightArray(nullptr)
	, _nowVelocityXArray(nullptr)
	, _nextVelocityXArray(nullptr)
	,_nowVelocityYArray(nullptr)
	,_nextVelocityYArray(nullptr)
	, _normal(nullptr)
{
}

Liquid::~Liquid()
{
	delete _heightArray;
	delete _velocityXArray;
	delete _velocityYArray;
	delete _normal;
}

void  Liquid::init(int resolution_x, int resolution_y)
{
	_resolutionX = resolution_x;
	_resolutionY = resolution_y;

	_heightArray = new float[2 * resolution_x * resolution_y];
	_velocityXArray = new float[2 * resolution_x * resolution_y];
	_velocityYArray = new float[2 * resolution_x * resolution_y];

	_nowHeightArray = _heightArray;
	_nextHeightArray = _heightArray + resolution_x * resolution_y;
	
	_nowVelocityXArray = _velocityXArray;
	_nextVelocityXArray = _velocityXArray + resolution_x * resolution_y;

	_nowVelocityYArray = _velocityYArray;
	_nextVelocityYArray = _velocityYArray + resolution_x * resolution_y;

	_normal = new byte[(resolution_x-2) * (resolution_y-2) * 4];
	memset(_normal,0, (resolution_x - 2) * (resolution_y - 2) * 4);
	/*
	  *初始化数据
	 */
	int  index = 0;
	for (int y = 0; y < _resolutionY; ++y)
	{
		for (int x = 0; x< _resolutionX; ++x)
		{
			_nextHeightArray[index] = 0.2f;
			_nowHeightArray[index] = 0.2f;

			_nowVelocityXArray[index] = 0;
			_nowVelocityYArray[index] = 0;

			_nextVelocityXArray[index] = 0;
			_nextVelocityYArray[index] = 0;
			/*
			  *法线
			 */
			//float  frag_x = 1.0f * x/ _resolutionX;
			//float  frag_y = 1.0f * y/ _resolutionY;

			//_normal[index * 4 + 0] = byte(frag_x * 256);
			//_normal[index * 4 + 1] = byte(frag_y * 256);
			//_normal[index * 4 + 2] = sqrtf(1.0f - frag_x * frag_x - frag_y * frag_y) * 256;
			//_normal[index * 4 + 3] = 0.5 * 256;

			++index;
		}
	}
}

float  Liquid::blerp(float  *field, float local_x, float local_y)
{
		int  x = local_x;
		int  y = local_y;

		float fract_x = local_x - x;
		float fract_y = local_y - y;
		int    base_index = y * _resolutionX + x;

		float  v_1 = field[base_index];
		float v_2 = field[base_index + 1];
		float v_3 = field[base_index +1 + _resolutionX];
		float v_4 = field[base_index + _resolutionX];

		float fract_1_x = 1.0f - fract_x;
		float fract_1_y = 1.0f - fract_y;

		return fract_x * fract_y * v_3 + fract_1_x * fract_y * v_4 + fract_1_x * fract_1_y * v_1 + fract_x * fract_1_y * v_2;
}

void  Liquid::advect(float *field_now,float *field_next, float d_x, float d_y,float t)
{
	//针对每一个向量,增加由速度场导出的积分
	for (int y = 1; y < _resolutionY - 1; ++y)
	{
		for (int x = 1; x < _resolutionX - 1; ++x)
		{
			float p_x = x - t * blerp(_nowVelocityXArray, x + d_x,y);
			float p_y = y - t * blerp(_nowVelocityYArray,x,y+ d_y);

			field_next[y * _resolutionX + x] = blerp(field_now,p_x,p_y);
		}
	}
}

void  Liquid::updateHeight(float t)
{
	for (int y = 1; y < _resolutionY-1; ++y)
	{
		for (int x = 1; x < _resolutionX-1; ++x)
		{
			int  offset = y *_resolutionX + x;
			_nextHeightArray[offset] -= _nextHeightArray[offset] * t *(_nextVelocityXArray[offset + 1] - _nextVelocityXArray[offset] + _nextVelocityYArray[offset + _resolutionX] - _nextVelocityYArray[offset]);
		}
	}
}

void  Liquid::updateBoundary(float t)
{
	//底侧,最上侧
	int  base_top_index = (_resolutionY - 1) * _resolutionX;
	for (int x = 0; x < _resolutionX; ++x)
	{
		_nextHeightArray[x] = _nextHeightArray[x+_resolutionX];
		_nextVelocityXArray[x] = 0;// _nextVelocityXArray[x + _resolutionX];
		_nextVelocityYArray[x] = 0;// _nextVelocityYArray[x + _resolutionX];

		_nextHeightArray[x + base_top_index] = _nextHeightArray[x + base_top_index - _resolutionX];
		_nextVelocityXArray[x + base_top_index] = 0;// _nextVelocityXArray[x + base_top_index - _resolutionX];
		_nextVelocityYArray[x + base_top_index] = 0;// _nextVelocityYArray[x + base_top_index - _resolutionX];
	}
	//左侧/右侧
	int  base_left_index = 0;
	for (int y = 0; y < _resolutionY; ++y)
	{
		const int  base_l_index = y * _resolutionX;
		_nextHeightArray[base_l_index] = _nextHeightArray[base_l_index + 1];
		_nextVelocityXArray[base_l_index] = 0;// _nextVelocityXArray[base_l_index + 1];
		_nextVelocityYArray[base_l_index] = 0;// _nextVelocityYArray[base_l_index + 1];

		const int base_r_index = base_l_index + _resolutionX - 1;
		_nextHeightArray[base_r_index] = _nextHeightArray[base_r_index-1];
		_nextVelocityXArray[base_r_index] = 0;// _nextVelocityXArray[base_r_index - 1];
		_nextVelocityYArray[base_r_index] = 0;// _nextVelocityXArray[base_r_index - 1];
	}
}

void  Liquid::updateVelocity(float t)
{
	for (int y = 1; y < _resolutionY-1; ++y)
	{
		int  base_index = y * _resolutionX;
		for (int x = 1; x < _resolutionX-1; ++x)
		{
			int start_j = base_index + x;
			_nextVelocityXArray[start_j] += t *(_nextHeightArray[start_j - 1] - _nextHeightArray[start_j]);
			_nextVelocityYArray[start_j] += t *(_nextHeightArray[start_j - _resolutionX] - _nextHeightArray[start_j]);
		}
	}
}

void  Liquid::computeNormal()
{
	for (int y = 1; y < _resolutionY - 1; ++y)
	{
		int  base_index = y*_resolutionX;
		int  base_n = (y - 1)*(_resolutionX-2);
		for (int x = 1; x < _resolutionX - 1; ++x)
		{
			float  m_array[3];
			m_array[0] = _nextHeightArray[base_index + x+ 1] - _nextHeightArray[base_index + x];
			m_array[1] = _nextHeightArray[base_index + x + _resolutionX] - _nextHeightArray[base_index + x];
			m_array[2] = sqrtf(1.0f - m_array[0] * m_array[0] -m_array[1] * m_array[1]);// N dot L

			float  rind = 0.5f;
			float  dist = 0.5f;
			float a = rind * rind;
			float b = sqrtf(1.0f - a + a * m_array[2] * m_array[2]);

			float  refract_array[2];
			refract_array[0] = m_array[2] * m_array[0];
			refract_array[1] = m_array[2] * m_array[1];

			refract_array[0] = -b * m_array[0] + rind * refract_array[0];
			refract_array[1] = -b * m_array[1] + rind * refract_array[1];

			float refract_x = 1.0f *x/_resolutionX + dist * refract_array[0];
			float refract_y = 1.0f *y/ _resolutionY + dist * refract_array[1];

			float eye_array[3] = {0.0f,0.0f,-1.0f};
			float ndot_l = eye_array[0] * m_array[0] + eye_array[1] * m_array[1] + eye_array[2] * m_array[2];
			float reflect_array[3];

			reflect_array[0] = 2.0f * ndot_l * m_array[0] - eye_array[0];
			reflect_array[1] = 2.0f * ndot_l * m_array[1] - eye_array[1];
			reflect_array[2] = 2.0f * ndot_l * m_array[2] - eye_array[2];

			float f = 2.0f * sqrtf(reflect_array[0] * reflect_array[0] + reflect_array[1] * reflect_array[1] + reflect_array[2] * reflect_array[2]);
			float reflect_x = reflect_array[0]/f +0.5f;
			float reflect_y = reflect_array[1] / f + 0.5f;

			int  normal_j = (base_n + x-1) << 2;
			_normal[normal_j + 0] = reflect_x * 256;
			_normal[normal_j + 1] = reflect_y * 256;
			_normal[normal_j + 2] = refract_x * 256;
			_normal[normal_j + 3] = refract_y * 256;
		}
	}
	////边界,bottom + top
	//int  t_index = ((_resolutionY - 1) * _resolutionX) << 2;
	//for (int x = 0; x < _resolutionX; ++x)
	//{
	//	int base_index = x << 2;
	//	int inc_index = _resolutionX << 2;
	//	_normal[base_index + 0] = _normal[base_index + inc_index+0];
	//	_normal[base_index + 1] = _normal[base_index + inc_index + 1];
	//	_normal[base_index + 2] = 1.0f * x / _resolutionX * 256;
	//	_normal[base_index + 3] = 0.0f;

	//	_normal[t_index + base_index + 0] = _normal[t_index + base_index - inc_index +0];
	//	_normal[t_index + base_index + 1] = _normal[t_index + base_index - inc_index + 1];
	//	_normal[t_index + base_index + 2] = 1.0f*x / _resolutionX * 256;
	//	_normal[t_index + base_index + 3] = 1.0f *(_resolutionY-1) / _resolutionY;
	//}
	////边界left + right
	//for (int y = 0; y < _resolutionY; ++y)
	//{
	//	int l_index = y * _resolutionY * 4;
	//	int r_index = l_index + (_resolutionX - 1) * 4;
	//	_normal[l_index + 0] = _normal[l_index + 4];
	//	_normal[l_index + 1] = _normal[l_index + 5];
	//	_normal[l_index + 2] = 0;
	//	_normal[l_index + 3] = 1.0f *y / _resolutionY * 256;

	//	_normal[r_index + 0] = _normal[r_index - 4];
	//	_normal[r_index + 1] = _normal[r_index - 3];
	//	_normal[r_index + 2] = 1.0f *(_resolutionX - 1)/_resolutionX * 256;
	//	_normal[r_index + 3] = 1.0f *y / _resolutionY * 256;
	//}
}

void  Liquid::update(float t)
{
	advect(_nowHeightArray,_nextHeightArray,0.5f,0.5f,t);
	advect(_nowVelocityXArray,_nextVelocityXArray, 0.5f, 0.0f, t);
	advect(_nowVelocityYArray,_nextVelocityYArray, 0.0f, 0.5f, t);

	updateBoundary(t);

	updateHeight(t);
	updateVelocity(t);

	computeNormal();

	float  *tmp = _nextHeightArray;
	_nextHeightArray = _nowHeightArray;
	_nowHeightArray = tmp;

	tmp = _nextVelocityXArray;
	_nextVelocityXArray = _nowVelocityXArray;
	_nowVelocityXArray = tmp;

	tmp = _nextVelocityYArray;
	_nextVelocityYArray = _nowVelocityYArray;
	_nowVelocityYArray = tmp;
}

void Liquid::touchAt(float x, float y)
{
	int  touch_x = x * _resolutionX;
	int  touch_y = y * _resolutionY;

	if (touch_x >= 0 && touch_x < _resolutionX && touch_y >=0 && touch_y < _resolutionY)
	{
		float disp_h = 0.5f;
		int base_index = touch_y * _resolutionX + touch_x;
		if (disp_h > _nowHeightArray[base_index])
			disp_h = _nowHeightArray[base_index];
		
		_nowHeightArray[base_index] -= disp_h;
		_nowHeightArray[base_index - _resolutionX] += disp_h * 0.25f;
		_nowHeightArray[base_index + _resolutionX] += disp_h * 0.25f;
		_nowHeightArray[base_index - 1] += disp_h * 0.25f;
		_nowHeightArray[base_index + 1] += disp_h * 0.25f;
		//Velocity
		_nowVelocityXArray[base_index] -= disp_h * 0.25f;
		_nowVelocityYArray[base_index] -= disp_h * 0.25f;

		_nowVelocityXArray[base_index + 1] -= disp_h *0.25f;
		_nowVelocityYArray[base_index + 1] -= disp_h *0.25f;

		_nowVelocityXArray[base_index - 1] -= disp_h * 0.25f;
		_nowVelocityYArray[base_index - 1] -= disp_h *0.25f;
	}
}

byte  *Liquid::getNormal()const
{
	return _normal;
}