/*
  *�����,��Camera.h/Camera.cpp��ʵ�ֲ�ͬ,��ʵ���ʺϵ����ϵ�����
  *�������ڲ���Ѱַ,�����Գ���������ʵ��,Camera2��ʵ�����Ÿ��������
  *����͵ľ��Ƕ�������ĽǶȵ�����
  *2017��11��26��
  *@Author:xiaohuaxiong
*/
#ifndef __CAMERA2_H__
#define __CAMERA2_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
__NS_GLK_BEGIN
class Camera2 :public Object
{
private:
	//��ͼ�����������
	GLVector3	  _X, _Y, _Z;
	GLVector3    _forwardZ, _upVector;
	//�۾���λ��,�Լ�Ŀ��λ�õ�λ��
	GLVector3   _eyePosition, _targetPosition;
	GLVector4   _nearFarFovRatioVec;
	//��ͼ����/��ͼͶӰ����
	Matrix          _viewMatrix, _projMatrix,_viewProjMatrix;
	//��ͼͶӰ����������
	Matrix          _viewProjMatrixInverse;
	//�Ƿ���ͼͶӰ������Ҫ���¼���
	bool              _isViewProjDirty;
	//�Ƿ���ͼͶӰ������������Ҫ���¼���
	bool              _isViewProjInverseDirty;
private:
	Camera2();
	/*
	  *������ͼ����
	 */
	void   updateViewMatrix();
public:
	/*
	  *������������͸��ͶӰ����Ĳ���������ͬ
	 */
	void             initWithPerspective(float fov,float whrate,float nearZ,float farZ);
	/*
	  *��������������������Ĳ�����������ͬ
	 */
	void             initWithOrtho(float lx,float rx,float by,float ty,float nearZ,float farZ);
	/*
	  *����͸�Ӿ���
	 */
	static   Camera2   *createWithPerspective(float fov,float whrate,float nearZ,float farZ);
	/*
	  *��������ͶӰ����
	 */
	static   Camera2   *createWithOrtho(float lx,float rx,float by,float ty,float nearZ,float farZ);
	/*
	  *��ͼ����,�볣��Ĵ���͸�Ӿ���ķ�����ͬ,�ú���û��upVector����,
	  upVector��������ֵ��Ҫ��̬����
	 */
	void    lookAt(const GLVector3 &eyePosition,const GLVector3 &targetPosition);
	/*
	  *����ƫ�ƺ�������
	 */
	void    translate(const GLVector3 &offset);
	/*
	  *��ת,ע�������ת�Ĺ�����Camera��Ĳ�֮ͬ��
	  *xOffsetΪ�������X�᷽���ƫ����
	  *yOffsetΪ�������Y�᷽���ƫ����
	 */
	void   rotate(float xOffset,float yOffset);
	/*
	  *��ȡ�۾���λ��
	 */
	const GLVector3& getEyePosition()const { return _eyePosition; };
	/*
	  *��ȡĿ��λ��
	 */
	const GLVector3& getTargetPosition()const {return _targetPosition;};
	/*
	  *��ȡ��ͼ����
	 */
	const Matrix&       getViewMatrix()const { return _viewMatrix; };
	/*
	  *��ȡͶӰ����
	 */
	const Matrix&      getProjMatrix()const { return _projMatrix; };
	/*
	  *��ȡ��ͼͶӰ����
	 */
	const Matrix&       getViewProjMatrix();
	/*
	  *��ȡ��ͼͶӰ����������
	 */
	const Matrix&      getViewProjReversematrix();
	/*
	  *��ȡ��������X/Y/Z
	*/
	const GLVector3& getXVector()const { return _X; };
	const GLVector3& getUpVector()const { return _upVector; };
	const GLVector3& getForwardVector()const { return _forwardZ; };
	/*
	  *��ȡ��Զƽ��ľ���,�����͸��ͶӰ
	 */
	const GLVector4&  getNearFarFovRatio()const { return _nearFarFovRatioVec; };
};

__NS_GLK_END
#endif