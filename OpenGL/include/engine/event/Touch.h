/*
  *�����¼�
  *2017-5-27
  *@Author:xiaohuaxiong
 */
#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "engine/GLState.h"
__NS_GLK_BEGIN

enum TouchState
{
	TouchState_None=0,//��Ч�Ĵ���״̬
	TouchState_Pressed=1,//��������
	TouchState_Moved=2,//�����ƶ�
	TouchState_Released=3,//��������
};

__NS_GLK_END
#endif
