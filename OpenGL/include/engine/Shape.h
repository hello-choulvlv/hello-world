/*
  *���ٴ���ĳ������״
  *2016-5-30 13:17:45
  */
//Version:1.0 ʵ�������������������
//Version 2.0:�޸��˹���������,�����������ݵ�bug
//Version 3.0:��������պ�
//Version 4.0:����˶����ߵ�ֱ��֧��
//Version 5.0:����˶������ߵ�֧��
#ifndef   __SHAPE_H__
#define  __SHAPE_H__
#include<engine/Object.h>
#include<engine/Geometry.h>
__NS_GLK_BEGIN
//����������״�ĳ���,����ʹ����״���GL����ֻ��ʹ��GL_TRIANGLE_STRIPE����
//������д�������ʹ��
class    Shape :public  Object
{
private:
	Shape(Shape &);
protected:
	int          _numberOfVertex;//�������Ŀ
	int          _numberOfIndice;
//indice buffer id
	unsigned  int    _indiceVBO;
//vertex buffer id
	unsigned int    _vertexVBO;
//����
	unsigned int   _texCoordVBO;
//������
	unsigned int    _normalVBO;
//��������
	unsigned int   _tangentVBO;
	//��Χ��
	AABB               _aabb;
protected:
	Shape();
public:
	virtual     int         numberOfVertex();//��ȡ�������Ŀ
	virtual     int         numberOfIndice();//������������Ŀ
	int                         getVertexBufferId()const;
	int                         getTexBufferId()const;
	int                         getNormalBufferId()const;
//�������к���������Ϊ��ص���������ɫ���е�λ��
//�󶨶���������
	virtual     void      bindVertexObject(int   _loc);
//����������,ע��,�е�������״����Ҫ��д�������,��Ϊ������������ķ�������2��
	virtual     void      bindTexCoordObject(int  _loc);

	virtual     void      bindNormalObject(int _loc);//�󶨷��߻�����
//����������,Ŀǰ��ʱֻʵ����������߼���
	virtual     void      bindTangentObject(int  _loc);

	virtual     void      bindIndiceObject();//������������

	virtual     void      drawShape();
//�������еİ�
	void      finish();
	virtual     ~Shape();
	const AABB&    getAABB()const { return _aabb; };
};
//����
class      Ground:public Shape
{
private:
private:
	Ground(Ground &);
	Ground();
	void        initWithGrid(int    _size,float scale);//��������ĺ���/�������Ŀ
public:
	~Ground();
	static    Ground               *createWithGrid(int  _size,float scale);
//���ڵ�������,��Ϊ���еĴη��ֶ���һ����,����û�б�Ҫ������������
	virtual     void       bindTangentObject(int _loc);
};
//������
class   Cube :public  Shape
{
private:
	void         initWithScale(float  scale);
	Cube(Cube &);
	Cube();
public:
	~Cube();
//������ı���,ʵ�ʵ��������ǰ��ձ�׼��״������,ʹ������Ҫ�����Լ��ĳߴ�����
	static     Cube           *createWithScale(float scale);
	virtual  void    drawShape();
	virtual  void    bindIndiceObject();
	virtual  void    bindTexCoordObject(int _loc);
};
//��
class   Sphere :public  Shape
{
private:
	Sphere(Sphere &);
	Sphere();
	void     initWithSlice(int slices,float radius);
public:
	~Sphere();
	static    Sphere        *createWithSlice(int slices,float radius);//��Ƭ����Ŀ,��ƬԽ��,�������Խ��Ȼ,��Ȼ�����ٶȾ�Խ��
};
//��պ�
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
//����,�������涼����һ���������
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
//�ռ�����
class     Mesh :public  Shape
{
public:
	//��������ķ�ʽ,
	enum  MeshType
	{
		MeshType_None=0,//��Ч������
		MeshType_XOY=1,//��XOYƽ���ϴ�������
		MeshType_XOZ = 2,//��XOZƽ�洴������
		MeshType_YOZ = 3,//��YOZƽ�洴������
	};
private:
	GLVector3     _normal;//����ķ���
	GLVector3     _tangent;//���������
private:
	Mesh();
	Mesh(Mesh &);
private:
//ScaleX,ScaleY�ֱ�Ϊ������X,Y��������ŵı���
//texIntensity:Ϊ�����������ϵ��ܶ�,
	void         initWithMesh(int   grid_size,float   scaleX,float   scaleY,float   texIntensity);
	void         initWithMesh(int   grid_size, float  scaleX, float  scaleY, float  texIntensity,MeshType meshType);
public:
	~Mesh();
	//����XOYƽ���ϵ�����,�������������������ϵԭ��
	static      Mesh       *createWithIntensity(int   grid_size,float  scaleX,float  scaleY,float  texIntensity);
	//�Ը����ķ�ʽ��������
	static      Mesh       *createWithIntensity(int   grid_size, float  scaleX, float  scaleY, float  texIntensity,MeshType meshType);
	virtual     void      bindNormalObject(int _loc);//�󶨷��߻�����
	virtual     void      bindTangentObject(int _loc);
};
//����׶
class Pyramid : public Shape
{
	float _height;
	float _lengthOfEdge;
private:
	Pyramid(float );
	void              init(float l);
public:
	//�����߳�,��������׶����
	static Pyramid    *create(float length_of_edge);
	virtual    void    bindNormalObject(int loc);
	virtual    void    bindVertexObject(int loc);
	virtual    void    bindTexCoordObject(int loc);
	virtual    void    bindTangentObject(int loc);
	virtual    void    drawShape();
};
__NS_GLK_END
#endif