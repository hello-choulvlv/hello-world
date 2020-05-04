/*
  *点定位完整算法实现
  *2020年4月13日
  *@author:xiaohuaxiong
 */
#include "point_location.h"
#include "matrix/matrix.h"
#include "data_struct/link_list.h"
#include <assert.h>

using namespace cocos2d;
NS_GT_BEGIN

LocationLexer::~LocationLexer() {

}

void local_point_visit(LocationLexer &lexer, std::vector<NodeLocal *> &node_array,short ref_result) {
	link_list<NodeLocal*>  node_queue;
	NodeLocal *root_node = lexer.root;

	node_queue.push_back(root_node);
	root_node->ref = ref_result;
	while (node_queue.size() != 0) {
		root_node = node_queue.head()->tv_value;
		vector_fast_push_back(node_array, root_node);
		node_queue.pop_front();

		if (root_node->child_l != nullptr && root_node->child_l->ref != ref_result) {
			node_queue.push_back(root_node->child_l);
			root_node->child_l->ref = ref_result;
		}

		if (root_node->child_r != nullptr && root_node->child_r->ref != ref_result) {
			node_queue.push_back(root_node->child_r);
			root_node->child_r->ref = ref_result;
		}
	}
}
/*
  *初始化梯形图
 */
void local_point_init_trapzoid(LocationLexer &lexer, std::vector<Segment2D> &segments) {
	int array_size = segments.size();
	Segment2D   &low_segment = segments[array_size - 2];
	Segment2D   &up_segment = segments[array_size - 1];
	NodeLocal  *node_local = new NodeLocal(LocalType::LocalType_Trapzoid);
	lexer.root = node_local;

	Trapzoid *trap_ptr = new Trapzoid(low_segment.start_point,low_segment.final_point);
	trap_ptr->up_seg_ptr = &up_segment;
	trap_ptr->low_seg_ptr = &low_segment;
	node_local->trap_ptr = trap_ptr;
	//其它的指针皆为nullptr
	lexer.size +=1;
}
/*
*点查找算法实现
*所有的顶点位置具有一般性假设
*/
NodeLocal*   local_point_find_location(LocationLexer &lexer, const cocos2d::Vec2 &point) {
	NodeLocal  *node_local = lexer.root;
	//查找过程中需要依据节点的类型而采用不同的策略
	while (node_local != nullptr) {
		if (node_local->node_type == LocalType_Endpoint) {//端点,此时需要检索需要向左或者向右走
			if (point.x < node_local->endpoint_ptr->x)
				node_local = node_local->child_l;
			else
				node_local = node_local->child_r;
		}
		else if (node_local->node_type == LocalType_Segment) {//线段,此时需要判断,点在线段之上,还是在线段之下
			float f = cross(node_local->segment_ptr->start_point,node_local->segment_ptr->final_point,point);
			if (f > 0.0f)
				node_local = node_local->child_l;
			else
				node_local = node_local->child_r;
		}
		else {
			//如果具体到梯形,此时一定走到了叶子节点,于是要么属于包含关系,要么发生了错误,这里为了调试起见,
			//再一次的计算了包含关系,实际上在有限范围限定下,必有包含关系,否则一定是程序错误
			Trapzoid  *trap = node_local->trap_ptr;
			bool b1 = point.x > trap->left_point.x && point.x < trap->right_point.x;
			bool b2 = cross(trap->low_seg_ptr->start_point,trap->low_seg_ptr->final_point,point) > 0.0f;
			bool b3 = cross(trap->up_seg_ptr->start_point,trap->up_seg_ptr->final_point,point) < 0.0f;
			assert(b1 & b2 & b3);
			break;
		}
	}
	return node_local;
}
/*
  *查找与给定的目标线段相交的梯形集合
 */
static void local_point_follow_segment(LocationLexer &lexer,const Segment2D  &seg,link_list<NodeLocal *> &follow_nodes) {
	const Vec2 &left_point = seg.start_point;
	const Vec2 &right_point = seg.final_point;

	NodeLocal  *node_local = local_point_find_location(lexer, left_point);
	assert(node_local && node_local->node_type == LocalType_Trapzoid);
	follow_nodes.push_back(node_local);

	//循环遍历,如果线段的右端点跨越了前面的梯形
	while (right_point.x > node_local->trap_ptr->right_point.x) {
		//检测哪一个邻接梯形满足位置关系
		Trapzoid  *trap = node_local->trap_ptr;
		//float f = cross(trap->left_point,trap->right_point,right_point);
		float f = cross(left_point,right_point,trap->right_point);
		//此时右端点在线段之上,因此需要取右下侧的梯形
		if (f > 0.0f)
			node_local = trap->right_low;
		else
			node_local = trap->right_up;
		assert(node_local && node_local->node_type == LocalType_Trapzoid);
		follow_nodes.push_back(node_local);
	}
}
/*
  *一个完全落在梯形内部的线段分裂梯形
 */
static void local_point_split_trapzoid(NodeLocal  *node_local,Segment2D &seg) {
	assert(node_local && node_local->node_type == LocalType_Trapzoid);

	//需要同时生成四个梯形,其中最后一个可以复用第一个
	NodeLocal *a_node_ptr = new NodeLocal(LocalType_Trapzoid);
	Trapzoid  *a_trap = new Trapzoid(node_local->trap_ptr->left_point, seg.start_point);
	a_trap->low_seg_ptr = node_local->trap_ptr->low_seg_ptr;
	a_trap->up_seg_ptr = node_local->trap_ptr->up_seg_ptr;
	a_node_ptr->trap_ptr = a_trap;

	NodeLocal *b_node_ptr = new NodeLocal(LocalType::LocalType_Trapzoid);//最右侧的梯形
	Trapzoid  *b_trap = new Trapzoid(seg.final_point,node_local->trap_ptr->right_point);
	b_trap->low_seg_ptr = node_local->trap_ptr->low_seg_ptr; 
	b_trap->up_seg_ptr = node_local->trap_ptr->up_seg_ptr;
	b_node_ptr->trap_ptr = b_trap;
	//C
	NodeLocal *c_node_ptr = new NodeLocal(LocalType::LocalType_Trapzoid);
	Trapzoid    *c_trap = new Trapzoid(seg.start_point,seg.final_point);
	c_trap->low_seg_ptr = &seg;
	c_trap->up_seg_ptr = node_local->trap_ptr->up_seg_ptr;
	c_node_ptr->trap_ptr = c_trap;

	NodeLocal *d_node_ptr = new NodeLocal(LocalType_Trapzoid);
	Trapzoid   *d_trap = new Trapzoid(seg.start_point,seg.final_point);
	d_trap->low_seg_ptr = node_local->trap_ptr->low_seg_ptr;
	d_trap->up_seg_ptr = &seg;
	d_node_ptr->trap_ptr = d_trap;
	//新生成的梯形与原邻接梯形之间的关系重构
	Trapzoid  *origin_trap = node_local->trap_ptr;
	a_node_ptr->trap_ptr->left_low = origin_trap->left_low;
	a_node_ptr->trap_ptr->left_up = origin_trap->left_up;

	b_node_ptr->trap_ptr->right_low = origin_trap->right_low;
	b_node_ptr->trap_ptr->right_up = origin_trap->right_up;

	if (origin_trap->left_low) {
		Trapzoid *left_trap = origin_trap->left_low->trap_ptr;
		if (left_trap->right_low == node_local)
			left_trap->right_low = a_node_ptr;
		if (left_trap->right_up == node_local)
			left_trap->right_up = a_node_ptr;
	}

	if (origin_trap->left_up) {
		Trapzoid *left_trap = origin_trap->left_up->trap_ptr;
		if (left_trap->right_low == node_local)
			left_trap->right_low = a_node_ptr;
		if (left_trap->right_up == node_local)
			left_trap->right_up = a_node_ptr;
	}

	if (origin_trap->right_low) {
		Trapzoid *right_trap = origin_trap->right_low->trap_ptr;
		if (right_trap->left_low == node_local)
			right_trap->left_low = b_node_ptr;
		if (right_trap->left_up == node_local)
			right_trap->left_up = b_node_ptr;
	}

	if (origin_trap->right_up) {
		Trapzoid *right_trap = origin_trap->right_up->trap_ptr;
		if (right_trap->left_low == node_local)
			right_trap->left_low = b_node_ptr;
		if (right_trap->left_up == node_local)
			right_trap->left_up = b_node_ptr;
	}

	NodeLocal *p_ptr = node_local;
	p_ptr->node_type = LocalType_Endpoint;
	p_ptr->endpoint_ptr = &seg.start_point;
	delete p_ptr->trap_ptr;
	p_ptr->trap_ptr = nullptr;

	NodeLocal *q_ptr = new NodeLocal(LocalType::LocalType_Endpoint);
	q_ptr->endpoint_ptr = &seg.final_point;

	NodeLocal *si_ptr = new NodeLocal(LocalType::LocalType_Segment);
	si_ptr->segment_ptr = &seg;
	//重建以上数据结构的关系
	p_ptr->child_r = q_ptr;
	p_ptr->child_l = a_node_ptr;

	q_ptr->child_l = si_ptr;
	q_ptr->child_r = b_node_ptr;

	si_ptr->child_l = c_node_ptr;
	si_ptr->child_r = d_node_ptr;

	//新形成的梯形之间的关系构建
	a_trap->right_low = d_node_ptr;
	a_trap->right_up = c_node_ptr;

	b_trap->left_low = d_node_ptr;
	b_trap->left_up = c_node_ptr;

	c_trap->left_low = a_node_ptr;
	c_trap->left_up = a_node_ptr;

	c_trap->right_low = b_node_ptr;
	c_trap->right_up = b_node_ptr;

	d_trap->left_low = a_node_ptr;
	d_trap->left_up = a_node_ptr;

	d_trap->right_low = b_node_ptr;
	d_trap->right_up = b_node_ptr;
}
//公共函数,梯形的邻接关系重构
void static_local_point_adj_rearrange(NodeLocal *node_local, NodeLocal *origin_node, NodeLocal *one_node_ptr, NodeLocal *next_node_ptr) {
	Trapzoid *trap_ptr = node_local->trap_ptr;
	if (trap_ptr->left_low == origin_node) {
		if (origin_node->trap_ptr->right_low == node_local)
			one_node_ptr->trap_ptr->right_low = next_node_ptr;
		if (origin_node->trap_ptr->right_up == node_local)
			one_node_ptr->trap_ptr->right_up = next_node_ptr;
		next_node_ptr->trap_ptr->left_low = one_node_ptr;
	}
	else {
		NodeLocal  *quad_node_ptr = trap_ptr->left_low;
		if (quad_node_ptr) {
			if (quad_node_ptr->trap_ptr->right_low == node_local)
				quad_node_ptr->trap_ptr->right_low = next_node_ptr;
			if (quad_node_ptr->trap_ptr->right_up == node_local)
				quad_node_ptr->trap_ptr->right_up = next_node_ptr;
		}
		next_node_ptr->trap_ptr->left_low = quad_node_ptr;
	}

	if (trap_ptr->left_up == origin_node) {
		if (origin_node->trap_ptr->right_up == node_local)
			one_node_ptr->trap_ptr->right_up = next_node_ptr;
		if (origin_node->trap_ptr->right_low == node_local)
			one_node_ptr->trap_ptr->right_low = next_node_ptr;
		next_node_ptr->trap_ptr->left_up = one_node_ptr;
	}
	else {
		NodeLocal  *quad_node_ptr = trap_ptr->left_up;
		if (quad_node_ptr) {
			if (quad_node_ptr->trap_ptr->right_low == node_local)
				quad_node_ptr->trap_ptr->right_low = next_node_ptr;
			if (quad_node_ptr->trap_ptr->right_up == node_local)
				quad_node_ptr->trap_ptr->right_up = next_node_ptr;
		}
		next_node_ptr->trap_ptr->left_up = quad_node_ptr;
	}
}
/*
  *分裂梯形序列
  *注意:第一个跟最后一个梯形比较特殊,需要单独的处理
 */
void local_point_split_trapzoid_sequence(LocationLexer &lexer,link_list<NodeLocal*> &follow_nodes,Segment2D &seg) {
	//开头的梯形
	NodeLocal *secondary_local = follow_nodes.head()->tv_value;
	//最先生成的梯形
	NodeLocal	 *a_node_ptr = new NodeLocal(LocalType_Trapzoid);
	Trapzoid     *a_trap_ptr = new Trapzoid(secondary_local->trap_ptr->left_point,seg.start_point);
	a_node_ptr->trap_ptr = a_trap_ptr;
	//重构原来的对应关系
	Trapzoid  *origin_trap = secondary_local->trap_ptr;
	a_trap_ptr->low_seg_ptr = origin_trap->low_seg_ptr;
	a_trap_ptr->up_seg_ptr = origin_trap->up_seg_ptr;

	a_trap_ptr->left_low = origin_trap->left_low;
	a_trap_ptr->left_up = origin_trap->left_up;

	if (origin_trap->left_low) {
		Trapzoid *left_trap = origin_trap->left_low->trap_ptr;
		if (left_trap->right_low == secondary_local)
			left_trap->right_low = a_node_ptr;
		if (left_trap->right_up == secondary_local)
			left_trap->right_up = a_node_ptr;
	}
	if (origin_trap->left_up) {
		Trapzoid *left_trap = origin_trap->left_up->trap_ptr;
		if (left_trap->right_low == secondary_local)
			left_trap->right_low = a_node_ptr;
		if (left_trap->right_up == secondary_local)
			left_trap->right_up = a_node_ptr;
	}
	//记录下原梯形的右顶点,原来的梯形数据结构暂时不删除,保留用在循环中
	secondary_local->node_type = LocalType_Endpoint;
	secondary_local->endpoint_ptr = &seg.start_point;

	NodeLocal	 *si_ptr = new NodeLocal(LocalType_Segment);
	si_ptr->segment_ptr = &seg;
	secondary_local->child_l = a_node_ptr;
	secondary_local->child_r = si_ptr;
	//剩下的信息可以构建两个梯形,但是其右侧的边界目前尚不能判定,至于该梯形的邻接关系,将由线段seg决定
	NodeLocal *one_node_ptr = new NodeLocal(LocalType_Trapzoid);
	Trapzoid  *one_trap_ptr = new Trapzoid(seg.start_point, Vec2::ZERO);//右侧暂时写上一个临时数据ZERO
	one_trap_ptr->low_seg_ptr = &seg;
	one_trap_ptr->up_seg_ptr = origin_trap->up_seg_ptr;
	one_node_ptr->trap_ptr = one_trap_ptr;
	//以目前已知的数据,并不能得出当前梯形的右边界,该数据的获取要延后,现在填充一个临时数据
	NodeLocal *other_node_ptr = new NodeLocal(LocalType::LocalType_Trapzoid);
	Trapzoid *other_trap_ptr = new Trapzoid(seg.start_point, Vec2::ZERO);//seg.final_point.x < next_trap_ptr->right_point.x?seg.final_point: next_trap_ptr->right_point);
	other_trap_ptr->low_seg_ptr = origin_trap->low_seg_ptr;
	other_trap_ptr->up_seg_ptr = &seg;
	other_node_ptr->trap_ptr = other_trap_ptr;
	//线段,两梯形关系构建
	si_ptr->child_l = one_node_ptr;
	si_ptr->child_r = other_node_ptr;
	a_node_ptr->trap_ptr->right_up = one_node_ptr;
	a_node_ptr->trap_ptr->right_low = other_node_ptr;
	//在遍历的过程中,将伴随着新的邻接关系的生成
	//最后的梯形
	NodeLocal *tripple_local = follow_nodes.back()->tv_value;
	follow_nodes.pop_front();
	follow_nodes.pop_back();
	//经过相关运算之后,one_node_ptr正在线段seg之上,other_node_ptr在线段seg之下
	NodeLocal *origin_node = secondary_local;
	for (auto *it_ptr = follow_nodes.head(); it_ptr != nullptr; it_ptr = follow_nodes.next(it_ptr)) {
		NodeLocal	 *node_local = it_ptr->tv_value;
		NodeLocal  *top_node_ptr = nullptr, *low_node_ptr = nullptr;
		//判断当前边界顶点的相对位置,是否居于目标线段之上/之下?
		const Vec2 &left_point = node_local->trap_ptr->left_point;
		float f = cross(seg.start_point,seg.final_point, left_point);
		if (f > 0.0f) {
			one_node_ptr->trap_ptr->right_point = node_local->trap_ptr->left_point;
			top_node_ptr = new NodeLocal(LocalType_Trapzoid);//应该说,此时该梯形的右边界依然是未知的,其原理跟上面的if语句相同
			top_node_ptr->trap_ptr = new Trapzoid(left_point,Vec2::ZERO);
			top_node_ptr->trap_ptr->low_seg_ptr = &seg;
			top_node_ptr->trap_ptr->up_seg_ptr = node_local->trap_ptr->up_seg_ptr;
		}
		else {
			other_node_ptr->trap_ptr->right_point = left_point;
			low_node_ptr = new NodeLocal(LocalType_Trapzoid);
			low_node_ptr->trap_ptr = new Trapzoid(left_point, Vec2::ZERO);
			low_node_ptr->trap_ptr->up_seg_ptr = &seg;
			low_node_ptr->trap_ptr->low_seg_ptr = node_local->trap_ptr->low_seg_ptr;
		}
		//变更当前梯形顶点的结构,以及其邻接梯形之间的拓扑关系
		Trapzoid *trap_ptr = node_local->trap_ptr;
		node_local->node_type = LocalType_Segment;
		node_local->segment_ptr = &seg;

		if (f > 0.0f) {
			node_local->child_l = top_node_ptr;
			node_local->child_r = other_node_ptr;
			//左/右侧指针重定位
			static_local_point_adj_rearrange(node_local, origin_node, one_node_ptr, top_node_ptr);
			
			one_node_ptr = top_node_ptr;
		}
		else {
			node_local->child_l = one_node_ptr;
			node_local->child_r = low_node_ptr;
			//同上
			static_local_point_adj_rearrange(node_local, origin_node,other_node_ptr, low_node_ptr);

			other_node_ptr = low_node_ptr;
		}

		origin_node = node_local;
		origin_trap = node_local->trap_ptr;
	}
	//剩下的代码为最后一个梯形的处理,与第一个梯形的处理方式类似,也需要分裂与关系重构
	NodeLocal	 *top_node_ptr = nullptr,*low_node_ptr = nullptr;
	float f = cross(seg.start_point, seg.final_point, tripple_local->trap_ptr->left_point);
	if (f > 0.0f) {
		one_node_ptr->trap_ptr->right_point = tripple_local->trap_ptr->left_point;
		top_node_ptr = new NodeLocal(LocalType_Trapzoid);
		top_node_ptr->trap_ptr = new Trapzoid(tripple_local->trap_ptr->left_point, seg.final_point);
		top_node_ptr->trap_ptr->low_seg_ptr = &seg;
		top_node_ptr->trap_ptr->up_seg_ptr = tripple_local->trap_ptr->up_seg_ptr;
		//此时不必检测空值
		static_local_point_adj_rearrange(tripple_local, origin_node, one_node_ptr, top_node_ptr);
		one_node_ptr = top_node_ptr;

		other_node_ptr->trap_ptr->right_point = seg.final_point;
	}
	else {
		other_node_ptr->trap_ptr->right_point = tripple_local->trap_ptr->left_point;
		low_node_ptr = new NodeLocal(LocalType_Trapzoid);
		low_node_ptr->trap_ptr = new Trapzoid(tripple_local->trap_ptr->left_point, seg.final_point);
		low_node_ptr->trap_ptr->low_seg_ptr = tripple_local->trap_ptr->low_seg_ptr;
		low_node_ptr->trap_ptr->up_seg_ptr = &seg;

		static_local_point_adj_rearrange(tripple_local, origin_node, other_node_ptr, low_node_ptr);
		other_node_ptr = low_node_ptr;

		one_node_ptr->trap_ptr->right_point = seg.final_point;
	}
	//需要新建立一个梯形,最右侧的梯形
	NodeLocal *right_node = new NodeLocal(LocalType_Trapzoid);
	right_node->trap_ptr = new Trapzoid(seg.final_point, tripple_local->trap_ptr->right_point);
	right_node->trap_ptr->up_seg_ptr = tripple_local->trap_ptr->up_seg_ptr;
	right_node->trap_ptr->low_seg_ptr = tripple_local->trap_ptr->low_seg_ptr;
	//新的线段节点
	NodeLocal	 *right_seg_node = new NodeLocal(LocalType_Segment);
	right_seg_node->segment_ptr = &seg;
	right_seg_node->child_l = one_node_ptr;
	right_seg_node->child_r = other_node_ptr;
	//修改最后一个节点的类型
	tripple_local->node_type = LocalType_Endpoint;
	tripple_local->endpoint_ptr = &seg.final_point;
	tripple_local->child_l = right_seg_node;
	tripple_local->child_r = right_node;
	//邻接关系修正
	one_node_ptr->trap_ptr->right_low = right_node;
	one_node_ptr->trap_ptr->right_up = right_node;

	other_node_ptr->trap_ptr->right_low = right_node;
	other_node_ptr->trap_ptr->right_up = right_node;

	right_node->trap_ptr->left_low = other_node_ptr;
	right_node->trap_ptr->left_up = one_node_ptr;
	//right_node对应着原tripple节点的右侧邻接关系
	origin_trap = tripple_local->trap_ptr;
	right_node->trap_ptr->right_low = tripple_local->trap_ptr->right_low;
	right_node->trap_ptr->right_up = tripple_local->trap_ptr->right_up;

	if (origin_trap->right_low) {
		NodeLocal *st = origin_trap->right_low;
		if (st->trap_ptr->left_low == tripple_local)
			st->trap_ptr->left_low = right_node;
		if (st->trap_ptr->left_up == tripple_local)
			st->trap_ptr->left_up = right_node;
	}

	if (origin_trap->right_up) {
		NodeLocal *st = origin_trap->right_up;
		if (st->trap_ptr->left_low == tripple_local)
			st->trap_ptr->left_low = right_node;
		if (st->trap_ptr->left_up == tripple_local)
			st->trap_ptr->left_up = right_node;
	}
	//销毁所有的已过时的梯形数据结构
	delete secondary_local->trap_ptr;
	secondary_local->trap_ptr = nullptr;

	delete tripple_local->trap_ptr;
	tripple_local->trap_ptr = nullptr;
	for (auto *it_ptr = follow_nodes.head(); it_ptr != nullptr; it_ptr = follow_nodes.next(it_ptr)) {
		delete it_ptr->tv_value->trap_ptr;
		it_ptr->tv_value->trap_ptr = nullptr;
	}
}

void local_point_create_trapzoid(LocationLexer &lexer, std::vector<Segment2D> &segments) {
	//初始化梯形图
	local_point_init_trapzoid(lexer, segments);

	link_list<NodeLocal*> follow_nodes;
	int array_size = segments.size();
	for (int j = 0; j < array_size - 2; ++j) {
		Segment2D &seg = segments[j];
		local_point_follow_segment(lexer, seg, follow_nodes);
		auto *it_ptr = follow_nodes.head();
		//队列中有一个节点/多个节点的处理情况大不相同,需要分开处理
		if (follow_nodes.size() == 1) 
			local_point_split_trapzoid(it_ptr->tv_value, seg);
		else 
			local_point_split_trapzoid_sequence(lexer,follow_nodes,seg);
		follow_nodes.clear();
	}
}

NS_GT_END