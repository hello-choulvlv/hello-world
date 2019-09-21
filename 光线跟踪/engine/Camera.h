/*
  *�������,��Ҫ���ڴ��͵��ε���Ⱦ,�ռ䳡��׷�ٵ�һϵ�е�3d����
  *date:2017-6-14
  *@Author:xiaohuaxiong
 */
#ifndef __CAMERA_H__
#define __CAMERA_H__
#include "Geometry.h"
#include "Types.h"
/*
  *��������,�ڴ��͵���������
 */
class Camera 
{
	CameraType       _cameraType;
	//��ͼ����
	mat4x4    _viewMatrix;
	//��ͼ�������
	mat4x4    _inverseViewMatrix;
	//ͶӰ����
	mat4x4    _projMatrix;
	//ͶӰ�������
	mat4x4   _inverseProjMatrix;
	//��ͼͶӰ����
	mat4x4   _viewProjMatrix;
	//��ͼͶӰ�������
	mat4x4   _inverseViewProjMatrix;
	//����������ɢ������������,���ںϳ���ͼ����,ͬʱҲ�������ͼ�������Լ��
	float_3    _xAxis;
	float_3    _yAxis;
	float_3    _zAxis;
	//��������ӵ��Ŀ�⳯��ĵ�
	float_3   _eyePosition;
	float_3   _targetPosition;
	//��ת����
	float_3   _rotateVec;
	//ƽ������
	float_3   _translateVec;
	//�Ƿ���ͼ���������䶯
	bool             _isViewDirty;
	//�Ƿ�ͶӰ�������˱䶯
	bool            _isProjMatrixDirty;
	//�Ƿ���ͼͶӰ�������˱仯
	bool            _isViewProjDirty;
	//�Ƿ���ͼͶӰ�������������˱仯
	bool            _isInverseViewProjDirty;
private:
	Camera();
	Camera(Camera &);
	void   initCamera(const float_3 &eyePosition,const float_3 &targetPosition,const float_3 &upVec);
public:
	~Camera();
	/*
	  *�����������,�����۾����ڵ�����,Ŀ�⳯�������,�Լ����ϵķ�������
	  *ע��,��������·�������Ӧ�ó���,�����ڳ�������,�������ε�ʱ��ʼ�ջ�������������Լ��
	 */
	static Camera    *createCamera(const float_3 &eyePosition,const float_3 &targetPosition,const float_3 &upVec);
	/*
	  *���������������
	 */
	inline CameraType   getType()const { return _cameraType; };
	/*
	  *����͸��ͶӰ����
	  *@param:angleΪ͸�ӽǶ�
	  *@param:widthRate��Ļ�ĺ��ݱ�
	  *@param:screenHeight��Ļ�ĸ߶�
	  *@param:nearZ,farZ��Զƽ��
	 */
	void    setPerspective(const float angle,const float widthRate,const float nearZ,const float farZ);
	/*
	  *��������ͶӰ
	  *@param:left,right��Ļ�����ҿ��
	  *@param:bottom,top��Ļ�ĵײ�,�ϲ��Ŀ��
	  *@param:near,far��Զƽ��
	*/
	void   setOrtho(float left,float right,float bottom,float top,float nearZ,float farZ);
	/*
	  *��ȡ��ͼͶӰ����
	 */
	const mat4x4 &getViewProjMatrix();
	/*
	  *��ȡ��ͼ����
	 */
	const mat4x4& getViewMatrix()const;
	/*
	  *��ȡͶӰ����
	 */
	const mat4x4 &getProjMatrix()const;
	/*
	  *��ȡ��ͼ�������
	 */
	const mat4x4 &getInverseViewMatrix();
	/*
	  *��ȡ��ͼͶӰ�������
	 */
	const mat4x4 &getInverseViewProjMatrix();
	/*
	  *������ͼ�������ת����,dx,dy��ʾ��ͼ�������ת������X��Y��ı仯��,�ԽǶ�Ϊ׼
	  *�����ļ��㷽ʽΪ dx = -offsetY/winSize.height * _ANGLE_FACTOR_ * 0.5f;
	  *dy = offsetX /winSize.width *_ANGLE__FACTOR_ *0.5f;
	  *����winSize�����ڵĴ�С,_ANGLE_FACTOR_���Բ鿴Ӱ���е���ֵ
	 */
	void                  updateRotateMatrix(float dx,float dy);
	/*
	  *������ͼ�����ƽ�ƾ���
	  *dx:��ʾ��X���ƶ��ľ���
	  *dz:��ʾ��Z�᷽���ƶ��ľ���
	  *�˶��߶����Ա�׼OpenGL����ϵ
	  *��ֵ��ʾ��������,��ֵ��ʾ������
	 */
	void                  updateTranslateMatrix(float dx,float dz);
	/*
	  *��ȡ�����������
	 */
	const float_3&  getCameraPosition()const;
};

#endif