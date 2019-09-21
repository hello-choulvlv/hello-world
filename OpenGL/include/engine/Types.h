/*
  *有关类型的定义,以及一些枚举类型
  *@date:2017-4-15
  @Author:xiaohuaxiong
 */
#ifndef __TYPES_H__
#define __TYPES_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/event/KeyCode.h"
#include "engine/event/Mouse.h"
//回调函数类型定义,周期回调函数
typedef void (glk::Object::*GLKUpdateCallback)(float );
//触屏起始
typedef bool (glk::Object::*GLKTouchCallback)(const glk::GLVector2 &);
//移动
typedef void (glk::Object::*GLKTouchMotionCallback)(const glk::GLVector2 &);
//结束
typedef void (glk::Object::*GLKTouchReleaseCallback)(const glk::GLVector2 &);

////////////////键盘按键事件回调函数////////////////
typedef bool (glk::Object::*GLKKeyPressCallback)( glk::KeyCodeType keyCode);
typedef void (glk::Object::*GLKKeyReleaseCallback)( glk::KeyCodeType keyCode);

////////////////鼠标事件//////////////////////
typedef bool (glk::Object::*GLKMousePressCallback)( glk::MouseType mouseType,const glk::GLVector2 &position);
typedef void (glk::Object::*GLKMouseMotionCallback)( glk::MouseType mouseType,const glk::GLVector2 &position);
typedef void (glk::Object::*GLKMouseReleaseCallback)( glk::MouseType mouseType,const glk::GLVector2 &position);
//////////////////////////宏/////////////////////////////////////////////
#define  glk_touch_selector(selector)     static_cast<GLKTouchCallback >(&selector)
#define  glk_move_selector(selector)     static_cast<GLKTouchMotionCallback >(&selector)
#define  glk_release_selector(selector)  static_cast<GLKTouchReleaseCallback >(&selector)

//定义简化使用键盘回调函数的宏
#define glk_key_press_selector(selector) static_cast<GLKKeyPressCallback>(&selector)
#define glk_key_release_selector(selector) static_cast<GLKKeyReleaseCallback>(&selector)
//简化鼠标事件回调函数定义
#define glk_mouse_press_selector(selector) static_cast<GLKMousePressCallback>(&selector)
#define glk_mouse_move_selector(selector) static_cast<GLKMouseMotionCallback>(&selector)
#define glk_mouse_release_selector(selector) static_cast<GLKMouseReleaseCallback>(&selector)

///////////////////////////引擎需要用到的一些枚举///////////////////////////
__NS_GLK_BEGIN
enum CameraType
{
	CameraType_None = 0,//无效的类型
	CameraType_Perspertive = 1,//透视矩阵
	CameraType_Ortho =2,//正交矩阵
};
//
typedef unsigned char byte;
__NS_GLK_END
#endif