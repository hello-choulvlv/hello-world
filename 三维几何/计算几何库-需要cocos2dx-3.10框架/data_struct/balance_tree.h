/*
  *平衡树模板算法
  *该数据结构的主要用途在线段扫描线算法系列中使用
  *version:2.0增加内存分配器
  *@date:2019年10月30日
  *@author:xiaohuaxiong
 */
#ifndef __BALANCE_TREE_H__
#define __BALANCE_TREE_H__
#include "gt_common/geometry_types.h"
#include "memory_alloc.h"
#include<assert.h>
#include<functional>
NS_GT_BEGIN

enum ColorType { ColorType_Red, ColorType_Black };
#define node_color(node) ((!node)?ColorType_Black:(node)->color_type)
#define __check_red_black_property 0
#define __debug_valid 0

//red_black_tree专用的红内存分配器
template<typename MN,typename TV>
class red_black_tree_alloc {
	MN  *_cache_head;
	int    _cache_size, _cache_capacity;
public:
	red_black_tree_alloc(const red_black_tree_alloc &) = delete;
	red_black_tree_alloc& operator=(const red_black_tree_alloc &) = delete;

	red_black_tree_alloc(int cache_capacity = 0x7FFFFFFF) :_cache_head(nullptr),_cache_size(0),_cache_capacity(cache_capacity){};
	~red_black_tree_alloc() {
		while (_cache_head)
		{
			MN  *node = _cache_head->r_child;
			delete _cache_head;
			_cache_head = node;
		}
		_cache_head = nullptr;
		_cache_size = 0;
		_cache_capacity = 0;
	};

	void release(MN *node) {
		if (_cache_size < _cache_capacity) {
			node->r_child = _cache_head;
			_cache_head = node;
			++_cache_size;
		}
		else delete node;
	};

	MN* alloc(const TV &tv_value,bool &b_recycle) {
		MN *node = nullptr;
		if (_cache_head) {
			node = _cache_head;
			_cache_head = node->r_child;
			--_cache_size;
			b_recycle = true;
		}
		else {
			node = new MN(tv_value);
			b_recycle = false;
		}
		return node;
	};
};

template<typename TW>
class red_black_tree {
public:
	struct internal_node
	{
		TW						tw_value;
		ColorType			color_type;
		internal_node  *parent, *l_child, *r_child;
		internal_node(const TW &tv) :tw_value(tv), color_type(ColorType_Red),parent(nullptr), l_child(nullptr), r_child(nullptr) {};
		internal_node(const TW &t_value, ColorType node_color2, internal_node* la_child, internal_node* ra_child) :tw_value(t_value), color_type(node_color2), parent(nullptr), l_child(la_child), r_child(ra_child) {};
	};
private:
	struct internal_node  *_root, *_cache_head;
	red_black_tree_alloc<internal_node, TW>   *_mem_alloc;
	int       _node_size, _cache_size, _cache_capacity;
public:
	red_black_tree(int capacity_size = 0x7FFFFFFF, red_black_tree_alloc<internal_node,TW> *mem_alloc = nullptr) : _root(nullptr), _cache_head(nullptr), _mem_alloc(mem_alloc), _node_size(0), _cache_size(0), _cache_capacity(capacity_size) {
	};
	~red_black_tree() {
		if (_node_size){
			internal_node   **node_array; 
			internal_node  *child;
			internal_node *fix_array[512];

			node_array = _node_size <= 512 ? fix_array : new internal_node  *[_node_size];
			child = find_minimum();

			int  j = 0;
			do
			{
				node_array[j++] = child;
				child = find_next(child);
			} while (child != nullptr);

			for (j = 0; j < _node_size; ++j){
				if (_mem_alloc)
					_mem_alloc->release(node_array[j]);
				else
					delete node_array[j];
			}

			if(node_array != fix_array)
				delete[] node_array;

			node_array = nullptr;
			_node_size = 0;
		}
		//_cache
		internal_node  *child = _cache_head;
		while (child){
			internal_node *t = child;
			child = child->r_child;
			delete t;
		}
		_root = nullptr;
		_cache_head = nullptr;
		_cache_size = 0;
		_cache_capacity = 0;
	};
	void clear()
	{
		if (_node_size){
			//平衡树的规模一般较小,此时可以使用固定的数组
			internal_node *fix_array[512];
			internal_node **remind_array = _node_size <= 512 ? fix_array : new internal_node*[_node_size];
			int j = 0;
			internal_node  *child = find_minimum();
			do
			{
				remind_array[j++] = child;
				child = find_next(child);
			} while (child != nullptr);
			assert(j == _node_size);

			for (int j = 0; j < _node_size; ++j){
				if (!_mem_alloc){
					remind_array[j]->r_child = _cache_head;
					_cache_head = remind_array[j];
				}
				else
					_mem_alloc->release(remind_array[j]);
			}
			if (!_mem_alloc)
				_cache_size += _node_size;
			
			if (remind_array != fix_array){
				delete[] remind_array;
				remind_array = nullptr;
			}
			_root = nullptr;
			_node_size = 0;
		}
	};
	int size()const { return _node_size; };

	internal_node *find_next(internal_node  *node)
	{
		internal_node  *child = node->r_child;
		if (child != nullptr){
			while (child->l_child)
				child = child->l_child;
			return child;
		}

		internal_node *parent = node->parent;
		while (parent && parent->r_child == node) {
			node = parent;
			parent = parent->parent;
		}
		return parent;
	};

	internal_node *find_prev(internal_node  *node)
	{
		internal_node  *child = node->l_child;
		if (child != nullptr){
			while (child->r_child)
				child = child->r_child;
		}
		else{
			for (child = node->parent; child != nullptr && node == child->l_child; child = child->parent)
				node = child;
		}
		return child;
	};

	internal_node *find_minimum()
	{
		internal_node  *child = _root;
		while (child && child->l_child)//first
			child = child->l_child;
		return child;
	};

	internal_node* lookup(const TW &tw_value, std::function<int (const TW &, const TW &)> &compare_func) {
		internal_node* n = _root;
		while (n != nullptr) {
			int comp_result = compare_func(tw_value, n->tw_value);
			if (comp_result == 0) {
				return n;
			}
			else if (comp_result < 0) {
				n = n->l_child;
			}
			else {
				assert(comp_result > 0);
				n = n->r_child;
			}
		}
		return n;
	};

	internal_node* insert(const TW &tw_value, std::function<int(const TW &, const TW &)> &compare_func) {
		internal_node* inserted_node = nullptr;
		if (_root == nullptr) {
			_root = inserted_node =  apply_memory(tw_value);
		}
		else {
			internal_node* n = _root;
			while (1) {
				int comp_result = compare_func(tw_value, n->tw_value);
				if (comp_result == 0) {
					return nullptr;
				}
				else if (comp_result < 0) {
					if (n->l_child == nullptr) {
						inserted_node = apply_memory(tw_value);
						n->l_child = inserted_node;
						break;
					}
					else {
						n = n->l_child;
					}
				}
				else {
					assert(comp_result > 0);
					if (n->r_child == nullptr) {
						inserted_node = apply_memory(tw_value);
						n->r_child = inserted_node;
						break;
					}
					else {
						n = n->r_child;
					}
				}
			}
			inserted_node->parent = n;
		}
		insert_case(inserted_node);
		verify_properties();
		return inserted_node;
	};
	/*
	  *插入节点之后的修正
	 */
	void insert_case(internal_node *insert_node){
		if (!insert_node->parent) {
			insert_node->color_type = ColorType_Black;
			return;
		}
		while (node_color(insert_node->parent) == ColorType_Red){
			//两大判断分支,已知parent的颜色为红色,则其必然有父节点
			internal_node *parent = insert_node->parent;
			internal_node *grand_parent = parent->parent;
			if (parent == grand_parent->l_child) {
				internal_node *right_uncle = grand_parent->r_child;
				if (node_color(right_uncle) == ColorType_Red) {//叔叔节点
					parent->color_type = ColorType_Black;
					if (right_uncle)right_uncle->color_type = ColorType_Black;
					grand_parent->color_type = ColorType_Red;
					insert_node = grand_parent;
				}
				else if (insert_node == parent->r_child) {//此时已知叔叔节点的颜色为黑色,因此需要做出复杂的变换
					insert_node = parent;
					rotate_left(insert_node);

					parent = insert_node->parent;
					parent->color_type = ColorType_Black;
					grand_parent = parent->parent;
					grand_parent->color_type = ColorType_Red;
					rotate_right(grand_parent);
					break;
				}
				else {
					parent->color_type = ColorType_Black;
					grand_parent->color_type = ColorType_Red;
					rotate_right(grand_parent);
					break;
				}
			}
			else {
				internal_node *left_unccle = grand_parent->l_child;
				if (node_color(left_unccle) == ColorType_Red) {
					parent->color_type = ColorType_Black;
					if (left_unccle)left_unccle->color_type = ColorType_Black;
					grand_parent->color_type = ColorType_Red;
					insert_node = grand_parent;
				}
				else if (insert_node == parent->l_child) {
					insert_node = parent;
					rotate_right(insert_node);

					parent = insert_node->parent;
					parent->color_type = ColorType_Black;
					grand_parent = parent->parent;
					grand_parent->color_type = ColorType_Red;
					rotate_left(grand_parent);
					break;
				}
				else {
					parent->color_type = ColorType_Black;
					grand_parent->color_type = ColorType_Red;
					rotate_left(grand_parent);
					break;
				}
			}
		}
		_root->color_type = ColorType_Black;
	}
	//删除相关的数值,并返回下一个节点
	internal_node* remove(const TW &tw_value, std::function<int(const TW &, const TW &)> &compare_func) {
		internal_node* search_node = lookup(tw_value, compare_func);
		if (search_node)
			return remove_case(search_node);
		return nullptr;
	};

	void replace_node_case(internal_node *old_node,internal_node *new_node) {
		internal_node *parent = old_node->parent;
		if (!parent)
			_root = new_node;
		else if (parent->l_child == old_node)
			parent->l_child = new_node;
		else parent->r_child = new_node;

		new_node->parent = parent;
		//child
		new_node->l_child = old_node->l_child;
		if (new_node->l_child)
			new_node->l_child->parent = new_node;

		new_node->r_child = old_node->r_child;
		if (new_node->r_child)
			new_node->r_child->parent = new_node;
		//color of node
		new_node->color_type = old_node->color_type;
	}

	internal_node *remove_case(internal_node *search_node){
		if (_node_size == 1) {
			_root = nullptr;
			release_memory(search_node);
			return nullptr;
		}
		internal_node *next_node = find_next(search_node);
		internal_node *target_node = nullptr;
		//以下代码的目标是,查找一个层级最低的替代节点,并且由于red_black_tree的性质,target_node的层级不可能比search_node的更高
		if (!search_node->l_child || !search_node->r_child)
			target_node = search_node;
		else
			target_node = next_node;

		//如果replace_node != nullptr,则其必为target_node的唯一的子节点
		internal_node *replace_node = target_node->l_child? target_node->l_child: target_node->r_child;
		internal_node *parent_node = target_node->parent;
		if (replace_node != nullptr)replace_node->parent = parent_node;

		if (!parent_node)
			_root = replace_node;
		else if (parent_node->l_child == target_node)
			parent_node->l_child = replace_node;
		else
			parent_node->r_child = replace_node;

		//替换相关的数据,search_node -> target_node,这个过程只修改指针,而不能修改数据本身
		ColorType old_color = target_node->color_type;
		if (target_node != search_node) {
			//注意,这一步代码只有当target_node是search_node的直接子节点时,才会起作用,少了这一句代码,算法将直接错乱
			if (parent_node == search_node)parent_node = target_node;
			replace_node_case(search_node, target_node);
		}
		//该行代码保证了如果算法在没有保证数据一致性时,将直接发生崩溃,而非继续沿着错误的道路继续走下去
		//search_node->parent = search_node->l_child = search_node->r_child = nullptr;
		//如果删除的目标不是红色的节点,则预示着red_black_tree的平衡已经被破坏了
		if (old_color == ColorType_Black)
			remove_case(parent_node, replace_node);
		release_memory(search_node);
		verify_properties();
		return next_node;
	};

	void remove_case(internal_node *parent_node,internal_node *replace_node) {
		//replace_node始终具有二重黑色,该算法将始终围绕这个核心内容进行运算
		while (node_color(replace_node) == ColorType_Black && replace_node != _root) {
			if (parent_node->l_child == replace_node) {
				internal_node	*sibling_node = parent_node->r_child;//assert(sibling_node != nullptr)
				assert(sibling_node != nullptr);
				//此时需要做出一些些变换,并将变换后的情况转换为后面的三种情况中的任何一种
				if (sibling_node->color_type == ColorType_Red) {
					sibling_node->color_type = ColorType_Black;
					parent_node->color_type = ColorType_Red;
					rotate_left(parent_node);
					sibling_node = parent_node->r_child;
				}
				//Case 1:如果两个后继节点同时为黑色,此时情况比较简单
				if (node_color(sibling_node->l_child) == ColorType_Black && node_color(sibling_node->r_child) == ColorType_Black) {
					sibling_node->color_type = ColorType_Red;
					replace_node = parent_node;
					parent_node = replace_node->parent;
				}
				else if (node_color(sibling_node->l_child) == ColorType_Red) {
					sibling_node->color_type = ColorType_Red;
					sibling_node->l_child->color_type = ColorType_Black;
					rotate_right(sibling_node);
					sibling_node = parent_node->r_child;

					sibling_node->color_type = parent_node->color_type;
					parent_node->color_type = ColorType_Black;
					sibling_node->r_child->color_type = ColorType_Black;
					rotate_left(parent_node);
					replace_node = _root;
					break;
				}
				else {
					sibling_node->color_type = parent_node->color_type;
					sibling_node->r_child->color_type = ColorType_Black;
					parent_node->color_type = ColorType_Black;
					rotate_left(parent_node);
					replace_node = _root;
					break;
				}
			}
			else {
				internal_node *sibling_node = parent_node->l_child;
				assert(sibling_node != nullptr);
				if (sibling_node->color_type == ColorType_Red) {
					sibling_node->color_type = ColorType_Black;
					parent_node->color_type = ColorType_Red;
					rotate_right(parent_node);
					sibling_node = parent_node->l_child;
				}
				if (node_color(sibling_node->l_child) == ColorType_Black && node_color(sibling_node->r_child) == ColorType_Black) {
					sibling_node->color_type = ColorType_Red;
					replace_node = parent_node;
					parent_node = replace_node->parent;
				}
				else if (node_color(sibling_node->r_child) == ColorType_Red) {
					sibling_node->color_type = ColorType_Red;
					sibling_node->r_child->color_type = ColorType_Black;
					rotate_left(sibling_node);
					sibling_node = parent_node->l_child;

					sibling_node->color_type = parent_node->color_type;
					parent_node->color_type = ColorType_Black;
					sibling_node->l_child->color_type = ColorType_Black;
					rotate_right(parent_node);
					replace_node = _root;
					break;
				}
				else {
					sibling_node->color_type = parent_node->color_type;
					sibling_node->l_child->color_type = ColorType_Black;
					parent_node->color_type = ColorType_Black;
					rotate_right(parent_node);
					replace_node = _root;
					break;
				}
			}
		}
		replace_node->color_type = ColorType_Black;
	}

	internal_node* find_maximum(internal_node* n) {
		assert(n != nullptr);
		while (n->r_child != nullptr) {
			n = n->r_child;
		}
		return n;
	};
private:
	void verify_properties() {
#if __check_red_black_property
		property_1(_root);
		property_2(_root);
		property_4(_root);
		property_5(_root);
#endif
	};

	void property_1(internal_node* n) {
		assert(node_color(n) == ColorType_Red || node_color(n) == ColorType_Black);
		if (n == nullptr) return;
		property_1(n->l_child);
		property_1(n->r_child);
	};

	void property_2(internal_node* _root) {
		assert(node_color(_root) == ColorType_Black);
	};

	void property_4(internal_node* n) {
		if (node_color(n) == ColorType_Red) {
			assert(node_color(n->l_child) == ColorType_Black);
			assert(node_color(n->r_child) == ColorType_Black);
			assert(node_color(n->parent) == ColorType_Black);
		}
		if (n == nullptr) return;
		property_4(n->l_child);
		property_4(n->r_child);
	};

	void property_5(internal_node* root) {
		int black_count_path = -1;
		property_5_helper(_root, 0, &black_count_path);
	};

	void property_5_helper(internal_node* n, int black_count, int* path_black_count) {
		if (node_color(n) == ColorType_Black) {
			black_count++;
		}
		if (n == nullptr) {
			if (*path_black_count == -1) {
				*path_black_count = black_count;
			}
			else {
				assert(black_count == *path_black_count);
			}
			return;
		}
		property_5_helper(n->l_child, black_count, path_black_count);
		property_5_helper(n->r_child, black_count, path_black_count);
	};

	void rotate_left(internal_node* n) {
		internal_node* r = n->r_child, *parent = n->parent;

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
	};

	void rotate_right(internal_node* n) {
		internal_node* L = n->l_child,*parent = n->parent;
		
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
	};
	//内存管理
	internal_node   *apply_memory(const TW &tv_value) {
		internal_node	*tw_node = nullptr;
		++_node_size;
		bool b_recyle = false;
		if (_mem_alloc){
			tw_node = _mem_alloc->alloc(tv_value,b_recyle);
			if (b_recyle) {
				if(&tv_value != &tw_node->tw_value)//直接比较对象的地址,在有些算法中,会使用到red_black_tree的结构技巧
					tw_node->tw_value = tv_value;
				tw_node->color_type = ColorType_Red;
				 tw_node->parent = tw_node->l_child = tw_node->r_child = nullptr;
			}
			return tw_node;
		}

		if (_cache_size) {
			tw_node = _cache_head;
			_cache_head = _cache_head->r_child;
			--_cache_size;
			if(&tw_node->tw_value != &tv_value)
				tw_node->tw_value = tv_value;
			tw_node->color_type = ColorType_Red;
			 tw_node->parent = tw_node->l_child = tw_node->r_child = nullptr;
		}
		else
			tw_node = new internal_node(tv_value);
		return tw_node;
	};
	void		release_memory(internal_node *tw_node) {
		--_node_size;
		//首先检测是否有内存管理器
		if (_mem_alloc){
			_mem_alloc->release(tw_node);
			return;
		}

		if (_cache_size < _cache_capacity){
			tw_node->r_child = _cache_head;
			_cache_head = tw_node;
			++_cache_size;
		}
		else delete tw_node;
	};
}; 
#undef node_color
#undef __check_red_black_property 
#undef __debug_valid 

NS_GT_END
#endif