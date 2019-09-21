/*
  *键盘按键编码以及按键的状态等定义
  *2017-5-26 09:59:51
  *@Author:xiaohuaxiong
 */
#ifndef __KEY_CODE_H__
#define __KEY_CODE_H__
#include "engine/GLState.h"
__NS_GLK_BEGIN
//键盘按键的类型,目前支持的类型并不全面
enum KeyCodeType
{
	KeyCode_NONE = 0,//Invalid Key
	//Normal Key
	KeyCode_A = 1,//A
	KeyCode_B = 2, //B
	KeyCode_C = 3,//C
	KeyCode_D = 4,//D
	KeyCode_E = 5,//E
	KeyCode_F=6,//E
	KeyCode_G=7,//G
	KeyCode_H=8,
	KeyCode_I=9,
	KeyCode_J=10,
	KeyCode_K=11,
	KeyCode_L=12,
	KeyCode_M=13,
	KeyCode_N=14,
	KeyCode_O=15,
	KeyCode_P=16,
	KeyCode_Q=17,
	KeyCode_R=18,
	KeyCode_S=19,
	KeyCode_T=20,
	KeyCode_U=21,
	KeyCode_V=22,
	KeyCode_W=23,
	KeyCode_X=24, 
	KeyCode_Y=25,
	KeyCode_Z=26,
	//Assist Key
	KeyCode_LEFT =33,
	KeyCode_RIGHT = 34,
	KeyCode_UP = 35,
	KeyCode_DOWN = 36,
	KeyCode_ALT = 37,
	KeyCode_CTRL = 38,//Ctrl
	KeyCode_SHIFT = 39,//Shift
	//Function Key
	KeyCode_F1 = 41,//F1
	KeyCode_F2=42,//F2
	KeyCode_F3=43,//F3
	KeyCode_F4=44,
	KeyCode_F5=45,
	KeyCode_F6=46,
	KeyCode_F7=47,
	KeyCode_F8=48,
	KeyCode_F9=49,
	KeyCode_F10=50,
	KeyCode_F11=51,
	KeyCode_F12=52,
	//Other Key
	KeyCode_TAB = 53,//table
	KeyCode_SPACE = 54,//Space
	KeyCode_ENTER = 55,//enter key
	//Number
	KeyCode_0=60,
	KeyCode_1,//61
	KeyCode_2=62,
	KeyCode_3=63,
	KeyCode_4=64,
	KeyCode_5=65,
	KeyCode_6=66,
	KeyCode_7=67,
	KeyCode_8=68,
	KeyCode_9=69,
};
//键盘按键的状态 
enum KeyCodeState
{
	KeyCodeState_None=0,//无效的按键状态
	KeyCodeState_Pressed=1,//键盘按键按下
	KeyCodeState_Released=2,//键盘按键被释放
};
__NS_GLK_END
#endif