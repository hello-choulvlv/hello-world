/*
  *程序的入口,所有程序的初始化与销毁都将会从这里经过
  *@2018年3月1日
  *@Author:xiaohuaxiong
*/
#ifndef __APP_ENTRY_H__
#define __APP_ENTRY_H__
class AppEntry
{
	int   _appId;
public:
	AppEntry();
	//当应用程序初始化时调用
	void      onCreateComplete();
	//当应用程序切换到后台时调用
	void      onEnterBackground();
	//应用程序从后台返回到前台时调用
	void      onEnterForeground();
	//应用程序销毁时调用
	void      onDestroy();
	~AppEntry();
};
#endif