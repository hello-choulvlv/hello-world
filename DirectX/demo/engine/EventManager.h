/*
  *�¼�������
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

	//�ɷ������¼�
	void    dispatchKeyEvent(KeyState  keyState,KeyCode  keyCode);
	//�ɷ�����¼�
	void    dispatchTouchEvent(TouchState  touchState,const Vec2 &touchPoint);
	//���¼����뵽������
	void    addKeyListener(KeyEventListener  *keyListener,int priority);
	void    addTouchListener(TouchEventListener *touchListener,int priority);
	//�Ӷ������Ƴ����¼�������
	void    removeKeyListener(KeyEventListener *keyListener);
	void    removeTouchListener(TouchEventListener *TouchListener);

	void    destroy();
};
#endif