/*
  *��ɢ�㼯��Voronoiͼ֮��̲���㷨ʵ��
  *2020��6��9��
  *@author:xiaohuaxiong
 */
#ifndef __fk_algorithm_h__
#define __fk_algorithm_h__
#include "gt_common/geometry_types.h"
#include "data_struct/priority_queue.h"
#include "fk_data.h"
#include "fk_diagram.h"
#include "beach_line.h"
#include "math/Vec2.h"
#include <functional>

NS_GT_BEGIN

class FortuneAlgorithm {
	FkDiagram    _diagram;

	std::function<float(const FxArc *,const FxArc *)> _compute_func;
	std::function<bool(FkEvent *const&, FkEvent *const &)>         _compare_func;
	std::function<int(FkEvent *&, int)>   _modify_func;

	//�¼�����
	priority_queue<FkEvent*>   _event_queue;
	FxBeachLine                             _beach_line;
	//��̲�ߵĻ�׼��
	float                _beachLineY;
public:
	FortuneAlgorithm(const std::vector<cocos2d::Vec2> &locations);
	~FortuneAlgorithm() = default;

	void  build();
	void  bound();//�趨�߽�
	void  handleSiteEvent(FkEvent  *event_ptr);
	void  handleCircleEvent(FkEvent *event_ptr);

	//���������߻�
	FxArc*  breakArc(FxArc  *middle_arc, FkSite  *target_site);
	//ɾ���뻡��������¼�
	void  removeEvent(FxArc  *target_arc);
	//ɾ��������Ļ�
	void removeArc(FkEvent  *event_ptr);
	//���������ڵĻ�֮�����ӹ�����
	void addEdge(FxArc *left_edge,FxArc *right_edge);
	//����¼�,���¼���ָԲ�¼�
	void addEvent(FxArc *left_arc,FxArc *middle_arc, FxArc *right_arc);
	//
	void setDestination(FxArc *left_arc, FxArc *right_arc,cocos2d::Vec2 *location);
	void setOrigin(FxArc *left_arc, FxArc *right_arc,cocos2d::Vec2 *location);
	void setPrevEdge(FxEdge *prev, FxEdge *next);

	FkDiagram &getDiagram() { return _diagram; };
};

NS_GT_END
#endif