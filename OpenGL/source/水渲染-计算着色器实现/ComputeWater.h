/*
  *由计算着色器实现的水渲染
  *2017-7-25
  *@Author:xiaohuaxiong
 */
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/GLProgram.h"
#include "engine/Camera.h"
#include "engine/Shape.h"
#include "engine/GLTexture.h"

#include "engine/event/TouchEventListener.h"

#include "WaterComputeShader.h"
#include "WaterNormalShader.h"
#include "RenderShader.h"

class ComputeWater :public glk::Object
{
	//网格
	glk::Mesh			*_waterMesh;
	//网格的大小
	const int               _meshSize;
	//每网格单元的大小
	const  int              _meshUnitSize;
	//立方体贴图
	glk::GLCubeMap  *_texCubeMap;
	//Shader
	WaterComputeShader    *_computeWaterShader;
	WaterNormalShader      *_computeNormalShader;
	RenderShader                 *_renderShader;
	//水渲染参数
	glk::GLVector4                 _waterParam;
	glk::Matrix                       _mMatrix;//模型矩阵
	glk::GLVector4                _waterColor;
	//菲涅尔系数
	glk::GLVector3                _freshnelParam;
	/*
	  *shader buffer
	 */
	unsigned              _heightFieldBuffer;
	unsigned              _velocityFieldBuffer;
	unsigned              _outHeightFieldBuffer;
	unsigned              _normalFieldBuffer;
	/*
	  *摄像机
	 */
	glk::Camera        *_camera;
	//引入触屏事件
	glk::TouchEventListener   *_touchEventListener;
	glk::GLVector2                    _offsetVec;//触屏的偏移量
	/*
	  *键盘事件
	 */
	glk::KeyEventListener       *_keyEventListener;
	int                                         _keyMask;//键盘按键掩码
	//当前时间累积
	float                     _deltaTime;
private:
	ComputeWater();
	ComputeWater(const ComputeWater &);
	void      init();
public:
	~ComputeWater();
	static ComputeWater  *create();
	/*
	  *生成Shader Buffer对象
	 */
	void    initShaderBuffer();
	/*
	  *初始化关于水渲染的uniform参数
	 */
	void   initWaterParam();
	/*
	  *计算水渲染过程中的网格高度场
	 */
	void   updateWaterComputeShader();
	/*
	  *计算法线
	 */
	void   updateWaterNormal();
	/*
	  *画出水波动画
	*/
	void   drawWater();
	/*
	  *draw
	 */
	void  draw();
	/*
	  *update
	 */
	void  update(float deltaTime);
	/*
	  *触屏事件
	 */
	bool  onTouchBegan(const glk::GLVector2 *touchPoint);
	void  onTouchMoved(const glk::GLVector2 *touchPoint);
	void  onTouchEnded(const glk::GLVector2 *touchPoint);
	/*
	  *在屏幕上的某一点产生水波纹
	*/
	void  onTouchWaterWave(const glk::GLVector2 *touchPoint);
	/*
	  *键盘事件
	*/
	bool onKeyPressed(const glk::KeyCodeType keyCode);
	void onKeyReleased(const glk::KeyCodeType keyCode);
};