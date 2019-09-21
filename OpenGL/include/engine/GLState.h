/*
  *@aim:OpenGLӦ�ó���ʹ�õĺ�
  &2016-4-30
*/
#ifndef   __GL_STATE_H__
#define  __GL_STATE_H__

/////////////////////////////////�꿪��////////////////////////////////////////////////////////////////
//OpenGL�İ汾����,�Ƿ���OpenGL�汾,������OpenGLES�汾
#if defined _WIN32 || defined _LINUX || defined _APPLE
#define      __OPENGL_VERSION__
#endif
//�Ƿ���������ɫ��,Ĭ���ǲ�������,��OpenGLES�汾��,�����ֹ�����
#ifdef __OPENGL_VERSION__
#define      __GEOMETRY_SHADER__
#endif
//�Ƿ�������,��ɫ������,������
//#define     __ENABLE_PROGRAM_CACHE__    
//����������
#define     __ENABLE_TEXTURE_CACHE__
///////////////////////////////////////////////////////////////////////////////////////////////////
//ö�ٳ���,������ɫ�������Ա���λ�õĲ�һ��,��ʱ���趨ȷ�е�ֵ
#define      GLAttribPosition            0   //λ������
#define      GLAttribTexCoord          1   //��������
#define      GLAttribNormal             2  //����

//��õ���ɫ����������,��Spriteʹ��
#define      OpenGLSpriteProgram                     "GLK_OpenGLSpriteProgram"
//һ�������ɫ��
#define      OpenGLNormalLightProgram          "GLK_OpenGLLightProgram"
//���Դ��ɫ��
#define      OpenGLPointLightProgram             "GLK_OpenGLPointLightProgram"
//�ṹ����ƫ��
#define   __offsetof(s,m)           (char *)(&((s *)NULL)->m)
//�Ƿ����� tsb_image��,��������������,����򽫻�ʹ�øÿ�������е�ͼƬ
#define _USE_STB_IMAGE_    1
#ifndef  NULL
#define  NULL  0
#endif
#ifndef MATH_PI
#define MATH_PI 3.14159265358
#endif
//������Ƕ�֮���ת��
#define    GLK_RADIUS_TO_ANGLE(radius)  ((radius)*180/MATH_PI)
//�Ƕ��뻡��֮���ת��
#define    GLK_ANGLE_TO_RADIUS(angle) ((angle)*MATH_PI/180)
//���ż���
#define    __SIGNFloat(sign)   (-(  ((sign)&0x1)<<1)+1)
//����ϵ��
#define    GLK_GRAVITY_CONSTANT		9.810f
//����������ռ�����
#define	 __NS_GLK_BEGIN                 namespace glk {
#define    __NS_GLK_END                     }
#define    __US_GLK__						    using namespace glk;
#endif