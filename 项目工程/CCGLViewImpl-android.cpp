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

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#include "platform/android/CCGLViewImpl-android.h"
#include "base/CCDirector.h"
#include "base/ccMacros.h"
#include "base/CCConfiguration.h"
#include "platform/android/jni/JniHelper.h"
#include "CCGL.h"

#include <stdlib.h>
#include <android/log.h>

// <EGL/egl.h> exists since android 2.3
#include <EGL/egl.h>
PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOESEXT = 0;
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOESEXT = 0;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOESEXT = 0;

//#if defined(GL_ARB_texture_storage) || defined(GL_EXT_texture_storage)
PFN_GL_TEXTURESTORAGE2D_EXT_PROC  glTexStorage2DEXT_CV = 0;
//#endif

//#ifndef GL_EXT_draw_instanced
PFNGLDRAWARRAYSINSTANCEDEXTPROC glDrawArraysInstancedVK;
PFNGLDRAWELEMENTSINSTANCEDEXTPROC glDrawElementsInstancedVK;
//#endif

//#ifndef GL_EXT_instanced_arrays
PFNGLVERTEXATTRIBDIVISOREXTPROC glVertexAttribDivisorVK;
//#endif


PFNGLTEXIMAGE3D_API    glTexImage3D_CV = nullptr;
PFNGLTEXSUBIMAGE3D_API glTexSubImage3D_CV = nullptr;

#define apk_debug_log(...) __android_log_print(ANDROID_LOG_DEBUG,"--cc--gl---proc",__VA_ARGS__)

void initExtensions() {
    glGenVertexArraysOESEXT = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
    glBindVertexArrayOESEXT = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
    glDeleteVertexArraysOESEXT = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");

    //GL V3 is diffrient with V2
    //不能使用这种方式判定,实验表明,在V3版本下,获取的相关函数与V2情况下,是截然不同,V2版本下的函数在V3下虽不为nullptr,但是不会起任何的作用
    auto *config = cocos2d::Configuration::getInstance();
    config->gatherGPUInfo();
    bool supportV3 = config->supportGLES3();

    const char *storage_func = supportV3 ? "glTexStorage2D" : "glTexStorage2DEXT";
    glTexStorage2DEXT_CV = (PFN_GL_TEXTURESTORAGE2D_EXT_PROC)eglGetProcAddress(storage_func);
    //apk_debug_log("------glTexStorage2DEXT_CV-----%s-",glTexStorage2DEXT_CV != nullptr? "true" : "false");

    const char *draw_arrays = supportV3 ? "glDrawArraysInstanced" : "glDrawArraysInstancedEXT";
    const char *draw_elements = supportV3 ? "glDrawElementsInstanced" : "glDrawArraysInstancedEXT";

    glDrawArraysInstancedVK = (PFNGLDRAWARRAYSINSTANCEDEXTPROC)eglGetProcAddress(draw_arrays);
    glDrawElementsInstancedVK = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC)eglGetProcAddress(draw_elements);

    //apk_debug_log("------glDrawArraysInstancedVK--------%s----",glDrawArraysInstancedVK != nullptr? "true" : "false");
    //apk_debug_log("------glDrawElementsInstancedVK------%s-----",glDrawElementsInstancedVK != nullptr ? "true" : "false");

    const char *divisor_func = supportV3 ? "glVertexAttribDivisor" : "glVertexAttribDivisorEXT";
    glVertexAttribDivisorVK = (PFNGLVERTEXATTRIBDIVISOREXTPROC)eglGetProcAddress(divisor_func);
    //apk_debug_log("--------glVertexAttribDivisorVK--------%s------",glVertexAttribDivisorVK != nullptr ? "true" : "false");

    glTexImage3D_CV = (PFNGLTEXIMAGE3D_API)eglGetProcAddress("glTexImage3D");
    glTexSubImage3D_CV = (PFNGLTEXSUBIMAGE3D_API)eglGetProcAddress("glTexSubImage3D");

    if(supportV3 && (!glTexImage3D_CV || !glTexSubImage3D_CV))
        apk_debug_log("error ---- could not get texture 2d array api ......");
}

NS_CC_BEGIN

GLViewImpl* GLViewImpl::createWithRect(const std::string& viewName, Rect rect, float frameZoomFactor)
{
    auto ret = new GLViewImpl;
    if(ret && ret->initWithRect(viewName, rect, frameZoomFactor)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

GLViewImpl* GLViewImpl::create(const std::string& viewName)
{
    auto ret = new GLViewImpl;
    if(ret && ret->initWithFullScreen(viewName)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

GLViewImpl* GLViewImpl::createWithFullScreen(const std::string& viewName)
{
    auto ret = new GLViewImpl();
    if(ret && ret->initWithFullScreen(viewName)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

GLViewImpl::GLViewImpl()
{
    initExtensions();
}

GLViewImpl::~GLViewImpl()
{

}

bool GLViewImpl::initWithRect(const std::string& viewName, Rect rect, float frameZoomFactor)
{
    return true;
}

bool GLViewImpl::initWithFullScreen(const std::string& viewName)
{
    return true;
}


bool GLViewImpl::isOpenGLReady()
{
    return (_screenSize.width != 0 && _screenSize.height != 0);
}

void GLViewImpl::end()
{
    JniHelper::callStaticVoidMethod("org/cocos2dx/lib/Cocos2dxHelper", "terminateProcess");
}

void GLViewImpl::swapBuffers()
{
}

void GLViewImpl::setIMEKeyboardState(bool bOpen)
{
    if (bOpen) {
        JniHelper::callStaticVoidMethod("org/cocos2dx/lib/Cocos2dxGLSurfaceView", "openIMEKeyboard");
    } else {
        JniHelper::callStaticVoidMethod("org/cocos2dx/lib/Cocos2dxGLSurfaceView", "closeIMEKeyboard");
    }
}

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
