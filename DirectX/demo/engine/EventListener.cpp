/*
 *EventListener
 *2018/3/15
 *@author:xiaohuaxiong
 */
#include "EventListener.h"

EventListener::EventListener(EventType eventType) :
	_eventType(eventType)
	,_priority(0)
	, _shouldSwallowEvent(false)
{
}