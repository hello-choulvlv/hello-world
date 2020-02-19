/*
  *���ȼ�����
  *2019��11��5��
  *@author:xiaohuaxiong
  *@version:2.0
  *@data:2020��2��17��֧������λ�õ�ɾ��,ֻ����Ҫ����һЩ��������
  *ע��modify_function,��������ֵ��-1,��˵����Ҫ���ظýڵ��ʵ��λ��,����Ϊʵ�ʵ�λ��
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
	//�Ƿ�����ڴ�,����˱�־�򿪵Ļ�,���ڴ��һֱ����,����ᶯ̬�ı�
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
	//�����ѽṹ
	void adjust_heap(std::function<bool(const KP &a, const KP &b)> &compare_func, std::function<int(KP &target, int index_new)> &modify_func) {
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
				bool b = modify_func ? modify_func(_root_heap[branch_l],branch_l):false;
				branch_l = parent_index;
				parent_index >>= 1;
			}
			if(branch_l != child_index)
				_root_heap[branch_l] = elem;
			bool b = modify_func ? modify_func(_root_heap[branch_l],branch_l):false;
		}
	};
	//�����ȼ����в���Ԫ��
	void insert(const KP &elem, std::function<bool(const KP &a, const KP &b)> &compare_func, std::function<int(KP &target, int index_new)> &modify_func) {
		//���ȼ���Ƿ������㹻
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
	//���ʵ�һ��Ԫ��
	const KP &head() {
		assert(_heap_size);
		return _root_heap[1];
	};
	//ɾ����һ��Ԫ��
	void remove_head(std::function<bool(const KP &a, const KP &b)> &compare_func, std::function<int(KP &target, int index_new)> &modify_func) {
		assert(_heap_size);
		//ʹ�����һ��Ԫ�����,ɾ���Ĳ����Ǵ��ϵ���
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
		//����ﵽ���ٽ�����,����Ҫ�����ڴ��
		shrunk();
	};
	//ɾ������һ��Ԫ��,��ʱ��ҪһЩ��������
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
	//�����ڴ�
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
	//�����ڴ�
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