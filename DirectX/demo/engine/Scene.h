/*
  *���г�������Ⱦ�Ļ���,���е���Ҫ��DirectX����Ⱦ�������̳и���
  *@2018��3��1��
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
	//ÿ֡����,�����ĺ���Ϊ����һ֡�������˶೤ʱ��
	virtual   void   update(float  dt) = 0;
	//��Ⱦʱ����
	virtual  void    render() = 0;
	virtual              ~Scene() {};
};
#endif