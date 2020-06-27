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
	//frustumClippingTest();
	//pointLocationTest();
	//fortuneAlgorithmTest();
	remoteVoronoiTest();

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

void static_draw_line(gt::NodeLocal *node_local,DrawNode *draw_node,const Color4F *color_target) {
	gt::Trapzoid *trap_ptr = node_local->trap_ptr;
	Vec2 p0, p1, p2, p3;
	gt::segment_vertical_intersent(trap_ptr->low_seg_ptr->start_point, trap_ptr->low_seg_ptr->final_point, trap_ptr->left_point.x, p0);
	gt::segment_vertical_intersent(trap_ptr->low_seg_ptr->start_point, trap_ptr->low_seg_ptr->final_point, trap_ptr->right_point.x, p1);
	gt::segment_vertical_intersent(trap_ptr->up_seg_ptr->start_point, trap_ptr->up_seg_ptr->final_point, trap_ptr->right_point.x, p2);
	gt::segment_vertical_intersent(trap_ptr->up_seg_ptr->start_point, trap_ptr->up_seg_ptr->final_point, trap_ptr->left_point.x, p3);

	Color4F  color(gt::random(), gt::random(), gt::random(), 1.0f);
	draw_node->drawLine(p0, p1, color_target?*color_target:color);
	draw_node->drawLine(p1, p2, color_target ? *color_target : color);
	draw_node->drawLine(p2, p3, color_target ? *color_target : color);
	draw_node->drawLine(p3, p0, color_target ? *color_target : color);
}

void HelloWorld::pointLocationTest() {
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);
	//首先需要生成不相交的线段集合,然后求出真包围盒
	float length_l = 200.0f;
	auto &winSize = _director->getWinSize();
	float local_x = winSize.width * 0.5f;
	float local_y = winSize.height * 0.5f;

	int array_size = 17;
	Vec2  bb_min(FLT_MAX,FLT_MAX), bb_max(-FLT_MAX,-FLT_MAX);
	std::vector<gt::Segment2D>  seg_array(array_size);

	for (int j = 0; j< array_size; ) {
		gt::Segment2D  seg;
		seg.start_point = Vec2(local_x * gt::randomf10(),local_y * gt::randomf10());
		seg.final_point = seg.start_point + Vec2 (length_l * gt::random(),length_l * gt::random());

		bool b2 = false;
		for (int s = 0; s < j; ++s) {
			b2 |= gt::segment_segment_intersect_test(seg,seg_array[s]);
			if (b2)break;
		}
		if (!b2) {
			seg_array[j++] = seg;
			bb_min.x = fminf(bb_min.x,seg.start_point.x);
			bb_min.y = fminf(bb_min.y,seg.start_point.y);

			bb_max.x = fmaxf(bb_max.x,seg.final_point.x);
			bb_max.y = fmaxf(bb_max.y,seg.final_point.y);
		}
	}
	gt::Segment2D low_seg = {bb_min + Vec2(-100,-100),Vec2(bb_max.x + 100.0f,bb_min.y - 100.0f)};
	gt::Segment2D up_seg = {Vec2(bb_min.x-100.0f,bb_max.y+100.0f),Vec2(bb_max.x +100.0f,bb_max.y + 100.0f)};
	seg_array.push_back(low_seg);
	seg_array.push_back(up_seg);

	//点定位算法测试
	gt::LocationLexer  local_lexer;
	gt::local_point_create_trapzoid(local_lexer, seg_array);
	//画出所有的梯形,其过程是对生成的数据结构的一次遍历
	std::vector<gt::NodeLocal*> nodes_array;
	local_point_visit(local_lexer, nodes_array,-1);
	for (int j = 0; j < nodes_array.size(); ++j) {
		gt::NodeLocal *node_local = nodes_array[j];
		if (node_local->node_type == gt::LocalType_Trapzoid) {
			static_draw_line(node_local, draw_node,nullptr);
		}
	}
	//点定位查询
	const Vec2 target_location(gt::randomf10() * local_x * 0.8f,gt::randomf10() * local_y * 0.8f);
	gt::NodeLocal *node_local = gt::local_point_find_location(local_lexer, target_location);
	assert(!node_local || node_local && node_local->node_type == gt::LocalType_Trapzoid);
	//画出相关的点,以及所在的梯形
	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition(target_location);
	root_node->addChild(sprite);

	if(node_local != nullptr)
		static_draw_line(node_local, draw_node,&Color4F::RED);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::fortuneAlgorithmTest() {
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);
	//随机生成离散点集
	int64_t seed = 1592362715;// time(nullptr);
	CCLOG("new seed -->%ld",seed);
	std::default_random_engine  random_engine(seed);
	std::uniform_real_distribution<float> distribution;

	const int array_size = 16;
	float fx_width = _director->getWinSize().width * 0.5f;
	float fx_height = _director->getWinSize().height * 0.5f;

	std::vector<Vec2>  discard_points(array_size);
	for (int j = 0; j < array_size; ++j) {
		discard_points[j] = Vec2((distribution(random_engine) * 2.0f - 1.0f) * fx_width, (distribution(random_engine) * 2.0f -1.0f) * fx_height);
		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(discard_points[j]);
		root_node->addChild(sprite);
	}

	gt::FortuneAlgorithm   fx_algorithm(discard_points);
	fx_algorithm.build();
	fx_algorithm.bound();
	//画出各条边,包括无界边,目前先暂时画出有限边
	gt::FkDiagram  &fx_diagram = fx_algorithm.getDiagram();
	gt::link_list<gt::FxEdge*>  &half_edges = fx_diagram.getHalfEdges();
	for (auto *it_ptr = half_edges.head(); it_ptr != nullptr; it_ptr = it_ptr->next) {
		gt::FxEdge  *half_edge = it_ptr->tv_value;
		if (half_edge->origin_ptr != nullptr && half_edge->destination_ptr != nullptr) {
			draw_node->drawLine(*half_edge->origin_ptr, *half_edge->destination_ptr, Color4F::GREEN);
		}
	}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::remoteVoronoiTest() {
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);
	//随机生成离散点集
	int64_t seed = 1592362715;// time(nullptr);
	CCLOG("new seed -->%ld", seed);
	std::default_random_engine  random_engine(seed);
	std::uniform_real_distribution<float> distribution;

	const int array_size = 16;
	float fx_width = _director->getWinSize().width * 0.4f;
	float fx_height = _director->getWinSize().height * 0.4f;

	std::vector<Vec2>  discard_points(array_size);
	for (int j = 0; j < array_size; ++j) {
		discard_points[j] = Vec2((distribution(random_engine) * 2.0f - 1.0f) * fx_width, (distribution(random_engine) * 2.0f - 1.0f) * fx_height);
		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(discard_points[j]);
		root_node->addChild(sprite);
	}
	std::vector<gt::FxvSite>   site_array;
#if 0
	gt::link_list<Vec2>  hull_list;
	gt::fx_create_clipper(discard_points, hull_list);
	site_array.resize(hull_list.size());
	int j = 0;
	for (auto *it_ptr = hull_list.head(); it_ptr != nullptr; it_ptr = it_ptr->next,++j) {
		site_array[j].location = it_ptr->tv_value;
		draw_node->drawLine(it_ptr->tv_value, it_ptr->next ? it_ptr->next->tv_value:hull_list.head()->tv_value, Color4F::GREEN);
	}
	gt::fx_create_remote_voronoi3(hull_list, site_array);
	for (j = 0; j < 3; ++j) {
		gt::FxvSite &now_site = site_array[j];
		//其次画出所有的边
		Color4F	color(distribution(random_engine), distribution(random_engine), distribution(random_engine), 1.0f);
		gt::FxvEdge  *edge_ptr = now_site.head_ptr;
		while (edge_ptr != nullptr) {
			draw_node->drawLine(edge_ptr->origin, edge_ptr->destination, color);
			edge_ptr = edge_ptr->next;
		}
	}
#endif

#if 1
	gt::fx_create_remote_voronoi(discard_points, site_array);
	//首先画出凸壳
	int array_size2 = site_array.size();
	for (int j = 0; j < array_size2; ++j) {
		gt::FxvSite &now_site = site_array[j];
		draw_node->drawLine(site_array[j].location,site_array[j+1 < array_size2?j+1:0].location,Color4F::GREEN);
		//其次画出所有的边
		Color4F	color(distribution(random_engine), distribution(random_engine), distribution(random_engine),1.0f);
		gt::FxvEdge  *edge_ptr = now_site.head_ptr;
		while (edge_ptr != nullptr) {
			draw_node->drawLine(edge_ptr->origin,edge_ptr->destination,color);
			edge_ptr = edge_ptr->next;
		}
	}
#endif
	root_node->setCameraMask(s_CameraMask);
}