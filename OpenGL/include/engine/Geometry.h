/*
  *��������,�������ݽṹ
  *2016-5-21
   */
//Version 5.0 �����е��йؾ���Ĳ������뵽��������,��Ϊ����ĳ�Ա����ʵ��
//Version 6.0 �����ߵļ������뵽����,������,��������������㷨��
//Version 7.0 ��������Ԫ���ļ���
//Version 8.0 Ϊ֧��ŷ���Ƕ���ӵľ�����ת����
//Version 9.0 ȫ��֧�ֹ�����Ԫ��������
//Version 10.0 ɾ������ʷ��������,������Ԫ����ʵ������һ���������ļ���
//Version 11.0  ������ƽ�淽��/��Χ����,����������֧��ģ��/�����������жϵĻ���
#ifndef   __GEOMETRY_H__
#define  __GEOMETRY_H__
#include<engine/GLState.h>
__NS_GLK_BEGIN
	//2,3,4ά����
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
		//ʹ��Z��������+X��/+Y������,���������Ĺ�������ѭ�����ԭ��
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
	//�ռ����߷���
	class  Ray
	{
		//���ߵķ���,���뾭����λ��
		GLVector3    _direction;
		//�����ϵ����
		GLVector3    _point;
	public:
		Ray() {};
		Ray(const GLVector3 &direction,const GLVector3 &point,bool needNormalize=true);
		void    init(const GLVector3 &direction,const GLVector3 &point, bool needNormalize=true);
		inline const GLVector3& getDirecction()const { return _direction; };
		inline const GLVector3& getStartPoint()const { return _point; };
	};
	//ƽ�淽��ʽ,��ʽΪ A*x+B*y+C*z-d=0
	class Plane
	{
	public:
		//ƽ�淨����(��λ��֮���)
		GLVector3    _normal;
		//(0,0,0)�����ڵ�ƽ��(ƽ�淨����Ϊ_normal)���ƽ��֮����������
		float              _distance;
	public:
		Plane();
		//A*x+B*y+C*z-d=0
		//needNormalize�Ƿ���Ҫ��λ��,Ĭ����Ҫ�ڴ������֮�����µ�λ��,false�򲻻����µ�λ��
		Plane(const GLVector3 &normal,const float distance,bool needNormalize=true);
		Plane(const GLVector3 &normal,const GLVector3 &vertex,bool needNormalize=true);
		void   init(const GLVector3 &normal,const float distance,bool needNormalize=true);
		//��������,ƽ���ϵ�����һ������,��ƽ�淽��
		void   init(const GLVector3 &normal,const GLVector3 &vertex, bool needNormalize=true);
		//��ȡƽ�淽�̵ķ�����
		const GLVector3 &getNormal()const;
		//��ȡ�������
		float   getDistance()const;
		//����Ҫ�ĺ���,���������3d�����,��ƽ����������
		float   distance(const GLVector3 &p3d)const;
	};
	//�ռ��Χ��
	class     AABB
	{
	public:
		//��Χ�е����,��С��
		GLVector3    _minBox;
		GLVector3    _maxBox;
	public:
		//�ɸ�����8��3d���������Χ��
		AABB(const GLVector3 *);
		//�ɸ�����8��3d������������Χ��
		AABB(const GLVector4 *);
		//
		AABB();
		AABB(const GLVector3 &minBox,const GLVector3 &maxBox);
		//
		AABB(const GLVector4 &minBox,const GLVector4 &maxBox);
		void   init(const GLVector3 *);
		void   init(const GLVector4 *);
	};
	//���׾���,�������б���Ƴ��������ľ���,ͨ���˶������Ľ׾��󵼳�,�еĵط�ʹ�ô�����Ϊ��������ת����ʹ��
	//���Ľ׾���ͬ,���׾�������к������ò����������Ч��,
	//Ҳ���ǵ���һ��Ӱ��������ݵĲ��������ԭ�������Ӱ��,���ǻ����һ��ȫ�µľ���,
	//ԭ�������ݻᶪʧ,���������Ҫʹ�ü����������,��ʹ�þ���˷�
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
		//�������
		Matrix3         reverse()const;
		void                reverse(Matrix3 &)const;
		//��ת������ת����
		void                rotate(float angle,const GLVector3 &axis);
		void                rotate(float angle,float x,float y,float z);
		//���ž���
		void                scale(float x,float y,float z);
		void                scale(const GLVector3 &scaleFactor);
		//����ʽ
		float               det()const;
		//��TBN����,normal�����ǵ�λ����
		static void      tbn(const GLVector3 &normal,Matrix3 &);
		//��TBN����������
		static void      reverseTBN(const GLVector3 &normal,Matrix3 &tbn);
		//�ҳ���ά������
		GLVector3   operator*(const GLVector3 &)const;
		Matrix3        operator*(const Matrix3 &src)const;
		Matrix3&     operator=(Matrix3 &);
	};

	class Quaternion;
	class Frustum;
	class Camera;
	//��ά����,ȫ�µ�ʵ��
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
		//������ͼ����ʹ��
		Matrix(const GLVector3 &row1,const GLVector3 &row2,const GLVector3 &row3,const GLVector3 &eyePosition);
		//����ָ��������ݵ�ָ��,������ָ��
		inline    const float     *pointer() const { return (float*)m; };
		//���ص�λ����
		void      identity();
		//����֮��Ŀ��ٸ���
		void     copy(const Matrix   &);
		//����
		void     scale(const float scaleX, const float scaleY, const float  scaleZ);
		//ƽ��float 
		void    translate(const float deltaX, const float  deltaY,const float deltaZ);
		//ƽ��deltaXYZ����
		void    translate(const GLVector3 &deltaXYZ);
		//��ת
		void    rotate(float  angle, float x, float y, float z);
		void    rotate(float angle,const GLVector3 &axis);
		//��X����ת
		void    rotateX(float pitch);
		//��Y����ת
		void    rotateY(float yaw);
		//��Z����ת
		void    rotateZ(float roll);
		//�ҳ���ͼ����
		void    lookAt(const GLVector3  &eyePosition, const GLVector3  &targetPosition, const GLVector3  &upVector);
		//�ҳ�����ͶӰ����
		void    orthoProject(float  left, float right, float  bottom, float  top, float  nearZ, float  farZ);
		static void    createOrtho(float  left, float right, float  bottom, float  top, float  nearZ, float  farZ,Matrix &proj);
		//͸��ͶӰ����
		void    perspective(float fovy, float aspect, float nearZ, float farZ);
		static   void    createPerspective(float fov,float aspect,float nearZ,float farZ,Matrix &proj);
		//һ��ͶӰ����
		void    frustum(float left, float right, float bottom, float top, float nearZ, float farZ);
		//����˷�,this=this*srcA
		void    multiply(const Matrix   &srcA);
		//this=srcA*srcB
		void    multiply(Matrix   &srcA, Matrix   &rscB);
		//ƫ�þ���,�˾�����ר��Ϊ��Ӱ�����ṩֱ�ӵ�֧��,ͨ��ʹ�ù�Դ����֮��,��Ҫ��������,ƫ�ƾ���,���ô˺���
		//�൱����������˷�һ�����,����û�о������ݵĸ���,��˸�ֱ��,�Ҽ����ٶȸ���
		void   offset();
		//�����еľ������Ƶ������߾���
		Matrix3     normalMatrix()const;
		//�ض�Ϊ3ά����
		Matrix3       trunk()const;
		void             trunk(Matrix3 &)const;
		//�������
		Matrix             reverse()const;
		void                  reverse(Matrix &rm)const;
		//����ʽ
		float                 det()const;
		//���� �˷������
		Matrix    operator*(const Matrix   &)const;
		//�����������˷�
		GLVector4  operator*(const GLVector4  &)const;
		//����֮��ĸ���
		Matrix&    operator=(const Matrix  &);
	};
	//���淽��ʽʵ��
	int  esGenSphere(int numSlices, float radius, float **vertices, float **normals, float  **tangents,
		float **texCoords, int **indices, int  *numberOfVertex);

	//numberOfVertex:������Ŀ,position 3����,normals������,indices
	int  esGenCube(float scale, float **vertices, float **normals, float **tangents,
		float **texCoords, int *numberOfVertex);

	//ƽ�����������㷨,Ŀǰ���㷨��ʵ���Ѿ���Shape.cpp��������ʵ��
	int  esGenSquareGrid(int size, float scale, float **vertices, int **indices, int *numberOfVertex);

	////////////////////////////////////////////////////////////����ʽ//////////////////////////////
	//�ĸ����ֹ��ɵĶ�ά���������ʽ
	float     detFloat(float  x1, float y1, float x2, float y2);
	//����������ά������
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
