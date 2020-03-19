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
	simplePolygonIntersect();

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