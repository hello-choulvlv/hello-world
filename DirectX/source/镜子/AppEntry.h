/*
  *��������,���г���ĳ�ʼ�������ٶ���������ﾭ��
  *@2018��3��1��
  *@Author:xiaohuaxiong
*/
#ifndef __APP_ENTRY_H__
#define __APP_ENTRY_H__
class AppEntry
{
	int   _appId;
public:
	AppEntry();
	//��Ӧ�ó����ʼ��ʱ����
	void      onCreateComplete();
	//��Ӧ�ó����л�����̨ʱ����
	void      onEnterBackground();
	//Ӧ�ó���Ӻ�̨���ص�ǰ̨ʱ����
	void      onEnterForeground();
	//Ӧ�ó�������ʱ����
	void      onDestroy();
	~AppEntry();
};
#endif