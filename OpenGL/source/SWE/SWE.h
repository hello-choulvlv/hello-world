/*
  *浅水方程实现
  *2018年12月28日
  *@author:xiaohuaxiong
 */
#ifndef __SWE_H__
#define __SWE_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"
#include "engine/GLTexture.h"
#include "engine/event/TouchEventListener.h"
#include "Liquid.h"

class SWE : public glk::Object
{
	Liquid	 _liquid;
	//纹理对象
	unsigned  _normalTexture;
	glk::GLProgram	*_glProgram;
	glk::GLTexture    *_mainTexture, *_secondaryTexture;

	glk::TouchEventListener  *_touchEventListener;
	//shader uniform location
	int  _mainTextureLoc;
	int  _secondaryTextureLoc;
	int  _normalTextureLoc;
public:
	SWE();
	~SWE();
	void initSWE();
	/*
	  *触屏事件
	 */
	bool  onTouchBegan(const glk::GLVector2 &touchPoint);
	void  onTouchMoved(const glk::GLVector2 &touchPoint);
	void  onTouchEnded(const glk::GLVector2 &touchPoint);
	/*
	  *渲染
	 */
	void  render();
	/*
	  *每帧刷新
	 */
	void  update(float t);
};
#endif