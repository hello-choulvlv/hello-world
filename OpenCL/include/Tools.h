/*
  *工具类的集合
  *2017-5-3
  *@Version:1.0
  *@Author:xiaohuaxiong
 */
#include<clMicros.h>
_CLK_NS_BEGIN_
class Tools
{
private:
	Tools();
	Tools(Tools &);
	~Tools();
public:
	//获取文件的内容
	static const char *getFileContent(const char *filename);
	//检测宿主机是否是小尾顺序
	static bool   isLittleEndine();
};
_CLK_NS_END_