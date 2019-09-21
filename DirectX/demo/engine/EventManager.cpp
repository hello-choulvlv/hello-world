/*
  *事件管理器
  *2018/3/15
  *@author:xiaohuaxiong
 */
#include "EventManager.h"

static EventManager *s_eventManagerInstance = nullptr;

EventManager::EventManager()
{
}

EventManager::~EventManager()
{
}

void EventManager::destroy()
{
	for (auto it = _keyListeners.begin(); it != _keyListeners.end(); ++it)
		(*it)->release();
	_keyListeners.clear();

	for (auto it = _touchListeners.begin(); it != _touchListeners.end(); ++it)
		(*it)->release();
	_touchListeners.clear();

	delete this;
	s_eventManagerInstance = nullptr;
}

EventManager  *EventManager::getInstance()
{
	if (!s_eventManagerInstance)
		s_eventManagerInstance = new EventManager();
	return s_eventManagerInstance;
}

void EventManager::dispatchKeyEvent(KeyState keyState, KeyCode keyCode)
{
	if (keyState == KeyState_Pressed)
	{
		bool shouldSwallow = false;
		for (auto it = _keyListeners.begin(); it != _keyListeners.end() && !shouldSwallow; ++it)
		{
			(*it)->onKeyPressed(keyCode);
			shouldSwallow = (*it)->shouldSwallowEvent();
		}
	}
	else if (keyState == KeyState_Released)
	{
		bool shouldSwallow = false;
		for (auto it = _keyListeners.begin(); it != _keyListeners.end() && !shouldSwallow; ++it)
		{
			(*it)->onKeyReleased(keyCode);
			shouldSwallow = (*it)->shouldSwallowEvent();
		}
	}
}

void EventManager::dispatchTouchEvent(TouchState touchState, const Vec2 &touchPoint)
{
	bool shouldSwallow = false;
	if (touchState == TouchState_Pressed)
	{
		for (auto it = _touchListeners.begin(); it != _touchListeners.end() && !shouldSwallow; ++it)
		{
			(*it)->onTouchPressed(touchPoint);
			shouldSwallow = (*it)->shouldSwallowEvent();
		}
	}
	else if (touchState == TouchState_Moved)
	{
		for (auto it = _touchListeners.begin(); it != _touchListeners.end() && !shouldSwallow; ++it)
		{
			(*it)->onTouchMoved(touchPoint);
			shouldSwallow = (*it)->shouldSwallowEvent();
		}
	}
	else if (touchState == TouchState_Released)
	{
		for (auto it = _touchListeners.begin(); it != _touchListeners.end() && !shouldSwallow; ++it)
		{
			(*it)->onTouchReleased(touchPoint);
			shouldSwallow = (*it)->shouldSwallowEvent();
		}
	}
}

void EventManager::addKeyListener(KeyEventListener *keyListener, int priority)
{
	keyListener->setPriority(priority);
	keyListener->retain();
	auto it = _keyListeners.begin();
	for (; it != _keyListeners.end(); ++it)
	{
		KeyEventListener *listener = *it;
		assert(listener != keyListener);
		if (priority < (*it)->getPriority())
			break;
	}
	_keyListeners.insert(it, keyListener);
}

void EventManager::addTouchListener(TouchEventListener *touchListener, int priority)
{
	touchListener->setPriority(priority);
	touchListener->retain();
	auto it = _touchListeners.begin();
	for (; it != _touchListeners.end(); ++it)
	{
		TouchEventListener *listener = *it;
		assert(listener != touchListener);
		if (priority < listener->getPriority())
			break;
	}
	_touchListeners.insert(it, touchListener);
}

void EventManager::removeKeyListener(KeyEventListener *keyListener)
{
	for (auto it = _keyListeners.begin(); it != _keyListeners.end(); ++it)
	{
		if (*it == keyListener)
		{
			_keyListeners.erase(it);
			keyListener->release();
			break;
		}
	}
}

void EventManager::removeTouchListener(TouchEventListener *TouchListener)
{
	for (auto it = _touchListeners.begin(); it != _touchListeners.end(); ++it)
	{
		if (*it == TouchListener)
		{
			_touchListeners.erase(it);
			TouchListener->release();
			break;
		}
	}
}