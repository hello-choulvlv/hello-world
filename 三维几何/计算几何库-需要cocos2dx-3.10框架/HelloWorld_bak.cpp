/*
  *测试函数所在地
  *2019年7月9日
  *@author:xiaohuaxiong
 */
#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "geometry.h"
#include "DrawNode3D.h"

USING_NS_CC;

void  HelloWorld::testRotateHullAlorithm()
{
	const int point_num = 67;
	//随机生成离散的点
	std::vector<Vec2>   points;
	points.reserve(point_num);
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
	}

	std::vector<Vec2> polygon_vector;
	bool check_polygon = gt::polygon_compute_convex_hull(points, polygon_vector);
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
		draw_polygon->drawLine(polygon_vector[index_l - 1], polygon_vector[index_l], color);
	draw_polygon->drawLine(polygon_vector.back(), polygon_vector.front(), color);

	root_node->addChild(draw_polygon);
}

void HelloWorld::testMinimumCycleNormal()
{
	const int point_num = 67;
	//随机生成离散的点
	std::vector<Vec2>   points;
	points.reserve(point_num);
	auto &winSize = _director->getWinSize();
	float cx = winSize.width * 0.5f;
	float cy = winSize.height * 0.5f;

	float w = 180.0f;
	float h = 220.0f;

	Node  *root_node = Node::create();
	root_node->setPosition(cx, cy);
	this->addChild(root_node);

	for (int index_j = 0; index_j < point_num; ++index_j)
	{
		points.push_back(Vec2(w * (2.0f * rand() / RAND_MAX - 1.0f), h * (2.0f * rand() / RAND_MAX - 1.0f)));
	}

	std::vector<Vec2> polygon_vector;
	gt::Cycle  cycle;
	gt::compute_points_cycle_normal(points, cycle);
	//建立sprite
	for (int index_j = 0; index_j < point_num; ++index_j)
	{
		Sprite *bubble_sprite = Sprite::create("llk_yd.png");
		bubble_sprite->setPosition(points[index_j]);
		root_node->addChild(bubble_sprite);
	}
	//画线
	DrawNode  *draw_polygon = DrawNode::create();
	root_node->addChild(draw_polygon);

	Color4F color(1.0f, 1.0f, 1.0f, 1.0f);
	draw_polygon->drawCircle(cycle.center, cycle.radius, 360, 2.0f * M_PI * cycle.radius * 0.25f, false, color);

	gt::Cycle  secondary_cycle;
	gt::compute_points_minimum_cycle(points, secondary_cycle);
	draw_polygon->drawCircle(secondary_cycle.center, secondary_cycle.radius, 360.0f, 2.0f * M_PI * secondary_cycle.radius * 0.25f, false, Color4F::RED);
}

void  HelloWorld::testCycleCreation()
{
	float  w = 150;
	float h = 200;
	Vec2   point1(w * (2.0f * rand() / RAND_MAX - 1.0f), h * (2.0f * rand() / RAND_MAX - 1.0f));
	Vec2   point2(w * (2.0f * rand() / RAND_MAX - 1.0f), h * (2.0f * rand() / RAND_MAX - 1.0f));
	Vec2   point3(w * (2.0f * rand() / RAND_MAX - 1.0f), h * (2.0f * rand() / RAND_MAX - 1.0f));

	Node  *root_node = Node::create();
	root_node->setPosition(_director->getWinSize().width * 0.5f, _director->getWinSize().height * 0.5f);
	this->addChild(root_node);

	DrawNode   *drawNode = DrawNode::create();
	root_node->addChild(drawNode);

	gt::Cycle  cycle;
	Color4F   color(1.0f, 1.0f, 1.0f, 1.0f);
	gt::cycle_create(point1, point2, cycle);
	drawNode->drawCircle(cycle.center, cycle.radius, 360, 2.0f * M_PI * cycle.radius / 4.0f, false, color);

	Sprite  *point_sprite1 = Sprite::create("llk_yd.png");
	point_sprite1->setPosition(point1);
	root_node->addChild(point_sprite1);

	Sprite  *point_sprite2 = Sprite::create("llk_yd.png");
	point_sprite2->setPosition(point2);
	root_node->addChild(point_sprite2);

	Sprite  *point_sprite3 = Sprite::create("llk_yd.png");
	point_sprite3->setPosition(point3);
	root_node->addChild(point_sprite3);

	gt::cycle_create(point1, point2, point3, cycle);
	drawNode->drawCircle(cycle.center, cycle.radius, 360, 2.0f * M_PI * cycle.radius / 4.0f, false, color);
}
/*
*大量的随机点表明,
*最小包围球算法得出的球体更为紧凑
*一般算法与最小算法相比,其扩张规模一半在[0-20]%之间
*有时也会出现完全重合的现象
*/
void  HelloWorld::testMinimumSphere()
{
	Node *root_node = Node::create();
	//root_node->setPosition(_director->getWinSize().width * 0.5f, _director->getWinSize().height * 0.5f);
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float w = 200;
	float h = 250;
	float d = 100.0f;
	Color4F	 color4(1, 1, 1, 1);
	//随机生成三点
	int    point_count = 4;
	std::vector<Vec3>  points;
	points.reserve(point_count);
	Sprite3D  *parent = Sprite3D::create();
	root_node->addChild(parent);

	std::vector<Sprite *>  sprites;
	sprites.reserve(point_count);
	for (int index_j = 0; index_j < point_count; ++index_j)
	{
		points.push_back(Vec3(w * (2.0f *rand() / RAND_MAX - 1.0f), h * (2.0f *rand() / RAND_MAX - 1.0f), d * (2.0f *rand() / RAND_MAX - 1.0f)));
		Sprite  *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition3D(points.back());
		sprite->setColor(Color3B::BLUE);
		parent->addChild(sprite);

		sprites.push_back(sprite);
	}

	root_node->setCameraMask(s_CameraMask);

	//最小包围球算法
	gt::Sphere  cycle3;
	gt::create_minimum_sphere4(points[0], points[1], points[2], points[3], cycle3);
	//draw_node->drawCircle(cycle3.center, cycle3.radius, 2.0f * M_PI * cycle3.radius / 4, cycle3.normal, Color4F::RED);
	draw_node->drawSphere(cycle3.center, cycle3.radius, 32, 32, Color4F::RED);
	CCLOG("minimum_sphere4->,%.2f,%.2f,%.2f,r->%.2f", cycle3.center.x, cycle3.center.y, cycle3.center.z, cycle3.radius);

	//再使用一般算法
	//gt::create_sphere4(points[0], points[1], points[2], points[3], cycle3);
	gt::compute_minimum_sphere_normal(points, cycle3);
	draw_node->drawSphere(cycle3.center, cycle3.radius, 32, 32, Color4F::WHITE);
	//检测点的相对位置
	for (int index_j = 0; index_j < point_count; ++index_j)
	{
		if (!check_point_insideof_sphere(cycle3, points[index_j]))// sprites[index_j]->getPosition3D()))
			sprites[index_j]->setColor(Color3B::RED);
	}
	CCLOG("minimum_sphere4->,%.2f,%.2f,%.2f,r->%.2f", cycle3.center.x, cycle3.center.y, cycle3.center.z, cycle3.radius);
}

void  HelloWorld::testMinimumSphere4Plane()
{
	Node *root_node = Node::create();
	//root_node->setPosition(_director->getWinSize().width * 0.5f, _director->getWinSize().height * 0.5f);
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float w = 200;
	float h = 250;
	float d = 100.0f;
	//随机生成三点
	Vec3  v1(w * (2.0f *rand() / RAND_MAX - 1.0f), h * (2.0f *rand() / RAND_MAX - 1.0f), 0);
	Vec3  v2(w * (2.0f *rand() / RAND_MAX - 1.0f), h * (2.0f *rand() / RAND_MAX - 1.0f), 0);
	Vec3 v3(w * (2.0f *rand() / RAND_MAX - 1.0f), h * (2.0f *rand() / RAND_MAX - 1.0f), 0);
	Vec3 v4(w * (2.0f *rand() / RAND_MAX - 1.0f), h * (2.0f *rand() / RAND_MAX - 1.0f), 0);

	//增加上点标志
	Sprite3D  *parent = Sprite3D::create();
	parent->setForce2DQueue(true);

	root_node->addChild(parent);
	Sprite  *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(v1);
	sprite->setColor(Color3B::RED);
	parent->addChild(sprite);

	sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(v2);
	sprite->setColor(Color3B::GREEN);
	parent->addChild(sprite);

	sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(v3);
	sprite->setColor(Color3B::BLUE);
	parent->addChild(sprite);

	sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(v4);
	parent->addChild(sprite);

	root_node->setCameraMask(s_CameraMask);

	gt::Sphere  cycle3;
	gt::create_minimum_sphere4_plane(v1, v2, v3, v4, cycle3);
	//gt::create_sphere3(v1, v2, v3, cycle3);
	draw_node->drawCircle(cycle3.center, cycle3.radius, 2.0f * M_PI * cycle3.radius / 4, Vec3(0, 0, 1), Color4F::RED);
}

void  HelloWorld::testOBBIntersect()
{
	//随机生成两个OBB
	const float  extent[3] = { 200,200,200 };//OBB半径
											 //第一个
	Vec3  min_bb1(extent[0] * (2.0f *rand() / RAND_MAX - 1.0f), extent[1] * (2.0f *rand() / RAND_MAX - 1.0f), extent[2] * (2.0f *rand() / RAND_MAX - 1.0f));
	Vec3  max_bb1 = min_bb1 + Vec3(extent[0] * (0.4f + 1.0f * rand() / RAND_MAX), extent[1] * (0.4f + 1.0f *rand() / RAND_MAX), extent[2] * (0.4f + 1.0f * rand() / RAND_MAX));
	Vec3  axis1 = gt::normalize(Vec3(2.0f *rand() / RAND_MAX - 1.0f, 2.0f *rand() / RAND_MAX - 1.0f, 2.0f *rand() / RAND_MAX - 1.0f));
	float   angle1 = 360.0f * rand() / RAND_MAX;
	gt::OBB	obb1;
	gt::obb_create(obb1, min_bb1, max_bb1, axis1, angle1);
	//gt::obb_create(obb1, Vec3(-150,-150,-150),Vec3(0,0,0));

	//第二个
	Vec3  min_bb2(extent[0] * (2.0f *rand() / RAND_MAX - 1.0f), extent[1] * (2.0f *rand() / RAND_MAX - 1.0f), extent[2] * (2.0f *rand() / RAND_MAX - 1.0f));
	Vec3  max_bb2 = min_bb2 + Vec3(extent[0] * (0.4f + 1.0f * rand() / RAND_MAX), extent[1] * (0.4f + 1.0f * rand() / RAND_MAX), extent[2] * (0.4f + 1.0f * rand() / RAND_MAX));
	Vec3  axis2 = gt::normalize(Vec3(2.0f *rand() / RAND_MAX - 1.0f, 2.0f *rand() / RAND_MAX - 1.0f, 2.0f *rand() / RAND_MAX - 1.0f));
	float   angle2 = 360.0f * rand() / RAND_MAX;
	gt::OBB	obb2;
	gt::obb_create(obb2, min_bb2, max_bb2, axis2, angle2);
	//gt::obb_create(obb2, Vec3(-10, -10, -10), Vec3(100, 100, 100));
	Color4F   intersect_color = Color4F::WHITE, yColor = Color4F::GREEN, zColor = Color4F::BLUE;
	if (gt::obb_obb_intersect_test(obb1, obb2))
		intersect_color = Color4F::RED;
	//画出Cube
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);
	root_node->setCameraMask(s_CameraMask);

	Vec3 vertex[8];
	obb_create_obb_vertex8(obb1, vertex);
	draw_node->drawCube2(vertex, intersect_color, yColor, zColor);

	obb_create_obb_vertex8(obb2, vertex);
	draw_node->drawCube2(vertex, intersect_color, yColor, zColor);
}

void HelloWorld::testSegmentDistance()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 300.0f;
	//随机生成四个点
	Vec3  points[4];
	Sprite *sprite_mask[4];
	for (int index_j = 0; index_j < 4; ++index_j)
	{
		points[index_j] = Vec3(length_l * (1.0f * rand() / RAND_MAX), length_l * (1.0f * rand() / RAND_MAX), length_l * (1.0f * rand() / RAND_MAX));
		sprite_mask[index_j] = Sprite::create("llk_yd.png");
		sprite_mask[index_j]->setPosition3D(points[index_j]);
		root_node->addChild(sprite_mask[index_j]);
	}

	Vec3 intersect_apoint, intersect_bpoint;
	gt::Segment  seg1 = {
		points[0],points[1],
	},
	seg2 = {
		points[2],points[3],
	};
	gt::Line  line1, line2;

	draw_node->drawLine(points[0], points[1], Color4F::RED);
	draw_node->drawLine(points[2], points[3], Color4F::BLUE);

	gt::segment_segment_minimum_distance(seg1, seg2, intersect_apoint, intersect_bpoint);

	Sprite *asprite = Sprite::create("llk_yd.png");
	asprite->setPosition3D(intersect_apoint);
	asprite->setColor(Color3B::RED);
	root_node->addChild(asprite);

	asprite = Sprite::create("llk_yd.png");
	asprite->setPosition3D(intersect_bpoint);
	asprite->setColor(Color3B::RED);
	root_node->addChild(asprite);

	gt::line_create(line1, points[0], points[1]);
	gt::line_create(line2, points[2], points[3]);
	gt::line_line_minimum_distance(line1, line2, intersect_apoint, intersect_bpoint);

	asprite = Sprite::create("llk_yd.png");
	asprite->setPosition3D(intersect_apoint);
	asprite->setColor(Color3B::GREEN);
	root_node->addChild(asprite);

	asprite = Sprite::create("llk_yd.png");
	asprite->setPosition3D(intersect_bpoint);
	asprite->setColor(Color3B::GREEN);
	root_node->addChild(asprite);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::pointToPlane()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 300.0f;
	Vec3  points[4];

	for (int index_j = 0; index_j < 4; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition3D(points[index_j]);
		root_node->addChild(sprite);
	}

	draw_node->drawTriangle(points[0], points[1], points[2], Color4F::WHITE);

	gt::Plane  plane;
	gt::plane_create(plane, points[0], points[1], points[2]);

	Vec3 intersect_point;
	gt::plane_point_distance(points[3], plane, intersect_point);

	draw_node->drawLine(points[3], intersect_point, Color4F::RED);
	draw_node->drawLine(intersect_point, points[0], Color4F::RED);
	draw_node->drawLine(intersect_point, points[1], Color4F::RED);
	draw_node->drawLine(intersect_point, points[2], Color4F::RED);

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(points[3]);
	sprite->setColor(Color3B::RED);
	root_node->addChild(sprite);

	sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(intersect_point);
	sprite->setColor(Color3B::RED);
	root_node->addChild(sprite);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::pointToSegment()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 400.0f;
	Vec3  points[3];

	for (int index_j = 0; index_j < 3; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition3D(points[index_j]);
		root_node->addChild(sprite);
	}

	gt::Segment  segment;
	gt::segment_create(segment, points[0], points[1]);
	draw_node->drawLine(points[0], points[1], Color4F::BLUE);

	Vec3 intersect_point;
	gt::line_point_minimum_distance(points[2], segment, &intersect_point);
	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(intersect_point);
	root_node->addChild(sprite);

	draw_node->drawLine(intersect_point, points[2], Color4F::RED);
	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::pointToAABB()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 400.0f;
	Vec3  points[2];

	for (int index_j = 0; index_j < 2; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
		//Sprite *sprite = Sprite::create("llk_yd.png");
		//sprite->setPosition3D(points[index_j]);
		//root_node->addChild(sprite);
	}

	Vec3 extent(length_l * (0.3f + gt::random()), length_l * (0.3f + gt::random()), length_l * (0.3f + gt::random()));
	Vec3 max_vertex = points[0] + extent;

	draw_node->drawAABB(points[0], max_vertex, Color4F::WHITE);

	gt::AABB  aabb;
	gt::aabb_create(aabb, points[0], max_vertex);

	Vec3 intersect_point;
	gt::aabb_point_minimum_distance(aabb, points[1], intersect_point);

	draw_node->drawLine(points[1], intersect_point, Color4F::RED);

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(points[1]);
	root_node->addChild(sprite);

	sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(intersect_point);
	root_node->addChild(sprite);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::pointToOBB()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 400.0f;
	Vec3  points[2];

	for (int index_j = 0; index_j < 2; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	Vec3 extent(length_l * (0.3 + gt::random()), length_l * (0.3 + gt::random()), length_l * (0.3 + gt::random()));
	Vec3 axis = gt::normalize(gt::randomf10(), gt::randomf10(), gt::randomf10());
	float angle = 360.0f * gt::random();

	gt::OBB obb;
	gt::obb_create(obb, points[0], points[0] + extent, axis, angle);

	Vec3 vertex[8];
	gt::obb_create_obb_vertex8(obb, vertex);
	draw_node->drawCube2(vertex, Color4F::RED, Color4F::GREEN, Color4F::BLUE);

	Vec3 intersect_point;
	gt::obb_point_minimum_distance(obb,points[1], intersect_point);

	draw_node->drawLine(points[1], intersect_point, Color4F::MAGENTA);

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(points[1]);
	root_node->addChild(sprite);

	sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(intersect_point);
	root_node->addChild(sprite);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::pointToRectangle()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	Vec3  points[2];

	for (int index_j = 0; index_j < 2; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	//Vec3 extent(length_l * (0.3 + gt::random()),length_l * (0.3 + gt::random()), 0);
	Vec3 axis(gt::randomf10(), gt::randomf10(), gt::randomf10());
	float angle = 360 * gt::random();

	//gt::Rectangle rectangle;
	//gt::gtrectangle_create(rectangle, points[0], points[0] + extent,axis,angle);

	Vec2 extent(length_l * (0.3 + gt::random()), length_l * (0.3 + gt::random()));
	gt::Rectangle3v rectangle;
	gt::rectangle3v_create(rectangle, points[0], extent, axis, angle);

	Vec3 vertex[4];
	//gt::gtrectangle_get_vertex(rectangle, vertex);
	gt::rectangle3v_get_vertex(rectangle, vertex);
	draw_node->drawLineCircle(vertex, 4, Color4F::WHITE);

	Vec3 intersect_point;
	//gt::gtrectangle_point_min_distance(rectangle,points[1],intersect_point);
	gt::rectangle3v_point_min_distance(rectangle, points[1], intersect_point);

	draw_node->drawLine(points[1], intersect_point, Color4F::MAGENTA);

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(points[1]);
	root_node->addChild(sprite);

	sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(intersect_point);
	root_node->addChild(sprite);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::segmentToTriangle()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	const int array_size = 5;

	Vec3  points[array_size];

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}
	//画出线段
	draw_node->drawLine(points[3], points[4], Color4F::BLUE);

	Vec3 segemtn_point, triangle_point;
	gt::Segment segment1 = {
		points[2],points[0],
	}, segment2 = {
		points[3],points[4],
	};
#if 0
	float f1 = gt::segment_segment_minimum_distance(segment1, segment2, segemtn_point, triangle_point);
	draw_node->drawLine(points[2], points[0], Color4F::WHITE);
	draw_node->drawLine(triangle_point, segemtn_point, Color4F::RED);
	//使用直线算法
	gt::Line  line1, line2;
	gt::line_create(line1, points[2], points[0]);
	gt::line_create(line2, points[3], points[4]);
	float f2 = gt::line_line_minimum_distance(line1, line2, segemtn_point, triangle_point);
	draw_node->drawLine(triangle_point, segemtn_point, Color4F::GREEN);
#endif
#if 1 
	//画出三角形
	draw_node->drawTriangle(points, Color4F::WHITE);
	draw_node->drawLine(points[3], points[4], Color4F::BLUE);
	gt::Triangle triangle;
	gt::triangle_create(triangle, points);
	gt::triangle_segment_distance(triangle, points + 3, triangle_point, segemtn_point);
#endif
	draw_node->drawLine(triangle_point, segemtn_point, Color4F::RED);

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(triangle_point);
	root_node->addChild(sprite);

	sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(segemtn_point);
	root_node->addChild(sprite);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doAABBAndOBBIntersectWithPlane()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	const int array_size = 3;
	//三角形平面
	Vec3  points[array_size];

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	Vec3 center = (points[0] + points[1] + points[2]) * 0.333333f;
	Vec3 normal = gt::cross_normalize(points[1] - points[0], points[2] - points[0]);
	Vec2 extent(1000, 1000);
	draw_node->drawMesh(center, normal, extent, 64, 64, Color4F::GREEN);
	//draw_node->drawLine(center, center + normal * 200.0f, Color4F::RED);

	gt::Plane plane;
	gt::plane_create(plane, normal, center);

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(center);
	root_node->addChild(sprite);

	//创建AABB包围盒
	Vec3 bb_min(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	Vec3 bb_extent(length_l * (0.3f + gt::random()), length_l * (0.3f + gt::random()), length_l * (0.3f + gt::random()));
#if 0
	gt::AABB  aabb;
	gt::aabb_create(aabb, bb_min, bb_min + bb_extent);
	//画出AABB
	draw_node->drawAABB(bb_min, aabb.bb_max, gt::aabb_plane_intersect_test(aabb, plane) ? Color4F::RED : Color4F::BLUE);
#endif
	//OBB相交测试
	gt::OBB obb;
	gt::obb_create(obb, bb_min, bb_min + bb_extent, gt::normalize(Vec3(gt::randomf10(), gt::randomf10(), gt::randomf10())), 360.0f * gt::random());

	const Color4F &color = gt::obb_plane_intersect_test(obb, plane) ? Color4F::RED : Color4F::BLUE;
	Vec3 vertex[8];
	gt::obb_create_obb_vertex8(obb, vertex);
	draw_node->drawCube2(vertex, color, color, color);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::pointToTriangle()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	Vec3  points[4];

	for (int index_j = 0; index_j < 4; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(points[3]);
	root_node->addChild(sprite);

	gt::Triangle triangle;
	gt::triangle_create(triangle, points);

	Vec3 intersect_point;
	gt::triangle_point_min_distance(triangle, points[3], intersect_point);

	draw_node->drawTriangle(points, Color4F::WHITE);

	sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(intersect_point);
	root_node->addChild(sprite);

	draw_node->drawLine(points[3], intersect_point, Color4F::RED);

	draw_node->drawLine(intersect_point, points[0], Color4F::BLUE);
	draw_node->drawLine(intersect_point, points[1], Color4F::BLUE);
	draw_node->drawLine(intersect_point, points[2], Color4F::BLUE);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::pointToTetrahedron()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	const int array_size = 5;

	Vec3  points[array_size];

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(points[4]);
	root_node->addChild(sprite);
	//
	draw_node->drawTetrahedron(points, Color4F::WHITE);
	//创建四面体
	gt::Tetrahedron   tet;
	gt::tetrahedron_create(tet, points);

	Vec3 intersect_point;
	gt::tetrahedron_point_min_distance(points, points[4], intersect_point);

	sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(intersect_point);
	root_node->addChild(sprite);

	draw_node->drawLine(points[4], intersect_point, Color4F::RED);

	root_node->setCameraMask(s_CameraMask);
}


void HelloWorld::doConeIntersectWithPlane()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	const int array_size = 3;
	//三角形平面
	Vec3  points[array_size];

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	Vec3 center = (points[0] + points[1] + points[2]) * 0.333333f;
	Vec3 normal = gt::cross_normalize(points[1] - points[0], points[2] - points[0]);
	Vec2 extent(1000, 1000);
	draw_node->drawMesh(center, normal, extent, 64, 64, Color4F::GREEN);

	gt::Plane plane;
	gt::plane_create(plane, normal, center);

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(center);
	root_node->addChild(sprite);
	//创建锥形体
	const Vec3 top(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	const Vec3 normal2 = gt::normalize(Vec3(gt::randomf10(), gt::randomf10(), gt::randomf10()));
	float h = length_l * (0.6f + gt::random() * 0.4f);
	float r = length_l * (0.2f + 0.5 * gt::random());
	gt::Cone cone;
	gt::cone_create(cone, top, normal2, h, r);

	draw_node->drawCone(top, normal2, h, r, 16, 24, gt::cone_plane_intersect_test(cone, plane) ? Color4F::RED : Color4F::BLUE);


	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doAABBAndOBBIntersectWithSphere()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;

	Vec3 min_bb(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	Vec3 extent(length_l * (0.3f + gt::random() * 0.7f), length_l * (0.3f + gt::random() * 0.7f), length_l * (0.3f + gt::random() * 0.7f));

	Vec3 center(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	float radius = length_l * (0.4f + gt::random() * 0.6f);
	gt::Sphere  sphere;
	gt::sphere_create(sphere, center, radius);
#if 0
	gt::AABB aabb;
	gt::aabb_create(aabb, min_bb, min_bb + extent);
	draw_node->drawAABB(min_bb, aabb.bb_max, Color4F::WHITE);

	draw_node->drawSphere(center, radius, 64, 64, gt::aabb_sphere_intersect_test(aabb, sphere) ? Color4F::RED : Color4F::GREEN);
#else
	Vec3 normal = gt::normalize(gt::randomf10(), gt::randomf10(), gt::randomf10());
	float angle = 360 * gt::random();
	gt::OBB  obb;
	gt::obb_create(obb, min_bb, min_bb + extent, normal, angle);
	Vec3 vertex[8];
	gt::obb_create_obb_vertex8(obb, vertex);
	draw_node->drawCube2(vertex, Color4F::WHITE, Color4F::WHITE, Color4F::WHITE);

	draw_node->drawSphere(center, radius, 64, 64, gt::obb_sphere_intersect_test(obb, sphere) ? Color4F::RED : Color4F::GREEN);
#endif

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doTriangleIntersectWithSphere()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	const int array_size = 3;
	//三角形平面
	Vec3  points[array_size];

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	gt::Triangle triangle;
	gt::triangle_create(triangle, points);

	draw_node->drawTriangle(points, Color4F::WHITE);

	Vec3 center(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	float radius = length_l * (0.4f + gt::random() * 0.6f);
	gt::Sphere  sphere;
	gt::sphere_create(sphere, center, radius);

	draw_node->drawSphere(center, radius, 64, 64, gt::triangle_sphere_intersect_test(triangle, sphere) ? Color4F::RED : Color4F::GREEN);
	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doAABBAndOBBIntersectWithTriangle()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	const int array_size = 3;
	//三角形平面
	Vec3  points[array_size];

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	gt::Triangle triangle;
	gt::triangle_create(triangle, points);
	draw_node->drawTriangle(points, Color4F::WHITE);

	//AABB
	Vec3  bb_min(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	Vec3 extent(length_l * (0.3f + gt::random() * 0.7f), length_l * (0.3f + gt::random() * 0.7f), length_l * (0.3f + gt::random() * 0.7f));
#if 0
	gt::AABB  aabb;
	gt::aabb_create(aabb, bb_min, bb_min + extent);
	draw_node->drawAABB(bb_min, aabb.bb_max, gt::aabb_triangle_intersect_test(aabb, triangle) ? Color4F::RED : Color4F::GREEN);
#else
	const Vec3 normal = gt::normalize(gt::randomf10(), gt::randomf10(), gt::randomf10());
	float radius = 360.0f * gt::random();
	gt::OBB obb;
	gt::obb_create(obb, bb_min, bb_min + extent, normal, radius);
	Vec3  vertex[8];
	gt::obb_create_obb_vertex8(obb, vertex);
	const Color4F  &color = gt::obb_triangle_intersect_test(obb, triangle) ? Color4F::RED : Color4F::GREEN;
	draw_node->drawCube2(vertex, color, color, color);
#endif
	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doTriangleAndTriangleInttersect()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	const int array_size = 3;
	//三角形平面
	Vec3  points[array_size];

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	gt::Triangle triangle;
	gt::triangle_create(triangle, points);

	Vec3 points2[3];
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points2[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	gt::Triangle triangle2;
	gt::triangle_create(triangle2, points2);

	bool intersect = gt::triangle_triangle_intersect_test(triangle, triangle2);
	draw_node->drawTriangle(points, intersect ? Color4F::RED : Color4F::GREEN);
	draw_node->drawTriangle(points2, intersect ? Color4F::RED : Color4F::GREEN);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doRayAndSegmentIntersectWithSphere()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	const Vec3 center(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	float radius = length_l * (0.4f + 0.6f * gt::random());

	gt::Sphere sphere;
	gt::sphere_create(sphere, center, radius);
	draw_node->drawSphere(center, radius, 64, 64, Color4F::GREEN);

	const Vec3 start_point(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	const Vec3 final_point(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	gt::Segment segment;
	gt::segment_create(segment, start_point, final_point);

	draw_node->drawLine(start_point, final_point, gt::segment_sphere_intersect_test(segment, sphere) ? Color4F::RED : Color4F::WHITE);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doRayIntersectWithAABB()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	const Vec3 center(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	const Vec3 extent(length_l * (0.3f + gt::random()), length_l * (0.3f + gt::random()), length_l * (0.3f + gt::random()));

	gt::AABB  aabb;
	gt::aabb_create(aabb, center - extent * 0.5f, center + extent * 0.5f);

	const Vec3 origin(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	const Vec3 normal(gt::randomf10(), gt::randomf10(), gt::randomf10());

	gt::Ray  ray;
	gt::ray_create(ray, origin, normal);

	bool b = gt::ray_aabb_intersect_test(ray, aabb);

	draw_node->drawAABB(aabb.bb_min, aabb.bb_max, b ? Color4F::RED : Color4F::GREEN);
	draw_node->drawLine(origin, origin + ray.direction * length_l, b ? Color4F::RED : Color4F::GREEN);

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(origin);
	root_node->addChild(sprite);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doRayIntersectWithOBB()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	const Vec3 center(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	const Vec3 extent(length_l * (0.3f + gt::random()), length_l * (0.3f + gt::random()), length_l * (0.3f + gt::random()));
	const Vec3 normal = gt::normalize(gt::randomf10(), gt::randomf10(), gt::randomf10());
	float angle = 360.0f * gt::random();

	gt::OBB  obb;
	gt::obb_create(obb, center - extent * 0.5f, center + extent * 0.5f, normal, angle);

	const Vec3 origin(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());

	gt::Ray  ray;
	gt::ray_create(ray, origin, gt::normalize(gt::randomf10(), gt::randomf10(), gt::randomf10()));

	bool b = gt::ray_obb_intersect_test(ray, obb);

	Vec3 vertex[8];
	gt::obb_create_obb_vertex8(obb, vertex);

	const Color4F &color = b ? Color4F::RED : Color4F::GREEN;
	draw_node->drawCube2(vertex, color, color, color);
	draw_node->drawLine(origin, origin + ray.direction * (2.0f * length_l), color);

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(origin);
	root_node->addChild(sprite);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doAABBAndOBBIntersectWithSegment()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	const Vec3 center(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	const Vec3 extent(length_l * (0.3f + gt::random()), length_l * (0.3f + gt::random()), length_l * (0.3f + gt::random()));

	const Vec3 origin(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());

	gt::Segment segment;
	gt::segment_create(segment, origin, origin + gt::normalize(gt::randomf10(), gt::randomf10(), gt::randomf10()) * length_l);

#if 1
	const Vec3 normal = gt::normalize(gt::randomf10(), gt::randomf10(), gt::randomf10());
	float angle = 360.0f * gt::random();

	gt::OBB  obb;
	gt::obb_create(obb, center - extent * 0.5f, center + extent * 0.5f, normal, angle);
	bool b = gt::obb_segment_intersect_test(obb, segment);

	Vec3 vertex[8];
	gt::obb_create_obb_vertex8(obb, vertex);

	const Color4F &color = b ? Color4F::RED : Color4F::GREEN;
	draw_node->drawCube2(vertex, color, color, color);
#else
	gt::AABB aabb;
	gt::aabb_create(aabb, center - extent * 0.5f, center + extent * 0.5f);
	bool b = gt::aabb_segment_intersect_test(aabb, segment);
	const Color4F &color = b ? Color4F::RED : Color4F::GREEN;
	draw_node->drawAABB(aabb.bb_min, aabb.bb_max, color);
#endif

	draw_node->drawLine(segment.start_point, segment.final_point, color);
	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doLineIntersectWithTriangle()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;

	const int array_size = 3;
	//三角形平面
	Vec3  points[array_size];

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	const Vec3 origin(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	const Vec3 secondary(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());

	gt::Line line;
	gt::line_create(line, origin, secondary);

	gt::Triangle triangle;
	gt::triangle_create(triangle, points);

	bool b = gt::line_triangle_intersect_test(line, triangle);
	//bool b = gt::triangle_line_intersect_test(triangle, line);

	draw_node->drawLine(line.start_point, line.start_point + line.direction * length_l * 3.0f, b ? Color4F::RED : Color4F::GREEN);
	draw_node->drawTriangle(points, b ? Color4F::RED : Color4F::GREEN);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doSegmentIntersectWithTriangle()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;

	const int array_size = 3;
	//三角形平面
	Vec3  points[array_size];

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec3(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	}

	//const Vec3 v1 = (points[0] + points[1]) * 0.5f;
	//const Vec3 direction = points[2] - v1;
	const Vec3 origin(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	const Vec3 secondary(length_l * (0.0f + 0.3f * gt::randomf10()), length_l * (0.0f + 0.3f * gt::randomf10()), length_l * (0.3f + gt::randomf10()));

	gt::Segment segment;
	gt::segment_create(segment, origin, secondary);

	gt::Triangle triangle;
	gt::triangle_create(triangle, points);

	bool b = gt::segment_triangle_intersect_test(segment, triangle);
	//bool b = gt::triangle_segment_intersect_test(triangle, segment);

	draw_node->drawLine(segment.start_point, segment.final_point, b ? Color4F::RED : Color4F::GREEN);
	draw_node->drawTriangle(points, b ? Color4F::RED : Color4F::GREEN);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doCylinderIntersectWithLineAndSegment()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode3D  *draw_node = DrawNode3D::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	//圆柱体
	const Vec3 bottom(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	const Vec3 top(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	float radius = length_l * (0.3 + 0.3 * gt::random());

	gt::Cylinder cylinder;
	gt::cylinder_create(cylinder, bottom, top, radius);

	//直线
	Vec3 origin(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());
	Vec3 other(length_l * gt::randomf10(), length_l * gt::randomf10(), length_l * gt::randomf10());

#if 0
	gt::Line line;
	gt::line_create(line, origin, other);

	bool b = gt::cylinder_line_intersect_test(cylinder, line);
	const Color4F &color = b ? Color4F::RED : Color4F::GREEN;
	draw_node->drawLine(origin, line.direction * length_l * 3.0f, color);
#else
#if 0
	origin = top + cylinder.direction * 2;
	other = origin + cylinder.direction * 100.0f;
#endif
	gt::Segment segment;
	gt::segment_create(segment, origin, other);

	bool b = gt::cylinder_segment_intersect_test(cylinder, segment);
	const Color4F &color = b ? Color4F::RED : Color4F::GREEN;
	draw_node->drawLine(segment.start_point, segment.final_point, color);
#endif

	draw_node->drawCylinder(bottom, cylinder.direction, cylinder.length, radius, 16, 32, 32, color);


	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(origin);
	root_node->addChild(sprite);

	sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(bottom);
	root_node->addChild(sprite);

	sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(top);
	root_node->addChild(sprite);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doLineRaySegementIntersectWithPolygon()
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

	std::vector<Vec2>  points_vec;
	std::vector<gt::Plane2D>   planes;

	//计算最小凸多边形
	gt::polygon_compute_convex_hull(points, points_vec);
	//计算2d超平面
	gt::Polygon  polygon;
	gt::polygon_create(polygon, points_vec);

	//随机生成两点
	float scale_factor = 2.0f;
	const Vec3 origin(length_l * scale_factor * gt::randomf10(), length_l * scale_factor * gt::randomf10(), 0.0f);//Vec3(points_vec[0].x, points_vec[0].y,0) - Vec3(10,10,0);//
	const Vec3 bottom(length_l * scale_factor * gt::randomf10(), length_l * scale_factor * gt::randomf10(), 0.0f);//Vec3(points_vec[1].x, points_vec[1].y, 0) - Vec3(10, 10, 0);;// 
																												  //直线
#if 0
	gt::Line line;
	gt::line_create(line, origin, bottom);
	bool b = gt::polygon_line_intersect_test(polygon, line);
#endif

#if 0
	gt::Segment  segment;
	gt::segment_create(segment, origin, bottom);
	bool b = gt::polygon_segment_intersect_test(polygon, segment);
#endif

	gt::Ray  ray;
	gt::ray_create(ray, origin, bottom - origin);
	bool b = gt::polygon_ray_intersect_test(polygon, ray);

	const Color4F &color = b ? Color4F::RED : Color4F::GREEN;
	/*
	*画出直线条带
	*/
	draw_node->drawPoly(points_vec.data(), points_vec.size(), true, color);
	draw_node->drawLine(*(Vec2*)&origin, *(Vec2*)&bottom, color);

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition3D(origin);
	root_node->addChild(sprite);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doPolygonContainsPoint()
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

	std::vector<Vec2>  points_vec;
	std::vector<gt::Plane2D>   planes;

	//计算最小凸多边形
	gt::polygon_compute_convex_hull(points, points_vec);
	//计算2d超平面
	gt::Polygon  polygon;
	gt::polygon_create(polygon, points_vec);

	//随机生成离散点
	const Vec2 point(length_l * 1.0f * gt::randomf10(), length_l * 1.0f * gt::randomf10());

	bool b = gt::polygon_contains_point(polygon, point);
	const Color4F &color = b ? Color4F::RED : Color4F::GREEN;

	draw_node->drawPoly(points_vec.data(), points_vec.size(), true, color);

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition(point);
	root_node->addChild(sprite);
	sprite->setColor(Color3B(color));

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::doPolygonCycleIntersectTest()
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

	std::vector<Vec2>  points_vec;
	std::vector<gt::Plane2D>   planes;

	//计算最小凸多边形
	gt::polygon_compute_convex_hull(points, points_vec);
	//计算2d超平面
	gt::Polygon  polygon;
	gt::polygon_create(polygon, points_vec);

	//随机生成离散点
	const Vec2 point(length_l * 2.0f * gt::randomf10(), length_l * 2.0f * gt::randomf10());
	float radius = length_l * gt::random();

	gt::Cycle cycle;
	gt::cycle_create(cycle, point, radius);

	bool b = gt::polygon_cycle_intersect_test(polygon, cycle);
	const Color4F &color = b ? Color4F::RED : Color4F::GREEN;

	draw_node->drawPoly(points_vec.data(), points_vec.size(), true, color);
	draw_node->drawCircle(point, radius, 360.0f, 2.0f * M_PI * radius / 5.0f, false, color);

	root_node->setCameraMask(s_CameraMask);
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
	gt::plane_create(plane1, points[0], points[1], points[2]);
	gt::plane_create(plane2, points[3], points[4], points[5]);

	gt::Line  line;
	bool b = gt::plane_plane_intersect_test(plane1, plane2, line);
	const Color4F &color = b ? Color4F::RED : Color4F::GREEN;

	draw_node->drawMesh((points[0] + points[1] + points[2])* 0.333f, plane1.normal, Vec2(length_l, length_l), 32, 32, Color4F::WHITE);
	draw_node->drawMesh((points[3] + points[4] + points[5])* 0.333f, plane2.normal, Vec2(length_l, length_l), 32, 32, Color4F::WHITE);

	if (b)
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

	gt::Plane   plane1, plane2, plane3;
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
	float length_l = 300.0f;
	////////////////////////////////////////////
	//画出有向矩形OR
	const Vec2 corner(length_l * gt::randomf10(), length_l * gt::randomf10());
	const Vec2 other(length_l * gt::randomf10(), length_l * gt::randomf10());
	const Vec2 normal = gt::normalize(other - corner);

	draw_node->drawLine(corner, other, Color4F::RED);

	float line_length = sqrtf(gt::length2(corner - other));
	//计算逻辑上下方的两个端点
	Vec2 v0 = corner + world_center;
	float unit_x = winSize.width / horizontal_num;
	float unit_y = winSize.height / vertical_num;
	//
	int start_x = v0.x / unit_x;
	int start_y = v0.y / unit_y;
	Vec2 pixel(start_x * unit_x, start_y * unit_y);
	//所有已经法线的相交区域
	std::vector<Vec2>   grid_map;
	Vec2 delta, accumulate;
	int step_x, step_y;
	if (normal.x == 0.0f)
	{
		accumulate.x = FLT_MAX;
		step_x = 0;
	}
	else if (normal.x > 0.0f)
	{
		accumulate.x = (pixel.x + unit_x - v0.x) / normal.x;
		step_x = 1;
	}
	else
	{
		accumulate.x = (pixel.x - v0.x) / normal.x;
		step_x = -1;
	}

	if (normal.y == 0.0f)
	{
		accumulate.y = FLT_MAX;
		step_y = 0;
	}
	else if (normal.y > 0.0f)
	{
		accumulate.y = (pixel.y + unit_y - v0.y) / normal.y;
		step_y = 1;
	}
	else
	{
		accumulate.y = (pixel.y - v0.y) / normal.y;
		step_y = -1;
	}
	delta.x = step_x * unit_x / (normal.x == 0.0f ? 0.0001f : normal.x);
	delta.y = step_y * unit_y / (normal.y == 0.0f ? 0.0001f : normal.y);
	int loop_count = 0;
	float extra_l = FLT_MAX;
	if (normal.x != 0)
		extra_l = fabsf(unit_x / normal.x);
	if (normal.y != 0)
		extra_l = fminf(extra_l, fabsf(unit_y / normal.y));

	int final_x = (other.x + world_center.x) / unit_x;
	int final_y = (other.y + world_center.y) / unit_y;

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
		float min_f = fminf(accumulate.x, accumulate.y);
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

	draw_node->drawLine(Vec2(start_x * unit_x, start_y * unit_y) - world_center, Vec2(final_x * unit_x, final_y * unit_y) - world_center, Color4F::BLUE);

	int delta_x = final_x - start_x;
	int delta_y = final_y - start_y;
	int abs_x = delta_x >= 0 ? delta_x : -delta_x;
	int abs_y = delta_y >= 0 ? delta_y : -delta_y;

	int step_x = final_x > start_x ? 1 : (final_x < start_x ? -1 : 0);
	int step_y = final_y > start_y ? 1 : (final_y < start_y ? -1 : 0);
	std::vector<cocos2d::Vec2>  location_array;
	if (abs_x >= abs_y)
	{
		int loops_count = abs_x + 1;
		int e = 0;
		for (int x = 0; x < loops_count; ++x)
		{
			e += abs_y << 1;
			location_array.push_back(Vec2(start_x, start_y));
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
			location_array.push_back(Vec2(start_x, start_y));
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
		const Vec2 &secondary = location_array[index_l + 1 >= location_array.size() ? 0 : index_l + 1];
		draw_node->drawLine(Vec2(location.x * unit_x, location.y * unit_y) - world_center, Vec2(secondary.x * unit_x, secondary.y * unit_y) - world_center, Color4F::RED);
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
		Vec2 start_point(winSize.width * gt::random(), winSize.height * gt::random());
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
		float length_l = 40 * (0.1 + gt::random());
		segment_array[index_l].final_point = segment_array[index_l].start_point + gt::normalize(segment_array[index_l].final_point - segment_array[index_l].start_point) * length_l;
		//画出线段
		draw_node->drawLine(segment_array[index_l].start_point, segment_array[index_l].final_point, Color4F::GREEN);
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
		point_array[index_l] = Vec2(length_l * gt::randomf10(), length_l * gt::randomf10());
		Sprite  * sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(point_array[index_l]);
		root_node->addChild(sprite);
	}
	//计算多边形
	std::vector<Vec2> polygon_points;
	polygon_points.reserve(array_size);
	gt::polygon_compute_convex_hull(point_array, polygon_points);
	//画出多边形的边
	for (int index_l = 0; index_l < polygon_points.size(); ++index_l)
		draw_node->drawLine(polygon_points[index_l], polygon_points[index_l + 1 >= polygon_points.size() ? 0 : index_l + 1], Color4F::WHITE);
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

	gt::Polygon  polygon1, polygon2;
	std::vector<Vec2>  polygon_array1, polygon_array2;
	gt::polygon_compute_convex_hull(points1, polygon_array1);
	gt::polygon_compute_convex_hull(points2, polygon_array2);

	gt::polygon_create(polygon1, polygon_array1);
	gt::polygon_create(polygon2, polygon_array2);

	Vec2 a, b;
	bool intersect = gt::polygon_polygon_intersect_test(polygon1, polygon2, a, b);

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
	for (int index_l = 0, last_index = polygon.size() - 1; index_l < polygon.size(); ++index_l)
	{
		draw_node->drawLine(polygon[last_index], polygon[index_l], Color4F::GREEN);
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
	CCLOG("fast->%d,prim->%d", number, number2);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::simplePolygonYDecompose()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 600;
	const int array_size = 17;
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
	//画出离散的边
	for (int index_j = 0; index_j < array_size; ++index_j)
		draw_node->drawLine(points[index_j], points[index_j < array_size - 1 ? index_j + 1 : 0], Color4F::GREEN);
	//计算简单多边形的单调划分
	std::map<int, int> addtional_edge_map;
	std::vector<int>		monotone_polygon_vec;
	std::vector<int>		boundary_index_vec;
	gt::polygon_simple_decompose(points, addtional_edge_map, monotone_polygon_vec, boundary_index_vec);
	//画出来额外的边
	for (auto it = addtional_edge_map.begin(); it != addtional_edge_map.end(); ++it)
		draw_node->drawLine(points[it->first], points[it->second], Color4F::RED);
	//计算离散y单调多边形的集合
	std::vector<int>  points_sequence, points_index;
	gt::polygon_simple_cycle_sequence(array_size, addtional_edge_map, points_sequence, points_index);

	int base_j = 0;
	Vec2 offset(600, 200);
	for (int index_l = 0; index_l < points_index.size(); ++index_l)
	{
		int boundary_l = points_index[index_l];
		const Color4F  color_array(gt::random(), gt::random(), gt::random(), 1.0f);
		//for (int loop_l = base_j; loop_l < boundary_l; ++loop_l)
		//	draw_node->drawLine(points[points_sequence[loop_l]] + offset, points[points_sequence[loop_l < boundary_l-1?loop_l +1: base_j]] + offset, color_array);
		//对新产生的y单调多边形,进行三角剖分
		const int *points_array = points_sequence.data() + base_j;
		std::map<int, int>  local_edge_map;
		std::vector<int>		triangle_sequence_vec;
		gt::polygon_monotone_triangulate(points, points_array, boundary_l - base_j, triangle_sequence_vec, local_edge_map);
		//画出额外的边
		//for (auto it = local_edge_map.begin(); it != local_edge_map.end(); ++it)
		//	draw_node->drawLine(points[it->first], points[it->second],color_array);
		for (int index_j = 0; index_j < triangle_sequence_vec.size(); index_j += 2)
			draw_node->drawLine(points[triangle_sequence_vec[index_j]], points[triangle_sequence_vec[index_j + 1]], color_array);
		base_j = boundary_l;
	}
	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::twoDimensionLinearlyProgram()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 600;
	const int array_size = 17;
	//测试直线相交算法
	gt::Line2D line1, line2;
	std::vector<Vec2>  points(array_size);
	for (int index_j = 0; index_j < array_size; ++index_j)
		points[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

	gt::line_create(line1, points[0], points[1]);
	gt::line_create(line2, points[2], points[3]);

	draw_node->drawLine(points[0], points[1], Color4F::GREEN);
	draw_node->drawLine(points[2], points[3], Color4F::GREEN);

	Vec2 intersect_point;
	gt::line_line_intersect_point(line1, line2, intersect_point);

	Sprite *sprite = Sprite::create("llk_yd.png");
	sprite->setPosition(intersect_point);
	root_node->addChild(sprite);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::halfPlaneIntersectTest()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 700;
	const int array_size = 6;
	////超直线相交测试 6种
	//gt::SuperLine2D	aline, bline;
	//Vec2 intersect_point;
	////直线与直线的相交测试
	//aline.line_type = gt::LineType_Line;
	//aline.start_point = Vec2(555.9709f,-369.5364);// Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());
	////aline.unknown = gt::normalize(aline.start_point,Vec2(length_w * gt::randomf10(), length_l * gt::randomf10()));
	//aline.unknown = gt::normalize(-0.4820f,0.8761f);// Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

	//bline.line_type = gt::LineType_Ray;
	//bline.start_point = Vec2(-504.3671f,119.364f);// Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());
	//bline.unknown = gt::normalize(-0.5754f,-0.8184f);// Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());// gt::normalize(aline.start_point, Vec2(length_w * gt::randomf10(), length_l * gt::randomf10()));
	////bline.unknown = gt::normalize(aline.start_point, Vec2(length_w * gt::randomf10(), length_l * gt::randomf10()));

	//bool b = gt::superline_intersect_test(aline, bline, intersect_point);
	//const Color4F &color = b ? Color4F::RED:Color4F::GREEN;

	////draw_node->drawLine(aline.start_point, aline.start_point + aline.unknown * 800, color);
	////draw_node->drawLine(aline.start_point, aline.unknown, color);
	//draw_node->drawLine(aline.start_point - aline.unknown * 400,aline.start_point + aline.unknown * 400,color);

	////draw_node->drawLine(bline.start_point - bline.unknown * 400, bline.start_point + bline.unknown * 400, color);
	//draw_node->drawLine(bline.start_point, bline.start_point + bline.unknown * 800, color);
	////draw_node->drawLine(bline.start_point, bline.unknown, color);

	//Sprite *sprite = Sprite::create("llk_yd.png");
	//sprite->setPosition(bline.start_point);
	//sprite->setColor(Color3B::RED);
	//root_node->addChild(sprite);

	////sprite = Sprite::create("llk_yd.png");
	////sprite->setPosition(bline.unknown);
	////sprite->setColor(Color3B::RED);
	////root_node->addChild(sprite);

	////sprite = Sprite::create("llk_yd.png");
	////sprite->setPosition(aline.start_point);
	////sprite->setColor(Color3B::RED);
	////root_node->addChild(sprite);

	////sprite = Sprite::create("llk_yd.png");
	////sprite->setPosition(aline.unknown);
	////sprite->setColor(Color3B::RED);
	////root_node->addChild(sprite);

	//if (b)
	//{
	//	Sprite *sprite = Sprite::create("llk_yd.png");
	//	sprite->setPosition(intersect_point);
	//	root_node->addChild(sprite);
	//}
	//
	std::vector<gt::Line2D>  lines(array_size);

	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		gt::Line2D &line = lines[index_j];
		line.start_point = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		const Vec2 final_point(length_w * gt::randomf10(), length_l * gt::randomf10());
		line.direction = gt::normalize(line.start_point, final_point);
	}
	//
	int boundary_array[2];
	std::vector<gt::SuperLine2D> selected_planes;
	std::vector<Vec2>   polygon_points;
	gt::HalfResultType   result_type = gt::half_planes_intersect(lines, selected_planes);
	const Color4F &color = result_type == gt::ResultType_Polygon ? Color4F::RED : Color4F::GREEN;

	//先画出直线,然后画出离散点集
	for (int index_l = 0; index_l < array_size; ++index_l)
	{
		auto &line = lines[index_l];
		const Vec2 start_point = line.start_point - line.direction * 1200;
		const Vec2 final_point = line.start_point + line.direction * 1200;
		draw_node->drawLine(start_point, final_point, color);
		//画出法线
		const Vec2 normal(-line.direction.y, line.direction.x);
		draw_node->drawLine(line.start_point, line.start_point + normal * 100, Color4F::BLUE);
	}
	//画出离散点集
	for (int index_l = 0; result_type == gt::ResultType_Polygon && index_l < selected_planes.size(); ++index_l)
	{
		auto &super_line = selected_planes[index_l];
		if (super_line.line_type == gt::LineType_Ray)
		{
			draw_node->drawLine(super_line.start_point, super_line.start_point + super_line.unknown * 800.0f, Color4F::WHITE);
			Sprite *sprite = Sprite::create("llk_yd.png");
			sprite->setPosition(super_line.start_point);
			root_node->addChild(sprite);
		}
		else if (super_line.line_type == gt::LineType_Segment)
		{
			draw_node->drawLine(super_line.start_point, super_line.unknown, Color4F::WHITE);
			Sprite *sprite = Sprite::create("llk_yd.png");
			sprite->setPosition(super_line.start_point);
			root_node->addChild(sprite);

			sprite = Sprite::create("llk_yd.png");
			sprite->setPosition(super_line.unknown);
			root_node->addChild(sprite);
		}
		else
			assert(false);
	}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::rotateHullMaxDistanceTest()
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

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_j]);
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

	int start_index = 0, final_index = 0;
	float distance = gt::rotate_hull_max_distance(polygon, start_index, final_index);
	draw_node->drawLine(polygon[start_index], polygon[final_index], Color4F::RED);
	//手工验证一下
	float f = 0.0f;

	int la, lb;
	for (int index_l = 0; index_l < polygon.size() - 1; ++index_l)
	{
		for (int secondary_l = index_l + 1; secondary_l < polygon.size(); ++secondary_l)
		{
			float l = gt::length(polygon[index_l], polygon[secondary_l]);
			if (l > f)
			{
				f = l;
				la = index_l;
				lb = secondary_l;
			}
		}
	}
	assert(fabsf(f - distance) <= 0.01f);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::rotateHullWidthTest()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 700;
	const int array_size = 33;
	//三角形平面
	std::vector<Vec2>  points(array_size);
	std::vector<Sprite*>  sprite_array[array_size];
	//生成随机离散点,并计算boundingbox
	Vec2 origin(FLT_MAX, FLT_MAX), bottom(-FLT_MAX, -FLT_MAX);
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_j]);
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

	Vec2 start_point, final_point;
	float distance = gt::rotate_hull_width(polygon, start_point, final_point);
	draw_node->drawLine(start_point, final_point, Color4F::RED);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::rotateHullTwoMaxDistanceTest()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 700;
	const int array_size = 33;
	//三角形平面
	std::vector<Vec2>  points(array_size);
	//生成随机离散点,并计算boundingbox
	Vec2 origin(FLT_MAX, FLT_MAX), bottom(-FLT_MAX, -FLT_MAX);
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		origin.x = fminf(origin.x, points[index_j].x);
		origin.y = fminf(origin.y, points[index_j].y);

		bottom.x = fmaxf(bottom.x, points[index_j].x);
		bottom.y = fmaxf(bottom.y, points[index_j].y);
	}

	std::vector<Vec2>  points2(array_size);
	Vec2 origin2(FLT_MAX, FLT_MAX), bottom2(-FLT_MAX, -FLT_MAX);
	//生成随机离散点,并计算boundingbox
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points2[index_j] = Vec2(length_w * gt::randomf10() + 200.0f, length_l * gt::randomf10());

		origin2.x = fminf(origin2.x, points2[index_j].x);
		origin2.y = fminf(origin2.y, points2[index_j].y);

		bottom2.x = fmaxf(bottom2.x, points2[index_j].x);
		bottom2.y = fmaxf(bottom2.y, points2[index_j].y);
	}
	Vec2 offset1 = (origin2 - bottom) * 0.5f, offset2 = (bottom - origin2) * 0.5f;
	offset1.x -= 140.0f;
	offset1.y = 0.0f;

	offset2.x += 40.0f;
	offset2.y = 0.0f;
	for (int index_l = 0; index_l < array_size; ++index_l)
	{
		points2[index_l] -= offset1;
		points[index_l] -= offset2;

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points2[index_l]);
		root_node->addChild(sprite);

		sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_l]);
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

	//求离散点集的凸包
	std::vector<Vec2> polygon2;
	gt::polygon_compute_convex_hull(points2, polygon2);
	//画出多边形的边界
	for (int index_l = 0; index_l < polygon2.size(); ++index_l)
	{
		int secondary_l = (index_l + 1) % polygon2.size();
		draw_node->drawLine(polygon2[index_l], polygon2[secondary_l], Color4F::WHITE);
	}

	int ahull_index1 = 0, bhull_index2 = 0;
	gt::rotate_hull_max_between(polygon, polygon2, ahull_index1, bhull_index2);
	draw_node->drawLine(polygon[ahull_index1], polygon2[bhull_index2], Color4F::RED);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::rotateHullTwoMinDistanceTest()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 700;
	const int array_size = 33;
	//三角形平面
	std::vector<Vec2>  points(array_size);
	//生成随机离散点,并计算boundingbox
	Vec2 origin(FLT_MAX, FLT_MAX), bottom(-FLT_MAX, -FLT_MAX);
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		origin.x = fminf(origin.x, points[index_j].x);
		origin.y = fminf(origin.y, points[index_j].y);

		bottom.x = fmaxf(bottom.x, points[index_j].x);
		bottom.y = fmaxf(bottom.y, points[index_j].y);
	}

	std::vector<Vec2>  points2(array_size);
	Vec2 origin2(FLT_MAX, FLT_MAX), bottom2(-FLT_MAX, -FLT_MAX);
	//生成随机离散点,并计算boundingbox
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points2[index_j] = Vec2(length_w * gt::randomf10() + 200.0f, length_l * gt::randomf10());

		origin2.x = fminf(origin2.x, points2[index_j].x);
		origin2.y = fminf(origin2.y, points2[index_j].y);

		bottom2.x = fmaxf(bottom2.x, points2[index_j].x);
		bottom2.y = fmaxf(bottom2.y, points2[index_j].y);
	}
	Vec2 offset1 = (origin2 - bottom) * 0.5f, offset2 = (bottom - origin2) * 0.5f;
	offset1.x -= 140.0f;
	offset1.y = 0.0f;

	offset2.x += 40.0f;
	offset2.y = 0.0f;
	for (int index_l = 0; index_l < array_size; ++index_l)
	{
		points2[index_l] -= offset1;
		points[index_l] -= offset2;

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points2[index_l]);
		root_node->addChild(sprite);

		sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_l]);
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

	//求离散点集的凸包
	std::vector<Vec2> polygon2;
	gt::polygon_compute_convex_hull(points2, polygon2);
	//画出多边形的边界
	for (int index_l = 0; index_l < polygon2.size(); ++index_l)
	{
		int secondary_l = (index_l + 1) % polygon2.size();
		draw_node->drawLine(polygon2[index_l], polygon2[secondary_l], Color4F::WHITE);
	}

	Vec2 ahull_point, bhull_point;
	gt::rotate_hull_min_between(polygon, polygon2, ahull_point, bhull_point);
	draw_node->drawLine(ahull_point, bhull_point, Color4F::RED);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::rotateHullMinAreaTest()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 700;
	const int array_size = 33;
	//三角形平面
	std::vector<Vec2>  points(array_size);
	std::vector<Sprite*>  sprite_array[array_size];
	//生成随机离散点,并计算boundingbox
	Vec2 origin(FLT_MAX, FLT_MAX), bottom(-FLT_MAX, -FLT_MAX);
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_j]);
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

	Vec2 rect_points[4];
	float distance = gt::rotate_hull_min_area(polygon, rect_points);

	draw_node->drawLine(rect_points[0], rect_points[1], Color4F::RED);
	draw_node->drawLine(rect_points[1], rect_points[2], Color4F::RED);
	draw_node->drawLine(rect_points[2], rect_points[3], Color4F::RED);
	draw_node->drawLine(rect_points[3], rect_points[0], Color4F::RED);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::rotateHullMinPerimeterTest()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 700;
	const int array_size = 33;
	//三角形平面
	std::vector<Vec2>  points(array_size);
	std::vector<Sprite*>  sprite_array[array_size];
	//生成随机离散点,并计算boundingbox
	Vec2 origin(FLT_MAX, FLT_MAX), bottom(-FLT_MAX, -FLT_MAX);
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_j]);
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

	Vec2 rect_points[4];
	float distance = gt::rotate_hull_min_perimeter(polygon, rect_points);

	draw_node->drawLine(rect_points[0], rect_points[1], Color4F::RED);
	draw_node->drawLine(rect_points[1], rect_points[2], Color4F::RED);
	draw_node->drawLine(rect_points[2], rect_points[3], Color4F::RED);
	draw_node->drawLine(rect_points[3], rect_points[0], Color4F::RED);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::rotateHullOnionDecomposition()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 500;
	const int array_size = 17;
	//三角形平面
	std::vector<Vec2>  points(array_size);
	std::vector<Sprite*>  sprite_array[array_size];
	//生成随机离散点,并计算boundingbox
	Vec2 origin(FLT_MAX, FLT_MAX), bottom(-FLT_MAX, -FLT_MAX);
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_j]);
		root_node->addChild(sprite);
	}

	std::vector<const Vec2 *> triangle_edges;
	gt::rotate_hull_onion_decomposite(points, triangle_edges);
	//画出三角形边
	for (int index_l = 0; index_l < triangle_edges.size(); index_l += 2)
		draw_node->drawLine(*triangle_edges[index_l], *triangle_edges[index_l + 1], Color4F::GREEN);

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::rotateHullSpiralDecomposition()
{
	Node *root_node = Node::create();
	this->addChild(root_node);

	DrawNode  *draw_node = DrawNode::create();
	root_node->addChild(draw_node);

	float  length_l = 600.0f;
	float length_w = 500;
	const int array_size = 33;
	//三角形平面
	std::vector<Vec2>  points(array_size);
	std::vector<Sprite*>  sprite_array[array_size];
	//生成随机离散点,并计算boundingbox
	Vec2 origin(FLT_MAX, FLT_MAX), bottom(-FLT_MAX, -FLT_MAX);
	for (int index_j = 0; index_j < array_size; ++index_j)
	{
		points[index_j] = Vec2(length_w * gt::randomf10(), length_l * gt::randomf10());

		Sprite *sprite = Sprite::create("llk_yd.png");
		sprite->setPosition(points[index_j]);
		root_node->addChild(sprite);
	}

	std::vector<const Vec2 *> triangle_edges;
	//gt::rotate_hull_spiral_line(points, triangle_edges);
	gt::rotate_hull_spiral_decomposite(points, triangle_edges);
	//画出三角形边
	for (int index_l = 0; index_l < triangle_edges.size() - 1; index_l += 2)// 2)
	{
		//if (triangle_edges[index_l] && triangle_edges[index_l + 1])
		draw_node->drawLine(*triangle_edges[index_l], *triangle_edges[index_l + 1], Color4F::GREEN);
	}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::rotateHullPolygonUnion()
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

	std::vector<Vec2> polygon_union;
	gt::rotate_hull_polygon_union(polygon, polygon2, polygon_union);
	int polygon_size = polygon_union.size();
	for (int index_l = 0; index_l < polygon_union.size(); ++index_l)
	{
		draw_node->drawLine(polygon_union[index_l], polygon_union[index_l + 1 >= polygon_size ? 0 : index_l + 1], Color4F::RED);
	}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::rotateHullPolygonIntersect()
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

	std::vector<Vec2> polygon_intersect;
	bool b = gt::rotate_hull_polygon_intersect(polygon, polygon2, polygon_intersect);
	if (b)
	{
		int polygon_size = polygon_intersect.size();
		for (int index_l = 0; index_l < polygon_intersect.size(); ++index_l)
		{
			draw_node->drawLine(polygon_intersect[index_l], polygon_intersect[index_l + 1 >= polygon_size ? 0 : index_l + 1], Color4F::RED);
		}
	}

	root_node->setCameraMask(s_CameraMask);
}

void HelloWorld::rotateHullPolygonInnerTangent()
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
		points2[index_j] = Vec2(800.0f + length_w * gt::randomf10(), length_l * gt::randomf10());

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

	Vec2 tangent[4];
	bool b = gt::rotate_hull_inner_tangent(polygon, polygon2, tangent);
	if (b)
	{
		draw_node->drawLine(tangent[0], tangent[1], Color4F::RED);
		draw_node->drawLine(tangent[2], tangent[3], Color4F::RED);
	}

	root_node->setCameraMask(s_CameraMask);
}