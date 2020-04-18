#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "geometry.h"
#include "DrawNode3D.h"
#include "data_struct/balance_tree.h"
#include "data_struct/priority_queue.h"

USING_NS_CC;

const short s_CameraMask = (short)CameraFlag::USER2;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
	Layer::init();
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto &winSize = _director->getWinSize();
	int64_t seed = time(nullptr);// 1585913882;// time(nullptr); ;// 1585913525;
	CCLOG("seed->%ld", seed);
	srand(seed);//14,23,27

	_flyCamera = Camera::createPerspective(60.0f, winSize.width/ winSize.height,200.0f,20000.0f);
	_flyCamera->setPosition3D(Vec3(0,0,winSize.height/1.1566f));
	_flyCamera->lookAt(Vec3::ZERO);
	_flyCamera->setCameraFlag(CameraFlag::USER2);
	this->addChild(_flyCamera);

	EventListenerTouchOneByOne *touch_event = EventListenerTouchOneByOne::create();
	touch_event->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
	touch_event->onTouchMoved = CC_CALLBACK_2(HelloWorld::onTouchMoved,this);
	touch_event->onTouchEnded = CC_CALLBACK_2(HelloWorld::onTouchEnded,this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touch_event, this);

	_keyMask = 0;
	EventListenerKeyboard *key_event = EventListenerKeyboard::create();
	key_event->onKeyPressed = CC_CALLBACK_2(HelloWorld::onKeyPressed,this);
	key_event->onKeyReleased = CC_CALLBACK_2(HelloWorld::onKeyReleased,this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(key_event, this);

	//testRotateHullAlorithm();
	//testMinimumCycleNormal();
	//testCycleCreation();
	//testMinimumSphere();
	//testMinimumSphere4Plane();

	//testOBBIntersect();
	//testSegmentDistance();
	//pointToPlane();
	//pointToSegment();
	//pointToAABB();
	//pointToOBB();
	//pointToRectangle();
	//pointToTriangle();
	//pointToTetrahedron();
	//segmentToTriangle();
	//doAABBAndOBBIntersectWithPlane();
	//doConeIntersectWithPlane();
	//doAABBAndOBBIntersectWithSphere();
	//doTriangleIntersectWithSphere();
	//doAABBAndOBBIntersectWithTriangle();
	//doTriangleAndTriangleInttersect();
	//doRayAndSegmentIntersectWithSphere();
	//doRayIntersectWithAABB();
	//doRayIntersectWithOBB();
	//doAABBAndOBBIntersectWithSegment();
	//doLineIntersectWithTriangle();
	//doSegmentIntersectWithTriangle();
	//doCylinderIntersectWithLineAndSegment();
	//doLineRaySegementIntersectWithPolygon();
	//doPolygonContainsPoint();
	//doPolygonCycleIntersectTest();
	//doPlaneIntersectWithPlane();
	//doPlane3Intersecttest();
	//do2DSpacePartionAndORectIntersect();
	//do2DSpacePartionAndORectIntersect2();
	//bresenhamAlgorithm();
	//segmentNIntersectTest();
	//dispersePointsMaxDistanceTest();
	//dispersePointsMinDistanceTest();
	//dispersePoints3DMinDistanceTest();
	//gjkAlgorithmTest();
	//quickHullAlgorithmTest();
	//quickSegmentIntersectTest();
	//testRedBlackTree();
	//simplePolygonYDecompose();
	//twoDimensionLinearlyProgram();
	//twoPolygonTangentLine();
	//delaunayTriangulate();
	//voronoiAreaPartion();
	//voronoiAreaPartion2();
	//halfPlaneIntersectTest();
	//rotateHullMaxDistanceTest();
	//rotateHullWidthTest();
	//rotateHullTwoMaxDistanceTest();
	//rotateHullTwoMinDistanceTest();
	//rotateHullMinAreaTest();
	//rotateHullMinPerimeterTest();
	//rotateHullOnionDecomposition();
	//rotateHullSpiralDecomposition();
	//rotateHullPolygonUnion();
	//rotateHullPolygonIntersect();
	//rotateHullPolygonInnerTangent();
	//rotateHullPolygonMinkowski();
	//rotateHullPolygonsNarrowSurface();
	//convexHull3dAlgorithm();
	//priorityQueueTest();
	//convexHull3dRandom();
	//balanceTreeMemSlab();
	//simplePolygonIntersect();
	//simplePolygonContainsPoint();
	//linearProgram2d();
	//linearProgramTest();//1585913882/1586053580;// 
	frustumClippingTest();

	schedule(schedule_selector(HelloWorld::updateCamera));
    return true;
}

void  HelloWorld::updateCamera(float dt)
{
	if (_keyMask)
	{
		Vec3 move_step;
		//获取摄像机的旋转矩阵
		const Vec3 rotate = _flyCamera->getRotation3D();
		const  Quaternion quaternion = _flyCamera->getRotationQuat();
		Mat4  matrix_rotate;
		Mat4::createRotation(quaternion, &matrix_rotate);
		//提取方向向量
		Vec3  direction_x(matrix_rotate.m[0], matrix_rotate.m[1], matrix_rotate.m[2]);
		Vec3  direction_z(matrix_rotate.m[8], matrix_rotate.m[9], matrix_rotate.m[10]);
		Vec3  direction_y(matrix_rotate.m[4], matrix_rotate.m[5], matrix_rotate.m[6]);
		float speed = 6;
		//AD
		if (_keyMask & 0x1)
			move_step -= direction_z * speed;
		if (_keyMask & 0x2)
			move_step += direction_z * speed;

		//WS
		if (_keyMask & 0x4)
			move_step -= direction_x * speed;
		if (_keyMask & 0x8)
			move_step += direction_x * speed;
		//EX
		if (_keyMask & 0x10)
			move_step += direction_y * speed;
		if (_keyMask & 0x20)
			move_step -= direction_y * speed;

		Vec3 position = _flyCamera->getPosition3D();
		_flyCamera->setPosition3D(move_step + position);
	}
}

bool HelloWorld::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event)
{
	_startPoint = touch->getLocation();
	return true;
}

void  HelloWorld::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event)
{
	Vec2 touch_point = touch->getLocation();
	Vec3 rotation = _flyCamera->getRotation3D();
	float  offset_x = touch_point.x - _startPoint.x;
	float  offset_y = touch_point.y - _startPoint.y;
	rotation.x -= 0.25f * offset_y / _director->getWinSize().height * 180.0f;
	rotation.y += 0.25f * offset_x / _director->getWinSize().width * 180.0f;
	_flyCamera->setRotation3D(rotation);

	_startPoint = touch_point;
}

void HelloWorld::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event)
{
}

void HelloWorld::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, Event *event)
{
	if (keyCode == EventKeyboard::KeyCode::KEY_W)
		_keyMask |= 0x1;
	else if (keyCode == EventKeyboard::KeyCode::KEY_S)
		_keyMask |= 0x2;
	else if (keyCode == EventKeyboard::KeyCode::KEY_A)
		_keyMask |= 0x4;
	else if (keyCode == EventKeyboard::KeyCode::KEY_D)
		_keyMask |= 0x8;
	else if (keyCode == EventKeyboard::KeyCode::KEY_E)
		_keyMask |= 0x10;
	else if (keyCode == EventKeyboard::KeyCode::KEY_X)
		_keyMask |= 0x20;
}

void  HelloWorld::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
	if (keyCode == EventKeyboard::KeyCode::KEY_W)
		_keyMask &= ~0x1;
	else if (keyCode == EventKeyboard::KeyCode::KEY_S)
		_keyMask &= ~0x2;
	else if (keyCode == EventKeyboard::KeyCode::KEY_A)
		_keyMask &= ~0x4;
	else if (keyCode == EventKeyboard::KeyCode::KEY_D)
		_keyMask &= ~0x8;
	else if (keyCode == EventKeyboard::KeyCode::KEY_E)
		_keyMask &= ~0x10;
	else if (keyCode == EventKeyboard::KeyCode::KEY_X)
		_keyMask &= ~0x20;
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/
    
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void HelloWorld::testRedBlackTree()
{
	std::function<int(const int&a, const int &b)> compare_func = [](const int &a,const int &b)->int {
		return a < b;
	};
	gt::red_black_tree<int>   rb_tree(32);
	int l_other = 0;
	for (int index_l = 0; index_l < 128; ++index_l)
		rb_tree.insert(index_l, compare_func);
	//输出
	auto search_node = rb_tree.lookup(0, compare_func);
	assert(search_node!=nullptr);
	int  target = -1;
	while (search_node)
	{
		assert(target < search_node->tw_value);
		target = search_node->tw_value;
		search_node = rb_tree.find_next(search_node);
	}
	//删除
	int start_j = gt::random() * 64;
	int final_j = start_j + 12;
	for (; start_j < final_j; ++start_j)
		rb_tree.remove(start_j, compare_func);

	search_node = rb_tree.find_minimum();
	assert(search_node != nullptr);
	target = -1;
	while (search_node)
	{
		assert(target < search_node->tw_value);
		target = search_node->tw_value;
		search_node = rb_tree.find_next(search_node);
	}
}

void HelloWorld::twoPolygonTangentLine()
{
	const int point_num = 67;
	//随机生成离散的点
	std::vector<Vec2>   points,points2;
	points.reserve(point_num);
	points2.reserve(point_num);

	auto &winSize = _director->getWinSize();
	float cx = winSize.width * 0.5f;
	float cy = winSize.height * 0.5f;

	float w = 300.0f;
	float h = 350.0f;

	Node  *root_node = Node::create();
	root_node->setPosition(cx, cy);
	this->addChild(root_node);

	for (int index_j = 0; index_j < point_num; ++index_j)
	{
		points.push_back(Vec2(w * (2.0f * rand() / RAND_MAX - 1.0f), h * (2.0f * rand() / RAND_MAX - 1.0f)));
		points2.push_back(Vec2(w * (2.0f * rand() / RAND_MAX - 1.0f), h * (2.0f * rand() / RAND_MAX - 1.0f)));
	}
	//计算两多边形的间距
	int target_1 = 0, target_2 = 0;
	for (int index_l = 1; index_l < point_num; ++index_l)
	{
		if (points[index_l].x > points[target_1].x)
			target_1 = index_l;
		if (points2[index_l].x < points2[target_2].x)
			target_2 = index_l;
	}
	int distance_x = (points2[target_2].x - points[target_1].x) * 0.6;
	if (distance_x < 0)
	{
		for (int index_l = 0; index_l < point_num; ++index_l)
		{
			points[index_l].x += distance_x;
			points2[index_l].x -= distance_x;
		}
	}

	std::vector<Vec2> polygon_vector,polygon_vector2;
	bool check_polygon = gt::polygon_compute_convex_hull(points, polygon_vector);
	gt::polygon_compute_convex_hull(points2, polygon_vector2);
	//建立sprite
	for (int index_j = 0; index_j < point_num; ++index_j)
	{
		Sprite *bubble_sprite = Sprite::create("llk_yd.png");
		bubble_sprite->setPosition(points[index_j]);
		root_node->addChild(bubble_sprite);
	}
	//画线
	DrawNode  *draw_polygon = DrawNode::create();
	Color4F color(1.0f, 1.0f, 1.0f, 1.0f);
	for (int index_l = 1; index_l < polygon_vector.size(); ++index_l)
	{
		draw_polygon->drawLine(polygon_vector[index_l - 1], polygon_vector[index_l], color);
		Sprite *bubble_sprite = Sprite::create("llk_yd.png");
		bubble_sprite->setPosition(polygon_vector[index_l]);
		root_node->addChild(bubble_sprite);
	}
	draw_polygon->drawLine(polygon_vector.back(), polygon_vector.front(), color);

	for (int index_l = 1; index_l < polygon_vector2.size(); ++index_l)
	{
		draw_polygon->drawLine(polygon_vector2[index_l - 1], polygon_vector2[index_l], color);
		Sprite *bubble_sprite = Sprite::create("llk_yd.png");
		bubble_sprite->setPosition(polygon_vector2[index_l]);
		root_node->addChild(bubble_sprite);
	}
	draw_polygon->drawLine(polygon_vector2.back(), polygon_vector2.front(), color);
	//计算两者的公切线
	int tangent_index_array[4];
	gt::polygon_polygon_tangent_line(polygon_vector, polygon_vector2, tangent_index_array);
	draw_polygon->drawLine(polygon_vector[tangent_index_array[0]], polygon_vector2[tangent_index_array[1]], Color4F::RED);
	draw_polygon->drawLine(polygon_vector[tangent_index_array[2]], polygon_vector2[tangent_index_array[3]], Color4F::YELLOW);

	root_node->addChild(draw_polygon);
}

void HelloWorld::delaunayTriangulate()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 700;
	const int array_size = 512;
	//三角形平面
	std::vector<Vec2>  points(array_size + 3);
	std::vector<Sprite*>  sprite_array[array_size];
	//生成随机离散点,并计算boundingbox
	Vec2 origin(FLT_MAX, FLT_MAX), bottom(-FLT_MAX,-FLT_MAX);
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		origin.x = fminf(origin.x,points[index_j].x);
		origin.y = fminf(origin.y,points[index_j].y);

		bottom.x = fmaxf(bottom.x,points[index_j].x);
		bottom.y = fmaxf(bottom.y,points[index_j].y);

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_j]);
		root_node->addChild(sprite);
		sprite_array[index_j];
	}
	//Cycle
	//gt::Cycle cycle;
	//gt::DelaunayTriangle delaunay_triangle = {short(0),short(1),short(2)};
	//static_create_cycle_by_triangle(cycle, points,delaunay_triangle);
	//draw_node->drawCircle(cycle.center,cycle.radius, 360, 2.0f * M_PI * cycle.radius / 4.0f, false, 1.0f,1.0f, Color4F::GREEN);
	//计算离散点集的boundingbox的外接三角形
	Vec2 triangle[3];

	gt::rect_outerline_triangle(origin, bottom - origin, triangle);
	//draw_node->drawLines(triangle, 3, true, Color4F::BLUE);

	points[array_size] = triangle[0];
	points[array_size + 1] = triangle[1];
	points[array_size + 2] = triangle[2];

	std::vector<gt::DelaunayTriangle>   delaunay_trianges;
	int real_size = 0;
	timeval   time_val1, time_val2;
	gettimeofday(&time_val1,nullptr);
	//256==>cost time---->72.971
	//512==>cost time---->160.276
	//gt::delaunay_triangulate_bowyer_washton(points, delaunay_trianges, real_size);//

	//256==>cost time---->231.057==>cost time---->147.405
	//512==>cost time---->311.903
	//512==>cost time---->224.056
	//512==>cost time---->223.063
	gt::delaunay_triangulate_random(points, delaunay_trianges, real_size);//
	gettimeofday(&time_val2,nullptr);
	CCLOG("cost time---->%.3f",(time_val2.tv_sec - time_val1.tv_sec) * 1000.0f + (time_val2.tv_usec - time_val1.tv_usec )/1000.0f);

	for (int index_l = 0; index_l < real_size; ++index_l)
	{
		auto &delaunay = delaunay_trianges[index_l];
		Color4F color(gt::random(),gt::random(),gt::random(),1.0f);
		draw_node->drawLine(points[delaunay.v1],points[delaunay.v2],color);
		draw_node->drawLine(points[delaunay.v2], points[delaunay.v3], color);
		draw_node->drawLine(points[delaunay.v3], points[delaunay.v1], color);


		//gt::Cycle cycle;
		//gt::cycle_create(points[delaunay.v1], points[delaunay.v2], points[delaunay.v3], cycle);
		//draw_node->drawCircle(cycle.center, cycle.radius,360.0f,2.0f * M_PI * cycle.radius/4.0f,false,color);
	}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::voronoiAreaPartion()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 700;
	const int array_size = 127;
	//三角形平面
	std::vector<Vec2>  points(array_size + 3);
	std::vector<Sprite*>  sprite_array[array_size];
	//生成随机离散点,并计算boundingbox
	Vec2 origin(FLT_MAX, FLT_MAX), bottom(-FLT_MAX, -FLT_MAX);
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		origin.x = fminf(origin.x, points[index_j].x);
		origin.y = fminf(origin.y, points[index_j].y);

		bottom.x = fmaxf(bottom.x, points[index_j].x);
		bottom.y = fmaxf(bottom.y, points[index_j].y);

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_j]);
		root_node->addChild(sprite);
		sprite_array[index_j];
	}
	//计算离散点集的boundingbox的外接三角形
	Vec2 triangle[3];

	gt::rect_outerline_triangle(origin, bottom - origin, triangle);

	points[array_size] = triangle[0];
	points[array_size + 1] = triangle[1];
	points[array_size + 2] = triangle[2];

	std::vector<Vec2>   edge_points;
	std::vector<int>      ray_edge_array, normal_edge_array;
	gt::voronoi_delaunay_triangle(points, edge_points, normal_edge_array, ray_edge_array);//

	for (int index_l = 0; index_l < normal_edge_array.size(); index_l += 2)
	{
		Color4F color(gt::random(), gt::random(), gt::random(), 1.0f);
		draw_node->drawLine(edge_points[normal_edge_array[index_l]], edge_points[normal_edge_array[index_l+1]], color);
	}
	for (int index_l = 0; index_l < ray_edge_array.size(); index_l +=2)
	{
		Vec2 &base_point = edge_points[ray_edge_array[index_l]];
		draw_node->drawLine(base_point, base_point + edge_points[ray_edge_array[index_l +1]] * 400,Color4F::RED);
	}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::voronoiAreaPartion2()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 700;
	const int array_size = 127;
	//三角形平面
	std::vector<Vec2>  points(array_size + 2);
	std::vector<Sprite*>  sprite_array[array_size];
	//生成随机离散点,并计算boundingbox
	Vec2 origin(FLT_MAX, FLT_MAX), bottom(-FLT_MAX, -FLT_MAX);
	float boundary = 8000.0f;
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		origin.x = fminf(origin.x, points[index_j].x - boundary);
		origin.y = fminf(origin.y, points[index_j].y - boundary);

		bottom.x = fmaxf(bottom.x, points[index_j].x + boundary);
		bottom.y = fmaxf(bottom.y, points[index_j].y + boundary);

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_j]);
		root_node->addChild(sprite);
		sprite_array[index_j];
	}
	//计算离散点集的boundingbox的外接四边形
	points[array_size] = origin;
	points[array_size + 1] = bottom;

	std::vector<gt::VoronoiSite>   sites_array;
	gt::voronoi_increament_policy(points, sites_array);
	//针对每一个Voronoi单元,遍历
	//srand(19);
	int link_size = 0;
	for (int index_l = 0; index_l < sites_array.size(); ++index_l)
	{
		Color4F color(gt::random(), gt::random(), gt::random(), 1.0f);
		gt::VoronoiSite &site = sites_array[index_l];
		gt::VoronoiEdge *head = site.head;
		do
		{
			draw_node->drawLine(head->origin, head->bottom, color);
			head = head->next;
			link_size += 1;
		} while (head != site.head);
	}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::rotateHullPolygonMinkowski()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 400.0f;
	float length_w = 300;
	const int array_size = 33;
	std::vector<Vec2>  points(array_size);
	//生成随机离散点,并计算boundingbox
	Vec2 origin(FLT_MAX, FLT_MAX), bottom(-FLT_MAX, -FLT_MAX);
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_j]);
		root_node->addChild(sprite);
		bottom.x = fmaxf(points[index_j].x, bottom.x);
	}

	std::vector<Vec2>  points2(array_size);
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points2[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points2[index_j]);
		root_node->addChild(sprite);
	}
	//求离散点集的凸包
	std::vector<Vec2> polygon;
	gt::polygon_compute_convex_hull(points, polygon);
	//画出多边形的边界
	for (int index_l = 0; index_l < polygon.size(); ++index_l)
	{
		int secondary_l = (index_l + 1) % polygon.size();
		draw_node->drawLine(polygon[index_l], polygon[secondary_l], Color4F::GREEN);
	}

	std::vector<Vec2> polygon2;
	gt::polygon_compute_convex_hull(points2, polygon2);
	for (int index_l = 0; index_l < polygon2.size(); ++index_l)
	{
		int secondary_l = (index_l + 1) % polygon2.size();
		draw_node->drawLine(polygon2[index_l], polygon2[secondary_l], Color4F::GREEN);
	}

	std::vector<Vec2> polygon_mink;
	gt::rotate_hull_polygon_minkowski(polygon, polygon2, polygon_mink);

	int polygon_size = polygon_mink.size();
	for (int index_l = 0; index_l < polygon_mink.size(); ++index_l)
	{
		draw_node->drawLine(polygon_mink[index_l], polygon_mink[index_l + 1 >= polygon_size ? 0 : index_l + 1], Color4F::RED);
	}

	std::vector<Vec2> polygon_mink2;
	gt::rotate_hull_polygon_minkowski_prim(polygon, polygon2, polygon_mink2);
	int polygon_size2 = polygon_mink2.size();
	for (int index_l = 0; index_l < polygon_mink2.size(); ++index_l)
	{
		draw_node->drawLine(polygon_mink2[index_l], polygon_mink2[index_l + 1 >= polygon_size2 ? 0 : index_l + 1], Color4F::BLUE);
	}
	assert(polygon_size == polygon_size2);
	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::rotateHullPolygonsNarrowSurface()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 1334.0f/6;
	float length_w = 750.0f/6.0f;
	const int array_size = 33;
	const Size &winSize = _director->getWinSize();

	std::vector<Vec2>  points_array[6];
	//网格划分
	//生成随机离散点,并计算boundingbox
	for (int base_j = 0; base_j < 6; ++base_j)
	{
		Vec2 base_location(1334 * gt::random(), 750 * gt::random());
		std::vector<Vec2> &points = points_array[base_j];
		points.resize(array_size);
		for (int index_j = 0; index_j < array_size; ++index_j)
		{
			points[index_j] = base_location + Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());
		}
	}
	//求离散点集的凸包
	std::vector<Vec2> polygons[6];
	for (int base_j = 0; base_j < 6; ++base_j)
	{
		gt::polygon_compute_convex_hull(points_array[base_j], polygons[base_j]);
	}
	//画出多边形的边界
	for (int base_j = 0; base_j < 6; ++base_j)
	{
		const std::vector<Vec2> &polygon = polygons[base_j];
		for (int index_l = 0; index_l < polygon.size(); ++index_l)
		{
			int secondary_l = (index_l + 1) % polygon.size();
			draw_node->drawLine(polygon[index_l], polygon[secondary_l], Color4F::GREEN);
		}
	}
	//最佳横截带
	Vec2 surface[4];
	float distance = gt::rotate_hull_polygons_narrow_surface(polygons, 6, surface);
	//画出两条横截带
	draw_node->drawLine(surface[0] - surface[1] * 600, surface[0] + surface[1] * 600,Color4F::RED);
	draw_node->drawLine(surface[0], surface[0] + Vec2(-surface[1].y, surface[1].x) * 200.0f,Color4F::BLUE);

	draw_node->drawLine(surface[2] - surface[3] * 600, surface[2] + surface[3] * 600.0f,Color4F::RED);
	draw_node->drawLine(surface[2], surface[2] + Vec2(-surface[3].y, surface[3].x) * 200.0f,Color4F::BLUE);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::convexHull3dAlgorithm()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 750.0f;
	float length_w = 650.0f;
	float length_d = 650.0f;

	const int array_size = 128;
	//三角形平面
	std::vector<cocos2d::Vec3> points(array_size);

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_w * gt::randomf10(), length_d * gt::randomf10());
		Sprite *s = Sprite::create("llk_yd.png");
		s->setPosition3D(points[index_j]);
		root_node->addChild(s);
	}

	timeval  tiv1, tiv2;
	gettimeofday(&tiv1,nullptr);
	std::vector<gt::Plane3 *>  planes;
	bool b = gt::quick_hull_algorithm3d(points, planes);
	gettimeofday(&tiv2, nullptr);

	CCLOG("algorithm cost time:%.2f\n",(tiv2.tv_sec - tiv1.tv_sec) * 1000.0f + (tiv2.tv_usec - tiv1.tv_usec)/1000.0f);
	//实验证明,经过优化后的算法的运行时间明显的缩短了
	//static_create_tetrahedron(points, planes);
	for (auto it= planes.begin(); it != planes.end(); ++it)
	{
		gt::Plane3 *plane = *it;
		Color4F color(gt::random(),gt::random(),gt::random(),1.0f);

		draw_node->drawLine(points[plane->v1],points[plane->v2], color);
		draw_node->drawLine(points[plane->v2], points[plane->v3], color);
		draw_node->drawLine(points[plane->v3], points[plane->v1], color);

		//画出法线
		//const Vec3 normal = gt::cross_normalize(points[plane->v1],points[plane->v2],points[plane->v3]);
		//draw_node->drawLine(points[plane->v1],points[plane->v1] + normal * 400.0f, color);

		delete plane;
	}
	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::convexHull3dRandom()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 750.0f;
	float length_w = 650.0f;
	float length_d = 650.0f;

	const int array_size = 128;
	//三角形平面
	std::vector<cocos2d::Vec3> points(array_size);

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_w * gt::randomf10(), length_d * gt::randomf10());
		Sprite *s = Sprite::create("llk_yd.png");
		s->setPosition3D(points[index_j]);
		root_node->addChild(s);
	}

	timeval  tiv1, tiv2;
	gettimeofday(&tiv1, nullptr);
	std::vector<gt::Plane3 *>  planes;
	bool b = gt::convex_hull_3d_optimal(points, planes);
	gettimeofday(&tiv2, nullptr);

	CCLOG("algorithm cost time:%.2f\n", (tiv2.tv_sec - tiv1.tv_sec) * 1000.0f + (tiv2.tv_usec - tiv1.tv_usec) / 1000.0f);
	//实验证明,经过优化后的算法的运行时间明显的缩短了
	for (auto it = planes.begin(); it != planes.end(); ++it)
	{
		gt::Plane3 *plane = *it;
		Color4F color(gt::random(), gt::random(), gt::random(), 1.0f);

		draw_node->drawLine(points[plane->v1], points[plane->v2], color);
		draw_node->drawLine(points[plane->v2], points[plane->v3], color);
		draw_node->drawLine(points[plane->v3], points[plane->v1], color);

		delete plane;
	}
	root_node->setCameraMask(s_CameraMask);
}

void testRedBlackTreeDepth(gt::red_black_tree<int> &rb_tree) {
	auto search_node = rb_tree.find_minimum();
	assert(search_node != nullptr);
	int  target = -1;
	int child_size = 0;
	int tree_depth_old = -1;
	while (search_node)
	{
		assert(target < search_node->tw_value);
		target = search_node->tw_value;
		++child_size;

		if (!search_node->l_child && !search_node->r_child) {
			int depth_tree = 1;
			auto *visit_node = search_node;
			while (visit_node) {
				if (visit_node->color_type == gt::ColorType_Black)
					++depth_tree;
				visit_node = visit_node->parent;
			}
			if (tree_depth_old == -1)
				tree_depth_old = depth_tree;
			assert(tree_depth_old == depth_tree);
		}
		search_node = rb_tree.find_next(search_node);
	}
	assert(child_size == rb_tree.size());
}

void HelloWorld::balanceTreeMemSlab()
{
	std::function<int(const int&a, const int &b)> compare_func = [](const int &a, const int &b)->int {
		return a > b?1:(a < b?-1:0);
	};
	gt::red_black_tree_alloc<gt::red_black_tree<int>::internal_node, int>  mem_alloc(1024);
	gt::red_black_tree<int>   rb_tree(0, &mem_alloc),rb_tree2(0, &mem_alloc);

	int value_array[128];
	int tree_depth_old = -1;
	for (int index_l = 0; index_l < 128; ++index_l)
	{
		int v = gt::random() * 709875;
		value_array[index_l] = v;

		rb_tree.insert(v, compare_func);
		rb_tree2.insert(v,compare_func);
	}

	std::function<bool(const int &a, const int &b)> compare_func2 = [](const int &a, const int &b)->bool {
		return a < b;
	};
	gt::quick_sort<int>(value_array,128,compare_func2);
	//输出
	testRedBlackTreeDepth(rb_tree);
	//////////////////////////////tree2//////////////////////
	testRedBlackTreeDepth(rb_tree2);
	/////////////////////////////tree2///////////////////////
	//删除
	int j = gt::random() * 16;
	int js = j + 100;
	for (; j < js; ++j)
	{
		if (value_array[j] == 56413){//442168) {
			int x = 0;
			int y = 0;
		}
		rb_tree.remove(value_array[j], compare_func);
		rb_tree2.remove(value_array[j],compare_func);
		//////////////////////test tree2/////////////////////
		testRedBlackTreeDepth(rb_tree2);
		/////////////////////test tree2/////////////////////
	}

	for (int j = js; j < 260/*js + 400*/; ++j)
	{
		if (j == 259) {
			int x = 0;
			int y = 0;
		}
		int v = gt::random() * 709875;
		rb_tree.insert(v, compare_func);

		if (rand() & 0x4)
			rb_tree.remove(v,compare_func);
		testRedBlackTreeDepth(rb_tree2);
	}
	//重新加测树的结构是否合法
	testRedBlackTreeDepth(rb_tree);
	testRedBlackTreeDepth(rb_tree2);
}

void HelloWorld::simplePolygonIntersect() {
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 700.0f;
	float length_w = 700;
	const int array_size = 15;
	//三角形平面
	std::vector<Vec2>  points(array_size);
	std::vector<Vec2>  points2(array_size);

	for (int j = 0; j < array_size; ++j)
	{
		points[j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());
		points2[j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());
	}
	//生成简单多边形
	gt::polygon_simple_generate(points, points);
	gt::polygon_simple_generate(points2, points2);
	//画出离散的边
	for (int index_j = 0; index_j < array_size; ++index_j) {
		draw_node->drawLine(points[index_j], points[index_j < array_size - 1 ? index_j + 1 : 0], Color4F::GREEN);
		draw_node->drawLine(points2[index_j], points2[index_j < array_size - 1 ? index_j + 1 : 0], Color4F::BLUE);

#if 1
		char buffer[128];
		sprintf(buffer,"%d->%d,%d",index_j,(int)points[index_j].x,(int)points[index_j].y);
		Label *label1 = Label::createWithSystemFont(buffer, "Arial",14);
		label1->setPosition(points[index_j]);
		label1->setColor(Color3B::GREEN);
		root_node->addChild(label1);

		sprintf(buffer, "%d->%d,%d",index_j,(int)points2[index_j].x,(int)points[index_j].y);
		Label *label2 = Label::createWithSystemFont(buffer, "Arial",14);
		label2->setPosition(points2[index_j]);
		//label2->setColor(Color3B::BLUE);
		root_node->addChild(label2);
#endif
	}

	std::vector<gt::simple_interleave>  intersect_array;
	gt::simple_polygon_intersect(points, points2, intersect_array);
	for (int j = 0; j < intersect_array.size(); ++j){
		auto &interleave_point = intersect_array[j];
		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(interleave_point.interleave_point);
		root_node->addChild(sprite);
	}
	//求多边形的交集合
#if 1
	std::list<std::vector<Vec2>> polygon_intersect_array;
	gt::simple_polygon_interleave_set(points, points2, intersect_array, polygon_intersect_array);

	gt::random();
	for (auto it = polygon_intersect_array.begin(); it != polygon_intersect_array.end(); ++it) {
		auto &polygon = *it;
		int array_size2 = polygon.size();
		Color4F color(gt::random(), gt::random(), gt::random(), 1.0f);
		for (int j = 0; j < array_size2; ++j) {
			draw_node->drawLine(polygon[j], polygon[(j + 1) % array_size2], color);
		}
		//break;
	}
#endif
	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::simplePolygonContainsPoint() {
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 700.0f;
	float length_w = 700;
	const int array_size = 15;
	//三角形平面
	std::vector<Vec2>  points(array_size);

	for (int j = 0; j < array_size; ++j)
		points[j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

	gt::polygon_simple_generate(points, points);
	//画出离散的边
	for (int index_j = 0; index_j < array_size; ++index_j) 
		draw_node->drawLine(points[index_j], points[index_j < array_size - 1 ? index_j + 1 : 0], Color4F::GREEN);

	//随机一个点
	const Vec2 target_point(length_w * gt::randomf10(), length_l * gt::randomf10());
	bool b_intersect = gt::simple_polygon_contains_point(points, target_point);
	Sprite *sprite = Sprite::create("llk_yd.png");
	if (b_intersect)
		sprite->setColor(Color3B::RED);
	sprite->setPosition(target_point);
	root_node->addChild(sprite);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::linearProgram2d() {
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 700.0f;
	float length_w = 700;
	const int array_size = 4 + 4;
	//三角形平面
	std::vector<gt::Line2D>  lines_array(array_size);

	for (int j = 0; j < array_size - 4; ++j) {
		Vec2 point = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());
		lines_array[j].start_point = point;
		lines_array[j].direction = gt::normalize(Vec2(length_w * gt::randomf10(), length_l * gt::randomf10()));
	}

	float boundary = 1000;// 800000;
	lines_array[array_size-4].start_point = Vec2(-boundary,-boundary);
	lines_array[array_size - 4].direction = Vec2(1.0f, 0);

	lines_array[array_size - 3].start_point = Vec2(boundary,-boundary);
	lines_array[array_size - 3].direction = Vec2(0, 1.0f);

	lines_array[array_size - 2].start_point = Vec2(boundary, boundary);
	lines_array[array_size - 2].direction = Vec2(-1.0f, 0.0f);

	lines_array[array_size - 1].start_point = Vec2(-boundary, boundary);
	lines_array[array_size - 1].direction = Vec2(0.0f,-1.0f);

	std::vector<cocos2d::Vec2>  intersect_array;
	cocos2d::Vec2 intersect_point, direction;
	float coeff_array[3] = {
		gt::randomf10(),
		gt::randomf10(),
		gt::randomf10() * 100,
	};
	int solve_value = gt::linearly_program_2d(lines_array, coeff_array, intersect_array, intersect_point, direction);
	//画出离散的边
	for (int index_j = 0; index_j < array_size - 4; ++index_j) {
		const gt::Line2D &line = lines_array[index_j];
		const Vec2 normal = line.direction * 1200.0f;
		draw_node->drawLine(line.start_point - normal,line.start_point + normal, Color4F::GREEN);
		//以及相关的法线
		draw_node->drawLine(line.start_point,line.start_point + Vec2(-line.direction.y,line.direction.x) * 200.0f,Color4F::BLUE);
	}
	for (int j = array_size-4; j < array_size ; ++j) {
		const gt::Line2D &line = lines_array[j];
		const Vec2 normal = line.direction * 2000.0f;
		draw_node->drawLine(line.start_point, line.start_point + normal, Color4F::GREEN);
		//以及相关的法线
		draw_node->drawLine(line.start_point + normal * 0.5f, line.start_point +normal * 0.5f + Vec2(-line.direction.y, line.direction.x) * 200.0f, Color4F::BLUE);
	}
	//如果交集不为空
	if (solve_value) {
		//画出目标直线
		const Vec2 &normal = *(const Vec2*)coeff_array;
		draw_node->drawLine(intersect_point - normal * 200.0f,intersect_point + normal * 200.0f,Color4F::WHITE);
		int array_size = intersect_array.size();
		for (int j = 0; j < array_size; ++j) {
			draw_node->drawLine(intersect_array[j],intersect_array[j < array_size-1?j+1:0],Color4F::RED);
		}
	}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::linearProgramTest() {
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	draw_node->setPosition(Vec2(-200.0f,-200.0f));
	root_node->addChild(draw_node);

	float width = 600.0f, height = 400.0f;
#define test_sample_idx 3
#if test_sample_idx == 0
	std::vector<float>   matrix_array = {
		1.0f,1.0f,3.0f,0.0f,0.0f,0.0f,//0
		2.0f,2.0f,5.0f,0.0f,0.0f,0.0f,//1
		4.0f,1.0f,2.0f,0.0f,0.0f,0.0f,//2
	};
	const int  nonebasic_num = 3;
	const int basic_num = 3;

	const int total_num = nonebasic_num + basic_num;
	std::vector<float*> constraints = { matrix_array.data(),matrix_array.data() + total_num,matrix_array.data() + total_num * 2 };
	std::vector<float>   const_array = { 30.0f,24.0f,36.0f };
	std::vector<float>   exp_array = { 3.0f,1.0f,2.0f,0.0f,0.0f,0.0f,0.0f };
#elif test_sample_idx == 1
	std::vector<float>   matrix_array = {
		1.0f,-1.0f,0.0f,0.0f,//0
		2.0f,1.0f,0.0f,0.0f,//1
	};
	const int  nonebasic_num = 2;
	const int basic_num = 2;

	const int total_num = nonebasic_num + basic_num;
	std::vector<float*> constraints = { matrix_array.data(),matrix_array.data() + total_num };
	std::vector<float>   const_array = { 1.0f,2.0f};
	std::vector<float>   exp_array = { -5.0f,-3.0f,0.0f,0.0f,0.0f,};
#elif test_sample_idx == 2
	std::vector<float>   matrix_array = {
		2.0f,-1.0f,0.0f,0.0f,//0
		1.0f,-5.0f,0.0f,0.0f,//1
};
	const int  nonebasic_num = 2;
	const int basic_num = 2;

	const int total_num = nonebasic_num + basic_num;
	std::vector<float*> constraints = { matrix_array.data(),matrix_array.data() + total_num };
	std::vector<float>   const_array = { 2.0f,-4.0f };
	std::vector<float>   exp_array = { 2.0f,-1.0f,0.0f,0.0f,0.0f, };
#else
	//随机生成三条直线
	std::vector<float> matrix_array(3 * 5);
	std::vector<float>   const_array(3);
	const int  nonebasic_num = 2;
	const int basic_num = 3;
	const int total_num = nonebasic_num + basic_num;
	for (int j = 0; j < 3; ++j) {
		float *target_array = matrix_array.data() + total_num * j;
		//生成直线
		const Vec2 origin_point(width * gt::random(),height * gt::random());
		const Vec2 final_point(width * gt::random(), height * gt::random());
		const Vec2 direction = gt::normalize(origin_point,final_point);
		const Vec2 normal(-direction.y,direction.x);
		const Vec2 center_point = (origin_point + final_point) * 0.5f;
		draw_node->drawLine(origin_point - direction *400.0f,final_point + direction * 400.0f,Color4F::GREEN);
		draw_node->drawLine(center_point, center_point - normal * 200.0f,Color4F::BLUE);//法线
		//化简成直线方程式
		target_array[0] = normal.x;
		target_array[1] = normal.y;
		const_array[j] = gt::dot(normal,origin_point);
	}
	
	std::vector<float*> constraints = { matrix_array.data(),matrix_array.data() + total_num,matrix_array.data() + 2 * total_num};
	//最后增加一条目标表达式
	const Vec2 origin_point(width * gt::random(), height * gt::random());
	const Vec2 final_point(width * gt::random(), height * gt::random());
	const Vec2 direction = gt::normalize(origin_point, final_point);
	const Vec2 normal(-direction.y, direction.x);
	const Vec2 center_point = (origin_point + final_point) * 0.5f;
	std::vector<float>   exp_array = { normal.x,normal.y,0.0f,0.0f,0.0f,0.0f};

	//将目标直线穿过原点
	float distance = -gt::dot(origin_point,normal);
	draw_node->drawLine(origin_point - direction * 200.0f + normal * distance,final_point + normal * distance + direction * 200.0f,Color4F::WHITE);
	draw_node->drawLine(center_point + normal * distance,center_point + normal * distance + normal * 400.0f,Color4F::RED);
#endif
	std::vector<float>   record_array(nonebasic_num + basic_num);
	//首先画出线性规划所有约束
	draw_node->drawLine(Vec2::ZERO,Vec2(width,0.0f),Color4F::YELLOW);
	draw_node->drawLine(Vec2::ZERO,Vec2(0.0f,height),Color4F::YELLOW);
	gt::SimplexType type_ref = gt::simplex_linear_program(constraints, const_array, exp_array, record_array);
	CCLOG("final result-->%s",type_ref == gt::SimplexType_Success?"Success":type_ref == gt::SimplexType_Unboundary?"Unboundary":"Failed");
	if (type_ref == gt::SimplexType_Success) {
		CCLOG("total-->%.2f",exp_array.back());
		char buffer[512];
		char *buffer_ptr = buffer;
		for (int j = 0; j < record_array.size(); ++j) {
			buffer_ptr += sprintf(buffer_ptr,"%d->%.2f\n",j,record_array[j]);
		}
		CCLOG("%s",buffer);
		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(draw_node->getPosition() + Vec2(record_array[0], record_array[1]));
		root_node->addChild(sprite);
#if test_sample_idx > 2 
		float f = gt::dot(normal,Vec2(record_array[0],record_array[1]));
		CCLOG("as verify result-->%.2f.",f);
#endif
	}
	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::frustumClippingTest() {
	//对于目前的视锥体算法来说,无论规则与否,都是可以正常处理的
	//在这里,为了简化测试流程,我们使用标准的规则视锥体
	float fov = 45.0f;
	float zeye = 100.0f;
	float height = _director->getWinSize().height * 0.5f;
	//计算视图/投影矩阵
	Mat4  view_matrix, proj_matrix;

	Vec3 eye_position(0, 0, zeye),target_position(gt::randomf10() * 100.0f, gt::randomf10() * 100.0f, zeye - 500.0f);
	float rate_ratio = _director->getWinSize().width / _director->getWinSize().height;
	float near_f = 100.0f;
	float far_f = 500.0f;

	Mat4::createLookAt(eye_position, target_position,Vec3::UNIT_Y,&view_matrix);
	Mat4::createPerspective(fov, rate_ratio, near_f, far_f, &proj_matrix);

	Mat4  view_proj_matrix = proj_matrix * view_matrix;

	//计算8个视锥体顶点
	Vec3 zaxis = gt::normalize(target_position,eye_position);
	Vec3 xaxis = gt::cross_normalize(Vec3::UNIT_Y,zaxis);
	Vec3 yaxis = gt::cross_normalize(zaxis,xaxis);

	Vec3 near_center = eye_position - zaxis * near_f;
	Vec3 far_center = eye_position - zaxis * far_f;

	float f = tanf(gt_radian(fov) * 0.5f);
	float near_height = f * near_f;
	float far_height = f * far_f;

	Vec3 near_y_step = yaxis * near_height;
	Vec3 near_x_step = xaxis * (rate_ratio * near_height);

	Vec3 far_y_step = yaxis * far_height;
	Vec3 far_x_step = xaxis * (rate_ratio * far_height);

	Vec3  frustum_vertex[8] = {
		near_center - near_x_step - near_y_step,
		near_center + near_x_step - near_y_step,
		near_center + near_x_step + near_y_step,
		near_center - near_x_step + near_y_step,

		far_center - far_x_step - far_y_step,
		far_center + far_x_step - far_y_step,
		far_center + far_x_step + far_y_step,
		far_center - far_x_step + far_y_step,
	};

	gt::Frustum2  frustum;
	Vec3 light_direction = gt::normalize((gt::random() + 0.6f) * 1000.0f,(gt::random() + 0.6f) * 1000.0f,(gt::random()+0.6f) * 1000.0f);
	frustum.initGeometryPlanes(view_proj_matrix);
	frustum.initShadowPlanes(frustum_vertex, light_direction);

	//画出视锥体的结构,12条棱
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

#define gt_draw_line(s1,s2) draw_node->drawLine(frustum_vertex[s1], frustum_vertex[s2],Color4F::GREEN);
	gt_draw_line(0,1);
	gt_draw_line(1,2);
	gt_draw_line(2,3);
	gt_draw_line(3,0);

	gt_draw_line(4, 5);
	gt_draw_line(5,6);
	gt_draw_line(6,7);
	gt_draw_line(7,4);

	gt_draw_line(3,7);
	gt_draw_line(2,6);
	gt_draw_line(0,4);
	gt_draw_line(1,5);
#undef gt_draw_line
	//测试几何体裁剪
	float length_l = 200.0f;
	Vec3 min_bb(gt::randomf10() * length_l, gt::randomf10() * length_l, gt::randomf10() * length_l);
	Vec3 extent(gt::random() * length_l, gt::random() * length_l, gt::random() * length_l);
#if 0
	gt::AABB aabb;

	gt::aabb_create(aabb, min_bb, min_bb + extent);
	bool b2 = frustum.isLocateInFrustum(aabb);
	const Color4F &color = b2 ?Color4F::RED:Color4F::WHITE ;
	draw_node->drawAABB(min_bb, aabb.bb_max, color);
#else
	Vec3 normal = gt::normalize(gt::randomf10(), gt::randomf10(), gt::randomf10());
	float angle = 360 * gt::random();
	gt::OBB  obb;
	gt::obb_create(obb, min_bb, min_bb + extent, normal, angle);
	Vec3 vertex[8];
	gt::obb_create_obb_vertex8(obb, vertex);
	bool b2 = frustum.isLocateInFrustum(obb);
	const Color4F &color = b2 ? Color4F::RED : Color4F::WHITE;
	draw_node->drawCube2(vertex, color, color, color);
#endif

	root_node->setCameraMask(s_CameraMask);
}