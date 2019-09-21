/*
 *���,�������,�ڴ˳�����,Ϊ�˼�һЩ����,����û��ʵ�ּ����¼�
 *2017-06-24
 *@Author:xiaohuaxiong
 */
#ifndef __GOD_RAY_H__
#define __GOD_RAY_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/RenderTexture.h"
#include "engine/event/TouchEventListener.h"
#include "RayShader.h"
#include "RenderShader.h"
class GodRay :public glk::Object 
{
private:
	unsigned			_twistBufferId;//�����߶��㼯��
	unsigned        _twistIndexId;//��������������
	int                   _vertexSize;
	int                   _indexSize;
	RayShader     *_rayShader;
	RenderShader *_renderShader;
	//������ͼ����,ͶӰ����ȵ��������
	glk::Matrix     _viewMatrix;
	glk::Matrix     _viewProjMatrix;
	glk::Matrix     _projMatrix;
	glk::Matrix     _modelMatrix;
	//
	glk::GLVector3   _eyePosition;
	glk::GLVector3   _targetPosition;
	glk::GLVector3   _axis, _yaxis, _zaxis;
	glk::GLVector2   _offsetPoint;
	/*
	  *��ɫ
	 */
	glk::GLVector4   _color;
	//��Ⱦ������
	glk::RenderTexture   *_rtt;
	//�����¼�
	glk::TouchEventListener *_touchEventListener;
private:
	GodRay();
	GodRay(GodRay &);
	void    initGodRay();
public:
	~GodRay();
	static GodRay *createGodRay();
	//���������߶���
	void    loadTwistObject();
	//������ͼͶӰ����
	void    initCamera(const glk::GLVector3 &eyePosition,const glk::GLVector3 &targetPosition);
	/*
	  *��Ⱦ
	 */
	void    draw();
	/*
	  *��ʵ����
	 */
	void    update(float deltaTime);
	//�ϳ���ͼ����
	void    buildeViewMatrix();
	/*
	  *�����¼�
	 */
	bool   onTouchBegan(const glk::GLVector2 *touchPoint);
	void   onTouchMoved(const glk::GLVector2 *touchPoint);
	void   onTouchEnded(const glk::GLVector2  *touchPoint);
};
#endif