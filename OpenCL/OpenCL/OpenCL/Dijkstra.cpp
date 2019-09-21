/*
  *�Ͻ�˹�����㷨ʵ��
  *2019��2��21��
  *@author:xiaohuaxiong
 */
#include "csk_types.h"
#include "CSKContext.h"
#include<stdlib.h>
//ͼ���ܵ���Ŀ
//ͼ�бߵ���Ŀ
static const int  s_vertexCount = 100;
static const int  s_adjentEdge = 50;
/*
  *ͼ���ݽṹ
 */
struct Graphics
{
	int    *vertexAdjent;
	int    *vertexEdge;
	float *weight;
	int     vertexNumber;//�������Ŀ
	int     edgeNumber;//�ߵ���Ŀ
	Graphics();
	~Graphics();
	//�������ͼ
	void  generateRandomGraph(int   vertexCount);
};

Graphics::Graphics():
	vertexAdjent(nullptr)
	,vertexEdge(nullptr)
	,weight(nullptr)
	,vertexNumber(0)
	,edgeNumber(0)
{
}

Graphics::~Graphics()
{
	delete vertexAdjent;
	delete vertexEdge;
	delete weight;

	vertexEdge = nullptr;
	vertexAdjent = nullptr;
	weight = nullptr;
	vertexNumber = 0;
	edgeNumber = 0;
}

void Graphics::generateRandomGraph(int vertexCount)
{
	int  base_index = 0;
	int  total_edge_count = vertexCount * s_adjentEdge;
	vertexAdjent = new int[vertexCount];
	vertexEdge = new int[total_edge_count];
	weight = new float[total_edge_count];

	vertexNumber = vertexCount;
	edgeNumber = total_edge_count;

	for(int index_j = 0; index_j < vertexCount; ++index_j)
	{
		vertexAdjent[index_j] = s_adjentEdge * index_j;
	}

	for (int index_j = 0; index_j < total_edge_count; ++index_j)
	{
		vertexEdge[index_j] = rand() % vertexCount;
		weight[index_j] = 100.0f * rand() / RAND_MAX;
	}
}

int  main(int argc,char *argv[])
{
	CSKContext   vk_context;
	vk_context.init();


	return 0;
}