/*
  *OpenGL �������,�����¼�����
  *@date:2017-4-15
  *@Author:xiaohuaxiong
 */
#ifndef __EVENT_MANAGER_H__
#define __EVENT_MANAGER_H__
#include "Geometry.h"
#include "KeyCode.h"
#include "Mouse.h"
#include "Touch.h"
#include "TouchEventListener.h"
#include "KeyEventListener.h"
#include "MouseEventListener.h"
#include<vector>
#include<map>

class EventManager
{
private:
	//�����¼��ļ���
	std::vector<TouchEventListener *>		     _touchEventArrays;
	std::map<TouchEventListener *,int>      _touchEventPriority;
	//�����¼�����
	std::vector<KeyEventListener*>               _keyEventArrays;
	std::map<KeyEventListener*, int>            _keyEventPriority;
	//����¼�����
	std::vector<MouseEventListener *>          _mouseEventArrays;
	std::map<MouseEventListener*, int>        _mouseEventPriority;
	//�Ƿ������������¼��ɷ�֮��
	bool                   _isInTouchEventDispatch;
	//�Ƿ������ڼ����¼��ɷ���
	bool                   _isInKeyEventDispatch;
	//�Ƿ�����������¼��ɷ���
	bool                   _isInMouseEventDispatch;
private:

	EventManager();
	EventManager(EventManager &);
	static EventManager   _static_eventManager;
//friend
	//�ɷ������¼�
	void        dispatchTouchEvent(TouchState  touchState,const float_2 &touchPoint);
	//�ɷ�����¼�
	void        dispatchMouseEvent(MouseType mouseType,MouseState mouseState,const float_2 &mousePoint);
	//�ɷ������¼�
	void        dispatchKeyEvent(KeyCodeType keyCode,KeyCodeState keyState);
public:
	~EventManager();
	static EventManager   *getInstance();
	//ɾ����ĳһ��������ص��¼�
	void        removeListener(SceneTracer *);
	//ɾ��ĳһ���¼�
	void        removeListener(EventListener *eventListener);
	//����¼�������
	//@param:priority�������ȼ�,����ֵԽС����ʾ�����¼��Ĵ���Խ��ǰ
	void        addTouchEventListener(TouchEventListener *touchEvent,int priority);
	//�����¼����������
	void        addKeyEventListener(KeyEventListener *keyEvent,int priority);
	//����¼����������
	void        addMouseEventListener(MouseEventListener *mouseEvent,int priority);
};

#endif