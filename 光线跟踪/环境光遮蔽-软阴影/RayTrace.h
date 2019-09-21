/*
  *光线跟踪场景渲染
  *全局使用OpenGL坐标系
  *2018年5月18日
  *@author:xiaohuaxiong
 */
#ifndef __RAY_TRACER_H__
#define __RAY_TRACER_H__
#include "engine/SceneTracer.h"
#include "engine/Camera2.h"
#include "Shape.h"
#include <vector>
/*
  *光源
 */
struct Light
{
public:
	Shape    *_lightShape;
	//光的颜色
	float_3    _lightColor;
	//环境光//散射光
	float         _ambientCoef,_diffuseCoef;
	//光照强度衰减因子
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
	//环境光遮蔽因子
	float                     _ambientOcclusionCoef;
	//采样数目
	int                        _sampleCount;
	Texture               _textureEarth;
	Texture               _textureCube;
	Texture               _textureFloor;
	//是否开启环境光遮蔽功能
	bool                     _isAmbientEnabled;
	//是否开启软阴影
	bool                     _isSoftShadow;
public:
	RayTracer(Camera2 *camera,Camera *camera2);
	~RayTracer();
	static RayTracer  *create(Camera2 *camera,Camera *camera2);
	//初始化场景
	void        initTracerScene();
	//计算几何体的空间关系
	bool        underObstacle(Shape *shape,const float_3 &point,const float_3 &lightDirection,float lightDistance);
	//计算某一点的光照强度
	float_3   lightIntensity(Shape *shape,const float_3 &point,const float_3 &normal,const float_3 &lightPosition,float ambientOcclusion);
	//计算环境光遮蔽因子
	float        ambientIntensity(Shape *shape,const float_3&point,const float_3 &normal);
	//计算某一点的最终颜色
	void        illuminatePoint(Shape *shape,const float_3 &point,const float_3 &normal,float_3 &color);
	//跟踪场景中的外部NDC坐标(dx,dy)
	void        rayTrace(float dx, float dy,float_3 &color);
	float_3   rayTrace(const float_3 &point,const float_3 &ray,Shape *shape,int depth);
};

#endif