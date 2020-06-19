/*
  *Voronoiͼ֮Fortune��̲���㷨ʵ��
  *2020��6��8��
  *���е����ݽṹ����ض���
 */
#ifndef __fk_data_h__
#define __fk_data_h__
#include "gt_common/geometry_types.h"
#include "math/Vec2.h"
NS_GT_BEGIN
struct FkSite;
struct FkEvent;
//���
struct FxEdge{
	cocos2d::Vec2   *origin_ptr, *destination_ptr;
	struct FkSite		*site_ptr;
	struct FxEdge	*prev,*next,*twin;

	explicit FxEdge(FkSite *site,cocos2d::Vec2 *a_start = nullptr,cocos2d::Vec2 *b_over=nullptr):origin_ptr(a_start),destination_ptr(b_over),site_ptr(site),prev(nullptr),next(nullptr),twin(nullptr){};
};

struct FkSite {
	cocos2d::Vec2	location;
	FxEdge              *head_ptr, *tail_ptr;

	explicit FkSite(const cocos2d::Vec2 &point) :location(point),head_ptr(nullptr),tail_ptr(nullptr){};
};

enum FxColorType {
	FxColorType_Red = 0,//��
	FxColorType_Black = 1,//��
};

struct FxArc {
	struct FkSite   *site_ptr;
	struct FkEvent *event_ptr;
	struct FxEdge *left_edge, *right_edge;

	FxColorType  color_type;
	int                     debug_bits;
	struct FxArc *l_child, *r_child, *parent, *prev, *next;

	explicit FxArc(FkSite  *site,FkEvent *event):site_ptr(site),event_ptr(event),left_edge(nullptr),right_edge(nullptr), color_type(FxColorType_Red), debug_bits(0),l_child(nullptr),r_child(nullptr), parent(nullptr),prev(nullptr),next(nullptr) {};
};

enum FkEventType {
	FkEventType_Site = 0,//�����¼�
	FkEventType_Circle = 1,//Բ�¼�
};

struct FkEvent {
	FkEventType   event_type;
	//��׼��,�����ݽ�����Բ�¼���ʹ��,�ڲ�ͬ���¼�������,���������ָ���ĺ��岢����ͬ
	float                   base_y;
	cocos2d::Vec2 location;

	struct FxArc    *arc_ptr;
	struct FkSite    *site_ptr;
	int                      queue_idx;//�����ݽ��������ȼ������б�ʹ�õ�

	explicit FkEvent(FkSite *site):event_type(FkEventType_Site),base_y(site->location.y), arc_ptr(nullptr),site_ptr(site),queue_idx(-1) {};
	explicit FkEvent(float base, const cocos2d::Vec2 &point,FxArc *arc) :base_y(base),location(point),arc_ptr(arc), site_ptr(nullptr), queue_idx(-1){};
};

NS_GT_END
#endif