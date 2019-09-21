/*
  *公共数据/函数,类型/宏
  *2018年3月31日
  *@author:xiaohuaxiong
 */
#ifndef __COMMON_H__
#define __COMMON_H__
#include<d3d11.h>
#include<d3dx10math.h>
//按十六字节对齐计算空间
#define __align16(x)  (((x)+0xF) & 0xFFFFFFF0)
//1/sqrtf(2)
#define HALF_SQRT_2	0.7071068f
//重力加速度,以厘米计算
#define GRAV_ACCEL	981.0f	// The acceleration of gravity, cm/s^2
//Key Mask
#define KEY_MASK_W 0x1
#define KEY_MASK_S   0x2
#define KEY_MASK_A  0x4
#define KEY_MASK_D  0x8
//
extern const   int    s_window_width;
extern const   int    s_window_height;
//DirectX
extern ID3D11Device							*s_device ;
extern ID3D11DeviceContext			*s_deviceContext ;
extern IDXGISwapChain                   *s_swapChain ;
extern ID3D11RenderTargetView    *s_renderTargetView ;
extern ID3D11DepthStencilView      *s_depthStencilView ;
extern ID3D11DepthStencilState      *s_depthStencilState ;
extern ID3D11RasterizerState           *s_rasterizerState ;
extern D3DXMATRIX                           s_reflect_matrix;
//对于海平面仿真需要用到的数据
struct OceanParam
{
	//网格的维度,此值需要是2的整次幂
	int             map_dim;
	//网格的实际宽度
	float		  patch_length;
	//风的方向
	D3DXVECTOR2    wind_dir;
	//风的速度
	float          wind_speed;
	//时间的缩放比例
	float          time_scale;
	//波的振幅
	float          amplitude;
	//风的依赖程度,当出现与风的方向相反的波时,需要用这个参数调节波
	float          wind_dependency;
	//波的缩放比例
	float          choppy_scale;
	OceanParam():
		map_dim(512),
		patch_length(2000),
		wind_dir(0.8,0.6),
		wind_speed(600),
		time_scale(0.8f),
		amplitude(0.35f),
		wind_dependency(0.07f),
		choppy_scale(1.3f)
	{
		
	}
};
//
struct Immutable
{
	int    actual_dim;
	int    output_width;
	int    output_height;
	int    dx_offset;
	int    dy_offset;
	float pixel_width;
	float pixel_height;
	float choppy_scale;
};
//计算空间三段域所必须的数据
struct SpectrumConstant
{
	int actual_dim;
	int input_width;
	int output_width;
	int dx_offset;
	int dy_offset;
};

struct ChangePerFrame
{
	float   choppy_scale;
	float   time;
};
//创建一个无需访问资源视图
void  createUnorderAccessView(ID3D11Buffer *buffer,int totalSize,ID3D11UnorderedAccessView **unorderAccessView,ID3D11ShaderResourceView **shaderResourceView);
//创建一个纹理和shader视图/渲染到纹理视图
void  createTextureAndViews2(int width,int height,DXGI_FORMAT format,ID3D11Texture2D **outTexture,ID3D11ShaderResourceView **outResourceView,ID3D11RenderTargetView **outRenderTargetView);
//生成高斯分布随机数
float  gauss_random();
//phillips波频
float phillips(const D3DXVECTOR2 &K, const D3DXVECTOR2 &W, float v, float a, float dir_depend);
#endif