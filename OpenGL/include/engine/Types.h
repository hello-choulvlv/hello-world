/*
  *�й����͵Ķ���,�Լ�һЩö������
  *@date:2017-4-15
  @Author:xiaohuaxiong
 */
#ifndef __TYPES_H__
#define __TYPES_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/event/KeyCode.h"
#include "engine/event/Mouse.h"
//�ص��������Ͷ���,���ڻص�����
typedef void (glk::Object::*GLKUpdateCallback)(float );
//������ʼ
typedef bool (glk::Object::*GLKTouchCallback)(const glk::GLVector2 &);
//�ƶ�
typedef void (glk::Object::*GLKTouchMotionCallback)(const glk::GLVector2 &);
//����
typedef void (glk::Object::*GLKTouchReleaseCallback)(const glk::GLVector2 &);

////////////////���̰����¼��ص�����////////////////
typedef bool (glk::Object::*GLKKeyPressCallback)( glk::KeyCodeType keyCode);
typedef void (glk::Object::*GLKKeyReleaseCallback)( glk::KeyCodeType keyCode);

////////////////����¼�//////////////////////
typedef bool (glk::Object::*GLKMousePressCallback)( glk::MouseType mouseType,const glk::GLVector2 &position);
typedef void (glk::Object::*GLKMouseMotionCallback)( glk::MouseType mouseType,const glk::GLVector2 &position);
typedef void (glk::Object::*GLKMouseReleaseCallback)( glk::MouseType mouseType,const glk::GLVector2 &position);
//////////////////////////��/////////////////////////////////////////////
#define  glk_touch_selector(selector)     static_cast<GLKTouchCallback >(&selector)
#define  glk_move_selector(selector)     static_cast<GLKTouchMotionCallback >(&selector)
#define  glk_release_selector(selector)  static_cast<GLKTouchReleaseCallback >(&selector)

//�����ʹ�ü��̻ص������ĺ�
#define glk_key_press_selector(selector) static_cast<GLKKeyPressCallback>(&selector)
#define glk_key_release_selector(selector) static_cast<GLKKeyReleaseCallback>(&selector)
//������¼��ص���������
#define glk_mouse_press_selector(selector) static_cast<GLKMousePressCallback>(&selector)
#define glk_mouse_move_selector(selector) static_cast<GLKMouseMotionCallback>(&selector)
#define glk_mouse_release_selector(selector) static_cast<GLKMouseReleaseCallback>(&selector)

///////////////////////////������Ҫ�õ���һЩö��///////////////////////////
__NS_GLK_BEGIN
enum CameraType
{
	CameraType_None = 0,//��Ч������
	CameraType_Perspertive = 1,//͸�Ӿ���
	CameraType_Ortho =2,//��������
};
//
typedef unsigned char byte;
__NS_GLK_END
#endif