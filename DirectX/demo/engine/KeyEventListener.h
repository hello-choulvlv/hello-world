/*
  *¼üÅÌÊÂ¼þ¼àÌýÆ÷
  *2018/3/15
  *@author:xiaohuaxiong
 */
#ifndef __KEY_EVENT_LISTENER_H__
#define __KEY_EVENT_LISTENER_H__
#include "Scene.h"
#include "dxTypes.h"
#include "EventListener.h"
class EventManager;
class KeyEventListener : public EventListener
{
	friend class EventManager;
	Scene                            *_target;
	KeyPressedCallback   _keyPressedCallback;
	KeyReleasedCallback _keyReleasedCallback;
private:
	explicit KeyEventListener();
	KeyEventListener(const KeyEventListener &);
	void      initWithTarget(Scene *target,KeyPressedCallback  keyPressedcallback,KeyReleasedCallback keyReleasedCallback);

	void    onKeyPressed(KeyCode  keyCode);
	void    onKeyReleased(KeyCode keyCode);
public:
	~KeyEventListener();
	static KeyEventListener  *create(Scene *target, KeyPressedCallback  keyPressedcallback, KeyReleasedCallback keyReleasedCallback);
};
#endif