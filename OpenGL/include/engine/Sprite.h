/*
  *����,2DͼƬ���ƵĹ���
  *2016-6-16 20:50:42
  *С����
  */
#ifndef  __SPRITE_H__
#define __SPRITE_H__
#include<engine/Object.h>
#include<engine/GLTexture.h>
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
__NS_GLK_BEGIN
//�������Ǽ���ʹ�����Ѿ���ȫ������OpenGL�ľ���任,���Ծ���ı�����ȫ����ÿһ����������Ӧ�ľ������������ֵ�
//������ʹ������ʹ�ø����,����ȴ�Ƚ������ƿ�,ʹ������Ҫ���ע��
class  Sprite :public Object
{
//����
private:
	GLTexture	    *_glTexture;
	GLProgram    *_glProgram;
	unsigned         _vertexVBO;//���㻺��������
	unsigned         _vertexArrayBufferId;
	//uniform variable index
	int                   _baseMapLoc;//�����uniform����
	int                   _mvMatrixLoc;//ģ����ͼ���������
	int                   _renderColorLoc;//�������ɫ����
//	GLVector3      _position;//λ��
//	GLVector3      _scale;
	GLVector4      _renderColor;//�������ɫ
	Matrix        _mvMatrix;//�任����
private:
	Sprite(Sprite &);
	Sprite();
	void         initWithFile(const  char  *file_name);
public:
	~Sprite();
	static   Sprite            *createWithFile(const char  *file_name);
//������Ⱦ��ɫ
	void         setRenderColor(GLVector4  *);
//���þ���ı任����
	void        setAffineMatrix(Matrix   *);
//��ȡ�任����
	Matrix        *getAffineMattix();
//���ƾ���,ע��,���ʹ���˲�����������ĺ���,��ʹ�ö����Դ��ľ���,����ʹ���������
	//note:�˺��������ڻ�������ͼ����м䲽���е���,�����п��ܳ��ֲ�ȷ������Ϊ
	void        render();
	void        render(Matrix   *,unsigned otherTextureId=0);
//��ȡ����Ĵ�С
	Size        &getContentSize();
//����Mipmap
	void       setMipmap();
	GLProgram		*getGLProgram(){return  _glProgram;};
};
__NS_GLK_END
#endif