/*
  *Voronoi图数据结构
  *2020年6月9日
  *@author:xiaohuaxiong
 */
#ifndef __fk_diagram_h__
#define __fk_diagram_h__
#include "gt_common/geometry_types.h"
#include "data_struct/link_list.h"
#include "fk_data.h"
#include <vector>

NS_GT_BEGIN
class FkDiagram {
	std::vector<FkSite>				_sitesVec;
	link_list<cocos2d::Vec2>		_vertexList;
	link_list<FxEdge*>               _edgesList;
public:
	FkDiagram(const std::vector<cocos2d::Vec2> &locations);
	~FkDiagram();

	FxEdge  *createHalfEdge(FkSite *site_ptr);
	cocos2d::Vec2   *createVertex(const cocos2d::Vec2 &point);

	FkSite  *getSite(int j) { return &_sitesVec[j]; };
	link_list<FxEdge *>&  getHalfEdges() { return _edgesList; };
};
NS_GT_END
#endif