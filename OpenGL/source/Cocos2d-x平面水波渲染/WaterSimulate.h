/*
 *Water Simulation for game fish
 *@Version:2.0 for run on OpenGLES 2.0 normally
 *2017/2/23
 *@Author:xiaoxiong
 */
#ifndef __WATER_SIMULATE_H__
#define __WATER_SIMULATE_H__
#include "cocos2d.h"
#include "effect/RenderFramebuffer.h"
//#include "renderer/CCTextureCube.h"
/*
  *note:centerof WaterSimulate is left bottom
  *design by resolution 960x640
  */
#define __WATER_SQUARE_LANDSCAPE__  128 //landscape
#define __WATER_SQUARE_PORTRATE__  128
//Object of square mesh grid
class MeshSquare:public cocos2d::Ref
{
private:
	unsigned _vertexBufferId;//Vertex Buffer id of OpenGL Vertex
	unsigned _normalBufferId;//Vertex normal id
	unsigned _indexBufferId;
	int            _countOfIndex;//record  count of index
	int            _countOfVertex;//record count of vertex
	MeshSquare();
	void          initWithMeshSquare(float width, float height, int xgrid, int ygrid, float fragCoord);
public:
	~MeshSquare();
	unsigned getNormal()const{ return _normalBufferId; };
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
};
/*
 *@note: no matter what the position WaterSimulate is,is position is center of screen
 *@note:I design it because i want it to be a image process class ,not a effect
 *@note: but ,when it generate or run,it will cost so much resource,in some device,it will be an obstacle
*/
class WaterSimulate :public cocos2d::Node
{
private:
	//record velocity of current water wave
	float								_nowHeight[__WATER_SQUARE_PORTRATE__+2][__WATER_SQUARE_LANDSCAPE__+2];
	float								_nowVelocity[__WATER_SQUARE_PORTRATE__+2][__WATER_SQUARE_LANDSCAPE__+2];
	int                               _srcMeshIndex;//record current mesh buffer to be processed
	MeshSquare				*_meshSquare;
	unsigned						_imageBufferId;
	char								*_imageBuffer;

	cocos2d::GLProgram *_glProgram;
	cocos2d::CustomCommand _drawWaterMeshCommand;
	int       _projMatrixLoc;
	int		_modelMatrixLoc;
	int       _positionLoc;
	int       _normalLoc;
	int       _fragCoordLoc;
	int       _skyboxLoc;
	int       _freshnelParamLoc;
	//
	bool    _isFingerTouch;
	cocos2d::Vec2   _fingerPoint;

	WaterSimulate();
	void             initWithSkybox();
public:
	~WaterSimulate();
	static WaterSimulate *create();
	//
	void setTouchPoint(cocos2d::Vec2 &);
	//need to override
	virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4& transform, uint32_t flags);
	//we defined it to draw water mesh
    void drawWaterMesh(const cocos2d::Mat4& transform, uint32_t flags);
	//need to override
	virtual void visit(cocos2d::Renderer *renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags);
};
#endif