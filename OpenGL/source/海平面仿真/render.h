/*
  *文件头,渲染海水
  *@author:xiaohuaxiong
  *@2018年4月6日
 */
#ifndef __RENDER_H__
#define __RENDER_H__
#include<d3d11.h>
#include "Camera.h"

///////////////////////////所有函数的声明///////////////////////
void     initBuffer();
void     initShaderView();
void     initSamplerState();
void     initFreshnelMap();
void     initRenderResource();
void     destroyRenderResource();
//渲染海洋网格模型
void     renderOceanMeshShader(Camera  &camera, ID3D11ShaderResourceView *shaderViewDisplacement, ID3D11ShaderResourceView *shaderViewGradient, float time);
#endif