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
	//remoteVoronoiTest();
	//visiblityMapTest();
	//testGJKAlogorithm();
	//testChungWangAlgorithm();
	testEarTriangleAlgorithm();

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
	if (_touchCallback != nullptr)
		_touchCallback(touch->getLocation(),0);
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

	if (_touchCallback != nullptr)
		_touchCallback(touch->getLocation(),1);
}

void HelloWorld::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event)
{
	if (_touchCallback != nullptr)
		_touchCallback(touch->getLocation(), 2);
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

	if (_keyCallback != nullptr)
		_keyCallback(keyCode,event,0);
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

	if (_keyCallback != nullptr)
		_keyCallback(keyCode,event,1);
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

void HelloWorld::visiblityMapTest() {
	//创建多个多边形,简单多边形即可,这里为了简化测试代码,统统使用了凸多边形
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);
	//随机生成离散点集
	int64_t seed = time(nullptr);
	CCLOG("new seed -->%ld", seed);
	std::default_random_engine  random_engine(seed);
	std::uniform_real_distribution<float> distribution;

	const int array_size = 4;
	const int vertex_size = 6;

	const int row_size = 2;
	const int column_size = 2;

	std::vector<Vec2>    polygons[array_size];

	const Size &win_size = _director->getWinSize();
	float  cell_w = win_size.width/ column_size;
	float cell_h = win_size.height / row_size;

	int total_vertex_size = 2;
	for (int j = 0; j < array_size; ++j) {
		std::vector<Vec2> polygon(vertex_size);

		int  idx_row = j/column_size;
		int idx_column = j%column_size;

		Vec2 offset_v2 = Vec2(cell_w * idx_column - win_size.width * 0.5f,cell_h * idx_row - win_size.height * 0.5f);
		for (int k = 0; k < vertex_size; ++k)
			polygon[k] = Vec2(cell_w * distribution(random_engine),cell_h * distribution(random_engine)) + offset_v2;

		gt::polygon_compute_convex_hull(polygon, polygons[j]);
		total_vertex_size += polygons[j].size();
	}
	//随机选出两个点,这两个点不能包含在任何的一个多边形内
	cocos2d::Vec2 source_point, target_point;
	bool b2 = false;
	do 
	{
		source_point.x = (distribution(random_engine) * 2.0f - 1.0f) * win_size.width * 0.5f;
		source_point.y = (distribution(random_engine) * 2.0f - 1.0f) * win_size.height * 0.5f;

		b2 = false;
		for (int j = 0; j < array_size; ++j)
			b2 |= gt::polygon_contains_point(polygons[j],source_point);
	} while (b2);

	b2 = false;
	do
	{
		target_point.x = (distribution(random_engine) * 2.0f - 1.0f) * win_size.width * 0.5f;
		target_point.y = (distribution(random_engine) * 2.0f - 1.0f) * win_size.height * 0.5f;

		b2 = false;
		for (int j = 0; j < array_size; ++j)
			b2 |= gt::polygon_contains_point(polygons[j], target_point);
	} while (b2);

	//画出多边形
	std::vector<Vec2>  vertex_array;
	vertex_array.push_back(source_point);
	vertex_array.push_back(target_point);

	for (int j = 0; j < array_size; ++j) {
		vertex_array.insert(vertex_array.end(), polygons[j].begin(), polygons[j].end());
		draw_node->drawLines(polygons[j].data(), polygons[j].size(), true, Color4F(distribution(random_engine), distribution(random_engine), distribution(random_engine), 1.0f));
	}

	Sprite *s = Sprite::create("llk_yd.png");
	s->setPosition(source_point);
	root_node->addChild(s);

	Sprite *s2 = Sprite::create("llk_yd.png");
	s2->setPosition(target_point);
	root_node->addChild(s2);

	std::vector<gt::rtVertex*> *rt_vertex_array = gt::rt_compute_visibility_map(polygons,array_size,source_point,target_point);
	
	//for (int j = 0; j < total_vertex_size; ++j) {
	int select_idx = 1;// j;
		auto &vertex_adjs = rt_vertex_array[select_idx];
		Color4F color(distribution(random_engine), distribution(random_engine), distribution(random_engine),1.0f);
		for (int k = 0; k < vertex_adjs.size(); ++k)
			draw_node->drawLine(vertex_array[select_idx],*vertex_adjs[k]->location_ptr,color);
	//}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::testGJKAlogorithm() {
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
		points1[index_l] = Vec2(length_l * gt::randomf10() + offset_left, length_l * gt::randomf10());
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

	std::vector<Vec2>  polygon_array1, polygon_array2;
	gt::polygon_compute_convex_hull(points1, polygon_array1);
	gt::polygon_compute_convex_hull(points2, polygon_array2);

	Vec2 intersect_points[2];
	bool intersect = gt::gjk_algorithm_optimal(polygon_array1, polygon_array2, intersect_points);

	const Color4F &color = intersect ? Color4F::RED : Color4F::GREEN;
	for (int index_l = 0; index_l < polygon_array1.size(); ++index_l)
	{
		draw_node->drawLine(polygon_array1[index_l], polygon_array1[index_l + 1 >= polygon_array1.size() ? 0 : index_l + 1], color);
	}

	for (int index_l = 0; index_l < polygon_array2.size(); ++index_l)
	{
		draw_node->drawLine(polygon_array2[index_l], polygon_array2[index_l + 1 >= polygon_array2.size() ? 0 : index_l + 1], color);
	}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::testChungWangAlgorithm() {
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
		points1[index_l] = Vec2(length_l * gt::randomf10() + offset_left, length_l * gt::randomf10());
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

	std::vector<Vec2>  polygon_array1, polygon_array2;
	gt::polygon_compute_convex_hull(points1, polygon_array1);
	gt::polygon_compute_convex_hull(points2, polygon_array2);

	Vec2 intersect_points;
	bool intersect = gt::chung_wang_seperate_algorithm(polygon_array1, polygon_array2, intersect_points);

	const Color4F &color = intersect ? Color4F::RED : Color4F::GREEN;
	for (int index_l = 0; index_l < polygon_array1.size(); ++index_l)
	{
		draw_node->drawLine(polygon_array1[index_l], polygon_array1[index_l + 1 >= polygon_array1.size() ? 0 : index_l + 1], color);
	}

	for (int index_l = 0; index_l < polygon_array2.size(); ++index_l)
	{
		draw_node->drawLine(polygon_array2[index_l], polygon_array2[index_l + 1 >= polygon_array2.size() ? 0 : index_l + 1], color);
	}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::testEarTriangleAlgorithm() {
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 600;
	const int array_size = 12;
	//三角形平面
	std::vector<Vec2>  points(array_size);

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_j]);
		root_node->addChild(sprite);
	}
	//生成简单多边形
	gt::polygon_simple_generate(points, points);
	for (int j = 0; j < points.size(); ++j) {
		draw_node->drawLine(points[j], points[j+1 >= points.size()?0:j+1], Color4F::GREEN);
	}
	//对生成的简单多边形进行三角分解
	std::vector<short> triangle_list;
	gt::simple_polygon_ear_triangulate(points, triangle_list);

	int polygon_size = points.size();
	for (int j = 0; j < triangle_list.size(); j+= 3) {
		int prev_j = triangle_list[j];
		int target_j = triangle_list[j+1];
		int next_j = triangle_list[j+2];

		Color4F  color(gt::random(),gt::random(),gt::random(),1.0f);
		//对以上三边进行测试
		int abs_i = std::abs(prev_j - target_j);
		if (abs_i != 1 && abs_i != polygon_size)
			draw_node->drawLine(points[prev_j],points[target_j], color);

		abs_i = std::abs(target_j - next_j);
		if (abs_i != 1 && abs_i != polygon_size)
			draw_node->drawLine(points[target_j], points[next_j], color);

		abs_i = std::abs(next_j - prev_j);
		if (abs_i != 1 && abs_i != polygon_size)
			draw_node->drawLine(points[next_j], points[prev_j], color);
	}
	root_node->setCameraMask(s_CameraMask);
}