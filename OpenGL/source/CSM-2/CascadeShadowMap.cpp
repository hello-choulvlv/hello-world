/*
  *层叠阴影实现CSM
  *@date:2017-4-11
  *@Author:xiaohuaxiong
  */
#include"CascadeShadowMap.h"
#include<engine/GLContext.h>
#include<engine/event/EventManager.h>
#include<GL/glew.h>
#include<math.h>
#include<limits>
#include<stdio.h>
//从光源的角度或者摄像机的角度观察整个场景时的视角
#define _FOV_ANGLE_   60.0f
//CSM分割系数
#define _CSM_SPLIT_FACTOR_	0.5f
//视锥体分段数目
#define _FRUSTUM_SEGMENT_COUNT_ 4
//阴影贴图的大小
#define _LIGHT_MAP_SIZE_  2048
CascadeShadowMap::CascadeShadowMap()
{
	_lightShader = nullptr;
	_cameraShader = nullptr;
	_nearZ = 0.1f;
	_farZ = 400.0f;
}

CascadeShadowMap::~CascadeShadowMap()
{
	delete _lightShader;
	_lightShader = nullptr;
	delete _cameraShader;
	_cameraShader = nullptr;

	_groundMesh->release();
	_sphere->release();

	_groundTexture->release();
	_sphereTexture->release();
	_csmShadowArray->release();
	glk::EventManager::getInstance()->removeListener(_touchEvent);
	_touchEvent->release();
	glk::EventManager::getInstance()->removeListener(_keyEvent);
	_keyEvent->release();
}

CascadeShadowMap *CascadeShadowMap::createCascadeShadowMap()
{
	CascadeShadowMap *cascadeMap = new CascadeShadowMap();
	cascadeMap->initCascadeShadowMap();
	return cascadeMap;
}

void  CascadeShadowMap::initCascadeShadowMap()
{
	//投影矩阵
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	_projMatrix.identity();
	_projMatrix.perspective(_FOV_ANGLE_, winSize.width / winSize.height, _nearZ, _farZ);
	//
	initLight(glk::GLVector3(128.0f, 128.0f, 128.0f), glk::GLVector3(0.0f, 0.0f,0.0f/* -128.0f / 1.2f*/), glk::GLVector3(0.0f, 1.0f, 0.0f));
	initCamera(glk::GLVector3(0.0f, 128.0f/1.5f, 5.0f), glk::GLVector3(0.0f, 0.0f, -128.0f/1.2f), glk::GLVector3(0.0f, 1.0f, 0.0f));
	/*
	  *Shader initialize
	 */
	_lightShader = LightShader::createShaderWithSource("shader/csm-2/CSM_VS.glsl", "shader/csm-2/CSM_GS.glsl", "shader/csm-2/CSM_FS.glsl");
	_cameraShader = CameraShader::createCameraShader("shader/csm-2/Render_VS.glsl", "shader/csm-2/Render_FS.glsl");
	/*
	  *Mesh Object,Sphere Object
	 */
	_groundMesh = glk::Mesh::createWithIntensity(4.0f, 128.0f, 128.0f, 1.0f);

	_sphere = glk::Sphere::createWithSlice(128, 10.0f);

	_groundTexture = glk::GLTexture::createWithFile("tga/map/ground.tga");
	_sphereTexture = glk::GLTexture::createWithFile("tga/Earth512x256.tga");
	_groundTexture->retain();
	_sphereTexture->retain();
	//CSM Shadow Array
	_csmShadowArray = glk::ShadowMap::createWithMapLayer(glk::Size(_LIGHT_MAP_SIZE_,_LIGHT_MAP_SIZE_),_FRUSTUM_SEGMENT_COUNT_);
	/*
	  *计算分段视锥体的分段远平面
	 */
	updateFrustumSegment();
	/*
	  *计算光空间视锥体分割,以及分层的适口范围
	 */
	//updateLightViewFrustum();
	//注册事件
	_touchEvent = glk::TouchEventListener::createTouchListener(this, 
		glk_touch_selector(CascadeShadowMap::onTouchBegan),
		glk_move_selector(CascadeShadowMap::onTouchMoved),
		glk_release_selector(CascadeShadowMap::onTouchEnded)
		);
	glk::EventManager::getInstance()->addTouchEventListener(_touchEvent,0);
	//注册键盘监听事件
	_keyEvent = glk::KeyEventListener::createKeyEventListener(this, glk_key_press_selector(CascadeShadowMap::onKeyPressed), 
		glk_key_release_selector(CascadeShadowMap::onKeyReleased));
	glk::EventManager::getInstance()->addKeyEventListener(_keyEvent, 0);
}
//计算由远近平面以及光源的视图矩阵决定的包围盒的大小
void CascadeShadowMap::frustumBoudingboxInLightSpaceView(const float nearZ, const float farZ, glk::GLVector4 &boxMin, glk::GLVector4 &boxMax)
{
	const float tanOfFov = tanf(_FOV_ANGLE_*_RADIUS_FACTOR_*0.5f);
	//计算横纵比
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	const float	whRate = winSize.width/winSize.height;
	//计算近平面的宽高
	const float  nearHeight = 2.0f * nearZ * tanOfFov;
	const float  nearWidth = nearHeight * whRate;
	//计算远平面的宽高
	const float  farHeight =2.0f *  farZ * tanOfFov;
	const float  farWidth = farHeight *whRate;
	//将摄像机的视图矩阵分解成旋转矩阵以及平移矩阵
	glk::Matrix     inverseMatrix = _cameraViewMatrix.reverse();
	//求摄像机的位置，此位置包括以下求取的向量，全部都是以世界坐标系来计算的
	glk::GLVector4     cameraPosition = glk::GLVector4(0.0f,0.0f,0.0f,1.0f) * inverseMatrix;
	//求摄像机的各个方向,前向,右向,上方,注意齐次坐标的使用方法,以及向量与矩阵的乘法规则
	glk::GLVector4      forwardVec = glk::GLVector4(0.0f,0.0f,-1.0f,0.0f) * inverseMatrix;
	glk::GLVector4      rightVec = glk::GLVector4(1.0f,0.0f,0.0f,0.0f) * inverseMatrix;
	glk::GLVector4      upVec = glk::GLVector4(0.0f, 1.0f, 0.0f, 0.0f)*inverseMatrix;
	//计算视锥体的8个3维坐标点
	const glk::GLVector4   nearCenter = cameraPosition + forwardVec * nearZ;
	const glk::GLVector4   farCenter = cameraPosition + forwardVec*farZ;
    glk::GLVector4     frustumLocation[8]= {
		nearCenter -  rightVec * 0.5f * nearWidth  - upVec * 0.5f * nearHeight,//near left bottom
		nearCenter + rightVec * 0.5f *nearWidth -  upVec * 0.5f * nearHeight,//near right bottom
		nearCenter - rightVec *0.5f * nearWidth  + upVec * 0.5f * nearHeight,//near left up
		nearCenter + rightVec*0.5f * nearWidth +  upVec * 0.5f * nearHeight,//near right up

		farCenter  - rightVec *0.5f  * farWidth -  upVec   *  0.5f  * farHeight,//far left bottom
		farCenter + rightVec *0.5f  * farWidth -  upVec   *  0.5f  * farHeight,//far right bottom
		farCenter - rightVec  *0.5f  * farWidth + upVec   *  0.5f  * farHeight,//far left up
		farCenter + rightVec *0.5f  * farWidth + upVec   *  0.5f *  farHeight,//far right up
	};
	//计算最小视锥体包围盒
	const float max_float =  std::numeric_limits<float>::max();
	const float min_float = std::numeric_limits<float>::lowest();
	boxMin = glk::GLVector4(max_float, max_float, max_float, max_float);
	boxMax = glk::GLVector4(min_float, min_float, min_float, min_float);
	for (int j = 0; j < 8; ++j)
	{
		//计算在光空间中的坐标
		glk::GLVector4     lightSpaceVec = frustumLocation[j] * _lightViewMatrix;
		boxMin = boxMin.min(lightSpaceVec);
		boxMax = boxMax.max(lightSpaceVec);
	}
}

void    CascadeShadowMap::buildCropMatrix(const glk::GLVector3 &maxCorner, const glk::GLVector3 &minCorner,glk::Matrix &cropMatrix)
{
	/*
	  *注意scaleZ的设定与其他的不同
	 */
	const float scaleX = 2.0f/(maxCorner.x - minCorner.x);
	const float scaleY = 2.0f / (maxCorner.y - minCorner.y);
	const float scaleZ = 1.0f / (maxCorner.z - minCorner.z);

	cropMatrix.identity();
	cropMatrix.translate(-(maxCorner.x+minCorner.x)*0.5f,-(maxCorner.y+minCorner.y)*0.5f,-(minCorner.z+maxCorner.z)*0.5f);
	cropMatrix.scale(scaleX,scaleY,scaleZ);
}

void    CascadeShadowMap::calculateCropMatrix(const glk::GLVector3 &maxCorner, const glk::GLVector3 &minCorner, glk::Matrix &cropMatrix)
{
	/*
	  *求包围盒的裁剪坐标
	 */
	//因为maxCorner/minCorner的坐标已经是光空间的坐标，所以只需要经过投影变换即可
	const glk::GLVector4 lightSpaceCoord[8] = {
		glk::GLVector4(minCorner.x,minCorner.y,minCorner.z,1.0f),//bottom left nearZ
		glk::GLVector4(minCorner.x,maxCorner.y,minCorner.z,1.0f),//top left nearZ
		glk::GLVector4(minCorner.x,maxCorner.y,maxCorner.z,1.0f),//top left farZ
		glk::GLVector4(minCorner.x,minCorner.y,maxCorner.z,1.0f),//bottom left farZ

		glk::GLVector4(maxCorner.x,minCorner.y,maxCorner.z,1.0f),//right bottom farZ
		glk::GLVector4(maxCorner.x,maxCorner.y,maxCorner.z,1.0f),//right top farZ
		glk::GLVector4(maxCorner.x,maxCorner.y,minCorner.z,1.0f),//right top nearZ
		glk::GLVector4(maxCorner.x,minCorner.y,minCorner.z,1.0f),//right  bottom nearZ
	};
	//const float max_float = 0x7FFFFFFF;
	//const float min_float = -0x7FFFFFFF;
	glk::GLVector4   clipMinCorner = lightSpaceCoord[0] * _lightProjMatrix;
	glk::GLVector4   clipMaxCorner= lightSpaceCoord[0] * _lightProjMatrix;
	for (int i = 1; i < 8; ++i)
	{
		glk::GLVector4   clipVertex = lightSpaceCoord[i] * _lightProjMatrix ;
		//clip coord
		clipVertex.x /= clipVertex.w;
		clipVertex.y /= clipVertex.w;
		clipVertex.z /= clipVertex.w;
		//
		clipMinCorner = clipMinCorner.min(clipVertex);
		clipMaxCorner = clipMaxCorner.max(clipVertex);
	}
	clipMinCorner.z = 0.0f;
	//
	buildCropMatrix(clipMaxCorner.xyz(), clipMinCorner.xyz(),cropMatrix);
}
void		CascadeShadowMap::initCamera(const glk::GLVector3 &eyePosition, const glk::GLVector3 &viewPosition, const glk::GLVector3 &upVector)
{
	_eyePosition = eyePosition;
	_cameraViewMatrix.identity();
	_cameraViewMatrix.lookAt(eyePosition, viewPosition, upVector);
	_cameraViewPorjMatrix = _cameraViewMatrix * _projMatrix;
	const float *mat = _cameraViewMatrix.pointer();
	//计算欧拉角以及旋转向量,下面的公式得出的原因是roll-yaw-pitch旋转矩阵相乘
	//具体请参见 https://www.physicsforums.com/threads/decomposition-of-a-rotation-matrix.623740/
	const float pitch = atan2f(-mat[2*4+1],mat[2*4+2]);
	const float yaw = atan2f(mat[2*4+0],glk::GLVector2(-mat[2*4+1],mat[2*4+2]).length());
	//
	_rotateVector = glk::GLVector3(mat[3*4+0],mat[3*4+1],mat[3*4+2]);
	//逆向求旋转之前的向量
	glk::Matrix matX;
	matX.rotateX(-pitch*_ANGLE_FACTOR_);
	glk::Matrix matY;
	matY.rotateY(-yaw*_ANGLE_FACTOR_);
	glk::GLVector4 rotateVec4 = glk::GLVector4(_rotateVector.x,_rotateVector.y,_rotateVector.z,0.0f) * matX;
	rotateVec4 = rotateVec4 * matY;
	//旋转向量
	_rotateVector = glk::GLVector3(rotateVec4.x, rotateVec4.y, rotateVec4.z);
	glk::GLVector3   targetVec = viewPosition - eyePosition;
	//欧拉角
	_pitchYawRoll = glk::GLVector3(pitch,yaw,0.0f);
}

void CascadeShadowMap::initLight(const glk::GLVector3 &lightPosition, const glk::GLVector3 &lightViewPosition, const glk::GLVector3 &upVector)
{
	_lightDirection = (lightViewPosition - lightPosition).normalize();
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	_lightViewMatrix.identity();
	_lightViewMatrix.lookAt(lightPosition, lightViewPosition, upVector);
	/*
	  *计算光源的投影矩阵
	 */
	_lightProjMatrix.identity();
	const float height = 2.0f * tanf(_FOV_ANGLE_ *_RADIUS_FACTOR_ * 0.5f) * _farZ;
	const float width = height * winSize.width / winSize.height;
	_lightProjMatrix.orthoProject(-width*0.5f,width*0.5f,-height*0.5f,height*0.5f,_nearZ,_farZ);
	_lightViewProjMatrix = _lightViewMatrix * _lightProjMatrix;
}

//计算各个平面的视口范围,以及分割的视锥体的光空间正交偏移缩放矩阵
void CascadeShadowMap::updateLightViewFrustum()
{
	float nearZ = _nearZ;
	glk::GLVector4 minCorner, maxCorner;
	//求分段的视锥体的CropMatrix
	for (int i = 0; i < _FRUSTUM_SEGMENT_COUNT_; ++i)
	{
		frustumBoudingboxInLightSpaceView(nearZ, _farSegemtns[i], minCorner, maxCorner);
		//计算裁剪矩阵
		calculateCropMatrix(maxCorner.xyz(), minCorner.xyz(), _cropMatrix[i]);
		_cropTextureMatrix[i] = _lightViewProjMatrix * _cropMatrix[i];
		_cropTextureMatrix[i].offset();
		//_cropTextureMatrix[i].scale(0.5f,0.5f,0.5f);
		//_cropTextureMatrix[i].translate(0.5f, 0.5f, 0.5f);
		nearZ = _farSegemtns[i];
	}
}

void CascadeShadowMap::updateFrustumSegment()
{
	float *frustumSegment = static_cast<float*>(&_normalSegments.x);
	for (int i = 1; i < _FRUSTUM_SEGMENT_COUNT_ + 1; ++i)
	{
		const float splitFactor = 1.0f*i/_FRUSTUM_SEGMENT_COUNT_;
		const float normalCoeffcient = _nearZ * powf(_farZ/_nearZ,splitFactor);
		const float correntCoeffcient = _nearZ +(_farZ-_nearZ)*splitFactor;
		const float finalCoeffcient = normalCoeffcient * _CSM_SPLIT_FACTOR_ + (1.0f -_CSM_SPLIT_FACTOR_)*correntCoeffcient;
		_farSegemtns[i - 1] = finalCoeffcient;

		const glk::GLVector4 farCoeffcient = glk::GLVector4(0.0f,0.0f,-finalCoeffcient,1.0f) * _projMatrix;
		//转换成规范化坐标
		frustumSegment[i - 1] = farCoeffcient.z / farCoeffcient.w * 0.5f + 0.5f;
	}
}

void CascadeShadowMap::renderCameraView()
{
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	glViewport(0, 0,winSize.width,winSize.height );
	glDisable(GL_POLYGON_OFFSET_FILL);
	// Clear render target.
	glClear(GL_DEPTH_BUFFER_BIT);
	const float worldWidth = 128.0f;
	const float worldHeight = -250.0f;
	const float sphereY =  20.0f;
	//shader program
	_cameraShader->perform();
	_cameraShader->setViewProjMatrix(_cameraViewPorjMatrix);
	_cameraShader->setLightDirection(_lightDirection);
	_cameraShader->setEyePosition(_eyePosition);
	_cameraShader->setCropMatrix(_cropTextureMatrix,_FRUSTUM_SEGMENT_COUNT_);
	_cameraShader->setNormalSegments(_normalSegments);
	_cameraShader->setBaseMap(_sphereTexture->name(),0);
	_cameraShader->setShadowMapArray(_csmShadowArray->getDepthTexture(),1);
	//计算几何体,5 x 5 几何体阵列
	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 5; ++j)
		{
			glk::Matrix     transformMatrix;
			transformMatrix.translate(((i+1) / 6.0f *2.0f - 1.0f)*worldWidth,sphereY,(j+1)*worldHeight/6.0f);
			//glk::Matrix     modelViewProjMatrix = transformMatrix * _cameraViewPorjMatrix;
			glk::Matrix3   normalMatrix = transformMatrix.normalMatrix();
			_cameraShader->setModelMatrix(transformMatrix);
			_cameraShader->setNormalMatrix(normalMatrix);
			_sphere->bindVertexObject(0);
			_sphere->bindNormalObject(1);
			_sphere->bindTexCoordObject(2);
			_sphere->drawShape();
		}
	}
	//地面网格
	glk::Matrix modelMatrix;
	modelMatrix.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
	modelMatrix.translate(0.0f, 0.0f, -128.0f);
	glk::Matrix3  normalMatrix = modelMatrix.normalMatrix();
	_cameraShader->setModelMatrix(modelMatrix);
	//const glk::GLVector3 _normalVector = glk::GLVector3(0.0f,0.0f,1.0f)*normalMatrix;
	_cameraShader->setNormalMatrix(normalMatrix);
	_groundMesh->bindVertexObject(0);
	_groundMesh->bindNormalObject(1);
	_groundMesh->bindTexCoordObject(2);
	//
	_cameraShader->setBaseMap(_groundTexture->name(), 0);
	_groundMesh->drawShape();
}
/*
  *渲染阴影阵列,层渲染
 */
void CascadeShadowMap::renderLightView()
{
	_csmShadowArray->activeShadowFramebuffer();
	glEnable(GL_POLYGON_OFFSET_FILL);
	glColorMask(0, 0, 0, 0);

	updateLightViewFrustum();
	_lightShader->perform();
	//所有几何体都需要的公共数据
	_lightShader->setCropMatrix(_cropMatrix,_FRUSTUM_SEGMENT_COUNT_);
	//渲染几何体
	const float worldWidth = 128.0f;
	const float worldHeight = -250.0f;
	const float sphereY = 20.0f;
	//
	//计算几何体,5 x 5 几何体阵列
	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 5; ++j)
		{
			glk::Matrix     transformMatrix;
			transformMatrix.translate(((i + 1) / 6.0f *2.0f - 1.0f)*worldWidth, sphereY, (j + 1)*worldHeight / 6.0f);
			glk::Matrix     modelViewProjMatrix = transformMatrix * _lightViewMatrix * _lightProjMatrix;
			_lightShader->setMVPMatrix(modelViewProjMatrix);
			_sphere->bindVertexObject(0);
			_sphere->drawShape();
		}
	}
	//地面网格
	glk::Matrix modelMatrix;
	modelMatrix.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
	modelMatrix.translate(0.0f, 0.0f, -128.0f);
	glk::Matrix mvpMatrix = modelMatrix * _lightViewProjMatrix;
	_lightShader->setMVPMatrix(mvpMatrix);
	_groundMesh->bindVertexObject(0);
	_groundMesh->drawShape();
	//
	glColorMask(1, 1, 1, 1);
	glDisable(GL_POLYGON_OFFSET_FILL);
	_csmShadowArray->restoreFramebuffer();
}

void CascadeShadowMap::update(const float deltaTime)
{
	//计算视图矩阵
	_translateVec.x = 15.0f * _velocityVec.x ;
	_translateVec.z = 15.0f * _velocityVec.z ;
	//还原成世界坐标系下的坐标
	glk::GLVector4   originVec = glk::GLVector4(-_translateVec.x, 0.0f, -_translateVec.z, 0.0f) * _cameraViewMatrix.reverse();
	_rotateVector.x += originVec.x*deltaTime;
	_rotateVector.y += originVec.y * deltaTime;
	_rotateVector.z += originVec.z * deltaTime;
	_cameraViewMatrix.identity();
	_cameraViewMatrix.translate(_rotateVector.x, _rotateVector.y, _rotateVector.z);
	_cameraViewMatrix.rotateY(_pitchYawRoll.y*_ANGLE_FACTOR_);
	_cameraViewMatrix.rotateX(_pitchYawRoll.x*_ANGLE_FACTOR_);
	_cameraViewPorjMatrix = _cameraViewMatrix *_projMatrix;
}

bool  CascadeShadowMap::onTouchBegan(const glk::GLVector2 *touchPoint)
{
	_offset = *touchPoint;
	return true;
}

void CascadeShadowMap::onTouchMoved(const glk::GLVector2 *touchPoint)
{
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	_pitchYawRoll.x += -(touchPoint->y - _offset.y) / winSize.height*__MATH_PI__*0.6f;
	_pitchYawRoll.y += (touchPoint->x - _offset.x)/winSize.width*__MATH_PI__ * 0.6f;
	_offset = *touchPoint;
}

void CascadeShadowMap::onTouchEnded(const glk::GLVector2 *touchPoint)
{
	_offset = *touchPoint;
}

bool CascadeShadowMap::onKeyPressed(const glk::KeyCodeType keyCode)
{
	if (keyCode == glk::KeyCodeType::KeyCode_W)
		_velocityVec.z = -1.0f;
	else if (keyCode == glk::KeyCodeType::KeyCode_S)
		_velocityVec.z = 1.0f;
	else if (keyCode == glk::KeyCodeType::KeyCode_A)
		_velocityVec.x = -1.0f;
	else if (keyCode == glk::KeyCodeType::KeyCode_D)
		_velocityVec.x = 1.0f;
	return true;
}

void CascadeShadowMap::onKeyReleased(const glk::KeyCodeType keyCode)
{
	if (keyCode == glk::KeyCodeType::KeyCode_W)
		_velocityVec.z = 0.0f;
	else if (keyCode == glk::KeyCodeType::KeyCode_S)
		_velocityVec.z = 0.0f;
	else if (keyCode == glk::KeyCodeType::KeyCode_A)
		_velocityVec.x = 0.0f;
	else if (keyCode == glk::KeyCodeType::KeyCode_D)
		_velocityVec.x = 0.0f;
}