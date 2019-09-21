/*
*@aim:OpenGL��������������װ
*version2.0,��������������ɫ�������֧��
//Version 3.0:�����˶������,ȫ��ͶӰ�����֧��
//Version 4.0:�����˶��¼��Ĺ���
//Version 5.0(2016-12-9 20:34:11):�����˸�˹�����Լ�Phillips����(������������Ҫ�ں�ƽ��ϵͳ��ʹ��)
&2016-4-30
*/
#ifndef  __GLCONTEXT_H__
#define __GLCONTEXT_H__
#include<engine/Geometry.h>
void         _static_OnDraw();
void         _static_OnUpdate(int);
int           main(int, char  **);
__NS_GLK_BEGIN
//OpenGL��������Ⱦ����,�Լ��̳���һЩ�ô��Ƚ�Ƶ���ĺ���
struct   GLContext
{
	friend         void         _static_OnDraw();
	friend         void         _static_OnUpdate(int);
	friend         int           main(int, char  **);
	//�û�˽������
	void      *userObject;
public:
	//��ʱ�ص�����
	void(*update)(GLContext *, float  deltaTime,float delay_time);//
	void(*draw)(GLContext *);

	void(*init)(GLContext *);//    ��ʼ������
	void(*finalize)(GLContext *);//����ر�ʱ�ص�

	int          _lastTickCount;//�ϴλ�ȡ�Ŀ���������
#ifndef _WIN32
	int          _baseTickCount;
#endif
	//��Ļ�ߴ�,��λ����
	Size                winSize;
	Size                _shadowSize;//��Ӱ�Ŀ��,���߱���ȳ�
	GLVector2     winPosition;
	//���ڻ�����������
	int         displayMode;
	//ȫ�ֱ�־,���庬����μ�GLState.h��tDrawFlagType
	unsigned          _globleFlag;
	//һ���ǹ���ȫ����ɫ������
	GLVector2       _near_far_plane;//��Զƽ��ľ���
	Matrix             _projMatrix;        //ȫ��ͶӰ����
private:
	GLContext(GLContext &);
	GLContext();
	static        GLContext     _singleGLContext;
public:
	static     GLContext         *getInstance();
	//ע��ӿ�
	void      registerUpdateFunc(void(*update)(GLContext*, float,float));
	void      registerDrawFunc(void(*draw)(GLContext *));
	void      registerInitFunc(void(*init)(GLContext *));
	void      registerShutDownFunc(void(*finalize)(GLContext *));
	//���ش��ڵĴ�С
	const Size      &getWinSize()const;
	//���ô��ڵĴ�С
	void      setWinSize(Size &);
	//������Ӱ���ڵĿ��
	void      setShadowSize(Size   &sSize) { _shadowSize = sSize; };
	Size      &getShadowSize() { return _shadowSize; };
	//���ô��ڻ�����������
	void      setDisplayMode(int flag);
	int        getDisplayMode();
	//void      setWindowTitle(char   *);
	void       setWinPosition(GLVector2 &);
	//���ý�ƽ��Ԫƽ��ľ���
	void           setNearFarPlane(GLVector2   &);
	GLVector2     &getNearFarPlane();
	//����ȫ��ͶӰ����
	void                  setProjMatrix(Matrix   &);
	Matrix            &getProjMatrix();
	void                setGlobleFlag(unsigned  flag) { _globleFlag = flag; };
	unsigned        getGlobleFlag()const { return _globleFlag; };
//private:
	//const char     *getWindowTitle()const;
};
//ע�Ὣ�趨�õĲ���ע������ڳ���,ע�����ﲻ�ܵ���OpenGL����
__NS_GLK_END
#endif