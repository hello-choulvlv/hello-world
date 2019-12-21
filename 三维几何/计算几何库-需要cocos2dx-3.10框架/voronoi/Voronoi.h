/*
  *Voronoiͼ�㷨+ƽ��㼯�������ʷ��㷨
  *��Ϊ���㷨ϵ�бȽ��Ӵ���Ҹ���,�������Ŷ��ֵ���ȫ��ͬ��˼·,����֮���������еĹ�ϵ
  *�����Ҫ������ȡ�����γ�һ���ļ�.
  *2019��12��12��
  *@author:xiaoxiong
*/
#include "gt_common/geometry_types.h"
#include "math/Vec2.h"
#include "math/CCGeometry.h"
#include<set>

NS_GT_BEGIN
struct Cycle;
/*
  *�����ζ�������
 */
struct DelaunayTriangle
{
	short v1, v2, v3;
};

bool operator==(const DelaunayTriangle &a, const DelaunayTriangle &other);
//bool operator!=(const DelaunayTriangle &a, const DelaunayTriangle &other);

struct TwinTriangle
{
	DelaunayTriangle *left_triangle, *right_triangle;
};
//��
struct DelaunayEdge
{
	short v1, v2;
};
bool operator ==(const DelaunayEdge &, const DelaunayEdge &other);
bool operator >(const DelaunayEdge &, const DelaunayEdge &);
bool operator < (const DelaunayEdge &, const DelaunayEdge &);
/*
  *Delaunay�����ο��ٲ��ҽṹ
 */
struct DelaunayNode
{
	short v1, v2, v3,ref;
	DelaunayNode *lchild, *mchild, *rchild;

	DelaunayNode(const DelaunayTriangle &triangle,int ref_count = 0):v1(triangle.v1),v2(triangle.v2),v3(triangle.v3),ref(ref_count),lchild(nullptr),mchild(nullptr),rchild(nullptr) {};
};
bool operator==(const DelaunayNode &a, const DelaunayNode &other);
bool operator==(const DelaunayNode &a, const DelaunayTriangle &other);
//bool operator!=(const DelaunayNode &a, const DelaunayNode &other);

//˫��
struct TwinNode
{
	DelaunayNode *left_node, *right_node;
};

struct DelaunaySearch
{
	const std::vector<cocos2d::Vec2>  &disper_points;
	DelaunayNode   root;
	int node_size;

	DelaunaySearch(const std::vector<cocos2d::Vec2> &adisper_points, DelaunayTriangle &init_triangle) :disper_points(adisper_points), root(init_triangle), node_size(1){ root.ref = 2; };
	/*
	  *�������������в���һ������
	  *��ʱ�����������εķ���
	 */
	DelaunayNode   *insert(int point_index);
	/*
	  *��ѯ���������ڵĽڵ�
	 */
	DelaunayNode   *lookup(int point_index);
	/*
	  *�������ڵ������κϲ�,ע��ϲ��Ĺ����У������������䵽���Ƿ�����ڽ�,�������Ҫʹ�����Լ���֤
	  *target�ǲ��ҹ��������ɵ�,left/right�����ⲿ���ɵ�
	 */
	void  merge(DelaunayNode *target,DelaunayNode *left,DelaunayNode *right);
	/*
	  *�����ڵ�
	 */
	void visit(std::set<DelaunayNode*> &,bool visit_leaf);
	/*
	  *destroy
	 */
	void destroy(std::set<DelaunayNode *> &);
	/*
	  *����
	 */
	~DelaunaySearch();
};

void static_create_cycle_by_triangle(Cycle &cycle, const std::vector<cocos2d::Vec2> &disper_points, const DelaunayTriangle &delaunay_triangle);
/*
  *��������ε���С���������
  *��������Ϊһ����������
 */
void rect_outerline_triangle(const cocos2d::Vec2 &origin,const cocos2d::Vec2 &extent,cocos2d::Vec2 triangle[3]);
/*
  *������ɢ�㼯��Delaunay�����ʷ�
  *ʹ��Bowyer-Watson�㷨
  *�㷨���������Ѿ�������صļ���
  *����ӳ��������εĶ�������λ���������
  *������Ч��������ݴ���3��
*/
void delaunay_triangulate_bowyer_washton(const std::vector<cocos2d::Vec2> &disper_points,std::vector<DelaunayTriangle> &triangle_sequence,int &real_size);
/*
  *������ɢ�㼯�������ʷ�
  *ʹ������㷨
  *����������صļ�������
  *�ڸú����ĵ�һ����,����û�ж��㷨�Ĺؼ����ֽ����Ż�
  *�ڵڶ�����,��ô����ʹ��ĳЩ���ӵ����ݽṹ���Ż������������е�Ŀ��ٶ�λ�㷨����
  */
void delaunay_triangulate_random(const std::vector<cocos2d::Vec2> &disper_points, std::vector<DelaunayTriangle> &triangle_sequence, int &real_size);
NS_GT_END