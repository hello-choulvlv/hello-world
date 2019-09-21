/*
  *�����Ӱʵ��CSM
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
//�ӹ�Դ�ĽǶȻ���������ĽǶȹ۲���������ʱ���ӽ�
#define _FOV_ANGLE_   60.0f
//CSM�ָ�ϵ��
#define _CSM_SPLIT_FACTOR_	0.5f
//��׶��ֶ���Ŀ
#define _FRUSTUM_SEGMENT_COUNT_ 4
//��Ӱ��ͼ�Ĵ�С
#define _LIGHT_MAP_SIZE_  1024.0f
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
	glk::EventManager::getInstance()->removeListener(this);
}

CascadeShadowMap *CascadeShadowMap::createCascadeShadowMap()
{
	CascadeShadowMap *cascadeMap = new CascadeShadowMap();
	cascadeMap->initCascadeShadowMap();
	return cascadeMap;
}

void  CascadeShadowMap::initCascadeShadowMap()
{
	//ͶӰ����
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	_projMatrix.identity();
	_projMatrix.perspective(_FOV_ANGLE_, winSize.width / winSize.height, _nearZ, _farZ);
	//
	initLight(glk::GLVector3(128.0f, 128.0f, 128.0f), glk::GLVector3(0.0f, 0.0f,0.0f/* -128.0f / 1.2f*/), glk::GLVector3(0.0f, 1.0f, 0.0f));
	initCamera(glk::GLVector3(0.0f, 128.0f/1.5f, 5.0f), glk::GLVector3(0.0f, 0.0f, -128.0f/1.2f), glk::GLVector3(0.0f, 1.0f, 0.0f));
	/*
	  *Shader initialize
	 */
	_lightShader = LightShader::createShaderWithSource("shader/csm/CSM_VS.glsl", "shader/csm/CSM_GS.glsl", "shader/csm/CSM_FS.glsl");
	_cameraShader = CameraShader::createCameraShader("shader/csm/Render_VS.glsl", "shader/csm/Render_FS.glsl");
	/*
	  *Mesh Object,Sphere Object
	 */
	_groundMesh = glk::Mesh::createWithIntensity(4.0f, 128.0f, 128.0f, 1.0f);

	_sphere = glk::Sphere::createWithSlice(128, 10.0f);

	_groundTexture = glk::GLTexture::createWithFile("tga/map/ground.tga");
	_sphereTexture = glk::GLTexture::createWithFile("tga/Earth512x256.tga");
	//CSM Shadow Array
	_csmShadowArray = glk::ShadowMap::createWithMapLayer(glk::Size(_LIGHT_MAP_SIZE_,_LIGHT_MAP_SIZE_),_FRUSTUM_SEGMENT_COUNT_);
	/*
	  *����ֶ���׶��ķֶ�Զƽ��
	 */
	updateFrustumSegment();
	/*
	  *�����ռ���׶��ָ�,�Լ��ֲ���ʿڷ�Χ
	 */
	//updateLightViewFrustum();
	//ע���¼�
	_touchEvent = glk::TouchEventListener::createTouchListener(this, 
		glk_touch_selector(CascadeShadowMap::onTouchBegan),
		glk_move_selector(CascadeShadowMap::onTouchMoved),
		glk_release_selector(CascadeShadowMap::onTouchEnded)
		);
	glk::EventManager::getInstance()->addTouchEventListener(_touchEvent,0);
}
//������Զ��ƽ���Լ���Դ����ͼ��������İ�Χ�еĴ�С
void CascadeShadowMap::frustumBoudingboxInLightSpaceView(const float nearZ, const float farZ, glk::GLVector4 &boxMin, glk::GLVector4 &boxMax)
{
	//const float tanOfFov = tanf(_FOV_ANGLE_*_RADIUS_FACTOR_*0.5f);
	////������ݱ�
	//auto &winSize = glk::GLContext::getInstance()->getWinSize();
	//const float	whRate = winSize.width/winSize.height;
	////�����ƽ��Ŀ��
	//const float  nearHeight = 2.0f * nearZ * tanOfFov;
	//const float  nearWidth = nearHeight * whRate;
	////����Զƽ��Ŀ��
	//const float  farHeight =2.0f *  farZ * tanOfFov;
	//const float  farWidth = farHeight *whRate;
	////�����������ͼ����ֽ����ת�����Լ�ƽ�ƾ���
	//glk::Matrix     inverseMatrix = _cameraViewMatrix.reverse();
	////���������λ�ã���λ�ð���������ȡ��������ȫ����������������ϵ�������
	//glk::GLVector4     cameraPosition = glk::GLVector4(0.0f,0.0f,0.0f,1.0f) * inverseMatrix;
	////��������ĸ�������,ǰ��,����,�Ϸ�,ע����������ʹ�÷���,�Լ����������ĳ˷�����
	//glk::GLVector4      forwardVec = glk::GLVector4(0.0f,0.0f,-1.0f,0.0f) * inverseMatrix;
	//glk::GLVector4      rightVec = glk::GLVector4(1.0f,0.0f,0.0f,0.0f) * inverseMatrix;
	//glk::GLVector4      upVec = glk::GLVector4(0.0f, 1.0f, 0.0f, 0.0f)*inverseMatrix;
	////������׶���8��3ά�����
	//const glk::GLVector4   nearCenter = cameraPosition + forwardVec * nearZ;
	//const glk::GLVector4   farCenter = cameraPosition + forwardVec*farZ;
 //   glk::GLVector4     frustumLocation[8]= {
	//	nearCenter -  rightVec * 0.5f * nearWidth  - upVec * 0.5f * nearHeight,//near left bottom
	//	nearCenter + rightVec * 0.5f *nearWidth -  upVec * 0.5f * nearHeight,//near right bottom
	//	nearCenter - rightVec *0.5f * nearWidth  + upVec * 0.5f * nearHeight,//near left up
	//	nearCenter + rightVec*0.5f * nearWidth +  upVec * 0.5f * nearHeight,//near right up

	//	farCenter  - rightVec *0.5f  * farWidth -  upVec   *  0.5f  * farHeight,//far left bottom
	//	farCenter + rightVec *0.5f  * farWidth -  upVec   *  0.5f  * farHeight,//far right bottom
	//	farCenter - rightVec  *0.5f  * farWidth + upVec   *  0.5f  * farHeight,//far left up
	//	farCenter + rightVec *0.5f  * farWidth + upVec   *  0.5f *  farHeight,//far right up
	//};
	////������С��׶���Χ��
	//const float max_float =  std::numeric_limits<float>::max();
	//const float min_float = std::numeric_limits<float>::lowest();
	//boxMin = glk::GLVector4(max_float, max_float, max_float, max_float);
	//boxMax = glk::GLVector4(min_float, min_float, min_float, min_float);
	//for (int j = 0; j < 8; ++j)
	//{
	//	//�����ڹ�ռ��е�����
	//	//frustumLocation[j].w = 1.0f;
	//	glk::GLVector4     lightSpaceVec = frustumLocation[j] * _lightViewMatrix;
	//	boxMin = boxMin.min(lightSpaceVec);
	//	boxMax = boxMax.max(lightSpaceVec);
	//}


	const float max_float = std::numeric_limits<float>::max();
	const float min_float = std::numeric_limits<float>::lowest();
	glk::GLVector4 frustumMin(max_float, max_float, max_float, max_float);
	glk::GLVector4 frustumMax(min_float, min_float, min_float, min_float);
	/*
	*�����۾�ʵ�ʿ����ĳ�������������ϵ�µķ�Χ
	*/
	const float tanOfFov = tanf(_FOV_ANGLE_*_RADIUS_FACTOR_*0.5f);
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	const float	whRate = winSize.width/winSize.height;
	const float nearHeight = 2.0f * tanOfFov * nearZ;
	const float nearWidth = nearHeight * whRate;
	const float farHeight = 2.0f * tanOfFov * farZ;
	const float farWidth = farHeight * whRate;

	glk::Matrix    inverseMatrix = _cameraViewMatrix.reverse();
	const glk::GLVector4 originPosition = glk::GLVector4(0.0f, 0.0f, 0.0f, 1.0f) * inverseMatrix;
	const glk::GLVector4 originViewDir = glk::GLVector4(0.0f, 0.0f, -1.0f, 0.0f) * inverseMatrix;
	const glk::GLVector4 originUpDir = glk::GLVector4(0.0f, 1.0f, 0.0f, 0.0f) * inverseMatrix;
	const glk::GLVector4 originRightDir = glk::GLVector4(1.0f, 0.0f, 0.0f, 0.0f) * inverseMatrix;

	const glk::GLVector4 nc = originPosition + originViewDir * nearZ; // near center
	const glk::GLVector4 fc = originPosition + originViewDir * farZ; // far center
																		 //std::function<void(float)>  m_func;
																		 // Vertices in a world space.
																		 //�����۾�����������ռ��е���׶������
	glk::GLVector4 vertices[8] = {
		nc - originUpDir * nearHeight * 0.5f - originRightDir * nearWidth * 0.5f, // nbl (near, bottom, left)
		nc - originUpDir * nearHeight * 0.5f + originRightDir * nearWidth * 0.5f, // nbr
		nc + originUpDir * nearHeight * 0.5f + originRightDir * nearWidth * 0.5f, // ntr
		nc + originUpDir * nearHeight * 0.5f - originRightDir * nearWidth * 0.5f, // ntl
		fc - originUpDir * farHeight  * 0.5f - originRightDir * farWidth * 0.5f, // fbl (far, bottom, left)
		fc - originUpDir * farHeight  * 0.5f + originRightDir * farWidth * 0.5f, // fbr
		fc + originUpDir * farHeight  * 0.5f + originRightDir * farWidth * 0.5f, // ftr
		fc + originUpDir * farHeight  * 0.5f - originRightDir * farWidth * 0.5f, // ftl
	};

	for (unsigned int vertId = 0; vertId < 8; ++vertId) {
		// Light view space.
		vertices[vertId] = vertices[vertId] * _lightViewMatrix;
		// Update bounding box.
		frustumMin = frustumMin.min(vertices[vertId]);
		frustumMax = frustumMax.max(vertices[vertId]);
	}

	boxMin = frustumMin;
	boxMax = frustumMax;
}

void		CascadeShadowMap::initCamera(const glk::GLVector3 &eyePosition, const glk::GLVector3 &viewPosition, const glk::GLVector3 &upVector)
{
	_eyePosition = eyePosition;
	_cameraViewMatrix.identity();
	_cameraViewMatrix.lookAt(eyePosition, viewPosition, upVector);
	_cameraViewPorjMatrix = _cameraViewMatrix * _projMatrix;
	const float *mat = _cameraViewMatrix.pointer();
	//����ŷ�����Լ���ת����,����Ĺ�ʽ�ó���ԭ����roll-yaw-pitch��ת�������
	//������μ� https://www.physicsforums.com/threads/decomposition-of-a-rotation-matrix.623740/
	const float pitch = atan2f(-mat[2*4+1],mat[2*4+2]);
	const float yaw = atan2f(mat[2*4+0],glk::GLVector2(-mat[2*4+1],mat[2*4+2]).length());
	//
	_rotateVector = glk::GLVector3(mat[3*4+0],mat[3*4+1],mat[3*4+2]);
	//��������ת֮ǰ������
	glk::Matrix matX;
	matX.rotateX(-pitch*_ANGLE_FACTOR_);
	glk::Matrix matY;
	matY.rotateY(-yaw*_ANGLE_FACTOR_);
	glk::GLVector4 rotateVec4 = glk::GLVector4(_rotateVector.x,_rotateVector.y,_rotateVector.z,0.0f) * matX;
	rotateVec4 = rotateVec4 * matY;
	//��ת����
	_rotateVector = glk::GLVector3(rotateVec4.x, rotateVec4.y, rotateVec4.z);
	glk::GLVector3   targetVec = viewPosition - eyePosition;
	//ŷ����
	_pitchYawRoll = glk::GLVector3(pitch,yaw,0.0f);
}

void CascadeShadowMap::initLight(const glk::GLVector3 &lightPosition, const glk::GLVector3 &lightViewPosition, const glk::GLVector3 &upVector)
{
	_lightDirection = (lightViewPosition - lightPosition).normalize();
	auto &winSize = glk::GLContext::getInstance()->getWinSize();
	_lightViewMatrix.identity();
	_lightViewMatrix.lookAt(lightPosition, lightViewPosition, upVector);
}

//�������ƽ����ӿڷ�Χ,�Լ��ָ����׶��Ĺ�ռ�����ƫ�����ž���
void CascadeShadowMap::updateLightViewFrustum()
{
#ifndef __USE_ORIGIN_
	//��Χ�еĴ�С,���������ӿ����ű���
	glk::GLVector4  frustumMin, frustumMax;
	frustumBoudingboxInLightSpaceView(_nearZ, _farZ, frustumMin, frustumMax);
	_lightProjMatrix.identity();
	_lightProjMatrix.orthoProject(frustumMin.x,frustumMax.x,frustumMin.y,frustumMax.y,0.0f,-frustumMin.z);
	//�ֶε���׶��İ�Χ��
	glk::GLVector4  segmentMin, segmentMax;
	float   lastSegment = _nearZ;
	glk::GLVector2 frustumSize(frustumMax.x-frustumMin.x,frustumMax.y-frustumMin.y);
	//��ȡ����ʿڷ�Χ
	glk::GLVector2 viewportDimention;
	glGetFloatv(GL_MAX_VIEWPORT_DIMS, &viewportDimention.x);
	for (int i = 0; i < _FRUSTUM_SEGMENT_COUNT_; ++i)
	{
		frustumBoudingboxInLightSpaceView(lastSegment, _farSegemtns[i], segmentMin, segmentMax);
		//�������½ǵ����꣬�Լ�����֮���ƫ��
		glk::GLVector2   bottomLeftVec(segmentMin.x-frustumMin.x,segmentMin.y-frustumMin.y);
		//X,Y����Ŀ��
		const float segmentX = segmentMax.x - segmentMin.x;
		const float segmentY = segmentMax.y - segmentMin.y;
		//�������½ǵ�ƫ��ռ�������ֶε���׶��ı���
		glk::GLVector2 segmentOffsetScale(bottomLeftVec.x/segmentX,bottomLeftVec.y/segmentY);
		//������׶����ֶ���׶��Ŀ��֮��ı���
		glk::GLVector2 frustumSegmentScale(frustumSize.x/segmentX, frustumSize.y/segmentY);
		//�����ϵó��ı�����ϵ��չ����Ӱ��ͼ��ʵ�ʳߴ�֮��
		glk::GLVector2 pixelSegmentOffset= segmentOffsetScale*_LIGHT_MAP_SIZE_;
		glk::GLVector2 pixelFrustumSegment = frustumSegmentScale * _LIGHT_MAP_SIZE_;
		//���õ���������׶��ķ�Χ������ʿڷ�Χ���Ƚ�,���ʵ�ʵ��ʿڷ�Χ��������ʿڷ�Χ���������Ӧ�����Ų���
		glk::GLVector2 scaleFactor(
			viewportDimention.x>pixelFrustumSegment.x?1.0f:viewportDimention.x/pixelFrustumSegment.x,
			viewportDimention.y>pixelFrustumSegment.y?1.0f:viewportDimention.y/pixelFrustumSegment.y
			);
		//�����ӿڵ����½�����,�Լ���صĳߴ�
		_viewport[i] = glk::GLVector4(
				-pixelSegmentOffset.x*scaleFactor.x,
				-pixelSegmentOffset.y*scaleFactor.y,
				pixelFrustumSegment.x * scaleFactor	.x,
				pixelFrustumSegment.y * scaleFactor.y
			);
		glViewportIndexedfv(i, &_viewport[i].x);
		//����ֶ���׶��Ĺ�ռ���ͼͶӰ����ƫ�ƾ���VPSB-Matrix
		_lightVPSBMatrix[i].identity();
		_lightVPSBMatrix[i].multiply(_lightViewMatrix);
		_lightVPSBMatrix[i].orthoProject(segmentMin.x, segmentMax.x, segmentMin.y, segmentMax.y, 0.0f, -frustumMin.z);
		_lightVPSBMatrix[i].scale(scaleFactor.x*0.5f, scaleFactor.y*0.5f, 0.5f);
		_lightVPSBMatrix[i].translate(scaleFactor.x*0.5f,scaleFactor.y*0.5f,0.5f);
		//����Ĵ��������ȥ֮������һЩ���⣬���ڽ����
		//	lastSegment = _farSegemtns[i];
	}
#else
	// Find a bounding box of whole camera frustum in light view space.
	//nv::vec4f frustumMin(std::numeric_limits<float>::max());
	// nv::vec4f frustumMax(std::numeric_limits<float>::lowest());
	glk::GLVector4 frustumMin, frustumMax;
	frustumBoudingboxInLightSpaceView(_nearZ, _farZ, frustumMin, frustumMax);

	// Update light projection matrix to only cover the area viewable by the camera
	_lightProjMatrix.identity();
	_lightProjMatrix.orthoProject(frustumMin.x, frustumMax.x, frustumMin.y, frustumMax.y, 0.0f, -frustumMin.z);
	//nv::ortho3D(lightProj, segmentMin.x, segmentMin.x + segmentSize, segmentMin.y, segmentMin.y + segmentSize, 0.0f, frustumMin.z);

	glk::GLVector2 m_viewportDims;
	glGetFloatv(GL_MAX_VIEWPORT_DIMS, &m_viewportDims.x);
	// Find a bounding box of segment in light view space.
	float nearSegmentPlane = 0.1f;
	//��׶��ָ�
	for (unsigned int i = 0; i < _FRUSTUM_SEGMENT_COUNT_; ++i) {
		//nv::vec4f segmentMin(std::numeric_limits<float>::max());
		// nv::vec4f segmentMax(std::numeric_limits<float>::lowest());
		glk::GLVector4 segmentMin, segmentMax;
		frustumBoudingboxInLightSpaceView(nearSegmentPlane, _farSegemtns[i], segmentMin, segmentMax);

		// Update viewports.
		glk::GLVector2 frustumSize(frustumMax.x - frustumMin.x, frustumMax.y - frustumMin.y);
		const float segmentSizeX = segmentMax.x - segmentMin.x;
		const float segmentSizeY = segmentMax.y - segmentMin.y;
		//const float segmentSize = segmentSizeX < segmentSizeY ? segmentSizeY : segmentSizeX;
		//�ڷֶε�����ͶӰ������,segmentMin.x���ܱ�frustumMin.x��С,ͬ�����y������˵��Ҳ�����
		const glk::GLVector2 offsetBottomLeft(segmentMin.x - frustumMin.x, segmentMin.y - frustumMin.y);
		const glk::GLVector2 offsetSegmentSizeRatio(offsetBottomLeft.x / segmentSizeX, offsetBottomLeft.y / segmentSizeY);
		//�˴�����������׶���뵱ǰ�ֶε���׶��ĳߴ��ϵı���
		const glk::GLVector2 frustumSegmentSizeRatio(frustumSize.x / segmentSizeX, frustumSize.y / segmentSizeY);
		//����ֶε���׶��ռ����Light_Texture_Size * LIGHT_TEXTURE_SIZE �Ŀռ�
		glk::GLVector2 pixelOffsetTopLeft(offsetSegmentSizeRatio * _LIGHT_MAP_SIZE_);
		//�����ȫ����׶����ռ�ݵ������ռ�ߴ�Ϊ
		glk::GLVector2 pixelFrustumSize(frustumSegmentSizeRatio * _LIGHT_MAP_SIZE_);

		// Scale factor that helps if frustum size is supposed to be bigger
		// than maximum viewport size.
		//����������ӿڷ�Χ֮��,���Ŀǰ����������ʿڷ�Χ��OpenGL������ӿڷ�ΧҪ��
		glk::GLVector2 scaleFactor(
			m_viewportDims.x > pixelFrustumSize.x ? 1.0f : m_viewportDims.x / pixelFrustumSize.x,
			m_viewportDims.y > pixelFrustumSize.y ? 1.0f : m_viewportDims.y / pixelFrustumSize.y);
		//ƫ�������ŵ�����ӿ���
		pixelOffsetTopLeft = pixelOffsetTopLeft * scaleFactor;
		pixelFrustumSize = pixelFrustumSize * scaleFactor;

		_viewport[i] = glk::GLVector4(-pixelOffsetTopLeft.x, -pixelOffsetTopLeft.y, pixelFrustumSize.x, pixelFrustumSize.y);
		glViewportIndexedfv(i, &_viewport[i].x);

		_lightVPSBMatrix[i].identity();
		_lightVPSBMatrix[i].multiply(_lightViewMatrix);
		_lightVPSBMatrix[i].orthoProject(segmentMin.x, segmentMin.x + segmentSizeX, segmentMin.y, segmentMin.y + segmentSizeY, 0.0f, -frustumMin.z);
		_lightVPSBMatrix[i].scale(0.5f * scaleFactor.x, 0.5f * scaleFactor.y, 0.5f);
		_lightVPSBMatrix[i].translate(0.5f * scaleFactor.x, 0.5f * scaleFactor.y, 0.5f);

		//nearSegmentPlane = m_farPlanes[i];
	}

#endif
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
		//ת���ɹ淶������
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
	_cameraShader->setLightVPSBMatrix(_lightVPSBMatrix);
	_cameraShader->setNormalSegments(_normalSegments);
	_cameraShader->setBaseMap(_sphereTexture->name(),0);
	_cameraShader->setShadowMapArray(_csmShadowArray->getDepthTexture(),1);
	//���㼸����,5 x 5 ����������
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
	//��������
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
  *��Ⱦ��Ӱ����,����Ⱦ
 */
void CascadeShadowMap::renderLightView()
{
	_csmShadowArray->activeShadowFramebuffer();
	glEnable(GL_POLYGON_OFFSET_FILL);
	glColorMask(0, 0, 0, 0);

	updateLightViewFrustum();
	_lightShader->perform();
	//���м����嶼��Ҫ�Ĺ�������
	_lightShader->setShadowMapSize(glk::Size(_LIGHT_MAP_SIZE_,_LIGHT_MAP_SIZE_));
	_lightShader->setViewports(_viewport, _FRUSTUM_SEGMENT_COUNT_);
	//��Ⱦ������
	const float worldWidth = 128.0f;
	const float worldHeight = -250.0f;
	const float sphereY = 20.0f;
	//
	//���㼸����,5 x 5 ����������
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
	//��������
	glk::Matrix modelMatrix;
	modelMatrix.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
	modelMatrix.translate(0.0f, 0.0f, -128.0f);
	glk::Matrix mvpMatrix = modelMatrix * _lightViewMatrix * _lightProjMatrix;
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
	
}