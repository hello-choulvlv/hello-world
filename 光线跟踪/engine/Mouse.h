/*
  *����¼�,�����Լ�������״̬
  *2017-05-27
  *@Author:xiaohuaxiong
 */
#ifndef __MOUSE_H__
#define __MOUSE_H__
enum MouseType
{
	MouseType_None=0,//��Ч����갴��
	MouseType_Left = 1,//���
	MouseType_Middle = 2,//�м�
	MouseType_Right = 3,//�ʼ�
};

enum MouseState
{
	MouseState_None=0,//��Ч��״̬
	MouseState_Pressed=1,//��걻����
	MouseState_Moved = 2,//����������ƶ�����
	MouseState_Released=3,//��걻�ͷ�
};
#endif