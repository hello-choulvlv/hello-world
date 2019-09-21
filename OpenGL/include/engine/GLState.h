/*
  *@aim:OpenGL应用程序使用的宏
  &2016-4-30
*/
#ifndef   __GL_STATE_H__
#define  __GL_STATE_H__

/////////////////////////////////宏开关////////////////////////////////////////////////////////////////
//OpenGL的版本控制,是否是OpenGL版本,或者是OpenGLES版本
#if defined _WIN32 || defined _LINUX || defined _APPLE
#define      __OPENGL_VERSION__
#endif
//是否开启几何着色器,默认是不开启的,在OpenGLES版本中,必须禁止这个宏
#ifdef __OPENGL_VERSION__
#define      __GEOMETRY_SHADER__
#endif
//是否开启缓存,着色器缓存,纹理缓存
//#define     __ENABLE_PROGRAM_CACHE__    
//开启纹理缓存
#define     __ENABLE_TEXTURE_CACHE__
///////////////////////////////////////////////////////////////////////////////////////////////////
//枚举常量,由于着色器内属性变量位置的不一致,暂时不设定确切的值
#define      GLAttribPosition            0   //位置坐标
#define      GLAttribTexCoord          1   //纹理坐标
#define      GLAttribNormal             2  //法线

//最常用的着色器对象名字,在Sprite使用
#define      OpenGLSpriteProgram                     "GLK_OpenGLSpriteProgram"
//一般光照着色器
#define      OpenGLNormalLightProgram          "GLK_OpenGLLightProgram"
//点光源着色器
#define      OpenGLPointLightProgram             "GLK_OpenGLPointLightProgram"
//结构体内偏移
#define   __offsetof(s,m)           (char *)(&((s *)NULL)->m)
//是否启用 tsb_image库,如果开启了这个宏,则程序将会使用该库加载所有的图片
#define _USE_STB_IMAGE_    1
#ifndef  NULL
#define  NULL  0
#endif
#ifndef MATH_PI
#define MATH_PI 3.14159265358
#endif
//弧度与角度之间的转换
#define    GLK_RADIUS_TO_ANGLE(radius)  ((radius)*180/MATH_PI)
//角度与弧度之间的转换
#define    GLK_ANGLE_TO_RADIUS(angle) ((angle)*MATH_PI/180)
//符号计算
#define    __SIGNFloat(sign)   (-(  ((sign)&0x1)<<1)+1)
//重力系数
#define    GLK_GRAVITY_CONSTANT		9.810f
//引擎的命名空间名字
#define	 __NS_GLK_BEGIN                 namespace glk {
#define    __NS_GLK_END                     }
#define    __US_GLK__						    using namespace glk;
#endif