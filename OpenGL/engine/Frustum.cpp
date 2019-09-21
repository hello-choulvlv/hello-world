/*
  *ƽ��ͷ��ʵ��
  *@date:2017-5-20
  *@Author:xiaohuaxiong
*/
#include"engine/Frustum.h"
__NS_GLK_BEGIN
//�Ӿ�������ȡƽ�淽�̿��Բο���������
//http://www.lighthouse3d.com/tutorials/view-frustum-culling/clip-space-approach-extracting-the-planes/

Frustum::Frustum(const Matrix & viewProjMatrix)
{
	this->init(viewProjMatrix);
}

Frustum::Frustum()
{

}

void Frustum::init(const Matrix &viewProjMatrix)
{
	const float  (*array)[4] = viewProjMatrix.m;
	//+X
	_planes[0].init(GLVector3(-array[0][0] - array[0][3], -array[1][0] - array[1][3], -array[2][0] - array[2][3]), array[3][0] + array[3][3]);
	//-X
	_planes[1].init(GLVector3(array[0][0] - array[0][3], array[1][0] - array[1][3], array[2][0] - array[2][3]), array[3][3] - array[3][0]);
	//+Y
	_planes[2].init(GLVector3(-array[0][1] - array[0][3], -array[1][1] - array[1][3], -array[2][1] - array[2][3]), array[3][1] + array[3][3]);
	//-Y
	_planes[3].init(GLVector3(array[0][1] - array[0][3], array[1][1] - array[1][3], array[2][1] - array[2][3]), array[3][3] - array[3][1]);
	//+Z
	_planes[4].init(GLVector3(-array[0][2] - array[0][3], -array[1][2] - array[1][3], -array[2][2] - array[2][3]), array[3][2] + array[3][3]);
	//-Z
	_planes[5].init(GLVector3(array[0][2] - array[0][3], array[1][2] - array[1][3], array[2][2] - array[2][3]), array[3][3] - array[3][2]);
}

void Frustum::init(float lndcX, float rndcX, float bndcY, float tndcY, const Matrix &viewProjMatrixInverse)
{
	GLVector4 boxVertex[8] = {
		GLVector4(lndcX,bndcY,-1.0f,1.0f),//near plan
		GLVector4(rndcX,bndcY,-1.0f,1.0f),//
		GLVector4(lndcX,tndcY,-1.0f,1.0f),
		GLVector4(rndcX,tndcY,-1.0f,1.0f),//

		GLVector4(lndcX,bndcY,1.0f,1.0f),//far plan
		GLVector4(rndcX	,bndcY,1.0f,1.0f),
		GLVector4(lndcX,tndcY,1.0f,1.0f),
		GLVector4(rndcX,tndcY,1.0f,1.0f)
	};
	GLVector3   afterVertex[8];
	for (int i = 0; i < 8; ++i)
	{
		const GLVector4 position = boxVertex[i] * viewProjMatrixInverse;
		afterVertex[i] = GLVector3(position.x/position.w,position.y/position.w,position.z/position.w);
	}
	/*
	  *create 6 normal plan
	 */
	//+Xƽ��
	GLVector3  crossVec = (afterVertex[1] - afterVertex[5]).cross(afterVertex[7]-afterVertex[5]);
	_planes[0].init(crossVec,crossVec.dot(afterVertex[5]));
	//-X
	crossVec = (afterVertex[4]-afterVertex[0]).cross(afterVertex[2]-afterVertex[0]);
	_planes[1].init(crossVec,crossVec.dot(afterVertex[0]));
	//+Y
	crossVec = (afterVertex[7]-afterVertex[6]).cross(afterVertex[2]-afterVertex[6]);
	_planes[2].init(crossVec,crossVec.dot(afterVertex[6]));
	//-Y
	//GLVector3 a = afterVertex[1] - afterVertex[0];
	//GLVector3 b = afterVertex[4] - afterVertex[0];
	crossVec = (afterVertex[1]-afterVertex[0]).cross(afterVertex[4]-afterVertex[0]);
	_planes[3].init(crossVec,crossVec.dot(afterVertex[0]));
	//+Z
	crossVec =(afterVertex[0]-afterVertex[1]).cross(afterVertex[3]-afterVertex[1]) ;
	_planes[4].init(crossVec,crossVec.dot(afterVertex[1]));
	//-Z
	crossVec = (afterVertex[5]-afterVertex[4]).cross(afterVertex[6]-afterVertex[4]);
	_planes[5].init(crossVec,crossVec.dot(afterVertex[4]));
}

bool Frustum::isOutOfFrustum(const AABB &box)const
{
	GLVector3 p3d;
	for (int i = 0; i < 6; ++i)
	{
		const Plane  *plane = _planes+i;
		const GLVector3 &normal = plane->getNormal();
		//ѡ���뷨�ߵĸ��������෴�������,��ʱ���������ƽ����������Ϊ��,��һ��˵��ģ�͵İ�Χ����ƽ��ͷ��֮��
		//������Ҫע�����,���ж�����Ϊ�������
		p3d.x = normal.x > 0.0f ? box._minBox.x:box._maxBox.x;
		p3d.y = normal.y > 0.0f ? box._minBox.y:box._maxBox.y;
		p3d.z = normal.z > 0.0f ? box._minBox	.z:box._maxBox.z;
		if (plane->distance(p3d) > 0.0f)
			return true;
	}
	return false;
}
bool Frustum::isOutOfFrustum(const GLVector3 &position)const
{
	//����������
	for (int k = 0; k < 6; ++k)
	{
		if (_planes[k].distance(position)<0)//������κ�һ��ƽ��ı���,��˵������׶��֮��
			return true;
	}
	return false;
}
__NS_GLK_END
