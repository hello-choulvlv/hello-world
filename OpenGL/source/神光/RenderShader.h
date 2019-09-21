//作为GodRay应用程序的后半部分颜色处理
//2017-6-28
//@Author:xiaohuaxiong
#ifndef __RENDER_SHADER_H__
#define __RENDER_SHADER_H__
#include "engine/GLProgram.h"
#include "engine/Geometry.h"

class RenderShader :public glk::Object
{
private:
	glk::GLProgram       *_glProgram;
	int  _modelViewProjMatrixLoc;//MVP矩阵的位置
	int  _baseMapLoc;//作为纹理输入的位置
	int  _positionLoc;//position
	int  _fragCoordLoc;//frag coord
private:
	RenderShader(RenderShader &);
	RenderShader();
	void   initWithFile(const char *vsFile,const char *fsFile);
public:
	~RenderShader();
	static RenderShader *createRenderShader(const char *vsFile,const char *fsFile);
	//设置模型投影矩阵
	void  setMVPMatrix(const glk::Matrix &mvp);
	//设置输入纹理
	void  setBaseMap(int texture,int unit);
	//获取顶点的位置
	int     getPositionLoc()const;
	//获取纹理坐标的位置
	int     getFragCoordLoc()const;
	void   perform()const;
};
#endif