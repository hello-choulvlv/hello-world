/*
*�����¼��������ĸ���
*2018/3/15
*@author:xiaohuaxiong
*/
#ifndef __EVENT_LISTENER_H__
#define __EVENT_LISTENER_H__
#include "Object.h"
enum EventType
{
	EventType_None = 0,//��Ч���¼�
	EventType_Key = 1,//�����¼�
	EventType_Touch = 2,//�����¼�
	EventType_Mouse = 3,//����¼�
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