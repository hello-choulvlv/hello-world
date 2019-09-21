/*
  *键盘按键侦听类实现
  *2017-5-26 10:25:00
  *@Author:xiaohuaxiong
*/
#include "KeyEventListener.h"
KeyEventListener::KeyEventListener() :EventListener(EventType_Key)
{
	_eventTarget = nullptr;
	_keyPressCallback = nullptr;
	_keyReleaseCallback = nullptr;
	_isResponseKeyPress = false;
}

void KeyEventListener::initWithTarget(SceneTracer *target, GLKKeyPressCallback pressCallback, GLKKeyReleaseCallback releaseCallback)
{
	_eventTarget = target;
	_keyPressCallback = pressCallback;
	_keyReleaseCallback = releaseCallback;
}

KeyEventListener *KeyEventListener::createKeyEventListener(SceneTracer *target, GLKKeyPressCallback pressCallback, GLKKeyReleaseCallback releaseCallback)
{
	KeyEventListener *eventListener = new KeyEventListener();
	eventListener->initWithTarget(target, pressCallback, releaseCallback);
	return eventListener;
}

void KeyEventListener::registerKeyPressCallback(GLKKeyPressCallback pressCallback)
{
	_keyPressCallback = pressCallback;
}

void KeyEventListener::registerKeyreleaseCallback(GLKKeyReleaseCallback releaseCallback)
{
	_keyReleaseCallback = releaseCallback;
}

bool KeyEventListener::isResponseKeyPress()const
{
	return _isResponseKeyPress;
}

void KeyEventListener::onKeyPressed(KeyCodeType keyCode)
{
	_isResponseKeyPress = false;
	if (_keyPressCallback)
		_isResponseKeyPress = (_eventTarget->*_keyPressCallback)(keyCode);
}

void KeyEventListener::onKeyReleased(KeyCodeType keyCode)
{
	if (_isResponseKeyPress && _keyReleaseCallback)
	{
		(_eventTarget->*_keyReleaseCallback)(keyCode);
	}
}