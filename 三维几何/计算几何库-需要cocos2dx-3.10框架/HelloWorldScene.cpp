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
	testRedBlackTree();

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

int compare_func(const int &a,const int &b,const int &l_other)
{
	return a < b ? -1: (a > b? 1 : 0);
}

void HelloWorld::testRedBlackTree()
{
	gt::red_black_tree<int, int>   rb_tree(compare_func,32);
	int l_other = 0;
	for (int index_l = 0; index_l < 128; ++index_l)
		rb_tree.insert(index_l, l_other);
	//输出
	auto search_node = rb_tree.lookup(0, l_other);
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
		rb_tree.remove(start_j, l_other);

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

void HelloWorld::doPlaneIntersectWithPlane()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;

	const int array_size = 6;
	//三角形平面
	Vec3  points[array_size];

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	gt::Plane   plane1, plane2;
	gt::plane_create(plane1, points[0],points[1],points[2]);
	gt::plane_create(plane2, points[3],points[4],points[5]);

	gt::Line  line;
	bool b = gt::plane_plane_intersect_test(plane1, plane2, line);
	const Color4F &color = b ? Color4F::RED : Color4F::GREEN;

	draw_node->drawMesh((points[0] + points[1] +points[2])* 0.333f, plane1.normal,Vec2(length_l,length_l),32,32,Color4F::WHITE);
	draw_node->drawMesh((points[3]+points[4]+points[5])* 0.333f, plane2.normal, Vec2(length_l, length_l), 32, 32, Color4F::WHITE);

	if(b)
	draw_node->drawLine(line.start_point - line.direction * 3.0f * length_l, line.start_point + line.direction * 3.0f * length_l, color);

	float d1 = plane1.distanceTo(line.start_point);
	float d2 = plane2.distanceTo(line.start_point);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doPlane3Intersecttest()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;

	const int array_size = 9;
	//三角形平面
	Vec3  points[array_size];

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	gt::Plane   plane1, plane2,plane3;
	gt::plane_create(plane1, points[0], points[1], points[2]);
	gt::plane_create(plane2, points[3], points[4], points[5]);
	gt::plane_create(plane3, points[6], points[7], points[8]);

	gt::Line  line;
	Vec3 intersect_point;
	//bool b = gt::plane_plane_intersect_test(plane1, plane2, line);
	bool b = gt::plane_plane_plane_intersect_test(plane1, plane2, plane3, intersect_point);
	const Color4F &color = b ? Color4F::RED : Color4F::GREEN;

	draw_node->drawMesh((points[0] + points[1] + points[2])* 0.333f, plane1.normal, Vec2(length_l, length_l), 32, 32, Color4F::WHITE);
	draw_node->drawMesh((points[3] + points[4] + points[5])* 0.333f, plane2.normal, Vec2(length_l, length_l), 32, 32, Color4F::WHITE);
	draw_node->drawMesh((points[6] + points[7] + points[8])* 0.333f, plane3.normal, Vec2(length_l, length_l), 32, 32, Color4F::WHITE);

	if (b)
	{
		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition3D(intersect_point);
		root_node->addChild(sprite);
		sprite->setColor(Color3B::RED);
	}

	//float d1 = plane1.distanceTo(line.start_point);
	//float d2 = plane2.distanceTo(line.start_point);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::do2DSpacePartionAndORectIntersect()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	auto &winSize = _director->getWinSize();
	//空间划分网格横纵数目
	const int horizontal_num = 9;
	const int vertical_num = 6;
	//画出网格
	const Vec2 bottom_left(-winSize.width * 0.5f,-winSize.height *0.5f);
	const Vec2 direction(winSize.width,winSize.height);
	//
	const Vec2 world_center(winSize.width * 0.5f, winSize.height *0.5f);
	//横向
	for(int index_l = 0; index_l <= horizontal_num; ++index_l)
	{
		const Vec2 bottom = bottom_left + Vec2(winSize.width * index_l/horizontal_num,0.0f);
		draw_node->drawLine(bottom, bottom + Vec2(0.0f,winSize.height),Color4F::GREEN);
	}
	//纵向
	for (int index_l = 0; index_l <= vertical_num; ++index_l)
	{
		const Vec2 left = bottom_left + Vec2(0.0f,winSize.height * index_l/vertical_num);
		draw_node->drawLine(left,left + Vec2(winSize.width,0.0f),Color4F::GREEN);
	}
	float length_l = 300.0f;
	////////////////////////////////////////////
	//画出有向矩形OR
	const Vec2 corner(length_l * gt::randomf10(),length_l * gt::randomf10());
	const Vec2 other(length_l * gt::randomf10(), length_l * gt::randomf10());
	const Vec2 normal = gt::normalize(other - corner);

	draw_node->drawLine(corner,other, Color4F::RED);

	float line_length = sqrtf(gt::length2(corner -other));
	//计算逻辑上下方的两个端点
	Vec2 v0 = corner + world_center;
	float unit_x = winSize.width / horizontal_num;
	float unit_y = winSize.height / vertical_num;
	//
	int start_x = v0.x / unit_x;
	int start_y = v0.y / unit_y;
	Vec2 pixel(start_x * unit_x,start_y * unit_y);
	//所有已经法线的相交区域
	std::vector<Vec2>   grid_map;
	Vec2 delta,accumulate;
	int step_x, step_y;
	if (normal.x == 0.0f)
	{
		accumulate.x = FLT_MAX;
		step_x = 0;
	}
	else if (normal.x > 0.0f)
	{
		accumulate.x = (pixel.x + unit_x - v0.x)/ normal.x;
		step_x = 1;
	}
	else
	{
		accumulate.x = (pixel.x - v0.x)/ normal.x;
		step_x = -1;
	}

	if (normal.y == 0.0f)
	{
		accumulate.y = FLT_MAX;
		step_y = 0;
	}
	else if (normal.y > 0.0f)
	{
		accumulate.y = (pixel.y + unit_y -v0.y)/ normal.y;
		step_y = 1;
	}
	else
	{
		accumulate.y = (pixel.y - v0.y)/ normal.y;
		step_y = -1;
	}
	delta.x = step_x * unit_x/(normal.x ==0.0f?0.0001f: normal.x);
	delta.y = step_y * unit_y / (normal.y == 0.0f?0.0001f: normal.y);
	int loop_count = 0;
	float extra_l = FLT_MAX;
	if (normal.x != 0)
		extra_l = fabsf(unit_x/normal.x);
	if (normal.y != 0)
		extra_l = fminf(extra_l,fabsf(unit_y/normal.y));

	int final_x = (other.x  + world_center.x)/ unit_x;
	int final_y = (other.y +world_center.y)/ unit_y;

	while (loop_count < horizontal_num + vertical_num)
	{
		if (start_x >= 0 && start_x < horizontal_num && start_y >= 0 && start_y < vertical_num)
		{
			Sprite *sprite = Sprite::create("llk_yd.png");
			sprite->setPosition(start_x* unit_x + 0.5f * unit_x - world_center.x, start_y * unit_y + 0.5f *unit_y - world_center.y);
			root_node->addChild(sprite);
		}
		if (start_x == final_x && start_y == final_y)
			break;
		float min_f = fminf(accumulate.x,accumulate.y);
		if (min_f == accumulate.x)
		{
			accumulate.x += delta.x;
			start_x += step_x;
		}

		if (min_f == accumulate.y)
		{
			accumulate.y += delta.y;
			start_y += step_y;
		}

		++loop_count;
		//循环结束的条件为,走到了线段的终点
	}

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition(corner);
	root_node->addChild(sprite);
	sprite->setColor(Color3B::RED);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::bresenhamAlgorithm()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	auto &winSize = _director->getWinSize();
	//空间划分网格横纵数目
	const int horizontal_num = 9;
	const int vertical_num = 6;
	const float unit_x = winSize.width / horizontal_num;
	const float unit_y = winSize.height / vertical_num;
	//画出网格
	const Vec2 bottom_left(-winSize.width * 0.5f, -winSize.height *0.5f);
	const Vec2 direction(winSize.width, winSize.height);
	//
	const Vec2 world_center(winSize.width * 0.5f, winSize.height *0.5f);
	//横向
	for (int index_l = 0; index_l <= horizontal_num; ++index_l)
	{
		const Vec2 bottom = bottom_left + Vec2(winSize.width * index_l / horizontal_num, 0.0f);
		draw_node->drawLine(bottom, bottom + Vec2(0.0f, winSize.height), Color4F::GREEN);
	}
	//纵向
	for (int index_l = 0; index_l <= vertical_num; ++index_l)
	{
		const Vec2 left = bottom_left + Vec2(0.0f, winSize.height * index_l / vertical_num);
		draw_node->drawLine(left, left + Vec2(winSize.width, 0.0f), Color4F::GREEN);
	}
	int start_x = horizontal_num * gt::random();
	int start_y = vertical_num * gt::random();

	int final_x = horizontal_num * gt::random();
	int final_y = vertical_num * gt::random();

	draw_node->drawLine(Vec2(start_x * unit_x,start_y * unit_y) - world_center,Vec2(final_x * unit_x,final_y * unit_y) - world_center,Color4F::BLUE);

	int delta_x = final_x - start_x;
	int delta_y = final_y - start_y;
	int abs_x = delta_x >= 0 ? delta_x : -delta_x;
	int abs_y = delta_y >= 0 ? delta_y : -delta_y;

	int step_x = final_x > start_x ? 1 : (final_x < start_x ? -1 : 0);
	int step_y = final_y > start_y ? 1 : (final_y < start_y? -1: 0);
	std::vector<cocos2d::Vec2>  location_array;
	if (abs_x >= abs_y)
	{
		int loops_count = abs_x + 1;
		int e = 0;
		for (int x = 0; x < loops_count; ++x)
		{
			e += abs_y << 1;
			location_array.push_back(Vec2(start_x,start_y));
			start_x += step_x;
			if (e >= abs_x)
			{
				start_y += step_y;
				e -= abs_x << 1;
			}
		}
	}
	else
	{
		int loops_count = abs_y + 1;
		int e = 0;
		for (int y = 0; y < loops_count; ++y)
		{
			location_array.push_back(Vec2(start_x,start_y));
			e += abs_x << 1;
			start_y += step_y;
			if (e >= abs_y)
			{
				start_x += step_x;
				e -= abs_y << 1;
			}
		}
	}
	//画出直线
	for (int index_l = 0; index_l < location_array.size(); ++index_l)
	{
		const Vec2 &location = location_array[index_l];
		const Vec2 &secondary = location_array[index_l +1 >= location_array.size()?0:index_l +1];
		draw_node->drawLine(Vec2(location.x * unit_x,location.y * unit_y) - world_center,Vec2(secondary.x * unit_x,secondary.y * unit_y) - world_center,Color4F::RED);
	}
	
	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::segmentNIntersectTest()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	auto &winSize = _director->getWinSize();
	const Vec2 screen_center(winSize.width * 0.5f, winSize.height * 0.5f);

	const int array_size = 64;
	gt::Segment2D    segment_array[array_size];
	for (int index_l = 0; index_l < array_size; ++index_l)
	{
		Vec2 start_point(winSize.width * gt::random(),winSize.height * gt::random());
		Vec2 final_point(winSize.width * gt::random(), winSize.height * gt::random());
		if (start_point.x < final_point.x || start_point.x == final_point.x && start_point.y < final_point.y)
		{
			segment_array[index_l].start_point = start_point - screen_center;
			segment_array[index_l].final_point = final_point - screen_center;
		}
		else
		{
			segment_array[index_l].start_point = final_point - screen_center;
			segment_array[index_l].final_point = start_point - screen_center;
		}
		float length_l = 40 * (0.1 +gt::random());
		segment_array[index_l].final_point = segment_array[index_l].start_point + gt::normalize(segment_array[index_l].final_point - segment_array[index_l].start_point) * length_l;
		//画出线段
		draw_node->drawLine(segment_array[index_l].start_point, segment_array[index_l].final_point,Color4F::GREEN);
	}

	bool test = gt::segments2d_N_intersect_test(segment_array, array_size);
	if (test)
	{
		Sprite  * sprite = Sprite::create("llk_yd.png");
		root_node->addChild(sprite);
		sprite->setColor(Color3B::RED);
	}
	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::dispersePointsMaxDistanceTest()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float length_l = 400;
	const int array_size = 64;
	std::vector<Vec2>  point_array(array_size);
	for (int index_l = 0; index_l < array_size; ++index_l)
	{
		point_array[index_l] = Vec2(length_l * gt::randomf10(),length_l * gt::randomf10());
		Sprite  * sprite = Sprite::create("llk_yd.png");
		sprite-> setPosition(point_array[index_l]);
		root_node->addChild(sprite);
	}
	//计算多边形
	std::vector<Vec2> polygon_points;
	polygon_points.reserve(array_size);
	gt::polygon_compute_minimum(point_array, polygon_points);
	//画出多边形的边
	for(int index_l =0;index_l < polygon_points.size();++index_l)
		draw_node->drawLine(polygon_points[index_l],polygon_points[index_l +1 >= polygon_points.size()?0:index_l+1],Color4F::WHITE);
	//计算最大距离
	Vec2 max_a, max_b;
	gt::polygon_compute_max_distance(polygon_points, max_a, max_b);
	draw_node->drawLine(max_a, max_b, Color4F::GREEN);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::dispersePointsMinDistanceTest()
{
	float length_l = 10000;
	const int array_size = 10373;
	std::vector<Vec2>  point_array(array_size);
	for (int index_l = 0; index_l < array_size; ++index_l)
		point_array[index_l] = Vec2(length_l * gt::randomf10(), length_l * gt::randomf10());
	//
	Vec2 a, b, c, d;
	timeval   tv1,tv2;
	int  day_time = gettimeofday(&tv1,nullptr);
	float d1 = gt::point_prim_compute_minimum_distance(point_array,a,b);
	int day_t2 = gettimeofday(&tv2,nullptr);
	CCLOG("prim cost time->%d",(tv2.tv_sec - tv1.tv_sec) * 1000 +(tv2.tv_usec - tv1.tv_usec)/1000);

	day_time = gettimeofday(&tv1, nullptr);
	float d2 = gt::point_compute_minimum_distance(point_array, c, d);
	day_t2 = gettimeofday(&tv2, nullptr);
	CCLOG("fast cost time->%d", (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000);

	CCLOG("prim-method->%f",d1);
	CCLOG("fast-method->%f",d2);
	
	//输出如下:
	//prim cost time->31705
	//	fast cost time->88
	//	prim - method->1.364788
	//	fast - method->1.364788
}

void HelloWorld::dispersePoints3DMinDistanceTest()
{
	float length_l = 10000;
	const int array_size = 12512;
	std::vector<Vec3>  point_array(array_size);
	for (int index_l = 0; index_l < array_size; ++index_l)
		point_array[index_l] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	//
	Vec3 a, b, c, d;
	timeval   tv1, tv2;
	int  day_time = gettimeofday(&tv1, nullptr);
	float d1 = gt::point_prim_compute_minimum_distance(point_array, a, b);
	int day_t2 = gettimeofday(&tv2, nullptr);
	CCLOG("prim cost time->%d", (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000);

	day_time = gettimeofday(&tv1, nullptr);
	float d2 = gt::point_compute_minimum_distance(point_array, c, d);
	day_t2 = gettimeofday(&tv2, nullptr);
	CCLOG("fast cost time->%d", (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000);

	CCLOG("prim-method->%f", d1);
	CCLOG("fast-method->%f", d2);
	//输入如下
	//seed->1566387480
	//	prim cost time->50873
	//	fast cost time->1340
	//	prim - method->25.729469
	//	fast - method->25.729469
}

void HelloWorld::gjkAlgorithmTest()
{
	Node  *root_node = Node::create();
	this->addChild(root_node);
	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	const int point_size = 32;
	std::vector<Vec2>   points1(point_size);
	float offset_left = -300;
	float length_l = 200;

	for (int index_l = 0; index_l < point_size; ++index_l)
	{
		points1[index_l] = Vec2(length_l * gt::randomf10() + offset_left,length_l * gt::randomf10());
		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points1[index_l]);
		root_node->addChild(sprite);
	}

	std::vector<Vec2> points2(point_size);
	for (int index_l = 0; index_l < point_size; ++index_l)
	{
		points2[index_l] = Vec2(length_l * gt::randomf10() + 80, length_l * gt::randomf10());
		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points2[index_l]);
		root_node->addChild(sprite);
	}

	gt::Polygon  polygon1, polygon2;
	std::vector<Vec2>  polygon_array1, polygon_array2;
	gt::polygon_compute_minimum(points1, polygon_array1);
	gt::polygon_compute_minimum(points2,polygon_array2);

	gt::polygon_create(polygon1, polygon_array1);
	gt::polygon_create(polygon2, polygon_array2);

	Vec2 a, b;
	bool intersect = gt::polygon_polygon_intersect_test(polygon1, polygon2,a,b);

	const Color4F &color = intersect ? Color4F::RED : Color4F::GREEN;
	for (int index_l = 0; index_l < polygon_array1.size(); ++index_l)
	{
		draw_node->drawLine(polygon_array1[index_l], polygon_array1[index_l + 1>= polygon_array1 .size()?0:index_l+1],color);
	}

	for (int index_l = 0; index_l < polygon_array2.size(); ++index_l)
	{
		draw_node->drawLine(polygon_array2[index_l], polygon_array2[index_l + 1 >= polygon_array2.size() ? 0 : index_l + 1], color);
	}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::quickHullAlgorithmTest()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 200.0f;
	const int array_size = 64;
	//三角形平面
	std::vector<Vec2>  points(array_size);

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec2(length_l * gt::randomf10(), length_l * gt::randomf10());

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_j]);
		root_node->addChild(sprite);
	}
	std::vector<Vec2> polygon;
	gt::quick_hull_algorithm2d(points, polygon);
	//画线
	for (int index_l = 0,last_index = polygon.size()-1; index_l < polygon.size();++index_l)
	{
		draw_node->drawLine(polygon[last_index],polygon[index_l],Color4F::GREEN);
		last_index = index_l;
	}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::quickSegmentIntersectTest()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	auto &winSize = _director->getWinSize();
	const float scale_factor = 1.0f;
	const Vec2 screen_center(winSize.width * 0.5f * scale_factor, winSize.height * 0.5f * scale_factor);

	const int array_size = 128;
	std::vector<gt::Segment2D>    segment_array(array_size);
	for (int index_l = 0; index_l < array_size; ++index_l)
	{
		Vec2 start_point(scale_factor * winSize.width * gt::random(), scale_factor * winSize.height * gt::random());
		Vec2 final_point(scale_factor * winSize.width * gt::random(), scale_factor * winSize.height * gt::random());
		//对生成的线段端点做额外的处理
		if (start_point.y > final_point.y || start_point.y == final_point.y && start_point.x < final_point.x)
		{
			segment_array[index_l].start_point = start_point - screen_center;
			segment_array[index_l].final_point = final_point - screen_center;
		}
		else
		{
			segment_array[index_l].start_point = final_point - screen_center;
			segment_array[index_l].final_point = start_point - screen_center;
		}
		float length_l = 120 * (0.4f + gt::random());
		segment_array[index_l].final_point = segment_array[index_l].start_point + gt::normalize(segment_array[index_l].final_point - segment_array[index_l].start_point) * length_l;
		//画出线段
		draw_node->drawLine(segment_array[index_l].start_point, segment_array[index_l].final_point, Color4F::GREEN);
	}
	//交点
	std::vector<Vec2>	intersect_points;
	int number = gt::segment_n_intersect_point(segment_array, intersect_points);
	for (int index_l = 0; index_l < number; ++index_l)
	{
		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(intersect_points[index_l]);
		sprite->setColor(Color3B::RED);
		root_node->addChild(sprite);
	}

	std::vector<Vec2>	intersect_points2;
	int number2 = gt::segment_n_intersect_prim(segment_array, intersect_points2);
	CCLOG("fast->%d,prim->%d",number,number2);

	root_node->setCameraMask(s_CameraMask);
}