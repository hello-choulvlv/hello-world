#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "geometry.h"
#include "DrawNode3D.h"
#include "data_struct/balance_tree.h"

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
	int64_t seed = time(nullptr);//1572057392;
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
	delaunayTriangulate();

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
		float speed = 6;
		if (_keyMask & 0x1)
			move_step -= direction_z * speed;
		if (_keyMask & 0x2)
			move_step += direction_z * speed;

		if (_keyMask & 0x4)
			move_step -= direction_x * speed;
		if (_keyMask & 0x8)
			move_step += direction_x * speed;

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
	std::function<bool(const int&a, const int &b)> compare_func = [](const int &a,const int &b)->bool {
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
	bool check_polygon = gt::polygon_compute_minimum(points, polygon_vector);
	gt::polygon_compute_minimum(points2, polygon_vector2);
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
	const int array_size = 128;
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
	gt::delaunay_triangulate_bowyer_washton(points, delaunay_trianges, real_size);

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