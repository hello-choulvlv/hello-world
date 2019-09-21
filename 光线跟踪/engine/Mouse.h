/*
  *鼠标事件,按键以及按键的状态
  *2017-05-27
  *@Author:xiaohuaxiong
 */
#ifndef __MOUSE_H__
#define __MOUSE_H__
enum MouseType
{
	MouseType_None=0,//无效的鼠标按键
	MouseType_Left = 1,//左键
	MouseType_Middle = 2,//中键
	MouseType_Right = 3,//邮件
};

enum MouseState
{
	MouseState_None=0,//无效的状态
	MouseState_Pressed=1,//鼠标被按下
	MouseState_Moved = 2,//鼠标正处于移动当中
	MouseState_Released=3,//鼠标被释放
};
#endif