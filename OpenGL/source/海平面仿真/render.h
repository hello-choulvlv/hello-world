/*
  *�ļ�ͷ,��Ⱦ��ˮ
  *@author:xiaohuaxiong
  *@2018��4��6��
 */
#ifndef __RENDER_H__
#define __RENDER_H__
#include<d3d11.h>
#include "Camera.h"

///////////////////////////���к���������///////////////////////
void     initBuffer();
void     initShaderView();
void     initSamplerState();
void     initFreshnelMap();
void     initRenderResource();
void     destroyRenderResource();
//��Ⱦ��������ģ��
void     renderOceanMeshShader(Camera  &camera, ID3D11ShaderResourceView *shaderViewDisplacement, ID3D11ShaderResourceView *shaderViewGradient, float time);
#endif