/*
  *辉光合成Shader
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
	//模型视图投影矩阵
	int		_mvpMatrixLoc;
	//高程度模糊的纹理图
	int       _highTextureLoc;
	//地程度模糊的纹理
	int       _lowTextureLoc;
	//基本纹理
	int       _baseTextureLoc;
	//分散程度
	int       _dispersalLoc;
	//辉光半径
	int       _haloWidthLoc;
	//最后颜色缩放
	int       _intensityLoc;
	//太阳的屏幕坐标
	int       _sunProjPositionLoc;
	//扭曲程度
	int       _distortionLoc;
	//位置坐标
	int       _positionLoc;
	//纹理坐标
	int       _fragCoordLoc;
private:
	HaloShader();
	~HaloShader();
	void   initWithFile(const char *vsFile,const char *fsFile);
public:
	static HaloShader *HaloShader::create(const char *vsFile,const char *fsFile);
	/*
	  *设置模型视图投影矩阵
	 */
	void			setMVPMatrix(const glk::Matrix &mvp);
	/*
	  *设置高程度的纹理位置
	 */
	void        setHighTexture(int textureId,int unit);
	/*
	  *设置地程度的纹理位置
	 */
	void        setLowTexture(int textureId,int unit);
	/*
	  *设置基本纹理
	 */
	void        setBaseTexture(int textureId,int unit);
	/*
	  *设置分散程度
	 */
	void        setDispersal(float dispersal);
	/*
	  *设置辉光半径
	 */
	void       setHaloWidth(float haloWidth);
	/*
	  *设置最后的颜色缩放
	 */
	void       setHaloIntensity(float intensity);
	/*
	  *设置太阳的最后的屏幕坐标(NDC)之后[0-1]之间的坐标
	 */
	void      setSunProjPosition(const glk::GLVector2 &projPosition);
	/*
	  *设置生成辉光的时候颜色的扭曲程度
	 */
	void      setDistortion(const glk::GLVector3 &distortion);
	//获取位置坐标
	int        getPositionLoc()const;
	//获取纹理坐标
	int        getFragCoordLoc()const;
	//
	void      perform()const;
};
#endif