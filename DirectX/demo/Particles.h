/*
  &粒子系统,火焰实现
  *2018/3/21
  *@author:xiaohuaxiong
 */
#ifndef __PARTICLES_H__
#define __PARTICLES_H__
#include "engine/Scene.h"
#include "engine/Camera.h"
#include <d3d10.h>
#include <d3dx10.h>

class Particles : public Scene
{
	//火焰纹理
	ID3D10ShaderResourceView    *_flareResourceView;
	ID3D10ShaderResourceView    *_randomResourceView;
	//buffer
	ID3D10Buffer                               *_bufferPrev, *_bufferNext;
	//
	ID3D10Effect                                *_effect;
	ID3D10EffectTechnique             *_effectStreamTech;
	ID3D10EffectTechnique             *_effectColorTech;
	//input
	ID3D10InputLayout                    *_inputLayoutStream;
	ID3D10InputLayout                    *_inputLayoutColor;
	//Shader Resource Variable
	ID3D10EffectMatrixVariable    *v_viewProjMatrix;
	ID3D10EffectVectorVariable    *v_eyePosition;
	ID3D10EffectVectorVariable    *v_emmiterPosition;
	ID3D10EffectScalarVariable     *v_totalTime;
	ID3D10EffectScalarVariable     *v_timeInterval;
	ID3D10EffectShaderResourceVariable *v_baseTexture,*v_randomTexture;
	//
	Camera                                           *_camera;
	bool                                                   _isFirstRun;
	float                                                  _delayTime;
	float                                                  _timeInterval;
	//
	D3DXVECTOR3						      _emmiterPosition;
public:
	Particles();
	~Particles();
	void               init();
	void               initStreamBuffer();
	void               initEffect();
	void               initInputLayout();
	void               initTexture();
	//
	void               update(float dt);
	void               render();
	//
};
#endif