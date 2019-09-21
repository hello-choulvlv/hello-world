/*
  *���̰�������
  *2017-5-26 09:54:47
  *@Author:xiaohuaxiong
*/
#ifndef __KEY_EVENT_LISTENER_H__
#define __KEY_EVENT_LISTENER_H__
#include "Types.h"
#include "EventListener.h"
#include "SceneTracer.h"
class EventManager;
class KeyEventListener :public EventListener
{
	friend class EventManager;
private:
	//�¼�������
	SceneTracer                           *_eventTarget;
	//���̰�������֮��Ļص�
	GLKKeyPressCallback  _keyPressCallback;
	//���̰����ɿ�֮��Ļص�
	GLKKeyReleaseCallback _keyReleaseCallback;
	//�����Ƿ��ڰ�������֮�������Ӧ�������ɿ��¼�
	bool                                  _isResponseKeyPress;
private:
	KeyEventListener();
	KeyEventListener(KeyEventListener &);
	void   initWithTarget(SceneTracer *target,GLKKeyPressCallback pressCallback,GLKKeyReleaseCallback releaseCallback);
private:
	//EventManager�ڰ�������֮��ص�
	void   onKeyPressed(KeyCodeType keyCode);
	//EventManager�ڰ����ɿ�֮��ص�
	void   onKeyReleased(KeyCodeType keyCode);
public:
	static KeyEventListener *createKeyEventListener(SceneTracer *target, GLKKeyPressCallback pressCallback, GLKKeyReleaseCallback releaseCallback);
	void   registerKeyPressCallback(GLKKeyPressCallback pressCallback);
	void   registerKeyreleaseCallback(GLKKeyReleaseCallback releaseCallback);
	bool  isResponseKeyPress()const;
};
#endif