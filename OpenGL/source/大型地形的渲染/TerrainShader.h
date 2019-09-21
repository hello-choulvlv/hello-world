/*
  *��Ⱦ���ε�OpenGL Shader
  *@date:2017-6-8
  *@Author:xiaohuaxiong
  */
#ifndef __TERRAIN_SHADER_H__
#define __TERRAIN_SHADER_H__
#include "engine/GLProgram.h"
#include "engine/Geometry.h"
class TerrainShader
{
	glk::GLProgram *_terrainProgram;
	//ģ�;����shaderλ��
	int                         _modelMatrixLoc;
	//��ͼͶӰ�����λ��
	int                         _viewProjMatrixLoc;
	//���߾���
	int                         _normalMatrixLoc;
	//�۾���λ��
	int                         _eyePositionLoc;
	//��ɫ
	int                         _colorLoc;
	//���ߵ���ɫ
	int                         _lightColorLoc;
	//���ߵķ���
	int                         _lightDirectionLoc;
	//�����λ��
	int                         _positionLoc;
	//����ķ���
	int                         _normalLoc;
private:
	TerrainShader();
	TerrainShader(TerrainShader &);
	void    initWithFile(const char *vsFile,const char *fsFile);
public:
	~TerrainShader();

	static TerrainShader *createTerrainShader(const char *vsFile,const char *fsFile);
	/*
	  *����ģ�;���
	 */
	void   setModelMatrix(const glk::Matrix &modelMatrix);
	/*
	  *������ͼͶӰ����
	 */
	void   setViewProjMatrix(const glk::Matrix &viewProjMatrix);
	/*
	  *���÷��߾���
	 */
	void   setNormalMatrix(const glk::Matrix3 &normalMatrix);
	/*
	  *���ù۲��ߵ�λ��
	 */
	void   setEyePosition(const glk::GLVector3 &eyePosition);
	/*
	  *���ù��ߵķ���
	*/
	void   setLightDirection(const glk::GLVector3 &lightDirection);
	/*
	  *���ù��ߵ���ɫ
	 */
	void   setLightColor(const glk::GLVector4 &lightColor);
	/*
	  *���õ��εĻ�����ɫ
	 */
	void   setTerrainColor(const glk::GLVector4 &color);
	/*
	  *�����λ��
	 */
	int     getPositionLoc()const;
	/*
	  *���ߵ�λ��
	 */
	int     getNormalLoc()const;
	/*
	  *ʹ��Shader
	 */
	void   perform()const;
};
#endif