#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
extern const short s_CameraMask;
class HelloWorld : public cocos2d::Layer
{
	cocos2d::Camera  *_flyCamera;
	cocos2d::Vec2       _startPoint;
	int                            _keyMask;
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);

	bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event);

	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
	void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode,cocos2d::Event* event);

	void updateCamera(float dt);
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);
	//������ת�����㷨
	void  testRotateHullAlorithm();
	//������ɢ�㼯�İ�Χ԰����ͨ�㷨
	void  testMinimumCycleNormal();
	//����Բ�ļ��㺯��
	void  testCycleCreation();
	//������С��Χ����㷨
	void  testMinimumSphere();
	//ƽ�������ĵ�,������С��ΧԲ�㷨
	void  testMinimumSphere4Plane();
	//OBB�ཻ����
	void  testOBBIntersect();
	//ֱ��֮/�߶�֮�������������
	void testSegmentDistance();
	//�㵽ƽ����������
	void pointToPlane();
	//�㵽�߶ε��������
	void pointToSegment();
	//�㵽AABB���������
	void pointToAABB();
	//�㵽OBB�������
	void pointToOBB();
	//�㵽3d���ε������,���㷨������ʵ��
	void pointToRectangle();
	//�㵽�����ε��������
	void pointToTriangle();
	//�㵽��������������
	void pointToTetrahedron();
	//�߶ε�������֮����������
	void segmentToTriangle();
	//AABB/OBB��ƽ���Ƿ��ཻ
	void doAABBAndOBBIntersectWithPlane();
	//׶������ƽ����ཻ����
	void doConeIntersectWithPlane();
	//AABB/OBB��������ཻ����
	void doAABBAndOBBIntersectWithSphere();
	//��������������ཻ����
	void doTriangleIntersectWithSphere();
	//AABB/OBB��������֮����ཻ����
	void doAABBAndOBBIntersectWithTriangle();
	//��������������֮����ཻ����
	void doTriangleAndTriangleInttersect();
	//����/�߶���������ཻ����
	void doRayAndSegmentIntersectWithSphere();
	//������AABB���ཻ����
	void doRayIntersectWithAABB();
	//������OBB���ཻ����
	void doRayIntersectWithOBB();
	//�߶���AABB/OBB֮����ཻ����
	void doAABBAndOBBIntersectWithSegment();
	//�������������ཻ����
	void doLineIntersectWithTriangle();
	//�߶���������֮����ཻ����
	void doSegmentIntersectWithTriangle();
	//������Բ������ཻ����
	void doCylinderIntersectWithLineAndSegment();
	//����/����/�߶������ε��ཻ����
	void doLineRaySegementIntersectWithPolygon();
	//��������İ�����ϵ
	void doPolygonContainsPoint();
	//�������Բ���ཻ����
	void doPolygonCycleIntersectTest();
	//��ƽ����ཻ����
	void doPlaneIntersectWithPlane();
	//����ƽ��֮����ཻ����
	void doPlane3Intersecttest();
	//2D�ռ����񻮷���������εĿ�����
	void do2DSpacePartionAndORectIntersect();
	//һ��������2d�ռ�������ཻ����
	//void do2DSpacePartionAndORectIntersect2();
	//Bresenhamֱ���㷨
	void bresenhamAlgorithm();
	//N���߶��ཻ����
	void segmentNIntersectTest();
	//N����ɢ�����Զ�����㷨����
	void dispersePointsMaxDistanceTest();
	//N����ɢ�������������,�ò��Ժ�������ʹ��gui��ʾ
	void dispersePointsMinDistanceTest();
	//��ɢ�㼯������������,3d�㷨
	void dispersePoints3DMinDistanceTest();
	//GJK�㷨����
	void gjkAlgorithmTest();
	//quick-hull�㷨����
	void quickHullAlgorithmTest();
	//�����߶��󽻲���
	void  quickSegmentIntersectTest();
	//ƽ����/���������
	void testRedBlackTree();
	//�򵥶���ε�y�����ֽ�
	void simplePolygonYDecompose();
	//��ά���Թ滮����
	void twoDimensionLinearlyProgram();
	//��������͹����εĹ�����
	void twoPolygonTangentLine();
	//������ɢ�㼯�������ʷ��㷨����
	void delaunayTriangulate();
	//������ɢ�㼯��Voronoi�����򻮷�
	void voronoiAreaPartion();
	//������ɢ�㼯��Voronoi���򻮷�
	void voronoiAreaPartion2();
	//��ƽ������㷨ʵ��
	void halfPlaneIntersectTest();
	//��ת�����㷨-->͹��ֱ���㷨����
	void rotateHullMaxDistanceTest();
	//��ת�����㷨->��͹����εĿ��
	void rotateHullWidthTest();
	//��ת�����㷨->͹�����֮���������
	void rotateHullTwoMaxDistanceTest();
	//��ת�����㷨->���������͹����ε���̾���
	void rotateHullTwoMinDistanceTest();
	//��ת�����㷨->�����ε���С�����Ӿ���
	void rotateHullMinAreaTest();
	//��ת�����㷨->�����ε���С�ܳ���Ӿ���
	void rotateHullMinPerimeterTest();
	//��ת�����㷨->����ɢ�㼯��������Ƿֽ�
	void rotateHullOnionDecomposition();
	//��ת�����㷨->����ɢ�㼯�����������ʷ�
	void rotateHullSpiralDecomposition();
	//��ת�����㷨->�ϲ�����͹�����
	void rotateHullPolygonUnion();
	//��ת�����㷨->����͹����εĽ�
	void rotateHullPolygonIntersect();
	//��ת�����㷨->������͹����ε��ڹ�����
	void rotateHullPolygonInnerTangent();
	//��ת�����㷨->��͹����ε�Minkowski��
	void rotateHullPolygonMinkowski();
	//��Ѻ�ش��㷨ʵ��
	void rotateHullPolygonsNarrowSurface();
	//3d͹��quicck-hull�㷨
	void convexHull3dAlgorithm();
	//���ȼ������㷨����
	void priorityQueueTest();
	//3d͹���㷨--����㷨����
	void convexHull3dRandom();
	//���Թ����ڴ��������ƽ����
	void balanceTreeMemSlab();
	//�򵥶������
	void simplePolygonIntersect();
	//�򵥶�������֮��İ�������
	void simplePolygonContainsPoint();
	//��ά���Թ滮�㷨
	void linearProgram2d();
	//һ�����Թ滮����
	void linearProgramTest();
};

#endif // __HELLOWORLD_SCENE_H__
