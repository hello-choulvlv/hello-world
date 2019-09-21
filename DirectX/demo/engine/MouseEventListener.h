/*
  *Êó±êÊÂ¼þ
  *2018/3/15
  *@author:xiaohuaxiong
 */
#ifndef __MOUSE_EVENT_LISTENER_H__
#define __MOUSE_EVENT_LISTENER_H__
#include "dxTypes.h"
#include "EventListener.h"
class EventManager;
class TouchEventListener : public EventListener
{
	friend class EventManager;
	Scene									*_target;
	TouchPressedCallback   _touchPressedcallback;
	TouchMovedCallback     _touchMovedCallback;
	TouchReleasedCallback  _touchReelasedCallback;
	bool                                     _shouldResponseEvent;
	int                                        _priority;
private:
	explicit TouchEventListener();
	TouchEventListener(const TouchEventListener &);
	void   initWithTarget(Scene *target,TouchPressedCallback, TouchMovedCallback,TouchReleasedCallback);
	
	void   onTouchPressed(const Vec2 &touchPoint);
	void   onTouchMoved(const Vec2 &touchPoint);
	void   onTouchReleased(const Vec2 &touchPoint);
public:
	~TouchEventListener();
	static TouchEventListener *create(Scene *target, TouchPressedCallback, TouchMovedCallback, TouchReleasedCallback);

	bool   shouldResponseEvent()const { return _shouldResponseEvent; };

};
#endif

