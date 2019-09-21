/*
  *����¼�����
  *2017-05-27
  *@Author:xiaohuaxiong
 */
#ifndef __MOUSE_EVENT_LISTENER_H__
#define __MOUSE_EVENT_LISTENER_H__
#include "Types.h"
#include "Mouse.h"
#include "EventListener.h"
#include "SceneTracer.h"
class EventManager;
class MouseEventListener :public EventListener
{
	friend class EventManager;
private:
	//����¼��ص�����
	SceneTracer                                  *_eventTarget;
	GLKMousePressCallback    _mousePressCallback;
	GLKMouseMotionCallback _mouseMoveCallback;
	GLKMouseReleaseCallback _mouseReleaseCallback;
	//�Ƿ������Ӧ����¼�
	bool										   _isResponseEvent;
private:
	MouseEventListener();
	MouseEventListener(MouseEventListener &);
	void         initWithTarget(SceneTracer *target,GLKMousePressCallback pressCallback,GLKMouseMotionCallback motionCallback, GLKMouseReleaseCallback releaseCallback);
	//һ�º���ֻ����EventManager��ʹ��
	void         onMousePressed( MouseType mouseType,const float_2 &position);
	void         onMouseMoved( MouseType mouseType,const float_2 &position);
	void         onMouseReleased( MouseType mouseType,const float_2 &position);
	bool         isResponseEvent()const;
public:
	~MouseEventListener();
	static MouseEventListener *createMouseEventListener(SceneTracer *target, GLKMousePressCallback pressCallback, GLKMouseMotionCallback motionCallback, GLKMouseReleaseCallback releaseCallback);
	//����ע��������¼���Ӧ����
	void         registerMousePressCallback(GLKMousePressCallback pressCallback);
	void         registerMouseMotionCallback(GLKMouseMotionCallback motionCallback);
	void         registerMouseReleaseCallback(GLKMouseReleaseCallback releaseCallback);
};
#endif