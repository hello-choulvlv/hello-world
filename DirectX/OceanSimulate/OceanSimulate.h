/*
  *��ˮ����
  *�����������:FFT/C++AMP
 */
#ifndef _OCEAN_SIMULATE_H_
#define _OCEAN_SIMULATE_H_
#include<d3dx11.h>
#include<d3d11.h>
#include<amp.h>
#include<amp_graphics.h>
#include<vector>
#include"fft512x512/FFT512x512.h"
#include "common.h"

class OceanSimulate
{
	ID3D11Device					*_device;
	ID3D11DeviceContext   *_deviceContext;
	//λ����ͼ
	ID3D11Texture2D          *_displacementTexture;
	ID3D11ShaderResourceView *_displacementShaderView;
	ID3D11RenderTargetView      *_displacementRenderView;
	//�ݶ�����
	ID3D11Texture2D          *_gradientTexture;
	ID3D11ShaderResourceView *_gradientShaderView;
	ID3D11RenderTargetView     *_gradientRenderView;
	//����������,����λ��/�ݶ�����ʱʹ��
	ID3D11SamplerState              *_samplerStatePoint;
	//C++AMP ������
	concurrency::accelerator_view    _acceleratorView;
	//��ʼ�߶ȳ�
	concurrency::array_view<const concurrency::graphics::float_2>   *_array_view_float2_h0;
	//��ʼ�Ƕȳ�omega
	concurrency::array_view<const float>   *_array_view_float_omega;
	//����������array_view���ɵ�FFT�任�Ļ�������,������ʱ���������
	concurrency::array_view<concurrency::graphics::float_2>               *_array_view_float2_ht;
	//����������ݾ���FFT512x512�������ɵĸ߶ȳ�/ƫ�Ƴ�
	concurrency::array_view<concurrency::graphics::float_2>               *_array_view_float2_dxyz;
	ID3D11Buffer								*_buffer_float2_dxyz;
	ID3D11ShaderResourceView   *_shaderViewDxyz;
	ID3D11UnorderedAccessView *_unorderViewDxyz;
	//buffer/�ı���
	ID3D11Buffer                              *_quad_buffer;
	//shader
	ID3D11VertexShader                 *_quad_vertex_shader;
	ID3D11PixelShader                    *_displace_pixel_shader;
	ID3D11PixelShader                    *_gradient_pixel_shader;
	//layout
	ID3D11InputLayout                   *_quad_layout;
	//constant buffer
	ID3D11Buffer                              *_immutable_buffer;
	//ID3D11Buffer                              *_cbPerFrameBuffer;
	//
	FFT512x512                                   _fft_512x512_plan;
	OceanParam                                 _oceanParam;
	Immutable                                    _immutable;
	ChangePerFrame                        _changePerFrame;
	SpectrumConstant                      _spectrumConstant;
public:
	OceanSimulate(OceanParam &param,ID3D11Device *device, ID3D11DeviceContext *deviceContext);
	~OceanSimulate();
	static OceanSimulate *create(OceanParam &param,ID3D11Device *device,ID3D11DeviceContext *deviceContext);
	//directX11
	void     init_directx11(const OceanParam &param,int total_size,int sizeof_float2);
	void     init_buffer(const OceanParam &param,std::vector<concurrency::graphics::float_2> &ho_data,std::vector<float> &omega_data,std::vector<concurrency::graphics::float_2> &zero_data);
	void     init_height_map(OceanParam &param,std::vector<concurrency::graphics::float_2> &h0_float_2,std::vector<float> &omega_float);
	//���������߶ȳ�
	void    update_displacement_map(float dt,float time);

	ID3D11ShaderResourceView *get_displacement_shader_view() { return _displacementShaderView; };

	ID3D11ShaderResourceView *get_gradient_shader_view() { return _gradientShaderView; };
};
#endif
