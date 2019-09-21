/*
  *Êó±êÊÂ¼þ¼àÌýÆ÷
  *2018/3/15
  *@author:xiaohuaxiong
 */
#include "MouseEventListener.h"

TouchEventListener::TouchEventListener():EventListener(EventType_Touch)
	,_target(nullptr)
	,_touchPressedcallback(nullptr)
	,_touchMovedCallback(nullptr)
	,_touchReelasedCallback(nullptr)
	,_shouldResponseEvent(false)	
{
}

TouchEventListener::~TouchEventListener()
{
}

void TouchEventListener::initWithTarget(Scene *target, TouchPressedCallback touchPressed, TouchMovedCallback touchMoved, TouchReleasedCallback touchReleased)
{
	_target = target;
	_touchPressedcallback = touchPressed;
	_touchMovedCallback = touchMoved;
	_touchReelasedCallback = touchReleased;
}

TouchEventListener    *TouchEventListener::create(Scene *target, TouchPressedCallback touchPressed, TouchMovedCallback touchMoved, TouchReleasedCallback touchReleased)
{
	TouchEventListener * eventListener = new TouchEventListener();
	eventListener->initWithTarget(target, touchPressed, touchMoved, touchReleased);
	return eventListener;
}

void TouchEventListener::onTouchPressed(const Vec2 &touchPoint)
{
	if (_touchPressedcallback)
		_shouldResponseEvent = (_target->*_touchPressedcallback)(touchPoint);
}

void TouchEventListener::onTouchMoved(const Vec2 &touchPoint)
{
	if (_touchMovedCallback && _shouldResponseEvent)
		(_target->*_touchMovedCallback)(touchPoint);
}

void TouchEventListener::onTouchReleased(const Vec2 &touchPoint)
{
	if (_touchReelasedCallback)
		(_target->*_touchReelasedCallback)(touchPoint);
}