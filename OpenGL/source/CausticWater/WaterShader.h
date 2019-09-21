/*
  *ˮ��ȾShader��װ
  *@2017-8-3
  *@Author:xiaohuaxiong
*/
#ifndef __WATER_SHADER_H__
#define __WATER_SHADER_H__
#include "engine/Object.h"
#include "engine/GLProgram.h"
#include "engine/Geometry.h"

class WaterShader :public glk::Object
{
	glk::GLProgram   *_glProgram;
	//Model Matrix
	int							_modelMatrixLoc;
	int                           _viewProjMatrixLoc;
	int                           _normalMatrixLoc;
	int                           _heightMapLoc;
	int                           _normalMapLoc;
	int                           _texCubeMapLoc;
	int                           _cubeMapNormalLoc;
	int                           _cameraPositionLoc;
	int                           _lightPositionLoc;
	int                           _halfCubeHeightLoc;
	int                           _waterHeightLoc;
	int                           _positionLoc;
	int                           _fragCoordLoc;
private:
	WaterShader();
	WaterShader(const WaterShader &);
	void		initWithFile(const char *vsFile,const char *fsFile);
public:
	~WaterShader();
	static WaterShader *create(const char *vsFile,const char *fsFile);
	/*
	  *����ģ�;���
	 */
	void		setModelMatrix(const glk::Matrix &modelMatrix);
	/*
	  *��ͼͶӰ����
	  */
	void     setViewProjMatrix(const glk::Matrix &viewProjMatrix);
	/*
	  *���߾���
	*/
	//void     setNormalMatrix(const glk::Matrix3 &normalMatrix);
	/*
	  *��������,�߶���ͼ
	 */
	void     setHeightMap(int heightMapId,int unit);
	/*
	  *������ͼ
	*/
	void     setNormalMap(int normalMapId,int unit);
	/*
	  *��������ͼ
	 */
	void    setTexCubeMap(int texCubeMapId,int	unit);
	/*
	  *�����巨��
	 */
	void    setCubeMapNormal(const glk::GLVector3 *cubeMapNormal,int size);
	/*
	  *�������λ��
	 */
	void   setCameraPosition(const glk::GLVector3 &cameraPosition);
	/*
	  *��Դ��λ��
	*/
	void   setLightDirection(const glk::GLVector3 &lightDirection);
	/*
	  *ˮ��ĸ߶�
	 */
	void   setWaterHeight(float waterHeight);
	/*
	  *��������ĸ߶�
	 */
	void   setHalfCubeHeight(float halfCubeHeight);
	/*
	  *perform
	 */
	void  perform();
	/*
	  *position
	 */
	int	getPositionLoc();
	/*
	  *fragCoord
	 */
	int  getFragCoordLoc();
};
#endif