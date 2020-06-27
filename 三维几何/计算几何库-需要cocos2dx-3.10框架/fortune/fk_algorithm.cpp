/*
  *海滩线算法实现
  *2020年6月9日
  *@author:xiaohuaxiong
 */
#include "fk_algorithm.h"
#include "matrix/matrix.h"
#include <math.h>

NS_GT_BEGIN

FortuneAlgorithm::FortuneAlgorithm(const std::vector<cocos2d::Vec2> &locations):_diagram(locations), _event_queue(locations.size(),true), _beach_line(0){
	//函数实现
	_compute_func = [this](const FxArc *a_ptr, const FxArc *b_ptr)->float {
		const cocos2d::Vec2 &point1 = a_ptr->site_ptr->location, &point2 = b_ptr->site_ptr->location;
		float x1 = point1.x, y1 = point1.y, x2 = point2.x, y2 = point2.y;
		float d1 = 1.0f / (2.0f * (y1 - _beachLineY));
		float d2 = 1.0f / (2.0f * (y2 - _beachLineY));
		float a = d1 - d2;
		float b = 2.0f * (x2 * d2 - x1 * d1);
		float c = (y1 * y1 + x1 * x1 - _beachLineY * _beachLineY) * d1 - (y2 * y2 + x2 * x2 - _beachLineY * _beachLineY) * d2;
		float delta = b * b - 4.0f * a * c;
		/*
		*抛物线的方程式(x - tx)^2 = 2*p*(y - (ty +l)/2)
		*分析抛物线的性质,最后可以证明得出,所得到的交点必然大于min(point1.x,point2.x)
		*/
		return (-b + sqrtf(delta)) / (2.0f * a);
	};

	_compare_func = [](FkEvent *const &a_ptr, FkEvent *const &b_ptr)->bool {
		return a_ptr->base_y > b_ptr->base_y;
	};

	_modify_func = [](FkEvent *&event_ptr,int queue_idx)->int {
		if (queue_idx != -1)
			event_ptr->queue_idx = queue_idx;
		return event_ptr->queue_idx;
	};
	//将所有的基点/以及相关的弧加入到队列中
	for (int j = 0; j < locations.size(); ++j) {
		FkEvent *event_ptr = new FkEvent(_diagram.getSite(j));
		_event_queue.insert(event_ptr,_compare_func,_modify_func);
	}
}

void FortuneAlgorithm::build() {
	int loop_count = 0;
	while (_event_queue.size() != 0) {
		FkEvent  *event_ptr = _event_queue.head();
		_event_queue.remove_head( _compare_func, _modify_func);

		_beachLineY = event_ptr->base_y;
		if (event_ptr->event_type == FkEventType_Site)
			handleSiteEvent(event_ptr);
		else
			handleCircleEvent(event_ptr);

		++loop_count;
		delete event_ptr;
	}
}

void FortuneAlgorithm::bound() {
	//对于剩下的弧,逐个的遍历,在该过程中将会形成新的边
	FxArc  *arc_ptr = _beach_line.find_mostleft();
	while(arc_ptr != nullptr && arc_ptr->next) {
		FxArc  *next_arc_ptr = arc_ptr->next;
		cocos2d::Vec2   center_point = (arc_ptr->site_ptr->location + next_arc_ptr->site_ptr->location) * 0.5f;
		cocos2d::Vec2   direction = normalize(-cocos2d::Vec2(arc_ptr->site_ptr->location.y - next_arc_ptr->site_ptr->location.y, next_arc_ptr->site_ptr->location.x - arc_ptr->site_ptr->location.x));
		FxEdge  *right_edge = arc_ptr->right_edge;
		FxEdge  *left_edge = next_arc_ptr->left_edge;

		cocos2d::Vec2 *vertex_ptr = _diagram.createVertex(center_point + direction * 400.0f);
		assert(!right_edge->origin_ptr && !left_edge->destination_ptr);
		right_edge->origin_ptr = vertex_ptr;
		left_edge->destination_ptr = vertex_ptr;

		arc_ptr = next_arc_ptr;
	}
}

void FortuneAlgorithm::handleSiteEvent(FkEvent *event_ptr) {
	if (!_beach_line.size()) {
		_beach_line.insert_root(_beach_line.alloc(event_ptr->site_ptr));
		return;
	}
	//查找目标弧
	FxArc  *locate_arc = _beach_line.lookup(event_ptr->site_ptr->location, _compute_func);
	assert(locate_arc != nullptr);

	removeEvent(locate_arc);
	FxArc  *middle_arc = breakArc(locate_arc, event_ptr->site_ptr);
	FxArc  *left_arc = middle_arc->prev, *right_arc = middle_arc->next;

	addEdge(left_arc, middle_arc);
	middle_arc->right_edge = middle_arc->left_edge;
	right_arc->left_edge = left_arc->right_edge;
	//add circle event if necessary
	if (left_arc->prev != nullptr)
		addEvent(left_arc->prev, left_arc, middle_arc);
	if (right_arc->next != nullptr)
		addEvent(middle_arc, right_arc, right_arc->next);
}

void FortuneAlgorithm::handleCircleEvent(FkEvent *event_ptr) {
	FxArc  *arc_ptr = event_ptr->arc_ptr;
	FxArc  *left_arc = arc_ptr->prev, *right_arc = arc_ptr->next;

	removeEvent(left_arc);
	removeEvent(right_arc);

	removeArc(event_ptr);

	if (left_arc->prev != nullptr)
		addEvent(left_arc->prev, left_arc, right_arc);

	if (right_arc->next != nullptr)
		addEvent(left_arc, right_arc, right_arc->next);
}

FxArc  *FortuneAlgorithm::breakArc(FxArc *origin_arc,FkSite  *target_site) {
	FxArc  *middle_arc = _beach_line.alloc(target_site);
	FxArc  *left_arc = _beach_line.alloc(origin_arc->site_ptr);
	FxArc  *right_arc = _beach_line.alloc(origin_arc->site_ptr);

	left_arc->left_edge = origin_arc->left_edge;
	right_arc->right_edge = origin_arc->right_edge;

	_beach_line.replace(origin_arc, middle_arc);
	_beach_line.insert_before(middle_arc, left_arc);
	_beach_line.insert_after(middle_arc, right_arc);
	_beach_line.release(origin_arc);

	return middle_arc;
}

void FortuneAlgorithm::removeArc(FkEvent  *event_ptr) {
	FxArc  *target_arc = event_ptr->arc_ptr;
	assert(target_arc->prev != nullptr && target_arc->next != nullptr);

	cocos2d::Vec2 *vertex_ptr = _diagram.createVertex(event_ptr->location);
	setDestination(target_arc->prev, target_arc, vertex_ptr);
	setDestination(target_arc, target_arc->next, vertex_ptr);

	target_arc->left_edge->next = target_arc->right_edge;
	target_arc->right_edge->prev = target_arc->left_edge;
	//删除相关联的圆事件
	//assert(!target_arc->prev->event_ptr && !target_arc->next->event_ptr);
	//removeEvent(target_arc->prev);
	//removeEvent(target_arc->next);

	FxArc  *prev_arc = target_arc->prev, *next_arc = target_arc->next;
	FxEdge  *prev_edge = prev_arc->right_edge;
	FxEdge  *next_edge = next_arc->left_edge;
	_beach_line.remove(target_arc);

	addEdge(prev_arc, next_arc);
	setOrigin(prev_arc, next_arc,vertex_ptr);
	setPrevEdge(prev_arc->right_edge,prev_edge);
	setPrevEdge(next_edge,next_arc->left_edge);
}

void FortuneAlgorithm::removeEvent(FxArc *target_arc) {
	if (target_arc->event_ptr != nullptr) {
		_event_queue.remove(target_arc->event_ptr, _compare_func, _modify_func);
		target_arc->event_ptr = nullptr;
	}
}

void FortuneAlgorithm::addEdge(FxArc *left_arc, FxArc *right_arc) {
	left_arc->right_edge = _diagram.createHalfEdge(left_arc->site_ptr);
	right_arc->left_edge = _diagram.createHalfEdge(right_arc->site_ptr);

	left_arc->right_edge->twin = right_arc->left_edge;
	right_arc->left_edge->twin = left_arc->right_edge;
}
//添加事件的过程比较复杂
void FortuneAlgorithm::addEvent(FxArc *left_arc, FxArc *middle_arc, FxArc *right_arc) {
	//第一步,求三个基点所对应的点形成的外接圆的圆心
	const cocos2d::Vec2 &point1 = left_arc->site_ptr->location;
	const cocos2d::Vec2 &point2 = middle_arc->site_ptr->location;
	const cocos2d::Vec2 &point3 = right_arc->site_ptr->location;
	cocos2d::Vec2 v1(-point1.y + point2.y,point1.x - point2.x);
	cocos2d::Vec2 v2(-point2.y + point3.y,point2.x - point3.x);
	cocos2d::Vec2 delta((point3.x - point1.x) * 0.5f,(point3.y - point1.y) * 0.5f);
	//这里的行列式也可以理解为两个向量所形成的平行四边形的面积,
	//将向量delta展开成向量r(圆心与点p2所形成的向量),v1,v2表表达形式之后再经过除法运算,可以证明,t确实为v1的模与圆半径的比值
	//因此该算法是正确且严谨的
	float t = cross(delta,v2)/cross(v1,v2);// delta.getDet(v2) / v1.getDet(v2);
	cocos2d::Vec2 center = (point1 + point2) * 0.5f + v1 * t;
	double r = length(center,point1);
	float y = center.y - r;

	float f2 = cross(point1,point2,point3);
	if (y <= _beachLineY && f2 < 0.0f) {
		FkEvent  *event_ptr = new FkEvent(y, center, middle_arc);
		middle_arc->event_ptr = event_ptr;
		_event_queue.insert(event_ptr, _compare_func, _modify_func);
	}
}

void FortuneAlgorithm::setDestination(FxArc *left_arc, FxArc *right_arc, cocos2d::Vec2 *location) {
	left_arc->right_edge->origin_ptr = location;
	right_arc->left_edge->destination_ptr = location;
}

void FortuneAlgorithm::setOrigin(FxArc *left_arc, FxArc *right_arc, cocos2d::Vec2 *location) {
	left_arc->right_edge->destination_ptr = location;
	right_arc->left_edge->origin_ptr = location;
}

void FortuneAlgorithm::setPrevEdge(FxEdge *prev, FxEdge *next) {
	prev->next = next;
	next->prev = prev;
}
NS_GT_END