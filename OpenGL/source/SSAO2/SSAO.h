/*
  *��Ļ�ռ价�����ڱ�
  *2017��12��22��5
  *@Author:xiaohuaxiong
 */
/*
  *���ڴ˳����˵��:
  *Ŀǰ�İ汾������ʵ�ֵ�,��2016������ʵ�ֹ�һ��,�����Ǹ��汾��֤����ʧ�ܵİ汾
  *�����Ǹ�ʱ���ҶԼ����ͼ��ѧ����⻹��������,�Ķ���Χ��Ȼ�����㷺
  *Ŀǰ��ʵ�ֻ���Learn-OpenGL/SSAO�̳���3D-cpp-tutorials����
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
	//�����巿��
	glk::Skybox	        *_skybox;
	//�����ϵ�����
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
	//ת������
	glk::Vec3				_kernelVec3[32];
	unsigned               _tangentTextureId;
	//�ƹ��λ��
	glk::Vec3                _lightPosition;
	//�ƹ����ɫ
	glk::Vec4                _lightColor;
	//�����ĸ����ӵ�ģ�ͱ任����/���߾���
	glk::Mat4              _pillarModelMatrix[4];
	glk::Mat3              _pillarNormalMatrix[4];
	//����ı任����
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
	//��ʼ��ת������,��������
	void   initKernelTangent();
	/*
	  *ÿ֡����
	 */
	void   update(float dt);
	void   render();
	//�ӳ���ɫ
	void   defferedRender();
	//����ռ��ڱ�����
	void   updateOcclusion();
	//ģ������������
	void    fuzzyOcclusion();
	/*
	  *�����¼�
	 */
	bool onTouchBegan(const glk::Vec2 &touchPoint);
	void onTouchMoved(const glk::Vec2 &touchPoint);
	void onTouchReleased(const glk::Vec2 &touchPoint);
	/*
	  *�����¼�
	 */
	bool onKeyPressed(glk::KeyCodeType keyCode);
	void onKeyReleased(glk::KeyCodeType keyCode);
};
#endif
