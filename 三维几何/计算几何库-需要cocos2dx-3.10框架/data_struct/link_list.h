/*
  *双向链表
  *2020年1月27日
  *@author:xiaohuaxiong
 */
#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__
#include "gt_common/geometry_types.h"
#include<functional>
NS_GT_BEGIN

template<typename KT>
class link_list<KT>
{
public:
	struct link_node
	{
		KT     kt_value;
		link_node *prev, *next;

		link_node(const KT &akt_value) :kt_value(akt_value),prev(nullptr),next(nullptr) {};
	};
private:
	link_node  *_head_root,*_tail_root,*_cache;
	int                  _node_size,_cache_size, _cache_capacity;
public:
	link_list(int cache_capacity = 0x7FFFFFFF):_head_root(nullptr), _tail_root(nullptr),_cache(nullptr),_node_size(0),_cache_size(0), _cache_capacity(cache_capacity){};

	~link_list() {
		link_node *node = _head_root;
		while (node)
		{
			link_node *s = node->next;
			delete node;
			node = s;
		}

		node = _cache;
		while (node)
		{
			link_node *s = node->next;
			delete node;
			node = s;
		}
	};
	link_node *push_front(const KT &kt_value) {
		link_node *node = apply(kt_value);
		node->next = _head_root;

		if (_head_root)_head_root->prev = node;
		else _tail_root = node;

		_head_root = node;
		++node_size;

		return node;
	};

	link_node *push_back(const KT &kt_value)
	{
		link_node *node = apply(kt_value);
		node->prev = _tail_root;
		if (_tail_root)_tail_root->next = node;
		else _head_root = node;

		_tail_root = node;
		++_node_size;

		return node;
	}

	link_node *head()const { return _head_root; };
	link_node *back()const { return _tail_root; };

	link_node *insert_before(link_node *interval_node, const KT &kt_value) {
		link_node *node = apply(kt_value);
		if (!interval_node->prev)
			_head_root = node;
		else
		{
			interval_node->prev->next = node;
			node->prev = interval_node->prev;
		}
		interval_node->prev = node;
		node->next = interval_node;
		++_node_size;
		return node;
	};

	link_node insert_after(link_node *interval_node, const KT &kt_value) {
		link_node *node = apply(kt_value);
		if (!interval_node)
			_tail_root = node;
		else
		{
			interval_node->next->prev = node;
			node->next = interval_node->next;
		}
		node->prev = interval_node;
		interval_node->next = node;
		++_node_size;
		return node;
	};

	link_node *lookup(const KT &kt_value,const std::function<bool (const KT &a,const KT &b)> &compare_func) {
		link_node *node = _head_root;
		while (node && !compare_func(kt_value, node->kt_value))
			node = node->next;
		return node;
	};

	link_node* remove(const KT &kt_value, const std::function<bool(const KT &a, const KT &b)> &compare_func)
	{
		link_node *node = _head_root;
		while (node && !compare_func(kt_value, node->kt_value))
			node = node->next;
		if (!node)return nullptr;

		link_node *prev = node->prev, *next = node->next;
		if (!prev)_head_root = next;
		else prev->next = next

		if (!next) _tail_root = prev;
		else next->prev = prev;

		release(node);
		return next;
	}

	link_node *remove(link_node *interval_node)
	{
		link_node *prev = interval_node->prev, *next = interval_node->next;

		if (!prev)_head_root = next;
		else prev->next = next

		if (!next) _tail_root = prev;
		else next->prev = prev;

		release(interval_node);
		return next;
	}

	link_node *apply(const KT &kt_value) {
		link_node *node = nullptr;
		if (_cache_size)
		{
			node = _cache;
			_cache = _cache->next;
			node->kt_value = kt_value;
			node->prev = node->next = nullptr;
			--_cache_size;
		}
		else
			node = new link_node(kt_value);
		return node;
	};
	void release(link_node *interval_node) {
		if (_cache_size < _cache_capacity)
		{
			interval_node->next = _cache;
			_cache = interval_node;
			++_cache_size;
		}
		else
			delete interval_node;
	};
};

NS_GT_END
#endif