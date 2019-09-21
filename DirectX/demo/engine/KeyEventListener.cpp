/*
  *¼üÅÌÊÂ¼þ¼àÌýÆ÷
  *2018/3/15
  *@author:xiaohuaxiong
 */
#include "KeyEventListener.h"

KeyEventListener::KeyEventListener() : EventListener(EventType_Key)
	,_target(nullptr)
	,_keyPressedCallback(nullptr)
	,_keyReleasedCallback(nullptr)
{
}

KeyEventListener::~KeyEventListener()
{

}

void KeyEventListener::initWithTarget(Scene *target, KeyPressedCallback keyPressedcallback, KeyReleasedCallback keyReleasedCallback)
{
	_target = target;
	_keyPressedCallback = keyPressedcallback;
	_keyReleasedCallback = keyReleasedCallback;
}

KeyEventListener *KeyEventListener::create(Scene *target, KeyPressedCallback keyPressedcallback, KeyReleasedCallback keyReleasedCallback)
{
	KeyEventListener *eventListener = new KeyEventListener();
	eventListener->initWithTarget(target, keyPressedcallback, keyReleasedCallback);
	return eventListener;
}

void  KeyEventListener::onKeyPressed(KeyCode keyCode)
{
	if (_keyPressedCallback)
		(_target->*_keyPressedCallback)(keyCode);
}

void KeyEventListener::onKeyReleased(KeyCode keyCode)
{
	if (_keyPressedCallback)
		(_target->*_keyReleasedCallback)(keyCode);
}