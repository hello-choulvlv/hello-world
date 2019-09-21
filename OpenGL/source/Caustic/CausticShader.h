/*
  *ˮ�⽻��/��ɢ��Caustic
  *@2017-8-23
  *@Author:xiaohuaxiong
 */
#ifndef __CAUSTIC_SHADER_H__
#define __CAUSTIC_SHADER_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"

class CausticShader :public glk::Object
{
	glk::GLProgram			*_glProgram;
	int								_waterHeightMapLoc;
	int								_waterNormalMapLoc;
	int								_modeMatrixLoc;
	int                               _groundHeightLoc;
	int                               _waterHeightLoc;
	int                               _lightDirectionLoc;
	int                               _resolutionLoc;
	int                               _groundMapLoc;
private:
	CausticShader();
	CausticShader(const CausticShader &);
	void             init(const char *vsFile,const char *fsFile);
public:
	~CausticShader();
	static  CausticShader *create(const char *vsFile,const char *fsFile);
	/*
	  *Model Matrix
	 */
	void         setModelMatrix(const glk::Matrix &modelMatrix);
	/*
	  *����ˮ��߶ȳ�����
	 */
	void          setWaterHeightMap(int textureId,int unit);
	/*
	  *����ˮ�淨������
	 */
	void          setWaterNormalMap(int textureId,int unit);
	/*
	  *���õ���߶�
	 */
	void         setGroundHeight(float groundHeight);
	/*
	  *����ˮ��ĸ߶�
	 */
	void         setWaterHeight(float waterHeight);
	/*
	  *���ù��ߵķ���
	 */
	void        setLightDirection(const glk::GLVector3 &lightDirection);
	/*
	  *���õ���ķֱ���
	 */
	void        setResolution(const glk::GLVector2 &resolution);
	/*
	  *���õ�������
	 */
	void       setGroundMap(int textureId,int unit);
	/*
	  *perform
	 */
	void       perform()const;
};

#endif