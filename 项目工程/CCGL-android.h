/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#ifndef __CCGL_H__
#define __CCGL_H__

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#define glClearDepth                glClearDepthf
#define glDeleteVertexArrays        glDeleteVertexArraysOES
#define glGenVertexArrays           glGenVertexArraysOES
#define glBindVertexArray           glBindVertexArrayOES
#define glMapBuffer                 glMapBufferOES
#define glUnmapBuffer               glUnmapBufferOES

#define GL_DEPTH24_STENCIL8         GL_DEPTH24_STENCIL8_OES
#define GL_WRITE_ONLY               GL_WRITE_ONLY_OES

// GL_GLEXT_PROTOTYPES isn't defined in glplatform.h on android ndk r7 
// we manually define it here
#include <GLES2/gl2platform.h>
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

// normal process
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
// gl2.h doesn't define GLchar on Android
typedef char GLchar;
// android defines GL_BGRA_EXT but not GL_BRGA
#ifndef GL_BGRA
#define GL_BGRA  0x80E1
#endif

//declare here while define in EGLView_android.cpp
extern PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOESEXT;
extern PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOESEXT;
extern PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOESEXT;

#define glGenVertexArraysOES glGenVertexArraysOESEXT
#define glBindVertexArrayOES glBindVertexArrayOESEXT
#define glDeleteVertexArraysOES glDeleteVertexArraysOESEXT

//#if defined(GL_ARB_texture_storage) || defined(GL_EXT_texture_storage)
typedef void (GL_APIENTRYP PFN_GL_TEXTURESTORAGE2D_EXT_PROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
extern  PFN_GL_TEXTURESTORAGE2D_EXT_PROC  glTexStorage2DEXT_CV;

#ifndef glTexStorage2D
#define glTexStorage2D  glTexStorage2DEXT_CV
#endif
//#endif

//#ifndef GL_EXT_draw_instanced
typedef void (GL_APIENTRYP PFNGLDRAWARRAYSINSTANCEDEXTPROC) (GLenum mode, GLint start, GLsizei count, GLsizei primcount);
typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSINSTANCEDEXTPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);

extern PFNGLDRAWARRAYSINSTANCEDEXTPROC glDrawArraysInstancedVK;
extern PFNGLDRAWELEMENTSINSTANCEDEXTPROC glDrawElementsInstancedVK;
//#endif

#define glDrawArraysInstanced			glDrawArraysInstancedVK
#define glDrawElementsInstanced 	glDrawElementsInstancedVK


//#ifndef GL_EXT_instanced_arrays
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBDIVISOREXTPROC) (GLuint index, GLuint divisor);
extern  PFNGLVERTEXATTRIBDIVISOREXTPROC glVertexAttribDivisorVK;
//#endif
#define glVertexAttribDivisor 			glVertexAttribDivisorVK

typedef void (GL_APIENTRYP PFNGLTEXIMAGE3D_API)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
extern PFNGLTEXIMAGE3D_API glTexImage3D_CV;// (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);

typedef void (GL_APIENTRYP PFNGLTEXSUBIMAGE3D_API)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
extern PFNGLTEXSUBIMAGE3D_API glTexSubImage3D_CV;// (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);

#define glTexImage3D glTexImage3D_CV
#define glTexSubImage3D glTexSubImage3D_CV

#ifndef GL_TEXTURE_2D_ARRAY
#define GL_TEXTURE_2D_ARRAY               0x8C1A
#endif

#ifndef GL_SAMPLER_2D_ARRAY
#define GL_SAMPLER_2D_ARRAY               0x8DC1
#endif

/*
  *安卓跟iOS的半浮点数定义似乎与官方给出的规范有冲突
  *目前这里使用的是官方给出的定义
 */
#ifndef GL_HALF_FLOAT
#define GL_HALF_FLOAT 0x140B
#endif

#ifndef GL_RG16F
#define GL_RG16F 0x822F
#endif

#ifndef GL_RG
#define GL_RG 0x8227
#endif

#ifndef GL_TEXTURE_COMPARE_MODE
#define GL_TEXTURE_COMPARE_MODE 0x884C
#endif

#ifndef GL_COMPARE_R_TO_TEXTURE
#define GL_COMPARE_R_TO_TEXTURE 0x884E
#endif

#ifndef GL_TEXTURE_COMPARE_FUNC
#define GL_TEXTURE_COMPARE_FUNC 0x884E
#endif

#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24 0x81A6
#endif

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#endif // __CCGL_H__
