/*
  *�㶨λ�㷨ʵ��
  *2020��4��13��
  *@author:xiaohuaxiong
 */
#ifndef __point_location_h__
#define __point_location_h__
#include "math/Vec2.h"
#include "line/line.h"
#include "gt_common/geometry_types.h"
#include <vector>

NS_GT_BEGIN
//�ڵ������
enum LocalType {
	LocalType_Endpoint = 0,//�˵�
	LocalType_Segment = 1,//�߶�
	LocalType_Trapzoid = 2,//����
};
struct NodeLocal;
//����
struct Trapzoid {
	cocos2d::Vec2   left_point, right_point;//���Ҷ˵�
	Segment2D       *up_seg_ptr, *low_seg_ptr;//���߶�,���߶�
	NodeLocal	        *left_up, *left_low, *right_up, *right_low;//����/����/����/���������ھ�,ע��left_up,left_low/right_up,right_low��ֵ�������

	Trapzoid(const cocos2d::Vec2 &aleft,const cocos2d::Vec2 &aright) :left_point(aleft),right_point(aright)
		,up_seg_ptr(nullptr)
		,low_seg_ptr(nullptr)
		,left_up(nullptr)
		,left_low(nullptr)
		,right_up(nullptr)
		,right_low(nullptr){};
};

struct NodeLocal {
	LocalType  node_type;
	cocos2d::Vec2   *endpoint_ptr;//�˵�
	Segment2D         *segment_ptr;//�߶�
	Trapzoid              *trap_ptr;//����
	NodeLocal  *child_l, *child_r;
	int                           ref;//���ʱ�־,����ؽڵ㱻����ʱʹ��

	NodeLocal(LocalType  atype) :node_type(atype)
		,endpoint_ptr(nullptr)
		,segment_ptr(nullptr)
		,trap_ptr(nullptr)
		,child_l(nullptr)
		,child_r(nullptr)
		, ref(0){};
};

struct LocationLexer {
	NodeLocal *root;
	int                  size;

	LocationLexer() :root(nullptr), size(0) {};
	~LocationLexer();
};
/*
  *����
 */
void local_point_visit(LocationLexer &lexer, std::vector<NodeLocal *> &node_array, short ref_result);
/*
*������㷨ʵ��
*/
NodeLocal*   local_point_find_location(LocationLexer &lexer, const cocos2d::Vec2 &point);
/*
  *��������ͼ
  *�������:��ʼ����ͼ���ݽṹ,���������߶α�ʾ
  *��ʼ�߽�,���㹻�������е��߶ζ˵�,�߽�����з�ʽλ���߶�,���߶�,���Ӧ��һ��ƽ��������ľ��ε����ϱ߽�
  *������߶α�ʾ��,start_point.x > final_point.xʼ�ձ�����ֵ,�����㷨�����ز�������
 */
void local_point_create_trapzoid(LocationLexer &lexer,std::vector<Segment2D> &segments);

NS_GT_END
#endif