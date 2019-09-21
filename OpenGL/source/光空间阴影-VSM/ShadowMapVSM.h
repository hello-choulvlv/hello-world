/*
  *VSM阴影纹理实现/纹理模糊实现
  *2018年8月1日
  *@author:xiaohuaxiong
*/
#ifndef __SHAODW_MAP_VSM_H__
#define __SHADOW_MAP_VSM_H__
#include "GL/glew.h"
#include "engine/Geometry.h"
#include "engine/GLProgram.h"

class ShadowMapVSM : public glk::Object
{
	//帧缓冲区对象
	unsigned               _framebufferIds[3];
	//记录上一次的帧缓冲去对象
	int               _lastFramebufferId;
	//颜色纹理对象
	unsigned               _textureIds[2];
	//深度纹理对象
	unsigned               _depthTextureId;
	//尺寸
	glk::Size                _framebufferSize;
	//需要用到的shader
	glk::GLProgram  *_fuzzyShader;
	int                            _baseMapLoc;
	int                            _pixelStepLoc;
	int                            _sampleCountLoc;
	//模糊的数目
	int                            _sampleCount;
	float                         _totalFactor;
	unsigned                _vertexBufferId;
public:
	ShadowMapVSM();
	~ShadowMapVSM();
	static ShadowMapVSM *create(const glk::Size &framebufferSize);
	bool              init(const glk::Size &framebufferSize);
	void              initGLProgram();
	void              initBuffer();
	//使用帧缓冲区对象
	void              save();
	void              restore();
	//
	void             setSampleCount(int sampleCount);
	int                getSampleCount()const;
	/*
	  *获取颜色纹理
	 */
	unsigned    getColorTexture()const;
	//纹理模糊
	void             fuzzy();
};
#endif