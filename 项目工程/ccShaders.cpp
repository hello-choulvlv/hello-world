/****************************************************************************
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2012 		cocos2d-x.org
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

#include "renderer/ccShaders.h"

#define STRINGIFY(A)  #A

NS_CC_BEGIN
//

#include "renderer/ccShader_Position_uColor.frag"

#include "renderer/ccShader_Position_uColor.vert"

//
#include "renderer/ccShader_PositionColor.frag"
#include "renderer/ccShader_PositionColor.vert"

//
#include "renderer/ccShader_PositionColorTextureAsPointsize.vert"

//
#include "renderer/ccShader_PositionTexture.frag"
#include "renderer/ccShader_PositionTexture.vert"

//
#include "renderer/ccShader_PositionTextureA8Color.frag"
#include "renderer/ccShader_PositionTextureA8Color.vert"

//
#include "renderer/ccShader_PositionTextureColor.frag"
#include "renderer/ccShader_PositionTextureColor.vert"

//
#include "renderer/ccShader_PositionTextureColor_noMVP.frag"
#include "renderer/ccShader_PositionTextureColor_noMVP.vert"

//
#include "renderer/ccShader_PositionTextureColorAlphaTest.frag"

//
#include "renderer/ccShader_PositionTexture_uColor.frag"
#include "renderer/ccShader_PositionTexture_uColor.vert"

#include "renderer/ccShader_PositionColorLengthTexture.frag"
#include "renderer/ccShader_PositionColorLengthTexture.vert"

#include "renderer/ccShader_UI_Gray.frag"
//
#include "renderer/ccShader_Label.vert"
#include "renderer/ccShader_Label_df.frag"
#include "renderer/ccShader_Label_df_glow.frag"
#include "renderer/ccShader_Label_normal.frag"
#include "renderer/ccShader_Label_outline.frag"

//
#include "renderer/ccShader_3D_PositionTex.vert"
#include "renderer/ccShader_3D_Color.frag"
#include "renderer/ccShader_3D_ColorTex.frag"
#include "renderer/ccShader_3D_PositionNormalTex.vert"
#include "renderer/ccShader_3D_ColorNormal.frag"
#include "renderer/ccShader_3D_ColorNormalTex.frag"
#include "renderer/ccShader_3D_Particle.vert"
#include "renderer/ccShader_3D_Particle.frag"
#include "renderer/ccShader_3D_Skybox.vert"
#include "renderer/ccShader_3D_Skybox.frag"
#include "renderer/ccShader_3D_Terrain.vert"
#include "renderer/ccShader_3D_Terrain.frag"
#include "renderer/ccShader_CameraClear.vert"
#include "renderer/ccShader_CameraClear.frag"

// ETC1 ALPHA support
#include "renderer/ccShader_ETC1AS_PositionTextureColor.frag"
#include "renderer/ccShader_ETC1AS_PositionTextureGray.frag"

#include "ccShader_3DSkinPositionNoralTexFish.vert"
#include "ccShader_3DSkinPositionNoralTexFish.frag"

const char *cc3d_ClearDepthBuffer_Vert = "attribute		vec4        a_position;"
"void     main(){"
"		gl_Position = CC_PMatrix * a_position;"
"		gl_Position.z = gl_Position.w;"
"}";
const char *cc3d_ClearDepthBuffer_Frag = "void main(){/* gl_FragColor=vec4(0.8,0.5,0.5,0.5);*/}";
//LiSM
const char *cc3d_ShadowMap_LiSM_Vert = "attribute vec4 a_position;\n"
"uniform  mat4  u_ModelMatrix;\n"
"uniform  mat4 u_ViewProjMatrix;\n"
"void	main()\n"
"{\n"
"	gl_Position = u_ViewProjMatrix * u_ModelMatrix * a_position;\n"
"}\n";
const char *cc3d_ShadowMap_LiSM_Frag = "void main()\n"
"{\n"
"}";
//VSM
const char *cc3d_ShadowMap_VSM_Vert = "attribute vec4 a_position;\n"
"uniform  mat4 u_ModelMatrix;\n"
"uniform  mat4 u_ViewProjMatrix;\n"
"varying vec4	v_position;\n"
"void main()"
"{\n"
"		vec4	position = u_ModelMatrix * a_position;\n"
"		gl_Position = u_ViewProjMatrix * position;\n"
"		v_position = position;\n"
"}";
const char *cc3d_ShadowMap_VSM_Frag = "uniform  mat4 u_LightViewMatrix;\n"
"varying	vec4	v_position;\n"
"void	main()\n"
"{"
"		vec4	position = u_LightViewMatrix * v_position;\n"
"		float    length_l = -position.z;\n"
"		gl_FragColor = vec4(length_l,length_l * length_l,0.0,1.0);\n"
"}";
//box fuzzy
const char *cc3d_BoxFuzzy_Vert = "attribute vec4	a_position;\n"
"attribute vec2	a_fragCoord;\n"
"varying vec2	v_fragCoord;\n"
"void	main()"
"{\n"
"	gl_Position = a_position;\n"
"	v_fragCoord = a_fragCoord;\n"
"}";
const char *cc3d_BoxFuzzy_Frag = "uniform  sampler2D  u_BaseMap;\n"
"uniform	vec2	u_PixelStep;\n"
"uniform	int		u_FuzzyCount;\n"
"varying  vec2	v_fragCoord;\n"
"void	main()\n"
"{\n"
"		float fuzzy_count = float(u_FuzzyCount);\n"
"		vec2  offsetPixel = v_fragCoord + u_PixelStep * fuzzy_count * 0.5;\n"
"		vec2  pixel2 = vec2(0.0);\n"
"		for(int k=0; k < u_FuzzyCount; ++k)\n"
"		{\n"
"			vec2  f_v2 = texture2D(u_BaseMap,offsetPixel + float(k) * u_PixelStep).xy;\n"
"			pixel2 += f_v2;\n"
"		}\n"
"		pixel2 /= fuzzy_count;\n"
"		gl_FragColor = vec4(pixel2,0.0,1.0);\n"
"}";

const char *cc3d_DebugTexture_Vert = "attribute vec4 a_position;\n"
"attribute vec2 a_fragCoord;\n"
"varying  vec2  v_fragCoord;\n"
"void	main()"
"{"
"		gl_Position = a_position;\n"
"		v_fragCoord = a_fragCoord;\n"
"}";
const char *cc3d_DebugTexture_Frag = "varying  vec2  v_fragCoord;\n"
"void  main()\n"
"{\n"
"	gl_FragColor = texture2D(CC_Texture0,v_fragCoord);\n"
"}";

const char *cc3d_FramebufferDepthClear_Vert = "attribute vec4 a_position;\n"
"void  main()\n"
"{"
"		gl_Position = vec4(a_position.xy,a_position.w,a_position.w);\n"
"}";
const char *cc3d_FramebufferDepthClear_Frag = "uniform  vec4  u_color;\n"
"void  main()\n"
"{\n"
"	gl_FragColor = vec4(0.0);\n"
"}\n";

NS_CC_END
