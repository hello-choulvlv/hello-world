/*
  *���ȼ�����
  *2019��11��5��
  *@author:xiaohuaxiong
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
public:
	priority_queue(int heap_size): _heap_size(0),_heap_capacity(heap_size){
		_root_heap = new KP[heap_size + 1];
	};
	priority_queue(KP *elem_array, int array_size, std::function<bool(const KP &a, const KP &b)> &compare_func): _heap_size(array_size),_heap_capacity(array_size){
		_root_heap = new KP[array_size +1];
		memcpy(_root_heap+1,elem_array,sizeof(KP) * array_size);
		adjust_heap(compare_func);
	};
	~priority_queue() {
		delete[] _root_heap;
		_root_heap = nullptr;
	};
	//�����ѽṹ
	void adjust_heap(std::function<bool(const KP &a, const KP &b)> &compare_func) {
		//�������·���
		for (int child_index = 2; child_index <= _heap_size; ++child_index)
		{
			int parent_index = child_index >> 1;
			const KP elem = _root_heap[child_index];
			//�����ϲ��Ѿ��ź�����
			int branch_l = child_index;
			while (parent_index && compare_func(elem, _root_heap[parent_index]))
			{
				_root_heap[branch_l] = _root_heap[parent_index];
				branch_l = parent_index;
				parent_index >>= 1;
			}
			if(branch_l != child_index)
				_root_heap[branch_l] = elem;
		}
	};
	//�����ȼ��������ղ���Ԫ��
	void insert(const KP &elem, std::function<bool(const KP &a, const KP &b)> &compare_func) {
		//���ȼ���Ƿ������㹻
		if (_heap_size >= _heap_capacity)
		{
			_heap_capacity = _heap_size << 1;
			KP  *extend_array = new KP[_heap_capacity + 1];
			memcpy(extend_array +1,_root_heap + 1,sizeof(KP) * _heap_size);
			delete[] _root_heap;
			_root_heap = extend_array;
		}
		int child_index = ++_heap_size;
		int parent_index = child_index >> 1;
		while (parent_index && compare_func(elem, _root_heap[parent_index]))
		{
			_root_heap[branch_l] = _root_heap[parent_index];
			child_index = parent_index;
			parent_index >>= 1;
		}
		_root_heap[child_index] = elem;
	};
	//���ʵ�һ��Ԫ��
	const KP &head() {
		assert(_heap_size);
		return _root_heap[1];
	};
	//ɾ����һ��Ԫ��
	void remove_head(std::function<bool(const KP &a, const KP &b)> &compare_func) {
		//ʹ�����һ��Ԫ�����
		int parent_l = 1;
		int child_l = 2;
		for (; child_l <= _heap_size; parent_l = child_l,child_l >>=1)
		{
			if (child_l + 1 <= _heap_size && !compare_func(_root_heap[child_l], _root_heap[child_l + 1]))
				++child_l;
			_root_heap[parent_l] = _root_heap[child_l];
		}
		if (parent_l != _heap_size)
			_root_heap[parent_l] = _root_heap[_heap_size];
		//����ﵽ���ٽ�����,����Ҫ�����ڴ��
		if (--_heap_size >= 8 && _heap_size <= _heap_capacity >> 2)
		{
			_heap_capacity = _heap_size << 1;
			KP *extent_array = new KP[_heap_capacity +1];
			memccpy(extend_array,_root_heap,sizeof(KP) * _heap_size);
			delete[] _root_heap;
			_root_heap = extent_array;
		}
	};
	const int size()const { return _heap_size; };
	const int capacity()const { return _heap_capacity; };
};
NS_GT_END