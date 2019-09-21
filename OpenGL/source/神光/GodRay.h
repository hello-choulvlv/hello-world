/*
 *神光,体积光线,在此程序中,为了简化一些操作,作者没有实现键盘事件
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
	unsigned			_twistBufferId;//螺旋线顶点集合
	unsigned        _twistIndexId;//索引缓冲区对象
	int                   _vertexSize;
	int                   _indexSize;
	RayShader     *_rayShader;
	RenderShader *_renderShader;
	//关于视图矩阵,投影矩阵等的相关数据
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
	  *颜色
	 */
	glk::GLVector4   _color;
	//渲染到纹理
	glk::RenderTexture   *_rtt;
	//触屏事件
	glk::TouchEventListener *_touchEventListener;
private:
	GodRay();
	GodRay(GodRay &);
	void    initGodRay();
public:
	~GodRay();
	static GodRay *createGodRay();
	//加载螺旋线对象
	void    loadTwistObject();
	//加载视图投影矩阵
	void    initCamera(const glk::GLVector3 &eyePosition,const glk::GLVector3 &targetPosition);
	/*
	  *渲染
	 */
	void    draw();
	/*
	  *事实更新
	 */
	void    update(float deltaTime);
	//合成视图矩阵
	void    buildeViewMatrix();
	/*
	  *触屏事件
	 */
	bool   onTouchBegan(const glk::GLVector2 *touchPoint);
	void   onTouchMoved(const glk::GLVector2 *touchPoint);
	void   onTouchEnded(const glk::GLVector2  *touchPoint);
};
#endif