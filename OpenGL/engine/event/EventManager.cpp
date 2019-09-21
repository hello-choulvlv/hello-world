/*
  *事件管理实现
  *@date:2017-4-15
  *@Author:xiaohuaxiong
 */
#include<engine/event/EventManager.h>
#include<assert.h>
__NS_GLK_BEGIN

EventManager     EventManager::_static_eventManager;

EventManager::EventManager()
{
	_touchEventArrays.reserve(64);
	_keyEventArrays.reserve(64);
	_mouseEventArrays.reserve(64);

	_isInTouchEventDispatch = false;
	_isInKeyEventDispatch = false;
	_isInMouseEventDispatch = false;
}
//释放所有的被注册的事件管理器
EventManager::~EventManager()
{
	std::vector<TouchEventListener *>::iterator it = _touchEventArrays.begin();
	for (; it != _touchEventArrays.end(); ++it)
	{
		(*it)->release();
	}
	_touchEventArrays.clear();
	_touchEventPriority.clear();
	//键盘事件清理
	std::vector<KeyEventListener *>::iterator it2 = _keyEventArrays.begin();
	for (; it2 != _keyEventArrays.end(); ++it2)
	{
		(*it2)->release();
	}
	_keyEventArrays.clear();
	_keyEventPriority.clear();
	//鼠标事件队列清理
	std::vector<MouseEventListener*>::iterator it3 = _mouseEventArrays.begin();
	for (; it3 != _mouseEventArrays.end(); ++it3)
	{
		(*it3)->release();
	}
	_mouseEventArrays.clear();
	_mouseEventPriority.clear();
}

EventManager *EventManager::getInstance()
{
	return &_static_eventManager;
}

void EventManager::dispatchTouchEvent(TouchState touchState, const GLVector2 &touchPoint)
{
	if (!_touchEventArrays.size())
		return;
	_isInTouchEventDispatch = true;
	TouchEventListener *targetEvent = nullptr;
	int      nowSize = 0;
	int      lastSize;
	//检测是否有触屏标志
		//首先派发触屏事件
		std::vector<TouchEventListener *>::iterator it = _touchEventArrays.begin();
		//注意,在事件派发的过程中，有可能会添加/删除事件,目前这个功能暂时不实现,带到哥有时间的时候再说
		if (touchState == TouchState_Pressed)
		{
			for (; it != _touchEventArrays.end(); ++it)
			{
				targetEvent = *it;
				lastSize = _touchEventArrays.size();
				//检测是否被释放，或者
				const bool interact = targetEvent->onTouchBegin(touchPoint);
				//如果有反馈,且事件被吞噬
				if (interact && targetEvent->isSwallowTouch())
					break;
				//检测是否中间产生了删除监听器事件,重新定位
			}
		}
		else if (touchState == TouchState_Moved)
		{
			for (; it != _touchEventArrays.end(); ++it)
			{
				(*it)->onTouchMoved(touchPoint);
			}
		}
		else if (touchState == TouchState_Released)
		{
			for (; it != _touchEventArrays.end(); ++it)
			{
				(*it)->onTouchEnded(touchPoint);
			}
		}
	_isInTouchEventDispatch = false;
}

void EventManager::dispatchMouseEvent(MouseType mouseType, MouseState mouseState, const GLVector2 &mousePoint)
{
	if (!_mouseEventArrays.size())
		return;
	_isInMouseEventDispatch = true;
	std::vector<MouseEventListener*>::iterator it = _mouseEventArrays.begin();
	if (mouseState == MouseState_Pressed)
	{
		for (; it != _mouseEventArrays.end(); ++it)
			(*it)->onMousePressed(mouseType, mousePoint);
	}
	else if (mouseState == MouseState_Moved)
	{
		for (; it != _mouseEventArrays.end(); ++it)
			(*it)->onMouseMoved(mouseType, mousePoint);
	}
	else if (mouseState == MouseState_Released)
	{
		for (; it != _mouseEventArrays.end(); ++it)
			(*it)->onMouseReleased(mouseType, mousePoint);
	}
	_isInMouseEventDispatch = false;
}

void EventManager::dispatchKeyEvent(KeyCodeType keyCode,KeyCodeState keyState)
{
	if (!_keyEventArrays.size())
		return;
	_isInKeyEventDispatch = true;
	std::vector<KeyEventListener*>::iterator it = _keyEventArrays.begin();
	if (keyState == KeyCodeState_Pressed)
	{
		for (; it != _keyEventArrays.end(); ++it)
		{
			(*it)->onKeyPressed(keyCode);
		}
	}
	else if (keyState == KeyCodeState_Released)
	{
		for (; it != _keyEventArrays.end();++it)
		{
				(*it)->onKeyReleased(keyCode);
		}
	}
	_isInKeyEventDispatch = false;
}

void EventManager::removeListener(Object *obj)
{
	std::vector<TouchEventListener *>::iterator  it = _touchEventArrays.begin();
	for (; it != _touchEventArrays.end();)
	{
		Object *targetObject = (*it)->getEventTarget();
		if (targetObject == obj)
		{
			(*it)->release();
			_touchEventPriority.erase(*it);
			it = _touchEventArrays.erase(it);
		}
		else
			++it;
	}
}

void EventManager::removeListener(EventListener *eventListener)
{
	const EventType eventType = eventListener->getEventType();
	if (eventType == EventType_Touch)
	{
		TouchEventListener *touchEvent = (TouchEventListener *)eventListener;
		std::map<TouchEventListener*, int>::iterator it = _touchEventPriority.find(touchEvent);
		if (it == _touchEventPriority.end())
			return;
		for (std::vector<TouchEventListener*>::iterator nit = _touchEventArrays.begin(); nit != _touchEventArrays.end(); ++nit)
		{
			if (*nit == touchEvent)
			{
				touchEvent->release();
				_touchEventArrays.erase(nit);
				_touchEventPriority.erase(it);
				break;
			}
		}
	}
	else if (eventType == EventType_Key)
	{
		KeyEventListener *keyEvent = (KeyEventListener *)eventListener;
		std::map<KeyEventListener *, int>::iterator it = _keyEventPriority.find(keyEvent);
		if (it == _keyEventPriority.end())
			return;
		for (std::vector<KeyEventListener*>::iterator nit = _keyEventArrays.begin(); nit != _keyEventArrays.end(); ++nit)
		{
			if (*nit == keyEvent)
			{
				keyEvent->release();
				_keyEventArrays.erase(nit);
				_keyEventPriority.erase(it);
				break;
			}
		}
	}
	else if (eventType == EventType_Mouse)
	{
		MouseEventListener *mouseEvent = (MouseEventListener*)eventListener;
		std::map<MouseEventListener*, int>::iterator it = _mouseEventPriority.find(mouseEvent);
		if (it == _mouseEventPriority.end())
			return;
		for (std::vector<MouseEventListener*>::iterator it2 = _mouseEventArrays.begin(); it2 != _mouseEventArrays.end(); ++it2)
		{
			if (*it2 == mouseEvent)
			{
				mouseEvent->release();
				_mouseEventArrays.erase(it2);
				_mouseEventPriority.erase(it);
				break;
			}
		}
	}
}

void EventManager::addTouchEventListener(TouchEventListener *touchEvent,int priority)
{
	//检测是否重复添加了
	std::map<TouchEventListener *, int>::iterator itt = _touchEventPriority.find(touchEvent);
	assert(itt == _touchEventPriority.end());
	//计算索引次序
	std::vector<TouchEventListener *>::iterator it = _touchEventArrays.begin();
	for (; it != _touchEventArrays.end(); ++it)
	{
		int nowPriority = _touchEventPriority[*it];
		if (priority <= nowPriority)
			break;
	}
	touchEvent->retain();
	_touchEventArrays.insert(it, touchEvent);
	_touchEventPriority[touchEvent] = priority;
}

void EventManager::addKeyEventListener(KeyEventListener *keyEvent, int priority)
{
	//检测是否重复添加了
	std::map<KeyEventListener*, int>::iterator it = _keyEventPriority.find(keyEvent);
	if (it != _keyEventPriority.end())
		return;
	std::vector<KeyEventListener*>::iterator	itof = _keyEventArrays.begin();
	for (; itof != _keyEventArrays.end(); ++itof)
	{
		int other_priority = _keyEventPriority[*itof];
		if (priority<=other_priority)
			break;
	}
	keyEvent->retain();
	_keyEventArrays.insert(itof,keyEvent);
	_keyEventPriority[keyEvent] = priority;
}

void EventManager::addMouseEventListener(MouseEventListener *mouseEvent, int priority)
{
	//检测是否重复添加了
	std::map<MouseEventListener*, int>::iterator it = _mouseEventPriority.find(mouseEvent);
	if (it != _mouseEventPriority.end())
		return;
	std::vector<MouseEventListener*>::iterator it2 = _mouseEventArrays.begin();
	for (; it2 != _mouseEventArrays.end(); ++it2)
	{
		const int oldPriority = _mouseEventPriority[*it2];
		if (priority <= oldPriority)
			break;
	}
	_mouseEventArrays.insert(it2,mouseEvent);
	_mouseEventPriority[mouseEvent] = priority;
}
__NS_GLK_END