/*
  *������ļ���
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
	//��ȡ�ļ�������
	static const char *getFileContent(const char *filename);
	//����������Ƿ���Сβ˳��
	static bool   isLittleEndine();
};
_CLK_NS_END_