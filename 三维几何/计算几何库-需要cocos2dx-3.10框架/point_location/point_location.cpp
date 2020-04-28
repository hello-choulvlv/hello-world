/*
  *�㶨λ�����㷨ʵ��
  *2020��4��13��
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
/*
  *��ʼ������ͼ
 */
void local_point_init_trapzoid(LocationLexer &lexer, std::vector<Segment2D> &segments) {
	int array_size = segments.size();
	Segment2D   &low_segment = segments[array_size - 2];
	Segment2D   &up_segment = segments[array_size - 1];
	NodeLocal  *node_local = new NodeLocal(LocalType::LocalType_Trapzoid);
	lexer.root = node_local;

	Trapzoid *trap = new Trapzoid(low_segment.start_point,low_segment.final_point);
	trap->up_seg_ptr = &up_segment;
	trap->low_seg_ptr = &low_segment;
	//������ָ���Ϊnullptr
	lexer.size +=1;
}
/*
*������㷨ʵ��
*���еĶ���λ�þ���һ���Լ���
*/
NodeLocal*   local_point_find_location(LocationLexer &lexer, const cocos2d::Vec2 &point) {
	NodeLocal  *node_local = lexer.root;
	//���ҹ�������Ҫ���ݽڵ�����Ͷ����ò�ͬ�Ĳ���
	while (node_local != nullptr) {
		if (node_local->node_type == LocalType_Endpoint) {//�˵�,��ʱ��Ҫ������Ҫ�������������
			if (point.x < node_local->endpoint_ptr->x)
				node_local = node_local->child_l;
			else
				node_local = node_local->child_r;
		}
		else if (node_local->node_type == LocalType_Segment) {//�߶�,��ʱ��Ҫ�ж�,�����߶�֮��,�������߶�֮��
			float f = cross(node_local->segment_ptr->start_point,node_local->segment_ptr->final_point,point);
			if (f > 0.0f)
				node_local = node_local->child_l;
			else
				node_local = node_local->child_r;
		}
		else {
			//������嵽����,��ʱһ���ߵ���Ҷ�ӽڵ�,����Ҫô������ϵ,Ҫô�����˴���,����Ϊ�˵������,
			//��һ�εļ����˰�����ϵ,ʵ���������޷�Χ�޶���,���а�����ϵ,����һ���ǳ������
			Trapzoid  *trap = node_local->trap_ptr;
			bool b1 = point.x > trap->left_point.x && point.x < trap->right_point.x;
			bool b2 = cross(trap->low_seg_ptr->start_point,trap->up_seg_ptr->final_point,point) > 0.0f;
			bool b3 = cross(trap->up_seg_ptr->start_point,trap->up_seg_ptr->final_point,point) < 0.0f;
			assert(b1 & b2 & b3);
			break;
		}
	}
	return node_local;
}
/*
  *�����������Ŀ���߶��ཻ�����μ���
 */
static void local_point_follow_segment(LocationLexer &lexer,const Segment2D  &seg,link_list<NodeLocal *> &follow_nodes) {
	const Vec2 &left_point = seg.start_point;
	const Vec2 &right_point = seg.final_point;

	NodeLocal  *node_local = local_point_find_location(lexer, left_point);
	assert(node_local && node_local->node_type == LocalType_Trapzoid);
	follow_nodes.push_back(node_local);

	//ѭ������,����߶ε��Ҷ˵��Խ��ǰ�������
	while (right_point.x > node_local->trap_ptr->right_point.x) {
		//�����һ���ڽ���������λ�ù�ϵ
		Trapzoid  *trap = node_local->trap_ptr;
		float f = cross(trap->left_point,trap->right_point,right_point);
		//��ʱ�Ҷ˵����߶�֮��,�����Ҫȡ���²������
		if (f > 0.0f)
			node_local = trap->right_low;
		else
			node_local = trap->right_up;
		assert(node_local && node_local->node_type == LocalType_Trapzoid);
		follow_nodes.push_back(node_local);
	}
}
/*
  *һ����ȫ���������ڲ����߶η�������
 */
static void local_point_split_trapzoid(NodeLocal  *node_local,Segment2D &seg) {
	assert(node_local && node_local->node_type == LocalType_Trapzoid);

	//��Ҫͬʱ�����ĸ�����,�������һ�����Ը��õ�һ��
	NodeLocal *a_node_ptr = new NodeLocal(LocalType_Trapzoid);
	Trapzoid  *a_trap = new Trapzoid(node_local->trap_ptr->left_point, seg.start_point);
	a_trap->low_seg_ptr = node_local->trap_ptr->low_seg_ptr;
	a_trap->up_seg_ptr = node_local->trap_ptr->up_seg_ptr;
	a_node_ptr->trap_ptr = a_trap;

	NodeLocal *b_node_ptr = new NodeLocal(LocalType::LocalType_Trapzoid);//���Ҳ������
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
	//�����ɵ�������ԭ�ڽ�����֮��Ĺ�ϵ�ع�
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
	//�ؽ��������ݽṹ�Ĺ�ϵ
	p_ptr->child_r = q_ptr;
	p_ptr->child_l = a_node_ptr;

	q_ptr->child_l = si_ptr;
	q_ptr->child_r = b_node_ptr;

	si_ptr->child_l = c_node_ptr;
	si_ptr->child_r = d_node_ptr;

	//���γɵ�����֮��Ĺ�ϵ����
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
/*
  *������������
  *ע��:��һ�������һ�����αȽ�����,��Ҫ�����Ĵ���
 */
void local_point_split_trapzoid_sequence(LocationLexer &lexer,link_list<NodeLocal*> &follow_nodes,Segment2D &seg) {
	//��ͷ������
	NodeLocal *secondary_local = follow_nodes.head()->tv_value;
	//��������
	NodeLocal *tripple_local = follow_nodes.back()->tv_value;
	//�������ɵ�����
	NodeLocal	 *a_node_ptr = new NodeLocal(LocalType_Trapzoid);
	Trapzoid     *a_trap_ptr = new Trapzoid(secondary_local->trap_ptr->left_point,seg.start_point);
	a_node_ptr->trap_ptr = a_trap_ptr;
	//�ع�ԭ���Ķ�Ӧ��ϵ
	Trapzoid  *origin_trap = secondary_local->trap_ptr;
	a_trap_ptr->left_low = origin_trap->left_low;
	a_trap_ptr->left_up = origin_trap->left_up;

	if (origin_trap->left_low) {
		Trapzoid *left_trap = origin_trap->left_low->trap_ptr;
		if (left_trap->right_low == secondary_local)
			left_trap->right_low = a_node_ptr;
		if (left_trap->right_up == secondary_local)
			left_trap->right_up = a_node_ptr;
	}
	//��¼��ԭ���ε��Ҷ���,ԭ�����������ݽṹ��ʱ��ɾ��,��������ѭ����
	Vec2   right_point = origin_trap->right_point;
	secondary_local->node_type = LocalType_Endpoint;
	secondary_local->trap_ptr = nullptr;
	secondary_local->endpoint_ptr = &seg.start_point;

	NodeLocal	 *si_ptr = new NodeLocal(LocalType_Segment);
	si_ptr->segment_ptr = &seg;
	secondary_local->child_l = a_node_ptr;
	secondary_local->child_r = si_ptr;
	//ʡ�µ���Ϣֻ�ܹ�����һ������,���ڸ����ε��ڽӹ�ϵ,�����߶�seg����
	NodeLocal *one_node_ptr = new NodeLocal(LocalType_Trapzoid);
	Trapzoid  *one_trap_ptr = new Trapzoid(seg.start_point, Vec2::ZERO);
	one_trap_ptr->low_seg_ptr = &seg;
	one_trap_ptr->up_seg_ptr = origin_trap->up_seg_ptr;
	one_node_ptr->trap_ptr = one_trap_ptr;
	//��Ŀǰ��֪������,�����ܵó���ǰ���ε��ұ߽�,�����ݵĻ�ȡҪ�Ӻ�,�������һ����ʱ����
	NodeLocal *other_node_ptr = new NodeLocal(LocalType::LocalType_Trapzoid);
	Trapzoid *other_trap_ptr = new Trapzoid(seg.start_point, Vec2::ZERO);//seg.final_point.x < next_trap_ptr->right_point.x?seg.final_point: next_trap_ptr->right_point);
	other_trap_ptr->low_seg_ptr = origin_trap->low_seg_ptr;
	other_trap_ptr->up_seg_ptr = &seg;
	other_node_ptr->trap_ptr = other_trap_ptr;
	//�߶�,�����ι�ϵ����
	si_ptr->child_l = one_node_ptr;
	si_ptr->child_r = other_node_ptr;
	a_node_ptr->trap_ptr->right_up = one_node_ptr;
	a_node_ptr->trap_ptr->right_low = other_node_ptr;
	//�ڱ����Ĺ�����,���������µ��ڽӹ�ϵ������
	follow_nodes.pop_front();
	follow_nodes.pop_back();
	//�����������֮��,one_node_ptr�����߶�seg֮��,other_node_ptr���߶�seg֮��
	NodeLocal *origin_node = secondary_local;
	for (auto *it_ptr = follow_nodes.head(); it_ptr != nullptr; it_ptr = follow_nodes.next(it_ptr)) {
		NodeLocal	 *node_local = it_ptr->tv_value;
		NodeLocal  *top_node_ptr = nullptr, *low_node_ptr = nullptr;
		//�жϵ�ǰ�߽綥������λ��,�Ƿ����Ŀ���߶�֮��/֮��?
		const Vec2 &left_point = node_local->trap_ptr->left_point;
		float f = cross(seg.start_point,seg.final_point, left_point);
		if (f > 0.0f) {
			one_node_ptr->trap_ptr->right_point = node_local->trap_ptr->left_point;
			top_node_ptr = new NodeLocal(LocalType_Trapzoid);//Ӧ��˵,��ʱ�����ε��ұ߽���Ȼ��δ֪��,��ԭ��������if�����ͬ
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
		//�����ǰ���ζ���Ľṹ,�Լ����ڽ�����֮������˹�ϵ
		node_local->node_type = LocalType_Segment;
		Trapzoid *trap_ptr = node_local->trap_ptr;
		node_local->trap_ptr = nullptr;
		node_local->segment_ptr = &seg;

		if (f > 0.0f) {
			node_local->child_l = top_node_ptr;
			node_local->child_r = other_node_ptr;
			one_node_ptr = top_node_ptr;

			if (trap_ptr->left_low == origin_node)
				top_node_ptr->trap_ptr->left_low = origin_node;
			if (trap_ptr->left_up == origin_node)
				top_node_ptr->trap_ptr->left_up = origin_node;
		}
		else {
			node_local->child_l = one_node_ptr;
			node_local->child_r = low_node_ptr;
			other_node_ptr = low_node_ptr;

			if (trap_ptr->left_low == origin_node)
				low_node_ptr->trap_ptr->left_low = origin_node;
			if (trap_ptr->left_up == origin_node)
				low_node_ptr->trap_ptr->left_up = origin_node;
		}

		origin_node = node_local;
	}
	//ʣ�µĴ���Ϊ���һ�����εĴ���,���һ�����εĴ���ʽ����,Ҳ��Ҫ�������ϵ�ع�
}

void local_point_create_trapzoid(LocationLexer &lexer, std::vector<Segment2D> &segments) {
	//��ʼ������ͼ
	local_point_init_trapzoid(lexer, segments);

	link_list<NodeLocal*> follow_nodes;
	int array_size = segments.size();
	for (int j = 0; j < array_size - 2; ++j) {
		Segment2D &seg = segments[j];
		local_point_follow_segment(lexer, seg, follow_nodes);
		auto *it_ptr = follow_nodes.head();
		//��������һ���ڵ�/����ڵ�Ĵ����������ͬ,��Ҫ�ֿ�����
		if (follow_nodes.size() == 1) 
			local_point_split_trapzoid(it_ptr->tv_value, seg);
		else 
			local_point_split_trapzoid_sequence(lexer,follow_nodes,seg);
		follow_nodes.clear();
	}
}

NS_GT_END