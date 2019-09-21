/*
  *延迟着色
  *2017年12月22日
  *@Author:xiaohuaxiong
 */
#ifndef __DEFFERED_SHADER_H__
#define __DEFFERED_SHADER_H__
#include "Object.h"
#include "Geometry.h"
__NS_GLK_BEGIN
class DefferedShader :public Object
{
	//帧缓冲区对象
	unsigned    _framebufferId;
	int                _lastFramebufferId;
	//颜色缓冲区对象,可能不止一个,具体的数目需要由传入参数决定
	unsigned    _colorbufferId[4];
	int               _colorbufferCount;
	//深度缓冲区对象
	unsigned    _depthbufferId;
	//模板缓冲区对象
	unsigned    _stenclebufferId;
	//当前的位标识
	int              _bufferBit;
	//缓冲区对象的尺寸
	Size           _framebufferSize;
	//是否在每次绑定当前的帧缓冲对象的时候都清理各种附着的缓冲区
	bool          _cleanBuffer;
	//是否需要在结束的时候恢复上次的帧缓冲区对象绑定
	bool          _needRestoreLastFramebuffer;
private:
	DefferedShader();
	DefferedShader(const DefferedShader &);
	bool     init(const Size &framebufferSize,int colorbufferCount);
public:
	~DefferedShader();
	static DefferedShader *create(const Size &framebufferSize,int colorbufferCount);
	/*
	  *激活当前的帧缓冲区对象
	 */
	void   active();
	/*
	  *恢复原来的帧缓冲区对象绑定
	 */
	void   restore();
	void  setRestoreLastFramebuffer(bool b) { _needRestoreLastFramebuffer = b; };
	bool  isRestoreLastFramebuffer()const {return _needRestoreLastFramebuffer;};
	/*
	  *获取颜色缓冲区对象
	 */
	void  getColorBuffer(unsigned *colorbuffer,int *bufferCount)const;
	//返回第一个颜色缓冲区对象
	unsigned getColorBuffer()const;
	//获取深度缓冲区对象
	unsigned getDepthBuffer()const { return _depthbufferId; };
	/*
	  *缓冲区清除标志
	 */
	bool  isCleanBuffer()const { return _cleanBuffer; };
	void  setCleanBuffer(bool b) { _cleanBuffer = b; };
	//framebuffer size
	const Size &getFramebufferSize()const { return _framebufferSize; };
};
__NS_GLK_END
#endif