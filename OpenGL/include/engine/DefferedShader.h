/*
  *�ӳ���ɫ
  *2017��12��22��
  *@Author:xiaohuaxiong
 */
#ifndef __DEFFERED_SHADER_H__
#define __DEFFERED_SHADER_H__
#include "Object.h"
#include "Geometry.h"
__NS_GLK_BEGIN
class DefferedShader :public Object
{
	//֡����������
	unsigned    _framebufferId;
	int                _lastFramebufferId;
	//��ɫ����������,���ܲ�ֹһ��,�������Ŀ��Ҫ�ɴ����������
	unsigned    _colorbufferId[4];
	int               _colorbufferCount;
	//��Ȼ���������
	unsigned    _depthbufferId;
	//ģ�建��������
	unsigned    _stenclebufferId;
	//��ǰ��λ��ʶ
	int              _bufferBit;
	//����������ĳߴ�
	Size           _framebufferSize;
	//�Ƿ���ÿ�ΰ󶨵�ǰ��֡��������ʱ��������ָ��ŵĻ�����
	bool          _cleanBuffer;
	//�Ƿ���Ҫ�ڽ�����ʱ��ָ��ϴε�֡�����������
	bool          _needRestoreLastFramebuffer;
private:
	DefferedShader();
	DefferedShader(const DefferedShader &);
	bool     init(const Size &framebufferSize,int colorbufferCount);
public:
	~DefferedShader();
	static DefferedShader *create(const Size &framebufferSize,int colorbufferCount);
	/*
	  *���ǰ��֡����������
	 */
	void   active();
	/*
	  *�ָ�ԭ����֡�����������
	 */
	void   restore();
	void  setRestoreLastFramebuffer(bool b) { _needRestoreLastFramebuffer = b; };
	bool  isRestoreLastFramebuffer()const {return _needRestoreLastFramebuffer;};
	/*
	  *��ȡ��ɫ����������
	 */
	void  getColorBuffer(unsigned *colorbuffer,int *bufferCount)const;
	//���ص�һ����ɫ����������
	unsigned getColorBuffer()const;
	//��ȡ��Ȼ���������
	unsigned getDepthBuffer()const { return _depthbufferId; };
	/*
	  *�����������־
	 */
	bool  isCleanBuffer()const { return _cleanBuffer; };
	void  setCleanBuffer(bool b) { _cleanBuffer = b; };
	//framebuffer size
	const Size &getFramebufferSize()const { return _framebufferSize; };
};
__NS_GLK_END
#endif