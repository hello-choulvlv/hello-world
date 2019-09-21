/*
  *���̰�������
  *2017-5-26 09:54:47
  *@Author:xiaohuaxiong
*/
#ifndef __KEY_EVENT_LISTENER_H__
#define __KEY_EVENT_LISTENER_H__
#include "engine/GLState.h"
#include "engine/Object.h"
#include "engine/Types.h"
#include "engine/event/EventListener.h"
__NS_GLK_BEGIN
class EventManager;
class KeyEventListener :public EventListener
{
	friend class EventManager;
private:
	//�¼�������
	Object                           *_eventTarget;
	//���̰�������֮��Ļص�
	GLKKeyPressCallback  _keyPressCallback;
	//���̰����ɿ�֮��Ļص�
	GLKKeyReleaseCallback _keyReleaseCallback;
	//�����Ƿ��ڰ�������֮�������Ӧ�������ɿ��¼�
	bool                                  _isResponseKeyPress;
private:
	KeyEventListener();
	KeyEventListener(KeyEventListener &);
	void   initWithTarget(Object *target,GLKKeyPressCallback pressCallback,GLKKeyReleaseCallback releaseCallback);
private:
	//EventManager�ڰ�������֮��ص�
	void   onKeyPressed(KeyCodeType keyCode);
	//EventManager�ڰ����ɿ�֮��ص�
	void   onKeyReleased(KeyCodeType keyCode);
public:
	static KeyEventListener *createKeyEventListener(Object *target, GLKKeyPressCallback pressCallback, GLKKeyReleaseCallback releaseCallback);
	void   registerKeyPressCallback(GLKKeyPressCallback pressCallback);
	void   registerKeyreleaseCallback(GLKKeyReleaseCallback releaseCallback);
	bool  isResponseKeyPress()const;
};
__NS_GLK_END
#endif