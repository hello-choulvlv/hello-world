/*
  *海滩数据结构实现
  *2020年6月8日
  *@author:xiaohuaxiong
 */
#include "beach_line.h"

NS_GT_BEGIN
FxBeachLine::~FxBeachLine() {
	FxArc *fix_array[512];
	FxArc   **node_array = _node_size <= 512 ? fix_array : new FxArc  *[_node_size];
	FxArc  *child = find_mostleft();

	for (int j = 0; j < _node_size; ++j,child = child->next)
		node_array[j] = child;

	for (int j = 0; j < _node_size; ++j)
		delete node_array[j];

	if (node_array != fix_array)
		delete[] node_array;

	node_array = nullptr;
	_node_size = 0;
	//_cache
	child = _cache_head;
	while (child) {
		FxArc *t = child;
		child = child->r_child;
		delete t;
	}
	_root = nullptr;
	_cache_head = nullptr;
	_cache_size = 0;
	_cache_capacity = 0;
}

void FxBeachLine::clear()
{
	if (_node_size) {
		//平衡树的规模一般较小,此时可以使用固定的数组
		FxArc *fix_array[512];
		FxArc **remind_array = _node_size <= 512 ? fix_array : new FxArc*[_node_size];

		FxArc  *child = find_mostleft();
		for (int j = 0; j < _node_size; ++j,child = child->next)
			remind_array[j++] = child;

		for (int j = 0; j < _node_size; ++j) {
			remind_array[j]->r_child = _cache_head;
			_cache_head = remind_array[j];
		}
		_cache_size += _node_size;

		if (remind_array != fix_array) {
			delete[] remind_array;
			remind_array = nullptr;
		}
		_root = nullptr;
		_node_size = 0;
	}
};

FxArc *FxBeachLine::find_next(FxArc  *node)
{
	FxArc  *child = node->r_child;
	if (child != nullptr) {
		while (child->l_child)
			child = child->l_child;
		return child;
	}

	FxArc *parent = node->parent;
	while (parent && parent->r_child == node) {
		node = parent;
		parent = parent->parent;
	}
	return parent;
};

FxArc *FxBeachLine::find_prev(FxArc  *node)
{
	FxArc  *child = node->l_child;
	if (child != nullptr) {
		while (child->r_child)
			child = child->r_child;
	}
	else {
		for (child = node->parent; child != nullptr && node == child->l_child; child = child->parent)
			node = child;
	}
	return child;
};

FxArc *FxBeachLine::find_mostleft()
{
	FxArc  *child = _root;
	while (child && child->l_child)//first
		child = child->l_child;
	return child;
};

FxArc* FxBeachLine::lookup(const cocos2d::Vec2 &point,std::function<float (const FxArc *, const FxArc *)> &compute_func) {
	FxArc* n = _root;
	while (n != nullptr) {
		float l_boundary = -FLT_MAX, r_boundary = FLT_MAX;
		if (n->prev != nullptr)
			l_boundary = compute_func(n->prev,n);
		if (n->next != nullptr)
			r_boundary = compute_func(n,n->next);

		if (point.x < l_boundary)
			n = n->l_child;
		else if (point.x > r_boundary)
			n = n->r_child;
		else
			break;
	}

	//debug_node(compute_func);
	return n;
};

void FxBeachLine::insert_root(FxArc  *inserted_node) {
	assert(!_root && _node_size == 0);
	_root = inserted_node;
	_root->color_type = FxColorType_Black;
	++_node_size;
}

void FxBeachLine::insert_before(FxArc *target_node, FxArc *inserted_node) {
	if (_node_size == 8) {
		int xx = 0;
		int yy = 0;
	}
	if (!target_node->l_child) {
		target_node->l_child = inserted_node;
		inserted_node->parent = target_node;
	}
	else {
		target_node->prev->r_child = inserted_node;
		inserted_node->parent = target_node->prev;
	}

	inserted_node->prev = target_node->prev;
	if (inserted_node->prev != nullptr)
		inserted_node->prev->next = inserted_node;

	inserted_node->next = target_node;
	target_node->prev = inserted_node;

	insert_fixup(inserted_node);
	++_node_size;
	check_cascade(_root);
	debug_node();
}

void FxBeachLine::insert_after(FxArc *target_node, FxArc *inserted_node) {
	if (!target_node->r_child) {
		target_node->r_child = inserted_node;
		inserted_node->parent = target_node;
	}
	else {
		target_node->next->l_child = inserted_node;
		inserted_node->parent = target_node->next;
	}

	inserted_node->next = target_node->next;
	if (inserted_node->next != nullptr)
		inserted_node->next->prev = inserted_node;

	inserted_node->prev = target_node;
	target_node->next = inserted_node;

	insert_fixup(inserted_node);
	++_node_size;
	check_cascade(_root);
	debug_node();
}
/*
*插入节点之后的修正
*/
void FxBeachLine::insert_fixup(FxArc *insert_node) {
	if (!insert_node->parent) {
		insert_node->color_type = FxColorType_Black;
		return;
	}
	while (fx_node_color(insert_node->parent) == FxColorType_Red) {
		//两大判断分支,已知parent的颜色为红色,则其必然有父节点
		FxArc *parent = insert_node->parent;
		FxArc *grand_parent = parent->parent;
		if (parent == grand_parent->l_child) {
			FxArc *right_uncle = grand_parent->r_child;
			if (fx_node_color(right_uncle) == FxColorType_Red) {//叔叔节点
				parent->color_type = FxColorType_Black;
				if (right_uncle)right_uncle->color_type = FxColorType_Black;
				grand_parent->color_type = FxColorType_Red;
				insert_node = grand_parent;
			}
			else if (insert_node == parent->r_child) {//此时已知叔叔节点的颜色为黑色,因此需要做出复杂的变换
				insert_node = parent;
				rotate_left(insert_node);

				parent = insert_node->parent;
				parent->color_type = FxColorType_Black;
				grand_parent = parent->parent;
				grand_parent->color_type = FxColorType_Red;
				rotate_right(grand_parent);
				break;
			}
			else {
				parent->color_type = FxColorType_Black;
				grand_parent->color_type = FxColorType_Red;
				rotate_right(grand_parent);
				break;
			}
		}
		else {
			FxArc *left_unccle = grand_parent->l_child;
			if (fx_node_color(left_unccle) == FxColorType_Red) {
				parent->color_type = FxColorType_Black;
				if (left_unccle)left_unccle->color_type = FxColorType_Black;
				grand_parent->color_type = FxColorType_Red;
				insert_node = grand_parent;
			}
			else if (insert_node == parent->l_child) {
				insert_node = parent;
				rotate_right(insert_node);

				parent = insert_node->parent;
				parent->color_type = FxColorType_Black;
				grand_parent = parent->parent;
				grand_parent->color_type = FxColorType_Red;
				rotate_left(grand_parent);
				break;
			}
			else {
				parent->color_type = FxColorType_Black;
				grand_parent->color_type = FxColorType_Red;
				rotate_left(grand_parent);
				break;
			}
		}
	}
	_root->color_type = FxColorType_Black;
}

void FxBeachLine::replace_node_case(FxArc *old_node, FxArc *new_node) {
	FxArc *parent = old_node->parent;
	if (!parent)
		_root = new_node;
	else if (parent->l_child == old_node)
		parent->l_child = new_node;
	else 
		parent->r_child = new_node;

	if(new_node != nullptr)
		new_node->parent = parent;
}

void FxBeachLine::replace(FxArc *old_node, FxArc *new_node) {
	replace_node_case(old_node, new_node);
	//child
	new_node->l_child = old_node->l_child;
	if (new_node->l_child)
		new_node->l_child->parent = new_node;

	new_node->r_child = old_node->r_child;
	if (new_node->r_child)
		new_node->r_child->parent = new_node;
	assert(new_node->parent != new_node);
	//color of node
	new_node->color_type = old_node->color_type;

	new_node->prev = old_node->prev;
	new_node->next = old_node->next;

	if (old_node->prev != nullptr)
		old_node->prev->next = new_node;
	if (old_node->next != nullptr)
		old_node->next->prev = new_node;
	check_cascade(_root);
	debug_node();
}

FxArc *FxBeachLine::remove(FxArc *search_node) {
	if (_node_size == 1) {
		_root = nullptr;
		release(search_node);
		return nullptr;
	}
	if (_node_size == 8) {
		int xx = 0;
		int yy = 0;
	}
	FxColorType  old_color= search_node->color_type;
	FxArc  *replace_node = nullptr,*other_node = nullptr,*parent_node = search_node->parent;
	if (!search_node->l_child) {
		replace_node = search_node->r_child;
		replace_node_case(search_node, replace_node);
	}
	else if (!search_node->r_child) {
		replace_node = search_node->l_child;
		replace_node_case(search_node,replace_node);
	}
	else {
		other_node = search_node->next;
		old_color = other_node->color_type;
		replace_node = other_node->r_child;

		if (other_node->parent != search_node) {
			parent_node = other_node->parent;
			replace_node_case(other_node, other_node->r_child);
			other_node->r_child = search_node->r_child;
			other_node->r_child->parent = other_node;
		}
		else 
			parent_node = other_node;

		replace_node_case(search_node, other_node);
		other_node->l_child = search_node->l_child;
		other_node->l_child->parent = other_node;
		other_node->color_type = search_node->color_type;
	}

	if (old_color == FxColorType_Black)
		remove_fixup(parent_node, replace_node);

	if (search_node->prev != nullptr)
		search_node->prev->next = search_node->next;
	if (search_node->next != nullptr)
		search_node->next->prev = search_node->prev;

	release(search_node);
	--_node_size;

	check_cascade(_root);
	debug_node();
	return nullptr;
};

void FxBeachLine::remove_fixup(FxArc *parent_node, FxArc *replace_node) {
	//replace_node始终具有二重黑色,该算法将始终围绕这个核心内容进行运算
	while (fx_node_color(replace_node) == FxColorType_Black && replace_node != _root) {
		if (parent_node->l_child == replace_node) {
			FxArc	*sibling_node = parent_node->r_child;//assert(sibling_node != nullptr)
			assert(sibling_node != nullptr);
			//此时需要做出一些些变换,并将变换后的情况转换为后面的三种情况中的任何一种
			if (sibling_node->color_type == FxColorType_Red) {
				sibling_node->color_type = FxColorType_Black;
				parent_node->color_type = FxColorType_Red;
				rotate_left(parent_node);
				sibling_node = parent_node->r_child;
			}
			//Case 1:如果两个后继节点同时为黑色,此时情况比较简单
			if (fx_node_color(sibling_node->l_child) == FxColorType_Black && fx_node_color(sibling_node->r_child) == FxColorType_Black) {
				sibling_node->color_type = FxColorType_Red;
				replace_node = parent_node;
				parent_node = replace_node->parent;
			}
			else if (fx_node_color(sibling_node->l_child) == FxColorType_Red) {
				sibling_node->color_type = FxColorType_Red;
				sibling_node->l_child->color_type = FxColorType_Black;
				rotate_right(sibling_node);
				sibling_node = parent_node->r_child;

				sibling_node->color_type = parent_node->color_type;
				parent_node->color_type = FxColorType_Black;
				sibling_node->r_child->color_type = FxColorType_Black;
				rotate_left(parent_node);
				replace_node = _root;
				break;
			}
			else {
				sibling_node->color_type = parent_node->color_type;
				sibling_node->r_child->color_type = FxColorType_Black;
				parent_node->color_type = FxColorType_Black;
				rotate_left(parent_node);
				replace_node = _root;
				break;
			}
		}
		else {
			FxArc *sibling_node = parent_node->l_child;
			assert(sibling_node != nullptr);
			if (sibling_node->color_type == FxColorType_Red) {
				sibling_node->color_type = FxColorType_Black;
				parent_node->color_type = FxColorType_Red;
				rotate_right(parent_node);
				sibling_node = parent_node->l_child;
			}
			if (fx_node_color(sibling_node->l_child) == FxColorType_Black && fx_node_color(sibling_node->r_child) == FxColorType_Black) {
				sibling_node->color_type = FxColorType_Red;
				replace_node = parent_node;
				parent_node = replace_node->parent;
			}
			else if (fx_node_color(sibling_node->r_child) == FxColorType_Red) {
				sibling_node->color_type = FxColorType_Red;
				sibling_node->r_child->color_type = FxColorType_Black;
				rotate_left(sibling_node);
				sibling_node = parent_node->l_child;

				sibling_node->color_type = parent_node->color_type;
				parent_node->color_type = FxColorType_Black;
				sibling_node->l_child->color_type = FxColorType_Black;
				rotate_right(parent_node);
				replace_node = _root;
				break;
			}
			else {
				sibling_node->color_type = parent_node->color_type;
				sibling_node->l_child->color_type = FxColorType_Black;
				parent_node->color_type = FxColorType_Black;
				rotate_right(parent_node);
				replace_node = _root;
				break;
			}
		}
	}
	replace_node->color_type = FxColorType_Black;
}

FxArc* FxBeachLine::find_mostright(FxArc* n) {
	assert(n != nullptr);
	while (n->r_child != nullptr) {
		n = n->r_child;
	}
	return n;
}

void FxBeachLine::rotate_left(FxArc* n) {
	FxArc* r = n->r_child, *parent = n->parent;

	if (parent != nullptr) {
		if (parent->l_child == n)
			parent->l_child = r;
		else parent->r_child = r;
	}
	else
		_root = r;

	n->r_child = r->l_child;
	if (n->r_child)n->r_child->parent = n;

	r->l_child = n;
	n->parent = r;
	r->parent = parent;
}

void FxBeachLine::rotate_right(FxArc* n) {
	FxArc* L = n->l_child, *parent = n->parent;

	if (parent != nullptr) {
		if (parent->l_child == n)
			parent->l_child = L;
		else parent->r_child = L;
	}
	else
		_root = L;

	n->l_child = L->r_child;
	if (n->l_child) n->l_child->parent = n;

	L->r_child = n;
	n->parent = L;
	L->parent = parent;
}
//内存管理
FxArc   *FxBeachLine::alloc(FkSite *site) {
	FxArc	*tw_node = nullptr;
	bool b_recyle = false;

	if (_cache_size) {
		tw_node = _cache_head;
		assert(tw_node->debug_bits == 1);
		_cache_head = _cache_head->r_child;
		--_cache_size;

		tw_node->site_ptr = site;
		tw_node->event_ptr = nullptr;
		tw_node->left_edge = nullptr;
		tw_node->right_edge = nullptr;

		tw_node->color_type = FxColorType_Red;
		tw_node->debug_bits = 0;
		tw_node->l_child = tw_node->r_child = tw_node->parent = tw_node->prev = tw_node->next = nullptr;
	}
	else
		tw_node = new FxArc(site,nullptr);
	return tw_node;
};
void		FxBeachLine::release(FxArc *tw_node) {
	//首先检测是否有内存管理器
	if (_cache_size < _cache_capacity) {
		tw_node->r_child = _cache_head;
		_cache_head = tw_node;
		assert(tw_node->debug_bits == 0);
		tw_node->debug_bits = 1;
		++_cache_size;
	}
	else delete tw_node;
}

void  FxBeachLine::debug_node_color(FxArc *n) {
	if (fx_node_color(n) == FxColorType_Red)
		assert(fx_node_color(n->parent) == FxColorType_Black && fx_node_color(n->l_child) == FxColorType_Black && fx_node_color(n->r_child) == FxColorType_Black);
}

void FxBeachLine::debug_node(std::function<float(const FxArc *, const FxArc *)> &compute_func) {
	FxArc	 *node = find_mostleft(),*assert_node = nullptr;
	float locate_x = -FLT_MAX;
	int      loop_count = 0;
	while (node != nullptr) {
		float  x = FLT_MAX;
		if (node->next != nullptr)
			x = compute_func(node,node->next);

		assert(locate_x <= x);
		assert(node->prev == assert_node);
		debug_node_color(node);

		assert_node = node;
		node = node->next;
		locate_x = x;
		++loop_count;
	}
}

void FxBeachLine::debug_node() {
	int black_count = -1;
	FxArc  *node = find_mostleft();
	while (node != nullptr) {
		assert(long(node->l_child) != 0xdddddddd && long(node->r_child) != 0xdddddddd);
		if (!node->l_child && !node->r_child) {
			int new_count = node->color_type == FxColorType_Red?0:1;
			FxArc *parent = node->parent;
			while (parent != nullptr) {
				if (parent->color_type == FxColorType_Black)
					++new_count;
				parent = parent->parent;
			}
			if (black_count == -1) black_count = new_count;
			assert(new_count == black_count);
		}
		assert(long(node->l_child) != 0xdddddddd && long(node->r_child) != 0xdddddddd);
		
		node = node->next;
	}
}

void  FxBeachLine::check_cascade(FxArc  *check_node) {
	if (check_node != nullptr) {
		assert(long(check_node) != 0xdddddddd);
		check_cascade(check_node->l_child);
		check_cascade(check_node->r_child);
	}
}
NS_GT_END