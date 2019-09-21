/*
  *触屏事件,只对鼠标的左键进行监听
  *@date:2017-4-15
  *@Author:xiaohuaxiong
 */
#ifndef __TOUCH_EVENT_LISTENER_H__
#define __TOUCH_EVENT_LISTENER_H__
#include "engine/Object.h"
#include "engine/Types.h"
#include "engine/Geometry.h"
#include "engine/event/EventListener.h"
__NS_GLK_BEGIN
class EventManager;
class TouchEventListener :public EventListener
{
	friend class EventManager;
private:
	//是否吞噬事件，当其标志为true时，低于其优先级的事件将接收不到触屏事件
	bool       _isSwallowEvent;
	//是否响应了触屏按下的事件
	bool      _isInteractTouch;
	//事件的接收者
	Object    *_eventTarget;
	//三个函数指针
	GLKTouchCallback     _touchBegan;
	GLKTouchMotionCallback _touchMotion;
	GLKTouchReleaseCallback _touchRelease;
private:
	TouchEventListener();
	TouchEventListener(TouchEventListener &);
	void   initWithTarget(Object *eventTarget);
	void   initWithTarget(Object *eventTarget, GLKTouchCallback touchBegin, GLKTouchMotionCallback touchMoved, GLKTouchReleaseCallback touchRelease);

	//派发事件
	bool    onTouchBegin(const GLVector2 &touchPoint);
	void    onTouchMoved(const GLVector2 &touchPoint);
	void    onTouchEnded(const GLVector2 &touchPoint);
public:
	~TouchEventListener();
	static TouchEventListener *createTouchListener(Object *eventTarget);
	static TouchEventListener *createTouchListener(Object *eventTarget,GLKTouchCallback touchBegin, GLKTouchMotionCallback touchMoved, GLKTouchReleaseCallback touchRelease);

	//是否吞噬触屏事件
	void    setSwallowTouch(bool );
	bool    isSwallowTouch()const;
	//是否响应触屏移动，结束事件
	bool    isRespondTouch()const;
	//获取事件派发者的地址
	Object *getEventTarget()const;
	//设置
	void    registerTouchCallback(GLKTouchCallback touchBegin);
	void    registerMotionCallback(GLKTouchMotionCallback touchMoved);
	void    registerReleaseCallback(GLKTouchReleaseCallback touchRelease);
};
__NS_GLK_END
#endif