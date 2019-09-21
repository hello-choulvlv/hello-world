//Author:xiaohuaxiong
//Version:1.0 �ṩ�˶Ծ����������Ĳ���,���� ��λ����,��ת����,ƽ�ƾ���,���ž���,ͶӰ����,��ͼ����,����˷�
//Version:2.0�����˾������,�Լ�3ά���������,2,3,4ά����������,���ṩ�˶������Ļ�������<��λ��,���,���,����>��֧��
//Version:3.0 �����˶Ծ���ֱ�������֧��,�Լ����߾�����Ƶ�,����ʽ��ֱ��֧��,���������ĳ˷�
//Version:4.0 �����˶�ƫ�þ����֧��,����ֱ�Ӷ���Ӱ���в���
//Version 5.0 �����е��йؾ���Ĳ������뵽��������,��Ϊ����ĳ�Ա����ʵ��,���Ժ��ʵ�ʿ�����,
//                    �Ƽ�ʹ���µ��ຯ��,��Ϊ���ǵĽӿڸ��Ѻ�,������
//Version 6.0 �����ߵļ������뵽����,������,��������������㷨��
//Version 7.0:������������ߵļ���,��ԭ����ֱ�����Ǻ���������������Ĺ���x��ƫ��
//Version 8.0:�����˶���Ԫ����֧��
//Version 9.0:�����˹�����Ԫ�������֮���ת�����bug,�Լ�Matrix.scale������bug
//Version 10.0 �������ļ��й��ھ������ʵ��,ɾ������ʷ�����ľ�����,�������Ĺ��̺��������ṩ֧��,������Ԫ������ֲ����һ���������ļ���
//Version 11.0 �����˿ռ�ƽ�淽����,��Χ��,��ƽ��ͷ��������һ���ļ���
//  Includes
//
#include<engine/Geometry.h>
#include<engine/GLState.h>
#include <math.h>
#include<string.h>
#include<assert.h>
#define PI 3.1415926535f
#define    __EPS__  0.000001f
__NS_GLK_BEGIN
//���������ߵļ���
int  esGenSphere(int numSlices, float radius, float **vertices, float **normals, float  **tangents,
	float **texCoords, int **indices ,int  *numberOfVertex)
{
	int i,j;
	int numParallels = numSlices / 2;
	int numVertices = ( numParallels + 1 ) * ( numSlices + 1 );
	int numIndices = numParallels * numSlices * 6;
	float angleStep = 2.0f * PI /numSlices;
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

	for ( i = 0; i < numParallels + 1; i++ )//Y����Ƭ,ֻ��Ҫ180�ȼ���
	{
		const float   _real_radius=radius*sinf(angleStep*i);
		const float   _real_cos=radius*cosf(angleStep*i);
		for ( j = 0; j < numSlices + 1; j++ )
		{
			int vertex = ( i * ( numSlices + 1 ) + j ) * 3;

			if ( _vertex )
			{
				_vertex[vertex + 0] = _real_radius *sinf ( angleStep * j );//��ΪangleStep����Z��ļн�,����ȡ����ֵ
				_vertex [vertex + 1] = _real_cos;//��y�����϶��µļн�
				_vertex[vertex + 2] = _real_radius *cosf ( angleStep * j );//��Z��ļн�
			}

			if ( _normal )
			{
				_normal[vertex + 0] = _vertex[vertex + 0] / radius;
				_normal[vertex + 1] = _vertex [vertex + 1] / radius;
				_normal[vertex + 2] = _vertex [vertex + 2] / radius;
			}
//���������淽�̹���x��ƫ����
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
//��������,��Ҫ������ȫ���Ķ���,��������
// Generate the indices
	GLVector3     *originVertex = (GLVector3 *)_vertex;
	GLVector2     *originTex = (GLVector2  *)_texCoord;
	GLVector3     *originTangent = (GLVector3 *)_tangent;
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
				int         _pindex1, _pindex2, _pindex3;
				_pindex1 =  i * vSkipSlice + j;
				_pindex2 = vHead * vSkipSlice + j;
				_pindex3 = vHead* vSkipSlice + vTail;

				*indexBuf++ = _pindex1;
				*indexBuf++ = _pindex2;
				*indexBuf++ = _pindex3;

				GLVector2    deltaUV1 = originTex[_pindex2] - originTex[_pindex1];
				GLVector2    deltaUV2 = originTex[_pindex3] - originTex[_pindex1];

				float     _lamda = 1.0f / (deltaUV1.x*deltaUV2.y - deltaUV1.y*deltaUV2.x);

				GLVector3       e1 = originVertex[_pindex2] - originVertex[_pindex1];
				GLVector3       e2 = originVertex[_pindex3] - originVertex[_pindex1];

				float     x1 = (deltaUV2.y*e1.x - deltaUV1.y*e2.x)*_lamda;
				float     y1 = (deltaUV2.y*e1.y - deltaUV1.y*e2.y)*_lamda;
				float     z1 = (deltaUV2.y*e1.z - deltaUV1.y*e2.z)*_lamda;

				//float    x2 = -_v1*e1.x + _u1*e2.x;
				//float    y2 = -_v1*e1.y + _u1*e2.y;
				//float    z2 = -_v1*e1.z + _u1*e2.z;
				GLVector3    _tangentValue(x1,y1,z1);
				originTangent[_pindex1] = _tangentValue;
				originTangent[_pindex2] = _tangentValue;
				originTangent[_pindex3] = _tangentValue;


				_pindex1 =i * vSkipSlice + j ;
				_pindex2 = vHead *vSkipSlice + vTail;
				_pindex3 =i * vSkipSlice + vTail ;

				*indexBuf++ = _pindex1;
				*indexBuf++ = _pindex2;
				*indexBuf++ =_pindex3 ;

				deltaUV1 = originTex[_pindex1] - originTex[_pindex3];
				deltaUV2 = originTex[_pindex2] - originTex[_pindex3];

				_lamda = 1.0f / (deltaUV1.x*deltaUV2.y - deltaUV1.y*deltaUV2.x);

				e1 = originVertex[_pindex1] - originVertex[_pindex3];
				e2 = originVertex[_pindex2] - originVertex[_pindex3];

				 x1 = (deltaUV2.y*e1.x - deltaUV1.y*e2.x)*_lamda;
				 y1 = (deltaUV2.y*e1.y - deltaUV1.y*e2.y)*_lamda;
				 z1 = (deltaUV2.y*e1.z - deltaUV1.y*e2.z)*_lamda;

				 _tangentValue=GLVector3(x1, y1, z1);
				 originTangent[_pindex1] = _tangentValue;
				 originTangent[_pindex2] = _tangentValue;
				 originTangent[_pindex3] = _tangentValue;
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
//�������������۲�,���е������ζ����������
	float cubeVerts[] =
	{
//ǰ
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
//��
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
//��
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
//��
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
//��
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
//��
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
	};
	//����
	float    tangent[] = {
		//ǰ
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		//��
		-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		//��
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		//��
		0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
		//��
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		//��
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	};
	float cubeNormals[] =
	{
//ǰ
		0.0f,0.0f,1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 
		0.0f, 0.0f, 1.0f,
//��
		0.0f,0.0f,-1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
//��
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
//��
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
//��
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
//��
		0.0f, -1.0f,0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
	};

	float cubeTex[] =
	{
//ǰ
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
//��
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
//��
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
//��
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
//��
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
//��
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
GLVector3     cross(GLVector3  *a,GLVector3  *b)
{
	float   x = a->y*b->z - a->z*b->y;
	float   y = a->z*b->x - a->x*b->z;
	float   z = a->x*b->y - a->y*b->x;

	return  GLVector3(x,y,z);
}
//vector dot
float       dot(GLVector3  *srcA, GLVector3  *srcB)
{
	return    srcA->x*srcB->x + srcA->y*srcB->y + srcA->z*srcB->z;
}

//����ʽ����
float     detFloat(float x1, float y1, float x2, float y2)
{
	return  x1*y2 - y1*x2;
}
float    detVector2(GLVector2   *a, GLVector2   *b)
{
	return a->x*b->y - a->y*b->x;
}
//
float   detVector3(GLVector3    *a,GLVector3       *b,GLVector3  *c)
{
	float   _result = 0;
	_result =a->x* (b->y*c->z-b->z*c->y);
	_result -= a->y*(b->x*c->z-b->z*c->x);
	_result += a->z*(b->x*c->y-b->y*c->x);
	return _result;
}
//��ά�������
static   void   static_fix_func(int  *_index,int  _current)//��������
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
/////////////////////////////��ά,��ά,��ά�����ҳ˾���//////////////////////////////////////
GLVector2    GLVector2::operator*(float  _factor)const
{
	return   GLVector2(x*_factor,y*_factor);
}
GLVector2   GLVector2::operator*(const GLVector2  &_mfactor)const
{
	return  GLVector2(x*_mfactor.x,y*_mfactor.y);
}
GLVector2   GLVector2::operator+(const GLVector2  &_factor)const
{
	return  GLVector2(x+_factor.x,y+_factor.y);
}
GLVector2   GLVector2::operator-(const GLVector2  &_factor)const
{
	return  GLVector2(x-_factor.x,y-_factor.y);
}
GLVector2   GLVector2::operator/(float _factor)const
{
	return  GLVector2(x/_factor,y/_factor);
}
GLVector2    GLVector2::operator/(const GLVector2  &_factor)const
{
	return  GLVector2(x/_factor.x,y/_factor.y);
}

GLVector2& GLVector2::operator=(const GLVector2 &src)
{
	x = src.x, y = src.y;
	return *this;
}

GLVector2& GLVector2::operator+=(const GLVector2 &src)
{
	x += src.x;
	y += src.y;
	return *this;
}

GLVector2& GLVector2::operator-=(const GLVector2 &src)
{
	x += src.x;
	y += src.y;
	return *this;
}

GLVector2& GLVector2::operator*=(const GLVector2 &src)
{
	x += src.x;
	y += src.y;
	return *this;
}

GLVector2& GLVector2::operator/=(const GLVector2 &src)
{
	x+=src.x;
	y += src.y;
	return *this;
}

GLVector2   GLVector2::normalize()const
{
	float  _length = sqrtf(x*x+y*y);
	assert(_length>=__EPS__);
	return  GLVector2(x/_length,y/_length);
}
float     GLVector2::dot(const GLVector2 &other)const
{
	return x*other.x + y*other.y;
}

const float GLVector2::length()const
{
	return sqrtf(x*x+y*y);
}
/////////////////////////////333333333333333333////////////////////////////////////
GLVector4   GLVector3::xyzw0()const
{
	return GLVector4(x,y,z,0.0f);
}

GLVector4 GLVector3::xyzw1()const
{
	return GLVector4(x,y,z,1.0f);
}
GLVector3   GLVector3::operator*(const Matrix3 &src)const
{
	float  x, y, z;
	x = this->x*src.m[0][0] + this->y*src.m[1][0] + this->z*src.m[2][0];
	y = this->x*src.m[0][1] + this->y*src.m[1][1] + this->z*src.m[2][1];
	z = this->x*src.m[0][2] + this->y*src.m[1][2] + this->z*src.m[2][2];

	return  GLVector3(x,y,z);
}

GLVector4 GLVector3::operator*(const Matrix &mat)const
{
	float  x, y, z,w;
	x = this->x*mat.m[0][0] + this->y*mat.m[1][0] + this->z*mat.m[2][0] + mat.m[3][0];
	y = this->x*mat.m[0][1] + this->y*mat.m[1][1] + this->z*mat.m[2][1] + mat.m[3][1];
	z = this->x*mat.m[0][2] + this->y*mat.m[1][2] + this->z*mat.m[2][2] + mat.m[3][2];
	w = this->x * mat.m[0][3] + this->y * mat.m[1][3] + this->z*mat.m[2][3] + mat.m[3][3];

	return  GLVector4(x, y, z,w);
}

GLVector3    GLVector3::operator*(const float   _factor)const
{
	return  GLVector3(x*_factor,y*_factor,z*_factor);
}
GLVector3   GLVector3::operator*(const GLVector3  &_factor)const
{
	return  GLVector3(x*_factor.x,y*_factor.y,z*_factor.z);
}
GLVector3   GLVector3::operator+(const GLVector3  &_factor)const
{
	return  GLVector3(x+_factor.x,y+_factor.y,z+_factor.z);
}
GLVector3   GLVector3::operator-(const GLVector3 &_factor)const
{
	return  GLVector3(x-_factor.x,y-_factor.y,z-_factor.z);
}
GLVector3   GLVector3::operator/(const float _factor)const
{
	return  GLVector3(x/_factor,y/_factor,z/_factor);
}
GLVector3   GLVector3::operator/(const GLVector3 &_factor)const
{
	return   GLVector3(x/_factor.x,y/_factor.y,z/_factor.z);
}
GLVector3& GLVector3::operator+=(const GLVector3 &offset)
{
	x += offset.x;
	y += offset.y;
	z += offset.z;
	return *this;
}

GLVector3& GLVector3::operator+=(float f)
{
	x += f;
	y += f;
	z += f;
	return *this;
}

GLVector3& GLVector3::operator-=(const GLVector3 &factor)
{
	x -= factor.x;
	y -= factor.y;
	z -= factor.z;
	return *this;
}
GLVector3& GLVector3::operator-=(float f)
{
	x -= f;
	y -= f;
	z -= f;
	return *this;
}

GLVector3& GLVector3::operator*=(const GLVector3 &factor)
{
	x *= factor.x;
	y *= factor.y;
	z *= factor.z;
	return *this;
}
GLVector3& GLVector3::operator*=(float f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

GLVector3& GLVector3::operator/=(const GLVector3 &factor)
{
	x /= factor.x;
	y /= factor.y;
	z /= factor.z;
	return *this;
}
GLVector3& GLVector3::operator/=(float f)
{
	x /= f;
	y /= f;
	z /= f;
	return *this;
}

GLVector3   GLVector3::normalize()const
{
	float      _length = sqrtf(x*x+y*y+z*z);
	assert(_length>=__EPS__);
	return    GLVector3(x/_length,y/_length,z/_length);
}
GLVector3   GLVector3::cross(const GLVector3 &axis)const
{
	return GLVector3(
		y*axis.z - z*axis.y,
		-x*axis.z + z*axis.x,
		x*axis.y - y*axis.x
		);
}
float    GLVector3::dot(const GLVector3 &other)const
{
	return x*other.x + y*other.y + z*other.z;
}
const float GLVector3::length()const
{
	return sqrtf(x*x+y*y+z*z);
}

GLVector3 GLVector3::min(const GLVector3 &other)const
{
	const float nx = x < other.x ? x : other.x;
	const float ny = y < other.y ? y : other.y;
	const float nz = z < other.z ? z : other.z;
	return GLVector3(nx, ny, nz);
}

GLVector3 GLVector3::max(const GLVector3 &other)const
{
	const float nx = x > other.x ? x : other.x;
	const float ny = y > other.y ? y : other.y;
	const float nz = z > other.z ? z : other.z;
	return GLVector3(nx, ny, nz);
}

void GLVector3::generateViewXY(const GLVector3 &Z, GLVector3 &X, GLVector3 &Y)
{
	GLVector3 upVector(0,1,0);
	GLVector3 rightVector(1,0,0);
	GLVector3 forwardVector(0,0,1);
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
	else if (Z.y ==1)//��ʱ�޷�ȷ�������X/Y�ķ���
	{
		X = rightVector;
		Y = GLVector3(0,0,-1);
	}
}

/////////////////////////4444444444444444///////////////////////////////////////
GLVector4     GLVector4::operator*(const Matrix &src)const
{
	float  nx,ny, nz, nw;
	nx = this->x*src.m[0][0] + this->y*src.m[1][0] + this->z*src.m[2][0] + this->w*src.m[3][0];
	ny = this->x*src.m[0][1] + this->y*src.m[1][1] + this->z*src.m[2][1] + this->w*src.m[3][1];
	nz = this->x*src.m[0][2] + this->y*src.m[1][2] + this->z*src.m[2][2] + this->w*src.m[3][2];
	nw = this->x*src.m[0][3] + this->y*src.m[1][3] + this->z*src.m[2][3] + this->w*src.m[3][3];
	return GLVector4(nx, ny, nz, nw);
}
GLVector4    GLVector4::normalize()const
{
	float   _length = sqrtf(x*x+y*y+z*z+w*w);
	assert(_length>__EPS__);
	return  GLVector4(x/_length,y/_length,z/_length,w/_length);
}
float    GLVector4::dot(const GLVector4 &other)const
{
	return x*other.x + y*other.y + z*other.z + w*other.w;
}

float  GLVector4::length()const
{
	return sqrtf(x*x+y*y+z*z+w*w);
}

float GLVector4::length3()const
{
	return sqrtf(x*x+y*y+z*z);
}

GLVector4 GLVector4::operator*(const float factor)const
{
	return GLVector4(x*factor,y*factor,z*factor,w*factor);
}

GLVector4 GLVector4::operator*(const GLVector4 &other)const
{
	return GLVector4(x*other.x,y*other.y,z*other.z,w*other.w);
}

GLVector4 GLVector4::operator+(const GLVector4 &other)const
{
	return GLVector4(x+other.x,y+other.y,z+other.z,w+other.w);
}

GLVector4 GLVector4::operator/(const GLVector4 &other)const
{
	return GLVector4(x/other.x,y/other.y,z/other.z,w/other.w);
}

const GLVector4& GLVector4::operator/=(const float factor)
{
	x /= factor;
	y /= factor;
	z /= factor;
	w /= factor;
	return *this;
}

const GLVector4& GLVector4::operator*=(const float factor)
{
	x *= factor;
	y *= factor;
	z *= factor;
	w *= factor;
	return *this;
}

GLVector4 GLVector4::operator/(const float factor)const
{
	return GLVector4(x/factor,y/factor,z/factor,w/factor);
}

GLVector4 GLVector4::operator-(const GLVector4 &other)const
{
	return GLVector4(x-other.x,y-other.y,z-other.z,w-other.w);
}

GLVector4 GLVector4::min(const GLVector4 &other)const
{
	const float nx = x < other.x ? x : other.x;
	const float ny = y < other.y ? y : other.y;
	const float nz = z < other.z ? z : other.z;
	const float nw = w < other.w ? w : other.w;
	return GLVector4(nx,ny,nz,nw);
}

GLVector4 GLVector4::max(const GLVector4 &other)const
{
	const float nx = x > other.x ? x : other.x;
	const float ny = y > other.y ? y : other.y;
	const float nz = z > other.z ? z : other.z;
	const float nw = w > other.w ? w : other.w;
	return GLVector4(nx, ny, nz, nw);
}
////////////////////////////��ά����ʵ��//////////////////////////////////
Matrix::Matrix()
{
	m[0][0] = 1.0f, m[0][1] = 0.0f, m[0][2] = 0.0f, m[0][3] = 0.0f;
	m[1][0] = 0.0f, m[1][1] = 1.0f, m[1][2] = 0.0f, m[1][3] = 0.0f;
	m[2][0] = 0.0f, m[2][1] = 0.0f, m[2][2] = 1.0f, m[2][3] = 0.0f;
	m[3][0] = 0.0f, m[3][1] = 0.0f, m[3][2] = 0.0f, m[3][3] = 1.0f;
}

Matrix::Matrix(const GLVector4 &row0, const GLVector4 &row1, const GLVector4 &row2, const GLVector4 &row3)
{
	m[0][0] = row0.x, m[0][1] = row0.y, m[0][2] = row0.z, m[0][3] = row0.w;
	m[1][0] = row1.x, m[1][1] = row1.y, m[1][2] = row1.z, m[1][3] = row1.w;
	m[2][0] = row2.x, m[2][1] = row2.y, m[2][2] = row2.z, m[2][3] = row2.w;
	m[3][0] = row3.x, m[3][1] = row3.y, m[3][2] = row3.z, m[3][3] = row3.w;
}

Matrix::Matrix(const GLVector3 &row1, const GLVector3 &row2, const GLVector3 &row3, const GLVector3 &eyePosition)
{
	m[0][0] = row1.x; m[0][1] = row2.x; m[0][2] = row3.x; m[0][3] = 0;
	m[1][0] = row1.y; m[1][1] = row2.y; m[1][2] = row3.y; m[1][3] = 0;
	m[2][0] = row1.z; m[2][1] = row2.z; m[2][2] = row3.z; m[2][3] = 0;
	m[3][0] = -row1.dot(eyePosition);
	m[3][1] = -row2.dot(eyePosition);
	m[3][2] = -row3.dot(eyePosition);
	m[3][3] = 1;
}

void     Matrix::identity()
{
	m[0][0] = 1.0f, m[0][1] = 0.0f, m[0][2] = 0.0f, m[0][3] = 0.0f;
	m[1][0] = 0.0f, m[1][1] = 1.0f, m[1][2] = 0.0f, m[1][3] = 0.0f;
	m[2][0] = 0.0f, m[2][1] = 0.0f, m[2][2] = 1.0f, m[2][3] = 0.0f;
	m[3][0] = 0.0f, m[3][1] = 0.0f, m[3][2] = 0.0f, m[3][3] = 1.0f;
}
void    Matrix::copy(const Matrix  &srcA)
{
	m[0][0] = srcA.m[0][0], m[0][1] = srcA.m[0][1], m[0][2] = srcA.m[0][2], m[0][3] = srcA.m[0][3];
	m[1][0] = srcA.m[1][0], m[1][1] = srcA.m[1][1], m[1][2] = srcA.m[1][2], m[1][3] = srcA.m[1][3];
	m[2][0] = srcA.m[2][0], m[2][1] = srcA.m[2][1], m[0][2] = srcA.m[2][2], m[2][3] = srcA.m[2][3];
	m[3][0] = srcA.m[3][0], m[3][1] = srcA.m[3][1], m[3][2] = srcA.m[3][2], m[3][3] = srcA.m[3][3];
}
//�ҳ����ž���
void   Matrix::scale(const float scaleX, const float scaleY, const float  scaleZ)
{
	m[0][0] *= scaleX; m[0][1] *= scaleY; m[0][2] *= scaleZ; 
	m[1][0] *= scaleX; m[1][1] *= scaleY; m[1][2] *= scaleZ; 
	m[2][0] *= scaleX; m[2][1] *= scaleY; m[2][2] *= scaleZ; 
	m[3][0] *= scaleX; m[3][1] *= scaleY; m[3][2] *= scaleZ; 
}
//ƽ��
void    Matrix::translate(const float deltaX, const float  deltaY, const float deltaZ)
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

void Matrix::translate(const GLVector3 &deltaXYZ)
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

void    Matrix::rotateX(float pitch)
{
	Matrix  matX;

	const float sinX = sinf(GLK_ANGLE_TO_RADIUS(pitch));
	const float cosX = cosf(GLK_ANGLE_TO_RADIUS(pitch));

	matX.m[1][1] = cosX;
	matX.m[1][2] = sinX;
	
	matX.m[2][1] = -sinX;
	matX.m[2][2] = cosX;
	this->multiply(matX);
}

void Matrix::rotateY(float yaw)
{
	Matrix matY;

	const float sinY = sinf(GLK_ANGLE_TO_RADIUS(yaw));
	const float cosY = cosf(GLK_ANGLE_TO_RADIUS(yaw));

	matY.m[0][0] = cosY;
	matY.m[0][2] = -sinY;

	matY.m[2][0] = sinY;
	matY.m[2][2] = cosY;
	this->multiply(matY);
}

void Matrix::rotateZ(float roll)
{
	Matrix matZ;
	const float sinZ = sinf(GLK_ANGLE_TO_RADIUS(roll));
	const float cosZ = cosf(GLK_ANGLE_TO_RADIUS(roll));

	matZ.m[0][0] = cosZ;
	matZ.m[0][1] = sinZ;

	matZ.m[1][0] = -sinZ;
	matZ.m[1][1] = cosZ;
	this->multiply(matZ);
}
//��ת
void    Matrix::rotate(float  angle, float x, float y, float z)
{
	float sinAngle, cosAngle;
	float mag = sqrtf(x * x + y * y + z * z);

	sinAngle = sinf(angle * PI / 180.0f);
	cosAngle = cosf(angle * PI / 180.0f);

	assert(mag > 0.0f);
	float xx, yy, zz, xy, yz, zx, xs, ys, zs;
	float oneMinusCos;
	Matrix tmp;

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
void Matrix::rotate(float angle, const GLVector3 &axis)
{
	float sinAngle, cosAngle;
	GLVector3 newAxis = axis.normalize();

	sinAngle = sinf(angle * PI / 180.0f);
	cosAngle = cosf(angle * PI / 180.0f);

	float xx, yy, zz, xy, yz, zx, xs, ys, zs;
	float oneMinusCos;
	Matrix tmp;

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
//��ͼͶӰ����
void    Matrix::lookAt(const GLVector3  &eyePosition, const GLVector3  &targetPosition, const GLVector3  &upVector)
{
	Matrix    tmp, *result = &tmp;
	GLVector3    N = (eyePosition - targetPosition).normalize();
	GLVector3    U =upVector.cross(N).normalize() ;
	assert(U.x*U.x+U.y*U.y+U.z*U.z>__EPS__);
	GLVector3    V = N.cross(U);
	memset(result, 0x0, sizeof(Matrix));
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
//����˷�
void    Matrix::multiply(const Matrix &srcA)
{
	Matrix     tmp;
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

	memcpy(this, &tmp, sizeof(Matrix));
}
//���ξ���˷�
void     Matrix::multiply(Matrix &srcA, Matrix &srcB)
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
		memcpy(m, p, sizeof(Matrix));
}
//���������
Matrix     Matrix::operator*(const Matrix   &srcA)const
{
	Matrix  tmp;
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
GLVector4  Matrix::operator*(const GLVector4  &vec)const
{
	float   x, y, z, w;
	x = vec.x*m[0][0] + vec.y*m[1][0] + vec.z*m[2][0] + vec.w*m[3][0];
	y = vec.x*m[0][1] + vec.y*m[1][1] + vec.z*m[2][1] + vec.w*m[3][1];
	z = vec.x*m[0][2] + vec.y*m[1][2] + vec.z*m[2][2] + vec.w*m[3][2];
	w = vec.x*m[0][3] + vec.y*m[1][3] + vec.z*m[2][3] + vec.w*m[3][3];
	return GLVector4(x, y, z, w);
}

Matrix&    Matrix::operator=(const Matrix  &src)
{
	if (this!=&src)
	memcpy(this,&src,sizeof(Matrix));
	return *this;
}
//����ͶӰ
void    Matrix::orthoProject(float  left, float right, float  bottom, float  top, float  nearZ, float  farZ)
{
	float       deltaX = right - left;
	float       deltaY = top - bottom;
	float       deltaZ = farZ - nearZ;
	assert(deltaX > 0.0f && deltaY > 0.0f && deltaZ > 0.0f);
	Matrix    ortho;

	ortho.m[0][0] = 2.0f / deltaX;
	ortho.m[3][0] = -(right + left) / deltaX;
	ortho.m[1][1] = 2.0f / deltaY;
	ortho.m[3][1] = -(top + bottom) / deltaY;
	ortho.m[2][2] =- 2.0f / deltaZ;
	ortho.m[3][2] =- (nearZ + farZ) / deltaZ;

	this->multiply(ortho);
}

void    Matrix::createOrtho(float left, float right, float bottom, float top, float nearZ, float farZ, Matrix &proj)
{
	float       deltaX = right - left;
	float       deltaY = top - bottom;
	float       deltaZ = farZ - nearZ;
	assert(deltaX > 0.0f && deltaY > 0.0f && deltaZ > 0.0f);

	proj.m[0][0] = 2.0f / deltaX;
	proj.m[3][0] = -(right + left) / deltaX;
	proj.m[1][1] = 2.0f / deltaY;
	proj.m[3][1] = -(top + bottom) / deltaY;
	proj.m[2][2] = -2.0f / deltaZ;
	proj.m[3][2] = -(nearZ + farZ) / deltaZ;
}
void    Matrix::frustum(float left, float right, float bottom, float top, float nearZ, float farZ)
{
	float       deltaX = right - left;
	float       deltaY = top - bottom;
	float       deltaZ = farZ - nearZ;
	Matrix    frust;

	assert(deltaX > 0.0f && deltaY > 0.0f && deltaZ > 0.0f && nearZ > 0.0f);

	frust.m[0][0] = 2.0f * nearZ / deltaX;
	frust.m[0][1] = frust.m[0][2] = frust.m[0][3] = 0.0f;

	frust.m[1][1] = 2.0f * nearZ / deltaY;
	frust.m[1][0] = frust.m[1][2] = frust.m[1][3] = 0.0f;

	frust.m[2][0] = (right + left) / deltaX;
	frust.m[2][1] = (top + bottom) / deltaY;
	frust.m[2][2] = -(nearZ + farZ) / deltaZ;
	frust.m[2][3] = -1.0f;

	frust.m[3][2] = -2.0f * nearZ * farZ / deltaZ;
	frust.m[3][0] = frust.m[3][1] = frust.m[3][3] = 0.0f;

	this->multiply(frust);
}

void    Matrix::perspective(float fovy, float aspect, float nearZ, float farZ)
{
	float frustumW, frustumH;

	frustumH = tanf(fovy / 360.0f * PI) * nearZ;
	frustumW = frustumH * aspect;

	this->frustum(-frustumW, frustumW, -frustumH, frustumH, nearZ, farZ);
}

void Matrix::createPerspective(float fov, float aspect, float nearZ, float farZ,Matrix &proj)
{
	float frustumH = tanf(fov / 360.0f * PI) * nearZ;
	float frustumW = frustumH * aspect;
	//
	float       deltaX = 2* frustumW;
	float       deltaY = 2* frustumH;
	float       deltaZ = farZ - nearZ;

	assert(deltaX > 0.0f && deltaY > 0.0f && deltaZ > 0.0f && nearZ > 0.0f);

	proj.m[0][0] = 2.0f * nearZ / deltaX;//==>deltaX*aspect/nearZ * 0.5==>tanf(fov / 360.0f * PI)*aspect
	proj.m[0][1] = proj.m[0][2] = proj.m[0][3] = 0.0f;

	proj.m[1][1] = 2.0f * nearZ / deltaY;
	proj.m[1][0] = proj.m[1][2] = proj.m[1][3] = 0.0f;

	proj.m[2][0] = 0;
	proj.m[2][1] = 0;
	proj.m[2][2] = -(nearZ + farZ) / deltaZ;
	proj.m[2][3] = -1.0f;

	proj.m[3][2] = -2.0f * nearZ * farZ / deltaZ;
	proj.m[3][0] = proj.m[3][1] = proj.m[3][3] = 0.0f;
} 

Matrix             Matrix::reverse()const
{
	Matrix     tmp;
	float             _det = this->det();
	assert(fabs(_det) > __EPS__);
	GLVector3    row1, row2, row3;
	int        _index[4];
	for (int i = 0; i < 4; ++i)
	{
		static_fix_func(_index, i);
		int     a = _index[0], b = _index[1], c = _index[2];
		//i,0
		row1 = GLVector3(m[a][1], m[a][2], m[a][3]);
		row2 = GLVector3(m[b][1], m[b][2], m[b][3]);
		row3 = GLVector3(m[c][1], m[c][2],m[c][3]);
		tmp.m[0][i] = __SIGNFloat(i + 0)*detVector3(&row1, &row2, &row3);
		//i,1
		row1 = GLVector3(m[a][0], m[a][2], m[a][3]);
		row2 = GLVector3(m[b][0], m[b][2], m[b][3]);
		row3 = GLVector3(m[c][0], m[c][2],m[c][3]);
		tmp.m[1][i] = __SIGNFloat(i + 1)*detVector3(&row1, &row2, &row3);
		//i,2
		row1 = GLVector3(m[a][0], m[a][1], m[a][3]);
		row2 = GLVector3(m[b][0], m[b][1], m[b][3]);
		row3 = GLVector3(m[c][0], m[c][1], m[c][3]);
		tmp.m[2][i] = __SIGNFloat(i + 2)*detVector3(&row1, &row2, &row3);
		//i,3
		row1 = GLVector3(m[a][0], m[a][1], m[a][2]);
		row2 = GLVector3(m[b][0], m[b][1], m[b][2]);
		row3 = GLVector3(m[c][0], m[c][1], m[c][2]);
		tmp.m[3][i] = __SIGNFloat(i + 3)*detVector3(&row1, &row2, &row3);
	}
	return   tmp;
}

void Matrix::reverse(Matrix &rm)const
{
	float             det = this->det();
	assert(fabs(det) > __EPS__);
	GLVector3    row1, row2, row3;
	int        index[4];
	for (int i = 0; i < 4; ++i)
	{
		static_fix_func(index, i);
		int     a = index[0], b = index[1], c = index[2];
		//i,0
		row1 = GLVector3(m[a][1], m[a][2], m[a][3]);
		row2 = GLVector3(m[b][1], m[b][2], m[b][3]);
		row3 = GLVector3(m[c][1], m[c][2], m[c][3]);
		rm.m[0][i] = __SIGNFloat(i + 0)*detVector3(&row1, &row2, &row3)/ det;
		//i,1
		row1 = GLVector3(m[a][0], m[a][2], m[a][3]);
		row2 = GLVector3(m[b][0], m[b][2], m[b][3]);
		row3 = GLVector3(m[c][0], m[c][2], m[c][3]);
		rm.m[1][i] = __SIGNFloat(i + 1)*detVector3(&row1, &row2, &row3)/ det;
		//i,2
		row1 = GLVector3(m[a][0], m[a][1], m[a][3]);
		row2 = GLVector3(m[b][0], m[b][1], m[b][3]);
		row3 = GLVector3(m[c][0], m[c][1], m[c][3]);
		rm.m[2][i] = __SIGNFloat(i + 2)*detVector3(&row1, &row2, &row3)/ det;
		//i,3
		row1 = GLVector3(m[a][0], m[a][1], m[a][2]);
		row2 = GLVector3(m[b][0], m[b][1], m[b][2]);
		row3 = GLVector3(m[c][0], m[c][1], m[c][2]);
		rm.m[3][i] = __SIGNFloat(i + 3)*detVector3(&row1, &row2, &row3)/ det;
	}
}
//����ʽ
float           Matrix::det()const
{
	GLVector3   row1, row2, row3;
	float  _det;
	//0,0
	row1 = GLVector3(m[1][1], m[1][2], m[1][3]);
	row2 = GLVector3(m[2][1], m[2][2], m[2][3]);
	row3 = GLVector3(m[3][1], m[3][2], m[3][3]);
	_det = m[0][0] * detVector3(&row1, &row2, &row3);
	//0,1
	row1 = GLVector3(m[1][0], m[1][2], m[1][3]);
	row2 = GLVector3(m[2][0], m[2][2], m[2][3]);
	row3 = GLVector3(m[3][0], m[3][2], m[3][3]);
	_det -= m[0][1] * detVector3(&row1, &row2, &row3);
	//0,2
	row1 = GLVector3(m[1][0], m[1][1], m[1][3]);
	row2 = GLVector3(m[2][0], m[2][1], m[2][3]);
	row3 = GLVector3(m[3][0], m[3][1], m[3][3]);
	_det += m[0][2] * detVector3(&row1, &row2, &row3);
	//0,3
	row1 = GLVector3(m[1][0], m[1][1], m[1][2]);
	row2 = GLVector3(m[2][0], m[2][1], m[2][2]);
	row3 = GLVector3(m[3][0], m[3][1], m[3][2]);
	_det -= m[0][3] * detVector3(&row1, &row2, &row3);
	return _det;
}

Matrix3     Matrix::normalMatrix()const
{
	Matrix3      tmp;
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
//�������
	tmp = tmp.reverse();
//ת��
#define     _SWAP_MAT3_(i,k) temp=tmp.m[i][k],  tmp.m[i][k]=tmp.m[k][i],tmp.m[k][i]=temp;
	_SWAP_MAT3_(0, 1)
    _SWAP_MAT3_(0, 2)
	_SWAP_MAT3_(1, 2)
#undef _SWAP_MAT3_
   return  tmp;
}

Matrix3    Matrix::trunk()const
{
	Matrix3   tmp;
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

void Matrix::trunk(Matrix3 &input)const
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
//ƫ�þ���
void     Matrix::offset()
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
//////////////////////////////��ά����//////////////////////////
Matrix3::Matrix3()
{
	m[0][0] = 1.0f; m[0][1] = 0.0f, m[0][2] = 0.0f;
	m[1][0] = 0.0f, m[1][1] = 1.0f, m[1][2] = 0.0f;
	m[2][0] = 0.0f, m[2][1] = 0.0f, m[2][2] = 1.0f;
}

Matrix3::Matrix3(const GLVector3 &row0, const GLVector3 &row1, const GLVector3 &row2)
{
	m[0][0] = row0.x, m[0][1] = row0.y, m[0][2] = row0.z;
	m[1][0] = row1.x, m[1][1] = row1.y, m[1][2] = row1.z;
	m[2][0] = row2.x, m[2][1] = row2.y, m[2][2] = row2.z;
}

Matrix3::Matrix3(const Matrix &mat4)
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

Matrix3     Matrix3::reverse()const
{
	Matrix3    tmp;
	float     _det = this->det();
	assert(fabs(_det) > __EPS__);

	tmp.m[0][0] = detFloat(m[1][1], m[1][2], m[2][1], m[2][2]) / _det;
	tmp.m[1][0] = -detFloat(m[1][0], m[1][2], m[2][0], m[2][2]) / _det;
	tmp.m[2][0] = detFloat(m[1][0], m[1][1], m[2][0], m[2][1]) / _det;

	tmp.m[0][1] = -detFloat(m[0][1], m[0][2], m[2][1], m[2][2]) / _det;
	tmp.m[1][1] = detFloat(m[0][0], m[0][2], m[2][0], m[2][2]) / _det;
	tmp.m[2][1] = -detFloat(m[0][0], m[0][1], m[2][0], m[2][1]) / _det;

	tmp.m[0][2] = detFloat(m[0][1], m[0][2], m[1][1], m[1][2]) / _det;
	tmp.m[1][2] = -detFloat(m[0][0], m[0][2], m[1][0], m[1][2]) / _det;
	tmp.m[2][2] = detFloat(m[0][0], m[0][1], m[1][0], m[1][1]) / _det;
	return  tmp;
}

void Matrix3::reverse(Matrix3 &tmp)const
{
	float     _det = this->det();
	assert(fabs(_det) > __EPS__);

	tmp.m[0][0] = detFloat(m[1][1], m[1][2], m[2][1], m[2][2]) / _det;
	tmp.m[1][0] = -detFloat(m[1][0], m[1][2], m[2][0], m[2][2]) / _det;
	tmp.m[2][0] = detFloat(m[1][0], m[1][1], m[2][0], m[2][1]) / _det;

	tmp.m[0][1] = -detFloat(m[0][1], m[0][2], m[2][1], m[2][2]) / _det;
	tmp.m[1][1] = detFloat(m[0][0], m[0][2], m[2][0], m[2][2]) / _det;
	tmp.m[2][1] = -detFloat(m[0][0], m[0][1], m[2][0], m[2][1]) / _det;

	tmp.m[0][2] = detFloat(m[0][1], m[0][2], m[1][1], m[1][2]) / _det;
	tmp.m[1][2] = -detFloat(m[0][0], m[0][2], m[1][0], m[1][2]) / _det;
	tmp.m[2][2] = detFloat(m[0][0], m[0][1], m[1][0], m[1][1]) / _det;
}

void     Matrix3::rotate(float angle, float x, float y, float z)
{
	float mag = sqrtf(x * x + y * y + z * z);

	const float sinAngle = sinf(angle * PI / 180.0f);
	const float cosAngle = cosf(angle * PI / 180.0f);

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

void    Matrix3::rotate(float angle, const GLVector3 &axis)
{
	GLVector3 newAxis = axis.normalize();
	float xx, yy, zz, xy, yz, zx, xs, ys, zs;
	float oneMinusCos;
	const float sinAngle = sinf(angle * PI / 180.0f);
	const float cosAngle = cosf(angle * PI / 180.0f);
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

void   Matrix3::scale(const GLVector3 &scaleFactor)
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

void Matrix3::scale(float x, float y, float z)
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

float     Matrix3::det()const
{
	float     _result;
	_result = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]);
	_result -= m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]);
	_result += m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
	return _result;
}

void Matrix3::tbn(const GLVector3 &normal, Matrix3 &tbn)
{
	GLVector3 X, Y;
	GLVector3::generateViewXY(normal, X, Y);
	//��ϳ�tbn����
	tbn.m[0][0] = X.x; tbn.m[0][1] = X.y; tbn.m[0][2] = X.z;
	tbn.m[1][0] = Y.x; tbn.m[1][1] = Y.y; tbn.m[1][2] = Y.z;
	tbn.m[2][0] = normal.x; tbn.m[2][1] = normal.y; tbn.m[2][2] = normal.z;
}

void Matrix3::reverseTBN(const GLVector3 &normal, Matrix3 &tbnr)
{
	GLVector3 X, Y;
	//float           mm[3][3];
	//Matrix3   &tmp = *(Matrix3*)mm;
	GLVector3::generateViewXY(normal, X, Y);
	//
	//tmp.m[0][0] = X.x; tmp.m[0][1] = X.y; tmp.m[0][2] = X.z;
	//tmp.m[1][0] = Y.x; tmp.m[1][1] = Y.y; tmp.m[1][2] = Y.z;
	//tmp.m[2][0] = normal.x; tmp.m[2][1] = normal.y; tmp.m[2][2] = normal.z;
	//��ϳ�TBN����������
	tbnr.m[0][0] = X.x; tbnr.m[0][1] = Y.x; tbnr.m[0][2] = normal.x;
	tbnr.m[1][0] = X.y; tbnr.m[1][1] = Y.y; tbnr.m[1][2] = normal.y;
	tbnr.m[2][0] = X.z; tbnr.m[2][1] = Y.z; tbnr.m[2][2] = normal.z;

	//tmp.reverse(tbnr);
}
//�ҳ���ά����
GLVector3     Matrix3::operator*(const GLVector3 &vec)const
{
	float x, y, z;
	x = m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z;
	y = m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z;
	z = m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z;

	return  GLVector3(x,y,z);
}

Matrix3 Matrix3::operator*(const Matrix3 &src)const
{
	Matrix3  tmp;
	for (int i = 0; i < 3; ++i)
	{
		tmp.m[i][0] = m[i][0] * src.m[0][0] + m[i][1] * src.m[1][0] + m[i][2] * src.m[2][0];
		tmp.m[i][1] = m[i][0] * src.m[0][1] + m[i][1] * src.m[1][1] + m[i][2] * src.m[2][1];
		tmp.m[i][2] = m[i][0] * src.m[0][2] + m[i][1] * src.m[1][2] + m[i][2] * src.m[2][2];
	}
	return tmp;
}

Matrix3&    Matrix3::operator=(Matrix3  &src)
{
	if (this != &src)
		memcpy(m,src.m,sizeof(Matrix3));
	return src;
}
////////////////////////////***�ռ����߷���ʽ***///////////////////////////
Ray::Ray(const GLVector3 &direction, const GLVector3 &point, bool needNormalize)
{
	init(direction,point,needNormalize);
}

void Ray::init(const GLVector3 &direction, const GLVector3 &point, bool needNormalize)
{
	if (needNormalize)
		_direction = direction.normalize();
	else
		_direction=direction;
	_point = point;
}
///////////////////////////////////ƽ�淽��////////////////////////////////
Plane::Plane(const GLVector3 &normal, const float distance, bool needNormalize):
	_normal(normal)
	,_distance(distance)
{
	if (needNormalize)
	{
		 float length = normal.length()*100;
		assert(length > __EPS__);
		_normal = GLVector3(normal.x*100 / length, normal.y *100/ length, normal.z*100 / length);
		_distance = distance / length;
	}
}

Plane::Plane(const GLVector3 &normal, const GLVector3 &vertex, bool needNormalize):
	_normal(normal)
{
	if (needNormalize)
		_normal/=normal.length();
	_distance = _normal.dot(vertex);
}

Plane::Plane()
{

}

const GLVector3 &Plane::getNormal()const
{
	return _normal;
}

float Plane::getDistance()const
{
	return _distance;
}

void Plane::init(const GLVector3 &normal, const float distance, bool needNormalize)
{
	if (needNormalize)
	{
		float scaleFactor = 100;
		float length = normal.length()*scaleFactor;
		if (length < __EPS__)
		{
			assert(length > __EPS__);
		}
		
		_normal = GLVector3(scaleFactor*normal.x / length, scaleFactor*normal.y / length, scaleFactor*normal.z / length);
		_distance = scaleFactor*distance / length;
	}
	else
	{
		_normal = normal;
		_distance = distance;
	}
}

void Plane::init(const GLVector3 &normal, const GLVector3 &vertex, bool needNormalize/* =true */)
{
	if (needNormalize)
	{
		float scaleFactor = 100;
		float length = normal.length()*scaleFactor;
		assert(length>=__EPS__);
		_normal = GLVector3(normal.x*scaleFactor/length,normal.y*scaleFactor/length,normal.z*scaleFactor/length);
		_distance = _normal.dot(vertex);
	}
	else
	{
		_normal = normal;
		_distance = normal.dot(vertex);
	}
}

float Plane::distance(const GLVector3 &p3d)const
{
	return _normal.dot(p3d) - _distance;
}
//////////////////////��Χ��/////////////////////
AABB::AABB(const GLVector3 *p3dCoord)
{
	_maxBox = *p3dCoord;
	_minBox = *p3dCoord;
	for (int i = 1; i < 8; ++i)
	{
		_maxBox = _maxBox.max(p3dCoord[i]);
		_minBox = _minBox.min(p3dCoord[i]);
	}
}

AABB::AABB(const GLVector4 *p4dCoord)
{
	_maxBox = p4dCoord->xyz();
	_minBox = p4dCoord->xyz();
	for (int i = 1; i < 8; ++i)
	{
		_maxBox = _maxBox.max(p4dCoord[i].xyz());
		_minBox = _minBox.min(p4dCoord[i].xyz());
	}
}

AABB::AABB(const GLVector3 &minBox, const GLVector3 &maxBox)
{
	_minBox = minBox;
	_maxBox = maxBox;
}

AABB::AABB(const GLVector4 &minBox, const GLVector4 &maxBox)
{
	_minBox = minBox.xyz();
	_maxBox = maxBox.xyz();
}

AABB::AABB()
{

}

void AABB::init(const GLVector3 *p3dCoord)
{
	_maxBox = *p3dCoord;
	_minBox = *p3dCoord;
	for (int i = 1; i < 8; ++i)
	{
		_maxBox = _maxBox.max(p3dCoord[i]);
		_minBox = _minBox.min(p3dCoord[i]);
	}
}

void AABB::init(const GLVector4 *p4dCoord)
{
	_maxBox = p4dCoord->xyz();
	_minBox = p4dCoord->xyz();
	for (int i = 1; i < 8; ++i)
	{
		_maxBox = _maxBox.max(p4dCoord[i].xyz());
		_minBox = _minBox.min(p4dCoord[i].xyz());
	}
}

__NS_GLK_END