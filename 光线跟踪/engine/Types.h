/*
  *�й����͵Ķ���,�Լ�һЩö������
  *@date:2017-4-15
  @Author:xiaohuaxiong
 */
#ifndef __TYPES_H__
#define __TYPES_H__
#include "Geometry.h"
#include "KeyCode.h"
#include "Mouse.h"
#include "SceneTracer.h"
//�ص��������Ͷ���,���ڻص�����
typedef void (SceneTracer::*GLKUpdateCallback)(float );
//������ʼ
typedef bool (SceneTracer::*GLKTouchCallback)(const float_2 &);
//�ƶ�
typedef void (SceneTracer::*GLKTouchMotionCallback)(const float_2 &);
//����
typedef void (SceneTracer::*GLKTouchReleaseCallback)(const float_2 &);

////////////////���̰����¼��ص�����////////////////
typedef bool (SceneTracer::*GLKKeyPressCallback)( KeyCodeType keyCode);
typedef void (SceneTracer::*GLKKeyReleaseCallback)( KeyCodeType keyCode);

////////////////����¼�//////////////////////
typedef bool (SceneTracer::*GLKMousePressCallback)( MouseType mouseType,const float_2 &position);
typedef void (SceneTracer::*GLKMouseMotionCallback)( MouseType mouseType,const float_2 &position);
typedef void (SceneTracer::*GLKMouseReleaseCallback)( MouseType mouseType,const float_2 &position);
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
enum CameraType
{
	CameraType_None = 0,//��Ч������
	CameraType_Perspertive = 1,//͸�Ӿ���
	CameraType_Ortho =2,//��������
};
//
typedef unsigned char byte;
#endif