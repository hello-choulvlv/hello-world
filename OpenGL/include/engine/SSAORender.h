/*
  *SSAO������,���ɸ���Ƭ�ε�����ڱ�ֵ
  *2016-10-17 16:39:51
  *@Author:С����
 */
#ifndef  __SSAO_RENDER_H__
#define __SSAO_RENDER_H__
#include<engine/Object.h>
#include<engine/Geometry.h>
#include<engine/GLProgram.h>

__NS_GLK_BEGIN
class    SSAORender :public  Object
{
	GLProgram         *_glProgram;
	unsigned               _mvpMatrixLoc;
	unsigned               _globlePositionLoc;
	unsigned               _globleNormalLoc;
	unsigned               _noiseMapLoc;
	unsigned               _samplesLoc;
	unsigned               _nearFarDistanceLoc;
	unsigned               _noiseScaleLoc;
	unsigned               _kernelRadiusLoc;
	unsigned               _projMatrixLoc;
//
	unsigned     _framebufferId;
	unsigned     _ssaoTextureId;
	unsigned     _noiseTextureId;
	float            _kernelRadius;
	GLVector2  _noiseScale;
	GLVector3   _samples[64];
private:
	SSAORender(SSAORender &);
public:
	SSAORender();
	~SSAORender();
	void           initSSAO();
public:
	unsigned   loadSSAOTexture(Matrix  &projMatrix, unsigned    _positionTextureId, unsigned   _normalTextureId);
};

__NS_GLK_END
#endif