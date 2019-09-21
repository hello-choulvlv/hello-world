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
#include<engine/GLState.h>
__NS_GLK_BEGIN
	//2,3,4维向量
	struct  GLVector2
	{
		float x, y;
		GLVector2(float a, float b)
		{
			x = a, y = b;
		}
		GLVector2(float v) { x = y = v; };
		GLVector2() { x = 0, y = 0; };
		GLVector2     operator*(float)const;
		GLVector2     operator*(const GLVector2 &)const;
		GLVector2     operator+(const GLVector2 &)const;
		GLVector2     operator-(const GLVector2 &)const;
		GLVector2     operator/(float)const;
		GLVector2     operator/(const GLVector2 &)const;
		GLVector2&  operator=(const GLVector2 &src);
		GLVector2&  operator+=(const GLVector2 &src);
		GLVector2&  operator-=(const GLVector2 &src);
		GLVector2& operator*=(const GLVector2 &src);
		GLVector2& operator/=(const GLVector2 &src);
		GLVector2     normalize()const;
		const float     length()const;
		float               dot(const GLVector2 &other)const;
	};
	typedef GLVector2 Vec2;
	struct    Size
	{
		float   width, height;
		Size() { width = 0, height = 0; };
		Size(float  a, float b)
		{
			width = a, height = b;
		}
	};
	class  Matrix3;
	class Matrix;
	typedef Matrix3  Mat3;
	struct  GLVector4;
	typedef GLVector4 Vec4;
	struct  GLVector3
	{
		float    x, y, z;
		GLVector3(float a, float b, float c)
		{
			x = a, y = b, z = c;
		}
		GLVector3(const float xyz)
		{
			x = y = z = xyz;
		}
		GLVector3(float ax,float ay)
		{
			x = ax;
			y = ay;
			z = 0;
		}
		GLVector3() { x = 0, y = 0, z = 0; };
		GLVector4  xyzw0()const;
		GLVector4  xyzw1()const;
		GLVector3   operator*(const Matrix3 &)const;
		GLVector4   operator*(const Matrix &)const;
		GLVector3   operator*(const GLVector3 &)const;
		GLVector3   operator*(const float)const;
		GLVector3   operator-(const GLVector3 &)const;
		GLVector3   operator+(const GLVector3 &)const;
		GLVector3   operator/(const float)const;
		GLVector3   operator/(const GLVector3 &)const;
		GLVector3& operator +=(const GLVector3 &);
		GLVector3& operator +=(float f);
		GLVector3& operator -=(const GLVector3 &);
		GLVector3& operator-=(float f);
		GLVector3& operator *=(const GLVector3 &);
		GLVector3& operator*=(float f);
		GLVector3& operator /=(const GLVector3 &);
		GLVector3& operator/=(float f);
		GLVector3   normalize()const;
		GLVector3   cross(const GLVector3 &)const;
		GLVector3   min(const GLVector3 &)const;
		GLVector3   max(const GLVector3 &)const;
		float              dot(const GLVector3 &other)const;
		const float    length()const;
		//使用Z向量产生+X轴/+Y轴向量,向量产生的规则结果遵循摄像机原理
		static   void  generateViewXY(const GLVector3 &Z,GLVector3 &X,GLVector3 &Y);
	};
	typedef GLVector3 Vec3;
	class  Matrix;
	typedef Matrix Mat4;
	struct GLVector4
	{
		float   x, y, z, w;
		GLVector4(float a, float b, float c, float d)
		{
			x = a, y = b;
			z = c, w = d;
		}
		GLVector4(const float xyzw)
		{
			x = y = z = w=xyzw;
		}
		GLVector4(const GLVector3&a, float d)
		{
			x = a.x, y = a.y, z = a.z, w = d;
		}
		GLVector4() { x = 0, y = 0, z = 0, w = 0; };
		GLVector3    xyz() const{ return GLVector3(x, y, z); };
		GLVector4   operator*(const Matrix &)const;
		GLVector4   operator*(const float )const;
		GLVector4   operator*(const GLVector4 &)const;
		GLVector4   operator-(const GLVector4 &)const;
		GLVector4   operator+(const GLVector4 &)const;
		GLVector4   operator/(const float )const;
		GLVector4   operator/(const GLVector4 &)const;
		float               length()const;
		float               length3()const;
		const GLVector4&   operator/=(const float);
		const GLVector4&   operator*=(const float);
		GLVector4   min(const GLVector4 &)const;
		GLVector4   max(const GLVector4 &)const;
		GLVector4   normalize()const;
		float              dot(const GLVector4 &other)const;
	};
	//空间射线方程
	class  Ray
	{
		//射线的方向,必须经过单位化
		GLVector3    _direction;
		//射线上的起点
		GLVector3    _point;
	public:
		Ray() {};
		Ray(const GLVector3 &direction,const GLVector3 &point,bool needNormalize=true);
		void    init(const GLVector3 &direction,const GLVector3 &point, bool needNormalize=true);
		inline const GLVector3& getDirecction()const { return _direction; };
		inline const GLVector3& getStartPoint()const { return _point; };
	};
	//平面方程式,形式为 A*x+B*y+C*z-d=0
	class Plane
	{
	public:
		//平面法向量(单位化之后的)
		GLVector3    _normal;
		//(0,0,0)点所在的平面(平面法向量为_normal)与该平面之间的有向距离
		float              _distance;
	public:
		Plane();
		//A*x+B*y+C*z-d=0
		//needNormalize是否需要单位化,默认需要在传入参数之后重新单位化,false则不会重新单位化
		Plane(const GLVector3 &normal,const float distance,bool needNormalize=true);
		Plane(const GLVector3 &normal,const GLVector3 &vertex,bool needNormalize=true);
		void   init(const GLVector3 &normal,const float distance,bool needNormalize=true);
		//给定法线,平面上的任意一个顶点,求平面方程
		void   init(const GLVector3 &normal,const GLVector3 &vertex, bool needNormalize=true);
		//获取平面方程的法向量
		const GLVector3 &getNormal()const;
		//获取有向距离
		float   getDistance()const;
		//最重要的函数,计算给定的3d坐标点,离平面的有向距离
		float   distance(const GLVector3 &p3d)const;
	};
	//空间包围盒
	class     AABB
	{
	public:
		//包围盒的最大,最小点
		GLVector3    _minBox;
		GLVector3    _maxBox;
	public:
		//由给定的8个3d坐标点计算包围盒
		AABB(const GLVector3 *);
		//由给定的8个3d齐次坐标点计算包围盒
		AABB(const GLVector4 *);
		//
		AABB();
		AABB(const GLVector3 &minBox,const GLVector3 &maxBox);
		//
		AABB(const GLVector4 &minBox,const GLVector4 &maxBox);
		void   init(const GLVector3 *);
		void   init(const GLVector4 *);
	};
	//三阶矩阵,在引擎中被设计成轻量级的矩阵,通常此对象由四阶矩阵导出,有的地方使用此类作为单独的旋转矩阵使用
	//与四阶矩阵不同,三阶矩阵的所有函数调用不会产生级联效果,
	//也就是调用一个影响矩阵内容的操作不会对原来的造成影响,而是会产生一个全新的矩阵,
	//原来的内容会丢失,所以如果需要使用级联矩阵操作,请使用矩阵乘法
	class     Matrix3
	{
	private:
		float   m[3][3];
	public:
		friend    class   Matrix;
		friend    struct GLVector3;
		Matrix3();
		Matrix3(const GLVector3 &row0,const GLVector3 &row1,const GLVector3 &row2);
		Matrix3(const Matrix &src);
		inline     const float    *pointer() const { return  (float*)m; };
		//求逆矩阵
		Matrix3         reverse()const;
		void                reverse(Matrix3 &)const;
		//旋转矩阵旋转矩阵
		void                rotate(float angle,const GLVector3 &axis);
		void                rotate(float angle,float x,float y,float z);
		//缩放矩阵
		void                scale(float x,float y,float z);
		void                scale(const GLVector3 &scaleFactor);
		//行列式
		float               det()const;
		//求TBN矩阵,normal必须是单位向量
		static void      tbn(const GLVector3 &normal,Matrix3 &);
		//求TBN矩阵的逆矩阵
		static void      reverseTBN(const GLVector3 &normal,Matrix3 &tbn);
		//右乘三维列向量
		GLVector3   operator*(const GLVector3 &)const;
		Matrix3        operator*(const Matrix3 &src)const;
		Matrix3&     operator=(Matrix3 &);
	};

	class Quaternion;
	class Frustum;
	class Camera;
	//四维矩阵,全新的实现
	class Matrix
	{
		friend class Matrix3;
		friend struct GLVector3;
	private:
		float   m[4][4];
	public:
		friend   struct    GLVector4;
		friend   class      Quaternion;
		friend   class      Frustum;
		friend   class      Camera;
		Matrix();
		Matrix(const GLVector4 &row0,const GLVector4 &row1,const GLVector4 &row2,const GLVector4 &row3);
		//创建视图矩阵使用
		Matrix(const GLVector3 &row1,const GLVector3 &row2,const GLVector3 &row3,const GLVector3 &eyePosition);
		//返回指向矩阵内容的指针,浮点型指针
		inline    const float     *pointer() const { return (float*)m; };
		//加载单位矩阵
		void      identity();
		//矩阵之间的快速复制
		void     copy(const Matrix   &);
		//缩放
		void     scale(const float scaleX, const float scaleY, const float  scaleZ);
		//平移float 
		void    translate(const float deltaX, const float  deltaY,const float deltaZ);
		//平移deltaXYZ向量
		void    translate(const GLVector3 &deltaXYZ);
		//旋转
		void    rotate(float  angle, float x, float y, float z);
		void    rotate(float angle,const GLVector3 &axis);
		//绕X轴旋转
		void    rotateX(float pitch);
		//绕Y轴旋转
		void    rotateY(float yaw);
		//绕Z轴旋转
		void    rotateZ(float roll);
		//右乘视图矩阵
		void    lookAt(const GLVector3  &eyePosition, const GLVector3  &targetPosition, const GLVector3  &upVector);
		//右乘正交投影矩阵
		void    orthoProject(float  left, float right, float  bottom, float  top, float  nearZ, float  farZ);
		static void    createOrtho(float  left, float right, float  bottom, float  top, float  nearZ, float  farZ,Matrix &proj);
		//透视投影矩阵
		void    perspective(float fovy, float aspect, float nearZ, float farZ);
		static   void    createPerspective(float fov,float aspect,float nearZ,float farZ,Matrix &proj);
		//一般投影矩阵
		void    frustum(float left, float right, float bottom, float top, float nearZ, float farZ);
		//矩阵乘法,this=this*srcA
		void    multiply(const Matrix   &srcA);
		//this=srcA*srcB
		void    multiply(Matrix   &srcA, Matrix   &rscB);
		//偏置矩阵,此矩阵是专门为阴影计算提供直接的支持,通常使用光源矩阵之后,需要乘以缩放,偏移矩阵,调用此函数
		//相当于两个矩阵乘法一起进行,并且没有矩阵数据的复制,因此更直接,且计算速度更快
		void   offset();
		//从现有的矩阵中推导出法线矩阵
		Matrix3     normalMatrix()const;
		//截断为3维矩阵
		Matrix3       trunk()const;
		void             trunk(Matrix3 &)const;
		//求逆矩阵
		Matrix             reverse()const;
		void                  reverse(Matrix &rm)const;
		//行列式
		float                 det()const;
		//重载 乘法运算符
		Matrix    operator*(const Matrix   &)const;
		//矩阵与向量乘法
		GLVector4  operator*(const GLVector4  &)const;
		//矩阵之间的复制
		Matrix&    operator=(const Matrix  &);
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
	float     detVector2(GLVector2  *a, GLVector2  *b);
	float     detVector3(GLVector3   *a, GLVector3   *b, GLVector3   *c);
	//////////////////////////////////////////////////////////////////////////////
	//typedef GLVector3 Vec3;
	//typedef GLVector4 Vec4;
	//typedef GLVector2 Vec2;
	//typedef Matrix       Mat4;
	//typedef Matrix3     Mat3;
__NS_GLK_END
#endif
