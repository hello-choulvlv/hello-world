/*
  *äÖÈ¾Ì«ÑôµÄshader
  *Ê±¼ä2017-7-18
 */
#ifndef __RENDER_SHADER_H__
#define __RENDER_SHADER_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"

class RenderShader :public glk::Object
{
	glk::GLProgram *_glProgram;
	int                         _mvpMatrixLoc;
	int                         _colorLoc;
	int                         _positionLoc;
private:
	RenderShader();
	RenderShader(RenderShader &);
	void   initWithFile(const char *vsFile,const char *fsFile);
public:
	~RenderShader();
	static RenderShader *create(const char *vsFile,const char *fsFile);
	/*
	  *MVPMatrix
	 */
	void  setMVPMatrix(const glk::Matrix &mvpMatrix);
	/*
	  *color
	 */
	void  setColor(const glk::GLVector4 &color);
	//
	int     getPositionLoc()const;
	//
	void  perform()const;
};
#endif