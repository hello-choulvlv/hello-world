/*
  *��������,�������ݽṹ
  *ֻ���Vulkan
  *ע����ΪVulkan������ϵ��OpenGL��������,���ͶӰ����Ĺ�����̾���������
  *2018��9��21��
   */

#ifndef   __GEOMETRY_H__
#define  __GEOMETRY_H__
	//2,3,4ά����
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
	struct    extent_2d
	{
		float   width, height;
		extent_2d() { width = 0, height = 0; };
		extent_2d(float  a, float b)
		{
			width = a, height = b;
		}
	};
	class  mat3x3;
	struct  float_4;
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
		float_3(float ax,float ay)
		{
			x = ax;
			y = ay;
			z = 0;
		}
		float_3() { x = 0, y = 0, z = 0; };
		float_4  xyzw0()const;
		float_4  xyzw1()const;
		float_3   operator*(const mat3x3 &)const;
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
		float_3& operator*=(float f);
		float_3& operator /=(const float_3 &);
		float_3& operator/=(float f);
		float_3   normalize()const;
		float_3   cross(const float_3 &)const;
		float_3   min(const float_3 &)const;
		float_3   max(const float_3 &)const;
		float              dot(const float_3 &other)const;
		const float    length()const;
		//ʹ��Z��������+X��/+Y������,���������Ĺ�������ѭ�����ԭ��
		static   void  generateViewXY(const float_3 &Z,float_3 &X,float_3 &Y);
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
			x = y = z = w=xyzw;
		}
		float_4(const float_3&a, float d)
		{
			x = a.x, y = a.y, z = a.z, w = d;
		}
		float_4() { x = 0, y = 0, z = 0, w = 0; };
		float_3    xyz() const{ return float_3(x, y, z); };
		float_4   operator*(const mat4x4 &)const;
		float_4   operator*(const float )const;
		float_4   operator*(const float_4 &)const;
		float_4   operator-(const float_4 &)const;
		float_4   operator+(const float_4 &)const;
		float_4   operator/(const float )const;
		float_4   operator/(const float_4 &)const;
		const float_4&   operator/=(const float);
		const float_4&   operator*=(const float);
		float_4   min(const float_4 &)const;
		float_4   max(const float_4 &)const;
		float_4   normalize()const;
		float              dot(const float_4 &other)const;
	};
	//���׾���,�������б���Ƴ��������ľ���,ͨ���˶������Ľ׾��󵼳�,�еĵط�ʹ�ô�����Ϊ��������ת����ʹ��
	//���Ľ׾���ͬ,���׾�������к������ò����������Ч��,
	//Ҳ���ǵ���һ��Ӱ��������ݵĲ��������ԭ�������Ӱ��,���ǻ����һ��ȫ�µľ���,
	//ԭ�������ݻᶪʧ,���������Ҫʹ�ü����������,��ʹ�þ���˷�
	class     mat3x3
	{
	private:
		float   m[3][3];
	public:
		friend    class   mat4x4;
		friend    struct float_3;
		mat3x3();
		mat3x3(const float_3 &row0,const float_3 &row1,const float_3 &row2);
		mat3x3(const mat4x4 &src);
		inline     const float    *pointer() const { return  (float*)m; };
		//�������
		mat3x3         reverse()const;
		void                reverse(mat3x3 &)const;
		//��ת������ת����
		void                rotate(float angle,const float_3 &axis);
		void                rotate(float angle,float x,float y,float z);
		//���ž���
		void                scale(float x,float y,float z);
		void                scale(const float_3 &scaleFactor);
		//����ʽ
		float               det()const;
		//��TBN����,normal�����ǵ�λ����
		static void      tbn(const float_3 &normal,mat3x3 &);
		//��TBN����������
		static void      reverseTBN(const float_3 &normal,mat3x3 &tbn);
		//�ҳ���ά������
		float_3   operator*(const float_3 &)const;
		mat3x3        operator*(const mat3x3 &src)const;
		mat3x3&     operator=(mat3x3 &);
	};
	//��ά����,ȫ�µ�ʵ��
	class mat4x4
	{
		friend class mat3x3;
	private:
		float   m[4][4];
	public:
		friend   struct    float_4;
		mat4x4();
		mat4x4(const float_4 &row0,const float_4 &row1,const float_4 &row2,const float_4 &row3);
		//������ͼ����ʹ��
		mat4x4(const float_3 &row1,const float_3 &row2,const float_3 &row3,const float_3 &eyePosition);
		//����ָ��������ݵ�ָ��,������ָ��
		inline    const float     *pointer() const { return (float*)m; };
		//���ص�λ����
		void      identity();
		//����֮��Ŀ��ٸ���
		void     copy(const mat4x4   &);
		//����
		void     scale(const float scaleX, const float scaleY, const float  scaleZ);
		//ƽ��float 
		void    translate(const float deltaX, const float  deltaY,const float deltaZ);
		//ƽ��deltaXYZ����
		void    translate(const float_3 &deltaXYZ);
		//��ת
		void    rotate(float  angle, float x, float y, float z);
		void    rotate(float angle,const float_3 &axis);
		//��X����ת
		void    rotateX(float pitch);
		//��Y����ת
		void    rotateY(float yaw);
		//��Z����ת
		void    rotateZ(float roll);
		//�ҳ���ͼ����
		void    lookAt(const float_3  &eyePosition, const float_3  &targetPosition, const float_3  &upVector);
		//�ҳ�����ͶӰ����
		void    orthoProject(float  left, float right, float  bottom, float  top, float  nearZ, float  farZ);
		static void    createOrtho(float  left, float right, float  bottom, float  top, float  nearZ, float  farZ,mat4x4 &proj);
		//͸��ͶӰ����
		void    perspective(float fovy, float aspect, float nearZ, float farZ);
		static   void    createPerspective(float fov,float aspect,float nearZ,float farZ,mat4x4 &proj);
		//һ��ͶӰ����
		void    frustum(float left, float right, float bottom, float top, float nearZ, float farZ);
		//����˷�,this=this*srcA
		void    multiply(const mat4x4   &srcA);
		//this=srcA*srcB
		void    multiply(mat4x4   &srcA, mat4x4   &rscB);
		//ƫ�þ���,�˾�����ר��Ϊ��Ӱ�����ṩֱ�ӵ�֧��,ͨ��ʹ�ù�Դ����֮��,��Ҫ��������,ƫ�ƾ���,���ô˺���
		//�൱����������˷�һ�����,����û�о������ݵĸ���,��˸�ֱ��,�Ҽ����ٶȸ���
		void   offset();
		//�����еľ������Ƶ������߾���
		mat3x3     normalMatrix()const;
		//�ض�Ϊ3ά����
		mat3x3       trunk()const;
		void             trunk(mat3x3 &)const;
		//�������
		mat4x4             reverse()const;
		void                  reverse(mat4x4 &rm)const;
		//����ʽ
		float                 det()const;
		//���� �˷������
		mat4x4    operator*(const mat4x4   &)const;
		//�����������˷�
		float_4  operator*(const float_4  &)const;
		//����֮��ĸ���
		mat4x4&    operator=(const mat4x4  &);
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
	float     detVector2(float_2  *a, float_2  *b);
	float     detVector3(float_3   *a, float_3   *b, float_3   *c);

#endif