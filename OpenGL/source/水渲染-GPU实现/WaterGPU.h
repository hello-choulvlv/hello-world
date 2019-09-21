/*
  *ˮ�����GPUʵ��
  *@2017-8-3
  *@Author:xiaohuaxiong
*/
#ifndef __WATER_GPU_H__
#define __WATER_GPU_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/Camera.h"
#include "engine/RenderTexture.h"
#include "engine/Shape.h"
#include "engine/GLTexture.h"

#include "engine/event/TouchEventListener.h"
#include "engine/event/MouseEventListener.h"
#include "engine/event/KeyEventListener.h"

#include "WaterHeightShader.h"
#include "WaterNormalShader.h"
#include "WaterShader.h"
#include "PoolShader.h"

class WaterGPU :public glk::Object
{
	//Shader
	WaterHeightShader     *_waterHeightShader;
	WaterNormalShader   *_waterNormalShader;
	WaterShader                *_waterShader;
	PoolShader                   *_poolShader;
	//Renderer Texture
	glk::RenderTexture      *_heightTexture0;
	glk::RenderTexture      *_heightTexture1;
	glk::RenderTexture      *_normalTexture;
	//Camera
	glk::Camera                 *_camera;
	//Mesh
	glk::Skybox	                  *_poolMesh;
	//ˮ������
	glk::Mesh                      *_waterMesh;
	//��������ͼ
	glk::GLCubeMap         *_texCubeMap;
	//������İ�߶�
	float                               _halfCubeHeight;
	//�����������ķ�������
	glk::GLVector3             _texCubeNormals[6];
	//ģ�;���
	glk::Matrix                  _waterModelMatrix;
	glk::Matrix                  _skyboxModelMatrix;
	//���߾���
	glk::Matrix3                _waterNormalMatrix;
	//��Դ��λ��
	glk::GLVector3            _lightPosition;
	//����ˮ������Ŷ�����
	glk::GLVector4           _waterParam;
	float                             _deltaTime;

	glk::TouchEventListener   *_touchEventListener;
	glk::KeyEventListener       *_keyEventListener;
	glk::MouseEventListener  *_mouseEventListener;
	int                                        _keyMask;
	glk::GLVector2                   _offsetVec2;
	//֡����������
	unsigned								_frameBufferId;
	//��������ͼ
	unsigned                            _photonTextureCubeMap[2];
	//ƽ����ͼ
	unsigned                            _photonTexture;
	//Photon Shader
	glk::GLProgram               *_photonProgram;
	//Photon Blur Shader
	glk::GLProgram              *_photonBlurProgram;
	//Photon���㻺��������
	unsigned                           _photonVertexbufferId;
	//Photon �������껺��������
	unsigned                           _photonFragCoordbufferId;
	//���ػ���������
	unsigned                           _pixelbufferId;
	//������ͼ
	glk::GLProgram              *_normalProgram;
	//��������ͼ����������
	unsigned                           _cubeMapfragCoordVertex;
	unsigned                           _cubeMapVertex;
	glk::Skybox                      *_skyboxVertex;
private:
	WaterGPU();
	WaterGPU(const WaterGPU &);
	void           init();
public:
	~WaterGPU();
	static WaterGPU *create();
	//��ʼ�������
	void          initCamera(const glk::GLVector3 &eyePosition,const glk::GLVector3 &targetPosition);
	//��ʼ��Photon��ز���,�Լ��ȹص�shader��shader����
	void          initPhotonParam();
	//���ù���ˮ�Ĳ���
	void          initWaterParam();
	//��Ⱦˮ�߶�����
	void          drawWaterHeightTexture();
	//��Ⱦˮ��������
	void          drawWaterNormalTexture();
	//��ȾCaustic
	void          drawCaustic();
	//��Ⱦ��պ�
	void         drawSkybox();
	//����
	void         drawTexture(int textureId);
	//draw
	void         draw();
	//update()
	void         update(float deltaTime);
	/*
	  *�����ص�����
	 */
	bool   onTouchBegan(const glk::GLVector2 *touchPoint);
	void   onTouchMoved(const glk::GLVector2 *touchPoint);
	void   onTouchEnded(const glk::GLVector2 *touchPoint);
	/*
	  *�����ص�����
	 */
	bool   onKeyPressed(const glk::KeyCodeType keyCode);
	void   onKeyReleased(const glk::KeyCodeType keyCode);
	/* 
	  *����Ҽ��ص�����
	 */
	bool   onMouseClick(glk::MouseType mouseType,const glk::GLVector2 *clickPoint);
	void   onMouseMoved(glk::MouseType mouseType,const glk::GLVector2 *clickPoint);
	void   onMouseEnded(glk::MouseType mouseType,const glk::GLVector2 *clickPoint);
	/*
	  *����Ļ����㴦����ˮ�沨��
	*/
	void   addWaterDrop(const glk::GLVector2 &touchPoint);
};
#endif