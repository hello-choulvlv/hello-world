/*
  *屏幕空间环境光遮蔽
  *2017年12月22日5
  *@Author:xiaohuaxiong
 */
/*
  *关于此程序的说明:
  *目前的版本是重新实现的,在2016年曾经实现过一版,不过那个版本被证明是失败的版本
  *而且那个时候我对计算机图形学的理解还并不深入,阅读范围仍然不够广泛
  *目前的实现基于Learn-OpenGL/SSAO教程与3D-cpp-tutorials材料
 */
#ifndef __SSAO_H__
#define __SSAO_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/GLProgram.h"
#include "engine/Shape.h"
#include "engine/Camera2.h"
#include "engine/event/TouchEventListener.h"
#include "engine/event/KeyEventListener.h"
#include "engine/DefferedShader.h"
#include "engine/RenderTexture.h"
class SSAO :public glk::Object
{
	//六面体房间
	glk::Skybox	        *_skybox;
	//房间上的柱子
	glk::Chest             *_pillar;
	//shader
	glk::GLProgram  *_lightProgram;
	glk::GLProgram  *_geometryProgram;
	glk::GLProgram  *_ssaoProgram;
	glk::GLProgram  *_fuzzyProgram;
	//
	glk::RenderTexture *_ssaoTexture;
	glk::RenderTexture *_fuzzyTexture0;
	glk::RenderTexture *_fuzzyTexture1;
	//camera
	glk::Camera2       *_camera;
	//
	glk::DefferedShader *_defferedShader;
	//转动核心
	glk::Vec3				_kernelVec3[32];
	unsigned               _tangentTextureId;
	//灯光的位置
	glk::Vec3                _lightPosition;
	//灯光的颜色
	glk::Vec4                _lightColor;
	//关于四个柱子的模型变换矩阵/法线矩阵
	glk::Mat4              _pillarModelMatrix[4];
	glk::Mat3              _pillarNormalMatrix[4];
	//房间的变换矩阵
	glk::Mat4              _skyboxModelMatrix;
	glk::Mat3               _skyboxNormalMatrix;
	//event
	glk::TouchEventListener  *_touchListener;
	glk::Vec2               _offsetVec2;
	//
	int                           _keyMask;
	glk::KeyEventListener *_keyListener;
public:
	SSAO();
	~SSAO();
	static SSAO *create();
	void   init();
	//初始化转动核心,切线纹理
	void   initKernelTangent();
	/*
	  *每帧调用
	 */
	void   update(float dt);
	void   render();
	//延迟着色
	void   defferedRender();
	//计算空间遮蔽因子
	void   updateOcclusion();
	//模糊环境光因子
	void    fuzzyOcclusion();
	/*
	  *触屏事件
	 */
	bool onTouchBegan(const glk::Vec2 &touchPoint);
	void onTouchMoved(const glk::Vec2 &touchPoint);
	void onTouchReleased(const glk::Vec2 &touchPoint);
	/*
	  *键盘事件
	 */
	bool onKeyPressed(glk::KeyCodeType keyCode);
	void onKeyReleased(glk::KeyCodeType keyCode);
};
#endif
