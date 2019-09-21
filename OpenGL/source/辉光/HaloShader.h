/*
  *�Թ�ϳ�Shader
  *2017-07-17
  *@Author:xiaohuaxiong
 */
#ifndef __HALO_SHADER_H__
#define __HALO_SHADER_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"

class HaloShader :public glk::Object
{
	glk::GLProgram *_glProgram;
	//ģ����ͼͶӰ����
	int		_mvpMatrixLoc;
	//�̶߳�ģ��������ͼ
	int       _highTextureLoc;
	//�س̶�ģ��������
	int       _lowTextureLoc;
	//��������
	int       _baseTextureLoc;
	//��ɢ�̶�
	int       _dispersalLoc;
	//�Թ�뾶
	int       _haloWidthLoc;
	//�����ɫ����
	int       _intensityLoc;
	//̫������Ļ����
	int       _sunProjPositionLoc;
	//Ť���̶�
	int       _distortionLoc;
	//λ������
	int       _positionLoc;
	//��������
	int       _fragCoordLoc;
private:
	HaloShader();
	~HaloShader();
	void   initWithFile(const char *vsFile,const char *fsFile);
public:
	static HaloShader *HaloShader::create(const char *vsFile,const char *fsFile);
	/*
	  *����ģ����ͼͶӰ����
	 */
	void			setMVPMatrix(const glk::Matrix &mvp);
	/*
	  *���ø̶߳ȵ�����λ��
	 */
	void        setHighTexture(int textureId,int unit);
	/*
	  *���õس̶ȵ�����λ��
	 */
	void        setLowTexture(int textureId,int unit);
	/*
	  *���û�������
	 */
	void        setBaseTexture(int textureId,int unit);
	/*
	  *���÷�ɢ�̶�
	 */
	void        setDispersal(float dispersal);
	/*
	  *���ûԹ�뾶
	 */
	void       setHaloWidth(float haloWidth);
	/*
	  *����������ɫ����
	 */
	void       setHaloIntensity(float intensity);
	/*
	  *����̫����������Ļ����(NDC)֮��[0-1]֮�������
	 */
	void      setSunProjPosition(const glk::GLVector2 &projPosition);
	/*
	  *�������ɻԹ��ʱ����ɫ��Ť���̶�
	 */
	void      setDistortion(const glk::GLVector3 &distortion);
	//��ȡλ������
	int        getPositionLoc()const;
	//��ȡ��������
	int        getFragCoordLoc()const;
	//
	void      perform()const;
};
#endif