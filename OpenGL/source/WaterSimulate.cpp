/*
 *Water Simulate
 *2017/2/23
 *@Author:xiaoxiong
 */
#include"WaterSimulate.h"
#include"engine/Geometry.h"
#include<GL/glew.h>
#include<assert.h>
#include<stdio.h>
//coefficient of wave flow out 
const float WaterCoeffValue = 0.95f;
const int  VertexAttribFloatCount = 3;//每个顶点的浮点数的数目
MeshSquare::MeshSquare()
{
	_vertexBufferId = 0;
	_fragBufferId = 0;
	_normalBufferId = 0;
	_indexBufferId = 0;
	_countOfIndex = 0;
	_countOfVertex = 0;
}

MeshSquare::~MeshSquare()
{
	glDeleteBuffers(1, &_vertexBufferId);
	glDeleteBuffers(1, &_fragBufferId);
	glDeleteBuffers(1, &_normalBufferId);
	glDeleteBuffers(1, &_indexBufferId);
	_vertexBufferId = 0;
	_fragBufferId = 0;
	_normalBufferId = 0;
	_indexBufferId = 0;
}

void MeshSquare::initWithMeshSquare(const float width, const float height, const int xgrid, const int ygrid, const float fragCoord)
{
	assert(xgrid>0 && ygrid>0);
	_countOfVertex = (xgrid + 1)*(ygrid+1);
	float *Vertex = new float[_countOfVertex*3];
	for (int j = 0; j < ygrid + 1; ++j)
	{
		//const float factor = 2.0f*j / ygrid  -1.0f;
		const float y = height*(2.0f*j / ygrid - 1.0f);
		float  *nowVertex = Vertex + j*(xgrid + 1)*3;
		for (int i = 0; i < xgrid + 1; ++i)
		{
			const int index =  i * 3;
			nowVertex[index] =width*(2.0f*i/xgrid  -1.0f) ;
			nowVertex[index + 1] = y;
			nowVertex[index + 2] = 0.0f;
			//
		//	nowVertex[index +3] = 1.0f * i/xgrid *fragCoord;
		//	nowVertex[index+4] = 1.0f * j/ygrid*fragCoord;
		}
	}
	//generate index data
	_countOfIndex = xgrid*ygrid*6;
	int *indexVertex = new int[_countOfIndex];
	for (int j = 0; j < ygrid; ++j)
	{
		int *nowIndex = indexVertex + j*xgrid * 6;
		for (int i = 0; i < xgrid; ++i)
		{
			const int _index = i * 6;
			nowIndex[_index] = (j + 1)*(xgrid+1) + i;
			nowIndex[_index + 1] = j*(xgrid+1) + i;
			nowIndex[_index + 2] = (j + 1)*(xgrid+1) + (i + 1);

			nowIndex[_index + 3] = (j + 1)*(xgrid+1) + (i + 1);
			nowIndex[_index + 4] = j*(xgrid+1) + i;
			nowIndex[_index + 5] = j*(xgrid+1) + i + 1;
		}
	}
	//generate vertex and frag coord
	glGenBuffers(1, &_vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, VertexAttribFloatCount*sizeof(float)*_countOfVertex , Vertex, GL_STATIC_DRAW);
	//
	float     *texCoord = Vertex;
	for (int i = 0; i < _countOfVertex; ++i)
	{
		const int     _row = i / (xgrid+1);
		const int     _column = i%(xgrid+1);
		texCoord[i << 1] = fragCoord*_column / xgrid;
		texCoord[(i << 1) + 1] = fragCoord*_row / ygrid;
	}
	glGenBuffers(1, &_fragBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _fragBufferId);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float)*_countOfVertex, texCoord, GL_STATIC_DRAW);
	//generate normal
//	glGenBuffers(1, &_normalBufferId);
//	glBindBuffer(GL_ARRAY_BUFFER, _normalBufferId);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * _countOfVertex, NULL, GL_DYNAMIC_DRAW);
	//generate index 
	glGenBuffers(1, &_indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*_countOfIndex , indexVertex,GL_STATIC_DRAW);

//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete indexVertex;
	delete Vertex;
}

void MeshSquare::drawMeshSquare(int posLoc, int normalLoc,int fragCoordLoc)
{
	//bind vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
	glEnableVertexAttribArray(posLoc);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER,_fragBufferId);
	glEnableVertexAttribArray(fragCoordLoc);
	glVertexAttribPointer(fragCoordLoc, 2, GL_FLOAT, GL_FALSE, 0,NULL);

	//bind normal buffer
	if (normalLoc > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, _normalBufferId);
		glEnableVertexAttribArray(normalLoc);
		glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
	glDrawElements(GL_TRIANGLES, _countOfIndex, GL_UNSIGNED_INT, NULL);
}

MeshSquare *MeshSquare::createMeshSquare(const float width,const  float height, const int xgrid, const int ygrid, const float fragCoord)
{
	MeshSquare *_meshSquare = new MeshSquare();
	_meshSquare->initWithMeshSquare(width, height, xgrid, ygrid, fragCoord);
	return _meshSquare;
}

