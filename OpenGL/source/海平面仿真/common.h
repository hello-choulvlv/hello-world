/*
  *��������/����,����/��
  *2018��3��31��
  *@author:xiaohuaxiong
 */
#ifndef __COMMON_H__
#define __COMMON_H__
#include<d3d11.h>
#include<d3dx10math.h>
//��ʮ���ֽڶ������ռ�
#define __align16(x)  (((x)+0xF) & 0xFFFFFFF0)
//1/sqrtf(2)
#define HALF_SQRT_2	0.7071068f
//�������ٶ�,�����׼���
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
//���ں�ƽ�������Ҫ�õ�������
struct OceanParam
{
	//�����ά��,��ֵ��Ҫ��2��������
	int             map_dim;
	//�����ʵ�ʿ��
	float		  patch_length;
	//��ķ���
	D3DXVECTOR2    wind_dir;
	//����ٶ�
	float          wind_speed;
	//ʱ������ű���
	float          time_scale;
	//�������
	float          amplitude;
	//��������̶�,���������ķ����෴�Ĳ�ʱ,��Ҫ������������ڲ�
	float          wind_dependency;
	//�������ű���
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
//����ռ������������������
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
//����һ�����������Դ��ͼ
void  createUnorderAccessView(ID3D11Buffer *buffer,int totalSize,ID3D11UnorderedAccessView **unorderAccessView,ID3D11ShaderResourceView **shaderResourceView);
//����һ�������shader��ͼ/��Ⱦ��������ͼ
void  createTextureAndViews2(int width,int height,DXGI_FORMAT format,ID3D11Texture2D **outTexture,ID3D11ShaderResourceView **outResourceView,ID3D11RenderTargetView **outRenderTargetView);
//���ɸ�˹�ֲ������
float  gauss_random();
//phillips��Ƶ
float phillips(const D3DXVECTOR2 &K, const D3DXVECTOR2 &W, float v, float a, float dir_depend);
#endif