/*
  *平衡树模板算法
  *该数据结构的主要用途在线段扫描线算法系列中使用
  *@date:2019年10月30日
  *@author:xiaohuaxiong
 */
#ifndef __BALANCE_TREE_H__
#define __BALANCE_TREE_H__
#include "gt_common/geometry_types.h"
#include<assert.h>
NS_GT_BEGIN
#include <assert.h>

enum ColorType { ColorType_Red, ColorType_Black };
#define node_color(node) ((!node)?ColorType_Black:(node)->color_type)
#define __check_red_black_protity 0

template<typename TW, typename SV>
class red_black_tree {
public:
	struct internal_node
	{
		TW						tw_value;
		ColorType			color_type;
		internal_node  *parent, *l_child, *r_child;

		internal_node(const TW &tv) :tw_value(tv), color_type(ColorType_Red), parent(nullptr), l_child(nullptr), r_child(nullptr) {
		};
		internal_node(const TW &t_value, ColorType node_color, internal_node* la_child, internal_node* ra_child) :tw_value(t_value), color_type(node_color), parent(nullptr), l_child(la_child), r_child(ra_child) {};
	};
private:
	struct internal_node  *_root, *_cache_head;
	int       _node_size, _cache_size, _cache_capacity;
	//比较函数,稍后读者将会发现最后面的一个参数是怎么来的
	int(*_compare_func)(const TW &a, const TW &b, const SV &l);
public:
	red_black_tree(int(*compare_func)(const TW &, const TW &b, const SV &), int capacity_size = 8) :_compare_func(compare_func), _root(nullptr), _cache_head(nullptr), _node_size(0), _cache_size(0), _cache_capacity(capacity_size) {
	};
	~red_black_tree() {
		if (_node_size)
		{
			internal_node   **node_array = new internal_node   *[_node_size];
			internal_node  *child = find_minimum();
			int  index_j = 0;
			node_array[index_j++] = child;
			while ((child = find_next(child)) != nullptr)
				node_array[index_j++] = child;
			for (int index_l = 0; index_l < _node_size; ++index_l)
				delete node_array[index_l];
			delete[] node_array;
		}
		//_cache
		internal_node  *child = _cache_head;
		while (child)
		{
			internal_node *t = child;
			child = child->r_child;
			delete t;
		}
		_root = nullptr;
		_cache_head = nullptr;
		_node_size = 0;
		_cache_size = 0;
		_cache_capacity = 0;
	};
public:

	//typedef int(*compare_func)(void* l_child, void* r_child);
	internal_node *find_next(internal_node  *node)
	{
		internal_node  *child = node->r_child;
		if (child != nullptr)
		{
			while (child->l_child)
				child = child->l_child;
		}
		else
		{
			for (child = node->parent; child != nullptr && node == child->r_child; child = child->parent)
				node = child;
		}
		return child;
	}

	internal_node *find_previous(internal_node  *node)
	{
		internal_node  *child = node->l_child
			if (child != nullptr)
			{
				while (child->r_child)
					child = child->r_child;
			}
			else
			{
				for (child = node->parent; child != nullptr && node == child->l_child; child = child->parent)
					node = child;
			}
		return child;
	}

	internal_node *find_minimum()
	{
		internal_node  *child = _root;
		while (child->l_child)//first
			child = child->l_child;
		return child;
	}

	internal_node* lookup(const TW &tw_value, const SV &sv_value) {
		internal_node* n = _root;
		while (n != nullptr) {
			int comp_result = _compare_func(tw_value, n->tw_value, sv_value);
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
	}

	void insert(const TW &tw_value, const SV &sv_value) {
		internal_node* inserted_node = nullptr;// new_node(tw_value, ColorType_Red, nullptr, nullptr);
		if (_root == nullptr) {
			_root = inserted_node =  apply_memory(tw_value);
		}
		else {
			internal_node* n = _root;
			while (1) {
				int comp_result = _compare_func(tw_value, n->tw_value, sv_value);
				if (comp_result == 0) {
					return;
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
		insert_case1(inserted_node);
		verify_properties();
	}

	void  remove(internal_node  *search_node)
	{
		if (search_node->l_child != nullptr && search_node->r_child != nullptr) {
			internal_node* pred = find_maximum(search_node->l_child);
			search_node->tw_value = pred->tw_value;
			search_node = pred;
		}

		assert(search_node->l_child == nullptr || search_node->r_child == nullptr);
		internal_node *child = search_node->r_child == nullptr ? search_node->l_child : search_node->r_child;
		if (node_color(search_node) == ColorType_Black) {
			search_node->color_type = node_color(child);
			delete_case1(search_node);
		}
		replace_node(search_node, child);
		if (search_node->parent == nullptr && child != nullptr)
			child->color_type = ColorType_Black;
		release_memory(search_node);

		verify_properties();
	}

	void remove(const TW &tw_value, const SV &sv_value) {
		internal_node* search_node = lookup(tw_value, sv_value);
		if (!search_node)
			remove(search_node);
	}

	internal_node* find_maximum(internal_node* n) {
		assert(n != nullptr);
		while (n->r_child != nullptr) {
			n = n->r_child;
		}
		return n;
	}
private:
	void delete_case1(internal_node* n) {
		if (n->parent == nullptr)
			return;
		else
			delete_case2(n);
	}

	void delete_case2(internal_node* n) {
		if (node_color(sibling(n)) == ColorType_Red) {
			n->parent->color_type = ColorType_Red;
			sibling(n)->color_type = ColorType_Black;
			if (n == n->parent->l_child)
				rotate_left(n->parent);
			else
				rotate_right(n->parent);
		}
		delete_case3(n);
	}

	void delete_case3(internal_node* n) {
		if (node_color(n->parent) == ColorType_Black &&
			node_color(sibling(n)) == ColorType_Black &&
			node_color(sibling(n)->l_child) == ColorType_Black &&
			node_color(sibling(n)->r_child) == ColorType_Black)
		{
			sibling(n)->color_type = ColorType_Red;
			delete_case1(n->parent);
		}
		else
			delete_case4(n);
	}

	void delete_case4(internal_node* n) {
		if (node_color(n->parent) == ColorType_Red &&
			node_color(sibling(n)) == ColorType_Black &&
			node_color(sibling(n)->l_child) == ColorType_Black &&
			node_color(sibling(n)->r_child) == ColorType_Black)
		{
			sibling(n)->color_type = ColorType_Red;
			n->parent->color_type = ColorType_Black;
		}
		else
			delete_case5(n);
	}

	void delete_case5(internal_node* n) {
		if (n == n->parent->l_child &&
			node_color(sibling(n)) == ColorType_Black &&
			node_color(sibling(n)->l_child) == ColorType_Red &&
			node_color(sibling(n)->r_child) == ColorType_Black)
		{
			sibling(n)->color_type = ColorType_Red;
			sibling(n)->l_child->color_type = ColorType_Black;
			rotate_right(sibling(n));
		}
		else if (n == n->parent->r_child &&
			node_color(sibling(n)) == ColorType_Black &&
			node_color(sibling(n)->r_child) == ColorType_Red &&
			node_color(sibling(n)->l_child) == ColorType_Black)
		{
			sibling(n)->color_type = ColorType_Red;
			sibling(n)->r_child->color_type = ColorType_Black;
			rotate_left(sibling(n));
		}
		delete_case6(n);
	}

	void delete_case6(internal_node* n) {
		sibling(n)->color_type = node_color(n->parent);
		n->parent->color_type = ColorType_Black;
		if (n == n->parent->l_child) {
			assert(node_color(sibling(n)->r_child) == ColorType_Red);
			sibling(n)->r_child->color_type = ColorType_Black;
			rotate_left(n->parent);
		}
		else
		{
			assert(node_color(sibling(n)->l_child) == ColorType_Red);
			sibling(n)->l_child->color_type = ColorType_Black;
			rotate_right(n->parent);
		}
	}
	internal_node* grandparent(internal_node* n) {
		assert(n != nullptr);
		assert(n->parent != nullptr);
		assert(n->parent->parent != nullptr);
		return n->parent->parent;
	}

	internal_node* sibling(internal_node* n) {
		assert(n != nullptr);
		assert(n->parent != nullptr);
		if (n == n->parent->l_child)
			return n->parent->r_child;
		else
			return n->parent->l_child;
	}

	internal_node* uncle(internal_node* n) {
		assert(n != nullptr);
		assert(n->parent != nullptr);
		assert(n->parent->parent != nullptr);
		return sibling(n->parent);
	}

	void verify_properties() {
#if __check_red_black_protity
		property_1(_root);
		property_2(_root);
		property_4(_root);
		property_5(_root);
#endif
	}

	void property_1(internal_node* n) {
		assert(node_color(n) == ColorType_Red || node_color(n) == ColorType_Black);
		if (n == nullptr) return;
		property_1(n->l_child);
		property_1(n->r_child);
	}

	void property_2(internal_node* _root) {
		assert(node_color(_root) == ColorType_Black);
	}

	void property_4(internal_node* n) {
		if (node_color(n) == ColorType_Red) {
			assert(node_color(n->l_child) == ColorType_Black);
			assert(node_color(n->r_child) == ColorType_Black);
			assert(node_color(n->parent) == ColorType_Black);
		}
		if (n == nullptr) return;
		property_4(n->l_child);
		property_4(n->r_child);
	}

	void property_5(internal_node* root) {
		int black_count_path = -1;
		property_5_helper(_root, 0, &black_count_path);
	}

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
	}

	internal_node* new_node(void* tw_value, ColorType node_color, internal_node* l_child, internal_node* r_child) {
		internal_node *result = new internal_node(tw_value, node_color, l_child, r_child);
		if (l_child != nullptr)  l_child->parent = result;
		if (r_child != nullptr) r_child->parent = result;
		return result;
	}
	void rotate_left(internal_node* n) {
		internal_node* r = n->r_child;
		replace_node(n, r);
		n->r_child = r->l_child;
		if (r->l_child != nullptr) {
			r->l_child->parent = n;
		}
		r->l_child = n;
		n->parent = r;
	}

	void rotate_right(internal_node* n) {
		internal_node* L = n->l_child;
		replace_node(n, L);
		n->l_child = L->r_child;
		if (L->r_child != nullptr) {
			L->r_child->parent = n;
		}
		L->r_child = n;
		n->parent = L;
	}

	void replace_node(internal_node* oldn, internal_node* newn) {
		if (oldn->parent == nullptr) {
			_root = newn;
		}
		else {
			if (oldn == oldn->parent->l_child)
				oldn->parent->l_child = newn;
			else
				oldn->parent->r_child = newn;
		}
		if (newn != nullptr) {
			newn->parent = oldn->parent;
		}
	}
	void insert_case1(internal_node* n) {
		if (n->parent == nullptr)
			n->color_type = ColorType_Black;
		else
			insert_case2(n);
	}

	void insert_case2(internal_node* n) {
		if (node_color(n->parent) == ColorType_Black)
			return;
		else
			insert_case3(n);
	}

	void insert_case3(internal_node* n) {
		if (node_color(uncle(n)) == ColorType_Red) {
			n->parent->color_type = ColorType_Black;
			uncle(n)->color_type = ColorType_Black;
			grandparent(n)->color_type = ColorType_Red;
			insert_case1(grandparent(n));
		}
		else {
			insert_case4(n);
		}
	}

	void insert_case4(internal_node* n) {
		if (n == n->parent->r_child && n->parent == grandparent(n)->l_child) {
			rotate_left(n->parent);
			n = n->l_child;
		}
		else if (n == n->parent->l_child && n->parent == grandparent(n)->r_child) {
			rotate_right(n->parent);
			n = n->r_child;
		}
		insert_case5(n);
	}

	void insert_case5(internal_node* n) {
		n->parent->color_type = ColorType_Black;
		grandparent(n)->color_type = ColorType_Red;
		if (n == n->parent->l_child && n->parent == grandparent(n)->l_child) {
			rotate_right(grandparent(n));
		}
		else {
			assert(n == n->parent->r_child && n->parent == grandparent(n)->r_child);
			rotate_left(grandparent(n));
		}
	}
	//内存管理
	internal_node   *apply_memory(const TW &tv_value) {
		internal_node	*tw_node = nullptr;
		if (_cache_size)
		{
			tw_node = _cache_head;
			_cache_head = _cache_head->r_child;
			--_cache_size;
			tw_node->tw_value = tv_value;
			tw_node->color_type = ColorType_Red;
			tw_node->parent = nullptr;
			tw_node->l_child = tw_node->r_child = nullptr;
		}
		else
			tw_node = new internal_node(tv_value);
		++_node_size;
		return tw_node;
	};
	void		release_memory(internal_node *tw_node) {
		if (_cache_size < _cache_capacity)
		{
			tw_node->r_child = _cache_head;
			_cache_head = tw_node;
			++_cache_size;
		}
		else
			delete tw_node;
		--_node_size;
	};
};
NS_GT_END
#endif