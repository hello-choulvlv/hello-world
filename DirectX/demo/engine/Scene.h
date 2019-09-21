/*
  *所有场景的渲染的基类,所有的需要在DirectX上渲染的类必须继承该类
  *@2018年3月1日
  *@Author:xiaohuaxiong
 */
#ifndef __SCENE_H__
#define __SCENE_H__
#include "Object.h"
#include<assert.h>
struct ID3D10Device;
class Scene : public Object
{
protected:
	ID3D10Device   *_device;
public:
	Scene();
	//每帧调用,参数的含义为离上一帧经过了了多长时间
	virtual   void   update(float  dt) = 0;
	//渲染时调用
	virtual  void    render() = 0;
	virtual              ~Scene() {};
};
#endif