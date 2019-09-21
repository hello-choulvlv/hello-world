/*
  *触屏事件实现
  *2017-4-15
  *@author:xiaohuaxiong
 */
#include<engine/event/TouchEventListener.h>
#include<assert.h>
#include<stdio.h>
__NS_GLK_BEGIN

TouchEventListener::TouchEventListener() :EventListener(EventType_Touch)
{
	_isSwallowEvent = false;
	_isInteractTouch = false;
	_eventTarget = nullptr;

	_touchBegan = nullptr;
	_touchMotion = nullptr;
	_touchRelease = nullptr;
}

TouchEventListener::~TouchEventListener()
{
	_eventTarget = nullptr;
}

void TouchEventListener::initWithTarget(Object *eventTarget, GLKTouchCallback touchBegin, GLKTouchMotionCallback touchMoved, GLKTouchReleaseCallback touchRelease)
{
	_eventTarget = eventTarget;
	_touchBegan = touchBegin;
	_touchMotion = touchMoved;
	_touchRelease = touchRelease;
}

void TouchEventListener::initWithTarget(Object *eventTarget)
{
	_eventTarget = eventTarget;
}

TouchEventListener *TouchEventListener::createTouchListener(Object *eventTarget, GLKTouchCallback touchBegin, GLKTouchMotionCallback touchMoved, GLKTouchReleaseCallback touchRelease)
{
	TouchEventListener *touchEvent = new TouchEventListener();
	touchEvent->initWithTarget(eventTarget, touchBegin, touchMoved, touchRelease);
	return touchEvent;
}

TouchEventListener *TouchEventListener::createTouchListener(Object *eventTarget)
{
	TouchEventListener *touchEvent = new TouchEventListener();
	touchEvent->initWithTarget(eventTarget);
	return touchEvent;
}

void TouchEventListener::setSwallowTouch(bool  b)
{
	_isSwallowEvent = b;
}

bool TouchEventListener::isSwallowTouch()const
{
	return _isSwallowEvent;
}

bool TouchEventListener::isRespondTouch()const
{
	return _isInteractTouch;
}

Object *TouchEventListener::getEventTarget()const
{
	return _eventTarget;
}
//register callback

void TouchEventListener::registerTouchCallback(GLKTouchCallback touchBegin)
{
	_touchBegan = touchBegin;
}

void TouchEventListener::registerMotionCallback(GLKTouchMotionCallback touchMoved)
{
	_touchMotion = touchMoved;
}

void TouchEventListener::registerReleaseCallback(GLKTouchReleaseCallback touchRelease)
{
	_touchRelease = touchRelease;
}

//事件派发
bool TouchEventListener::onTouchBegin(const GLVector2 &touchPoint)
{
	_isInteractTouch = false;
	if (_touchBegan)
		_isInteractTouch = (_eventTarget->*_touchBegan)(touchPoint);
	return _isInteractTouch;
}

void TouchEventListener::onTouchMoved(const GLVector2 &touchPoint)
{
	//只有经过了触发事件标志通过之后才能调用该函数
	if (_isInteractTouch && _touchMotion)
	{
		(_eventTarget->*_touchMotion)(touchPoint);
	}
}

void TouchEventListener::onTouchEnded(const GLVector2 &touchPoint)
{
	if (_isInteractTouch && _touchRelease)
	{
		(_eventTarget->*_touchRelease)(touchPoint);
	}
}


__NS_GLK_END