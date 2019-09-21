/*
  *体积光散射实现
  *2018年11月7日
  *@author:xiaohuaxiong
 */
#include "GL/glew.h"
#include "VolumLight.h"
#include "engine/GLCacheManager.h"
#include "engine/GLContext.h"
#include "engine/Math.h"

#include "engine/event/EventManager.h"

using namespace glk;
VolumLight::VolumLight():
	_camera(nullptr),
	_lightTexture(nullptr),
	_offlineRenderTexture(nullptr),
	_fuzzyRenderTexture1(nullptr),
	_fuzzyRenderTexture2(nullptr),
	_craftRenderTexture(nullptr),
	_materialShader(nullptr),
	_cutoutShader(nullptr),
	_rayExtruderShader(nullptr),
	_fuzzyShader(nullptr),
	_downSizeShader(nullptr),
	_combineShader(nullptr),
	_meshShape(nullptr),
	_pyramidShape(nullptr),
	_sphereShape(nullptr),
	_cubeShape(nullptr),
	_rayOpacity(1.0f),
	_rayRadius(0.17f),
	_lightPosition(0,5.0f,-9.0f),
	_lightColor(1.0f,1.0f,1.0f),
	_lightAmbientColor(0.10f,0.10f,0.10f),
	_rayColor(0.9f,0.9f,1.0f,1.0f),
	_touchListener(nullptr),
	_keyListener(nullptr),
	_keyCodeMask(0)
{
}

VolumLight::~VolumLight()
{
	_camera->release();
}

void  VolumLight::init()
{
	initRenderTexture();
	initShader();
	initGeometry();

	_lightTexture = GLTexture::createWithFile("tga/light/GodRay1.tga");

	auto &winSize = GLContext::getInstance()->getWinSize();
	_camera = Camera2::createWithPerspective(60.0f,winSize.width/winSize.height,0.15f,20.0f);
	GLVector3   eyePosition(0,4,4);
	GLVector3   targetPosition(0,2,-4);
	_camera->lookAt(eyePosition, targetPosition);

	auto &nearFarFovRatio = _camera->getNearFarFovRatio();
	_nearQFarRatio.x = -(nearFarFovRatio.x + nearFarFovRatio.y)/(nearFarFovRatio.y - nearFarFovRatio.x);
	_nearQFarRatio.y = -2.0f *nearFarFovRatio.x * nearFarFovRatio.y/(nearFarFovRatio.y- nearFarFovRatio.x);
	_nearQFarRatio.z = nearFarFovRatio.w;

	_occluderParams.x = 3.45f;
	_occluderParams.y = 3.80f;

	_pixelStepVec2.x = 0.5f/winSize.width;
	_pixelStepVec2.y = 0.5f / winSize.height;

	_fuzzyWeightInv = 0.0f;
	for (int k = 0; k < 5; ++k)
	{
		float x = k / 4.0f * 2.0f - 1.0f;
		_fuzzyWeights[k] = gauss_distribution(x, 0, 0,0, 1);

		_fuzzyWeightInv += _fuzzyWeights[k];
	}
	_fuzzyWeights[5] = 1.0f/_fuzzyWeightInv;

	_touchListener = TouchEventListener::createTouchListener(this, glk_touch_selector(VolumLight::onTouchBegan),glk_move_selector(VolumLight::onTouchMoved),glk_release_selector(VolumLight::onTouchEnded));
	EventManager::getInstance()->addTouchEventListener(_touchListener, 0);

	_keyListener = KeyEventListener::createKeyEventListener(this, glk_key_press_selector(VolumLight::onKeyPressed),glk_key_release_selector(VolumLight::onKeyReleased));
	EventManager::getInstance()->addKeyEventListener(_keyListener, 0);
}

void  VolumLight::initRenderTexture()
{
	auto &winSize = GLContext::getInstance()->getWinSize();
	_offlineRenderTexture = RenderTexture::createRenderTexture(winSize,RenderTexture::RenderType::ColorBuffer | RenderTexture::RenderType::DepthBuffer);
	_craftRenderTexture = RenderTexture::createRenderTexture(winSize, RenderTexture::RenderType::ColorBuffer);

	const Size  halfSize(winSize.width*0.5f,winSize.height*0.5f);
	_downSizeRenderTexture = RenderTexture::createRenderTexture(halfSize,RenderTexture::RenderType::ColorBuffer);
	_fuzzyRenderTexture1 = RenderTexture::createRenderTexture(halfSize, RenderTexture::RenderType::ColorBuffer);
	_fuzzyRenderTexture2 = RenderTexture::createRenderTexture(halfSize, RenderTexture::RenderType::ColorBuffer);
}

void  VolumLight::initShader()
{
	//Material Shader
	_materialShader = GLProgram::createWithFile("shader/volum_light/material_render_vs.glsl", "shader/volum_light/material_render_fs.glsl");
	_materialModelMatrixLoc = _materialShader->getUniformLocation("g_ModelMatrix");
	_materialNormalMatrixLoc = _materialShader->getUniformLocation("g_NormalMatrix");
	_materialViewProjMatrixLoc = _materialShader->getUniformLocation("g_ViewProjMatrix");
	_materialColorLoc = _materialShader->getUniformLocation("g_Color");
	_materialLightPositionLoc = _materialShader->getUniformLocation("g_LightPosition");
	_materialLightColorLoc = _materialShader->getUniformLocation("g_LightColor");
	_materialAmbientColorLoc = _materialShader->getUniformLocation("g_AmbientColor");
	//Cutout Shader
	_cutoutShader = GLProgram::createWithFile("shader/volum_light/volum_cutout_vs.glsl", "shader/volum_light/volum_cutout_fs.glsl");
	_cutoutDepthTextureLoc = _cutoutShader->getUniformLocation("g_DepthTexture");
	_cutoutColorTextureLoc = _cutoutShader->getUniformLocation("g_ColorTexture");
	_cutoutOccluderParamLoc = _cutoutShader->getUniformLocation("g_OccluderParams");
	_cutoutNearQFarLoc = _cutoutShader->getUniformLocation("g_NearQFar");
	_cutoutLightPositionScreenLoc = _cutoutShader->getUniformLocation("g_LightPositionScreen");
	//ray Extruder
	_rayExtruderShader = GLProgram::createWithFile("shader/volum_light/ray_extruder_vs.glsl", "shader/volum_light/ray_extruder_fs.glsl");
	_rayExtruderRayTextureLoc = _rayExtruderShader->getUniformLocation("g_RayTexture");
	_rayExtruderRayLengthVec2Loc = _rayExtruderShader->getUniformLocation("g_RayLengthVec2");
	_rayExtruderLightPositionScreenLoc = _rayExtruderShader->getUniformLocation("g_LightPositionScreen");
	//Fuzzy Shader
	_fuzzyShader = GLProgram::createWithFile("shader/volum_light/fuzzy_vs.glsl", "shader/volum_light/fuzzy_fs.glsl");
	_fuzzyBaseTextureLoc = _fuzzyShader->getUniformLocation("g_BaseTexture");
	_fuzzyPixelStepLoc = _fuzzyShader->getUniformLocation("g_PixelStep");
	_fuzzyWeightsLoc = _fuzzyShader->getUniformLocation("g_Weights");
	//down size shader
	_downSizeShader = GLProgram::createWithFile("shader/volum_light/down_size_vs.glsl", "shader/volum_light/down_size_fs.glsl");
	_downSizeBaseTextureLoc = _downSizeShader->getUniformLocation("g_BaseTexture");
	//Combine Shader
	_combineShader = GLProgram::createWithFile("shader/volum_light/combine_vs.glsl", "shader/volum_light/combine_fs.glsl");
	_combineBaseTextureLoc = _combineShader->getUniformLocation("g_BaseTexture");
	_combineRayTextureLoc = _combineShader->getUniformLocation("g_RayTexture");
	_combineRayColorLoc = _combineShader->getUniformLocation("g_RayColor");
	_combineRayOpacityLoc = _combineShader->getUniformLocation("g_RayOpacity");
}

void  VolumLight::initGeometry()
{
	//mesh
	_meshShape = Mesh::createWithIntensity(4, 10, 10, 1.0f);
	_meshModelMatrix1.rotate(-90.0f,1,0,0);
	_meshModelMatrix2.translate(0, 10, -10);

	_meshModelMatrix1.trunk(_meshNormalMatrix1);
	_meshModelMatrix2.trunk(_meshNormalMatrix2);
	//Cube
	_cubeShape = Cube::createWithScale(1.0f);
	_cubeModelMatrix1.scale(1,10,1);
	_cubeModelMatrix1.translate(-5, 10, -5);
	_cubeNormalMatrix1 = _cubeModelMatrix1.normalMatrix();

	_cubeModelMatrix2.scale(1,10,1);
	_cubeModelMatrix2.translate(5,10,-5);
	//Pyramid
	_pyramidShape = Pyramid::create(2.0f);
	_pyramidModelMatrix.translate(-2, 4, -5.5);
	_pyramidModelMatrix.trunk(_pyramidNormalMatrix);
	//Sphere
	_sphereShape = Sphere::createWithSlice(32, 2.0f);
	_sphereModelMatrix.translate(2, 4, -6.5);
	_sphereModelMatrix.trunk(_sphereNormalMatrix);
}

void  VolumLight::renderGeometry()
{
	_materialShader->perform();
	/*
	  *mesh-1
	 */
	_meshShape->bindVertexObject(0);
	_meshShape->bindNormalObject(1);
	glUniformMatrix4fv(_materialModelMatrixLoc,1,GL_FALSE,_meshModelMatrix1.pointer());
	glUniformMatrix4fv(_materialViewProjMatrixLoc,1,GL_FALSE,_camera->getViewProjMatrix().pointer());
	glUniformMatrix3fv(_materialNormalMatrixLoc,1,GL_FALSE,_meshNormalMatrix1.pointer());

	GLVector4  color(0.72f,0.67f,0.89f,1.0f);
	glUniform4fv(_materialColorLoc,1,&color.x);
	glUniform3fv(_materialLightColorLoc,1,&_lightColor.x);
	glUniform3fv(_materialLightPositionLoc,1,&_lightPosition.x);
	glUniform3fv(_materialAmbientColorLoc,1,&_lightAmbientColor.x);

	_meshShape->drawShape();
	/*
	  *Mesh-2
	 */
	glUniformMatrix4fv(_materialModelMatrixLoc,1,GL_FALSE,_meshModelMatrix2.pointer());
	glUniformMatrix3fv(_materialNormalMatrixLoc,1,GL_FALSE,_meshNormalMatrix2.pointer());
	color = GLVector4(0.32f,0.64f,0.56f,1.0f);
	glUniform4fv(_materialColorLoc,1,&color.x);
	_meshShape->drawShape();
	/*
	  *Cube-1
	 */
	_cubeShape->bindVertexObject(0);
	_cubeShape->bindNormalObject(1);
	glUniformMatrix4fv(_materialModelMatrixLoc,1,GL_FALSE,_cubeModelMatrix1.pointer());
	glUniformMatrix3fv(_materialNormalMatrixLoc,1,GL_FALSE,_cubeNormalMatrix1.pointer());
	color = GLVector4(1,1,0,1);
	glUniform4fv(_materialColorLoc,1,&color.x);
	_cubeShape->drawShape();
	/*
	  *Cube-2
	 */
	glUniformMatrix4fv(_materialModelMatrixLoc,1,GL_FALSE,_cubeModelMatrix2.pointer());
	glUniformMatrix3fv(_materialNormalMatrixLoc,1,GL_FALSE,_cubeNormalMatrix2.pointer());
	color = GLVector4(0,1,1,1);
	glUniform4fv(_materialColorLoc,1,&color.x);
	_cubeShape->drawShape();
	/*
	  *Pyramid-1
	 */
	_pyramidShape->bindVertexObject(0);
	_pyramidShape->bindNormalObject(1);
	glUniformMatrix4fv(_materialModelMatrixLoc,1,GL_FALSE,_pyramidModelMatrix.pointer());
	glUniformMatrix3fv(_materialNormalMatrixLoc,1,GL_FALSE,_pyramidNormalMatrix.pointer());
	color = GLVector4(0,1,0,1);
	glUniform4fv(_materialColorLoc,1,&color.x);
	_cubeShape->drawShape();
	/*
	  *Sphere-1
	 */
	_sphereShape->bindVertexObject(0);
	_cubeShape->bindNormalObject(1);
	glUniformMatrix4fv(_materialModelMatrixLoc,1,GL_FALSE,_sphereModelMatrix.pointer());
	glUniformMatrix3fv(_materialNormalMatrixLoc,1,GL_FALSE,_sphereNormalMatrix.pointer());
	color = GLVector4(1,0,1,1);
	glUniform4fv(_materialColorLoc,1,&color.x);
	_sphereShape->drawShape();
}
/*
  *抽取出场景的深度信息
 */
void  VolumLight::cutoutDepthVolum()
{
	//求眼睛与光源的方向
	GLVector4    light_position = _lightPosition * _camera->getViewMatrix();
	//求视图矩阵中Z轴的方向
	//const float *matrix_array = _camera->getViewMatrix().pointer();
	float   relative_direction = light_position.dot(GLVector4(0,0,-1,0));//light_position.x * matrix_array[8] + light_position.y * matrix_array[9] + light_position.z * matrix_array[10];
	//如果光源相对眼睛的方向在视图矩阵的视野范围之内
	GLVector4  light_screen_pos = _lightPosition * _camera->getViewProjMatrix();
	light_screen_pos.x /= light_screen_pos.w;
	light_screen_pos.y /= light_screen_pos.w;
	light_screen_pos.z /= light_screen_pos.w;
	float light_distance = light_position.length3();

	if (relative_direction > 0)
	{
		_cutoutShader->perform();
		int  identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
		glBindBuffer(GL_ARRAY_BUFFER, identityVertex);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,_offlineRenderTexture->getDepthBuffer());
		glUniform1i(_cutoutDepthTextureLoc,0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D,_lightTexture->getName());
		glUniform1i(_cutoutColorTextureLoc,1);

		glUniform2f(_cutoutLightPositionScreenLoc, light_screen_pos.x, light_screen_pos.y);
		if(_cutoutOccluderParamLoc!=-1)
			glUniform4f(_cutoutOccluderParamLoc, light_distance, light_distance +_occluderParams.x,_occluderParams.y,1.0f);
		if(_cutoutNearQFarLoc!=-1)
			glUniform4fv(_cutoutNearQFarLoc,1,&_nearQFarRatio.x);
		glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	}
}

void  VolumLight::downSizeLightVolum()
{
	_downSizeShader->perform();
	int identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER,identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_craftRenderTexture->getColorBuffer());
	glUniform1i(_downSizeBaseTextureLoc,0);

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

void  VolumLight::fuzzyTexture(unsigned textureId)
{
	_fuzzyRenderTexture1->activeFramebuffer();
	_fuzzyShader->perform();
	int identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER,identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)(sizeof(float)*3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glUniform1i(_fuzzyBaseTextureLoc,0);
	/*
	  *横向模糊
	 */
	glUniform2f(_fuzzyPixelStepLoc,_pixelStepVec2.x,0.0f);
	glUniform1fv(_fuzzyWeightsLoc,6,_fuzzyWeights);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	/*
	  *纵向模糊
	 */
	_fuzzyRenderTexture2->activeFramebuffer();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_fuzzyRenderTexture1->getColorBuffer());
	glUniform1i(_fuzzyBaseTextureLoc,0);
	glUniform2f(_fuzzyPixelStepLoc,0,_pixelStepVec2.y);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

void  VolumLight::extrudeLightVolum()
{
	GLVector4  light_screen_pos = _lightPosition * _camera->getViewProjMatrix();
	light_screen_pos.x /= light_screen_pos.w;
	light_screen_pos.y /= light_screen_pos.w;
	light_screen_pos.z /= light_screen_pos.w;

	_fuzzyRenderTexture1->activeFramebuffer();
	_rayExtruderShader->perform();

	int  identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER,identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)(sizeof(float)*3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_fuzzyRenderTexture2->getColorBuffer());
	glUniform1i(_rayExtruderRayTextureLoc,0);

	glUniform2f(_rayExtruderRayLengthVec2Loc, _rayRadius * 0.25f,_nearQFarRatio.z);
	glUniform2f(_rayExtruderLightPositionScreenLoc, light_screen_pos.x, light_screen_pos.y);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	/*
	 *step2
	 */
	_fuzzyRenderTexture2->activeFramebuffer();
	glUniform2f(_rayExtruderRayLengthVec2Loc,_rayRadius * 0.5f,_nearQFarRatio.z);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_fuzzyRenderTexture1->getColorBuffer());
	glUniform1i(_rayExtruderRayTextureLoc,0);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	/*
	  *step3
	 */
	_fuzzyRenderTexture1->activeFramebuffer();
	glUniform2f(_rayExtruderRayLengthVec2Loc,_rayRadius * 0.75f,_nearQFarRatio.z);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _fuzzyRenderTexture2->getColorBuffer());
	glUniform1i(_rayExtruderRayTextureLoc,0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	/*
	  *step4
	 */
	_fuzzyRenderTexture2->activeFramebuffer();
	glUniform2f(_rayExtruderRayLengthVec2Loc,_rayRadius * 1.0f,_nearQFarRatio.z);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_fuzzyRenderTexture1->getColorBuffer());
	glUniform1i(_rayExtruderRayTextureLoc,0);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

void VolumLight::combineLightVolum()
{
	_combineShader->perform();
	int identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	glBindBuffer(GL_ARRAY_BUFFER, identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)(sizeof(float)*3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,_fuzzyRenderTexture2->getColorBuffer());
	glUniform1i(_combineRayTextureLoc,0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,_offlineRenderTexture->getColorBuffer());
	glUniform1i(_combineBaseTextureLoc,1);

	glUniform4fv(_combineRayColorLoc,1,&_rayColor.x);
	glUniform1f(_combineRayOpacityLoc,_rayOpacity);

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

void  VolumLight::debugTexture(unsigned textureId)
{
	GLProgram  *debugProgram = GLCacheManager::getInstance()->findGLProgram(GLCacheManager::GLProgramType_DebugNormalTexture);
	int  baseTextureLoc = debugProgram->getUniformLocation("g_BaseMap");
	int  mvpMatrixLoc = debugProgram->getUniformLocation("g_MVPMatrix");
	int  identityVertex = GLCacheManager::getInstance()->loadBufferIdentity();
	Matrix   identityMat;

	debugProgram->perform();
	glBindBuffer(GL_ARRAY_BUFFER,identityVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)(sizeof(float)*3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,textureId);
	glUniform1i(baseTextureLoc,0);

	glUniformMatrix4fv(mvpMatrixLoc,1,GL_FALSE,identityMat.pointer());

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

void  VolumLight::render()
{
	auto &winSize = GLContext::getInstance()->getWinSize();
	/*
	  *offline render texture
	  *似乎离线渲染后,输入的深度值的范围是[-1,1]
	 */
	_offlineRenderTexture->activeFramebuffer();
	renderGeometry();
	/*
	  *process
	 */
	_craftRenderTexture->activeFramebuffer();
	glDisable(GL_DEPTH_TEST);
	cutoutDepthVolum();
	/*
	  *down size
	 */
	glViewport(0,0, winSize.width*0.5f, winSize.height*0.5f);
	_downSizeRenderTexture->activeFramebuffer();
	downSizeLightVolum();
	/*
	  *fuzzy
	 */
	fuzzyTexture(_downSizeRenderTexture->getColorBuffer());
	/*
	  *extruder
	 */
	extrudeLightVolum();
	/*
	  *restore
	 */
	glViewport(0,0, winSize.width, winSize.height);
	_offlineRenderTexture->disableFramebuffer();
	combineLightVolum();

	//debugTexture(_fuzzyRenderTexture2->getColorBuffer());
	glEnable(GL_DEPTH_TEST);
}

void VolumLight::update(float t, float s)
{
	if (_keyCodeMask)
	{
		GLVector3 move;
		if (_keyCodeMask & 0x1)
			move += _camera->getForwardVector();
		if (_keyCodeMask & 0x2)
			move -= _camera->getForwardVector();
		if (_keyCodeMask & 0x4)
			move -= _camera->getXVector();
		if (_keyCodeMask & 0x8)
			move += _camera->getXVector();
		_camera->translate(move * t );
	}
}

bool VolumLight::onTouchBegan(const glk::GLVector2 &touchPoint)
{
	_touchPoint = touchPoint;

	return true;
}

void  VolumLight::onTouchMoved(const glk::GLVector2 &touchPoint)
{
	GLVector2  offsetPoint = touchPoint - _touchPoint;
	_camera->rotate(-offsetPoint.x,offsetPoint.y);

	_touchPoint = touchPoint;
}

void VolumLight::onTouchEnded(const glk::GLVector2 &touchPoint)
{

}

bool VolumLight::onKeyPressed(glk::KeyCodeType keyCode)
{
	if (keyCode == KeyCode_W)
		_keyCodeMask |= 1;
	if (keyCode == KeyCode_S)
		_keyCodeMask |= 2;
	if (keyCode == KeyCode_A)
		_keyCodeMask |= 4;
	if (keyCode == KeyCode_D)
		_keyCodeMask |= 8;
	return true;
}

void VolumLight::onKeyReleased(glk::KeyCodeType keyCode)
{
	if (keyCode == KeyCode_W)
		_keyCodeMask &= ~1;
	if (keyCode == KeyCode_S)
		_keyCodeMask &= ~2;
	if (keyCode == KeyCode_A)
		_keyCodeMask &= ~4;
	if (keyCode == KeyCode_D)
		_keyCodeMask &= ~8;
}