/*
  *鼠标事件管理
  *2017-05-27
  *@Author:xiaohuaxiong
 */
#ifndef __MOUSE_EVENT_LISTENER_H__
#define __MOUSE_EVENT_LISTENER_H__
#include "Types.h"
#include "Mouse.h"
#include "EventListener.h"
#include "SceneTracer.h"
class EventManager;
class MouseEventListener :public EventListener
{
	friend class EventManager;
private:
	//鼠标事件回调函数
	SceneTracer                                  *_eventTarget;
	GLKMousePressCallback    _mousePressCallback;
	GLKMouseMotionCallback _mouseMoveCallback;
	GLKMouseReleaseCallback _mouseReleaseCallback;
	//是否继续响应鼠标事件
	bool										   _isResponseEvent;
private:
	MouseEventListener();
	MouseEventListener(MouseEventListener &);
	void         initWithTarget(SceneTracer *target,GLKMousePressCallback pressCallback,GLKMouseMotionCallback motionCallback, GLKMouseReleaseCallback releaseCallback);
	//一下函数只能在EventManager中使用
	void         onMousePressed( MouseType mouseType,const float_2 &position);
	void         onMouseMoved( MouseType mouseType,const float_2 &position);
	void         onMouseReleased( MouseType mouseType,const float_2 &position);
	bool         isResponseEvent()const;
public:
	~MouseEventListener();
	static MouseEventListener *createMouseEventListener(SceneTracer *target, GLKMousePressCallback pressCallback, GLKMouseMotionCallback motionCallback, GLKMouseReleaseCallback releaseCallback);
	//重新注册人鼠标事件响应函数
	void         registerMousePressCallback(GLKMousePressCallback pressCallback);
	void         registerMouseMotionCallback(GLKMouseMotionCallback motionCallback);
	void         registerMouseReleaseCallback(GLKMouseReleaseCallback releaseCallback);
};
#endif