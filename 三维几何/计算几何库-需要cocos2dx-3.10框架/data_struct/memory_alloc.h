/*
  *平衡树内存管理器,注意该管理器主要针对小块的内存,如果是
  *像数组这样的数据结构,该内存管理器则不会提供任何的支持
  *2020年2月19日
 */
#ifndef __memory_alloc_h__
#define __memory_alloc_h__
#include "gt_common/geometry_types.h"
#include<string.h>
#include<assert.h>
NS_GT_BEGIN
#define __check_repeat_node 0
template<typename MC,typename TV>
class memory_alloc
{
	MC  **_alloc_array;
	int       _alloc_size;
	int       _alloc_capacity;
	bool    _retain;
public:
	memory_alloc(int alloc_capacity = 256,bool retain = true) :_alloc_array(new MC*[alloc_capacity]),_alloc_size(0),_alloc_capacity(alloc_capacity),_retain(retain) {};
	~memory_alloc() {
		for (int j = 0; j < _alloc_size; ++j)
			delete _alloc_array[j];
		delete[] _alloc_array;

		_alloc_array = nullptr;
		_alloc_array = 0;
		_alloc_capacity = 0;
	};

	MC *alloc(const TV &tv_value,bool &b_recycle) {
		MC  *node = nullptr;
		if (_alloc_size)
		{
			node = _alloc_array[--_alloc_size];
			//检测是否需要收缩内存
			if (!_retain && _alloc_size > 16 && _alloc_size < _alloc_capacity >> 2)
			{
				_alloc_capacity >>= 1;
				MC **alloc_new = new MC*[_alloc_capacity];
				memcpy(alloc_new,_alloc_array,sizeof(MC *) * _alloc_size);

				delete[] _alloc_array;
				_alloc_array = alloc_new;
			}
			b_recycle = true;
			//assert(!node->in_use);
			//node->in_use = true;
		}
		else
		{
			node = new MC(tv_value);
			b_recycle = false;
		}
		return node;
	};

	void release(MC *node)
	{
		if (_alloc_size >= _alloc_capacity)
		{
			_alloc_capacity <<= 1;
			MC **extent_array = new MC*[_alloc_capacity];
			memcpy(extent_array,_alloc_array,sizeof(MC*) * _alloc_size);

			delete[] _alloc_array;
			_alloc_array = extent_array;
		}
#if __check_repeat_node
		for (int j = 0; j < _alloc_size; ++j)
			assert(_alloc_array[j] != node);
#endif
		//assert(node->in_use);
		//node->in_use = false;
		_alloc_array[_alloc_size++] = node;
	}
};

NS_GT_END
#endif