/*
 *Water Simulation for game fish
 *@Version:2.0 for run on OpenGLES 2.0 normally
 *2017/2/23
 *@Author:xiaoxiong
 */
#ifndef __WATER_SIMULATE_H__
#define __WATER_SIMULATE_H__
#include<engine/GLObject.h>
//#include "renderer/CCTextureCube.h"
/*
  *note:centerof WaterSimulate is left bottom
  *design by resolution 960x640
  */
#define __WATER_SQUARE_LANDSCAPE__  128 //landscape
#define __WATER_SQUARE_PORTRATE__  128
//Object of square mesh grid
class MeshSquare:public GLObject
{
private:
	unsigned _vertexBufferId;//Vertex Buffer id of OpenGL Vertex
	unsigned _fragBufferId;
	unsigned _normalBufferId;//Vertex normal id
	unsigned _indexBufferId;
	int            _countOfIndex;//record  count of index
	int            _countOfVertex;//record count of vertex
	MeshSquare();
	void          initWithMeshSquare(const float width, const float height,const  int xgrid, const int ygrid, const float fragCoord);
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
	static MeshSquare *createMeshSquare(const float width,const float height,const int xgrid,const int ygrid,const float fragCoord);
	/*
	 *@param:posLoc means position of position
	 *@param:normalLoc means position of normal 
	 *@param:fragLoc means position of frag
	 */
	void    drawMeshSquare(int posLoc,int normalLoc,int fragCoordLoc);
};

#endif