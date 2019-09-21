/*
  *有关类型的定义,以及一些枚举类型
  *@date:2017-4-15
  @Author:xiaohuaxiong
 */
#ifndef __TYPES_H__
#define __TYPES_H__
#include "Geometry.h"
#include "KeyCode.h"
#include "Mouse.h"
#include "SceneTracer.h"
//回调函数类型定义,周期回调函数
typedef void (SceneTracer::*GLKUpdateCallback)(float );
//触屏起始
typedef bool (SceneTracer::*GLKTouchCallback)(const float_2 &);
//移动
typedef void (SceneTracer::*GLKTouchMotionCallback)(const float_2 &);
//结束
typedef void (SceneTracer::*GLKTouchReleaseCallback)(const float_2 &);

////////////////键盘按键事件回调函数////////////////
typedef bool (SceneTracer::*GLKKeyPressCallback)( KeyCodeType keyCode);
typedef void (SceneTracer::*GLKKeyReleaseCallback)( KeyCodeType keyCode);

////////////////鼠标事件//////////////////////
typedef bool (SceneTracer::*GLKMousePressCallback)( MouseType mouseType,const float_2 &position);
typedef void (SceneTracer::*GLKMouseMotionCallback)( MouseType mouseType,const float_2 &position);
typedef void (SceneTracer::*GLKMouseReleaseCallback)( MouseType mouseType,const float_2 &position);
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
enum CameraType
{
	CameraType_None = 0,//无效的类型
	CameraType_Perspertive = 1,//透视矩阵
	CameraType_Ortho =2,//正交矩阵
};
//
typedef unsigned char byte;
#endif