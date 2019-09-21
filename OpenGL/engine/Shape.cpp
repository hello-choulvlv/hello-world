/*
  *外形轮廓类实现
*/
//Version:1.0 实现了最基本的物体数据
//Version 2.0:修改了关于立方体,球体生成数据的bug
//Version 3.0:增加了天空盒
//Version 4.0:添加了对切线的直接支持
//Version 5.0:改进了网格模型的生成
//Version 6.0:考虑到使用的方便性,该版本中重命名了类,啥去了类的开头GL
#include<engine/Shape.h>
#include<engine//Geometry.h>
#include<GL/glew.h>
#include<assert.h>
#include<stdlib.h>
#include<math.h>
__NS_GLK_BEGIN
/////////////////////////////////////////Shape Class//////////////////////////////
Shape::Shape()
{
	_numberOfVertex = 0;
	_numberOfIndice = 0;
	_indiceVBO = 0;
	_vertexVBO = 0;
	_texCoordVBO = 0;
	_normalVBO = 0;
	_tangentVBO = 0;
}
Shape::~Shape()
{
	glDeleteBuffers(1, &_indiceVBO);
	glDeleteBuffers(1, &_vertexVBO);
	glDeleteBuffers(1, &_texCoordVBO);
	glDeleteBuffers(1,&_normalVBO); 
	glDeleteBuffers(1,&_tangentVBO);
    _indiceVBO = 0;
	_vertexVBO = 0;
	_texCoordVBO = 0;
	_normalVBO = 0;
	_numberOfVertex = 0;
	_numberOfIndice = 0;
}
int    Shape::numberOfIndice()
{
	return _numberOfIndice;
}
int    Shape::numberOfVertex()
{
	return _numberOfVertex;
}
int      Shape::getVertexBufferId()const
{
	return _vertexVBO;
}

int     Shape::getTexBufferId()const
{
	return _texCoordVBO;
}

int    Shape::getNormalBufferId()const
{
	return _normalVBO;
}

void    Shape::bindIndiceObject()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indiceVBO);
}
void   Shape::bindVertexObject(int  _loc)
{
	glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
	glEnableVertexAttribArray(_loc);
	glVertexAttribPointer(_loc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}
void   Shape::bindNormalObject(int  _loc)
{
	glBindBuffer(GL_ARRAY_BUFFER, _normalVBO);
	glEnableVertexAttribArray(_loc);
	glVertexAttribPointer(_loc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}
void    Shape::bindTexCoordObject(int  _loc)
{
	glBindBuffer(GL_ARRAY_BUFFER, _texCoordVBO);
	glEnableVertexAttribArray(_loc);
	glVertexAttribPointer(_loc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
}
void   Shape::bindTangentObject(int loc)
{
	glBindBuffer(GL_ARRAY_BUFFER, _tangentVBO);
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}
void       Shape::finish()
{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void      Shape::drawShape()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indiceVBO);
	glDrawElements(GL_TRIANGLES, _numberOfIndice, GL_UNSIGNED_INT, NULL);
}
//////////////////////////////////////////GLGround  Class////////////////////////////////////////////////////////////////////////////
Ground::Ground()
{
}
//析构
Ground::~Ground()
{

}
//创建GLGround对象
Ground       *Ground::createWithGrid(int  _size,float scale)
{
	Ground    *_ground = new    Ground();
	_ground->initWithGrid(_size,scale);
	return   _ground;
}
//
//初始化
void          Ground::initWithGrid(int  _size,float scale)
{
		float     *vertex;
		int       *indice;
		_numberOfIndice = esGenSquareGrid(_size,scale, &vertex, &indice, &_numberOfVertex);
//生成缓冲区对象,绑定并填充数据
		glGenBuffers(1, &_vertexVBO);
		glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float)*_numberOfVertex, vertex,GL_STATIC_DRAW);
//tex coord
		float     *texCoord = vertex;
		const   float      _factor = _size - 1;
		for (int i = 0; i < _numberOfVertex; ++i)
		{
			int     _row = i/_size;
			int     _column = i%_size;
			texCoord[i << 1] = _column / _factor;
			texCoord[(i << 1) + 1] = _row / _factor;
		}
		glGenBuffers(1, &_texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, _texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float)*_numberOfVertex, texCoord, GL_STATIC_DRAW);
		texCoord = NULL;
//indice 
		glGenBuffers(1, &_indiceVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indiceVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*_numberOfIndice, indice,GL_STATIC_DRAW);
//normal
		const     int    _normal_size = _numberOfVertex * 3;
		float      *normal = vertex;
		for (int i = 0; i < _normal_size; i+=3)
		{
			normal[i] = 0.0f;
			normal[i + 1] = 0.0f;
			normal[i + 2] = 1.0f;
		}
		glGenBuffers(1, &_normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, _normalVBO);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float)*_numberOfVertex, normal, GL_STATIC_DRAW);
		normal = NULL;
//free memory
		delete indice;
		delete vertex;
		indice = NULL;
		vertex = NULL;
}
void     Ground::bindTangentObject(int _loc)
{
	glDisableVertexAttribArray(_loc);
	glVertexAttrib3fv(_loc,(float *)&GLVector3(1.0f,0.0f,0.0f));
}
///////////////////////////////////立方体////////////////////////////////////
Cube::Cube()
{
}
Cube::~Cube()
{
}
//
void        Cube::initWithScale(float  scale)
{
	float     *vertex, *normal, *texCoord,*tangent;
	_numberOfIndice = esGenCube(scale, &vertex, &normal, &texCoord,&tangent ,&_numberOfVertex);
//
	glGenBuffers(1, &_vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float)*_numberOfVertex,vertex,GL_STATIC_DRAW);
//
	glGenBuffers(1, &_normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _normalVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float)*_numberOfVertex, normal,GL_STATIC_DRAW);
//
	glGenBuffers(1, &_texCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float)*_numberOfVertex, texCoord, GL_STATIC_DRAW);
//
	glGenBuffers(1, &_tangentVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _tangentVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * _numberOfVertex, tangent, GL_STATIC_DRAW);
//
//free memory
	delete  vertex;
	delete  normal;
	delete  texCoord;
	delete  tangent;
	vertex = NULL;
	normal = NULL;
	texCoord = NULL;
}
//
Cube       *Cube::createWithScale(float scale)
{
	Cube     *_cube = new   Cube();
	_cube->initWithScale(scale);
	return  _cube;
}
void    Cube::bindIndiceObject()
{

}
void   Cube::bindTexCoordObject(int _loc)
{
	glBindBuffer(GL_ARRAY_BUFFER, _texCoordVBO);
	glEnableVertexAttribArray(_loc);
	glVertexAttribPointer(_loc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}
void   Cube::drawShape()
{
	glDrawArrays(GL_TRIANGLES,0,_numberOfVertex);
}
//
/////////////////////////////////////////////GLSphere///////////////////////////////////////
Sphere::Sphere()
{
}
//
Sphere::~Sphere()
{
}
//
Sphere     *Sphere::createWithSlice(int  slices, float radius)
{
	Sphere   *_sphere = new   Sphere();
	_sphere->initWithSlice(slices, radius);
	return _sphere;
}
//
void      Sphere::initWithSlice(int   slices,float  radius)
{
	float   *vertex, *texCoord, *normal,*tangent;
	int     *indice;

	_aabb._minBox = GLVector3(-radius,-radius,-radius);
	_aabb._maxBox = GLVector3(radius, radius, radius);
	_numberOfIndice = esGenSphere(slices, radius, &vertex, &normal, &tangent, &texCoord, &indice, &_numberOfVertex);

	glGenBuffers(1, &_vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float)*_numberOfVertex, vertex, GL_STATIC_DRAW);
//
	glGenBuffers(1, &_texCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float)*_numberOfVertex, texCoord, GL_STATIC_DRAW);
//
	glGenBuffers(1, &_normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _normalVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float)*_numberOfVertex, normal, GL_STATIC_DRAW);
//
	glGenBuffers(1, &_indiceVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indiceVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*_numberOfIndice, indice, GL_STATIC_DRAW);
//
	glGenBuffers(1, &_tangentVBO);
	glBindBuffer(GL_ARRAY_BUFFER,_tangentVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float)*_numberOfVertex, tangent,GL_STATIC_DRAW);
//free memory
	delete    vertex;
	delete    texCoord;
	delete   normal;
	delete   indice;
	delete   tangent;
	vertex = NULL;
	texCoord = NULL;
	normal = NULL;
	indice = NULL;
	tangent = NULL;
}
//天空盒实现
Skybox::Skybox()
{

}
Skybox::~Skybox()
{

}
void     Skybox::initWithScale(float scale)
{
//注意三角形的方向,从立方体的内部观察是正面的,
	float    vVertex[] = {
//前
		-1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
//后
		-1.0f, -1.0f, 1.0f,  -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,
//左
		-1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
//右
       1.0f, -1.0f, -1.0f,  1.0f, -1.0f, 1.0f,  1.0f, 1.0f, 1.0f,
       1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
//上
		-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, -1.0f,
//下
		-1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f
	};
//纹理坐标,纹理坐标与顶点坐标相同,注意发现的方向是从立方体的内部观察的
	float     vNormal[] = {
//前
		0.0f,0.0f,1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
//后
       0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,   0.0f, 0.0f, -1.0f,
	   0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,   0.0f, 0.0f, -1.0f,
//左
      1.0f,0.0f,0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
	  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
//右
      -1.0f,0.0f,0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
	  -1.0f, 0.0f, 0.0f,-1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
//上
      0.0f,-1.0f,0.0f,  0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
	  0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
//下
     0.0f,1.0f,0.0f, 0.0f, 1.0f, 0.0f,0.0f, 1.0f, 0.0f,
	 0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	 };
//纹理坐标,与顶点坐标相同
	float   vTexCoord[] = {
//前
		-1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
//后
		-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
//左
		-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
//右
		1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
//上
		-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f,1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,
//下
		-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f
	};
//切线,一把来说,不会使用这个数据
	float    tvTangent[] = {
//前
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
//后
        -1.0f,0.0f,0.0f,  -1.0f,0.0f,0.0f,   -1.0f,0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
//左
       0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
       0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
//右
       0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//上
      1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
//下
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	};
	_numberOfVertex = sizeof(vVertex) / sizeof(float);
	for (int i = 0; i < _numberOfVertex; ++i)
		vVertex[i] *= scale;
//生成OpenGL缓存对象
	glGenBuffers(1,&_vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vVertex), vVertex, GL_STATIC_DRAW);
//
	glGenBuffers(1,&_texCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER,_texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(vTexCoord),vTexCoord,GL_STATIC_DRAW);
//
	glGenBuffers(1, &_normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _normalVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vNormal), vNormal, GL_STATIC_DRAW);

	glGenBuffers(1,&_tangentVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _tangentVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vTexCoord), vTexCoord, GL_STATIC_DRAW);
	_numberOfVertex /= 3;
 }
 Skybox     *Skybox::createWithScale(float scale)
 {
	 Skybox	*_Skybox = new   Skybox();
	 _Skybox->initWithScale(scale);
	 return  _Skybox;
 }
 void    Skybox::drawShape()
 {
	 glDrawArrays(GL_TRIANGLES, 0, _numberOfVertex);
 }

 void    Skybox::bindTexCoordObject(int loc)
 {
	 glBindBuffer(GL_ARRAY_BUFFER, _texCoordVBO);
	 glEnableVertexAttribArray(loc);
	 glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
 }

 void    Skybox::bindIndiceObject()
 {

 }
 ///////////////////////////箱子//////////////////////////
Chest::Chest()
{

}
Chest::~Chest()
{

}
//
void     Chest::initWithScale(float scaleX,float scaleY,float scaleZ)
{
	//从立方体的外面观察,所有的三角形都是正方向的
	float cubeVerts[] =
	{
//前
		-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
//右
		1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
//后
		1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
//左
		-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
//上
		-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 
		1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
//下
		-1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 
		1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
	};

	float cubeNormals[] =
	{
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,

		-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
	};
//切线
	float    tangent[] = {
//前
		1.0f,0.0f,0.0f,   1.0f,0.0f,0.0f,    1.0f,0.0f,0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
//右
       0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
	   0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
//后
-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
//左
	   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//上
     1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
//下
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	};
	float cubeTex[] =
	{
		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	};
	_numberOfVertex = sizeof(cubeVerts)/(sizeof(float)*3);
	int index = 0;
	//缩放不同的面
	for (int i = 0; i < _numberOfVertex; ++i)
	{
		cubeVerts[index] *= scaleX;
		cubeVerts[index + 1] *= scaleY;
		cubeVerts[index + 2] *= scaleZ;
		index += 3;
	}

	glGenBuffers(1, &_vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);

	glGenBuffers(1, &_texCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeTex), cubeTex, GL_STATIC_DRAW);
	
	glGenBuffers(1, &_normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _normalVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);

	glGenBuffers(1, &_tangentVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _tangentVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tangent), tangent, GL_STATIC_DRAW);
}
//
Chest     *Chest::createWithScale(float scaleX,float scaleY,float scaleZ)
{
	Chest    *glChest = new  Chest();
	glChest->initWithScale(scaleX,scaleY,scaleZ);
	return glChest;
}
void    Chest::drawShape()
{
	glDrawArrays(GL_TRIANGLES, 0, _numberOfVertex);
}
void   Chest::bindIndiceObject()
{

}
////////////////////////网格//////////////////////////////////
Mesh::Mesh()
{
}
Mesh::~Mesh()
{
}

void          Mesh::initWithMesh(int grid_size, float scaleX, float scaleY, float texIntensity)
{
	initWithMesh(grid_size, scaleX, scaleY, texIntensity, MeshType::MeshType_XOY);
}

void          Mesh::initWithMesh(int    size,float scaleX, float scaleY, float texIntensity,MeshType meshType)
{
	int i, j;
	int numIndices = (size - 1) * (size - 1) * 2 * 3;
// Allocate memory for buffers
	int numVertices = size * size;
	_numberOfVertex = numVertices;
	_numberOfIndice = numIndices;
	float   stepSize = ((float)(size - 1)) / 2.0f;
	float   *vertex = new float[3 * numVertices];

	if (meshType == MeshType_XOY)
	{
		_aabb._minBox = GLVector3(-scaleX,-scaleY,0);
		_aabb._maxBox = GLVector3(scaleX,scaleY,0);
	}

	int  index = 0;
	for (i = 0; i < size; ++i) // row
	{
		const    float     locX = scaleY*(i / stepSize - 1.0f);
		for (j = 0; j < size; ++j) // column
		{
			if (meshType == MeshType::MeshType_XOY)
			{
				vertex[index] = scaleX*(j / stepSize - 1.0f);
				vertex[index + 1] = locX;
				vertex[index + 2] = 0.0f;
			}
			else if (meshType == MeshType::MeshType_XOZ)
			{
				vertex[index] = (j/stepSize -1.0f )*scaleX ;
				vertex[index+1] = 0.0f;
				vertex[index + 2] =-locX ;
			}
			else if (meshType == MeshType::MeshType_YOZ)
			{
				vertex[index] = 0.0f;
				vertex[index + 1] = locX;
				vertex[index + 2] = (1.0f - j/stepSize)*scaleX;
			}
			index += 3;
		}
	}
	//计算切线鱼与法线
	if (meshType == MeshType::MeshType_XOY)
	{
		_normal = GLVector3(0,0,1.0f);
		_tangent = GLVector3(1.0f,0.0f,0.0f);
	}
	else if (meshType ==MeshType::MeshType_XOZ)
	{
		_normal = GLVector3(0.0f,1.0f,0.0f);
		_tangent = GLVector3(1.0f,0,0);
	}
	else if (meshType==MeshType::MeshType_YOZ)
	{
		_normal = GLVector3(1,0,0);
		_tangent = GLVector3(0,0,-1);
	}

// Generate the indices
	int   *_indice = new int[numIndices];
	for (i = 0; i < size - 1; ++i)
	{
		for (j = 0; j < size - 1; ++j)
		{
// two triangles per quad
			int    _index = 6 * (j + i * (size - 1));
			_indice[_index] = j + i* size;
			_indice[_index + 1] = j + i* size + 1;
			_indice[_index + 2] = j + (i + 1) * size + 1;

			_indice[_index + 3] = j + (i + 1) * size + 1;
			_indice[_index + 4] = j + (i + 1) * size;
			_indice[_index + 5] = j + i* size;
		}
	}
//Gen Buffers
	glGenBuffers(1, &_vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, 3*sizeof(float)*_numberOfVertex, vertex,GL_STATIC_DRAW);
//
	glGenBuffers(1, &_indiceVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indiceVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*_numberOfIndice, _indice, GL_STATIC_DRAW);
//tex coord
	float     *texCoord = vertex;
	const   float      _factor = size - 1;
	for (int i = 0; i < _numberOfVertex; ++i)
	{
		int     _row = i / size;
		int     _column = i%size;
		texCoord[i << 1] = texIntensity*_column / _factor;
		texCoord[(i << 1) + 1] = texIntensity*_row / _factor;
	}
	glGenBuffers(1, &_texCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float)*_numberOfVertex, texCoord, GL_STATIC_DRAW);

	delete  vertex;
	delete  _indice;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
//
void       Mesh::bindNormalObject(int _loc)
{
	glDisableVertexAttribArray(_loc);
	glVertexAttrib3fv(_loc, &_normal.x);
}

void Mesh::bindTangentObject(int _loc)
{
	glDisableVertexAttribArray(_loc);
	glVertexAttrib3fv(_loc,&_tangent.x);
}

Mesh         *Mesh::createWithIntensity(int grid_size, float scaleX, float scaleY, float texIntensity)
{
	Mesh		*_mesh = new   Mesh();
	_mesh->initWithMesh(grid_size, scaleX, scaleY, texIntensity);
	return  _mesh;
}

Mesh  *Mesh::createWithIntensity(int grid_size, float scaleX, float scaleY, float texIntensity, MeshType meshType)
{
	Mesh *mesh = new Mesh();
	mesh->initWithMesh(grid_size, scaleX, scaleY, texIntensity, meshType);
	return mesh;
}
//////////////////////////Pyramid////////////////////////
Pyramid::Pyramid(float l):
	_lengthOfEdge(l)
{
	_height = sqrtf(6)/3.0f * l;
}
Pyramid *Pyramid::create(float length_of_edge)
{
	Pyramid *p = new Pyramid(length_of_edge);
	p->init(length_of_edge);
	return p;
}

void    Pyramid::init(float lengOfEdge)
{
	float   sqrtf_3 = sqrtf(3.0f);
	float   sqrtf_6 = sqrtf(6.0f);
	float   one_d_2sqrt3 = 1.0f / (2.0f*sqrtf_3);

	const GLVector3    v_1(-lengOfEdge * 0.5f,0, lengOfEdge*one_d_2sqrt3);
	const GLVector3    v_2(lengOfEdge*0.5f,0, lengOfEdge*one_d_2sqrt3);
	const GLVector3    v_3(0,0,-lengOfEdge / sqrtf_3);
	const GLVector3    v_4(0, lengOfEdge*sqrtf_6/3.0f,0);
	_aabb._minBox = GLVector3(v_1.x, v_1.y,v_3.z);
	_aabb._maxBox = GLVector3(v_2.x,v_4.y,v_2.z);
	//四个平面的法线
	const GLVector3   n_1 = (v_3 - v_1).cross(v_2 - v_1).normalize();
	const GLVector3   n_2 = (v_2 - v_1).cross(v_4 - v_1).normalize();
	const GLVector3   n_3 = (v_3-v_2).cross(v_4 -v_2).normalize();
	const GLVector3   n_4 = (v_1 - v_3).cross(v_4 - v_3).normalize();
	//12个顶点/法线/纹理坐标
	float    vertex_data[] = {
		v_2.x,v_2.y,v_2.z,n_1.x,n_1.y,n_1.z,0,1,//1
		v_1.x,v_1.y,v_1.z,n_1.x,n_1.y,n_1.z,0,0,//2
		v_3.x,v_3.y,v_3.z,n_1.x,n_1.y,n_1.z,1,1,//3

		v_4.x,v_4.y,v_4.z,n_3.x,n_3.y,n_3.z,0,1,//4
		v_2.x,v_2.y,v_2.z,n_3.x,n_3.y,n_3.z,0,0,//5
		v_3.x,v_3.y,v_3.z,n_3.x,n_3.y,n_3.z,1,0,//6

		v_4.x,v_4.y,v_4.z,n_4.x,n_4.y,n_4.z,0,1,//7
		v_3.x,v_3.y,v_3.z,n_4.x,n_4.y,n_4.z,1,0,//8
		v_1.x,v_1.y,v_1.z,n_4.x,n_4.y,n_4.z,1,1,//9

		v_4.x,v_4.y,v_4.z,n_2.x,n_2.y,n_2.z,1,1,//10
		v_1.x,v_1.y,v_1.z,n_2.x,n_2.y,n_2.z,0,0,//11
		v_2.x,v_2.y,v_2.z,n_2.x,n_2.y,n_2.z,1,0,//12
	};

	_numberOfVertex = 12;
	glGenBuffers(1, &_vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER,_vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, _numberOfVertex * sizeof(float) * 8, vertex_data,GL_STATIC_DRAW);
}

void Pyramid::bindVertexObject(int loc)
{
	glBindBuffer(GL_ARRAY_BUFFER,_vertexVBO);
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc,3,GL_FLOAT,GL_FALSE,sizeof(float)*8,nullptr);
}

void  Pyramid::bindNormalObject(int loc)
{
	glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc,3,GL_FLOAT,GL_FALSE,sizeof(float)*8,(void*)(sizeof(float)*3));
}

void Pyramid::bindTexCoordObject(int loc)
{
	glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8,(void*)(sizeof(float)*6));
}

void Pyramid::bindTangentObject(int loc)
{
	glDisableVertexAttribArray(loc);
	glVertexAttrib3f(loc,1.0f, 0,0);
}

void Pyramid::drawShape()
{
	glDrawArrays(GL_TRIANGLES, 0, _numberOfVertex);
}
__NS_GLK_END