/*
  *
 */
#include "GL/glew.h"
#include "GodRay.h"
#include "engine/GLContext.h"
#include "engine/GLCacheManager.h"
#include "engine/event/EventManager.h"
#include<math.h>

GodRay::GodRay()
{
	_twistBufferId = 0;
	_twistIndexId = 0;
	_rayShader = nullptr;
	_renderShader = nullptr;
}

GodRay::~GodRay()
{
	glDeleteBuffers(1, &_twistBufferId);
	glDeleteBuffers(1, &_twistIndexId);
	_twistBufferId = 0;
	_twistIndexId = 0;
	_rayShader->release();
	_renderShader->release();
	_rtt->release();
	glk::EventManager::getInstance()->removeListener(_touchEventListener);
	_touchEventListener->release();
}

GodRay *GodRay::createGodRay()
{
	GodRay *godRay = new GodRay();
	godRay->initGodRay();
	return godRay;
}

void   GodRay::initGodRay()
{
	loadTwistObject();
	initCamera(glk::GLVector3(0.0f,0.0f,90.0f),glk::GLVector3(0.0f,0.0f,0.0f));
	_color = glk::GLVector4(0.4f,0.2f,0.8f,1.0f);
	_rayShader = RayShader::createRayShader("shader/GodRay/GodRay_VS.glsl", "shader/GodRay/GodRay_FS.glsl");
	_renderShader = RenderShader::createRenderShader("shader/GodRay/Render_VS.glsl", "shader/GodRay/Render_FS.glsl");
	//渲染到纹理
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	_rtt = glk::RenderTexture::createRenderTexture(winSize, glk::RenderTexture::RenderType::ColorBuffer|glk::RenderTexture::RenderType::DepthBuffer);
	//事件监听
	_touchEventListener = glk::TouchEventListener::createTouchListener(this,glk_touch_selector(GodRay::onTouchBegan), glk_move_selector(GodRay::onTouchMoved),glk_release_selector(GodRay::onTouchEnded));
	glk::EventManager::getInstance()->addTouchEventListener(_touchEventListener,0);
}
void   GodRay::initCamera(const glk::GLVector3 &eyePosition, const glk::GLVector3 &targetPosition)
{
	_eyePosition = eyePosition;
	_targetPosition = targetPosition;
	/*
	*/
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	_projMatrix.perspective(60.0f, winSize.width / winSize.height, 0.1f, 100.0f);
	//_viewMatrix.lookAt(eyePosition, targetPosition, glk::GLVector3(0.0f,1.0f,0.0f));
	_zaxis = (_eyePosition - _targetPosition).normalize();
	_yaxis = glk::GLVector3(0.0f,1.0f,0.0f);
	_axis =_yaxis.cross(_zaxis) ;
	_yaxis = _zaxis.cross(_axis);
	buildeViewMatrix();
}
/*
  *创建沿着Y轴自下而上,且中心在世界坐标原点的螺旋体
 */
void GodRay::loadTwistObject()
{
	//旋螺线重复的次数
	const float twist = 5.0f;
	//每个螺旋线的长度
	const float length = 9.0f;
	//每个螺旋线的横切面被分割的次数
	const float hsplit = 60.0f;
	//每个螺旋体纵切面被分割的次数
	const float vsplit = 30.0f+1.0f;
	//螺旋线的半径
	const float radius = 5.0f;
	//螺旋体的半径
	const float thickness = 2.0f;
	//对于沿着Y轴,每次的跨度
	const float ystep = length/hsplit;
	//起始的两个螺旋线坐标
	const glk::GLVector3 yaxis(0.0f,1.0f,0.0f);
	glk::Matrix3   rotateMatrix;
	rotateMatrix.rotate(360.0f/hsplit,yaxis);
	glk::GLVector3 a(0.0f,0.0f, radius);
	glk::GLVector3 b = a*rotateMatrix;
	//螺旋体的坐标
	glk::GLVector3 x1(0.0f,0.0f,thickness);
	//构造螺旋体,在这一版中我们没有使用索引缓冲区对象
	_vertexSize = twist *hsplit * vsplit;
	glk::GLVector3 *VertexData = new  glk::GLVector3[_vertexSize*2];
	const float twistLength = length *twist;
	const float halfTwist = twistLength / 2.0f;
	const float step = twistLength / (twist *hsplit );
	const float repeatCount = twist * hsplit;
	float  L = -halfTwist;
	float  secondaryL = L + step;
	int     index = 0;
	for (float u=0; u< repeatCount; u+=1.0)
	{
		//沿着螺旋线的连续的离散的三个点
		glk::GLVector3 m1(a.x,L,a.z);
		glk::GLVector3  m2(b.x,secondaryL,b.z);
		//求法线
		glk::GLVector3  n1 = (m2-m1).normalize();
		//旋转矩阵
		glk::Matrix3       r1;
		r1.rotate(360.0f / (vsplit-1.0f), n1);
		glk::GLVector3 x1a = x1;
		for (float v = 0; v < vsplit; v+=1)
		{
			VertexData[index] = m1 + x1;
			VertexData[index + 1] = x1;
			index+= 2;
			x1 = x1 * r1;
		}
		x1 = x1a * rotateMatrix;
		a = b;
		b = b * rotateMatrix;
		L = secondaryL;
		secondaryL += step;
	}
	//求索引
	_indexSize = ceil(twist) * (hsplit-1)*(vsplit-1) * 6;
	int  *indexVertex = new int[_indexSize];
	const int ycount = twist *(hsplit-1);
	index = 0;
	for (int y = 0; y < ycount; ++y)
	{
		for (int x = 0; x < vsplit-1; ++x)
		{
			indexVertex[index] = (y+1)*vsplit +x;
			indexVertex[index + 1] = y*vsplit + x;
			indexVertex[index + 2] = (y + 1)*vsplit + x + 1;

			indexVertex[index + 3] = (y + 1)*vsplit + x + 1;
			indexVertex[index + 4] = y*vsplit+x;
			indexVertex[index + 5] = y*vsplit + x + 1;

			index += 6;
		}
	}
	//OpenGL缓冲区对象
	glGenBuffers(1, &_twistBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _twistBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glk::GLVector3) * 2* _vertexSize,VertexData,GL_STATIC_DRAW);
	//
	glGenBuffers(1, &_twistIndexId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _twistIndexId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*_indexSize,indexVertex,GL_STATIC_DRAW);
	//
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	delete VertexData;
	delete indexVertex;
	VertexData = nullptr;
	indexVertex = nullptr;
}

void   GodRay::draw()
{
	/*
	  *绑定Shader
	 */
	_rayShader->perform();
	//切换帧缓冲区对象
	_rtt->activeFramebuffer();
	/*
	  *绑定顶点缓冲区对象,索引缓冲区对象
	 */
	glBindBuffer(GL_ARRAY_BUFFER, _twistBufferId);
	if (_rayShader->getPositionLoc() >= 0)
	{
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, nullptr);
	}
	if (_rayShader->getNormalLoc() >= 0)
	{
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));
	}
	/*
	  *uniform
	 */
	_rayShader->setModelMatrix(_modelMatrix);
	_rayShader->setViewProjMatrix(_viewProjMatrix);
	_rayShader->setColor(_color);
	_rayShader->setEyePosition(_eyePosition);
	_rayShader->setNormalMatrix(_modelMatrix.normalMatrix());
	/*
	 */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _twistIndexId);
	glDrawElements(GL_TRIANGLES, _indexSize, GL_UNSIGNED_INT, nullptr);
	_rtt->disableFramebuffer();
	//使用默认的帧缓冲区对象
	_renderShader->perform();
	//使用单位顶点缓冲区对象
	int identityVertex = glk::GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER, identityVertex);
	if (_renderShader->getPositionLoc() >= 0)
	{
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);
	}
	if (_renderShader->getFragCoordLoc() >= 0)
	{
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5,(void*)(sizeof(float)*3));
	}
	//传递Uniform
	glk::Matrix identityMatrix;
	_renderShader->setMVPMatrix(identityMatrix);
	_renderShader->setBaseMap(_rtt->getColorBuffer(),0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void  GodRay::update(float deltaTime)
{

}

void   GodRay::buildeViewMatrix()
{
	typedef float(*MatrixArray)[4];
	MatrixArray array = (MatrixArray)_viewMatrix.pointer();
	array[0][0] = _axis.x;
	array[1][0] = _axis.y;
	array[2][0] = _axis.z;

	array[0][1] = _yaxis.x;
	array[1][1] = _yaxis.y;
	array[2][1] = _yaxis.z;

	array[0][2] = _zaxis.x;
	array[1][2] = _zaxis.y;
	array[2][2] = _zaxis.z;

	array[0][3] = 0.0f;
	array[1][3] = 0.0f;
	array[2][3] = 0.0f;

	array[3][0] = -_axis.dot(_eyePosition);
	array[3][1] = -_yaxis.dot(_eyePosition);
	array[3][2] = -_zaxis.dot(_eyePosition);
	array[3][3] = 1.0f;

	_viewProjMatrix = _viewMatrix * _projMatrix;
}

bool  GodRay::onTouchBegan(const glk::GLVector2 *touchPoint)
{
	_offsetPoint = *touchPoint;
	return true;
}

void GodRay::onTouchMoved(const glk::GLVector2 *touchPoint)
{
	const glk::GLVector2 nowPoint = *touchPoint;
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	//在X方向有偏移
	if (_offsetPoint.x != nowPoint.x)
	{
		float deltaX =( nowPoint.x - _offsetPoint.x)/winSize.width * __MATH_PI__ * _ANGLE_FACTOR_;
		glk::Matrix3 rotateM;
		rotateM.rotate(-deltaX, glk::GLVector3(0.0f,1.0f,0.0f));
		_axis = _axis * rotateM;
		_yaxis = _yaxis * rotateM;
		_zaxis = _zaxis * rotateM;
	}
	if (_offsetPoint.y != nowPoint.y)
	{
		float deltaY = (nowPoint.y - _offsetPoint.y) / winSize.height * __MATH_PI__ * _ANGLE_FACTOR_;
		glk::Matrix3 rotateN;
		rotateN.rotate(deltaY, _axis);
		_yaxis = _yaxis *rotateN;
		_zaxis = _zaxis * rotateN;
		//防止超过180度的上下旋转
		//if (_yaxis.y < 0)
		//{
		//	_zaxis = glk::GLVector3(0.0f,_zaxis.y>0.0f?1.0f:-1.0f,0.0f);
		//	_yaxis = _zaxis.cross(_axis);
		//}
	}
	_eyePosition = _targetPosition + _zaxis * (_eyePosition - _targetPosition).length();
	buildeViewMatrix();
	_offsetPoint = nowPoint;
}

void GodRay::onTouchEnded(const glk::GLVector2 *touchPoint)
{

}