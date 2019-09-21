/*
  *事件管理器
  *2018/3/15
  *@author:xiaohuaxiong
 */
#ifndef __EVENT_MANAGER_H__
#define __EVENT_MANAGER_H__
#include <vector>
#include "KeyEventListener.h"
#include "MouseEventListener.h"
class EventManager
{
	std::vector<KeyEventListener *>       _keyListeners;
	std::vector<TouchEventListener*>   _touchListeners;
private:
	explicit  EventManager();
	EventManager(const EventManager &);
public:
	~EventManager();
	static EventManager *getInstance();

	//派发键盘事件
	void    dispatchKeyEvent(KeyState  keyState,KeyCode  keyCode);
	//派发鼠标事件
	void    dispatchTouchEvent(TouchState  touchState,const Vec2 &touchPoint);
	//将事件加入到队列中
	void    addKeyListener(KeyEventListener  *keyListener,int priority);
	void    addTouchListener(TouchEventListener *touchListener,int priority);
	//从队列中移除掉事件监听器
	void    removeKeyListener(KeyEventListener *keyListener);
	void    removeTouchListener(TouchEventListener *TouchListener);

	void    destroy();
};
#endif