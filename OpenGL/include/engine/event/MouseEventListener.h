/*
  *����¼�����
  *2017-05-27
  *@Author:xiaohuaxiong
 */
#ifndef __MOUSE_EVENT_LISTENER_H__
#define __MOUSE_EVENT_LISTENER_H__
#include "engine/Types.h"
#include "engine/event/Mouse.h"
#include "engine/event/EventListener.h"
__NS_GLK_BEGIN
class EventManager;
class MouseEventListener :public EventListener
{
	friend class EventManager;
private:
	//����¼��ص�����
	Object                                  *_eventTarget;
	GLKMousePressCallback    _mousePressCallback;
	GLKMouseMotionCallback _mouseMoveCallback;
	GLKMouseReleaseCallback _mouseReleaseCallback;
	//�Ƿ������Ӧ����¼�
	bool										   _isResponseEvent;
private:
	MouseEventListener();
	MouseEventListener(MouseEventListener &);
	void         initWithTarget(Object *target,GLKMousePressCallback pressCallback,GLKMouseMotionCallback motionCallback, GLKMouseReleaseCallback releaseCallback);
	//һ�º���ֻ����EventManager��ʹ��
	void         onMousePressed( MouseType mouseType,const GLVector2 &position);
	void         onMouseMoved( MouseType mouseType,const GLVector2 &position);
	void         onMouseReleased( MouseType mouseType,const GLVector2 &position);
	bool         isResponseEvent()const;
public:
	~MouseEventListener();
	static MouseEventListener *createMouseEventListener(Object *target, GLKMousePressCallback pressCallback, GLKMouseMotionCallback motionCallback, GLKMouseReleaseCallback releaseCallback);
	//����ע��������¼���Ӧ����
	void         registerMousePressCallback(GLKMousePressCallback pressCallback);
	void         registerMouseMotionCallback(GLKMouseMotionCallback motionCallback);
	void         registerMouseReleaseCallback(GLKMouseReleaseCallback releaseCallback);
};
__NS_GLK_END
#endif