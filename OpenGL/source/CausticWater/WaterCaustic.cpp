/*
  *水渲染/Caustic
  *@2017-8-23
  *@xiaohuaxiong
 */
#include "GL/glew.h"
#include "WaterCaustic.h"
#include "engine/GLContext.h"
#include "engine/GLCacheManager.h"

__US_GLK__;
//水面网格的宽度
const float   WaterMesh = 128;
const float    WaterHeight = 256 * 0.5;
//Caustic的宽度
const  float   CausticMesh = 256;
const float     CausticHeight = 256;
WaterCaustic::WaterCaustic() :
_heightShader(nullptr)
,_normalShader(nullptr)
,_texCubeMap(nullptr)
,_waterHeight(0.0f)
, _waterMesh(nullptr)
, _camera(nullptr)
,_heightTexture0(nullptr)
,_heightTexture1(nullptr)
,_normalTexture(nullptr)
,_deltaTime(0)
{

}

WaterCaustic::~WaterCaustic()
{
	_heightShader->release();
	_normalShader->release();
	_texCubeMap->release();
	_waterMesh->release();
	_camera->release();
	_heightTexture1->release();
	_heightTexture0->release();
	_normalTexture->release();
}

WaterCaustic      *WaterCaustic::create()
{
	WaterCaustic *caustic = new WaterCaustic();
	caustic->init();
	return caustic;
}

void      WaterCaustic::init()
{
	_heightShader = WaterHeightShader::create("shader/Caustic/WaterHeight_VS.glsl", "shader/Caustic/WaterHeight_FS.glsl");
	_normalShader = WaterNormalShader::create("shader/Caustic/WaterNormal_VS.glsl","shader/Caustic/WaterNormal_FS.glsl");
	_waterShader = WaterShader::create("shader/Caustic/Water_VS.glsl","shader/Caustic/Water_FS.glsl");
	_fuzzyShader = FuzzyShader::create("shader/Caustic/Fuzzy_VS.glsl","shader/Caustic/Fuzzy_FS.glsl");
	_causticShader = CausticShader::create("shader/Caustic/Caustic_VS.glsl","shader/Caustic/Caustic_FS.glsl");
	_normalProgram = GLProgram::createWithFile("shader/WaterGPU/Normal_VS.glsl", "shader/WaterGPU/Normal_FS.glsl");
	_groundProgram = GLProgram::createWithFile("shader/Caustic/Ground_VS.glsl", "shader/Caustic/Ground_FS.glsl");
	const char *cubeMapFiles[6] = {
		"tga/water/pool/right.tga",//+X
		"tga/water/pool/left.tga",//-X
		"tga/water/pool/top.tga",//+Y
		"tga/water/pool/bottom.tga",//-Y
		"tga/water/pool/back.tga",//+Z
		"tga/water/pool/front.tga"//-Z
	};
	_texCubeMap = GLCubeMap::createWithFiles(cubeMapFiles);
	_groundTexture = GLTexture::createWithFile("tga/water/terrain_grass.tga");
	//Render Texture
	const Size   textureSize(WaterMesh,WaterMesh);
	const int     formatType[1] = {GL_RG16F};
	_heightTexture0 = RenderTexture::createRenderTexture(textureSize,RenderTexture::RenderType::ColorBuffer, formatType);
	_heightTexture1 = RenderTexture::createRenderTexture(textureSize, RenderTexture::RenderType::ColorBuffer, formatType);
	const int formatType2[1] = {GL_RGB16F};
	_normalTexture = RenderTexture::createRenderTexture(textureSize, RenderTexture::RenderType::ColorBuffer, formatType2);
	_heightTexture0->setClearBuffer(false);
	_heightTexture1->setClearBuffer(false);
	_normalTexture->setClearBuffer(false);
	//Mesh
	_waterMesh = Mesh::createWithIntensity(WaterMesh, WaterMesh, WaterMesh, 1.0f,Mesh::MeshType::MeshType_XOZ);
	_groundMesh = Mesh::createWithIntensity(4, CausticMesh, CausticMesh, 1.0f, Mesh::MeshType::MeshType_XOZ);
	//
	initWater();
	//Camera
	initCamera();
}

void   WaterCaustic::initCamera()
{
	_camera = Camera::createCamera(GLVector3(0.0f, WaterHeight / 1.2f, WaterHeight), GLVector3(0.0f, 0.0f,-WaterHeight / 1.2f), GLVector3(0.0f, 1.0f, 0.0f));
	auto &winSize = GLContext::getInstance()->getWinSize();
	_camera->setPerspective(60.0f, winSize.width/winSize.height,0.1f,1000.0f);
}

void WaterCaustic::initWater()
{
	_waterHeight = WaterHeight;
	_waterModelMatrix.translate(0, _waterHeight, -WaterMesh);
	_lightDirection = GLVector3(0.0f, WaterHeight * 3,0.0f);
	/*
	  *水底平面网格
	 */
	_groundHeight = -WaterHeight / 4.0f;
	const int MeshSize = (int)CausticMesh;
	float    *groundMesh = new float[5 * MeshSize*MeshSize];
	const float      intervalStep = 1.0f / (CausticMesh -1.0f);
	int                   index = 0;
	for (int z = 0; z < MeshSize; ++z)
	{
		float   zValue = 1.0f - z * intervalStep * 2.0f;
		for (int x = 0; x < MeshSize; ++x)
		{
			groundMesh[index++] = (x*intervalStep*2.0f - 1.0f)* CausticMesh;
			groundMesh[index++] = 0.0f;
			groundMesh[index++] = zValue * CausticMesh;
			//fragCoord
			groundMesh[index++] = x*intervalStep;
			groundMesh[index++] = z* intervalStep;
		}
	}

	glGenBuffers(1, &_groundMeshVertex);
	glBindBuffer(GL_ARRAY_BUFFER, _groundMeshVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) *5 * MeshSize * MeshSize, groundMesh, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);

	delete[] groundMesh;
	groundMesh = nullptr;
	//
	const Size causticSize(CausticMesh,CausticMesh);
	_causticTexture0 = RenderTexture::createRenderTexture(causticSize,RenderTexture::RenderType::ColorBuffer);
	_causticTexture1 = RenderTexture::createRenderTexture(causticSize, RenderTexture::RenderType::ColorBuffer);
	//地面
	_groundModelMatrix.translate(0, _groundHeight, -CausticMesh);
}

void   WaterCaustic::drawWaterHeightTexture()
{
	//绑定帧缓冲区对象,最后产生数值输出的一定是_heightTexture0
	_heightTexture0->activeFramebuffer();
	_heightShader->perform();

	int identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER, identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	_heightShader->setBaseMap(_heightTexture1->getColorBuffer(), 0);
	_heightShader->setMeshSize(GLVector2(WaterMesh, WaterMesh));
	_heightShader->setWaterParam(_waterParam);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	_heightTexture0->disableFramebuffer();
	_waterParam.w = 0;
}

void  WaterCaustic::drawWaterNormalTexture()
{
	//计算法线向量
	_normalTexture->activeFramebuffer();
	_normalShader->perform();

	int identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER, identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	_normalShader->setBaseMap(_heightTexture0->getColorBuffer(), 0);
	_normalShader->setMeshInterval(1.0f);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	_normalTexture->disableFramebuffer();
}

void   WaterCaustic::drawCaustic()
{
	_causticTexture0->activeFramebuffer();
	_causticShader->perform();
	glBindBuffer(GL_ARRAY_BUFFER, _groundMeshVertex);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5,(void*)(sizeof(float)*3));

	Matrix   modelMatrix;
	modelMatrix.translate(0, _waterHeight, -CausticMesh);
	_causticShader->setModelMatrix(modelMatrix);
	_causticShader->setWaterHeightMap(_heightTexture0->getColorBuffer(), 0);
	_causticShader->setWaterNormalMap(_normalTexture->getColorBuffer(), 1);
	_causticShader->setGroundMap(_causticTexture0->getColorBuffer(),2);
	_causticShader->setGroundHeight(_groundHeight);
	_causticShader->setWaterHeight(_waterHeight);
	_causticShader->setLightDirection(_lightDirection.normalize());
	_causticShader->setResolution(GLVector2(CausticMesh,CausticMesh));

	glDrawArrays(GL_POINTS, 0, (int)(CausticMesh*CausticMesh));
	/*
	  *模糊图形处理
	 */
	_causticTexture0->disableFramebuffer();
	//////////////////////////////////////
	_causticTexture1->activeFramebuffer();
	//先水平模糊
	_fuzzyShader->perform();
	_fuzzyShader->setBaseMap(_causticTexture0->getColorBuffer(), 0);
	_fuzzyShader->setPixelStep(GLVector2(1.0f / CausticMesh,0));
	int identityVertex=GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER,identityVertex);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5,(void*)(sizeof(float)*3));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	int result = glGetError();
	_causticTexture1->disableFramebuffer();
	//
	_causticTexture0->activeFramebuffer();
	_fuzzyShader->setBaseMap(_causticTexture1->getColorBuffer(), 0);
	_fuzzyShader->setPixelStep(GLVector2(0,1.0f/CausticMesh));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	////last
	_causticTexture0->disableFramebuffer();
}

void  WaterCaustic::drawGround()
{
	_groundProgram->perform();
	int mvpMatrixLoc = _groundProgram->getUniformLocation("g_MVPMatrix");
	int baseMapLoc = _groundProgram->getUniformLocation("g_BaseMap");
	int causticMapLoc = _groundProgram->getUniformLocation("g_CausticMap");
	//
	_groundMesh->bindVertexObject(0);
	_groundMesh->bindTexCoordObject(1);

	glUniformMatrix4fv(mvpMatrixLoc,1,GL_FALSE,(_groundModelMatrix*_camera->getViewProjMatrix()).pointer());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_groundTexture->getName());
	glUniform1i(baseMapLoc,0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _causticTexture0->getColorBuffer());
	glUniform1i(causticMapLoc, 1);

	_groundMesh->drawShape();
}

void WaterCaustic::drawTexture(int textureId)
{
	_normalProgram->perform();
	int identityVertexId = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER, identityVertexId);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glUniform1i(_normalProgram->getUniformLocation("g_BaseMap"), 0);

	Matrix identity;
	glUniformMatrix4fv(_normalProgram->getUniformLocation("g_MVPMatrix"), 1, GL_FALSE, identity.pointer());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void  WaterCaustic::drawWater()
{
	_waterShader->perform();
	//
	_waterMesh->bindVertexObject(0);
	_waterMesh->bindTexCoordObject(1);
	//
	_waterShader->setViewProjMatrix(_camera->getViewProjMatrix());
	_waterShader->setCameraPosition(_camera->getCameraPosition());
	const GLVector3 normals[6] = {
		GLVector3(1.0f,0,0),GLVector3(-1.0f,0,0),
		GLVector3(0,1,0),GLVector3(0,-1,0),
		GLVector3(0,0,1),GLVector3(0,0,-1),
	};
	_waterShader->setCubeMapNormal(normals,6);
	_waterShader->setHalfCubeHeight(WaterMesh);
	_waterShader->setHeightMap(_heightTexture0->getColorBuffer(), 0);
	_waterShader->setNormalMap(_normalTexture->getColorBuffer(), 1);
	_waterShader->setTexCubeMap(_texCubeMap->getName(), 2);
	_waterShader->setLightDirection(_lightDirection);
	_waterShader->setModelMatrix(_waterModelMatrix);
	_waterShader->setWaterHeight(_waterHeight);

	_waterMesh->drawShape();
}

void  WaterCaustic::draw()
{
	auto &winSize = GLContext::getInstance()->getWinSize();
	glViewport(0, 0, WaterMesh, WaterMesh);
	//
	drawWaterHeightTexture();
	drawWaterNormalTexture();
	//
	//
	glViewport(0, 0, CausticMesh, CausticMesh);
	drawCaustic();
	//
	glViewport(0, 0, winSize.width, winSize.height);
	drawWater();
	//drawTexture(_causticTexture0->getColorBuffer());
	drawGround();
	RenderTexture *rt = _heightTexture0;
	_heightTexture0 = _heightTexture1;
	_heightTexture1 = rt;
}

void  WaterCaustic::update(float deltaTime)
{
	_deltaTime += deltaTime;
	if (_deltaTime >= 0.1f)
	{
		_deltaTime = 0.0f;
		float dx = 1.0f * rand() / RAND_MAX;
		float dy = 1.0f * rand() / RAND_MAX;

		_waterParam.x = dx * WaterMesh;
		_waterParam.y = dy * WaterMesh;
		_waterParam.z = 4.0f;
		_waterParam.w = 1.0f;
	}
}