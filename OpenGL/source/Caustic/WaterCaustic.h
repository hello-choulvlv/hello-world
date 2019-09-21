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
	//ˮ������ĸ߶�,��Y����
	float									_waterHeight;
	//ˮ����ĸ߶�,��Y��Ϊ��׼
	float                                _groundHeight;
	glk::Mesh						*_waterMesh;
	glk::Mesh                       *_groundMesh;
	//����ˮ�������ƽ�ƾ���
	glk::Matrix                     _waterModelMatrix;
	glk::Matrix                     _groundModelMatrix;
	glk::Camera					*_camera;
	//ˮ��߶ȳ�
	glk::RenderTexture     *_heightTexture0;
	glk::RenderTexture     *_heightTexture1;
	//ˮ�淨�߳�
	glk::RenderTexture     *_normalTexture;
	//����ˮ�Ĳ����趨
	glk::GLVector4             _waterParam;
	//���ߵķ���,���뾭����λ��
	glk::GLVector3             _lightDirection;
	float                               _deltaTime;
	//������ߵ�ˮ��ƽ������
	unsigned                       _groundMeshVertex;
	//�洢��ɢ�ߵ�����
	glk::RenderTexture	*_causticTexture0;
	glk::RenderTexture   *_causticTexture1;
	//����,
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
	  *��Ⱦ�߶ȳ�
	 */
	void            drawWaterHeightTexture();
	/*
	  *��Ⱦ���߳�
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