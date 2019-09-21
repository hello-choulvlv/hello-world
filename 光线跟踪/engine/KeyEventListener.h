/*
  *键盘按键监听
  *2017-5-26 09:54:47
  *@Author:xiaohuaxiong
*/
#ifndef __KEY_EVENT_LISTENER_H__
#define __KEY_EVENT_LISTENER_H__
#include "Types.h"
#include "EventListener.h"
#include "SceneTracer.h"
class EventManager;
class KeyEventListener :public EventListener
{
	friend class EventManager;
private:
	//事件接收者
	SceneTracer                           *_eventTarget;
	//键盘按键按下之后的回调
	GLKKeyPressCallback  _keyPressCallback;
	//键盘按键松开之后的回调
	GLKKeyReleaseCallback _keyReleaseCallback;
	//按键是否在按键按下之后继续响应按键的松开事件
	bool                                  _isResponseKeyPress;
private:
	KeyEventListener();
	KeyEventListener(KeyEventListener &);
	void   initWithTarget(SceneTracer *target,GLKKeyPressCallback pressCallback,GLKKeyReleaseCallback releaseCallback);
private:
	//EventManager在按键按下之后回调
	void   onKeyPressed(KeyCodeType keyCode);
	//EventManager在按键松开之后回调
	void   onKeyReleased(KeyCodeType keyCode);
public:
	static KeyEventListener *createKeyEventListener(SceneTracer *target, GLKKeyPressCallback pressCallback, GLKKeyReleaseCallback releaseCallback);
	void   registerKeyPressCallback(GLKKeyPressCallback pressCallback);
	void   registerKeyreleaseCallback(GLKKeyReleaseCallback releaseCallback);
	bool  isResponseKeyPress()const;
};
#endif