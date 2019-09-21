/*
  *��Ⱦ������,����֡�����������Լ��丽��
  *ע��,����ֻ��������ͨ��Ⱦ��������,��������MRT,Deffer shader
  *@date:2017-06-27
  *@Author:xiaohuaxiong
  @Version2.0:����ɫ�������ĸ�ʽ��Ϊһ�ֿ�ѡ��Ĳ�����ȡ����
 */
#ifndef __RENDER_TENTURE_H__
#define __RENDER_TEXTURE_H__
#include "engine/GLState.h"
#include "engine/Object.h"
#include "engine/Geometry.h"
__NS_GLK_BEGIN
class RenderTexture :public glk::Object
{ 
public:
	//����֡����������ļ�����־,����ʹ�� | ���������
	enum RenderType
	{
		ColorBuffer = 0x1,//��Ҫ��ɫ������
		DepthBuffer = 0x2,//��Ҫ��Ȼ�����
		StencilBuffer = 0x4,//��Ҫģ�建����
		TotalBuffer = ColorBuffer | DepthBuffer | StencilBuffer,//ʹ�����еĻ�����
	};
	//������ɫ����������ĸ�ʽ��ѡ������
	enum ColorFormatType
	{
		ColorFormatType_R8               = 0,//ʹ����ɫ���GL_RED8
		ColorFormatType_RG8            =1,//ʹ����ɫGL_RG8
		ColorFormatType_RGB8         = 2,//ʹ����ɫGL_RG8
		ColorFormatType_RGBA8      = 3,//ʹ����ɫGL_RGBA8

		ColorFormatType_R_HALF = 4,//ʹ�ð븡����ɫGL_HALF8
		ColorFormatType_RG_HALF = 5,//ʹ�ð븡��GL_RG_HALF
		ColorFormatType_RGB_HALF = 6,//ʹ�ð븡��GL_RG_HALF
		ColorFormatType_RGBA_HALF = 7,//ʹ�ð븡��GL_RGBA_HALF

		ColorFormatType_R_FLOAT  =8,//ȫ����GL_R_FLOAT
		ColorFormatType_RG_FLOAT =9,//GL_RG_FLOAT
		ColorFormatType_RGB_FLOAT = 10,//GL_RGB_FLOAT
		ColorFormatType_RGBA_FLOAT = 11,//GL_RGBA_FLOAT
	};
	//������Ȼ���������ʹ�õĸ�ʽ
	enum DepthFormatType
	{
		DepthFormatType_Normal   = 0,//ʹ�ó���ĸ�ʽGL_DEPTH_COMPONENT24,
		DepthFormatType_32            =1,//ʹ�ø�ʽGL_DEPTH_COMPONENT32����
		DepthFormatType_32F			 = 2,//ʹ�ø�ʽGL_DEPTH_COMPONENT32F
	};
private:
	unsigned      _framebufferId;//֡����������
	unsigned      _colorbufferId;//��ɫ����������
	unsigned      _depthbufferId;//��Ȼ���������
	unsigned      _stencilbufferId;//ģ�建��������
	int			      _lastFramebufferId;
	Size               _frameSize;//֡�����������С
	bool              _isRestoreLastFramebuffer;//�ڽ�����ǰ�������İ󶨵�ʱ��,�Ƿ���Ҫ��ԭ��ԭ���Ļ�������
	bool              _isNeedClearBuffer;//�Ƿ���Ҫ��ʹ�õ�ǰ֡�����������ʱ��ͬʱ�����ɫ,���,ģ�建����,������˵Ļ�
	int                 _bufferBit;
private:
	bool             initWithRender(const Size &frameSize,unsigned  genType, ColorFormatType colorType, DepthFormatType depthType);
	bool             initWithRender(const Size &frameSize, unsigned  genType);
	RenderTexture();
	RenderTexture(RenderTexture &);
public:
	~RenderTexture();
	/*
	  *����֡����������,��Ҫ�����������Ĵ�С,�����ı�־
	  *��ҪRenderTypeö������
	 */
	static RenderTexture *createRenderTexture(const Size &frameSize,unsigned genType);
	/*
	  *�Ը����ĸ�ʽ����֡����������,formatTable�ĳ�����genType����������������Ӧ
	  *����˳������ɫ,���,ģ���˳���趨,���ĳһ��������û��,����Ľ����Ÿ���
	 */
	static RenderTexture *createRenderTexture(const Size &frameSize, unsigned genType, ColorFormatType colorType, DepthFormatType depthType);
	/*
	  *�л�����ǰ�Ļ���������,��ǰ������������¼�ϴε�֡�����������ʹ�� ���
	 */
	void    activeFramebuffer();
	/*
	  *��ֹ��ǰ֡��������
	 */
	void    disableFramebuffer()const;
	/*
	  *�����Ƿ���Ҫ��ԭ��ǰ�Ļ����������
	 */
	void    setRestoreLastFramebuffer(bool isRestore);
	/*
	  *�����Ƿ���Ҫ��ԭ��ǰ��֡����������
	 */
	bool   isRestoreLastFramebuffer()const;
	/*
	  *�Ƿ���Ҫ�ڰ󶨵�ǰ�����������ʱ�������صĻ�����
	 */
	void   setClearBuffer(bool b);
	/*
	  *�����Ƿ��ڰ󶨵�ǰ�����������ʱ��ͬʱ�������������
	 */
	bool isClearBuffer()const;
	/*
	  *���ص�ǰ����ɫ����������,ע�������ʹ����û��������صı�־,����ֵ��Ϊ0
	 */
	unsigned getColorBuffer()const;
	/*
	  *������Ȼ���������
	 */
	unsigned getDepthBuffer()const;
};
__NS_GLK_END
#endif