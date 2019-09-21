/*
  *快速创建某物体形状
  *2016-5-30 13:17:45
  */
//Version:1.0 实现了最基本的物体数据
//Version 2.0:修改了关于立方体,球体生成数据的bug
//Version 3.0:增加了天空盒
//Version 4.0:添加了对切线的直接支持
//Version 5.0:添加了对螺旋线的支持
#ifndef   __SHAPE_H__
#define  __SHAPE_H__
#include<engine/Object.h>
#include<engine/Geometry.h>
__NS_GLK_BEGIN
//所有物体形状的超类,所有使用形状类的GL函数只能使用GL_TRIANGLE_STRIPE调用
//必须重写该类才能使用
class    Shape :public  Object
{
private:
	Shape(Shape &);
protected:
	int          _numberOfVertex;//顶点的数目
	int          _numberOfIndice;
//indice buffer id
	unsigned  int    _indiceVBO;
//vertex buffer id
	unsigned int    _vertexVBO;
//纹理
	unsigned int   _texCoordVBO;
//法向量
	unsigned int    _normalVBO;
//切线向量
	unsigned int   _tangentVBO;
	//包围盒
	AABB               _aabb;
protected:
	Shape();
public:
	virtual     int         numberOfVertex();//获取顶点的数目
	virtual     int         numberOfIndice();//顶点索引的数目
	int                         getVertexBufferId()const;
	int                         getTexBufferId()const;
	int                         getNormalBufferId()const;
//以下所有函数的输入为相关的属性在着色器中的位置
//绑定顶顶缓冲区
	virtual     void      bindVertexObject(int   _loc);
//绑定纹理缓冲区,注意,有的物体形状类需要重写这个函数,因为可能纹理坐标的分量大于2个
	virtual     void      bindTexCoordObject(int  _loc);

	virtual     void      bindNormalObject(int _loc);//绑定法线缓冲区
//绑定切线向量,目前暂时只实现球体的切线计算
	virtual     void      bindTangentObject(int  _loc);

	virtual     void      bindIndiceObject();//绑定索引缓冲区

	virtual     void      drawShape();
//结束所有的绑定
	void      finish();
	virtual     ~Shape();
	const AABB&    getAABB()const { return _aabb; };
};
//地面
class      Ground:public Shape
{
private:
private:
	Ground(Ground &);
	Ground();
	void        initWithGrid(int    _size,float scale);//输入网格的横向/纵向的数目
public:
	~Ground();
	static    Ground               *createWithGrid(int  _size,float scale);
//对于地面类型,因为所有的次发现都是一样的,所以没有必要单独生成数据
	virtual     void       bindTangentObject(int _loc);
};
//立方体
class   Cube :public  Shape
{
private:
	void         initWithScale(float  scale);
	Cube(Cube &);
	Cube();
public:
	~Cube();
//立方体的比例,实际的立方体是按照标准形状创建的,使用者需要按照自己的尺寸缩放
	static     Cube           *createWithScale(float scale);
	virtual  void    drawShape();
	virtual  void    bindIndiceObject();
	virtual  void    bindTexCoordObject(int _loc);
};
//球
class   Sphere :public  Shape
{
private:
	Sphere(Sphere &);
	Sphere();
	void     initWithSlice(int slices,float radius);
public:
	~Sphere();
	static    Sphere        *createWithSlice(int slices,float radius);//切片的数目,切片越多,球的外形越自然,当然运行速度就越慢
};
//天空盒
class  Skybox :public Shape
{
private:
	Skybox();
	Skybox(Skybox  &);
	void        initWithScale(float scale);
public:
	~Skybox();
	static  Skybox         *createWithScale(float scale);
	virtual     void    drawShape();
	virtual     void    bindIndiceObject();//bump function
	virtual     void    bindTexCoordObject(int _loc);
};
//箱子,其六个面都是由一个纹理组成
class  Chest :public Shape
{
private:
	explicit Chest();
	Chest(Chest  &);
	void      initWithScale(float scaleX,float scaleY,float scaleZ);
public:
	~Chest();
	static    Chest     *createWithScale(float scaleX, float scaleY, float Z);
	virtual   void       drawShape();
	virtual   void       bindIndiceObject();
};
//空间网格
class     Mesh :public  Shape
{
public:
	//创建网格的方式,
	enum  MeshType
	{
		MeshType_None=0,//无效的网格
		MeshType_XOY=1,//在XOY平面上创建网格
		MeshType_XOZ = 2,//在XOZ平面创建网格
		MeshType_YOZ = 3,//在YOZ平面创建网格
	};
private:
	GLVector3     _normal;//网格的法线
	GLVector3     _tangent;//网格的切线
private:
	Mesh();
	Mesh(Mesh &);
private:
//ScaleX,ScaleY分别为网格在X,Y方向的缩放的倍数
//texIntensity:为网格在纹理上的密度,
	void         initWithMesh(int   grid_size,float   scaleX,float   scaleY,float   texIntensity);
	void         initWithMesh(int   grid_size, float  scaleX, float  scaleY, float  texIntensity,MeshType meshType);
public:
	~Mesh();
	//创建XOY平面上的网格,网格的中心在世界坐标系原点
	static      Mesh       *createWithIntensity(int   grid_size,float  scaleX,float  scaleY,float  texIntensity);
	//以给定的方式创建网格
	static      Mesh       *createWithIntensity(int   grid_size, float  scaleX, float  scaleY, float  texIntensity,MeshType meshType);
	virtual     void      bindNormalObject(int _loc);//绑定法线缓冲区
	virtual     void      bindTangentObject(int _loc);
};
//四棱锥
class Pyramid : public Shape
{
	float _height;
	float _lengthOfEdge;
private:
	Pyramid(float );
	void              init(float l);
public:
	//给定边长,创建四棱锥对象
	static Pyramid    *create(float length_of_edge);
	virtual    void    bindNormalObject(int loc);
	virtual    void    bindVertexObject(int loc);
	virtual    void    bindTexCoordObject(int loc);
	virtual    void    bindTangentObject(int loc);
	virtual    void    drawShape();
};
__NS_GLK_END
#endif