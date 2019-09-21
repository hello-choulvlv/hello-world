/*
 *2017-5-3
 */
#include<Tools.h>
#include<stdio.h>
_CLK_NS_BEGIN_

const char *Tools::getFileContent(const char *filename)
{
	char      *_buff = NULL;
	FILE      *_file = fopen(filename, "rb");

	if (!_file)
		return    NULL;
	fseek(_file, 0, SEEK_END);
	int      _size = (int)ftell(_file);

	fseek(_file, 0, SEEK_SET);
	_buff = new   char[_size + 2];
	fread(_buff, 1, _size, _file);
	_buff[_size] = '\0';

	fclose(_file);
	_file = NULL;
	return   _buff;
}

bool Tools::isLittleEndine()
{
	int  host = 1;
	const char *p = (char *)&host;
	return *p;
}

_CLK_NS_END_