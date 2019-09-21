/*
*所有事件监听器的父类
*2018/3/15
*@author:xiaohuaxiong
*/
#ifndef __EVENT_LISTENER_H__
#define __EVENT_LISTENER_H__
#include "Object.h"
enum EventType
{
	EventType_None = 0,//无效的事件
	EventType_Key = 1,//键盘事件
	EventType_Touch = 2,//触屏事件
	EventType_Mouse = 3,//鼠标事件
};

class EventListener :public Object
{
	EventType   _eventType;
	int                 _priority;
	bool              _shouldSwallowEvent;
protected:
	EventListener(EventType eventType);

	EventType     getType()const { return _eventType; };

	int                   getPriority()const { return _priority; };
	void                setPriority(int p) { _priority = p; };

	bool   shouldSwallowEvent()const { return _shouldSwallowEvent; };
	void   setSwallowEvent(bool b) { _shouldSwallowEvent = b; };
};
#endif