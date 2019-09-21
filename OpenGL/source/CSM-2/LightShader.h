/*
  *光视图下的场景渲染Shader
  *@date:2017-4-8
  *@author:xiaohuaxiong
  */
#ifndef __LIGHT_SHADER_H__
#define __LIGHT_SHADER_H__
#include<engine/Geometry.h>
#include<engine/GLProgram.h>
class LightShader
{
private:
	glk::GLProgram     *_lightProgram;
	//location of OpenGL Shader uniform 
	int                      _mvpMatrixLoc;
	int                      _cropMatrixLoc;
//	int                      _viewportArrayLoc;
//	int                      _shadowMapSizeLoc;

	glk::Matrix               _mvpMatrix;
	glk::Matrix               _cropMatrix[4];
//	glk::GLVector4		   _viewportArray[4];
//	glk::Size                    _shadowMapSize;
private:
	LightShader();
	void     loadShaderSource(const char *vsFile,const char *gsFile,const char *fsFile);
public:
	~LightShader();
	static LightShader *createShaderWithSource(const char *vsFile,const char *gsFile,const char *fsFile);
	//setting viewport size
//	void     setViewports(const glk::GLVector4 *viewport,const int  size);
	//setting shadow size
//	void     setShadowMapSize(const glk::Size  &shadowMapSize);
	//setting mvp matrix
	void     setMVPMatrix(glk::Matrix &mvpMatrix);
	//Crop Matrix
	void		setCropMatrix(const glk::Matrix cropMatrix[4],const int size);
	//perform shader
	void     perform();
};
#endif