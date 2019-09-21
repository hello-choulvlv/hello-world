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
	bool check_polygon = gt::polygon_compute_minimum(points, polygon_vector);
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
	gt::polygon_compute_minimum(points, points_vec);
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
	gt::polygon_compute_minimum(points, points_vec);
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
	gt::polygon_compute_minimum(points, points_vec);
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