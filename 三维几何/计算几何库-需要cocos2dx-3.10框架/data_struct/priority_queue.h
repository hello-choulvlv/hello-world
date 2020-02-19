/*
  *优先级队列
  *2019年11月5日
  *@author:xiaohuaxiong
  *@version:2.0
  *@data:2020年2月17日支持任意位置的删除,只是需要增加一些限制条件
  *注意modify_function,如果传入的值是-1,则说明需要返回该节点的实际位置,否则为实际的位置
 */
#include "gt_common/geometry_types.h"
#include<string.h>
#include<assert.h>
NS_GT_BEGIN
template<typename KP>
class priority_queue
{
	KP    *_root_heap;
	int      _heap_size;
	int      _heap_capacity;
	//是否拘留内存,如果此标志打开的话,则内存会一直递增,否则会动态改变
	bool   _retain;
public:
	priority_queue(int heap_size,bool retain_mem): _heap_size(0),_heap_capacity(heap_size), _retain(retain_mem){
		_root_heap = new KP[heap_size + 1];
	};
	priority_queue(KP *elem_array, int array_size, std::function<bool(const KP &a, const KP &b)> &compare_func, std::function<int(KP &target,int index_new)> &modify_func,bool retain_mem) : 
		_heap_size(array_size), 
		_heap_capacity(array_size),
		_retain(retain_mem){
		_root_heap = new KP[array_size +1];
		memcpy(_root_heap+1,elem_array,sizeof(KP) * array_size);
		adjust_heap(compare_func,modify_func);
	};
	~priority_queue() {
		delete[] _root_heap;
		_root_heap = nullptr;
	};
	//调整堆结构
	void adjust_heap(std::function<bool(const KP &a, const KP &b)> &compare_func, std::function<int(KP &target, int index_new)> &modify_func) {
		//从上向下分析
		for (int child_index = 2; child_index <= _heap_size; ++child_index)
		{
			int parent_index = child_index >> 1;
			const KP elem = _root_heap[child_index];
			//假设上层已经排好序了
			int branch_l = child_index;
			while (parent_index && compare_func(elem, _root_heap[parent_index]))
			{
				_root_heap[branch_l] = _root_heap[parent_index];
				bool b = modify_func ? modify_func(_root_heap[branch_l],branch_l):false;
				branch_l = parent_index;
				parent_index >>= 1;
			}
			if(branch_l != child_index)
				_root_heap[branch_l] = elem;
			bool b = modify_func ? modify_func(_root_heap[branch_l],branch_l):false;
		}
	};
	//向优先级队列插入元素
	void insert(const KP &elem, std::function<bool(const KP &a, const KP &b)> &compare_func, std::function<int(KP &target, int index_new)> &modify_func) {
		//首先检查是否容量足够
		expand();
		int child_index = ++_heap_size;
		int parent_index = child_index >> 1;
		while (parent_index && compare_func(elem, _root_heap[parent_index]))
		{
			_root_heap[child_index] = _root_heap[parent_index];
			bool b = modify_func ? modify_func(_root_heap[child_index], child_index):false;
			child_index = parent_index;
			parent_index >>= 1;
		}
		_root_heap[child_index] = elem;
		bool b = modify_func ? modify_func(_root_heap[child_index], child_index) : false;
	};
	//访问第一个元素
	const KP &head() {
		assert(_heap_size);
		return _root_heap[1];
	};
	//删除第一个元素
	void remove_head(std::function<bool(const KP &a, const KP &b)> &compare_func, std::function<int(KP &target, int index_new)> &modify_func) {
		assert(_heap_size);
		//使用最后一个元素填充,删除的步骤是从上到下
		int parent_l = 1;
		int child_l = 2; 
		KP &ele = _root_heap[_heap_size];
		--_heap_size;
		for (; child_l <= _heap_size; parent_l = child_l,child_l <<=1)
		{
			if (child_l < _heap_size && !compare_func(_root_heap[child_l], _root_heap[child_l + 1]))
				++child_l;
			if (compare_func(_root_heap[child_l], ele))
			{
				_root_heap[parent_l] = _root_heap[child_l];
				bool b = modify_func ? modify_func(_root_heap[parent_l], parent_l) : false;
			}
			else
				break;
		}
		if (_heap_size)
		{
			_root_heap[parent_l] = ele;
			bool b = modify_func ? modify_func(_root_heap[parent_l], parent_l) : false;
		}
		//如果达到了临界条件,是需要伸缩内存的
		shrunk();
	};
	//删除任意一个元素,此时需要一些条件限制
	void remove(KP &target,std::function<bool(const KP &a, const KP &b)> &compare_func, std::function<int(KP &target, int index_new)> &modify_func)
	{
		int parent_l = modify_func(target, -1);
		int child_l = parent_l << 1;
		KP &elem = _root_heap[_heap_size];
		--_heap_size;

		for (; child_l <= _heap_size; child_l <<= 1)
		{
			if (child_l < _heap_size && !compare_func(_root_heap[child_l], _root_heap[child_l + 1]))
				++child_l;
			if (compare_func(_root_heap[child_l], elem))
			{
				_root_heap[parent_l] = _root_heap[child_l];
				bool b = modify_func(_root_heap[parent_l], parent_l);
			}
			else
				break;
			parent_l = child_l;
		}
		if (_heap_size)
		{
			_root_heap[parent_l] = elem;
			bool b = modify_func(_root_heap[parent_l], parent_l);
		}

		shrunk();
	};
	//收缩内存
	void shrunk()
	{
		if (!_retain && _heap_size > 16 && _heap_size <= _heap_capacity >> 2)
		{
			_heap_capacity >>= 1;
			KP *extent_array = new KP[_heap_capacity + 1];
			memcpy(extent_array+1, _root_heap+1, sizeof(KP) * _heap_size);
			delete[] _root_heap;
			_root_heap = extent_array;
		}
	};
	//扩张内存
	void expand()
	{
		if (_heap_size >= _heap_capacity)
		{
			_heap_capacity = _heap_size << 1;
			KP  *extend_array = new KP[_heap_capacity + 1];
			memcpy(extend_array + 1, _root_heap + 1, sizeof(KP) * _heap_size);
			delete[] _root_heap;
			_root_heap = extend_array;
		}
	};
	const int size()const { return _heap_size; };
	const int capacity()const { return _heap_capacity; };
	void set_mem_retain(bool b) { _retain = b; };
	bool mem_retain() {return _retain;};

	KP*   data()const { return _root_heap +1; };
};
NS_GT_END