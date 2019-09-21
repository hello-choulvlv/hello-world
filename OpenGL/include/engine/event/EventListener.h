/*
*�����¼���ĸ���
*@2017-5-26 12:38:06
*@Author:xiaohuaxiong
*/
#ifndef __EVENT_LISTENER_H__
#define __EVENT_LISTENER_H__
#include "engine/GLState.h"
#include"engine/Object.h"
__NS_GLK_BEGIN
enum EventType
{
	EventType_None=0,//��Ч���¼���
	EventType_Touch=1,//������
	EventType_Mouse=2,//����¼���
	EventType_Key=3,//�����¼�
	EventType_Custum=4,//�û��Զ�����
};

class EventListener :public Object
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
__NS_GLK_END

#endif