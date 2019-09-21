/*
  *ǳˮ����ʵ��
  *2018��12��28��
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
	//�������
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
	  *�����¼�
	 */
	bool  onTouchBegan(const glk::GLVector2 &touchPoint);
	void  onTouchMoved(const glk::GLVector2 &touchPoint);
	void  onTouchEnded(const glk::GLVector2 &touchPoint);
	/*
	  *��Ⱦ
	 */
	void  render();
	/*
	  *ÿ֡ˢ��
	 */
	void  update(float t);
};
#endif