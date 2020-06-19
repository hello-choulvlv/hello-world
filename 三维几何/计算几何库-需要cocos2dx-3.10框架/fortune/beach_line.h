/*
  *Voronoiͼ֮Fortune��̲���㷨ʵ��
  *2020��6��8��
  *beach_line��һ�����ݽṹ,����Ϊ�����ں����,������ʹ�õĹ�����,���Ƕ�������˲��ֵ��Ż�
 */
#ifndef __beach_line_h__
#define __Beach_line_h__

#include "fk_data.h"
#include<assert.h>
#include<functional>

NS_GT_BEGIN
#define fx_node_color(node) ((!node)?FxColorType_Black:(node)->color_type)
class FxBeachLine {
private:
	struct FxArc  *_root, *_cache_head;
	int       _node_size, _cache_size, _cache_capacity;
public:
	FxBeachLine(int capacity_size = 0x7FFFFFFF) : _root(nullptr), _cache_head(nullptr), _node_size(0), _cache_size(0), _cache_capacity(capacity_size) {
	};
	~FxBeachLine();
	void clear();
	int size()const { return _node_size; };
	FxArc *find_next(FxArc  *node);

	FxArc *find_prev(FxArc  *node);
	FxArc *find_mostleft();
	FxArc* lookup(const cocos2d::Vec2 &point,std::function<float(const FxArc *, const FxArc *)> &compute_func);
	/*
	*����ڵ�֮�������
	*/
	void insert_fixup(FxArc *inserted_node);
	void insert_before(FxArc *target_node,FxArc *inserted_node);
	void insert_after(FxArc *target_node, FxArc *inserted_node);
	void insert_root(FxArc  *inserted_node);

	void replace_node_case(FxArc *old_node, FxArc *new_node);
	void replace(FxArc *old_node, FxArc *new_node);

	FxArc *remove(FxArc *search_node);
	void remove_fixup(FxArc *parent_node, FxArc *replace_node);

	FxArc* find_mostright(FxArc* n);

	void rotate_left(FxArc* n);
	void rotate_right(FxArc* n);
	//�ڴ����
	FxArc   *alloc(FkSite *site);
	void		release(FxArc *tw_node);
	//debug����
	void    debug_node_color(FxArc  *target_node);
	void    debug_node(std::function<float(const FxArc *,const FxArc *)> &compute_func);
	void    debug_node();
	void    check_cascade(FxArc  *check_node);
};
NS_GT_END
#endif