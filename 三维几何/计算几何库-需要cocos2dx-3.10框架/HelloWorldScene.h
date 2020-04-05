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
	//测试旋转卡壳算法
	void  testRotateHullAlorithm();
	//测试离散点集的包围园的普通算法
	void  testMinimumCycleNormal();
	//测试圆的计算函数
	void  testCycleCreation();
	//测试最小包围球的算法
	void  testMinimumSphere();
	//平面任意四点,测试最小包围圆算法
	void  testMinimumSphere4Plane();
	//OBB相交测试
	void  testOBBIntersect();
	//直线之/线段之间的最近距离测试
	void testSegmentDistance();
	//点到平面的最近距离
	void pointToPlane();
	//点到线段的最近距离
	void pointToSegment();
	//点到AABB的最近距离
	void pointToAABB();
	//点到OBB最近距离
	void pointToOBB();
	//点到3d矩形的最近点,该算法有两种实现
	void pointToRectangle();
	//点到三角形的最近距离
	void pointToTriangle();
	//点到四面体的最近距离
	void pointToTetrahedron();
	//线段到三角形之间的最近距离
	void segmentToTriangle();
	//AABB/OBB与平面是否相交
	void doAABBAndOBBIntersectWithPlane();
	//锥形体与平面的相交测试
	void doConeIntersectWithPlane();
	//AABB/OBB与球体的相交测试
	void doAABBAndOBBIntersectWithSphere();
	//三角形与球体的相交测试
	void doTriangleIntersectWithSphere();
	//AABB/OBB与三角形之间的相交测试
	void doAABBAndOBBIntersectWithTriangle();
	//三角形与三角形之间的相交测试
	void doTriangleAndTriangleInttersect();
	//光线/线段与球体的相交测试
	void doRayAndSegmentIntersectWithSphere();
	//光线与AABB的相交测试
	void doRayIntersectWithAABB();
	//光线与OBB的相交测试
	void doRayIntersectWithOBB();
	//线段与AABB/OBB之间的相交测试
	void doAABBAndOBBIntersectWithSegment();
	//光线与三角形相交测试
	void doLineIntersectWithTriangle();
	//线段与三角形之间的相交测试
	void doSegmentIntersectWithTriangle();
	//光线与圆柱体的相交测试
	void doCylinderIntersectWithLineAndSegment();
	//光线/射线/线段与多边形的相交测试
	void doLineRaySegementIntersectWithPolygon();
	//多边形与点的包含关系
	void doPolygonContainsPoint();
	//多边形与圆的相交测试
	void doPolygonCycleIntersectTest();
	//两平面的相交测试
	void doPlaneIntersectWithPlane();
	//三个平面之间的相交测试
	void doPlane3Intersecttest();
	//2D空间网格划分与有向矩形的快速求交
	void do2DSpacePartionAndORectIntersect();
	//一条射线与2d空间网格的相交测试
	//void do2DSpacePartionAndORectIntersect2();
	//Bresenham直线算法
	void bresenhamAlgorithm();
	//N条线段相交测试
	void segmentNIntersectTest();
	//N个离散点的最远距离算法测试
	void dispersePointsMaxDistanceTest();
	//N个离散点的最近距离测试,该测试函数不用使用gui显示
	void dispersePointsMinDistanceTest();
	//离散点集的最近距离测试,3d算法
	void dispersePoints3DMinDistanceTest();
	//GJK算法测试
	void gjkAlgorithmTest();
	//quick-hull算法测试
	void quickHullAlgorithmTest();
	//快速线段求交测试
	void  quickSegmentIntersectTest();
	//平衡树/红黑树测试
	void testRedBlackTree();
	//简单多边形的y单调分解
	void simplePolygonYDecompose();
	//二维线性规划测试
	void twoDimensionLinearlyProgram();
	//计算两个凸多边形的公切线
	void twoPolygonTangentLine();
	//计算离散点集的三角剖分算法测试
	void delaunayTriangulate();
	//计算离散点集的Voronoi子区域划分
	void voronoiAreaPartion();
	//计算离散点集的Voronoi区域划分
	void voronoiAreaPartion2();
	//半平面求教算法实现
	void halfPlaneIntersectTest();
	//旋转卡壳算法-->凸包直径算法测试
	void rotateHullMaxDistanceTest();
	//旋转卡壳算法->求凸多边形的宽度
	void rotateHullWidthTest();
	//旋转卡壳算法->凸多边形之间的最大距离
	void rotateHullTwoMaxDistanceTest();
	//旋转卡壳算法->求两分离的凸多边形的最短距离
	void rotateHullTwoMinDistanceTest();
	//旋转卡壳算法->求多边形的最小面积外接矩形
	void rotateHullMinAreaTest();
	//旋转卡壳算法->求多边形的最小周长外接矩形
	void rotateHullMinPerimeterTest();
	//旋转卡壳算法->求离散点集的洋葱三角分解
	void rotateHullOnionDecomposition();
	//旋转卡壳算法->求离散点集的螺旋三角剖分
	void rotateHullSpiralDecomposition();
	//旋转卡壳算法->合并两个凸多边形
	void rotateHullPolygonUnion();
	//旋转卡壳算法->求两凸多边形的交
	void rotateHullPolygonIntersect();
	//旋转卡壳算法->求两个凸多边形的内公切线
	void rotateHullPolygonInnerTangent();
	//旋转卡壳算法->求凸多边形的Minkowski和
	void rotateHullPolygonMinkowski();
	//最佳横截带算法实现
	void rotateHullPolygonsNarrowSurface();
	//3d凸壳quicck-hull算法
	void convexHull3dAlgorithm();
	//优先级队列算法测试
	void priorityQueueTest();
	//3d凸克算法--随机算法测试
	void convexHull3dRandom();
	//测试共享内存分配器版平衡树
	void balanceTreeMemSlab();
	//简单多边形求交
	void simplePolygonIntersect();
	//简单多边形与点之间的包含测试
	void simplePolygonContainsPoint();
	//二维线性规划算法
	void linearProgram2d();
	//一般线性规划测试
	void linearProgramTest();
};

#endif // __HELLOWORLD_SCENE_H__
