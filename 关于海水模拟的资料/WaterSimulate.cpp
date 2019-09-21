/*
*Water Simulate
*2017/2/23
*2018/8/25
*与之前的实现不同,本版本的C++程序要求机器必须支持半浮点数,浮点纹理,RG类型纹理,顶点纹理
*@Author:xiaoxiong
*/
#include "WaterSimulate.h"
static char *_static_WaterVSShader = "uniform 	mat4 u_modelMatrix;\n"
"uniform vec2        u_textureStep;\n"
"attribute 	vec4 a_position;\n"
"attribute   vec2 a_fragCoord;\n"
"varying 	vec2 v_fragCoord;\n"
"void main()\n"
"{\n"
"	vec4 _position = u_modelMatrix * a_position;\n"
"	gl_Position = CC_PMatrix * _position;\n"
"	vec2 offset_2 = texture2D(CC_Texture0,a_fragCoord).xy * u_textureStep;\n"
"	v_fragCoord = a_fragCoord + offset_2;\n"
"}\n";
//FS Shader
static char *_static_WaterFSShader = "uniform sampler2D	u_skybox;"
"uniform vec3      	u_freshnelParam;"
"varying vec2 		v_fragCoord;"
"void   main()"
"{"
"	//float _fresnelFactor = u_freshnelParam.x + u_freshnelParam.y * pow(1.0 - dot(vec3(0.0, 0.0, 1.0), _normal), u_freshnelParam.z);\n"
"	gl_FragColor = texture2D(u_skybox, v_fragCoord);\n"
"	//gl_FragColor.a = gl_FragColor.a;// * 0.7 + 0.3 * _fresnelFactor;\n"
"}";
//顶点运算Shader,公共单元
static const char *static_Water_Identity_Vert = "attribute vec4	 a_position;\n"
"attribute	vec2	a_fragCoord;\n"
"varying  vec2  v_fragCoord;\n"
"void	main()\n"
"{"
"		gl_Position = a_position;\n"
"		v_fragCoord = a_fragCoord;\n"
"}";
static const char *static_Water_Height_Frag = "uniform  vec4	u_WaterParam;\n"
"uniform vec2 u_PixelSteps[4];\n"
"uniform vec2 u_MeshSize;\n"
"varying  vec2  v_fragCoord;\n"
"void  main()\n"
"{"
"		float  velocity = 0.0;\n"
"		for(int k=0;k<4;++k)\n"
"			velocity += texture2D(CC_Texture0,v_fragCoord + u_PixelSteps[k]).r;\n"
"		vec2  field_2 = texture2D(CC_Texture0,v_fragCoord).rg;\n"
"		field_2.g = velocity * 0.5 -field_2.g;\n"
"		field_2.g *= u_WaterParam.z;\n"
"		if(u_WaterParam.w > 0.0)\n"
"		{\n"
"			vec2  diff_vec2 = u_WaterParam.xy - v_fragCoord * u_MeshSize;\n"
"			field_2.g -= 256.0/(dot(diff_vec2,diff_vec2) + 1.0);\n"
"		}\n"
"		gl_FragColor = vec4(field_2.g,field_2.r,0.0,1.0);\n"
"}";
//计算偏移场的片元shader
static const char *static_Water_Normal_Frag = "uniform vec2 u_PixelStep;\n"
"varying  vec2  v_fragCoord;\n"
"void  main()\n"
"{\n"
"	float height = texture2D(CC_Texture0,v_fragCoord).r;\n"
"	float height_2 = texture2D(CC_Texture0,v_fragCoord + vec2(u_PixelStep.x,0.0)).r;\n"
"	float height_4 = texture2D(CC_Texture0,v_fragCoord + vec2(0.0,u_PixelStep.y)).r;\n"
"  gl_FragColor = vec4(height_2 - height,height_4 - height,0.0,1.0);\n"
"}\n";
//const float _nowRadius = sqrt((i - idy)*(i - idy) + (j - idx)*(j - idx));
//dstMesh[i][j] -= 256.0f / (_nowRadius *_nowRadius + 1.0f);
using namespace cocos2d;
static const int static_MeshSize = 128;
//coefficient of wave flow out 
const float WaterCoeffValue = 0.97f;
const int  VertexAttribFloatCount = 5;//每个顶点的浮点数的数目
MeshSquare::MeshSquare()
{
	_vertexBufferId = 0;
	_indexBufferId = 0;
	_countOfIndex = 0;
	_countOfVertex = 0;
}

MeshSquare::~MeshSquare()
{
	glDeleteBuffers(1, &_vertexBufferId);
	glDeleteBuffers(1, &_indexBufferId);
	_vertexBufferId = 0;
	_indexBufferId = 0;
}

void MeshSquare::initWithMeshSquare(float width, float height, int xgrid, int ygrid, float fragCoord)
{
	assert(xgrid>0 && ygrid>0);
	int  meshSizeX = xgrid + 1;
	int  meshSizeY = ygrid + 1;
	_countOfVertex = meshSizeX*meshSizeY;
	float *Vertex = new float[_countOfVertex*VertexAttribFloatCount];
	for (int j = 0; j < ygrid + 1; ++j)
	{
		const float factor = 2.0f*j / ygrid - 1.0f;
		const float y = height*factor;
		float  *nowVertex = Vertex + j*meshSizeX*VertexAttribFloatCount;
		for (int i = 0; i < xgrid + 1; ++i)
		{
			const int index = i * VertexAttribFloatCount;
			nowVertex[index] = width*(2.0f*i / xgrid - 1.0f);
			nowVertex[index + 1] = y;
			nowVertex[index + 2] = 0.0f;
			//除非VertexAttribFloatCount>=5才能开启下面的属性
			nowVertex[index + 3] = 1.0f * i / xgrid *fragCoord;
			nowVertex[index + 4] = 1.0f * j / ygrid*fragCoord;
		}
	}
	//generate index data
	_countOfIndex = meshSizeX*ygrid * 2;
	short *indexVertex = new short[_countOfIndex];
	bool  counter_clock = true;
	for (int y = 0,vertex_index=0; y < ygrid; ++y)
	{
		int   y_row_index = y * meshSizeX;
		int   y_row_m1 = y_row_index + meshSizeX;
		if (counter_clock)
		{
			for (int x = 0; x < meshSizeX; ++x)
			{
				indexVertex[vertex_index] = y_row_m1 + x;
				indexVertex[vertex_index + 1] = y_row_index + x;

				vertex_index += 2;
			}
		}
		else
		{
			for (int x = meshSizeX-1; x >=0 ; --x)
			{
				indexVertex[vertex_index] = y_row_index + x;
				indexVertex[vertex_index + 1] = y_row_m1 + x;

				vertex_index += 2;
			}
		}
		counter_clock = !counter_clock;
	}
	GL::bindVAO(0);
	//generate vertex and frag coord
	glGenBuffers(1, &_vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*_countOfVertex * VertexAttribFloatCount, Vertex, GL_STATIC_DRAW);
	//generate index 
	glGenBuffers(1, &_indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(short)*_countOfIndex, indexVertex, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	delete[] indexVertex;
	delete[] Vertex;
}

void MeshSquare::drawMeshSquare(int posLoc, int normalLoc, int fragCoordLoc)
{
	//bind vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
	glEnableVertexAttribArray(posLoc);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float)*VertexAttribFloatCount, NULL);
	glEnableVertexAttribArray(fragCoordLoc);
	glVertexAttribPointer(fragCoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(float)*VertexAttribFloatCount, (void*)(sizeof(float) * 3));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
	glDrawElements(GL_TRIANGLE_STRIP, _countOfIndex, GL_UNSIGNED_SHORT, NULL);
}
//#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
void  MeshSquare::recreate(float width, float height, int xgrid, int ygrid, float fragCoord)
{
	initWithMeshSquare(width, height, xgrid, ygrid, fragCoord);
}
//#endif
MeshSquare *MeshSquare::createMeshSquare(float width, float height, int xgrid, int ygrid, float fragCoord)
{
	MeshSquare *_meshSquare = new MeshSquare();
	_meshSquare->initWithMeshSquare(width, height, xgrid, ygrid, fragCoord);
	return _meshSquare;
}

//////////////////////////////////Class of Water Simulate/////////////////////////////////////////////////////////////////////
WaterSimulate::WaterSimulate():
 _framebufferIndex(0)
, _identityVertexArray(0)
,_identityVertexBuffer(0)
,_heightProgram(nullptr)
,_normalProgram(nullptr)
,_meshSizeVec2(static_MeshSize, static_MeshSize)
,_waterParam(0,0,WaterCoeffValue,0)
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	,_backToForegroundListener(nullptr)
#endif
{
	_meshSquare = nullptr;
	_isFingerTouch = false;
	_renderFrame = nullptr;

	_framebufferIds[0] = _framebufferIds[1] = _framebufferIds[2] = 0;
	_textureIds[0] = _textureIds[1] = 0;

	float interval = 1.0f / static_MeshSize;
	_pixelSteps[0] = Vec2(interval,0.0f);
	_pixelSteps[1] = Vec2(-interval,0.0f);
	_pixelSteps[2] = Vec2(0.0f, interval);
	_pixelSteps[3] = Vec2(0.0f,-interval);
}

WaterSimulate::~WaterSimulate()
{
	_meshSquare->release();
	_glProgram->release();
	if (_renderFrame)//in some platform,it will be a null value
		_renderFrame->release();
	_meshSquare = nullptr;
	_glProgram = nullptr;
	_renderFrame = nullptr;
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	if (_backToForegroundListener)
		Director::getInstance()->getEventDispatcher()->removeEventListener(_backToForegroundListener);
#endif
}

void WaterSimulate::initWithSkybox()
{
	const cocos2d::Size &size = _director->getWinSize();
	//do some adapt to screen
	_meshSquare = MeshSquare::createMeshSquare(size.width *0.5f, size.height*0.5f, static_MeshSize - 1, static_MeshSize - 1, 1.0f);

	const cocos2d::Size &OpenGLSize = _director->getOpenGLView()->getFrameSize();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	GLView *glView = _director->getOpenGLView();
	const cocos2d::Size pcFrameSize = OpenGLSize * glView->getFrameZoomFactor() * glView->getRetinaFactor();
	_renderFrame = RenderFramebuffer::create(pcFrameSize);
#else
	_renderFrame = RenderFramebuffer::create(OpenGLSize);
#endif
	_textureStep = cocos2d::Vec2(1.0f / OpenGLSize.width, 1.0f / OpenGLSize.height);

	const float zeye = size.height / 1.1566f;
	//计算投影，视图矩阵
	GLVector3  _eyePosition(0.0f, 0.0f, zeye);
	GLVector3 _viewPosition(0.0f, 0.0f, 0.0f);
	GLVector3 _upVector(0.0f, 1.0f, 0.0f);
	_viewProjMatrix.lookAt(_eyePosition, _viewPosition, _upVector);

	_viewProjMatrix.perspective(60.0f, size.width / size.height, 1.0f, zeye + size.height / 2.0f);
	_freshnelParam = GLVector3(0.12f, 0.88f, 2.0f);
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	//
	_backToForegroundListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED, CC_CALLBACK_1(WaterSimulate::recreate, this));
	_director->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_backToForegroundListener, this);
#endif
	initGLProgram();
	initGLBuffer();
	initFramebuffer();
}
//初始化帧缓冲去对象
void WaterSimulate::initFramebuffer()
{
	int default_framebuffer, default_texture;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING,&default_framebuffer);
	glGetIntegerv(GL_TEXTURE_BINDING_2D,&default_texture);

	const Configuration *config = Configuration::getInstance();
	glGenFramebuffers(3,_framebufferIds);
   /*前两个帧缓冲去对象用来生成高度场,后面的一个用来生成偏移场*/
	glGenTextures(3, _textureIds);
	for (int k = 0; k < 3; ++k)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,_framebufferIds[k]);
		glBindTexture(GL_TEXTURE_2D,_textureIds[k]);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
#if defined(GL_ARB_texture_storage) || defined(GL_EXT_texture_storage)
		glTexStorage2D(GL_TEXTURE_2D,1, GL_RG16F, static_MeshSize, static_MeshSize);
#else
		glTexImage2D(GL_TEXTURE_2D,0, 0x822F, static_MeshSize, static_MeshSize,0, 0x8227, 0x140B,nullptr);
#endif
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,_textureIds[k],0);
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	glBindTexture(GL_TEXTURE_2D,default_texture);
	glBindFramebuffer(GL_FRAMEBUFFER,default_framebuffer);
}

void  WaterSimulate::initGLProgram()
{
	//渲染Shader
	_glProgram = GLProgram::createWithByteArrays(_static_WaterVSShader, _static_WaterFSShader);
	_glProgram->retain();
	_projMatrixLoc = _glProgram->getUniformLocation("CC_PMatrix");
	_modelMatrixLoc = _glProgram->getUniformLocation("u_modelMatrix");
	_skyboxLoc = _glProgram->getUniformLocation("u_skybox");
	_textureLoc = _glProgram->getUniformLocation("CC_Texture0");
	_freshnelParamLoc = _glProgram->getUniformLocation("u_freshnelParam");
	_textureStepLoc = _glProgram->getUniformLocation("u_textureStep");

	_positionLoc = _glProgram->getAttribLocation("a_position");
	_fragCoordLoc = _glProgram->getAttribLocation("a_fragCoord");
	//高度场Shader
	_heightProgram = GLProgram::createWithByteArrays(static_Water_Identity_Vert, static_Water_Height_Frag);
	_heightProgram->retain();
	_heightAttribPositionLoc = _heightProgram->getAttribLocation("a_position");
	_heightAttribFragCoordLoc = _heightProgram->getAttribLocation("a_fragCoord");
	_heightTextureLoc = _heightProgram->getUniformLocation("CC_Texture0");
	_heightWaterParamLoc = _heightProgram->getUniformLocation("u_WaterParam");
	_heightPixelStepsLoc = _heightProgram->getUniformLocation("u_PixelSteps");
	_heightMeshSizeLoc = _heightProgram->getUniformLocation("u_MeshSize");
	//法向量场Shader
	_normalProgram = GLProgram::createWithByteArrays(static_Water_Identity_Vert, static_Water_Normal_Frag);
	_normalProgram->retain();
	_normalAttribPositionLoc = _normalProgram->getAttribLocation("a_position");
	_normalAttribFragCoordLoc = _normalProgram->getAttribLocation("a_fragCoord");
	_normalPixelStepLoc = _normalProgram->getUniformLocation("u_PixelStep");
	_normalTextureLoc = _normalProgram->getUniformLocation("CC_Texture0");
}

void  WaterSimulate::initGLBuffer()
{
	GL::bindVAO(0);
	const float vertex_data[] = {
		-1.0f,1.0f,0.0f,0.0f,1.0f,//0
		-1.0f,-1.0f,0.0f,0.0f,0.0f,//1
		1.0f,1.0f,0.0f,1.0f,1.0f,//2
		1.0f,-1.0f,0.0f,1.0f,0.0f,//3
	};
	glGenBuffers(1,&_identityVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER,_identityVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER,sizeof(vertex_data),vertex_data,GL_STATIC_DRAW);

	glEnableVertexAttribArray(_heightAttribPositionLoc);
	glVertexAttribPointer(_heightAttribPositionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);

	glEnableVertexAttribArray(_heightAttribFragCoordLoc);
	glVertexAttribPointer(_heightAttribFragCoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 5));

	glBindBuffer(GL_ARRAY_BUFFER,0);
}

//#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
void WaterSimulate::recreate(cocos2d::EventCustom *recreateEvent)
{
	const cocos2d::Size &_size = _director->getWinSize();
	//do some adapt to screen
	_meshSquare->recreate(_size.width / 2.0f, _size.height / 2.0f, static_MeshSize - 1, static_MeshSize - 1, 1.0f);
	_glProgram->reset();
	_glProgram->initWithByteArrays(_static_WaterVSShader, _static_WaterFSShader);

	_projMatrixLoc = _glProgram->getUniformLocation("CC_PMatrix");
	_modelMatrixLoc = _glProgram->getUniformLocation("u_modelMatrix");
	_skyboxLoc = _glProgram->getUniformLocation("u_skybox");
	_freshnelParamLoc = _glProgram->getUniformLocation("u_freshnelParam");
	_textureStepLoc = _glProgram->getUniformLocation("u_textureStep");

	_positionLoc = _glProgram->getAttribLocation("a_position");
	_fragCoordLoc = _glProgram->getAttribLocation("a_fragCoord");
}
//#endif
WaterSimulate *WaterSimulate::create()
{
	WaterSimulate *_waterS = new WaterSimulate();
	_waterS->initWithSkybox();
	_waterS->autorelease();
	return _waterS;
}

bool WaterSimulate::isMachineSupport()
{
	const Configuration *config = Configuration::getInstance();

	bool  support = config->supportVertexTexture();//是否支持顶点纹理
	support &= config->supportHalfFloatTexture();//是否支持半浮点纹理
	support &= config->supportRGTexture();//是否支持RG类型纹理

	return support;
}

int   WaterSimulate::getVersion()
{
	return 1;
}

void WaterSimulate::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4& transform, uint32_t flags)
{
	//calculate height field of water
	if (_renderFrame)//in some platform ,it will be invalide
		_renderFrame->save();
	cocos2d::Node::draw(renderer, transform, flags);
}

void WaterSimulate::setTouchPoint(cocos2d::Vec2 &point)
{
	//背景图已经适配到了和屏幕的宽度一样大,所以可以完全按照屏幕的尺寸操作
	const cocos2d::Size &winSize = _director->getWinSize();
	_waterParam.x = point.x / winSize.width * static_MeshSize;
	_waterParam.y = point.y / winSize.height*static_MeshSize;
	_waterParam.w = 1.0f;
}

void WaterSimulate::visit(cocos2d::Renderer *renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags)
{
	if (!_visible)
		return;
	if (_renderFrame)
	{
		_drawWaterMeshCommand.init(_globalZOrder);
		_drawWaterMeshCommand.func = CC_CALLBACK_0(WaterSimulate::drawWaterMesh, this, parentTransform, parentFlags);
		renderer->addCommand(&_drawWaterMeshCommand);
	}

	cocos2d::Node::visit(renderer, parentTransform, parentFlags);
}

void WaterSimulate::drawHeightNormalField()
{
	int fb_idx_0 = _framebufferIndex;
	int fb_idx_1 = (_framebufferIndex + 1) & 0x1;
	_framebufferIndex = fb_idx_1;
	const Configuration  *config = Configuration::getInstance();
	//视口变换
	int   viewport[4];
	GL::queryViewport(viewport);
	GL::setViewPort(0, 0, static_MeshSize, static_MeshSize);
	glBindFramebuffer(GL_FRAMEBUFFER,_framebufferIds[fb_idx_1]);
	_heightProgram->use();
	//if (config->supportsShareableVAO())
	//	GL::bindVAO(_identityVertexArray);
	//else
	{
		glBindBuffer(GL_ARRAY_BUFFER,_identityVertexBuffer);

		glEnableVertexAttribArray(_heightAttribPositionLoc);
		glVertexAttribPointer(_heightAttribPositionLoc,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

		glEnableVertexAttribArray(_heightAttribFragCoordLoc);
		glVertexAttribPointer(_heightAttribFragCoordLoc,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)(sizeof(float)*3));
	}
	glUniform2fv(_heightPixelStepsLoc,4, static_cast<float*>(&_pixelSteps->x));
	glUniform4fv(_heightWaterParamLoc,1,&_waterParam.x);
	glUniform2f(_heightMeshSizeLoc, _meshSizeVec2.x, _meshSizeVec2.y);
	//关闭深度测试
	bool depthTest = GL::isDepthTest();
	GL::setDepthTest(false);
	//glColorMask(1,1,0,0);

	GL::bindTexture2DN(0, _textureIds[fb_idx_0]);
	glUniform1i(_heightTextureLoc,0);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	/*
	  *生成偏移场
	 */
	_normalProgram->use();
	glBindFramebuffer(GL_FRAMEBUFFER,_framebufferIds[2]);
	GL::bindTexture2DN(0, _textureIds[fb_idx_1]);
	glUniform1i(_normalTextureLoc,0);
	glUniform2f(_normalPixelStepLoc, 1.0f / static_MeshSize, 1.0f / static_MeshSize);

	glEnableVertexAttribArray(_normalAttribPositionLoc);
	glVertexAttribPointer(_normalAttribPositionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);

	glEnableVertexAttribArray(_normalAttribFragCoordLoc);
	glVertexAttribPointer(_normalAttribFragCoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	//glColorMask(1,1,1,1);
	GL::setDepthTest(depthTest);
	GL::setViewPort(viewport[0], viewport[1], viewport[2], viewport[3]);
	_waterParam.w = 0.0f;
}

void WaterSimulate::drawWaterMesh(const cocos2d::Mat4& transform, uint32_t flags)
{
	//int _defaultTextureId, _defaultVertexId;
	GL::bindVAO(0);
	drawHeightNormalField();

	_renderFrame->restore();
	//need calculate model matrix,self position
	Matrix _modelMatrix;
	_glProgram->use();

	glUniformMatrix4fv(_modelMatrixLoc, 1, GL_FALSE, _modelMatrix.pointer());
	glUniformMatrix4fv(_projMatrixLoc, 1, GL_FALSE, _viewProjMatrix.pointer());

	GL::bindTexture2D( _renderFrame->getColorBuffer());
	_glProgram->setUniformLocationWith1i(_skyboxLoc, 0);
	GL::bindTexture2DN(1,_textureIds[2]);
	_glProgram->setUniformLocationWith1i(_textureLoc,1);

	//glUniform3fv(_freshnelParamLoc, 1, &_freshnelParam.x);
	glUniform2f(_textureStepLoc, _textureStep.x, _textureStep.y);
	//this OpenGL State must be Opened
	bool _cullFace = GL::isCullFace();
	GL::setCullFace(true);
	cocos2d::GL::blendFunc(GL_ONE, GL_ZERO);
	_meshSquare->drawMeshSquare(_positionLoc, -1, _fragCoordLoc);
	//restore all OpenGL Context State
	GL::setCullFace(_cullFace);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
