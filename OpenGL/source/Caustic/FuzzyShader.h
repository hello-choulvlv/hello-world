/*
  *Í¼ÏñÄ£ºý´¦Àí
  *2017-8-23
  *@Author:xiaohuaxiong
 */
#ifndef __FUZZY_SHADER_H__
#define __FUZZY_SHADER_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"

class FuzzyShader :public glk::Object
{
	glk::GLProgram			*_glProgram;
	//base map
	int                                _baseMapLoc;
	int                                _pixelStepLoc;
	//
private:
	FuzzyShader();
	FuzzyShader(const FuzzyShader &);
	void			init(const char *vsFile,const char *fsFile);
public:
	~FuzzyShader();
	static		FuzzyShader      *create(const char *vsFile, const char *fsFile);
	/*
	  *base map
	 */
	void		setBaseMap(int textureId,int unit);
	/*
	  *pixel step
	 */
	void     setPixelStep(const glk::GLVector2   &pixelStep);
	/*
	  *perform
	 */
	void     perform();
};
#endif