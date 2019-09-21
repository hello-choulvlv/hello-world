/*
  *�����β���,ע��Triangle����𳣹��������ʵ��Ҫ���Ӻܶ�
  *��Ϊ������������ռ佨����Ѱַ�õ�
  *�����������ݲ����д�ͳ����������/����,������������
  *@date:2017/11/28
  *@Author:xiaohuaxiong
 */
#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__
#include "engine/Geometry.h"
class Triangle
{
private:
	//�����ε���������
	glk::GLVector3  _vertexA, _vertexB, _vertexC;
	//�����ε��������γɵ�����
	glk::GLVector3  _abVector,_bcVector,_caVector;
	//���ǵ������ߵĳ���
	float                      _edgeAB,_edgeBC,_edgeCA;
	//�����ε�ƽ�淽��
	glk::Plane          _nEquation;
	glk::GLVector3  _nXZ;//�����ε�ƽ�淽����XOZƽ���γɵ�ͶӰ����,���뾭����λ��
	//�����ε��������Լ�����Ϊƽ��,�ֱ𾭹��������������ƽ�淽��
	glk::Plane          _extraEquationA,_extraEquationB,_extraEquationC;
	//�ֱ�ֱ�������ε�������,����+Y��ͬ��ķ�������,���������ε����������γɵ�����ƽ�淽��
	glk::Plane          _yABEquation,_yBCEquation,_yCAEquation;
	//��������ֱ�������εı�������ķ����еķ������γɵ�ƽ�棬�Ҿ��������ε�ÿһ�������γɵ�ƽ��
	glk::Plane          _zABEquation,_zBCEquation,_zCAEquation;
private:
	/*
	  *����һ�������Ƿ��������������εķ���Ϊ�����γɵ�����������
	  *�˺����������ټ��㶥���Ƿ�λ������ƽ��֮��
	 */
	bool      isInside(const glk::GLVector3 &point)const;
public:
	Triangle();
	Triangle(const glk::GLVector3 &vertexA,const glk::GLVector3 &vertexB,const glk::GLVector3 &vertexC);
	void     init(const glk::GLVector3 &vertexA,const glk::GLVector3 &vertexB,const glk::GLVector3 &vertexC);
	  ///////////////*����ĺ���Ϊ�ռ伸���ཻ�Բ���*////////////////////////////
	/*
	  *�ռ�ֱ�߲���,�Ƿ�������ƽ���ཻ
	  *��������������ĺܶຯ���Ļ���
	 */
	bool     rayIntersectTriangle(const glk::Ray &ray,float &minDistance,glk::GLVector3 &intersectPoint);
	/*
	  *���۾���ƽ�����Ϸ���ʱ�����۾����ŷ���direction�Ƿ�������ƽ���ཻ
	  *eyeKneeDistanceΪ�۾���ϥ�ǵľ���
	*/
	bool    rayIntersectUnder(const glk::GLVector3 &eyePosition,float eyeKneeDistance,float &minDistance,float &height);
	/*
	  *���۾�������ƽ����·���ʱ��,�۾�����+Y�᷽�����Ͽ�,�Ƿ�������ƽ���ཻ,����ཻ,�����̾���
	 */
	bool   rayIntersectAbove(const glk::GLVector3 &eyePosition,float &minDistance,float &height);
	/*
	  *�������,���������۾��ľ����Ƿ�λ��ĳ�������ε����Ϸ�
	  *eyePosition:�۾��ľ���
	  *eyeKneeDistance:�۾���ϥ�ǵľ���
	  *closetDistance:�ڽӾ���,
	  *minDistance:�۾����ܴﵽ����̾���
	  *compensation:����ƫ����
	 */
	bool   rayDistanceTest(const glk::GLVector3 &eyePosition,float eyeKneeDistance,float closetDistance,float &minDistance,glk::GLVector3 &compensation);
};
#endif