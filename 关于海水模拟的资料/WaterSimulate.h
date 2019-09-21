/*
 *Water Simulation for game fish
 *@Version:2.0 for run on OpenGLES 2.0 normally
 *@Version:3.0 Vertex Calculation is moved in Shader
 *2017/2/23
 *2018/8/25
 *@Author:xiaoxiong
 */
#ifndef __WATER_SIMULATE_H__
#define __WATER_SIMULATE_H__
#include "cocos2d.h"
#include "RenderFramebuffer.h"
#ifdef _WIN32
#include "Fish3D/geometry/Geometry.h"
#else
#include "Geometry.h"
#endif
//#include "renderer/CCTextureCube.h"
/*
  *note:centerof WaterSimulate is left bottom
  *design by resolution 960x640
  */
//Object of square mesh grid
class MeshSquare:public cocos2d::Ref
{
private:
	unsigned _vertexBufferId;//Vertex Buffer id of OpenGL Vertex
	//unsigned _normalBufferId;//Vertex normal id
	unsigned _indexBufferId;
	int            _countOfIndex;//record  count of index
	int            _countOfVertex;//record count of vertex
	MeshSquare();
	void          initWithMeshSquare(float width, float height, int xgrid, int ygrid, float fragCoord);
public:
	~MeshSquare();
	//unsigned getNormal()const{ return _normalBufferId; };
	unsigned getVertex()const { return _vertexBufferId; };
	/*
	 *@param:width means width of square
	 *@param:height means height of square
	 *@param:xgrid means number of grid in row
	 *@param:ygrid means number of grid in column
	 *@param:fragCoord means max frag coord,if it greater than 1.0, texture will be repeated
	 *@note: format of png picture is y mirror
	 */
	static MeshSquare *createMeshSquare(float width,float height,int xgrid,int ygrid,float fragCoord);
	/*
	 *@param:posLoc means position of position
	 *@param:normalLoc means position of normal 
	 *@param:fragLoc means position of frag
	 */
	void    drawMeshSquare(int posLoc,int normalLoc,int fragCoordLoc);
//#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	/*
	  *recreate on platform Android
	 */
	void    recreate(float width, float height, int xgrid, int ygrid, float fragCoord);
//#endif
};
/*
 *@note: no matter what the position WaterSimulate is,is position is center of screen
 *@note:I design it because i want it to be a image process class ,not a effect
 *@note: but ,when it generate or run,it will cost so much resource,in some device,it will be an obstacle
*/
class WaterSimulate :public cocos2d::Node
{
private:
	//帧缓冲区对象/颜色缓冲区对象,需要三个
	unsigned                  _framebufferIds[3];
	unsigned                  _textureIds[3];
	unsigned                  _framebufferIndex;
	//顶点数组对象/顶点缓冲区对象
	unsigned                  _identityVertexArray;
	unsigned                  _identityVertexBuffer;//
	cocos2d::Vec2        _pixelSteps[4];
	cocos2d::Vec4        _waterParam;
	cocos2d::Vec2        _meshSizeVec2;
	//
	MeshSquare				*_meshSquare;
	RenderFramebuffer	*_renderFrame;

	cocos2d::GLProgram *_glProgram;
	cocos2d::GLProgram *_heightProgram;
	cocos2d::GLProgram *_normalProgram;
	cocos2d::CustomCommand _drawWaterMeshCommand;
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	cocos2d::EventListenerCustom  *_backToForegroundListener;
#endif
	//render location
	int       _projMatrixLoc;
	int		_modelMatrixLoc;
	int       _positionLoc;
	int       _fragCoordLoc;
	int       _skyboxLoc;
	int       _textureLoc;
	int       _freshnelParamLoc;
	int       _textureStepLoc;
	//height-program-location
	int        _heightAttribPositionLoc;
	int        _heightAttribFragCoordLoc;
	int        _heightTextureLoc;
	int        _heightPixelStepsLoc;
	int        _heightMeshSizeLoc;
	int       _heightWaterParamLoc;
	//normal-program-location
	int       _normalAttribPositionLoc;
	int       _normalAttribFragCoordLoc;
	int       _normalTextureLoc;
	int       _normalPixelStepLoc;
	//
	bool    _isFingerTouch;
	cocos2d::Vec2   _fingerPoint;
	cocos2d::Vec2   _textureStep;
	Matrix               _viewProjMatrix;
	GLVector3        _freshnelParam;

	WaterSimulate();
	void             initWithSkybox();
	/*
	  *初始化Shader
	 */
	void             initGLProgram();
	/*
	  *初始化顶点数组对象/顶点缓冲区对象
	 */
	void             initGLBuffer();
	/*
	  *初始化帧缓冲区对象,以及相关的颜色附着
	 */
	void             initFramebuffer();
	/*
	  *计算高度场/偏移场
	 */
	void             drawHeightNormalField();
public:
	~WaterSimulate();
	static WaterSimulate *create();
	/*
	  *提供检测机器是否支持此C++应用程序的创建
	 */
	static bool   isMachineSupport();
	static int      getVersion();
	//
	void setTouchPoint(cocos2d::Vec2 &);
	//need to override
	virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4& transform, uint32_t flags);
	//we defined it to draw water mesh
    void drawWaterMesh(const cocos2d::Mat4& transform, uint32_t flags);
	//need to override
	virtual void visit(cocos2d::Renderer *renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags);
//#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	//when on platform Android
	void     recreate(cocos2d::EventCustom *recreateEvent);
//#endif
};
#endif
