/*
  *双向链表
  *2020年1月27日
  *@author:xiaohuaxiong
  *@date:2020年2月18日
  *@version:2.0增加任意位置的快速删除操作,但是在操作的时候需要增加一些限制
  *@version:3.0增加内存分配器
 */
#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__
#include "gt_common/geometry_types.h"
#include<functional>
NS_GT_BEGIN

template<typename MN, typename TV>
class link_list_alloc {
	MN  *_cache_head;
	int    _cache_size, _cache_capacity;
public:
	link_list_alloc(const link_list_alloc &) = delete;
	link_list_alloc& operator=(const link_list_alloc &) = delete;

	link_list_alloc(int cache_capacity = 0x7FFFFFFF) :_cache_head(nullptr), _cache_size(0), _cache_capacity(cache_capacity) {};
	~link_list_alloc() {
		while (_cache_head)
		{
			MN  *node = _cache_head->next;
			delete _cache_head;
			_cache_head = node;
		}
		_cache_head = nullptr;
		_cache_size = 0;
		_cache_capacity = 0;
	};

	void release(MN *node) {
		if (_cache_size < _cache_capacity) {
			node->next = _cache_head;
			_cache_head = node;
			++_cache_size;
		}
		else delete node;
	};

	MN* alloc(const TV &tv_value, bool &b_recycle) {
		MN *node = nullptr;
		if (_cache_head) {
			node = _cache_head;
			_cache_head = node->next;
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

template<typename KT>
class link_list
{
public:
	struct link_node
	{
		KT     tv_value;
		link_node *prev, *next;

		link_node(const KT &akt_value) :tv_value(akt_value),prev(nullptr),next(nullptr) {};
	};
	typedef link_node link_node_t;
private:
	link_node  *_head_root,*_tail_root,*_cache;
	link_list_alloc<link_node, KT>  *_mem_alloc;
	int                  _node_size,_cache_size, _cache_capacity;
public:
	link_list(const link_list &) = delete;
	link_list& operator = (const link_list &) = delete;

	link_list(int cache_capacity = 0x7FFFFFFF, link_list_alloc<link_node, KT> *mem_alloc = nullptr):_head_root(nullptr), _tail_root(nullptr),_cache(nullptr), _mem_alloc(mem_alloc),_node_size(0),_cache_size(0), _cache_capacity(cache_capacity){};

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
		++_node_size;

		return node;
	};

	link_node *push_front(link_node *node) {
		node->next = _head_root;

		if (_head_root)_head_root->prev = node;
		else _tail_root = node;

		_head_root = node;
		++_node_size;

		return node;
	};

	void pop_back(){
		link_node *prev = _tail_root->prev;
		if (!prev)_head_root = nullptr;
		else prev->next = nullptr;

		release(_tail_root);
		_tail_root = prev;
		--_node_size;
	};

	void pop_front() {
		link_node *next = _head_root->next;
		if (!next)_tail_root = nullptr;
		else next->prev = nullptr;

		release(_head_root);
		_head_root = next;
		--_node_size;
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
	};

	link_node *push_back(link_node *node)
	{
		node->prev = _tail_root;
		if (_tail_root)_tail_root->next = node;
		else _head_root = node;

		_tail_root = node;
		++_node_size;

		return node;
	};

	int                size()const { return _node_size; };
	link_node *head()const { return _head_root; };
	link_node *back()const { return _tail_root; };

	link_node *next(const link_node *node)const { return node->next; };
	link_node *prev(const link_node *node)const { return node->prev; };

	link_node *insert_before(link_node *interval_node, const KT &kt_value) {
		link_node *node = apply(kt_value);
		if (!interval_node->prev)
			_head_root = node;
		else
			interval_node->prev->next = node;
		node->prev = interval_node->prev;
		interval_node->prev = node;
		node->next = interval_node;
		++_node_size;
		return node;
	};

	link_node *insert_before(link_node *interval_node, link_node *target_node) {
		if (!interval_node->prev)
			_head_root = target_node;
		else
			interval_node->prev->next = target_node;
		target_node->prev = interval_node->prev;
		interval_node->prev = target_node;
		target_node->next = interval_node;
		++_node_size;
		return target_node;
	};

	link_node* insert_after(link_node *interval_node, const KT &kt_value) {
		link_node *node = apply(kt_value);
		if (!interval_node->next)
			_tail_root = node;
		else
			interval_node->next->prev = node;
		node->next = interval_node->next;
		node->prev = interval_node;
		interval_node->next = node;
		++_node_size;
		return node;
	};

	link_node* insert_after(link_node *interval_node, link_node *target_node) {
		if (!interval_node->next)
			_tail_root = target_node;
		else
			interval_node->next->prev = target_node;
		target_node->next = interval_node->next;
		target_node->prev = interval_node;
		interval_node->next = target_node;
		++_node_size;
		return target_node;
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
		else prev->next = next;

		if (!next) _tail_root = prev;
		else next->prev = prev;

		--_node_size;
		release(node);
		return next;
	};

	link_node *remove(link_node *interval_node,bool b_release = true){
		link_node *prev = interval_node->prev, *next = interval_node->next;

		if (!prev)_head_root = next;
		else prev->next = next;

		if (!next) _tail_root = prev;
		else next->prev = prev;

		--_node_size;
		if(b_release)
			release(interval_node);
		return next;
	};

	link_node *apply(const KT &kt_value) {
		link_node *node = nullptr;
		if (_mem_alloc) {
			bool b_recycle = false;
			node = _mem_alloc->alloc(kt_value,b_recycle);
			if (b_recycle) {
				node->next = node->prev  = nullptr;
				node->tv_value = kt_value;
			}
			return node;
		}

		if (_cache_size)
		{
			node = _cache;
			_cache = _cache->next;
			node->tv_value = kt_value;
			node->prev = node->next = nullptr;
			--_cache_size;
		}
		else
			node = new link_node(kt_value);
		return node;
	};
	void release(link_node *interval_node) {
		if (_mem_alloc) {
			_mem_alloc->release(interval_node);
			return;
		}
		if (_cache_size < _cache_capacity)
		{
			interval_node->next = _cache;
			_cache = interval_node;
			++_cache_size;
		}
		else
			delete interval_node;
	};

	void clear() {
		//直接回收所有的节点单元
		if (_head_root)
		{
			_tail_root->next = _cache;
			_cache = _head_root;

			_cache_size += _node_size;
			_node_size = 0;
			_head_root = _tail_root = nullptr;
		}
	};
};
/*
  *需要提供一个宏,根据值的偏移量,来计算装载值的节点的地址
  *maturity
 */
#define link_node_addr(T,kts_value) ((link_list<T>::link_node_t *)((char *)&kts_value - (char *)&((link_list<T>::link_node_t*)nullptr)->kt_value))

NS_GT_END
#endif