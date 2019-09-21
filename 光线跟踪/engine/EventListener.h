/*
*所有事件类的父类
*@2017-5-26 12:38:06
*@Author:xiaohuaxiong
*/
#ifndef __EVENT_LISTENER_H__
#define __EVENT_LISTENER_H__
#include "Object.h"
enum EventType
{
	EventType_None=0,//无效的事件类
	EventType_Touch=1,//触屏类
	EventType_Mouse=2,//鼠标事件类
	EventType_Key=3,//键盘事件
	EventType_Custum=4,//用户自定义类
};
class EventListener : public Object
{
private:
	const EventType   _eventType;
public:
	EventListener(EventType eventType) :_eventType(eventType)
	{
	}
	inline EventType getEventType()const
	{
		return _eventType;
	}
};
#endif