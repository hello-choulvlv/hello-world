/*
  *离散点集的Voronoi图数据结构
  *2020年6月9日
  *@author:xiaohuaxiong
 */
#include "fk_diagram.h"

NS_GT_BEGIN

FkDiagram::FkDiagram(const std::vector<cocos2d::Vec2> &locations) {
	_sitesVec.reserve(locations.size());
	for (int j = 0; j < locations.size(); ++j) 
		_sitesVec.push_back(FkSite(locations[j]));
}

FkDiagram::~FkDiagram() {
	for (auto *it_ptr = _edgesList.head(); it_ptr != nullptr; it_ptr = it_ptr->next) {
		delete it_ptr->tv_value;
	}
}

FxEdge *FkDiagram::createHalfEdge(FkSite *site_ptr) {
	FxEdge  *half_edge = new FxEdge(site_ptr);
	if (!site_ptr->head_ptr)
		site_ptr->head_ptr = half_edge;
	_edgesList.push_back(half_edge);

	return half_edge;
}

cocos2d::Vec2  *FkDiagram::createVertex(const cocos2d::Vec2 &point) {
	_vertexList.push_back(point);
	return &_vertexList.back()->tv_value;
}

NS_GT_END