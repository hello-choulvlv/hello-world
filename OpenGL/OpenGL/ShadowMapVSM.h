/*
  *VSM��Ӱ����ʵ��/����ģ��ʵ��
  *2018��8��1��
  *@author:xiaohuaxiong
*/
#ifndef __SHAODW_MAP_VSM_H__
#define __SHADOW_MAP_VSM_H__
#include "GL/glew.h"
#include "engine/Geometry.h"
#include "engine/GLProgram.h"

class ShadowMapVSM : public glk::Object
{
	//֡����������
	unsigned               _framebufferIds[3];
	//��¼��һ�ε�֡����ȥ����
	int               _lastFramebufferId;
	//��ɫ�������
	unsigned               _textureIds[2];
	//����������
	unsigned               _depthTextureId;
	//�ߴ�
	glk::Size                _framebufferSize;
	//��Ҫ�õ���shader
	glk::GLProgram  *_fuzzyShader;
	int                            _baseMapLoc;
	int                            _pixelStepLoc;
	int                            _sampleCountLoc;
	//ģ������Ŀ
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
	//ʹ��֡����������
	void              save();
	void              restore();
	//
	void             setSampleCount(int sampleCount);
	int                getSampleCount()const;
	/*
	  *��ȡ��ɫ����
	 */
	unsigned    getColorTexture()const;
	//����ģ��
	void             fuzzy();
};
#endif