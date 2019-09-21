/*
  *水渲染GPU实现
  *2017-8-3
  *@Author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "engine/GLContext.h"
#include "engine/GLCacheManager.h"
#include "engine/event/EventManager.h"

#include "WaterGPU.h"
#include<assert.h>
__US_GLK__;

#define _KEY_MASK_W_  0x01
#define _KEY_MASK_S_   0x02
#define _KEY_MASK_A_  0x04
#define _KEY_MASK_D_  0x08
static const float MeshSize = 128.0f;
static const float PhotonSize = 512;
WaterGPU::WaterGPU() :
	_waterHeightShader(nullptr)
	, _waterNormalShader(nullptr)
	, _waterShader(nullptr)
	, _poolShader(nullptr)
	, _heightTexture0(nullptr)
	, _heightTexture1(nullptr)
	, _camera(nullptr)
	, _poolMesh(nullptr)
	, _waterMesh(nullptr)
	, _texCubeMap(nullptr)
	, _deltaTime(0)
	, _touchEventListener(nullptr)
	,_keyEventListener(nullptr)
	, _mouseEventListener(nullptr)
	, _keyMask(0)
{

}

WaterGPU::~WaterGPU()
{
	_waterHeightShader->release();
	_waterNormalShader->release();
	_waterShader->release();
	_poolShader->release();
	_heightTexture0->release();
	_heightTexture1->release();
	_camera->release();
	_poolMesh->release();
	_waterMesh->release();
	_texCubeMap->release();

	EventManager::getInstance()->removeListener(_touchEventListener);
	_touchEventListener->release();

	EventManager::getInstance()->removeListener(_keyEventListener);
	_keyEventListener->release();

	EventManager::getInstance()->removeListener(_mouseEventListener);
	_mouseEventListener->release();
}

WaterGPU * WaterGPU::create()
{
	WaterGPU * water = new WaterGPU();
	water->init();
	return water;
}

void		WaterGPU::init()
{
	//初始化shader
	_waterHeightShader = WaterHeightShader::create("shader/WaterGPU/WaterHeight_VS.glsl", "shader/WaterGPU/WaterHeight_FS.glsl");
	_waterNormalShader = WaterNormalShader::create("shader/WaterGPU/WaterNormal_VS.glsl","shader/WaterGPU/WaterNormal_FS.glsl");
	_waterShader = WaterShader::create("shader/WaterGPU/Water_VS.glsl", "shader/WaterGPU/Water_FS.glsl");
	_poolShader = PoolShader::create("shader/WaterGPU/Pool_VS.glsl","shader/WaterGPU/Pool_FS.glsl");
	/*
	  *天空盒
	 */
	_poolMesh = Skybox::createWithScale(MeshSize);
	_waterMesh = Mesh::createWithIntensity(MeshSize , MeshSize, MeshSize, 1.0f,Mesh::MeshType::MeshType_XOZ);
	const char *cubeMapFiles[6] = {
			"tga/water/pool/right.tga",//+X
			"tga/water/pool/left.tga",
			"tga/water/pool/top.tga",
			"tga/water/pool/bottom.tga",
			"tga/water/pool/front.tga",
			"tga/water/pool/back.tga"
	};
	_texCubeMap = GLCubeMap::createWithFiles(cubeMapFiles);
	/*
	  *立方体的各个法线向量
	 */
	_texCubeNormals[0] = GLVector3(-1.0f,0.0f,0.0f);//+X
	_texCubeNormals[1] = GLVector3(1.0f,0.0f,0.0f);//-X
	_texCubeNormals[2] = GLVector3(0.0f,-1.0f,0.0f);//+Y
	_texCubeNormals[3] = GLVector3(0.0f,1.0f,0.0f);//-Y
	_texCubeNormals[4] = GLVector3(0.0f,0.0f,-1.0f);//+Z
	_texCubeNormals[5] = GLVector3(0.0f,0.0f,1.0f);//-Z
	//立方体的半高度
	_halfCubeHeight = MeshSize;
	//Render Texture
	Size texSize(MeshSize,MeshSize);
	int   format[2] = {GL_RGBA32F,GL_DEPTH_COMPONENT32F};
	int   format2[2] = {GL_RGBA32F,GL_DEPTH_COMPONENT32F };
	_heightTexture0 = RenderTexture::createRenderTexture(texSize,RenderTexture::RenderType::ColorBuffer,format);
	_heightTexture1 = RenderTexture::createRenderTexture(texSize, RenderTexture::RenderType::ColorBuffer, format);
	_normalTexture = RenderTexture::createRenderTexture(texSize, RenderTexture::RenderType::ColorBuffer, format2);

	_heightTexture0->setClearBuffer(false);
	_heightTexture1->setClearBuffer(false);
	_normalTexture->setClearBuffer(false);
	//初始化摄像机
	initCamera(GLVector3(0.0f,MeshSize/1.f,MeshSize/2),GLVector3(0.0f,0.0f,-MeshSize/2.0f));
	//初始化关于水的参数
	initWaterParam();
	//
	initPhotonParam();
	_normalProgram = GLProgram::createWithFile("shader/WaterGPU/Normal_VS.glsl", "shader/WaterGPU/Normal_FS.glsl");
	/*
	  *注册监听事件
	 */
	_touchEventListener = TouchEventListener::createTouchListener(this, glk_touch_selector(WaterGPU::onTouchBegan),glk_move_selector(WaterGPU::onTouchMoved),glk_release_selector(WaterGPU::onTouchEnded));
	EventManager::getInstance()->addTouchEventListener(_touchEventListener, 0);

	_keyEventListener = KeyEventListener::createKeyEventListener(this, glk_key_press_selector(WaterGPU::onKeyPressed),glk_key_release_selector(WaterGPU::onKeyReleased));
	EventManager::getInstance()->addKeyEventListener(_keyEventListener, 0);
	/*
	  *鼠标
	 */
	_mouseEventListener = MouseEventListener::createMouseEventListener(this,glk_mouse_press_selector(WaterGPU::onMouseClick),glk_mouse_move_selector(WaterGPU::onMouseMoved),glk_mouse_release_selector(WaterGPU::onMouseEnded));
	EventManager::getInstance()->addMouseEventListener(_mouseEventListener, 0);
}

void		WaterGPU::initCamera(const glk::GLVector3 &eyePosition, const glk::GLVector3 &targetPosition)
{
	_camera = Camera::createCamera(eyePosition, targetPosition, GLVector3(0.0f,1.0f,0.0f));
	auto &winSize = GLContext::getInstance()->getWinSize();
	_camera->setPerspective(60.0f, winSize.width/winSize.height,0.1f,500.0f);
}

void  WaterGPU::initWaterParam()
{
	//平面网格模型矩阵
	_waterModelMatrix.identity();
	_waterModelMatrix.translate(0.0f,0.0f,-MeshSize);
	//天空盒的平移矩阵
	_skyboxModelMatrix.identity();
	_skyboxModelMatrix.translate(0.0f, 0.0f, -MeshSize);

	_lightPosition = GLVector3(0,PhotonSize,0);// GLVector3(MeshSize / 2.0f, MeshSize, -MeshSize);
}

void WaterGPU::initPhotonParam()
{
	//生成帧缓冲区对象,绑定的时机在运行时
	glGenFramebuffers(1, &_frameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER,_frameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//生成着色器
	_photonProgram = GLProgram::createWithFile("shader/WaterGPU/Photon_VS.glsl", "shader/WaterGPU/Photon_FS.glsl");
	//设置相关的参数

	_photonBlurProgram = GLProgram::createWithFile("shader/WaterGPU/SkyboxBlur_VS.glsl", "shader/WaterGPU/SkyboxBlur_FS.glsl");
	//CubeMap Texture
	glGenTextures(2, _photonTextureCubeMap);
	for (int i = 0; i < 2; ++i)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, _photonTextureCubeMap[i]);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		for (int j = 0; j < 6; j++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_RGBA8, PhotonSize, PhotonSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
	//
	glGenTextures(1,&_photonTexture);
	glBindTexture(GL_TEXTURE_2D,_photonTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,PhotonSize*2,PhotonSize*3,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
	glBindTexture(GL_TEXTURE_2D,0);
	//计算XOZ平面的顶点缓冲区对象,其覆盖面积与立方体Mesh一样大小
	const float stepInterval = 1.0f / (PhotonSize-1.0f);
	const int    size = PhotonSize - 1;
	GLVector3     *VertexData = new GLVector3[PhotonSize*PhotonSize];
	int                  index = 0;
	for (int y = 0; y < PhotonSize; ++y)
	{
		float S = (1.0f  - stepInterval *y * 2.0f) * MeshSize;
		for (int x = 0; x < PhotonSize; ++x)
		{
			VertexData[index++] = GLVector3(MeshSize*(x*stepInterval*2.0f-1.0f),0.0f,S);
		}
	}
	//在渲染Caustic的时候,将会以画点的方式将其渲染出来
	glGenBuffers(1, &_photonVertexbufferId);
	glBindBuffer(GL_ARRAY_BUFFER,_photonVertexbufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLVector3)*PhotonSize*PhotonSize,VertexData,GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	delete[] VertexData;
	VertexData = nullptr;
	//像素坐标
	index = 0;
	GLVector2 *fragCoordData = new GLVector2[PhotonSize*PhotonSize];
	for (int y = 0; y < PhotonSize; ++y)
	{
		for (int x = 0; x < PhotonSize; ++x)
		{
			fragCoordData[index++] = GLVector2(x*stepInterval,y*stepInterval);
		}
	}
	glGenBuffers(1,&_photonFragCoordbufferId);
	glBindBuffer(GL_ARRAY_BUFFER,_photonFragCoordbufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLVector2)*PhotonSize*PhotonSize,fragCoordData,GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	delete[] fragCoordData;
	fragCoordData = nullptr;
	//////////////////////////
	//偏置矩阵,将[-1,1]缩放到[0-1]区间
	Matrix biaseScaleMatrix;
	biaseScaleMatrix.offset();
	//然后缩放到2d纹理中的左下角区域
	biaseScaleMatrix.scale(1.0f / 2.0f, 1.0f / 3.0f, 1.0f);
	/*
	*注意下面的矩阵的计算方式,稍后将会以详细的注释的形式出现
	*/
	Matrix translate0, translate1, translate2, translate3, translate4, translate5;
	translate0.translate(0.0f / 2.0f, 0.0f / 3, 0.0f);
	translate1.translate(1.0f / 2.0f, 0.0f / 3.0f, 0.0f);

	translate2.translate(0.0f / 2.0f, 1.0f / 3.0f, 0.0f);
	translate3.translate(1.0f / 2.0f, 1.0f / 3.0f, 0.0f);

	translate4.translate(0.0f / 2.0f, 2.0f / 3.0f, 0.0f);
	translate5.translate(1.0f / 2.0f, 2.0f / 3.0f, 0.0f);
	//缩放矩阵,将[-PhotonSize,PhotonSize]缩放到[-1,1]区间
	Matrix scaleMatrix;
	scaleMatrix.scale(1.0f / MeshSize, 1.0f / MeshSize, 1.0f / MeshSize);
	//将3d立方体贴图的位置转换到2d纹理坐标,先缩放/再进行镜像转换/再进行偏置缩放计算,再进行平移
	/*缩放矩阵的由来:在创建纹理的时候, 纹理的宽度和高度分别为2.0 * PCMR, 3.0 * PCMR
		*最后面的初等矩阵产生的变换结果为
		* 4 : <X, -Y, Z> * 5 : <-X, -Y, Z>
		* 2 : <X, Z, Y> * 3 : <X, -Z, -Y>
		* 0 : <-Z, -Y, X> * 1 : <Z, -Y, -X>
		*关于立方体贴图的坐标的计算方式,请参见3D游戏与计算机图形学中的数学方法美Eric.Lengyel.扫描版 P102
		*/
	Matrix    photonWorldToMatrix[6] = {
		scaleMatrix * Matrix(GLVector4(0.0f, 0.0f, 1.0f, 0.0f), GLVector4(0.0f, -1.0f, 0.0f, 0.0f), GLVector4(-1.0f, 0.0f, 0.0f, 0.0f), GLVector4(0.0f, 0.0f, 0.0f, 1.0f)) *biaseScaleMatrix * translate0,
		scaleMatrix * Matrix(GLVector4(0.0f, 0.0f, -1.0f, 0.0f), GLVector4(0.0f, -1.0f, 0.0f, 0.0f), GLVector4(1.0f, 0.0f, 0.0f, 0.0f), GLVector4(0.0f, 0.0f, 0.0f, 1.0f)) *biaseScaleMatrix * translate1,
		scaleMatrix * Matrix(GLVector4(1.0f, 0.0f, 0.0f, 0.0f), GLVector4(0.0f, 0.0f, 1.0f, 0.0f), GLVector4(0.0f, 1.0f, 0.0f, 0.0f), GLVector4(0.0f, 0.0f, 0.0f, 1.0f)) *biaseScaleMatrix * translate2,
		scaleMatrix * Matrix(GLVector4(1.0f, 0.0f, 0.0f, 0.0f), GLVector4(0.0f, 0.0f, -1.0f, 0.0f), GLVector4(0.0f, -1.0f, 0.0f, 0.0f), GLVector4(0.0f, 0.0f, 0.0f, 1.0f)) *biaseScaleMatrix * translate3,
		scaleMatrix * Matrix(GLVector4(1.0f, 0.0f, 0.0f, 0.0f), GLVector4(0.0f, -1.0f, 0.0f, 0.0f), GLVector4(0.0f, 0.0f, 1.0f, 0.0f), GLVector4(0.0f, 0.0f, 0.0f, 1.0f)) *biaseScaleMatrix * translate4,
		scaleMatrix * Matrix(GLVector4(-1.0f, 0.0f, 0.0f, 0.0f), GLVector4(0.0f, -1.0f, 0.0f, 0.0f), GLVector4(0.0f, 0.0f, -1.0f, 0.0f), GLVector4(0.0f, 0.0f, 0.0f, 1.0f)) *biaseScaleMatrix * translate5
	};
	_photonProgram->perform();
	int photonWorldToMatrixLoc = _photonProgram->getUniformLocation("g_PhotonsWorldToTextureMatrix");
	glUniformMatrix4fv(photonWorldToMatrixLoc, 6, GL_FALSE, photonWorldToMatrix[0].pointer());
	//关于各个立方体贴图的平面法线
	int cubeMaoNormalLoc = _photonProgram->getUniformLocation("g_CubeMapNormals");
	glUniform3fv(cubeMaoNormalLoc,6,&_texCubeNormals->x);
	int lightPositionLoc = _photonProgram->getUniformLocation("g_LightPosition");
	glUniform3fv(lightPositionLoc,1,&_lightPosition.x);
	int  halfCubeMapWidthLoc = _photonProgram->getUniformLocation("g_HalfCubeMapWidth");
	glUniform1f(halfCubeMapWidthLoc, MeshSize);
	int  waterHeightLoc = _photonProgram->getUniformLocation("g_WaterHeightMap");
	glUniform1i(waterHeightLoc,0);
	int waterNormalLoc = _photonProgram->getUniformLocation("g_WaterNormalMap");
	glUniform1i(waterNormalLoc,1);
	int photonTextureLoc = _photonProgram->getUniformLocation("g_PhotonMap");
	glUniform1i(photonTextureLoc, 2);
	_photonProgram->disable();
	/*
	  *像素缓冲区对象
	 */
	glGenBuffers(1, &_pixelbufferId);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, _pixelbufferId);
	glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(int)*PhotonSize*PhotonSize,nullptr,GL_DYNAMIC_COPY);
	glBindBuffer(GL_PIXEL_PACK_BUFFER,0);
	/*
	  *标准立方体贴图
	  *纹理坐标的走向,这里我们采用GLCacheManager::loadIdentityVertex中的顶点走向,并修正了相关的坐标
	 */
	const GLVector3 CubeMapFragCoordData[24] = {
		GLVector3(1.0f,  -1.0f,  1.0f), GLVector3(1.0f,  1.0f, 1.0f), GLVector3(1.0f, -1.0f,  -1.0f), GLVector3(1.0f, 1.0f, -1.0f),//+X
		//GLVector3(1.0f,  1.0f,  -1.0f), GLVector3(1.0f,  -1.0f, -1.0f), GLVector3(1.0f, 1.0f,  1.0f), GLVector3(1.0f, -1.0f, 1.0f),//+X
		GLVector3(-1.0f,  -1.0f, -1.0f), GLVector3(1.0f,  1.0f,  -1.0f),  GLVector3(-1.0f, -1.0f, 1.0f),GLVector3(1.0f, 1.0f,  -1.0f),//-X
		GLVector3(1.0f, 1.0f,  1.0f), GLVector3(-1.0f, 1.0f,  -1.0f), GLVector3(1.0f, 1.0f, 1.0f),GLVector3(1.0f, 1.0f, -1.0f), //+Y
		GLVector3(-1.0f,  -1.0f, -1.0f), GLVector3(-1.0f,  -1.0f, 1.0f),  GLVector3(1.0f,  -1.0f,  -1.0f),GLVector3(1.0f,  -1.0f,  1.0f),//-Y
		GLVector3(-1.0f,  -1.0f, 1.0f), GLVector3(-1.0f,  1.0f, 1.0f),  GLVector3(1.0f, -1.0f, 1.0f),GLVector3(1.0f, 1.0f, 1.0f),//+Z
		GLVector3(1.0f,  -1.0f,  -1.0f), GLVector3(1.0f,  1.0f,  -1.0f),  GLVector3(-1.0f, -1.0f,  -1.0f),GLVector3(-1.0f, 1.0f,  -1.0f)//-Z
	};
	glGenBuffers(1, &_cubeMapfragCoordVertex);
	glBindBuffer(GL_ARRAY_BUFFER,_cubeMapfragCoordVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeMapFragCoordData), CubeMapFragCoordData,GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);

	const GLVector3 CubeMapVertex[4] = {
		GLVector3(-1,-1,0),GLVector3(1,-1,0),
		GLVector3(1,1,0),GLVector3(-1,1,0)
	};
	glGenBuffers(1, &_cubeMapVertex);
	glBindBuffer(GL_ARRAY_BUFFER, _cubeMapVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeMapVertex), CubeMapVertex,GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	_skyboxVertex = Skybox::createWithScale(1.0f);
}

void  WaterGPU::drawWaterHeightTexture()
{
	//绑定帧缓冲区对象,最后产生数值输出的一定是_heightTexture0
	_heightTexture0->activeFramebuffer();
	_waterHeightShader->perform();

	int identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER,identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5,(void*)(sizeof(float)*3));

	_waterHeightShader->setBaseMap(_heightTexture1->getColorBuffer(), 0);
	_waterHeightShader->setMeshSize(GLVector2(MeshSize,MeshSize));
	_waterHeightShader->setWaterParam(_waterParam);

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	_heightTexture0->disableFramebuffer();
	_waterParam.w = 0;
}

void WaterGPU::drawWaterNormalTexture()
{
	//计算法线向量
	_normalTexture->activeFramebuffer();
	_waterNormalShader->perform();

	int identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER,identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)(sizeof(float)*3));

	_waterNormalShader->setBaseMap(_heightTexture0->getColorBuffer(), 0);
	_waterNormalShader->setMeshInterval(1.0f);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	_normalTexture->disableFramebuffer();
}

void		WaterGPU::drawCaustic()
{
	int result;
	//绑定帧缓冲区对象,计算Caustic
	glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _photonTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	//绑定顶点缓冲区对象
	_photonProgram->perform();
	glBindBuffer(GL_ARRAY_BUFFER,_photonVertexbufferId);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,nullptr);

	glBindBuffer(GL_ARRAY_BUFFER,_photonFragCoordbufferId);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	//纹理
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_heightTexture0->getColorBuffer());
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,_normalTexture->getColorBuffer());

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,_photonTexture);

	glDrawArrays(GL_POINTS, 0, PhotonSize*PhotonSize);
	/*
	  *将PhotonTexture进行分配,并将各个区域映射到立方体贴图中
	 */
	result = glGetError();
	int defaultFramebuffer,defaultReadBuffer=0;
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &defaultFramebuffer);
	glGetIntegerv(GL_READ_BUFFER,&defaultReadBuffer);
	//
	glBindFramebuffer(GL_READ_FRAMEBUFFER,_frameBufferId);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	//pixel buffer coord
	float border[12] = {
		0,0,//+X
		PhotonSize,0,//-X
		0,PhotonSize,//+Y
		PhotonSize,PhotonSize,//-Y
		0,PhotonSize*2,//+Z
		PhotonSize,PhotonSize*2,//-Z
	};
	glBindTexture(GL_TEXTURE_CUBE_MAP, _photonTextureCubeMap[0]);
	result = glGetError();
	for (int i = 0; i < 6; ++i)
	{
		glBindBuffer(GL_PIXEL_PACK_BUFFER, _pixelbufferId);
		result = glGetError();
		glReadPixels(border[i * 2], border[i * 2 + 1],PhotonSize,PhotonSize,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
		result = glGetError();
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _pixelbufferId);
		result = glGetError();
		//glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,_photonTextureCubeMap[0]);
		result = glGetError();
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, PhotonSize, PhotonSize, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		result = glGetError();
		//glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0);
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
	result = glGetError();
	glBindBuffer(GL_PIXEL_PACK_BUFFER,0);
	result = glGetError();
	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
	//
	//glReadBuffer(defaultReadBuffer);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFramebuffer);
	/*
	  *模糊
	 */
	//float step = 1.0f / PhotonSize;
	//const GLVector3 Kernel[6] = {
	//		GLVector3(step,0,0),GLVector3(-step,0,0),
	//		GLVector3(0,step,0),GLVector3(0,-step,0),
	//		GLVector3(0,0,step),GLVector3(0,0,-step)
	//};
	//const GLVector3  kernelHorizontal[6] = {
	//	GLVector3(0,0,-step),GLVector3(0,0,step),
	//	GLVector3(step,0,0),GLVector3(step,0,0),
	//	GLVector3(step,0,0),GLVector3(-step,0,0)
	//};
	//const GLVector3 kernelVertical[6] = {
	//	GLVector3(0,-step,0),GLVector3(0,-step,0),
	//	GLVector3(0,0,-step),GLVector3(0,0,step),
	//	GLVector3(0,-step,0),GLVector3(0,-step,0)
	//};
	////
	//_photonBlurProgram->perform();
	//int kernelLoc = _photonBlurProgram->getUniformLocation("g_Kernel");
	//int   cubeMapLoc = _photonBlurProgram->getUniformLocation("g_CubeMap");
	//for (int i = 0; i < 6; ++i)
	//{
	//	glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);
	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, _photonTextureCubeMap[1], 0);
	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	//	//glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	//	glClear(GL_COLOR_BUFFER_BIT);
	//	//
	//	int vertexId = GLCacheManager::getInstance()->loadBufferIdentity();
	//	glBindBuffer(GL_ARRAY_BUFFER, vertexId);
	//	glEnableVertexAttribArray(0);
	//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, nullptr);
	//	//int fragCoord
	//	glBindBuffer(GL_ARRAY_BUFFER, _cubeMapfragCoordVertex);
	//	glEnableVertexAttribArray(1);
	//	glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,(void*)(sizeof(GLVector3)*4*i));
	//	//uniform
	//	glUniform3fv(kernelLoc, 1, &kernelHorizontal[i].x);

	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_CUBE_MAP, _photonTextureCubeMap[0]);
	//	glUniform1i(cubeMapLoc, 0);
	//	////
	//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	//}
	//for (int i = 0; i < 6; ++i)
	//{
	//	glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);
	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, _photonTextureCubeMap[0],0);
	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	//	//glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	//	glClear(GL_COLOR_BUFFER_BIT);
	//	//
	//	int vertexId = GLCacheManager::getInstance()->loadBufferIdentity();
	//	glBindBuffer(GL_ARRAY_BUFFER, vertexId);
	//	glEnableVertexAttribArray(0);
	//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);
	//	//int fragCoord
	//	glBindBuffer(GL_ARRAY_BUFFER, _cubeMapfragCoordVertex);
	//	glEnableVertexAttribArray(1);
	//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(GLVector3) * 4 * i));
	//	//
	//	glUniform3fv(kernelLoc, 1, &kernelVertical[i].x);

	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_CUBE_MAP, _photonTextureCubeMap[1]);
	//	glUniform1i(cubeMapLoc, 0);
	//	////
	//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	//}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void  WaterGPU::drawSkybox()
{
	_poolShader->perform();
	//绑定顶点数据
	_poolMesh->bindVertexObject(0);
	_poolMesh->bindTexCoordObject(1);
	//设置Uniform 变量
	_poolShader->setMVPMatrix(_skyboxModelMatrix * _camera->getViewProjMatrix());
	_poolShader->setTexCubeMap(_texCubeMap->getName(), 0);
	_poolShader->setCausticCubeMap(_photonTextureCubeMap[0],1);

	//_poolShader->setKernel(kernel,6);
	_poolMesh->drawShape();
}

void WaterGPU::drawTexture(int textureId)
{
	_normalProgram->perform();
	int identityVertexId = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER, identityVertexId);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float)*3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,textureId);
	glUniform1i(_normalProgram->getUniformLocation("g_BaseMap"),0);

	Matrix identity;
	glUniformMatrix4fv(_normalProgram->getUniformLocation("g_MVPMatrix"), 1, GL_FALSE, identity.pointer());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void WaterGPU::draw()
{
	auto &winSize = GLContext::getInstance()->getWinSize();
	glViewport(0, 0, MeshSize, MeshSize);
	glCullFace(GL_BACK);
	/*
	  *计算高度场
	 */
	drawWaterHeightTexture();
	/*
	  *计算法线向量
	 */
	drawWaterNormalTexture();
	//Caustic
	glViewport(0, 0, PhotonSize * 2, PhotonSize * 3);
	drawCaustic();
	//
	glViewport(0, 0, winSize.width, winSize.height);
	//drawTexture(_photonTexture);
	//glEnable(GL_DEPTH_TEST);
	/*glEnable(GL_CULL_FACE);*/
	/*
	  *天空盒
	 */
	drawSkybox();
	//glDisable(GL_CULL_FACE);
	/*
	  *水
	 */
	if (_camera->getCameraPosition().y > 0)
		glCullFace(GL_BACK);
	else
		glCullFace(GL_FRONT);
	_waterShader->perform();
	//顶点数据
	_waterMesh->bindVertexObject(0);
	_waterMesh->bindTexCoordObject(1);
	//uniform
	_waterShader->setCameraPosition(_camera->getCameraPosition());
	_waterShader->setCubeMapNormal(_texCubeNormals,6);
	_waterShader->setHalfCubeHeight(_halfCubeHeight);
	_waterShader->setHeightMap(_heightTexture0->getColorBuffer(),0);
	_waterShader->setNormalMap(_normalTexture->getColorBuffer(), 1);
	_waterShader->setTexCubeMap(_texCubeMap->getName(), 2);
	_waterShader->setModelMatrix(_waterModelMatrix);
	_waterShader->setLightPosition(_lightPosition);
	_waterShader->setViewProjMatrix(_camera->getViewProjMatrix());
	
	_waterMesh->drawShape();
	/*
	  *交换高度场RTT
	 */
	RenderTexture *rtt = _heightTexture0;
	_heightTexture0 = _heightTexture1;
	_heightTexture1 = rtt;
}

void WaterGPU::update(float deltaTime)
{
	_deltaTime += deltaTime;
	if (_deltaTime > 0.3)
	{
		//_deltaTime = 0;
		//float dx = 1.0f * rand() / RAND_MAX  ;
		//float dy = 1.0f * rand() / RAND_MAX ;

		//_waterParam.x = dx *MeshSize;
		//_waterParam.y = dy * MeshSize;
		//_waterParam.z = 4.0f;
		//_waterParam.w = 1.5f;
	}
	const float speed = 20.0f;
	GLVector2  stepVec2;
	if (_keyMask & _KEY_MASK_W_)
		stepVec2.x -= speed * deltaTime;
	if (_keyMask & _KEY_MASK_S_)
		stepVec2.x += speed * deltaTime;
	if (_keyMask & _KEY_MASK_A_)
		stepVec2.y -= speed * deltaTime;
	if (_keyMask & _KEY_MASK_D_)
		stepVec2.y += speed * deltaTime;
	if (stepVec2.x != 0 || stepVec2.y != 0)
		_camera->updateTranslateMatrix(stepVec2.y, stepVec2.x);
}

bool WaterGPU::onTouchBegan(const glk::GLVector2 *touchPoint)
{
	_offsetVec2 = *touchPoint;
	return true;
}

void WaterGPU::onTouchMoved(const glk::GLVector2 *touchPoint)
{
	GLVector2 touchOffset = *touchPoint - _offsetVec2;
	auto &winSize = GLContext::getInstance()->getWinSize();
	float pitch = -touchOffset.y / winSize.height * 57.2596f;
	float yaw = touchOffset.x / winSize.width *57.2596f;

	_camera->updateRotateMatrix(pitch, yaw);
	_offsetVec2 = *touchPoint;
}

void WaterGPU::onTouchEnded(const glk::GLVector2 *touchPoint)
{

}

bool WaterGPU::onKeyPressed(const glk::KeyCodeType keyCode)
{
	if (keyCode == KeyCodeType::KeyCode_W)
		_keyMask |= _KEY_MASK_W_;
	else if (keyCode == KeyCodeType::KeyCode_S)
		_keyMask |= _KEY_MASK_S_;
	else if (keyCode == KeyCodeType::KeyCode_A)
		_keyMask |= _KEY_MASK_A_;
	else if (keyCode == KeyCodeType::KeyCode_D)
		_keyMask |= _KEY_MASK_D_;
	return true;
}

void WaterGPU::onKeyReleased(glk::KeyCodeType keyCode)
{
	if (keyCode == KeyCodeType::KeyCode_W)
		_keyMask &= ~_KEY_MASK_W_;
	else if (keyCode == KeyCodeType::KeyCode_S)
		_keyMask &= ~_KEY_MASK_S_;
	else if (keyCode == KeyCodeType::KeyCode_A)
		_keyMask &= ~_KEY_MASK_A_;
	else if (keyCode == KeyCodeType::KeyCode_D)
		_keyMask &= ~_KEY_MASK_D_;
}

bool WaterGPU::onMouseClick(glk::MouseType mouseType, const glk::GLVector2 *clickPoint)
{
	if (mouseType == MouseType::MouseType_Right)
	{
		addWaterDrop(*clickPoint);
	}
	return mouseType == MouseType::MouseType_Right;
}

void WaterGPU::onMouseMoved(glk::MouseType mouseType, const glk::GLVector2 *clickPoint)
{
	addWaterDrop(*clickPoint);
}

void WaterGPU::onMouseEnded(glk::MouseType mouseType, const glk::GLVector2 *clickPoint)
{

}

void WaterGPU::addWaterDrop(const glk::GLVector2 &touchPoint)
{
	auto &winSize = GLContext::getInstance()->getWinSize();
	float ndcx = touchPoint.x / winSize.width *2.0f - 1.0f;
	float ndcy = touchPoint.y / winSize.height*2.0f - 1.0f;

	 GLVector4 openGLCoord = GLVector4(ndcx,ndcy,1.0f,1.0f) * _camera->getInverseViewProjMatrix();
	 openGLCoord.x /= openGLCoord.w;
	 openGLCoord.y /= openGLCoord.w;
	 openGLCoord.z /= openGLCoord.w;
	 //
	 auto &cameraPosition = _camera->getCameraPosition();
	 //水面的法线
	 const GLVector3 normal(0.0f,1.0f,0.0f);

	 const GLVector3 ray = (openGLCoord.xyz()-cameraPosition).normalize();
	 //求水面的平面的方程式Ax+By+Cz-D=0
	 float D = normal.dot(GLVector3(0,0,0));
	 //求视点与水平面的夹角的正弦值
	 float sinValue = -ray.dot(normal);
	 //如果视线不与水平面平行
	 if (sinValue != 0)
	 {
		 float distance = normal.dot(cameraPosition) - D;
		 distance /= sinValue;

		 GLVector3 targetPoint = cameraPosition +  ray * distance;
		 //判断视线是否经过了水面
		 if (targetPoint.x >= -MeshSize && targetPoint.x <= MeshSize && targetPoint.z <= 0 && targetPoint.z >= -MeshSize*2)
		 {
			 float dx = targetPoint.x / MeshSize *0.5f + 0.5f;
			 float dz = -targetPoint.z / MeshSize *0.5f;
			 _waterParam.x = dx * MeshSize;
			 _waterParam.y = dz * MeshSize;
			 _waterParam.z = 4.0f;
			 _waterParam.w = 1.5f;
		 }
	 }
}