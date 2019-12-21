/*
  *Voronoi图算法+平面点集的三角剖分算法
  *因为该算法系列比较庞大而且复杂,并且有着多种的完全不同的思路,两者之间有着密切的关系
  *因此需要单独抽取出来形成一个文件.
  *2019年12月12日
  *@author:xiaoxiong
*/
#include "gt_common/geometry_types.h"
#include "math/Vec2.h"
#include "math/CCGeometry.h"
#include<set>

NS_GT_BEGIN
struct Cycle;
/*
  *三角形顶点序列
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
//边
struct DelaunayEdge
{
	short v1, v2;
};
bool operator ==(const DelaunayEdge &, const DelaunayEdge &other);
bool operator >(const DelaunayEdge &, const DelaunayEdge &);
bool operator < (const DelaunayEdge &, const DelaunayEdge &);
/*
  *Delaunay三角形快速查找结构
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

//双边
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
	  *向三角形序列中插入一个顶点
	  *此时会引起三角形的分裂
	 */
	DelaunayNode   *insert(int point_index);
	/*
	  *查询三角形所在的节点
	 */
	DelaunayNode   *lookup(int point_index);
	/*
	  *两个相邻的三角形合并,注意合并的过程中，函数不会检查其到底是否真的邻接,在这个需要使用者自己保证
	  *target是查找过程中生成的,left/right是在外部生成的
	 */
	void  merge(DelaunayNode *target,DelaunayNode *left,DelaunayNode *right);
	/*
	  *遍历节点
	 */
	void visit(std::set<DelaunayNode*> &,bool visit_leaf);
	/*
	  *destroy
	 */
	void destroy(std::set<DelaunayNode *> &);
	/*
	  *销毁
	 */
	~DelaunaySearch();
};

void static_create_cycle_by_triangle(Cycle &cycle, const std::vector<cocos2d::Vec2> &disper_points, const DelaunayTriangle &delaunay_triangle);
/*
  *求给定矩形的最小外接三角形
  *此三角形为一个正三角形
 */
void rect_outerline_triangle(const cocos2d::Vec2 &origin,const cocos2d::Vec2 &extent,cocos2d::Vec2 triangle[3]);
/*
  *计算离散点集的Delaunay三角剖分
  *使用Bowyer-Watson算法
  *算法对于输入已经做了相关的假设
  *其外接超级三角形的顶点坐标位于最后三个
  *且其有效顶点的数据大于3个
*/
void delaunay_triangulate_bowyer_washton(const std::vector<cocos2d::Vec2> &disper_points,std::vector<DelaunayTriangle> &triangle_sequence,int &real_size);
/*
  *计算离散点集的三角剖分
  *使用随机算法
  *其与输入相关的假设如上
  *在该函数的第一版中,我们没有对算法的关键部分进行优化
  *在第二版中,我么将会使用某些复杂的数据结构来优化三角形序列中点的快速定位算法步骤
  */
void delaunay_triangulate_random(const std::vector<cocos2d::Vec2> &disper_points, std::vector<DelaunayTriangle> &triangle_sequence, int &real_size);
NS_GT_END