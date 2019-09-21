/*
  *DirectX �Ķ������������,�Լ���صĺ�,ȫ��ʹ�õ����ݽṹ
  *@2018��3��15��
  *author:xiaohuaxiong
 */
#ifndef __DX_TYPES_H__
#define __DX_TYPES_H__
#include "Scene.h"
#include "Geometry.h"
//�����¼�
enum  KeyCode
{
	KeyCode_0 = 48,
	KeyCode_1 = 49,
	KeyCode_2 = 50,
	KeyCode_3 = 51,
	KeyCode_4 = 52,
	KeyCode_5 = 53,
	KeyCode_6 = 54,
	KeyCode_7 = 55,
	KeyCode_8 = 56,
	KeyCode_9 = 57,

	KeyCode_A = 65,
	KeyCode_B = 66,
	KeyCode_C = 67,
	KeyCode_D = 68,
	KeyCode_E = 69,
	KeyCode_F = 70,
	KeyCode_G = 71,
	KeyCode_H = 72,
	KeyCode_I = 73,
	KeyCode_J = 74,
	KeyCode_K = 75,
	KeyCode_L = 76,
	KeyCode_M = 77,
	KeyCode_N = 78,
	KeyCode_O  = 79,
	KeyCode_P = 80,
	KeyCode_Q = 81,
	KeyCode_R = 82,
	KeyCode_S = 83,
	KeyCode_T = 84,
	KeyCode_U = 85,
	KeyCode_V = 86,
	KeyCode_W = 87,
	KeyCode_X = 88,
	KeyCode_Y = 89,
	KeyCode_Z = 90,

	//���ⰴ��
	KeyCode_SPACE = 92,
	KeyCode_TAB = 93,
	KeyCode_SHIFT = 94,
	KeyCode_CTRL = 95,
	KeyCode_ALT = 96,

	KeyCode_LEFT = 97,
	KeyCode_UP = 98,
	KeyCode_RIGHT = 99,
	KeyCode_DOWN = 100,
	//function key
	KeyCode_F1  = 101,
	KeyCode_F2 = 102,
	KeyCode_F3 = 103,
	KeyCode_F4 = 104,
	KeyCode_F5 = 105,
	KeyCode_F6 = 106,
	KeyCode_F7 = 107,
	KeyCode_F8 = 108,
	KeyCode_F9 = 109,
	KeyCode_F10 = 110,
	KeyCode_F11 = 111,
	KeyCode_F12 = 112,
};
//������״̬
enum  KeyState
{
	KeyState_None       = 0,
	KeyState_Pressed   = 1,
	KeyState_Released = 2,
};
//���İ�������
enum MouseKeyCode
{
	MouseKeyCode_Left = 0,
	MouseKeyCode_Middle = 1,
	MouseKeyCode_Right = 2,
};
//��갴����״̬
enum MouseState
{
	MouseState_None = 0,
	MouseState_Pressed = 1,
	MouseState_Moved = 2,
	MouseState_Released = 3,
};
//����״̬
enum TouchState
{
	TouchState_None = 0,
	TouchState_Pressed = 1,
	TouchState_Moved = 2,
	TouchState_Released = 3,
};
//�����������
enum CameraType
{
	CameraType_None             =0,//��Ч������
	CameraType_Perspective = 1,//͸��ͶӰ
	CameraType_Ortho            =2,//����ͶӰ
};
//�ص���������
typedef  bool  (Scene::*TouchPressedCallback)(const Vec2 &touchPoint);
typedef void   (Scene::*TouchMovedCallback)(const Vec2  &touchPoint);
typedef void   (Scene::*TouchReleasedCallback)(const Vec2 &touchPoint);

typedef void (Scene::*KeyPressedCallback)(KeyCode keyCode);
typedef void (Scene::*KeyReleasedCallback)(KeyCode keyCode);
//
#define touch_pressed_callback(selector)   static_cast<TouchPressedCallback>(&selector)
#define touch_moved_callback(selector)     static_cast<TouchMovedCallback>(&selector)
#define touch_released_callback(selector)  static_cast<TouchReleasedCallback>(&selector)

#define key_pressed_callback(selector)       static_cast<KeyPressedCallback>(&selector)
#define key_released_callback(selector)		static_cast<KeyReleasedCallback>(&selector)

//���ݾ������/����
#define float_pointer(x)    reinterpret_cast<float*>(&(x))
//����Ƕ�ʱʹ��
#define RADIANS_FACTOR  57.2957795133f
#define angle_of_radians(x)  (x)*57.2957795133f
#define radians_os_angle(x)  (x)/57.2957795133f
#endif