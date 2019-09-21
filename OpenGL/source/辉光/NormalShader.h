/*
  *ֱͨshader
  *2017-7-18
 */
#ifndef __NORMAL_SHADER_H__
#define __NORMAL_SHADER_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"

class NormalShader :public glk::Object
{
	glk::GLProgram *_glProgram;
	int                         _mvpMatrixLoc;
	int                         _baseMapLoc;
	int                         _positionLoc;
	int                         _fragCoordLoc;
private:
	NormalShader();
	NormalShader(NormalShader &);
	void initWithFile(const char *vsFile, const char *fsFile);
public:
	~NormalShader();
	static NormalShader *create(const char *vsFIle,const char *fsFile);
	//MVP
	void   setMVPMatrix(const glk::Matrix &mvpMatrix);
	//base map
	void   setBaseMap(int textureId,int unit);
	//
	void perform()const;
	//
	int   getPositionLoc()const;
	//
	int  getFragCoordLoc()const;
};
#endif