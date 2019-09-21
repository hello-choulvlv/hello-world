/*
  *�������,��Ҫ���ڴ��͵��ε���Ⱦ,�ռ䳡��׷�ٵ�һϵ�е�3d����
  *date:2017-6-14
  *@Author:xiaohuaxiong
 */
#ifndef __CAMERA_H__
#define __CAMERA_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/Types.h"
__NS_GLK_BEGIN
/*
  *��������,�ڴ��͵���������
 */
class Camera :public Object
{
	CameraType       _cameraType;
	//��ͼ����
	Matrix    _viewMatrix;
	//��ͼ�������
	Matrix    _inverseViewMatrix;
	//ͶӰ����
	Matrix    _projMatrix;
	//ͶӰ�������
	Matrix   _inverseProjMatrix;
	//��ͼͶӰ����
	Matrix   _viewProjMatrix;
	//��ͼͶӰ�������
	Matrix   _inverseViewProjMatrix;
	//����������ɢ������������,���ںϳ���ͼ����,ͬʱҲ�������ͼ�������Լ��
	GLVector3    _xAxis;
	GLVector3    _yAxis;
	GLVector3    _zAxis;
	//��������ӵ��Ŀ�⳯��ĵ�
	GLVector3   _eyePosition;
	GLVector3   _targetPosition;
	//��ת����
	GLVector3   _rotateVec;
	//ƽ������
	GLVector3   _translateVec;
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
	void   initCamera(const GLVector3 &eyePosition,const GLVector3 &targetPosition,const GLVector3 &upVec);
public:
	~Camera();
	/*
	  *�����������,�����۾����ڵ�����,Ŀ�⳯�������,�Լ����ϵķ�������
	  *ע��,��������·�������Ӧ�ó���,�����ڳ�������,�������ε�ʱ��ʼ�ջ�������������Լ��
	 */
	static Camera    *createCamera(const GLVector3 &eyePosition,const GLVector3 &targetPosition,const GLVector3 &upVec);
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
	const Matrix &getViewProjMatrix();
	/*
	  *��ȡ��ͼ����
	 */
	const Matrix& getViewMatrix()const;
	/*
	  *��ȡͶӰ����
	 */
	const Matrix &getProjMatrix()const;
	/*
	  *��ȡ��ͼ�������
	 */
	const Matrix &getInverseViewMatrix();
	/*
	  *��ȡ��ͼͶӰ�������
	 */
	const Matrix &getInverseViewProjMatrix();
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
	const GLVector3&  getCameraPosition()const;
};

__NS_GLK_END
#endif