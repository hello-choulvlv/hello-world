/*
  *OpenGL �������,�����¼�����
  *@date:2017-4-15
  *@Author:xiaohuaxiong
 */
#ifndef __EVENT_MANAGER_H__
#define __EVENT_MANAGER_H__
#include "engine/GLState.h"
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/event/KeyCode.h"
#include "engine/event/Mouse.h"
#include "engine/event/Touch.h"
#include "engine/event/TouchEventListener.h"
#include "engine/event/KeyEventListener.h"
#include "engine/event/MouseEventListener.h"
#include<vector>
#include<map>
void _static_mousePressCallback(int, int, int, int);
void  _static_mouseMotionCallback(int, int);
__NS_GLK_BEGIN
class EventManager
{
	friend void _static_mousePressCallback(int, int, int, int);
	friend void _static_mouseMotionCallback(int, int);
	friend void _static_keyPressCallback(unsigned char,int,int);
	friend void _static_keySpecialPressCallback(int keyCode, int x, int y);
	friend void _static_keyReleaseCallback(unsigned char, int, int);
	friend void _static_keySpecialReleaseCallback(int keyCode, int x, int y);
	friend class GLContext;
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
	void        dispatchTouchEvent(TouchState  touchState,const GLVector2 &touchPoint);
	//�ɷ�����¼�
	void        dispatchMouseEvent(MouseType mouseType,MouseState mouseState,const GLVector2 &mousePoint);
	//�ɷ������¼�
	void        dispatchKeyEvent(KeyCodeType keyCode,KeyCodeState keyState);
public:
	~EventManager();
	static EventManager   *getInstance();
	//ɾ����ĳһ��������ص��¼�
	void        removeListener(Object *);
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

__NS_GLK_END
#endif