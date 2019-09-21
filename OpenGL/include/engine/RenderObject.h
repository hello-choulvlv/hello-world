/*
 *基础渲染对象,如果想要以一种统一的方式渲染物体,都已实现这个类,然后调用GLContext::addChild
 *2016-10-9 10:14:15
 *@Author:小花熊
 */
#ifndef  __RENDER_OBJECT_H__
#define __RENDER_OBJECT_H__
#include<engine/Object.h>
#include<engine/Geometry.h>
__NS_GLK_BEGIN
/*
 *draw函数中flag的含义,请参见GLState.h enum _tDrawFlag
 */
class     RenderObject :public  Object
{
protected:
	float            _deltaTime;
private:
	RenderObject(RenderObject &);
protected:
	RenderObject(){ _deltaTime = 0.0f; };
	~RenderObject(){};
public:
	virtual       void           update(float    _deltaTime){};
	virtual       void           draw(Matrix        &_projMatrix, unsigned       flag){};
};
__NS_GLK_END
#endif
