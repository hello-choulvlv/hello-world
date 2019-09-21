/*
  *水体仿真GPU实现
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
	//水面网格
	glk::Mesh                      *_waterMesh;
	//立方体贴图
	glk::GLCubeMap         *_texCubeMap;
	//立方体的半高度
	float                               _halfCubeHeight;
	//立方体各个面的法线向量
	glk::GLVector3             _texCubeNormals[6];
	//模型矩阵
	glk::Matrix                  _waterModelMatrix;
	glk::Matrix                  _skyboxModelMatrix;
	//法线矩阵
	glk::Matrix3                _waterNormalMatrix;
	//光源的位置
	glk::GLVector3            _lightPosition;
	//关于水网格的扰动参数
	glk::GLVector4           _waterParam;
	float                             _deltaTime;

	glk::TouchEventListener   *_touchEventListener;
	glk::KeyEventListener       *_keyEventListener;
	glk::MouseEventListener  *_mouseEventListener;
	int                                        _keyMask;
	glk::GLVector2                   _offsetVec2;
	//帧缓冲区对象
	unsigned								_frameBufferId;
	//立方体贴图
	unsigned                            _photonTextureCubeMap[2];
	//平面贴图
	unsigned                            _photonTexture;
	//Photon Shader
	glk::GLProgram               *_photonProgram;
	//Photon Blur Shader
	glk::GLProgram              *_photonBlurProgram;
	//Photon顶点缓冲区对象
	unsigned                           _photonVertexbufferId;
	//Photon 像素坐标缓冲区对象
	unsigned                           _photonFragCoordbufferId;
	//像素缓冲区对象
	unsigned                           _pixelbufferId;
	//画测试图
	glk::GLProgram              *_normalProgram;
	//立方体贴图的纹理坐标
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
	//初始化摄像机
	void          initCamera(const glk::GLVector3 &eyePosition,const glk::GLVector3 &targetPosition);
	//初始化Photon相关参数,以及先关的shader，shader参数
	void          initPhotonParam();
	//设置关于水的参数
	void          initWaterParam();
	//渲染水高度纹理
	void          drawWaterHeightTexture();
	//渲染水面网格法线
	void          drawWaterNormalTexture();
	//渲染Caustic
	void          drawCaustic();
	//渲染天空盒
	void         drawSkybox();
	//测试
	void         drawTexture(int textureId);
	//draw
	void         draw();
	//update()
	void         update(float deltaTime);
	/*
	  *触屏回调函数
	 */
	bool   onTouchBegan(const glk::GLVector2 *touchPoint);
	void   onTouchMoved(const glk::GLVector2 *touchPoint);
	void   onTouchEnded(const glk::GLVector2 *touchPoint);
	/*
	  *按键回调函数
	 */
	bool   onKeyPressed(const glk::KeyCodeType keyCode);
	void   onKeyReleased(const glk::KeyCodeType keyCode);
	/* 
	  *鼠标右键回调函数
	 */
	bool   onMouseClick(glk::MouseType mouseType,const glk::GLVector2 *clickPoint);
	void   onMouseMoved(glk::MouseType mouseType,const glk::GLVector2 *clickPoint);
	void   onMouseEnded(glk::MouseType mouseType,const glk::GLVector2 *clickPoint);
	/*
	  *在屏幕坐标点处出发水面波纹
	*/
	void   addWaterDrop(const glk::GLVector2 &touchPoint);
};
#endif