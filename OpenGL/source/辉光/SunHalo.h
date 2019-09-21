/*
  *光晕实现
  *2017-07-14
  *@Author:xiaohuaxiong
 */
#ifndef __SUN_HALO_H__
#define __SUN_HALO_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/GLTexture.h"
#include "engine/RenderTexture.h"
#include "engine/Shape.h"
#include "engine/event/TouchEventListener.h"

#include "BlurShader.h"
#include "HaloShader.h"
#include "RenderShader.h"
#include "NormalShader.h"
/*
  *实时辉光
 */
class SunHalo :public glk::Object
{
	//两个帧缓冲区对象,交替的使用渲染-->模糊操作
	glk::RenderTexture  *_drawSunTexture;// 画太阳
	// 粗粒度的模糊
	glk::RenderTexture  *_drawThickTexture0;
	glk::RenderTexture  *_drawThickTexture1;
	//细粒度的模糊
	glk::RenderTexture  *_drawPreciseTexture0;
	glk::RenderTexture  *_drawPreciseTexture1;
	//混合
	glk::RenderTexture   *_drawMixTexture;
	//纹理
	glk::GLTexture          *_dirtyTexture;
	//球
	glk::Sphere                 *_sphereMesh;
	//视图矩阵/投影矩阵
	glk::Matrix                  _viewMatrix;
	glk::Matrix                  _projMatrix;
	glk::Matrix                  _viewProjMatrix;
	//Shader
	BlurShader                  *_blurShader;
	HaloShader                 *_haloShader;
	RenderShader             *_renderShader;
	NormalShader            *_normalShader;
	//event
	glk::TouchEventListener   *_touchEventListener;
	/*
	  *分解视图矩阵的时候使用的向量
	 */
	glk::GLVector3           _rotateVec;
	glk::GLVector3           _translateVec;
	//平移过的向量
	glk::GLVector2           _offsetVec;
private:
	SunHalo();
	void				init();
	//初始化视图矩阵/投影矩阵
	void            initCamera(const glk::GLVector3 &eyePosition,const glk::GLVector3 &targetPosition);
public:
	~SunHalo();
	static		   SunHalo *create();
	void            update(float dt);
	void            draw();
	/*
	  *画太阳
	 */
	void           drawSunTexture();
	/*
	  *粗粒度模糊帧缓冲区对象
	 */
	void           drawThickTexture();
	/*
	  *细粒度模糊帧缓冲区对象
	 */
	void          drawPreciseTexture();
	/*
	  *混合
	 */
	void         drawMixTexture();
	/*
	  *直通
	 */
	void        drawNormalTexture();
	/*
	  *测试函数,将渲染后的纹理画出来
	*/
	void        drawTexture(int textureId);
	/*
	  *触屏回调函数
	 */
	bool       onTouchBegan(const glk::GLVector2 *touchPoint);
	void       onTouchMoved(const glk::GLVector2 *touchPoint);
	void       onTouchEnded(const glk::GLVector2 *touchPoint);
};
#endif