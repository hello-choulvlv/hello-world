/*
  *���߸��ٳ�����Ⱦ
  *ȫ��ʹ��OpenGL����ϵ
  *2018��5��18��
  *@author:xiaohuaxiong
 */
#ifndef __RAY_TRACER_H__
#define __RAY_TRACER_H__
#include "engine/SceneTracer.h"
#include "engine/Camera2.h"
#include "Shape.h"
#include <vector>
/*
  *��Դ
 */
struct Light
{
public:
	Shape    *_lightShape;
	//�����ɫ
	float_3    _lightColor;
	//������//ɢ���
	float         _ambientCoef,_diffuseCoef;
	//����ǿ��˥������
	float         _quadraticAttenuation, _linearAttenuation, _constantAttenuation;
public:
	Light();
	~Light();
};
class RayTracer : public SceneTracer
{
	std::vector<Shape*>    _shapeVec;
	Light                  _light;
	Camera2          *_camera;
	Camera             *g_Camera;
	//�������ڱ�����
	float                     _ambientOcclusionCoef;
	//������Ŀ
	int                        _sampleCount;
	Texture               _textureEarth;
	Texture               _textureCube;
	Texture               _textureFloor;
	//�Ƿ����������ڱι���
	bool                     _isAmbientEnabled;
	//�Ƿ�������Ӱ
	bool                     _isSoftShadow;
public:
	RayTracer(Camera2 *camera,Camera *camera2);
	~RayTracer();
	static RayTracer  *create(Camera2 *camera,Camera *camera2);
	//��ʼ������
	void        initTracerScene();
	//���㼸����Ŀռ��ϵ
	bool        underObstacle(Shape *shape,const float_3 &point,const float_3 &lightDirection,float lightDistance);
	//����ĳһ��Ĺ���ǿ��
	float_3   lightIntensity(Shape *shape,const float_3 &point,const float_3 &normal,const float_3 &lightPosition,float ambientOcclusion);
	//���㻷�����ڱ�����
	float        ambientIntensity(Shape *shape,const float_3&point,const float_3 &normal);
	//����ĳһ���������ɫ
	void        illuminatePoint(Shape *shape,const float_3 &point,const float_3 &normal,float_3 &color);
	//���ٳ����е��ⲿNDC����(dx,dy)
	void        rayTrace(float dx, float dy,float_3 &color);
	float_3   rayTrace(const float_3 &point,const float_3 &ray,Shape *shape,int depth);
};

#endif