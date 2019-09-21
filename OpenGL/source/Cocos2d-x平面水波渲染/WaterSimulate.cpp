/*
 *Water Simulate
 *2017/2/23
 *@Author:xiaoxiong
 */
#include"WaterSimulate.h"
#include"geometry/Geometry.h"
using namespace cocos2d;
typedef		GLVector3	 (*WaterType)[__WATER_SQUARE_LANDSCAPE__];
typedef		float				(*WaterMeshType)[__WATER_SQUARE_LANDSCAPE__+2];
//coefficient of wave flow out 
const float WaterCoeffValue = 0.95f;
const int  VertexAttribFloatCount = 5;//每个顶点的浮点数的数目
MeshSquare::MeshSquare()
{
	_vertexBufferId = 0;
	_normalBufferId = 0;
	_indexBufferId = 0;
	_countOfIndex = 0;
	_countOfVertex = 0;
}

MeshSquare::~MeshSquare()
{
	glDeleteBuffers(1, &_vertexBufferId);
	glDeleteBuffers(1, &_normalBufferId);
	glDeleteBuffers(1, &_indexBufferId);
	_vertexBufferId = 0;
	_normalBufferId = 0;
	_indexBufferId = 0;
}

void MeshSquare::initWithMeshSquare(float width, float height, int xgrid, int ygrid, float fragCoord)
{
	assert(xgrid>0 && ygrid>0);
	_countOfVertex = (xgrid + 1)*(ygrid+1);
	float *Vertex = new float[_countOfVertex*VertexAttribFloatCount];
	for (int j = 0; j < ygrid + 1; ++j)
	{
		const float factor = 2.0f*j / ygrid  -1.0f;
		const float y = height*factor;
		float  *nowVertex = Vertex + j*(xgrid + 1)*VertexAttribFloatCount;
		for (int i = 0; i < xgrid + 1; ++i)
		{
			const int index = i * VertexAttribFloatCount;
			nowVertex[index] =width*(2.0f*i/xgrid  -1.0f) ;
			nowVertex[index + 1] = y;
			nowVertex[index + 2] = 0.0f;
			//除非VertexAttribFloatCount>=5才能开启下面的属性
			nowVertex[index +3] = 1.0f * i/xgrid *fragCoord;
			nowVertex[index+4] = 1.0f * j/ygrid*fragCoord;
		}
	}
	//generate index data
	_countOfIndex = xgrid*ygrid*6;
	int *indexVertex = new int[_countOfIndex];
	for (int j = 0; j < ygrid; ++j)
	{
		int *nowIndex = indexVertex + j*xgrid * 6;
		for (int i = 0; i < xgrid; ++i)
		{
			const int _index = i * 6;
			nowIndex[_index] = (j+1)*xgrid+i;
			nowIndex[_index + 1] = j*xgrid + i;
			nowIndex[_index + 2] = (j + 1)*xgrid + (i + 1);

			nowIndex[_index + 3] = (j+1)*xgrid+(i+1);
			nowIndex[_index + 4] = j*xgrid + i;
			nowIndex[_index + 5] = j*xgrid + i + 1;
		}
	}
	int _defaultVertexId,_defaultIndexId;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &_defaultVertexId);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &_defaultIndexId);
	//generate vertex and frag coord
	glGenBuffers(1, &_vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*_countOfVertex * VertexAttribFloatCount, Vertex, GL_STATIC_DRAW);
	//generate normal
	glGenBuffers(1,&_normalBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _normalBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * _countOfVertex, NULL, GL_DYNAMIC_DRAW);
	//generate index 
	glGenBuffers(1, &_indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*_countOfIndex , indexVertex,GL_STATIC_DRAW);

	delete indexVertex;
	delete Vertex;
	glBindBuffer(GL_ARRAY_BUFFER, _defaultVertexId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _defaultIndexId);
}

void MeshSquare::drawMeshSquare(int posLoc, int normalLoc,int fragCoordLoc)
{
	int _defaultVertexId;
	int _defaultIndexId;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &_defaultVertexId);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &_defaultIndexId);
	//bind vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
	glEnableVertexAttribArray(posLoc);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float)*VertexAttribFloatCount, NULL);
	glEnableVertexAttribArray(fragCoordLoc);
	glVertexAttribPointer(fragCoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(float)*VertexAttribFloatCount,(void*)(sizeof(float)*3));

	//bind normal buffer
	glBindBuffer(GL_ARRAY_BUFFER, _normalBufferId);
	glEnableVertexAttribArray(normalLoc);
	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0,NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
	glDrawElements(GL_TRIANGLES, _countOfIndex, GL_UNSIGNED_INT, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, _defaultVertexId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _defaultIndexId);
}

MeshSquare *MeshSquare::createMeshSquare(float width, float height, int xgrid, int ygrid, float fragCoord)
{
	MeshSquare *_meshSquare = new MeshSquare();
	_meshSquare->initWithMeshSquare(width, height, xgrid, ygrid, fragCoord);
	return _meshSquare;
}

//////////////////////////////////Class of Water Simulate/////////////////////////////////////////////////////////////////////
WaterSimulate::WaterSimulate()
{
	_meshSquare = NULL;
	_isFingerTouch = false;
	_srcMeshIndex = 0;
	_imageBufferId = 0;
	_imageBuffer = NULL;
}

WaterSimulate::~WaterSimulate()
{
	_meshSquare->release();
	_glProgram->release();
	glDeleteTextures(1,&_imageBufferId);
	_imageBufferId = 0;
	delete _imageBuffer;
	_imageBuffer = NULL;
}

void WaterSimulate::initWithSkybox()
{
	const cocos2d::Size &_size = Director::getInstance()->getWinSize();
	//do some adapt to screen
	_meshSquare = MeshSquare::createMeshSquare(_size.width / 2.0f, _size.height / 2.0f, __WATER_SQUARE_LANDSCAPE__ - 1, __WATER_SQUARE_PORTRATE__ - 1, 1.0f);
    
	_glProgram = GLProgram::createWithFilenames("shader/water_simple.vsh", "shader/water_simple.fsh");
	_glProgram->retain();
	_projMatrixLoc = _glProgram->getUniformLocation("CC_PMatrix");
	_modelMatrixLoc = _glProgram->getUniformLocation("u_modelMatrix");
	_skyboxLoc = _glProgram->getUniformLocation("u_skybox");
	_freshnelParamLoc = _glProgram->getUniformLocation("u_freshnelParam");

	_positionLoc = _glProgram->getAttribLocation("a_position");
	_normalLoc = _glProgram->getAttribLocation("a_normal");
	_fragCoordLoc = _glProgram->getAttribLocation("a_fragCoord");
	//set zero
	memset(_nowHeight, 0, sizeof(_nowHeight));
	memset(_nowVelocity,0,sizeof(_nowVelocity));

	int _defaultTextureId;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &_defaultTextureId);
	const cocos2d::Size &_OpenGLView = Director::getInstance()->getOpenGLView()->getFrameSize();
	glGenTextures(1, &_imageBufferId);
	glBindTexture(GL_TEXTURE_2D, _imageBufferId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _OpenGLView.width, _OpenGLView.height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	_imageBuffer = new char[(int)(_OpenGLView.width*_OpenGLView.height)*sizeof(char)*4];

	glBindTexture(GL_TEXTURE_2D,_defaultTextureId);
}

WaterSimulate *WaterSimulate::create()
{
	WaterSimulate *_waterS = new WaterSimulate();
	_waterS->initWithSkybox();
	_waterS->autorelease();
	return _waterS;
}

void WaterSimulate::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4& transform, uint32_t flags)
{
	//calculate height field of water
	WaterMeshType  waterMesh[2] = {_nowHeight,_nowVelocity};
	WaterMeshType  srcMesh = waterMesh[_srcMeshIndex];
	_srcMeshIndex = (_srcMeshIndex + 1) & 0x1;
	WaterMeshType  dstMesh = waterMesh[_srcMeshIndex];
	//process
	for (int i = 1; i <__WATER_SQUARE_PORTRATE__ + 1; ++i)
	{
		for (int j = 1; j <  __WATER_SQUARE_LANDSCAPE__ +1; ++j)
		{
			const float _arroundValue = srcMesh[i - 1][j - 1] + srcMesh[i - 1][j] + srcMesh[i - 1][j + 1] + srcMesh[i][j - 1]
				+ srcMesh[i][j + 1] + srcMesh[i + 1][j - 1] + srcMesh[i + 1][j] + srcMesh[i + 1][j + 1];
			dstMesh[i][j] = _arroundValue * 0.25f - dstMesh[i][j];
			dstMesh[i][j] *= WaterCoeffValue;
		}
	}
	//begin map buffer
	int _defaultNormalId;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &_defaultNormalId);
	const unsigned _normalBufferId = _meshSquare->getNormal();
	glBindBuffer(GL_ARRAY_BUFFER, _normalBufferId);
#ifdef _WIN32
	WaterType nowNormal = (WaterType)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(GLVector3)*(__WATER_SQUARE_PORTRATE__ )*(__WATER_SQUARE_LANDSCAPE__),GL_MAP_WRITE_BIT);
#else
    WaterType nowNormal =(WaterType)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
#endif
	//build normal
	for (int i = 1; i < __WATER_SQUARE_PORTRATE__ +1; ++i)
	{
		for (int j = 1; j < __WATER_SQUARE_LANDSCAPE__ +1; ++j)
		{
			nowNormal[i - 1][j - 1] = GLVector3(dstMesh[i][j + 1] - dstMesh[i][j], dstMesh[i + 1][j] - dstMesh[i][j], dstMesh[i][j]);
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
//	memcpy(_nowHeight, _outHeight, sizeof(_outHeight));
	glBindBuffer(GL_ARRAY_BUFFER, _defaultNormalId);
	//rain simulate
#ifdef __TIME_TEST__
	_deltaTime += 0.033f;
	if (_deltaTime>0.3f)
	{
		_deltaTime = 0.0f;
		const float idx =  rand_0_1()*__WATER_SQUARE_LANDSCAPE__ ;
		const float idy =  rand_0_1()*__WATER_SQUARE_PORTRATE__;
#else
	if (_isFingerTouch)
	{
		_isFingerTouch = false;
		const float idx = _fingerPoint.x ;
		const float idy = _fingerPoint.y;
#endif
		const float _radius = 16.0f;
		for (int i = 1; i < __WATER_SQUARE_PORTRATE__ + 1; ++i)
		{
			for (int j = 1; j < __WATER_SQUARE_LANDSCAPE__ +1; ++j)
			{
				const float _nowRadius = sqrt((i-idy)*(i-idy)+(j-idx)*(j-idx));
				dstMesh[i][j] -= 128.0f / (_nowRadius *_nowRadius +1.0f);
				//_nowHeight[i][j] -= exp(-_nowRadius/_radius*6.0f - 1.5f);
			}
		}
	}
	cocos2d::Node::draw(renderer,transform,flags);
}

void WaterSimulate::setTouchPoint(cocos2d::Vec2 &_point)
{
	//背景图已经适配到了和屏幕的宽度一样大,所以可以完全按照屏幕的尺寸操作
	const cocos2d::Size &_size = cocos2d::Director::getInstance()->getWinSize();
	//计算点离中心点的距离
	cocos2d::Vec2 _offsetPoint = _point - Vec2(_size.width/2,_size.height/2);
	//计算缩放值
	const float dx = _offsetPoint.x *2.0f / _size.width;
	const float dy = _offsetPoint.y *2.0f / _size.height;
	_fingerPoint.x = (dx *0.5f+0.5f) * __WATER_SQUARE_LANDSCAPE__;
	//note ,the adapt information of screen
	_fingerPoint.y = (dy*0.5f+0.5f)*__WATER_SQUARE_PORTRATE__;
	_isFingerTouch = true;
}

void WaterSimulate::visit(cocos2d::Renderer *renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags)
{
	if (!_visible)
		return;
	_drawWaterMeshCommand.init(_globalZOrder);
	_drawWaterMeshCommand.func = CC_CALLBACK_0(WaterSimulate::drawWaterMesh,this,parentTransform,parentFlags);
	renderer->addCommand(&_drawWaterMeshCommand);

	cocos2d::Node::visit(renderer, parentTransform, parentFlags);
}

void WaterSimulate::drawWaterMesh(const cocos2d::Mat4& transform, uint32_t flags)
{
	int _defaultTextureId,_defaultVertexId;
	glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &_defaultTextureId);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &_defaultVertexId);
	//need calculate model matrix,self position
	Matrix _modelMatrix;
	_glProgram->use();

	Vec3  _freshnelParam(0.12f, 0.88f, 2.0f);
	const cocos2d::Size &_size = Director::getInstance()->getWinSize();
	const float zeye = _size.height / 1.1566f;
	//计算投影，视图矩阵
	GLVector3  _eyePosition(0.0f, 0.0f, zeye);
//	Vec4  _waterColor(0.4f, 0.48f, 0.97f, 1.0f);
//	float   _waterRatio = 1.0f / 1.33f;
	Matrix _viewProj;
    GLVector3 _viewPosition(0.0f, 0.0f,0.0f);
    GLVector3 _upVector(0.0f,1.0f,0.0f);
	_viewProj.lookAt(_eyePosition, _viewPosition, _upVector);
	
	_viewProj.perspective(60.0f, _size.width / _size.height, 1.0f, zeye+_size.height/2.0f /*1000.0f*/);

	glUniformMatrix4fv(_modelMatrixLoc,1,GL_FALSE,_modelMatrix.pointer());
	glUniformMatrix4fv(_projMatrixLoc, 1, GL_FALSE, _viewProj.pointer());

	const cocos2d::Size &OpenGLSize = Director::getInstance()->getOpenGLView()->getFrameSize();
	//read pixel from OpenGL back color buffer to now image texture
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, OpenGLSize.width, OpenGLSize.height, GL_RGBA, GL_UNSIGNED_BYTE, _imageBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _imageBufferId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, OpenGLSize.width, OpenGLSize.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _imageBuffer);
	_glProgram->setUniformLocationWith1i(_skyboxLoc, 0);

	glUniform3fv(_freshnelParamLoc,1,&_freshnelParam.x);
	//this OpenGL State must be Opened
	bool _cullFace = glIsEnabled(GL_CULL_FACE);
	if (!_cullFace)
		glEnable(GL_CULL_FACE);
	_meshSquare->drawMeshSquare(_positionLoc, _normalLoc,_fragCoordLoc);
	//restore all OpenGL Context State
	if (!_cullFace)
		glDisable(GL_CULL_FACE);
	glReadBuffer(GL_NONE);
	glBindTexture(GL_TEXTURE_2D, _defaultTextureId);
}
