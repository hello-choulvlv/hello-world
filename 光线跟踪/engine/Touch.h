/*
  *触屏事件
  *2017-5-27
  *@Author:xiaohuaxiong
 */
#ifndef __TOUCH_H__
#define __TOUCH_H__

enum TouchState
{
	TouchState_None=0,//无效的触屏状态
	TouchState_Pressed=1,//触屏按下
	TouchState_Moved=2,//触屏移动
	TouchState_Released=3,//触屏结束
};

#endif
