/*
  *���͵��ε���Ⱦ
  *2017-06-07
  *@Author:xiaohuaxiong
  *@Version:1.0 ʵ����������ĵ�����Ⱦ,�ڴ˰汾�����е����ݵ�ʹ�ö�����ԭ��ģ��, ����һ���汾��,���ǽ�ʹ���Ĳ����Ե��ε���Ⱦ�ֿ�,�Ż�
  *@Version:2.0 
*/
#ifndef __TERRAIN_H__
#define __TERRAIN_H__
#include "engine/GLState.h"
#include "engine/Object.h"
#include "engine/GLTexture.h"
#include "engine/event/KeyEventListener.h"
#include "engine/event/TouchEventListener.h"
#include "TerrainShader.h"
#include<string>
class Terrain :public glk::Object
{
private:
	TerrainShader      *_terrainShader;
	//ģ�;���
	glk::Matrix             _modelMatrix;
	//��ͼ����
	glk::Matrix             _viewMatrix;
	//ͶӰ����
	glk::Matrix             _projMatrix;
	//��ͼͶӰ����
	glk::Matrix             _viewProjMatrix;
	//��ͼ���������������,��Ϊһ��demo,��û��ϵͳ����ȥ�����ͼ������,�����Ŀ�Ƚ�æ,�Ժ��п���ʱ��
	//�һᵥ��ȥ���Camera��,����5����������һ�𹹳���������ͼ����
	glk::GLVector3       _xAxis, _yAxis, _zAxis;
	//�۾���λ��
	glk::GLVector3       _eyePosition;
	glk::GLVector3       _targetPosition;
	//���ߵķ���,���뵥λ��
	glk::GLVector3       _lightDirection;
	//���ߵ���ɫ
	glk::GLVector4       _lightColor;
	//���εĻ�����ɫ
	glk::GLVector4       _terrainColor;
	//����ͶӰ�������ת������ƽ������
	glk::GLVector3       _rotateVec;
	glk::GLVector3       _translateVec;
	//�����¼�,�Ժ����
	glk::KeyEventListener      *_keyListener;
	//�����¼�,�Ժ����
	glk::TouchEventListener  *_touchListener;
	//��������
	int                                        _keyMask;
	//����ƫ����
	glk::GLVector2                   _touchOffset;
	//���㻺��������
	unsigned              _terrainVertexId;
	//��������������
	unsigned              _terrainIndexId;
	//���εĸ߶ȳ�����
	float                     *_heightField;
	//���εĿ��(�߶ȺͿ�����)
	int                       _terrainSize;
	//��¼���ε������С�߽�ֵ
	glk::GLVector3  _boundaryMin;
	glk::GLVector3  _boundaryMax;
private:
	Terrain();
	Terrain(Terrain &);
	/*
	  *���ض������ļ�
	*/
	void                     initWithFile(const std::string &filename);
	/*
	  *�����йع��ߵȵĲ���
	*/
	void                     initLightParam();
public:
	~Terrain();
	//ʹ�ø��������ļ�����
	static Terrain   *createTerrainWithFile(const std::string  &filename);
	//ʹ��ͼƬ�ļ����ص���
	static Terrain   *createTerrainWithTexture(glk::GLTexture *hightTexture);
	/*
	  *������ͼ��λ��,��ȡ��߳�ֵ
	 */
	inline  float        getHeightValue(int x,int z)const;
	/*
	  *����һ����ͼ����,��ȡ��ƽ���ĸ߳�ֵ
	 */
	 float         getHeightValueSmooth(float x,float z)const;
	/*
	  *������ͼ����,���������ڴ��е�ʵ������
	*/
	inline int            getRealIndex(int x,int z)const;
	/*
	  *��ʼ����ͼͶӰ��������
	 */
	void                     initCamera(const glk::GLVector3 &eyePosition,const glk::GLVector3 &targetPosition);
	//ʵʱ���»ص�����
	void                     update(const float deltaTime);
	//��Ⱦ
	void                     render();
	//�����¼�,�����¼�
	bool                     onKeyPressed(const glk::KeyCodeType keyCode);
	//�����ͷ�
	void                     onKeyReleased(const glk::KeyCodeType keyCode);
	//������ʼ
	bool                     onTouchBegan(const glk::GLVector2 *touchPoint);
	//�����ƶ�
	void                     onTouchMoved(const glk::GLVector2 *touchPoint);
	//����
	void                     onTouchEnded(const glk::GLVector2 *touchPoint);
	/*
	  *�ϳ���ͼ����,ʹ��_xAxis,_yAxis,_zAxis
	*/
	void                     buildViewMatrix();
};
#endif