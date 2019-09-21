/*
  *Water Caustic
  *2017-8-23
  *@Author:xiaohuaxiong
 */
#ifndef __WATER_CAUSTIC_H__
#define __WATER_CAUSTIC_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"
#include "engine/Shape.h"
#include "engine/Camera.h"
#include "engine/RenderTexture.h"
#include "engine/GLTexture.h"

#include "WaterHeightShader.h"
#include "WaterNormalShader.h"
#include "WaterShader.h"
#include "FuzzyShader.h"
#include "CausticShader.h"

class  WaterCaustic :public glk::Object
{
private:
	WaterHeightShader      *_heightShader;
	WaterNormalShader    *_normalShader;
	WaterShader                 *_waterShader;
	FuzzyShader					*_fuzzyShader;
	CausticShader				*_causticShader;
	glk::GLProgram            *_normalProgram;
	glk::GLProgram           *_groundProgram;
	glk::GLCubeMap          *_texCubeMap;
	glk::GLTexture             *_groundTexture;
	//水面网格的高度,在Y轴上
	float									_waterHeight;
	//水底面的高度,以Y轴为标准
	float                                _groundHeight;
	glk::Mesh						*_waterMesh;
	glk::Mesh                       *_groundMesh;
	//关于水面网格的平移矩阵
	glk::Matrix                     _waterModelMatrix;
	glk::Matrix                     _groundModelMatrix;
	glk::Camera					*_camera;
	//水面高度场
	glk::RenderTexture     *_heightTexture0;
	glk::RenderTexture     *_heightTexture1;
	//水面法线场
	glk::RenderTexture     *_normalTexture;
	//关于水的参数设定
	glk::GLVector4             _waterParam;
	//光线的方向,必须经过单位化
	glk::GLVector3             _lightDirection;
	float                               _deltaTime;
	//计算光线的水底平面网格
	unsigned                       _groundMeshVertex;
	//存储焦散线的纹理
	glk::RenderTexture	*_causticTexture0;
	glk::RenderTexture   *_causticTexture1;
	//测试,
private:
	void              init();
	void              initWater();
	void              initCamera();
	WaterCaustic();
	WaterCaustic(const WaterCaustic &);
public:
	~WaterCaustic();
	static     WaterCaustic     *create();
	void             update(float deltaTime);
	void             draw();
	/*
	  *渲染高度场
	 */
	void            drawWaterHeightTexture();
	/*
	  *渲染法线场
	 */
	void           drawWaterNormalTexture();
	/*
	  *Caustic
	 */
	void           drawCaustic();
	//ground
	void          drawGround();
	//
	void           drawTexture(int textureId);
	/*
	  *draw Water
	 */
	void           drawWater();
};
#endif