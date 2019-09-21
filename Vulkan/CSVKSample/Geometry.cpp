/*
  *针对Vulkan的数学函数库
  *由于Vulkan的坐标系与OpenGL不相同,投影矩阵的实现将不同
  *2018年9月21日
 */
#include "Geometry.h"
#include <math.h>
#include<string.h>
#include<assert.h>
#define    __EPS__  0.000001f
#define GLK_ANGLE_TO_RADIUS(x) ((x) * M_PI/180.0f)
#define GLK_RADIUS_TO_ANGLE(x)  ((x) * 180.0f/M_PI)
#define    __SIGNFloat(sign)   (-(  ((sign)&0x1)<<1)+1)
//新引入切线的计算
int  esGenSphere(int numSlices, float radius, float **vertices, float **normals, float  **tangents,
	float **texCoords, int **indices ,int  *numberOfVertex)
{
	int i,j;
	int numParallels = numSlices / 2;
	int numVertices = ( numParallels + 1 ) * ( numSlices + 1 );
	int numIndices = numParallels * numSlices * 6;
	float angleStep = 2.0f * M_PI /numSlices;
	*numberOfVertex = numVertices;
	float   *_vertex=NULL,*_texCoord=NULL,*_normal=NULL,*_tangent=NULL;
	// Allocate memory for buffers
	if ( vertices != NULL )
		_vertex = new float[3 * numVertices ];

	if ( normals != NULL )
		_normal= new  float[3 * numVertices ];

	if ( texCoords != NULL )
		_texCoord = new float[ 2 * numVertices ];

	if ( indices != NULL )
		*indices = new  int[numIndices];

	if (tangents)
		_tangent = new  float[3 * numVertices];

	for ( i = 0; i < numParallels + 1; i++ )//Y轴切片,只需要180度即可
	{
		const float   _real_radius=radius*sinf(angleStep*i);
		const float   _real_cos=radius*cosf(angleStep*i);
		for ( j = 0; j < numSlices + 1; j++ )
		{
			int vertex = ( i * ( numSlices + 1 ) + j ) * 3;

			if ( _vertex )
			{
				_vertex[vertex + 0] = _real_radius *sinf ( angleStep * j );//因为angleStep是与Z轴的夹角,所以取余弦值
				_vertex [vertex + 1] = _real_cos;//从y轴自上而下的夹角
				_vertex[vertex + 2] = _real_radius *cosf ( angleStep * j );//与Z轴的夹角
			}

			if ( _normal )
			{
				_normal[vertex + 0] = _vertex[vertex + 0] / radius;
				_normal[vertex + 1] = _vertex [vertex + 1] / radius;
				_normal[vertex + 2] = _vertex [vertex + 2] / radius;
			}
//切线是球面方程关于x的偏导数
			if (_tangent)
			{
					_tangent[vertex] = cosf(j*angleStep);
					_tangent[vertex + 1] = 0.0f;
					_tangent[vertex + 2] = sinf(j*angleStep);
			}
			if ( _texCoord )
			{
				int texIndex = ( i * ( numSlices + 1 ) + j ) * 2;
				_texCoord[texIndex + 0] = ( float ) j / ( float ) numSlices;
				_texCoord [texIndex + 1] =1.0f - ( float ) i /numParallels;
			}
		}
	}
//切线生成,需要先生成全部的顶点,纹理坐标
// Generate the indices
	float_3     *originVertex = (float_3 *)_vertex;
	float_2     *originTex = (float_2  *)_texCoord;
	float_3     *originTangent = (float_3 *)_tangent;
	if ( indices != NULL )
	{
		int *indexBuf =*indices;
		
		const      int     vSkipSlice = numSlices + 1;
		for ( i = 0; i < numParallels ; i++ )
		{
			const     int       vHead=i+1;
			for ( j = 0; j < numSlices; j++ )
			{
				const    int     vTail = j + 1;
				int         _M_PIndex1, _M_PIndex2, _M_PIndex3;
				_M_PIndex1 =  i * vSkipSlice + j;
				_M_PIndex2 = vHead * vSkipSlice + j;
				_M_PIndex3 = vHead* vSkipSlice + vTail;

				*indexBuf++ = _M_PIndex1;
				*indexBuf++ = _M_PIndex2;
				*indexBuf++ = _M_PIndex3;

				float_2    deltaUV1 = originTex[_M_PIndex2] - originTex[_M_PIndex1];
				float_2    deltaUV2 = originTex[_M_PIndex3] - originTex[_M_PIndex1];

				float     _lamda = 1.0f / (deltaUV1.x*deltaUV2.y - deltaUV1.y*deltaUV2.x);

				float_3       e1 = originVertex[_M_PIndex2] - originVertex[_M_PIndex1];
				float_3       e2 = originVertex[_M_PIndex3] - originVertex[_M_PIndex1];

				float     x1 = (deltaUV2.y*e1.x - deltaUV1.y*e2.x)*_lamda;
				float     y1 = (deltaUV2.y*e1.y - deltaUV1.y*e2.y)*_lamda;
				float     z1 = (deltaUV2.y*e1.z - deltaUV1.y*e2.z)*_lamda;

				//float    x2 = -_v1*e1.x + _u1*e2.x;
				//float    y2 = -_v1*e1.y + _u1*e2.y;
				//float    z2 = -_v1*e1.z + _u1*e2.z;
				float_3    _tangentValue(x1,y1,z1);
				originTangent[_M_PIndex1] = _tangentValue;
				originTangent[_M_PIndex2] = _tangentValue;
				originTangent[_M_PIndex3] = _tangentValue;


				_M_PIndex1 =i * vSkipSlice + j ;
				_M_PIndex2 = vHead *vSkipSlice + vTail;
				_M_PIndex3 =i * vSkipSlice + vTail ;

				*indexBuf++ = _M_PIndex1;
				*indexBuf++ = _M_PIndex2;
				*indexBuf++ =_M_PIndex3 ;

				deltaUV1 = originTex[_M_PIndex1] - originTex[_M_PIndex3];
				deltaUV2 = originTex[_M_PIndex2] - originTex[_M_PIndex3];

				_lamda = 1.0f / (deltaUV1.x*deltaUV2.y - deltaUV1.y*deltaUV2.x);

				e1 = originVertex[_M_PIndex1] - originVertex[_M_PIndex3];
				e2 = originVertex[_M_PIndex2] - originVertex[_M_PIndex3];

				 x1 = (deltaUV2.y*e1.x - deltaUV1.y*e2.x)*_lamda;
				 y1 = (deltaUV2.y*e1.y - deltaUV1.y*e2.y)*_lamda;
				 z1 = (deltaUV2.y*e1.z - deltaUV1.y*e2.z)*_lamda;

				 _tangentValue=float_3(x1, y1, z1);
				 originTangent[_M_PIndex1] = _tangentValue;
				 originTangent[_M_PIndex2] = _tangentValue;
				 originTangent[_M_PIndex3] = _tangentValue;
			}
		}
	}
	*vertices=_vertex;
	*texCoords=_texCoord;
	*normals=_normal;
	*tangents = _tangent;
	return numIndices;
}

//
/// \brief Generates geometry for a cube.  Allocates memory for the vertex data and stores
///        the results in the arrays.  Generate index list for a TRIANGLES
/// \param scale The size of the cube, use 1.0 for a unit cube.
/// \param vertices If not NULL, will contain array of float3 positions
/// \param normals If not NULL, will contain array of float3 normals
/// \param texCoords If not NULL, will contain array of float2 texCoords
/// \param indices If not NULL, will contain the array of indices for the triangle strip
/// \return The number of indices required for rendering the buffers (the number of indices stored in the indices array
///         if it is not NULL ) as a GL_TRIANGLE_STRIP
//
int  esGenCube ( float scale, float **vertices, float **normals,float **tangents,float **texCoords,int  *numberOfVertex )
{
//从立方体的外面观察,所有的三角形都是正方向的
	float cubeVerts[] =
	{
//前
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
//后
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
//左
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
//右
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
//上
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
//下
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
	};
	//切线
	float    tangent[] = {
		//前
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		//后
		-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		//左
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		//右
		0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
		//上
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		//下
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	};
	float cubeNormals[] =
	{
//前
		0.0f,0.0f,-1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f, 
		0.0f, 0.0f, -1.0f,
//后
		0.0f,0.0f,1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
//左
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
//右
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
//上
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
//下
		0.0f, -1.0f,0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
	};

	float cubeTex[] =
	{
//前
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
//后
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
//左
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
//右
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
//上
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
//下
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
	};
	*numberOfVertex = sizeof(cubeVerts) / sizeof(float)/3;
	float   *_vertex = new float[sizeof(cubeVerts) / sizeof(float)];

	for (int i = 0; i < sizeof(cubeVerts) / sizeof(float); ++i)
		_vertex[i] = cubeVerts[i] * scale;
	*vertices = _vertex;
	*texCoords = new  float[sizeof(cubeTex) / sizeof(float)];
	*normals = new  float[sizeof(cubeVerts) / sizeof(float)];
	*tangents = new   float[sizeof(tangent)/sizeof(float)];
	memcpy(*texCoords,cubeTex,sizeof(cubeTex));
	memcpy(*normals,cubeNormals,sizeof(cubeNormals));
	memcpy(*tangents,tangent,sizeof(tangent));
	return 0;
}

//
/// \brief Generates a square grid consisting of triangles.  Allocates memory for the vertex data and stores
///        the results in the arrays.  Generate index list as TRIANGLES.
/// \param size create a grid of size by size (number of triangles = (size-1)*(size-1)*2)
/// \param vertices If not NULL, will contain array of float3 positions
/// \param indices If not NULL, will contain the array of indices for the triangle strip
/// \return The number of indices required for rendering the buffers (the number of indices stored in the indices array
///         if it is not NULL ) as a GL_TRIANGLES
//
int  esGenSquareGrid ( int size, float scale,float **vertices,int **indices,int  *numberOfVertex )
{
	int i, j;
	int numIndices = ( size - 1 ) * ( size - 1 ) * 2 * 3;

	// Allocate memory for buffers
	if ( vertices != NULL )
	{
		int numVertices = size * size;
		*numberOfVertex = numVertices;
		float stepSize = (( float)(size - 1))/2.0f;
		float   *_vertex =new float[ 3 * numVertices ];
		*vertices = _vertex;

		for ( i = 0; i <size; ++i ) // row
		{
			   const    float     locX=scale*(i/stepSize-1.0f);
			for ( j = 0; j <size; ++j ) // column
			{
				int   _index = (i*size + j) * 3;
				_vertex[_index] = scale*(j/stepSize-1.0f);
				_vertex[_index + 1] = locX;
				_vertex[_index + 2] = 0.0f;
			}
		}
	}
	// Generate the indices
	if ( indices != NULL )
	{
		int   *_indice = new int[numIndices];
		*indices = _indice;
		for ( i = 0; i < size - 1; ++i )
		{
			for ( j = 0; j < size - 1; ++j )
			{
// two triangles per quad
				int    _index = 6 * (j + i * (size - 1));
				_indice[_index] = j + i* size;
				_indice[_index + 1] = j + i* size+1;
				_indice[_index + 2] = j + (i + 1) * size+1;

				_indice[_index + 3] = j + (i + 1) * size+1;
				_indice[_index + 4] = j + (i + 1) * size;
				_indice[_index + 5] =  j + i* size;
			}
		}
	}
	return numIndices;
}
//vector cross
float_3     cross(float_3  *a,float_3  *b)
{
	float   x = a->y*b->z - a->z*b->y;
	float   y = a->z*b->x - a->x*b->z;
	float   z = a->x*b->y - a->y*b->x;

	return  float_3(x,y,z);
}
//vector dot
float       dot(float_3  *srcA, float_3  *srcB)
{
	return    srcA->x*srcB->x + srcA->y*srcB->y + srcA->z*srcB->z;
}

//行列式计算
float     detFloat(float x1, float y1, float x2, float y2)
{
	return  x1*y2 - y1*x2;
}
float    detVector2(float_2   *a, float_2   *b)
{
	return a->x*b->y - a->y*b->x;
}
//
float   detVector3(float_3    *a,float_3       *b,float_3  *c)
{
	float   _result = 0;
	_result =a->x* (b->y*c->z-b->z*c->y);
	_result -= a->y*(b->x*c->z-b->z*c->x);
	_result += a->z*(b->x*c->y-b->y*c->x);
	return _result;
}
//四维矩阵的逆
static   void   static_fix_func(int  *_index,int  _current)//辅助函数
{
	int   i = 0;
	int   k = 0;
	while (i < 4)
	{
		if (i != _current)
			_index[k++] = i;
		++i;
	}
}
/////////////////////////////二维,三维,四维向量右乘矩阵//////////////////////////////////////
float_2    float_2::operator*(float  _factor)const
{
	return   float_2(x*_factor,y*_factor);
}
float_2   float_2::operator*(const float_2  &_mfactor)const
{
	return  float_2(x*_mfactor.x,y*_mfactor.y);
}
float_2   float_2::operator+(const float_2  &_factor)const
{
	return  float_2(x+_factor.x,y+_factor.y);
}
float_2   float_2::operator-(const float_2  &_factor)const
{
	return  float_2(x-_factor.x,y-_factor.y);
}
float_2   float_2::operator/(float _factor)const
{
	return  float_2(x/_factor,y/_factor);
}
float_2    float_2::operator/(const float_2  &_factor)const
{
	return  float_2(x/_factor.x,y/_factor.y);
}

float_2& float_2::operator=(const float_2 &src)
{
	x = src.x, y = src.y;
	return *this;
}

float_2& float_2::operator+=(const float_2 &src)
{
	x += src.x;
	y += src.y;
	return *this;
}

float_2& float_2::operator-=(const float_2 &src)
{
	x += src.x;
	y += src.y;
	return *this;
}

float_2& float_2::operator*=(const float_2 &src)
{
	x += src.x;
	y += src.y;
	return *this;
}

float_2& float_2::operator/=(const float_2 &src)
{
	x+=src.x;
	y += src.y;
	return *this;
}

float_2   float_2::normalize()const
{
	float  _length = sqrtf(x*x+y*y);
	assert(_length>=__EPS__);
	return  float_2(x/_length,y/_length);
}
float     float_2::dot(const float_2 &other)const
{
	return x*other.x + y*other.y;
}

const float float_2::length()const
{
	return sqrtf(x*x+y*y);
}
/////////////////////////////333333333333333333////////////////////////////////////
float_4   float_3::xyzw0()const
{
	return float_4(x,y,z,0.0f);
}

float_4 float_3::xyzw1()const
{
	return float_4(x,y,z,1.0f);
}
float_3   float_3::operator*(const mat3x3 &src)const
{
	float  x, y, z;
	x = this->x*src.m[0][0] + this->y*src.m[1][0] + this->z*src.m[2][0];
	y = this->x*src.m[0][1] + this->y*src.m[1][1] + this->z*src.m[2][1];
	z = this->x*src.m[0][2] + this->y*src.m[1][2] + this->z*src.m[2][2];

	return  float_3(x,y,z);
}
float_3    float_3::operator*(const float   _factor)const
{
	return  float_3(x*_factor,y*_factor,z*_factor);
}
float_3   float_3::operator*(const float_3  &_factor)const
{
	return  float_3(x*_factor.x,y*_factor.y,z*_factor.z);
}
float_3   float_3::operator+(const float_3  &_factor)const
{
	return  float_3(x+_factor.x,y+_factor.y,z+_factor.z);
}
float_3   float_3::operator-(const float_3 &_factor)const
{
	return  float_3(x-_factor.x,y-_factor.y,z-_factor.z);
}
float_3   float_3::operator/(const float _factor)const
{
	return  float_3(x/_factor,y/_factor,z/_factor);
}
float_3   float_3::operator/(const float_3 &_factor)const
{
	return   float_3(x/_factor.x,y/_factor.y,z/_factor.z);
}
float_3& float_3::operator+=(const float_3 &offset)
{
	x += offset.x;
	y += offset.y;
	z += offset.z;
	return *this;
}

float_3& float_3::operator+=(float f)
{
	x += f;
	y += f;
	z += f;
	return *this;
}

float_3& float_3::operator-=(const float_3 &factor)
{
	x -= factor.x;
	y -= factor.y;
	z -= factor.z;
	return *this;
}
float_3& float_3::operator-=(float f)
{
	x -= f;
	y -= f;
	z -= f;
	return *this;
}

float_3& float_3::operator*=(const float_3 &factor)
{
	x *= factor.x;
	y *= factor.y;
	z *= factor.z;
	return *this;
}
float_3& float_3::operator*=(float f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

float_3& float_3::operator/=(const float_3 &factor)
{
	x /= factor.x;
	y /= factor.y;
	z /= factor.z;
	return *this;
}
float_3& float_3::operator/=(float f)
{
	x /= f;
	y /= f;
	z /= f;
	return *this;
}

float_3   float_3::normalize()const
{
	float      _length = sqrtf(x*x+y*y+z*z);
	assert(_length>=__EPS__);
	return    float_3(x/_length,y/_length,z/_length);
}
float_3   float_3::cross(const float_3 &axis)const
{
	return float_3(
		y*axis.z - z*axis.y,
		-x*axis.z + z*axis.x,
		x*axis.y - y*axis.x
		);
}
float    float_3::dot(const float_3 &other)const
{
	return x*other.x + y*other.y + z*other.z;
}
const float float_3::length()const
{
	return sqrtf(x*x+y*y+z*z);
}

float_3 float_3::min(const float_3 &other)const
{
	const float nx = x < other.x ? x : other.x;
	const float ny = y < other.y ? y : other.y;
	const float nz = z < other.z ? z : other.z;
	return float_3(nx, ny, nz);
}

float_3 float_3::max(const float_3 &other)const
{
	const float nx = x > other.x ? x : other.x;
	const float ny = y > other.y ? y : other.y;
	const float nz = z > other.z ? z : other.z;
	return float_3(nx, ny, nz);
}

void float_3::generateViewXY(const float_3 &Z, float_3 &X, float_3 &Y)
{
	float_3 upVector(0,1,0);
	float_3 rightVector(1,0,0);
	float_3 forwardVector(0,0,1);
	if (Z.y == -1.0f)
	{
		X = rightVector;
		Y = forwardVector;
	}
	else if (Z.y > -1 && Z.y < 0)
	{
		X = upVector.cross(Z);
		Y = Z.cross(X);
	}
	else if (Z.y == 0)
	{
		Y = upVector;
		X = Y.cross(Z);
	}
	else if (Z.y > 0 && Z.y < 1)
	{
		X = upVector.cross(Z);
		Y = Z.cross(X);
	}
	else if (Z.y ==1)//此时无法确定摄像机X/Y的方向
	{
		X = rightVector;
		Y = float_3(0,0,-1);
	}
}

/////////////////////////4444444444444444///////////////////////////////////////
float_4     float_4::operator*(const mat4x4 &src)const
{
	float  nx,ny, nz, nw;
	nx = this->x*src.m[0][0] + this->y*src.m[1][0] + this->z*src.m[2][0] + this->w*src.m[3][0];
	ny = this->x*src.m[0][1] + this->y*src.m[1][1] + this->z*src.m[2][1] + this->w*src.m[3][1];
	nz = this->x*src.m[0][2] + this->y*src.m[1][2] + this->z*src.m[2][2] + this->w*src.m[3][2];
	nw = this->x*src.m[0][3] + this->y*src.m[1][3] + this->z*src.m[2][3] + this->w*src.m[3][3];
	return float_4(nx, ny, nz, nw);
}
float_4    float_4::normalize()const
{
	float   _length = sqrtf(x*x+y*y+z*z+w*w);
	assert(_length>__EPS__);
	return  float_4(x/_length,y/_length,z/_length,w/_length);
}
float    float_4::dot(const float_4 &other)const
{
	return x*other.x + y*other.y + z*other.z + w*other.w;
}

float_4 float_4::operator*(const float factor)const
{
	return float_4(x*factor,y*factor,z*factor,w*factor);
}

float_4 float_4::operator*(const float_4 &other)const
{
	return float_4(x*other.x,y*other.y,z*other.z,w*other.w);
}

float_4 float_4::operator+(const float_4 &other)const
{
	return float_4(x+other.x,y+other.y,z+other.z,w+other.w);
}

float_4 float_4::operator/(const float_4 &other)const
{
	return float_4(x/other.x,y/other.y,z/other.z,w/other.w);
}

const float_4& float_4::operator/=(const float factor)
{
	x /= factor;
	y /= factor;
	z /= factor;
	w /= factor;
	return *this;
}

const float_4& float_4::operator*=(const float factor)
{
	x *= factor;
	y *= factor;
	z *= factor;
	w *= factor;
	return *this;
}

float_4 float_4::operator/(const float factor)const
{
	return float_4(x/factor,y/factor,z/factor,w/factor);
}

float_4 float_4::operator-(const float_4 &other)const
{
	return float_4(x-other.x,y-other.y,z-other.z,w-other.w);
}

float_4 float_4::min(const float_4 &other)const
{
	const float nx = x < other.x ? x : other.x;
	const float ny = y < other.y ? y : other.y;
	const float nz = z < other.z ? z : other.z;
	const float nw = w < other.w ? w : other.w;
	return float_4(nx,ny,nz,nw);
}

float_4 float_4::max(const float_4 &other)const
{
	const float nx = x > other.x ? x : other.x;
	const float ny = y > other.y ? y : other.y;
	const float nz = z > other.z ? z : other.z;
	const float nw = w > other.w ? w : other.w;
	return float_4(nx, ny, nz, nw);
}
////////////////////////////四维矩阵实现//////////////////////////////////
mat4x4::mat4x4()
{
	m[0][0] = 1.0f, m[0][1] = 0.0f, m[0][2] = 0.0f, m[0][3] = 0.0f;
	m[1][0] = 0.0f, m[1][1] = 1.0f, m[1][2] = 0.0f, m[1][3] = 0.0f;
	m[2][0] = 0.0f, m[2][1] = 0.0f, m[2][2] = 1.0f, m[2][3] = 0.0f;
	m[3][0] = 0.0f, m[3][1] = 0.0f, m[3][2] = 0.0f, m[3][3] = 1.0f;
}

mat4x4::mat4x4(const float_4 &row0, const float_4 &row1, const float_4 &row2, const float_4 &row3)
{
	m[0][0] = row0.x, m[0][1] = row0.y, m[0][2] = row0.z, m[0][3] = row0.w;
	m[1][0] = row1.x, m[1][1] = row1.y, m[1][2] = row1.z, m[1][3] = row1.w;
	m[2][0] = row2.x, m[2][1] = row2.y, m[2][2] = row2.z, m[2][3] = row2.w;
	m[3][0] = row3.x, m[3][1] = row3.y, m[3][2] = row3.z, m[3][3] = row3.w;
}

mat4x4::mat4x4(const float_3 &row1, const float_3 &row2, const float_3 &row3, const float_3 &eyePosition)
{
	m[0][0] = row1.x; m[0][1] = row2.x; m[0][2] = row3.x; m[0][3] = 0;
	m[1][0] = row1.y; m[1][1] = row2.y; m[1][2] = row3.y; m[1][3] = 0;
	m[2][0] = row1.z; m[2][1] = row2.z; m[2][2] = row3.z; m[2][3] = 0;
	m[3][0] = -row1.dot(eyePosition);
	m[3][1] = -row2.dot(eyePosition);
	m[3][2] = -row3.dot(eyePosition);
	m[3][3] = 1;
}

void     mat4x4::identity()
{
	m[0][0] = 1.0f, m[0][1] = 0.0f, m[0][2] = 0.0f, m[0][3] = 0.0f;
	m[1][0] = 0.0f, m[1][1] = 1.0f, m[1][2] = 0.0f, m[1][3] = 0.0f;
	m[2][0] = 0.0f, m[2][1] = 0.0f, m[2][2] = 1.0f, m[2][3] = 0.0f;
	m[3][0] = 0.0f, m[3][1] = 0.0f, m[3][2] = 0.0f, m[3][3] = 1.0f;
}
void    mat4x4::copy(const mat4x4  &srcA)
{
	m[0][0] = srcA.m[0][0], m[0][1] = srcA.m[0][1], m[0][2] = srcA.m[0][2], m[0][3] = srcA.m[0][3];
	m[1][0] = srcA.m[1][0], m[1][1] = srcA.m[1][1], m[1][2] = srcA.m[1][2], m[1][3] = srcA.m[1][3];
	m[2][0] = srcA.m[2][0], m[2][1] = srcA.m[2][1], m[0][2] = srcA.m[2][2], m[2][3] = srcA.m[2][3];
	m[3][0] = srcA.m[3][0], m[3][1] = srcA.m[3][1], m[3][2] = srcA.m[3][2], m[3][3] = srcA.m[3][3];
}
//右乘缩放矩阵
void   mat4x4::scale(const float scaleX, const float scaleY, const float  scaleZ)
{
	m[0][0] *= scaleX; m[0][1] *= scaleY; m[0][2] *= scaleZ; 
	m[1][0] *= scaleX; m[1][1] *= scaleY; m[1][2] *= scaleZ; 
	m[2][0] *= scaleX; m[2][1] *= scaleY; m[2][2] *= scaleZ; 
	m[3][0] *= scaleX; m[3][1] *= scaleY; m[3][2] *= scaleZ; 
}
//平移
void    mat4x4::translate(const float deltaX, const float  deltaY, const float deltaZ)
{
	m[0][0] += m[0][3] * deltaX;
	m[0][1] += m[0][3] * deltaY;
	m[0][2] += m[0][3] * deltaZ;

	m[1][0] += m[1][3] * deltaX;
	m[1][1] += m[1][3] * deltaY;
	m[1][2] += m[1][3] * deltaZ;

	m[2][0] += m[2][3] * deltaX;
	m[2][1] += m[2][3] * deltaY;
	m[2][2] += m[2][3] * deltaZ;

	m[3][0] += m[3][3] * deltaX;
	m[3][1] += m[3][3] * deltaY;
	m[3][2] += m[3][3] * deltaZ;
}

void mat4x4::translate(const float_3 &deltaXYZ)
{
	m[0][0] += m[0][3] * deltaXYZ.x;
	m[0][1] += m[0][3] * deltaXYZ.y;
	m[0][2] += m[0][3] * deltaXYZ.z;

	m[1][0] += m[1][3] * deltaXYZ.x;
	m[1][1] += m[1][3] * deltaXYZ.y;
	m[1][2] += m[1][3] * deltaXYZ.z;

	m[2][0] += m[2][3] * deltaXYZ.x;
	m[2][1] += m[2][3] * deltaXYZ.y;
	m[2][2] += m[2][3] * deltaXYZ.z;

	m[3][0] += m[3][3] * deltaXYZ.x;
	m[3][1] += m[3][3] * deltaXYZ.y;
	m[3][2] += m[3][3] * deltaXYZ.z;
}

void    mat4x4::rotateX(float M_PItch)
{
	mat4x4  matX;

	const float sinX = sinf(GLK_ANGLE_TO_RADIUS(M_PItch));
	const float cosX = cosf(GLK_RADIUS_TO_ANGLE(M_PItch));

	matX.m[1][1] = cosX;
	matX.m[1][2] = sinX;
	
	matX.m[2][1] = -sinX;
	matX.m[2][2] = cosX;
	this->multiply(matX);
}

void mat4x4::rotateY(float yaw)
{
	mat4x4 matY;

	const float sinY = sinf(GLK_ANGLE_TO_RADIUS(yaw));
	const float cosY = cosf(GLK_ANGLE_TO_RADIUS(yaw));

	matY.m[0][0] = cosY;
	matY.m[0][2] = -sinY;

	matY.m[2][0] = sinY;
	matY.m[2][2] = cosY;
	this->multiply(matY);
}

void mat4x4::rotateZ(float roll)
{
	mat4x4 matZ;
	const float sinZ = sinf(GLK_ANGLE_TO_RADIUS(roll));
	const float cosZ = cosf(GLK_ANGLE_TO_RADIUS(roll));

	matZ.m[0][0] = cosZ;
	matZ.m[0][1] = sinZ;

	matZ.m[1][0] = -sinZ;
	matZ.m[1][1] = cosZ;
	this->multiply(matZ);
}
//旋转
void    mat4x4::rotate(float  angle, float x, float y, float z)
{
	float sinAngle, cosAngle;
	float mag = sqrtf(x * x + y * y + z * z);

	sinAngle = sinf(angle * M_PI / 180.0f);
	cosAngle = cosf(angle * M_PI / 180.0f);

	assert(mag > 0.0f);
	float xx, yy, zz, xy, yz, zx, xs, ys, zs;
	float oneMinusCos;
	mat4x4 tmp;

	x /= mag; y /= mag;  z /= mag;
   xx = x * x;  yy = y * y;   zz = z * z;
	xy = x * y;  yz = y * z;    zx = z * x;

	xs = x * sinAngle;  ys = y * sinAngle;   zs = z * sinAngle;   oneMinusCos = 1.0f - cosAngle;

	tmp.m[0][0] = (oneMinusCos * xx) + cosAngle;
	tmp.m[0][1] = (oneMinusCos * xy) + zs;
	tmp.m[0][2] = (oneMinusCos * zx) - ys;
	tmp.m[0][3] = 0.0F;

	tmp.m[1][0] = (oneMinusCos * xy) - zs;
	tmp.m[1][1] = (oneMinusCos * yy) + cosAngle;
	tmp.m[1][2] = (oneMinusCos * yz) + xs;
	tmp.m[1][3] = 0.0F;

	tmp.m[2][0] = (oneMinusCos * zx) + ys;
	tmp.m[2][1] = (oneMinusCos * yz) - xs;
	tmp.m[2][2] = (oneMinusCos * zz) + cosAngle;
	tmp.m[2][3] = 0.0F;

	tmp.m[3][0] = 0.0F;
	tmp.m[3][1] = 0.0F;
	tmp.m[3][2] = 0.0F;
	tmp.m[3][3] = 1.0F;

	this->multiply(tmp);
}
void mat4x4::rotate(float angle, const float_3 &axis)
{
	float sinAngle, cosAngle;
	float_3 newAxis = axis.normalize();

	sinAngle = sinf(angle * M_PI / 180.0f);
	cosAngle = cosf(angle * M_PI / 180.0f);

	float xx, yy, zz, xy, yz, zx, xs, ys, zs;
	float oneMinusCos;
	mat4x4 tmp;

	xx = newAxis.x * newAxis.x;  yy = newAxis.y * newAxis.y;   zz = newAxis.z * newAxis.z;
	xy = newAxis.x * newAxis.y;  yz = newAxis.y * newAxis.z;    zx = newAxis.z * newAxis.x;

	xs = newAxis.x * sinAngle;  ys = newAxis.y * sinAngle;   zs = newAxis.z * sinAngle;   oneMinusCos = 1.0f - cosAngle;

	tmp.m[0][0] = (oneMinusCos * xx) + cosAngle;
	tmp.m[0][1] = (oneMinusCos * xy) + zs;
	tmp.m[0][2] = (oneMinusCos * zx) - ys;
	tmp.m[0][3] = 0.0F;

	tmp.m[1][0] = (oneMinusCos * xy) - zs;
	tmp.m[1][1] = (oneMinusCos * yy) + cosAngle;
	tmp.m[1][2] = (oneMinusCos * yz) + xs;
	tmp.m[1][3] = 0.0F;

	tmp.m[2][0] = (oneMinusCos * zx) + ys;
	tmp.m[2][1] = (oneMinusCos * yz) - xs;
	tmp.m[2][2] = (oneMinusCos * zz) + cosAngle;
	tmp.m[2][3] = 0.0F;

	tmp.m[3][0] = 0.0F;
	tmp.m[3][1] = 0.0F;
	tmp.m[3][2] = 0.0F;
	tmp.m[3][3] = 1.0F;

	this->multiply(tmp);
}
//视图投影矩阵
void    mat4x4::lookAt(const float_3  &eyePosition, const float_3  &targetPosition, const float_3  &upVector)
{
	mat4x4    tmp, *result = &tmp;
	float_3    N = (eyePosition - targetPosition).normalize();
	float_3    U =upVector.cross(N).normalize() ;
	assert(U.x*U.x+U.y*U.y+U.z*U.z>__EPS__);
	float_3    V = N.cross(U);
	memset(result, 0x0, sizeof(mat4x4));
	result->m[0][0] = U.x;
	result->m[1][0] = U.y;
	result->m[2][0] = U.z;

	result->m[0][1] = V.x;
	result->m[1][1] = V.y;
	result->m[2][1] = V.z;

	result->m[0][2] = N.x;
	result->m[1][2] = N.y;
	result->m[2][2] = N.z;

	result->m[3][0] = -U.dot(eyePosition);
	result->m[3][1] = -V.dot(eyePosition);
	result->m[3][2] = -N.dot(eyePosition);
	result->m[3][3] = 1.0f;
	this->multiply(tmp);
}
//矩阵乘法
void    mat4x4::multiply(const mat4x4 &srcA)
{
	mat4x4     tmp;
	int         i;

	for (i = 0; i < 4; i++)
	{
		tmp.m[i][0] = (m[i][0] * srcA.m[0][0]) +
			(m[i][1] * srcA.m[1][0]) +
			(m[i][2] * srcA.m[2][0]) +
			(m[i][3] * srcA.m[3][0]);

		tmp.m[i][1] = (m[i][0] * srcA.m[0][1]) +
			(m[i][1] * srcA.m[1][1]) +
			(m[i][2] * srcA.m[2][1]) +
			(m[i][3] * srcA.m[3][1]);

		tmp.m[i][2] = (m[i][0] * srcA.m[0][2]) +
			(m[i][1] * srcA.m[1][2]) +
			(m[i][2] * srcA.m[2][2]) +
			(m[i][3] * srcA.m[3][2]);

		tmp.m[i][3] = (m[i][0] * srcA.m[0][3]) +
			(m[i][1] * srcA.m[1][3]) +
			(m[i][2] * srcA.m[2][3]) +
			(m[i][3] * srcA.m[3][3]);
	}

	memcpy(this, &tmp, sizeof(mat4x4));
}
//二次矩阵乘法
void     mat4x4::multiply(mat4x4 &srcA, mat4x4 &srcB)
{
	float    mm[4][4];
	float		(*p)[4]=mm;
	if (this != &srcA && this != &srcB)
		p = m;

	for (int i = 0; i < 4; ++i)
	{
		p[i][0] = (srcA.m[i][0] * srcB.m[0][0]) +(srcA.m[i][1] * srcB.m[1][0]) +(srcA.m[i][2] * srcB.m[2][0]) +(srcA.m[i][3] * srcB.m[3][0]);

		p[i][1] = (srcA.m[i][0] * srcB.m[0][1]) +(srcA.m[i][1] * srcB.m[1][1]) +(srcA.m[i][2] * srcB.m[2][1]) +(srcA.m[i][3] * srcB.m[3][1]);

		p[i][2] = (srcA.m[i][0] * srcB.m[0][2]) +(srcA.m[i][1] * srcB.m[1][2]) +(srcA.m[i][2] * srcB.m[2][2]) +(srcA.m[i][3] * srcB.m[3][2]);

		p[i][3] = (srcA.m[i][0] * srcB.m[0][3]) +(srcA.m[i][1] * srcB.m[1][3]) +(srcA.m[i][2] * srcB.m[2][3]) +(srcA.m[i][3] * srcB.m[3][3]);
	}
	if( p != m)
		memcpy(m, p, sizeof(mat4x4));
}
//运算符重载
mat4x4     mat4x4::operator*(const mat4x4   &srcA)const
{
	mat4x4  tmp;
	int         i;

	for (i = 0; i < 4; i++)
	{
		tmp.m[i][0] = (m[i][0] * srcA.m[0][0]) +
			(m[i][1] * srcA.m[1][0]) +
			(m[i][2] * srcA.m[2][0]) +
			(m[i][3] * srcA.m[3][0]);

		tmp.m[i][1] = (m[i][0] * srcA.m[0][1]) +
			(m[i][1] * srcA.m[1][1]) +
			(m[i][2] * srcA.m[2][1]) +
			(m[i][3] * srcA.m[3][1]);

		tmp.m[i][2] = (m[i][0] * srcA.m[0][2]) +
			(m[i][1] * srcA.m[1][2]) +
			(m[i][2] * srcA.m[2][2]) +
			(m[i][3] * srcA.m[3][2]);

		tmp.m[i][3] = (m[i][0] * srcA.m[0][3]) +
			(m[i][1] * srcA.m[1][3]) +
			(m[i][2] * srcA.m[2][3]) +
			(m[i][3] * srcA.m[3][3]);
	}
	return tmp;
}
float_4  mat4x4::operator*(const float_4  &vec)const
{
	float   x, y, z, w;
	x = vec.x*m[0][0] + vec.y*m[1][0] + vec.z*m[2][0] + vec.w*m[3][0];
	y = vec.x*m[0][1] + vec.y*m[1][1] + vec.z*m[2][1] + vec.w*m[3][1];
	z = vec.x*m[0][2] + vec.y*m[1][2] + vec.z*m[2][2] + vec.w*m[3][2];
	w = vec.x*m[0][3] + vec.y*m[1][3] + vec.z*m[2][3] + vec.w*m[3][3];
	return float_4(x, y, z, w);
}

mat4x4&    mat4x4::operator=(const mat4x4  &src)
{
	if (this!=&src)
	memcpy(this,&src,sizeof(mat4x4));
	return *this;
}
//正交投影
void    mat4x4::orthoProject(float  left, float right, float  bottom, float  top, float  nearZ, float  farZ)
{
	float       deltaX = right - left;
	float       deltaY = top - bottom;
	float       deltaZ = farZ - nearZ;
	assert(deltaX > 0.0f && deltaY > 0.0f && deltaZ > 0.0f);
	mat4x4    ortho;

	ortho.m[0][0] = 2.0f / deltaX;
	ortho.m[3][0] = -(right + left) / deltaX;
	ortho.m[1][1] = -2.0f / deltaY;
	ortho.m[3][1] = (top + bottom) / deltaY;
	ortho.m[2][2] = -1.0f / deltaZ;
	ortho.m[3][2] =- nearZ / deltaZ;

	this->multiply(ortho);
}

void    mat4x4::createOrtho(float left, float right, float bottom, float top, float nearZ, float farZ, mat4x4 &proj)
{
	float       deltaX = right - left;
	float       deltaY = top - bottom;
	float       deltaZ = farZ - nearZ;
	assert(deltaX > 0.0f && deltaY > 0.0f && deltaZ > 0.0f);

	proj.m[0][0] = 2.0f / deltaX;
	proj.m[3][0] = -(right + left) / deltaX;
	proj.m[1][1] = -2.0f / deltaY;
	proj.m[3][1] = (top + bottom) / deltaY;
	proj.m[2][2] = -1.0f / deltaZ;
	proj.m[3][2] = -nearZ / deltaZ;
}
void    mat4x4::frustum(float left, float right, float bottom, float top, float nearZ, float farZ)
{
	float       deltaX = right - left;
	float       deltaY = top - bottom;
	float       deltaZ = farZ - nearZ;
	mat4x4    frust;

	assert(deltaX > 0.0f && deltaY > 0.0f && deltaZ > 0.0f && nearZ > 0.0f);

	frust.m[0][0] = 2.0f * nearZ / deltaX;
	frust.m[0][1] = frust.m[0][2] = frust.m[0][3] = 0.0f;

	frust.m[1][1] = -2.0f * nearZ / deltaY;
	frust.m[1][0] = frust.m[1][2] = frust.m[1][3] = 0.0f;

	frust.m[2][0] = (right + left) / deltaX;
	frust.m[2][1] = -(top + bottom) / deltaY;
	frust.m[2][2] = - farZ / deltaZ;
	frust.m[2][3] = -1.0f;

	frust.m[3][2] = -nearZ * farZ / deltaZ;
	frust.m[3][0] = frust.m[3][1] = frust.m[3][3] = 0.0f;

	this->multiply(frust);
}

void    mat4x4::perspective(float fovy, float aspect, float nearZ, float farZ)
{
	float frustumW, frustumH;

	frustumH = tanf(fovy / 360.0f * M_PI) * nearZ;
	frustumW = frustumH * aspect;

	this->frustum(-frustumW, frustumW, -frustumH, frustumH, nearZ, farZ);
}

void mat4x4::createPerspective(float fov, float aspect, float nearZ, float farZ,mat4x4 &proj)
{
	float frustumH = tanf(fov / 360.0f * M_PI) * nearZ;
	float frustumW = frustumH * aspect;
	//
	float       deltaX = 2* frustumW;
	float       deltaY = 2* frustumH;
	float       deltaZ = farZ - nearZ;

	assert(deltaX > 0.0f && deltaY > 0.0f && deltaZ > 0.0f && nearZ > 0.0f);

	proj.m[0][0] = 2.0f * nearZ / deltaX;//==>deltaX*aspect/nearZ * 0.5==>tanf(fov / 360.0f * M_PI)*aspect
	proj.m[0][1] = proj.m[0][2] = proj.m[0][3] = 0.0f;

	proj.m[1][1] = -2.0f * nearZ / deltaY;
	proj.m[1][0] = proj.m[1][2] = proj.m[1][3] = 0.0f;

	proj.m[2][0] = 0;
	proj.m[2][1] = 0;
	proj.m[2][2] = - farZ/ deltaZ;
	proj.m[2][3] = -1.0f;

	proj.m[3][2] = - nearZ * farZ / deltaZ;
	proj.m[3][0] = proj.m[3][1] = proj.m[3][3] = 0.0f;
} 

mat4x4             mat4x4::reverse()const
{
	mat4x4     tmp;
	float             _det = this->det();
	assert(fabs(_det) > __EPS__);
	float_3    row1, row2, row3;
	int        _index[4];
	for (int i = 0; i < 4; ++i)
	{
		static_fix_func(_index, i);
		int     a = _index[0], b = _index[1], c = _index[2];
		//i,0
		row1 = float_3(m[a][1], m[a][2], m[a][3]);
		row2 = float_3(m[b][1], m[b][2], m[b][3]);
		row3 = float_3(m[c][1], m[c][2],m[c][3]);
		tmp.m[0][i] = __SIGNFloat(i + 0)*detVector3(&row1, &row2, &row3);
		//i,1
		row1 = float_3(m[a][0], m[a][2], m[a][3]);
		row2 = float_3(m[b][0], m[b][2], m[b][3]);
		row3 = float_3(m[c][0], m[c][2],m[c][3]);
		tmp.m[1][i] = __SIGNFloat(i + 1)*detVector3(&row1, &row2, &row3);
		//i,2
		row1 = float_3(m[a][0], m[a][1], m[a][3]);
		row2 = float_3(m[b][0], m[b][1], m[b][3]);
		row3 = float_3(m[c][0], m[c][1], m[c][3]);
		tmp.m[2][i] = __SIGNFloat(i + 2)*detVector3(&row1, &row2, &row3);
		//i,3
		row1 = float_3(m[a][0], m[a][1], m[a][2]);
		row2 = float_3(m[b][0], m[b][1], m[b][2]);
		row3 = float_3(m[c][0], m[c][1], m[c][2]);
		tmp.m[3][i] = __SIGNFloat(i + 3)*detVector3(&row1, &row2, &row3);
	}
	return   tmp;
}

void mat4x4::reverse(mat4x4 &rm)const
{
	float             det = this->det();
	assert(fabs(det) > __EPS__);
	float_3    row1, row2, row3;
	int        index[4];
	for (int i = 0; i < 4; ++i)
	{
		static_fix_func(index, i);
		int     a = index[0], b = index[1], c = index[2];
		//i,0
		row1 = float_3(m[a][1], m[a][2], m[a][3]);
		row2 = float_3(m[b][1], m[b][2], m[b][3]);
		row3 = float_3(m[c][1], m[c][2], m[c][3]);
		rm.m[0][i] = __SIGNFloat(i + 0)*detVector3(&row1, &row2, &row3)/ det;
		//i,1
		row1 = float_3(m[a][0], m[a][2], m[a][3]);
		row2 = float_3(m[b][0], m[b][2], m[b][3]);
		row3 = float_3(m[c][0], m[c][2], m[c][3]);
		rm.m[1][i] = __SIGNFloat(i + 1)*detVector3(&row1, &row2, &row3)/ det;
		//i,2
		row1 = float_3(m[a][0], m[a][1], m[a][3]);
		row2 = float_3(m[b][0], m[b][1], m[b][3]);
		row3 = float_3(m[c][0], m[c][1], m[c][3]);
		rm.m[2][i] = __SIGNFloat(i + 2)*detVector3(&row1, &row2, &row3)/ det;
		//i,3
		row1 = float_3(m[a][0], m[a][1], m[a][2]);
		row2 = float_3(m[b][0], m[b][1], m[b][2]);
		row3 = float_3(m[c][0], m[c][1], m[c][2]);
		rm.m[3][i] = __SIGNFloat(i + 3)*detVector3(&row1, &row2, &row3)/ det;
	}
}
//行列式
float           mat4x4::det()const
{
	float_3   row1, row2, row3;
	float  det;
	//0,0
	row1 = float_3(m[1][1], m[1][2], m[1][3]);
	row2 = float_3(m[2][1], m[2][2], m[2][3]);
	row3 = float_3(m[3][1], m[3][2], m[3][3]);
	det = m[0][0] * detVector3(&row1, &row2, &row3);
	//0,1
	row1 = float_3(m[1][0], m[1][2], m[1][3]);
	row2 = float_3(m[2][0], m[2][2], m[2][3]);
	row3 = float_3(m[3][0], m[3][2], m[3][3]);
	det -= m[0][1] * detVector3(&row1, &row2, &row3);
	//0,2
	row1 = float_3(m[1][0], m[1][1], m[1][3]);
	row2 = float_3(m[2][0], m[2][1], m[2][3]);
	row3 = float_3(m[3][0], m[3][1], m[3][3]);
	det += m[0][2] * detVector3(&row1, &row2, &row3);
	//0,3
	row1 = float_3(m[1][0], m[1][1], m[1][2]);
	row2 = float_3(m[2][0], m[2][1], m[2][2]);
	row3 = float_3(m[3][0], m[3][1], m[3][2]);
	det -= m[0][3] * detVector3(&row1, &row2, &row3);
	return det;
}

mat3x3     mat4x4::normalMatrix()const
{
	mat3x3      tmp;
	float            temp;
	tmp.m[0][0] = m[0][0];
	tmp.m[0][1] = m[0][1];
	tmp.m[0][2] = m[0][2];

	tmp.m[1][0] = m[1][0];
	tmp.m[1][1] = m[1][1];
	tmp.m[1][2] = m[1][2];

	tmp.m[2][0] = m[2][0];
	tmp.m[2][1] = m[2][1];
	tmp.m[2][2] = m[2][2];
//求逆矩阵
	tmp = tmp.reverse();
//转置
#define     _SWAP_MAT3_(i,k) temp=tmp.m[i][k],  tmp.m[i][k]=tmp.m[k][i],tmp.m[k][i]=temp;
	_SWAP_MAT3_(0, 1)
    _SWAP_MAT3_(0, 2)
	_SWAP_MAT3_(1, 2)
#undef _SWAP_MAT3_
   return  tmp;
}

mat3x3    mat4x4::trunk()const
{
	mat3x3   tmp;
#define   _MATRIX_TRUNK_(i,k)   tmp.m[i][k]=m[i][k]
	_MATRIX_TRUNK_(0, 0);
	_MATRIX_TRUNK_(0, 1);
	_MATRIX_TRUNK_(0, 2);

	_MATRIX_TRUNK_(1, 0);
	_MATRIX_TRUNK_(1, 1);
	_MATRIX_TRUNK_(1, 2);

	_MATRIX_TRUNK_(2, 0);
	_MATRIX_TRUNK_(2, 1);
	_MATRIX_TRUNK_(2, 2);
#undef _MATRIX_TRUNK_
	return tmp;
}

void mat4x4::trunk(mat3x3 &input)const
{
#define   _MATRIX_TRUNK_(i,k)   input.m[i][k]=m[i][k]
	_MATRIX_TRUNK_(0, 0);
	_MATRIX_TRUNK_(0, 1);
	_MATRIX_TRUNK_(0, 2);

	_MATRIX_TRUNK_(1, 0);
	_MATRIX_TRUNK_(1, 1);
	_MATRIX_TRUNK_(1, 2);

	_MATRIX_TRUNK_(2, 0);
	_MATRIX_TRUNK_(2, 1);
	_MATRIX_TRUNK_(2, 2);
#undef _MATRIX_TRUNK_
}
//偏置矩阵
void     mat4x4::offset()
{
#define    _OFFSET_MATRIX(i,k)  m[i][k]=m[i][k]*0.5f+temp
	float  temp = m[0][3] * 0.5f;
	_OFFSET_MATRIX(0, 0);
	_OFFSET_MATRIX(0, 1);
	_OFFSET_MATRIX(0, 2);

	temp = m[1][3] * 0.5f;
	_OFFSET_MATRIX(1, 0);
	_OFFSET_MATRIX(1, 1);
	_OFFSET_MATRIX(1, 2);

	temp = m[2][3] * 0.5f;
	_OFFSET_MATRIX(2, 0);
	_OFFSET_MATRIX(2, 1);
	_OFFSET_MATRIX(2, 2);

	temp = m[3][3] * 0.5f;
	_OFFSET_MATRIX(3, 0);
	_OFFSET_MATRIX(3, 1);
	_OFFSET_MATRIX(3, 2);
#undef _OFFSET_MATRIX
}
//////////////////////////////三维矩阵//////////////////////////
mat3x3::mat3x3()
{
	m[0][0] = 1.0f; m[0][1] = 0.0f, m[0][2] = 0.0f;
	m[1][0] = 0.0f, m[1][1] = 1.0f, m[1][2] = 0.0f;
	m[2][0] = 0.0f, m[2][1] = 0.0f, m[2][2] = 1.0f;
}

mat3x3::mat3x3(const float_3 &row0, const float_3 &row1, const float_3 &row2)
{
	m[0][0] = row0.x, m[0][1] = row0.y, m[0][2] = row0.z;
	m[1][0] = row1.x, m[1][1] = row1.y, m[1][2] = row1.z;
	m[2][0] = row2.x, m[2][1] = row2.y, m[2][2] = row2.z;
}

mat3x3::mat3x3(const mat4x4 &mat4)
{
	m[0][0] = mat4.m[0][0];
	m[0][1] = mat4.m[0][1];
	m[0][2] = mat4.m[0][2];

	m[1][0] = mat4.m[1][0];
	m[1][1] = mat4.m[1][1];
	m[1][2] = mat4.m[1][2];

	m[2][0] = mat4.m[2][0];
	m[2][1] = mat4.m[2][1];
	m[2][2] = mat4.m[2][2];
}

mat3x3     mat3x3::reverse()const
{
	mat3x3    tmp;
	float     det = this->det();
	assert(fabs(det) > __EPS__);

	tmp.m[0][0] = detFloat(m[1][1], m[1][2], m[2][1], m[2][2]) / det;
	tmp.m[1][0] = -detFloat(m[1][0], m[1][2], m[2][0], m[2][2]) / det;
	tmp.m[2][0] = detFloat(m[1][0], m[1][1], m[2][0], m[2][1]) / det;

	tmp.m[0][1] = -detFloat(m[0][1], m[0][2], m[2][1], m[2][2]) / det;
	tmp.m[1][1] = detFloat(m[0][0], m[0][2], m[2][0], m[2][2]) / det;
	tmp.m[2][1] = -detFloat(m[0][0], m[0][1], m[2][0], m[2][1]) / det;

	tmp.m[0][2] = detFloat(m[0][1], m[0][2], m[1][1], m[1][2]) / det;
	tmp.m[1][2] = -detFloat(m[0][0], m[0][2], m[1][0], m[1][2]) / det;
	tmp.m[2][2] = detFloat(m[0][0], m[0][1], m[1][0], m[1][1]) / det;
	return  tmp;
}

void mat3x3::reverse(mat3x3 &tmp)const
{
	float     det = this->det();
	assert(fabs(det) > __EPS__);

	tmp.m[0][0] = detFloat(m[1][1], m[1][2], m[2][1], m[2][2]) / det;
	tmp.m[1][0] = -detFloat(m[1][0], m[1][2], m[2][0], m[2][2]) / det;
	tmp.m[2][0] = detFloat(m[1][0], m[1][1], m[2][0], m[2][1]) / det;

	tmp.m[0][1] = -detFloat(m[0][1], m[0][2], m[2][1], m[2][2]) / det;
	tmp.m[1][1] = detFloat(m[0][0], m[0][2], m[2][0], m[2][2]) / det;
	tmp.m[2][1] = -detFloat(m[0][0], m[0][1], m[2][0], m[2][1]) / det;

	tmp.m[0][2] = detFloat(m[0][1], m[0][2], m[1][1], m[1][2]) / det;
	tmp.m[1][2] = -detFloat(m[0][0], m[0][2], m[1][0], m[1][2]) / det;
	tmp.m[2][2] = detFloat(m[0][0], m[0][1], m[1][0], m[1][1]) / det;
}

void     mat3x3::rotate(float angle, float x, float y, float z)
{
	float mag = sqrtf(x * x + y * y + z * z);

	const float sinAngle = sinf(angle * M_PI / 180.0f);
	const float cosAngle = cosf(angle * M_PI / 180.0f);

	assert(mag > 0.0f);
	float xx, yy, zz, xy, yz, zx, xs, ys, zs;
	float oneMinusCos;

	x /= mag; y /= mag;  z /= mag;
	xx = x * x;  yy = y * y;   zz = z * z;
	xy = x * y;  yz = y * z;    zx = z * x;

	xs = x * sinAngle;  ys = y * sinAngle;   zs = z * sinAngle;   oneMinusCos = 1.0f - cosAngle;

	m[0][0] = (oneMinusCos * xx) + cosAngle;
	m[0][1] = (oneMinusCos * xy) + zs;
	m[0][2] = (oneMinusCos * zx) - ys;

	m[1][0] = (oneMinusCos * xy) - zs;
	m[1][1] = (oneMinusCos * yy) + cosAngle;
	m[1][2] = (oneMinusCos * yz) + xs;

	m[2][0] = (oneMinusCos * zx) + ys;
	m[2][1] = (oneMinusCos * yz) - xs;
	m[2][2] = (oneMinusCos * zz) + cosAngle;
}

void    mat3x3::rotate(float angle, const float_3 &axis)
{
	float_3 newAxis = axis.normalize();
	float xx, yy, zz, xy, yz, zx, xs, ys, zs;
	float oneMinusCos;
	const float sinAngle = sinf(angle * M_PI / 180.0f);
	const float cosAngle = cosf(angle * M_PI / 180.0f);
	xx = newAxis.x * newAxis.x;  yy = newAxis.y * newAxis.y;   zz = newAxis.z * newAxis.z;
	xy = newAxis.x * newAxis.y;  yz = newAxis.y * newAxis.z;    zx = newAxis.z * newAxis.x;

	xs = newAxis.x * sinAngle;  ys = newAxis.y * sinAngle;   zs = newAxis.z * sinAngle;   oneMinusCos = 1.0f - cosAngle;

	m[0][0] = (oneMinusCos * xx) + cosAngle;
	m[0][1] = (oneMinusCos * xy) + zs;
	m[0][2] = (oneMinusCos * zx) - ys;

	m[1][0] = (oneMinusCos * xy) - zs;
	m[1][1] = (oneMinusCos * yy) + cosAngle;
	m[1][2] = (oneMinusCos * yz) + xs;

	m[2][0] = (oneMinusCos * zx) + ys;
	m[2][1] = (oneMinusCos * yz) - xs;
	m[2][2] = (oneMinusCos * zz) + cosAngle;
}

void   mat3x3::scale(const float_3 &scaleFactor)
{
	m[0][0] = scaleFactor.x;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = scaleFactor.y;
	m[1][2] = 0.0f;
	
	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = scaleFactor.z;
}

void mat3x3::scale(float x, float y, float z)
{
	m[0][0] = x;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = y;
	m[1][2] = 0.0f;

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = z;
}

float     mat3x3::det()const
{
	float     result;
	result = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]);
	result -= m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]);
	result += m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
	return result;
}

void mat3x3::tbn(const float_3 &normal, mat3x3 &tbn)
{
	float_3 X, Y;
	float_3::generateViewXY(normal, X, Y);
	//组合成tbn矩阵
	tbn.m[0][0] = X.x; tbn.m[0][1] = X.y; tbn.m[0][2] = X.z;
	tbn.m[1][0] = Y.x; tbn.m[1][1] = Y.y; tbn.m[1][2] = Y.z;
	tbn.m[2][0] = normal.x; tbn.m[2][1] = normal.y; tbn.m[2][2] = normal.z;
}

void mat3x3::reverseTBN(const float_3 &normal, mat3x3 &tbnr)
{
	float_3 X, Y;
	//float           mm[3][3];
	//mat3x3   &tmp = *(mat3x3*)mm;
	float_3::generateViewXY(normal, X, Y);
	//
	//tmp.m[0][0] = X.x; tmp.m[0][1] = X.y; tmp.m[0][2] = X.z;
	//tmp.m[1][0] = Y.x; tmp.m[1][1] = Y.y; tmp.m[1][2] = Y.z;
	//tmp.m[2][0] = normal.x; tmp.m[2][1] = normal.y; tmp.m[2][2] = normal.z;
	//组合成TBN矩阵的逆矩阵
	tbnr.m[0][0] = X.x; tbnr.m[0][1] = Y.x; tbnr.m[0][2] = normal.x;
	tbnr.m[1][0] = X.y; tbnr.m[1][1] = Y.y; tbnr.m[1][2] = normal.y;
	tbnr.m[2][0] = X.z; tbnr.m[2][1] = Y.z; tbnr.m[2][2] = normal.z;

	//tmp.reverse(tbnr);
}
//右乘三维向量
float_3     mat3x3::operator*(const float_3 &vec)const
{
	float x, y, z;
	x = m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z;
	y = m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z;
	z = m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z;

	return  float_3(x,y,z);
}

mat3x3 mat3x3::operator*(const mat3x3 &src)const
{
	mat3x3  tmp;
	for (int i = 0; i < 3; ++i)
	{
		tmp.m[i][0] = m[i][0] * src.m[0][0] + m[i][1] * src.m[1][0] + m[i][2] * src.m[2][0];
		tmp.m[i][1] = m[i][0] * src.m[0][1] + m[i][1] * src.m[1][1] + m[i][2] * src.m[2][1];
		tmp.m[i][2] = m[i][0] * src.m[0][2] + m[i][1] * src.m[1][2] + m[i][2] * src.m[2][2];
	}
	return tmp;
}

mat3x3&    mat3x3::operator=(mat3x3  &src)
{
	if (this != &src)
		memcpy(m,src.m,sizeof(mat3x3));
	return src;
}

