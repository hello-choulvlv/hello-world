/*
  *光线跟踪实现
  *2018年4月26日
  *@author:xiaohuaxiong
 */
#include "RayTrace.h"
#include<math.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>
//Light
Light::Light() :
	_lightShape(nullptr),
	_lightColor(1,1,1),
	_ambientCoef(0.25f),
	_diffuseCoef(0.75f),
	_quadraticAttenuation(0.001f),
	_linearAttenuation(0.001f),
	_constantAttenuation(1.f)
{
}

Light::~Light()
{
	delete _lightShape;
}


RayTracer::RayTracer(Camera2 *camera,Camera *camera2):
	_camera(camera),
	g_Camera(camera2),
	_sampleCount(16),
	_ambientOcclusionCoef(0.5f),
	_isAmbientEnabled(true),
	_isSoftShadow(true)
{
}

RayTracer::~RayTracer()
{
}

RayTracer  *RayTracer::create(Camera2 *camera,Camera *camera2)
{
	RayTracer *tracer = new RayTracer(camera,camera2);
	tracer->initTracerScene();
	return tracer;
}

bool RayTracer::underObstacle(Shape *shape, const float_3 &point, const float_3 &lightDirection, float lightDistance)
{
	float testDistance;
	for (auto it = _shapeVec.begin(); it != _shapeVec.end(); ++it)
	{
		Shape *object = *it;
		if (object != shape)
		{
			if (object->intersectTest(point, lightDirection, lightDistance, testDistance))
				return true;
		}
	}
	return false;
}

float_3 RayTracer::lightIntensity(Shape *shape, const float_3 &point, const float_3 &normal, const float_3 &lightPosition, float ambientOcclusion)
{
	//计算点point与lightPosition之间的距离
	float_3   lightDirection = lightPosition - point;
	float        distance2 = lightDirection.length2();
	float        distance = sqrtf(distance2+0.5f);

	lightDirection /= distance;
	float attenuation = _light._quadraticAttenuation*distance2 + _light._linearAttenuation*distance+_light._constantAttenuation;
	float  n_dot_l = normal.dot(lightDirection);
	if (n_dot_l > 0)
	{
		//检测是否处于阴影之下
		if (_light._lightShape->getType() == ShapeType_Sphere)
		{
			if (!underObstacle(shape, point, lightDirection, distance))
			{
				return  _light._lightColor * (_light._ambientCoef *ambientOcclusion + _light._diffuseCoef * n_dot_l) / attenuation;
			}
		}
		else
		{
			Quad *lightQuad = (Quad*)_light._lightShape;
			float    light_dot_normal = -normal.dot(lightQuad->_normal);
			if (light_dot_normal > 0)
			{
				if (!underObstacle(shape, point, lightDirection, distance))
				{
					return  _light._lightColor *(_light._ambientCoef * ambientOcclusion +_light._diffuseCoef*n_dot_l*light_dot_normal)/attenuation;
				}
			}
		}
	}
	return _light._lightColor * (_light._ambientCoef  *ambientOcclusion /attenuation);
}

float   RayTracer::ambientIntensity(Shape *shape, const float_3&point, const float_3 &normal)
{
	float    ambientOcclusion = 0.0f;
	for(int k=0;k<_sampleCount;++k)
	{
		float    distance = 10000, testDistance = 1.0f;
		//向normal指向的平面一侧随机发射光线
		float_3   randomNormal(2.0f*rand()/RAND_MAX-1.0f,2.0f*rand()/RAND_MAX,2.0f*rand()/RAND_MAX);
		randomNormal = randomNormal.normalize();
		float        nDotL = normal.dot(randomNormal);
		if (nDotL < 0)
		{
			randomNormal = float_3(-randomNormal.x,-randomNormal.y,-randomNormal.z);
			nDotL = -nDotL;
		}
		for (auto it = _shapeVec.begin(); it != _shapeVec.end(); ++it)
		{
			Shape *object = *it;
			if (object != shape && object->intersectTest(point, randomNormal, distance,testDistance))
				distance = testDistance;
		}
		ambientOcclusion += nDotL/(1.0f + distance * distance);
	}
	return 1.0f - ambientOcclusion / _sampleCount * _ambientOcclusionCoef;
}

void   RayTracer::illuminatePoint(Shape *shape,const float_3 &point, const float_3 &normal, float_3 &color)
{
	float ambientOcclusion = 1.0f;
	//暂时不开启环境光遮蔽功能
	if (_isAmbientEnabled)
		ambientOcclusion = ambientIntensity(shape,point,normal);
	//检测,是否需要处理软阴影
	if (!_isSoftShadow)
	{
		color *= lightIntensity(shape, point, normal, _light._lightShape->getPosition(), ambientOcclusion);
	}
	else//软阴影
	{
		Quad   *lightQuad = nullptr;
		Sphere *lightSphere = nullptr;
		if (_light._lightShape->getType() == ShapeType_Quad)
		{
			lightQuad = (Quad *)_light._lightShape;
			float_3      accuIntensity;
			for (int k = 0; k < _sampleCount; ++k)
			{
				float   s = 1.0f * rand() / RAND_MAX;
				float   t = 1.0f*rand() / RAND_MAX;
				float_3  lightPosition =lightQuad->_a + lightQuad->_ab * s + lightQuad->_ad * t;
				accuIntensity += lightIntensity(shape,point,normal,lightPosition,ambientOcclusion);
			}
			color *= accuIntensity/_sampleCount;
		}
		else
		{
			lightSphere = (Sphere*)_light._lightShape;
			float_3 accuIntensity;
			for (int k = 0; k < _sampleCount; ++k)
			{
				float_3   lightPosition = lightSphere->_center + float_3(2.0f*rand()/RAND_MAX-1.0f,2.0f*rand()/RAND_MAX-1.0f,2.0f*rand()/RAND_MAX-1.0f)*lightSphere->_radius;
				accuIntensity += lightIntensity(shape, point, normal, lightPosition, ambientOcclusion);
			}
			color *= accuIntensity / _sampleCount;
		}
	}
}
//目前暂时不实现反射与折射功能
//待程序可以运行并正确后我们将会将这一功能补充上去
float_3 RayTracer::rayTrace(const float_3 &point, const float_3 &ray, Shape *shape, int depth)
{
	Shape   *target_object = nullptr;
	Light     *target_light = nullptr;
	float        distance = FLT_MAX;
	float_3   intersect_point;
	for (auto it = _shapeVec.begin(); it != _shapeVec.end(); ++it)
	{
		if ((*it)->intersectTest(point, ray, distance, intersect_point))
		{
			target_object = *it;
		}
	}
	//接下检测是否与灯相交
	if (_light._lightShape->intersectTest(point, ray, distance, intersect_point))
	{
		target_light = &_light;
	}
	//如果与灯光相交
	if (target_light)
		return _light._lightColor;
	if (!target_object)
		return float_3(0);
	//否则对几何体进行进一步的计算
	//如果是四边形
	Quad   *quad = nullptr;
	Sphere *sphere = nullptr;
	if (target_object->getType() == ShapeType_Quad)
	{
		quad = (Quad*)target_object;
		float_3 &normal = quad->_normal;
		float_3 color = quad->_color;
		//获取纹理颜色
		Texture *texture = target_object->getTexture();
		if (texture)
		{
			float_3 someVec = intersect_point - quad->_a;
			color = texture->getColorBilinear(someVec.dot(quad->_tangent),someVec.dot(quad->_binormal));
		}
		illuminatePoint(target_object, intersect_point, normal, color);
		return color;
	}
	else
	{
		sphere = (Sphere*)target_object;
		//计算法线
		float_3    normal = (intersect_point - sphere->_center).normalize();
		float_3    color = sphere->_color;
		//检测纹理
		Texture *texture = sphere->getTexture();
		if (texture)
		{
			float    s = atan2f(normal.x,normal.z)/M_PI * 0.5f + 0.5f;
			float    t = asin(normal.y)/M_PI + 0.5f;
			color *= texture->getColorBilinear(s, t);
		}
		illuminatePoint(sphere, point, normal, color);
		return color;
	}
	return float_3(0);
}


void    RayTracer::initTracerScene()
{
	bool b = _textureEarth.loadTexture("image/earth.jpg");
	b &= _textureCube.loadTexture("image/cube.jpg");
	b &= _textureFloor.loadTexture("image/floor.jpg");
	assert(b);

	_shapeVec.reserve(24);

	_shapeVec.push_back(new Sphere(float_3(-2.0f, -1.0f, 2.0f), 0.5f, float_3(0.0f, 0.5f, 1.0f), nullptr, 0.875f,0,1.0f));
	_shapeVec.push_back(new Sphere(float_3(0.0f, -1.5f, 2.0f), 0.5f, float_3(0.0f, 0.5f, 1.0f), nullptr, 0.125f, 0.875f, 1.52f));
	_shapeVec.push_back(new Sphere(float_3(2.0f, -1.5f, -2.0f), 0.5f, float_3(1.0f, 1.0f, 1.0f), &_textureEarth,0,0,1.0f));

	mat3x3 R(22.5f, float_3(0.0f, 1.0f, 0.0f));
	float_3 V = float_3(2.0f, 0.0f, 2.0f);

	_shapeVec.push_back(new Quad( float_3(-0.5f, -2.0f, 0.5f)*R + V,  float_3(0.5f, -2.0f, 0.5f)*R + V,  float_3(0.5f, -1.0f, 0.5f)*R + V,  float_3(-0.5f, -1.0f, 0.5f)*R + V, float_3(1.0f, 1.0f, 1.0f), &_textureCube));
	_shapeVec.push_back(new Quad(float_3(0.5f, -2.0f, -0.5f)*R + V, float_3(-0.5f, -2.0f, -0.5f)*R + V, float_3(-0.5f, -1.0f, -0.5f)*R + V, float_3(0.5f, -1.0f, -0.5f)*R + V, float_3(1.0f, 1.0f, 1.0f), &_textureCube));
	_shapeVec.push_back(new Quad(float_3(0.5f, -2.0f, 0.5f)*R + V, float_3(0.5f, -2.0f, -0.5f)*R + V, float_3(0.5f, -1.0f, -0.5f)*R + V, float_3(0.5f, -1.0f, 0.5f)*R + V, float_3(1.0f, 1.0f, 1.0f), &_textureCube));
	_shapeVec.push_back(new Quad(float_3(-0.5f, -2.0f, -0.5f)*R + V, float_3(-0.5f, -2.0f, 0.5f)*R + V, float_3(-0.5f, -1.0f, 0.5f)*R + V, float_3(-0.5f, -1.0f, -0.5f)*R + V, float_3(1.0f, 1.0f, 1.0f), &_textureCube));
	_shapeVec.push_back(new Quad(float_3(-0.5f, -1.0f, 0.5f)*R + V,  float_3(0.5f, -1.0f, 0.5f)*R + V, float_3(0.5f, -1.0f, -0.5f)*R + V,  float_3(-0.5f, -1.0f, -0.5f)*R + V, float_3(1.0f, 1.0f, 1.0f), &_textureCube));
	_shapeVec.push_back(new Quad(float_3(-0.5f, -2.0f, -0.5f)*R + V, float_3(0.5f, -2.0f, -0.5f)*R + V, float_3(0.5f, -2.0f, 0.5f)*R + V, float_3(-0.5f, -2.0f, 0.5f)*R + V, float_3(1.0f, 1.0f, 1.0f), &_textureCube));

	_shapeVec.push_back(new Quad(float_3(-0.0f, -2.0f, 4.0f), float_3(4.0f, -2.0f, 4.0f), float_3(4.0f, -2.0f, -4.0f), float_3(-0.0f, -2.0f, -4.0f), float_3(1.0f, 1.0f, 1.0f), &_textureFloor));
	_shapeVec.push_back(new Quad(float_3(-4.0f, -2.0f, 4.0f), float_3(0.0f, -2.0f, 4.0f), float_3(0.0f, -2.0f, 0.0f), float_3(-4.0f, -2.0f, 0.0f), float_3(1.0f, 1.0f, 1.0f), &_textureFloor));
	_shapeVec.push_back(new Quad(float_3(0.0f, 2.0f, -4.0f), float_3(4.0f, 2.0f, -4.0f), float_3(4.0f, 2.0f, 4.0f), float_3(0.0f, 2.0f, 4.0f), float_3(1.0f, 1.0f, 1.0f),nullptr));
	_shapeVec.push_back(new Quad(float_3(-4.0f, 2.0f, 0.0f), float_3(0.0f, 2.0f, 0.0f), float_3(0.0f, 2.0f, 4.0f), float_3(-4.0f, 2.0f, 4.0f), float_3(1.0f, 1.0f, 1.0f),nullptr));
	_shapeVec.push_back(new Quad(float_3(-0.0f, -2.0f, -4.0f), float_3(4.0f, -2.0f, -4.0f), float_3(4.0f, 2.0f, -4.0f), float_3(-0.0f, 2.0f, -4.0f), float_3(1.0f, 1.0f, 1.0f),nullptr));
	_shapeVec.push_back(new Quad(float_3(4.0f, -2.0f, 4.0f), float_3(-4.0f, -2.0f, 4.0f), float_3(-4.0f, 2.0f, 4.0f), float_3(4.0f, 2.0f, 4.0f), float_3(1.0f, 1.0f, 1.0f),nullptr));
	_shapeVec.push_back(new Quad(float_3(4.0f, -2.0f, -4.0f), float_3(4.0f, -2.0f, 4.0f), float_3(4.0f, 2.0f, 4.0f), float_3(4.0f, 2.0f, -4.0f), float_3(0.0f, 1.0f, 0.0f),nullptr));
	_shapeVec.push_back(new Quad(float_3(-4.0f, -2.0f, 4.0f), float_3(-4.0f, -2.0f, -0.0f), float_3(-4.0f, 2.0f, -0.0f), float_3(-4.0f, 2.0f, 4.0f), float_3(1.0f, 0.0f, 0.0f),nullptr));
	_shapeVec.push_back(new Quad(float_3(-4.0f, -2.0f, 0.0f), float_3(0.0f, -2.0f, 0.0f), float_3(0.0f, 2.0f, 0.0f), float_3(-4.0f, 2.0f, 0.0f), float_3(1.0f, 1.0f, 1.0f),nullptr));
	_shapeVec.push_back(new Quad(float_3(0.0f, -2.0f, 0.0f), float_3(0.0f, -2.0f, -4.0f), float_3(0.0f, 2.0f, -4.0f), float_3(0.0f, 2.0f, 0.0f), float_3(1.0f, 1.0f, 1.0f),nullptr));

	float_3 S = float_3(-2.0f, 0.0f, 2.0f);

	_shapeVec.push_back(new Quad(float_3(-0.5f, 1.875f, 0.5f) + S, float_3(0.5f, 1.875f, 0.5f) + S, float_3(0.5f, 1.875f, -0.5f) + S, float_3(-0.5f, 1.875f, -0.5f) + S, float_3(1.0f, 1.0f, 1.0f),nullptr));
	_shapeVec.push_back(new Quad(float_3(-0.5f, 1.875f - 0.125f, 0.5f) + S, float_3(0.5f, 1.875f - 0.125f, 0.5f) + S, float_3(0.5f, 2.0f - 0.125f, 0.5f) + S, float_3(-0.5f, 2.0f - 0.125f, 0.5f) + S, float_3(1.0f, 1.0f, 1.0f),nullptr));
	_shapeVec.push_back(new Quad(float_3(0.5f, 1.875f - 0.125f, -0.5f) + S, float_3(-0.5f, 1.875f - 0.125f, -0.5f) + S, float_3(-0.5f, 2.0f - 0.125f, -0.5f) + S, float_3(0.5f, 2.0f - 0.125f, -0.5f) + S, float_3(1.0f, 1.0f, 1.0f),nullptr));
	_shapeVec.push_back(new Quad(float_3(-0.5f, 1.875f - 0.125f, -0.5f) + S, float_3(-0.5f, 1.875f - 0.125f, 0.5f) + S, float_3(-0.5f, 2.0f - 0.125f, 0.5f) + S, float_3(-0.5f, 2.0f - 0.125f, -0.5f) + S, float_3(1.0f, 1.0f, 1.0f),nullptr));
	_shapeVec.push_back(new Quad(float_3(0.5f, 1.875f - 0.125f, 0.5f) + S, float_3(0.5f, 1.875f - 0.125f, -0.5f) + S, float_3(0.5f, 2.0f - 0.125f, -0.5f) + S, float_3(0.5f, 2.0f - 0.125f, 0.5f) + S, float_3(1.0f, 1.0f, 1.0f),nullptr));

	_light._lightShape= new Quad(float_3(-0.5f, 1.875f - 0.125f, -0.5f) + S, float_3(0.5f, 1.875f - 0.125f, -0.5f) + S, float_3(0.5f, 1.875f - 0.125f, 0.5f) + S, float_3(-0.5f, 1.875f - 0.125f, 0.5f) + S, float_3(3.0f, 3.0f, 3.0f),nullptr);
	// Lights[0].Quad = new CQuad(float_3(-0.5f,  1.875f - 0.125f, -0.5f) + S, float_3( 0.5f,  1.875f - 0.125f, -0.5f) + S, float_3( 0.5f,  1.875f - 0.125f,  0.5f) + S, float_3(-0.5f,  1.875f - 0.125f,  0.5f) + S, float_3(1.0f, 1.0f, 1.0f));
	_light._ambientCoef = 0.25f;
	_light._diffuseCoef = 0.75f;
	_light._lightColor = float_3(2.0f);
}
/*
  *整个工程的核心函数,
  *给出NDC坐标,求出其最终的颜色
 */
void    RayTracer::rayTrace(float x, float y, float_3 &color)
{
	//求出NDC代表的世界空间位置
	//auto &viewProjMat4 =_camera->getViewProjReverseMatrix();
	//auto  &viewProj = _camera->getViewProjMatrix();
	//mat4x4   ppp = viewProj * viewProjMat4;
	//float_4   ndcCoord(dx,dy,0.5,1.0f);
	//float_4   worldPosition = ndcCoord * viewProjMat4;
	//worldPosition.x /= worldPosition.w;
	//worldPosition.y /= worldPosition.w;
	//worldPosition.z /= worldPosition.w;
	//auto &ray_mat4 = _camera->getRayMatrix();
	////眼睛的位置
	//auto &eyePosition = _camera->getEyePosition();
	//float_4 ndc(x+0.5f,y+0.5f,0.0f,1.0f);
	//float_4 target = ndc * ray_mat4;
	//float_3 ray = (*(float_3 *)&target).normalize();
	vec4  ray4 = g_Camera->RayMatrix * vec4(x+0.5f,y+0.5f,0.0f,1.0f);
	vec3  eyePosition = g_Camera->Position;
	float_3 normal = (*(float_3*)&ray4).normalize();
	color = rayTrace(*(float_3*)&eyePosition, normal, nullptr, 0);
}
