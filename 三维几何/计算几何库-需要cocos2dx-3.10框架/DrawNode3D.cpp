/****************************************************************************
 Copyright (c) 2014 Chukong Technologies Inc.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "DrawNode3D.h"
#include "matrix/matrix.h"
NS_CC_BEGIN


DrawNode3D::DrawNode3D()
: _vao(0)
, _vbo(0)
, _bufferCapacity(0)
, _bufferCount(0)
, _buffer(nullptr)
, _dirty(false)
{
    _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
}

DrawNode3D::~DrawNode3D()
{
    free(_buffer);
    _buffer = nullptr;
    
    glDeleteBuffers(1, &_vbo);
    _vbo = 0;
    
    if (Configuration::getInstance()->supportsShareableVAO())
    {
        glDeleteVertexArrays(1, &_vao);
        GL::bindVAO(0);
        _vao = 0;
    }
}

DrawNode3D* DrawNode3D::create()
{
    DrawNode3D* ret = new (std::nothrow) DrawNode3D();
    if (ret && ret->init())
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}

void DrawNode3D::ensureCapacity(int count)
{
    CCASSERT(count>=0, "capacity must be >= 0");
    
    if(_bufferCount + count > _bufferCapacity)
    {
		_bufferCapacity += MAX(_bufferCapacity, count);
		_buffer = (V3F_C4B*)realloc(_buffer, _bufferCapacity*sizeof(V3F_C4B));
	}
}

bool DrawNode3D::init()
{
    _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;

    setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_COLOR));
    
    ensureCapacity(512);
    
    if (Configuration::getInstance()->supportsShareableVAO())
    {
        glGenVertexArrays(1, &_vao);
        GL::bindVAO(_vao);
    }
    
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B)* _bufferCapacity, _buffer, GL_STREAM_DRAW);
    
    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B), (GLvoid *)offsetof(V3F_C4B, vertices));
    
    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B), (GLvoid *)offsetof(V3F_C4B, colors));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    if (Configuration::getInstance()->supportsShareableVAO())
    {
        GL::bindVAO(0);
    }
    
    CHECK_GL_ERROR_DEBUG();
    
    _dirty = true;
    
#if CC_ENABLE_CACHE_TEXTURE_DATA
    // Need to listen the event only when not use batchnode, because it will use VBO
    auto listener = EventListenerCustom::create(EVENT_COME_TO_FOREGROUND, [this](EventCustom* event){
    /** listen the event that coming to foreground on Android */
        this->init();
    });

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
#endif
    
    return true;
}

void DrawNode3D::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
    _customCommand.init(_globalZOrder, transform, flags);
    _customCommand.func = CC_CALLBACK_0(DrawNode3D::onDraw, this, transform, flags);
    renderer->addCommand(&_customCommand);
}

void DrawNode3D::onDraw(const Mat4 &transform, uint32_t flags)
{
    auto glProgram = getGLProgram();
    glProgram->use();
    glProgram->setUniformsForBuiltins(transform);
    glEnable(GL_DEPTH_TEST);
    RenderState::StateBlock::_defaultState->setDepthTest(true);
    GL::blendFunc(_blendFunc.src, _blendFunc.dst);

    if (_dirty)
    {
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B)*_bufferCapacity, _buffer, GL_STREAM_DRAW);
        _dirty = false;
    }
    if (Configuration::getInstance()->supportsShareableVAO())
    {
        GL::bindVAO(_vao);
    }
    else
    {
        GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        // vertex
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B), (GLvoid *)offsetof(V3F_C4B, vertices));

        // color
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B), (GLvoid *)offsetof(V3F_C4B, colors));
    }

    glDrawArrays(GL_LINES, 0, _bufferCount);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1,_bufferCount);
    CHECK_GL_ERROR_DEBUG();
}

void DrawNode3D::drawLine(const Vec3 &from, const Vec3 &to, const Color4F &color)
{
    unsigned int vertex_count = 2;
    ensureCapacity(vertex_count);
    
    Color4B col = Color4B(color);
    V3F_C4B a = {Vec3(from.x, from.y, from.z), col};
    V3F_C4B b = {Vec3(to.x, to.y, to.z), col, };
    
    V3F_C4B *lines = (V3F_C4B *)(_buffer + _bufferCount);
    lines[0] = a;
    lines[1] = b;
    
    _bufferCount += vertex_count;
    _dirty = true;
}

void DrawNode3D::drawCircle(const cocos2d::Vec3 &center, float radius, int segment, const cocos2d::Vec3 &normal_direction, const cocos2d::Color4F &color)
{
	float  *vertex_array = new float[segment * 8];
	//计算初始点,使用一个比较精巧的算法实现
	float  d = Vec3::dot(center,normal_direction);
	const Vec3  origin_point = Vec3(center.x + 10,center.y + 10,center.z +10);
	//求法线上某点与所设置的该点的连线的夹角
	float  normal_length = 50;
	const Vec3 normal_some_point = center + normal_direction * normal_length;
	const Vec3 direction_line = origin_point - normal_some_point;
	const Vec3 direction = direction_line.getNormalized();
	float  cos_v = -Vec3::dot(direction,normal_direction);
	float  length = normal_length/cos_v;
	const Vec3 plane_point = normal_some_point + direction * length;

	Vec3 rotate_vector = plane_point - center;
	rotate_vector = rotate_vector * (radius/rotate_vector.length());
	Color4B	 color4(color);
	for (int index_l = 0; index_l < segment; ++index_l)
	{
		//旋转角度
		Quaternion  quaternion(normal_direction, 2.0f * M_PI * index_l/segment);
		Quaternion  q2(normal_direction, 2.0f * M_PI * (index_l +1)/segment);

		const Vec3 p1 = quaternion * rotate_vector + center;
		const Vec3 p2 = q2 * rotate_vector + center;
		int  base_j = index_l * 8;
		vertex_array[base_j] = p1.x;
		vertex_array[base_j + 1] = p1.y;
		vertex_array[base_j + 2] = p1.z;
		*(Color4B *)(vertex_array + base_j + 3) = color4;

		base_j += 4;
		vertex_array[base_j] = p2.x;
		vertex_array[base_j + 1] = p2.y;
		vertex_array[base_j + 2] = p2.z;
		*(Color4B *)(vertex_array + base_j + 3) = color4;
	}

	ensureCapacity(segment * 2);
	memcpy(_buffer + _bufferCount, vertex_array, segment * 8 * sizeof(float));
	_bufferCount += segment * 2;
	_dirty = true;

	delete[] vertex_array;
	vertex_array = nullptr;
}

void  DrawNode3D::drawSphere(const cocos2d::Vec3 &center, float radius, short grid_x, short grid_y, const cocos2d::Color4F &color)
{
	//自下向上,从左到右创建mesh
	int  point_count = grid_x * grid_y * 4;
	int  stride_size = 3 +1;
	Color4B  secondary_color(color);
	float  *vertex_array = new float[point_count * stride_size];

	int  index_l = 0;
	for (int y = 0; y < grid_y; ++y)
	{
		float  height_y = -radius * cosf(M_PI * y/grid_y) + center.y;
		float  secondary_y = -radius * cosf(M_PI * (y+1)/grid_y) + center.y;

		float secondary_radius = radius * sinf(M_PI * y/grid_y);
		float tripple_radius = radius * sinf(M_PI * (y +1)/grid_y);

		for (int x = 0; x < grid_x; ++x, ++index_l)
		{
			float  x_1 = secondary_radius * sinf(2.0f * M_PI * x / grid_x) + center.x;
			float  x_2 = secondary_radius * sinf(2.0f *M_PI * (x+1)/grid_x) + center.x;

			float z_1 = secondary_radius * cosf(2.0f *M_PI * x/grid_x) + center.z;
			float z_2 = secondary_radius * cosf(2.0f *M_PI * (x+1)/grid_x) + center.z;

			int base_j = index_l * 16;
			vertex_array[base_j] = x_1;
			vertex_array[base_j + 1] = height_y;
			vertex_array[base_j + 2] = z_1;
			*((Color4B *)(vertex_array+(base_j + 3))) = secondary_color;

			base_j += 4;
			vertex_array[base_j] = x_2;
			vertex_array[base_j + 1] = height_y;
			vertex_array[base_j + 2] = z_2;
			*((Color4B *)(vertex_array + (base_j + 3))) = secondary_color;

			base_j += 4;
			vertex_array[base_j] = x_1;
			vertex_array[base_j + 1] = height_y;
			vertex_array[base_j + 2] = z_1;
			*((Color4B *)(vertex_array + (base_j + 3))) = secondary_color;

			base_j += 4;
			vertex_array[base_j] = tripple_radius * sinf(2.0f *M_PI * x / grid_x) + center.x;
			vertex_array[base_j + 1] = secondary_y;
			vertex_array[base_j + 2] = tripple_radius * cosf(2.0f *M_PI * x / grid_x) + center.z;
			*((Color4B *)(vertex_array + (base_j + 3))) = secondary_color;
		}
	}
	//填充数据
	ensureCapacity(point_count);
	memcpy(_buffer + _bufferCount, vertex_array, point_count * stride_size * sizeof(float));
	_bufferCount += point_count;
	_dirty = true;

	delete[] vertex_array;
	vertex_array = nullptr;
}

void DrawNode3D::drawMesh(const cocos2d::Vec3 &center, const Vec3 &normal, const Vec2 &extent, int xgrid, int ygrid,const Color4F &color)
{
	//算法将会以XOY平面为基础进行旋转
	const Vec3 base_axis(0,0,1);
	Vec3 axis_vec = gt::cross_normalize(base_axis, normal);
	float length = gt::length2(axis_vec);
	//计算旋转角度
	float angle = acosf(Vec3::dot(normal,base_axis));
	//需要判断是否旋转角度为0
	Quaternion  quaternion(length==0?base_axis :axis_vec, length == 0?0:angle);
	//计算两个旋转轴
	const Vec3 base_xdirection(extent.x,0,0),base_ydirection(0, extent.y,0);
	const Vec3 xdirection = quaternion * base_xdirection;
	const Vec3 ydirection = quaternion * base_ydirection;
	//计算离散点的坐标
	Vec3 base_location = center - xdirection - ydirection;
	Vec3 secondary_location = center + xdirection - ydirection;
	Color4B  color4b(color);
	int point_count = (xgrid + ygrid + 2) * 2;
	float *vertex_data = new float[point_count * 4];
	int base_j = 0;
	//纵向
	for (int base_y = 0; base_y < ygrid + 1; ++base_y)
	{
		float * array_data = vertex_data + base_j;
		*(Vec3 *)(array_data) = base_location + ydirection * (2.0f * base_y / ygrid);
		*(Vec3 *)(array_data +4) = secondary_location + ydirection * (2.0f * base_y / ygrid);
		*(Color4B *)(array_data + 3) = color4b;
		*(Color4B *)(array_data + 7) = color4b;

		base_j += 8;
	}
	//横向
	secondary_location = base_location + ydirection * 2.0f;
	for (int base_x = 0; base_x < xgrid + 1; ++base_x)
	{
		float * array_data = vertex_data + base_j;
		*(Vec3 *)(array_data) = base_location + xdirection * (2.0f * base_x/xgrid);
		*(Vec3 *)(array_data + 4) = secondary_location + xdirection *(2.0f * base_x/xgrid);
		*(Color4B*)(array_data + 3) = color4b;
		*(Color4B *)(array_data + 7) = color4b;

		base_j += 8;
	}
	//Copy to buffer
	ensureCapacity(point_count);
	memcpy(_buffer + _bufferCount, vertex_data, point_count * 4 * sizeof(float));
	_bufferCount += point_count;
	_dirty = true;

	delete[] vertex_data;
	vertex_data = nullptr;
}

void DrawNode3D::drawCone(const Vec3 &top, const Vec3 &normal, float h, float r, int cycle_count, int segment_count, const Color4F &color)
{
	//计算顶点的数目
	int point_count = (cycle_count * (segment_count + 1) + segment_count + 2) * 2;
	float *vertex_data = new float[point_count * 4];
	int  base_j = 0;
	const Vec3 base_axis(0,0,1);
	const Vec3 vertical_vec = gt::cross_normalize(base_axis, normal);
	float length = vertical_vec.length();
	const Vec3 normal_axis = length == 0?base_axis:vertical_vec;
	float rotate_angle = length == 0?0:acosf(normal.dot(base_axis));
	Quaternion quaternion(normal_axis,rotate_angle);
	Color4B color4b(color);
	//画出离散圆
	for (int j = 0; j < cycle_count; ++j)
	{
		const Vec3 center = top + normal * (h * (j +1)/ cycle_count);
		float f = r * (j + 1) / cycle_count;
		for (int base_k = 0; base_k < segment_count + 1; ++base_k)
		{
			float angle = 2.0f * M_PI * base_k/segment_count;
			Vec3 direction(cosf(angle),sinf(angle),0);
			direction = quaternion * direction;
			*(Vec3 *)(vertex_data + base_j) = center + direction * f;
			*(Color4B*)(vertex_data + base_j + 3) = color4b;

			float secondary_angle = 2.0f * M_PI * (base_k +1)/segment_count;
			direction = Vec3(cosf(secondary_angle),sinf(secondary_angle),0.0f);
			direction = quaternion * direction;
			*(Vec3 *)(vertex_data + base_j + 4) = center + direction * f;
			*(Color4B*)(vertex_data + base_j + 7) = color4b;

			base_j += 8;
		}
	}
	//画出锥形的母线
	const Vec3 center = top + normal * h;
	for (int j = 0; j < segment_count; ++j)
	{
		*(Vec3 *)(vertex_data + base_j) = top;
		*(Color4B *)(vertex_data + base_j + 3) = color4b;

		float angle = 2.0f * M_PI * j / segment_count;
		Vec3 direction(cosf(angle), sinf(angle), 0);
		direction = quaternion * direction;

		*(Vec3*)(vertex_data + base_j + 4) = center + direction * r;
		*(Color4B *)(vertex_data + base_j + 7) = color4b;

		base_j += 8;
	}
	ensureCapacity(point_count);
	memcpy(_buffer + _bufferCount, vertex_data, point_count * 4 * sizeof(float));
	_bufferCount += point_count;
	_dirty = true;

	delete[] vertex_data;
	vertex_data = nullptr;
}

void DrawNode3D::drawCylinder(const Vec3 &bottom, const Vec3 &top, float r, int generatrix_num, int segment_num, int cycle_num,const Color4F &color)
{
	const Vec3 direction = top - bottom;
	drawCylinder(bottom, gt::normalize(direction), direction.length(), r,generatrix_num, segment_num,cycle_num,color);
}

void DrawNode3D::drawCylinder(const Vec3 &bottom, const Vec3 &direction, float length, float r, int generatrix_num, int segment_num, int cycle_num,const Color4F &color)
{
	//以XOY平面为基准,计算旋转矩阵
	const Vec3 base_axis(0,0,1);
	const Vec3 tripple_axis = gt::cross_normalize(base_axis,direction);
	float angle = acosf(direction.z);
	float length2 = gt::length2(tripple_axis);
	const Vec3 normal_axis = length2 == 0.0f?base_axis:tripple_axis;
	angle = length2 ==0.0f?0:angle;
	Quaternion quaternion(normal_axis,angle);
	//计算离散点的数目
	int point_count =2 *  generatrix_num + 2 * (segment_num + 1) * cycle_num;
	float *vertex_data = new float[point_count * 4];
	int base_j = 0;
	Color4B color4b(color);
	//画出离散圆
	for (int index_l = 0; index_l <= cycle_num; ++index_l)
	{
		const Vec3 base_location = bottom + direction * (length * index_l/cycle_num);
		for (int index_j = 0; index_j < segment_num; ++index_j)
		{
			float radius = 2.0f * M_PI * index_j / segment_num;
			Vec3 d(cosf(radius),sinf(radius),0.0f);
			d = quaternion * d;

			*(Vec3 *)(vertex_data + base_j) = base_location + d * r;
			*(Color4B *)(vertex_data + base_j + 3) = color4b;
			base_j += 4;

			radius = 2.0f * M_PI * (index_j +1.0f) / segment_num;
			d = Vec3(cosf(radius), sinf(radius), 0.0f);
			d = quaternion * d;

			*(Vec3 *)(vertex_data + base_j) = base_location + d * r;
			*(Color4B *)(vertex_data + base_j + 3) = color4b;
			base_j += 4;
		}
	}
	//母线
	const Vec3 top = bottom + direction * length;
	for (int index_l = 0; index_l < generatrix_num; ++index_l)
	{
		float radius = 2.0f * M_PI * index_l / generatrix_num;
		Vec3 d(cosf(radius), sinf(radius), 0.0f);
		d = quaternion * d;

		*(Vec3 *)(vertex_data + base_j) = bottom + d * r;
		*(Color4B *)(vertex_data + base_j + 3) = color4b;
		base_j += 4;

		*(Vec3 *)(vertex_data + base_j) = top + d * r;
		*(Color4B *)(vertex_data + base_j + 3) = color4b;
		base_j += 4;
	}
	ensureCapacity(point_count);
	memcpy(_buffer + _bufferCount, vertex_data, point_count * 4 * sizeof(float));
	_bufferCount += point_count;
	_dirty = true;

	delete[] vertex_data;
	vertex_data = nullptr;
}

void DrawNode3D::drawTriangle(const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Color4F &color)
{
	drawLine(a, b, color);
	drawLine(b,c,color);
	drawLine(c,a,color);
}

void DrawNode3D::drawTriangle(const cocos2d::Vec3 *v, const cocos2d::Color4F &color)
{
	drawLine(v[0], v[1], color);
	drawLine(v[1], v[2], color);
	drawLine(v[2], v[0], color);
}

void DrawNode3D::drawTetrahedron(const Vec3 &a, const Vec3 &b, const Vec3 &c, const Vec3 &d, const Color4F &color)
{
	drawLine(a, b, color);
	drawLine(b, c, color);
	drawLine(c, a, color);

	drawLine(a, d, color);
	drawLine(b, d, color);
	drawLine(c, d, color);
}
void DrawNode3D::drawTetrahedron(const Vec3 vert[4], const Color4F &color)
{
	drawLine(vert[0], vert[1], color);
	drawLine(vert[1], vert[2], color);
	drawLine(vert[2], vert[0], color);

	drawLine(vert[0], vert[3], color);
	drawLine(vert[1], vert[3], color);
	drawLine(vert[2], vert[3], color);
}

void DrawNode3D::drawLineStrip(const cocos2d::Vec3 *vertex, int vertex_size, const Color4F color)
{
	for (int index_l = 1; index_l < vertex_size; ++index_l)
		drawLine(vertex[index_l -1],vertex[index_l],color);
}

void DrawNode3D::drawLineCircle(const cocos2d::Vec3 *vertex, int vertex_size, const Color4F color)
{
	for (int index_l = 0; index_l < vertex_size; ++index_l)
		drawLine(vertex[index_l], vertex[index_l + 1 >= vertex_size?0:index_l + 1], color);
}

void DrawNode3D::drawCube(Vec3* vertices, const Color4F &color)
{
    // front face
    drawLine(vertices[0], vertices[1], color);
    drawLine(vertices[1], vertices[2], color);
    drawLine(vertices[2], vertices[3], color);
    drawLine(vertices[3], vertices[0], color);
    
    // back face
    drawLine(vertices[4], vertices[5], color);
    drawLine(vertices[5], vertices[6], color);
    drawLine(vertices[6], vertices[7], color);
    drawLine(vertices[7], vertices[4], color);
    
    // edge
    drawLine(vertices[0], vertices[7], color);
    drawLine(vertices[1], vertices[6], color);
    drawLine(vertices[2], vertices[5], color);
    drawLine(vertices[3], vertices[4], color);
}

void DrawNode3D::drawCube2(cocos2d::Vec3* vertices, const Color4F &xColor, const Color4F &yColor, const Color4F	 &zColor)
{
	// front face
	drawLine(vertices[0], vertices[1], xColor);
	drawLine(vertices[1], vertices[2], yColor);
	drawLine(vertices[2], vertices[3], xColor);
	drawLine(vertices[3], vertices[0], yColor);

	// back face
	drawLine(vertices[4], vertices[5], xColor);
	drawLine(vertices[5], vertices[6], yColor);
	drawLine(vertices[6], vertices[7], xColor);
	drawLine(vertices[7], vertices[4], yColor);

	// edge
	drawLine(vertices[0], vertices[4], zColor);
	drawLine(vertices[1], vertices[5], zColor);
	drawLine(vertices[3], vertices[7], zColor);
	drawLine(vertices[2], vertices[6], zColor);
}

void DrawNode3D::drawAABB(const cocos2d::Vec3 &min_vertex, const cocos2d::Vec3 &max_vertex,const cocos2d::Color4F &color)
{
	drawLine(min_vertex,Vec3(max_vertex.x,min_vertex.y,min_vertex.z),color);
	drawLine(Vec3(max_vertex.x, min_vertex.y, min_vertex.z),Vec3(max_vertex.x,min_vertex.y,max_vertex.z),color);
	drawLine(Vec3(max_vertex.x, min_vertex.y, max_vertex.z), Vec3(min_vertex.x,min_vertex.y,max_vertex.z),color);
	drawLine(Vec3(min_vertex.x, min_vertex.y, max_vertex.z), min_vertex,color);

	drawLine(Vec3(min_vertex.x,max_vertex.y,min_vertex.z), Vec3(max_vertex.x, max_vertex.y, min_vertex.z), color);
	drawLine(Vec3(max_vertex.x, max_vertex.y, min_vertex.z), Vec3(max_vertex.x, max_vertex.y, max_vertex.z), color);
	drawLine(Vec3(max_vertex.x, max_vertex.y, max_vertex.z), Vec3(min_vertex.x, max_vertex.y, max_vertex.z), color);
	drawLine(Vec3(min_vertex.x, max_vertex.y, max_vertex.z), Vec3(min_vertex.x,max_vertex.y,min_vertex.z), color);

	drawLine(min_vertex, Vec3(min_vertex.x, max_vertex.y, min_vertex.z), color);
	drawLine(Vec3(max_vertex.x, min_vertex.y, min_vertex.z), Vec3(max_vertex.x, max_vertex.y, min_vertex.z), color);
	drawLine(Vec3(max_vertex.x, min_vertex.y, max_vertex.z), max_vertex, color);
	drawLine(Vec3(min_vertex.x, min_vertex.y, max_vertex.z), Vec3(min_vertex.x, max_vertex.y, max_vertex.z), color);
}

void DrawNode3D::clear()
{
    _bufferCount = 0;
    _dirty = true;
}

const BlendFunc& DrawNode3D::getBlendFunc() const
{
    return _blendFunc;
}

void DrawNode3D::setBlendFunc(const BlendFunc &blendFunc)
{
    _blendFunc = blendFunc;
}

NS_CC_END
