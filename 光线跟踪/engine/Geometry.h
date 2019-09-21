/*
  *几何向量,矩阵数据结构
  *2016-5-21
   */
   //Version 5.0 将所有的有关矩阵的操作纳入到矩阵类中,作为矩阵的成员函数实现
   //Version 6.0 将切线的计算引入到球体,立方体,地面网格的生成算法中
   //Version 7.0 引入了四元数的计算
   //Version 8.0 为支持欧拉角而添加的矩阵旋转函数
   //Version 9.0 全面支持关于四元数的运算
   //Version 10.0 删除了历史遗留的类,并将四元数的实现置于一个单独的文件中
   //Version 11.0  引入了平面方程/包围盒类,此两个类是支持模型/场景可视性判断的基础
#ifndef   __GEOMETRY_H__
#define  __GEOMETRY_H__
#define GLK_ANGLE_TO_RADIUS(x)    ((x)*3.14159265358f/180)
#define GLK_RADIUS_TO_ANGLE(x)    ((x)*180.0f/3.14159265358f)
	//2,3,4维向量
struct  float_2
{
	float x, y;
	float_2(float a, float b)
	{
		x = a, y = b;
	}
	float_2(float v) { x = y = v; };
	float_2() { x = 0, y = 0; };
	float_2     operator*(float)const;
	float_2     operator*(const float_2 &)const;
	float_2     operator+(const float_2 &)const;
	float_2     operator-(const float_2 &)const;
	float_2     operator/(float)const;
	float_2     operator/(const float_2 &)const;
	float_2&  operator=(const float_2 &src);
	float_2&  operator+=(const float_2 &src);
	float_2&  operator-=(const float_2 &src);
	float_2& operator*=(const float_2 &src);
	float_2& operator/=(const float_2 &src);
	float_2     normalize()const;
	const float     length()const;
	float               dot(const float_2 &other)const;
};
struct    Size
{
	float   width, height;
	Size() { width = 0, height = 0; };
	Size(float  a, float b)
	{
		width = a, height = b;
	}
};
class  mat3x3;
struct  float_4;
class mat4x4;
struct  float_3
{
	float    x, y, z;
	float_3(float a, float b, float c)
	{
		x = a, y = b, z = c;
	}
	float_3(const float xyz)
	{
		x = y = z = xyz;
	}
	float_3(float ax, float ay)
	{
		x = ax;
		y = ay;
		z = 0;
	}
	float_3() { x = 0, y = 0, z = 0; };
	float_3   operator*(const mat3x3 &)const;
	float_3   operator*(const mat4x4 &)const;
	float_3   operator*(const float_3 &)const;
	float_3   operator*(const float)const;
	float_3   operator-(const float_3 &)const;
	float_3   operator+(const float_3 &)const;
	float_3   operator/(const float)const;
	float_3   operator/(const float_3 &)const;
	float_3& operator +=(const float_3 &);
	float_3& operator +=(float f);
	float_3& operator -=(const float_3 &);
	float_3& operator-=(float f);
	float_3& operator *=(const float_3 &);
	float_3& operator *= (const mat4x4 &);
	float_3& operator *=(const mat3x3 &);
	float_3& operator*=(float f);
	float_3& operator /=(const float_3 &);
	float_3& operator/=(float f);
	float_3   normalize()const;
	float_3   cross(const float_3 &)const;
	float    dot(const float_3 &other)const;
	float    length()const;
	float    length2()const;

	//使用Z向量产生+X轴/+Y轴向量,向量产生的规则结果遵循摄像机原理
	static   void  generateViewXY(const float_3 &Z, float_3 &X, float_3 &Y);
};
class  mat4x4;
struct float_4
{
	float   x, y, z, w;
	float_4(float a, float b, float c, float d)
	{
		x = a, y = b;
		z = c, w = d;
	}
	float_4(const float xyzw)
	{
		x = y = z = w = xyzw;
	}
	float_4() { x = 0, y = 0, z = 0, w = 0; };
	float_4   operator*(const mat4x4 &)const;
	float_4   operator*(const float)const;
	float_4   operator*(const float_4 &)const;
	float_4   operator-(const float_4 &)const;
	float_4   operator+(const float_4 &)const;
	float_4   operator/(const float)const;
	float_4   operator/(const float_4 &)const;
	const float_4&   operator/=(const float);
	const float_4&   operator*=(const float);
	const float_4&   operator *= (const mat4x4 &);
	float_4   normalize()const;
	float              dot(const float_4 &other)const;
};
//空间射线方程
class  Ray
{
	//射线的方向,必须经过单位化
	float_3    _direction;
	//射线上的起点
	float_3    _point;
public:
	Ray() {};
	Ray(const float_3 &direction, const float_3 &point, bool needNormalize = true);
	void    init(const float_3 &direction, const float_3 &point, bool needNormalize = true);
	inline const float_3& getDirecction()const { return _direction; };
	inline const float_3& getStartPoint()const { return _point; };
};
//平面方程式,形式为 A*x+B*y+C*z-d=0
class Plane
{
public:
	//平面法向量(单位化之后的)
	float_3    _normal;
	//(0,0,0)点所在的平面(平面法向量为_normal)与该平面之间的有向距离
	float              _distance;
public:
	Plane();
	//A*x+B*y+C*z-d=0
	//needNormalize是否需要单位化,默认需要在传入参数之后重新单位化,false则不会重新单位化
	Plane(const float_3 &normal, const float distance, bool needNormalize = true);
	Plane(const float_3 &normal, const float_3 &vertex, bool needNormalize = true);
	void   init(const float_3 &normal, const float distance, bool needNormalize = true);
	//给定法线,平面上的任意一个顶点,求平面方程
	void   init(const float_3 &normal, const float_3 &vertex, bool needNormalize = true);
	//获取平面方程的法向量
	const float_3 &getNormal()const;
	//获取有向距离
	float   getDistance()const;
	//最重要的函数,计算给定的3d坐标点,离平面的有向距离
	float   distance(const float_3 &p3d)const;
};
//三阶矩阵,在引擎中被设计成轻量级的矩阵,通常此对象由四阶矩阵导出,有的地方使用此类作为单独的旋转矩阵使用
//与四阶矩阵不同,三阶矩阵的所有函数调用不会产生级联效果,
//也就是调用一个影响矩阵内容的操作不会对原来的造成影响,而是会产生一个全新的矩阵,
//原来的内容会丢失,所以如果需要使用级联矩阵操作,请使用矩阵乘法
class     mat3x3
{
private:
	float   m[3][3];
public:
	friend    class   mat4x4;
	friend    struct float_3;
	mat3x3();
	mat3x3(float angle,const float_3 &axis);
	mat3x3(const float_3 &row0, const float_3 &row1, const float_3 &row2);
	mat3x3(const mat4x4 &src);
	inline     const float    *pointer() const { return  (float*)m; };
	//求逆矩阵
	mat3x3         reverse()const;
	void                reverse(mat3x3 &)const;
	//旋转矩阵旋转矩阵
	void                rotate(float angle, const float_3 &axis);
	void                rotate(float angle, float x, float y, float z);
	//缩放矩阵
	void                scale(float x, float y, float z);
	void                scale(const float_3 &scaleFactor);
	//行列式
	float               det()const;
	//求TBN矩阵,normal必须是单位向量
	static void    tbn(const float_3 &normal, mat3x3 &);
	//求TBN矩阵的逆矩阵
	static void    reverseTBN(const float_3 &normal, mat3x3 &tbn);
	//右乘三维列向量
	float_3		operator*(const float_3 &)const;
	mat3x3      operator*(const mat3x3 &src)const;
	mat3x3&   operator=(const mat3x3 &);
};
//四维矩阵,全新的实现
class mat4x4
{
	friend class mat3x3;
private:
	float   m[4][4];
public:
	friend   struct    float_4;
	friend   class      Quaternion;
	friend   class      Frustum;
	friend   class      Camera;
	mat4x4();
	mat4x4(const float_4 &row0, const float_4 &row1, const float_4 &row2, const float_4 &row3);
	//创建视图矩阵使用
	mat4x4(const float_3 &row1, const float_3 &row2, const float_3 &row3, const float_3 &eyePosition);
	mat4x4(float angle,const float_3 &axis);
	//返回指向矩阵内容的指针,浮点型指针
	inline    const float     *pointer() const { return (float*)m; };
	//加载单位矩阵
	void      identity();
	//矩阵之间的快速复制
	void     copy(const mat4x4   &);
	//缩放
	void     scale(const float scaleX, const float scaleY, const float  scaleZ);
	//平移float 
	void    translate(const float deltaX, const float  deltaY, const float deltaZ);
	//平移deltaXYZ向量
	void    translate(const float_3 &deltaXYZ);
	//旋转
	void    rotate(float  angle, float x, float y, float z);
	void    rotate(float angle, const float_3 &axis);
	//绕X轴旋转
	void    rotateX(float pitch);
	//绕Y轴旋转
	void    rotateY(float yaw);
	//绕Z轴旋转
	void    rotateZ(float roll);
	//右乘视图矩阵
	void    lookAt(const float_3  &eyePosition, const float_3  &targetPosition, const float_3  &upVector);
	//右乘正交投影矩阵
	void    orthoProject(float  left, float right, float  bottom, float  top, float  nearZ, float  farZ);
	static void    createOrtho(float  left, float right, float  bottom, float  top, float  nearZ, float  farZ, mat4x4 &proj);
	//透视投影矩阵
	void    perspective(float fovy, float aspect, float nearZ, float farZ);
	static   void    createPerspective(float fov, float aspect, float nearZ, float farZ, mat4x4 &proj);
	//一般投影矩阵
	void    frustum(float left, float right, float bottom, float top, float nearZ, float farZ);
	//矩阵乘法,this=this*srcA
	void    multiply(const mat4x4   &srcA);
	//this=srcA*srcB
	void    multiply(mat4x4   &srcA, mat4x4   &rscB);
	//偏置矩阵,此矩阵是专门为阴影计算提供直接的支持,通常使用光源矩阵之后,需要乘以缩放,偏移矩阵,调用此函数
	//相当于两个矩阵乘法一起进行,并且没有矩阵数据的复制,因此更直接,且计算速度更快
	void   offset();
	//从现有的矩阵中推导出法线矩阵
	mat3x3     normalMatrix()const;
	//截断为3维矩阵
	mat3x3       trunk()const;
	void         trunk(mat3x3 &)const;
	//求逆矩阵
	mat4x4             reverse()const;
	void                  reverse(mat4x4 &rm)const;
	//行列式
	float                 det()const;
	//重载 乘法运算符
	mat4x4    operator*(const mat4x4   &)const;
	//矩阵与向量乘法
	float_4  operator*(const float_4  &)const;
	//矩阵之间的复制
	mat4x4&    operator=(const mat4x4  &);
};
//球面方程式实现
int  esGenSphere(int numSlices, float radius, float **vertices, float **normals, float  **tangents,
	float **texCoords, int **indices, int  *numberOfVertex);

//numberOfVertex:顶点数目,position 3分量,normals三分量,indices
int  esGenCube(float scale, float **vertices, float **normals, float **tangents,
	float **texCoords, int *numberOfVertex);

//平面网格生成算法,目前该算法的实现已经在Shape.cpp里面另有实现
int  esGenSquareGrid(int size, float scale, float **vertices, int **indices, int *numberOfVertex);

////////////////////////////////////////////////////////////行列式//////////////////////////////
//四个数字构成的二维矩阵的行列式
float     detFloat(float  x1, float y1, float x2, float y2);
//或者两个二维行向量
float     detVector2(float_2  *a, float_2  *b);
float     detVector3(float_3   *a, float_3   *b, float_3   *c);
float_3 rotate(const float_3 &u, float angle, const float_3 &v);

#endif
