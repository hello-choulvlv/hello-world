/*
  *鼠标事件管理实现
  *2017-5-27
  *@Author:xiaohuaxiong
 */
#include "engine/event/MouseEventListener.h"
__NS_GLK_BEGIN
MouseEventListener::MouseEventListener():EventListener(EventType_Mouse)
{
	_eventTarget = nullptr;
	_mousePressCallback = nullptr;
	_mouseMoveCallback = nullptr;
	_mouseReleaseCallback = nullptr;
	_isResponseEvent = false;
}

MouseEventListener::~MouseEventListener()
{

}

void MouseEventListener::initWithTarget(Object *target, GLKMousePressCallback pressCallback, GLKMouseMotionCallback motionCallback, GLKMouseReleaseCallback releaseCallback)
{
	_eventTarget = target;
	_mousePressCallback = pressCallback;
	_mouseMoveCallback = motionCallback;
	_mouseReleaseCallback = releaseCallback;
}

MouseEventListener *MouseEventListener::createMouseEventListener(Object *target, GLKMousePressCallback pressCallback, GLKMouseMotionCallback motionCallback, GLKMouseReleaseCallback releaseCallback)
{
	MouseEventListener *mouse = new MouseEventListener();
	mouse->initWithTarget(target, pressCallback,motionCallback ,releaseCallback);
	return mouse;
}

void MouseEventListener::registerMousePressCallback(GLKMousePressCallback pressCallback)
{
	_mousePressCallback = pressCallback;
}

void MouseEventListener::registerMouseMotionCallback(GLKMouseMotionCallback motionCallback)
{
	_mouseMoveCallback = motionCallback;
}

void MouseEventListener::registerMouseReleaseCallback(GLKMouseReleaseCallback releaseCallback)
{
	_mouseReleaseCallback = releaseCallback;
}

bool MouseEventListener::isResponseEvent()const
{
	return _isResponseEvent;
}

void  MouseEventListener::onMousePressed( MouseType mouseType,const GLVector2 &position)
{
	_isResponseEvent = false;
	if (_mousePressCallback)
		_isResponseEvent=(_eventTarget->*_mousePressCallback)(mouseType,position);
}
void MouseEventListener::onMouseMoved( MouseType mouseType, const GLVector2 &position)
{
	if (_isResponseEvent && _mouseMoveCallback)
		(_eventTarget->*_mouseMoveCallback)(mouseType,position);
}

void MouseEventListener::onMouseReleased( MouseType mouseType, const GLVector2 &position)
{
	if (_isResponseEvent && _mouseReleaseCallback)
		(_eventTarget->*_mouseReleaseCallback)(mouseType,position);
}

__NS_GLK_END