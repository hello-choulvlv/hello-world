/*
  *ʵʱ��Ӱ
  *@author:xiaoxiong
  *@date:2018��8��22��
 */
//����ʵʱ��Ӱ��һЩ����
//1:��������Ӱռ��������ͼ��ѧ��һ��,���Ҷ��Ǽ�����һ��
//1:���õĹ�Ӱ����������ǿ��������ʵ��
//2:Ŀǰ��Ϊ������Ӱ������SM/LiSM/SSV/VSM/PSSM/PSVSM,����������һ���򵥵Ľ���,�Ա���߿����˽�
//2:������Ӱ�������ص��벻��.
//2:��Ӱӳ��һֱ���Ų�������/�������ȵ�����,������Ӱ��Ʒ�������Ϊ�˽����һ������趨��.
//2:ֻ����������˼·��ͬ����.
//3:SM:������Ӱӳ��,�����ھ�̬С��ģ�ĳ���,SM���������л��ں�����������ȱ��.
//3:ͨ�����ַ���ֻ����չʾ��Ӱ��ʵ�ֲ���,������ģʹ��.
//4:LiSM(Light Space Shadow Map)��ռ�͸����Ӱ,����LiSM�Ľ���,���ϸ���������ϸ���ܲ���,
//4:����ϸ�ĵ���Nvidia�����GPU-GEMS3�ص�10�¸�����ѧ���ĵ�.��ԭ�����ﲻ�ٽ���.
//5:SSV(Stencil Shadow Map)ģ����Ӱ��,���ڼ��������Ӱӳ��,��Ϊû��ʹ����ȱȽ�,��Ϊ��������Ӱ��������.
//5:����ʵ�ָ���,��Ҫ������Geometry Shader,���Ҹ��Ӷ��뼸����Ĺ�ģ������.
//6:VSM(Variance Shadow Map)������Ӱ��ͼ,��ʵ����SM����,����ʹ���� ���ʷ�������,����ܲ���������������Ӱ.
//7:PSSM(Parallelism Split Shadow Map)���зָ���Ӱ,����׶����зָ��Բ����ֲ����Ӱ.���㸴�Ӷȷǳ���,
//��ָ����Ŀ������.������Ҫ������ʵ������Ⱦ+Geometry Shader + ����Ⱦ,���ַ������ʺ��ƶ�ƽ̨,��Ϊ�ƶ�ƽ̨��
//ͼ�������汾�ձ�ϵ�,ֻ�м��������֧��GS,��iOSƽ̨������֧��GS.
//8:PSVSM(Parallelism Split Variance Shadow Map):PSSM + VSM���ۺ���.
//9:�����Ϸ�����,���ǽ���ʵ������LiSM/VSM,��Ϊ���ǵļ�����С,���Ҳ�������Ӱ�����ǿ��ŷ���.
//9:������Ϊһ�ֿ�ѡ����,���ǽ�����������ѡ����뵽������,ʵ�����,��ѡ���ܹ�������ǿ���ɵ���Ӱ������.
//����ʵʱ��Ӱ����ϸ����,���Բο�Nvidia�����<GPU-GEMS3>��������:��������Ӱ
//�Լ�Elmar Eisemann / Michael Schwarz / Ulf Assarsson��ͬ������<ʵʱ��Ӱ����>
#ifndef CC_SHADOW_H
#define CC_SHADOW_H
#include "math/CCGeometry.h"
#include "base/CCRef.h"
//#include "renderer/CCGLProgram.h"
#include "2d/CCCamera.h"
NS_CC_BEGIN
class Sprite3D;
class Mesh;
class GLProgram;
class EventListenerCustom;
class EventCustom;
enum ShadowType
{
	ShadowType_None = 0,//normal Shadow Map
	ShadowType_LiSM =1,//Light Space Shadow Map
	ShadowType_VSM = 2,//Variance Shadow Map
};
/*
  *��ʹ��ShadowParam���ݽṹ��ʱ��
  *�йظ��������Ƿ�Ҫʹ�õ���������
 */
enum  LightParamMask
{
	LightParamMask_None = 0x0,
	LightParamMask_Position = 0x1,//�Ƿ�ʹ��a_position����������
	LightParamMask_BlendWeight =0x2,//�Ƿ�ʹ��a_blendWeight����������
	LightParamMask_BlendIndex =0x4,//�Ƿ�ʹ��a_blendIndex����������
	LightParamMask_LightViewMatrix = 0x8,//�Ƿ�ʹ��u_LightViewMatrix uniform����������
	LightParamMask_LightProjMatrix = 0x10,//�Ƿ�ʹ��u_LightProjMatrix uniform����������
	LightParamMask_LightViewProjMatrix = 0x20,//�Ƿ�ʹ��u_LightViewProjMatrix����������
};

/*�������Ӱ����*/
class CC_DLL ShadowMap
{
protected:
	ShadowType    _shadowType;
	unsigned _framebufferId;//֡����ȥ����
	unsigned _depthbufferId;//��Ȼ���������
	Size          _framebufferSize;
public:
	ShadowMap(const Size &framebufferSize,ShadowType type);
	~ShadowMap();
	virtual bool         initFramebuffer();
	/*����֡����������*/
	virtual unsigned   getFramebufferId()const;
	/*������Ȼ���������*/
	virtual unsigned   getDepthbufferId()const;
	/*���ؾ���ʵ�ֵ�����*/
	ShadowType  getShadowType()const;
	/*��Ⱦ���֮��Ķ���,���������Ϊ��*/
	virtual   void   process() {};
	/*
	  *��׿��App���´���
	 */
	virtual   void   recreate();
};

class CC_DLL ShadowMapLiSM : public ShadowMap
{
public:
	ShadowMapLiSM(const Size &framebufferSize);
	void     initShadowProgram();
	virtual bool initFramebuffer();
};
/*
  *VSM��ʵ��Ҫ����һЩ,��ΪVSM������LiSM�������ڶ�Ԫ����ȱȽ�
  *���ǻ����б�ѩ��-Cantelli�������ɶ������ĸ��ʷ�������
  *����������Ҫ����һЩ
 */
class CC_DLL ShadowMapVSM : public ShadowMap
{
	unsigned    _framebufferIds[3];//��Ҫ����֡����ȥ����
	unsigned    _textureIds[2];
	unsigned  _identityVertexBuffer;//
	//���ô���Shader
	GLProgram       *_fuzzyProgram;
	int              _fuzzyAttribPositionLoc;
	int              _fuzzyAttribFragCoordLoc;
	int              _fuzzyBaseMapLoc;
	int              _fuzzyPixelStepLoc;
	int              _fuzzyFuzzyCountLoc;
	//����ģ������Ĳ�����Ŀ,Ĭ����Ŀλ4,����������ʱ��������
	int              _fuzzyCount;
public:
	ShadowMapVSM(const Size &framebufferSize);
	~ShadowMapVSM();
	void     initShadowProgram();
	virtual bool initFramebuffer();
	virtual unsigned getFramebufferId()const;
	/*����������֮��,������и�˹ģ������*/
	virtual void process();
	/*��������Ŀ*/
	void    setSampleCount(int fuzzyCount);
	int       getSampleCount()const;
	/*��ȡ�������*/
	unsigned getDepthbufferId()const;
	/*
	  *���õ�λ����������
	  *����ɾ����ʱ�򲻻�������κεĲ���
	 */
	void    setVertexBufferIdentity(unsigned  vertex_buffer);
	/*
	  *��׿App���´���
	 */
	virtual  void   recreate();
};
/*
  *���ݲ��������ݽṹ
 */
struct CC_DLL ShadowParam
{
	Camera  *camera;//�����
	ShadowType  shadowType;//��Ӱ������
	int                     shadowSize;/*��Ӱ��ͼ�Ĵ�С*/
	Vec3                 lightPosition;/*��ǽ��λ��*/
	Vec3                 lightDirection;/*��ķ���*/
};
/*
  *�����ݽṹ�������ݸ�MeshCommand
  *
 */
struct CC_DLL LightShadowParam
{
	Mat4              *lightViewMatrix;
	Mat4              *lightProjMatrix;
	Mat4              *lightViewProjMatrix;
	//vertex stride
	int                   vertexStride;
	//attrib position
	int                   positionLoc;
	int                   blendWeightLoc;
	int                   blendIndexLoc;
	//uniform location
	int                   matrixPaletteLoc;
	int                   modelViewLoc;
	int                   projLoc;
};
/*
  *��ʵʱ��Ӱ��ʵ�ֹ�����,���������㷨���,������ʵ�ֹ����н���������ϸ��ע��
  *����ʵʱ��Ӱ��Ϊһ�����Ƿ�Χ�ǳ���ĸ���,���ǲ���ʵ�ִ�ƽ�й⵽���Դ�ٵ��۹�Ƶ����в�ͬ��Դ
  *��������Ӱ,��ֻʵ��ƽ�й��������Ӱ.
  *�����������Ȥ�Ļ�,Ҳ�������н�û��ʵ�ֵĲ��ֲ�����ȥ
  *������Ӱ�ı�Ҫ����Ϊ:Camera/ShadowMap��Ϊ��
 */
class CC_DLL Shadows : public Ref
{
	//��Ӱ����ʵ������
	ShadowMap    *_shadowMap;
	GLProgram     *_skinProgram;
	GLProgram     *_lismProgram;
	//�볡����ص������
	Camera             *_sceneCamera;
	//��׶��İ�Χ��
	AABB                  _frustumAABB;
	//������ģ�͵İ�Χ��
	AABB                    _casterAABB;
	//�����ɫ
	Vec3                   _lightColor;
	//���������ɫ
	Vec3                   _lightAmbientColor;
	//���ߵķ���
	Vec3                    _lightDirection;
	//��ǽ��λ��
	Vec3                    _lightPosition;
	//��Դ����ͼ����
	Mat4                   _lightViewMatrix;
	//��Դ��ͶӰ����
	Mat4                   _lightProjMatrix;
	//��Դ����ͼͶӰ����
	Mat4                   _lightViewProjMatrix;
	//����shader,�Ƿ��Թ���ӽǲ鿴����
	int                        _useLightMode;
	//��Ϊ��������ɲ��ֵļ���ģ��,�˼���ģ������������Ӱ
	std::vector<Mesh*>   _shadowCasters;
	//�Ƿ��������,�������־����λtrue֮��,���ɵ���Ӱ������������������,
	//��Ȼ������Ҳ������,���ȡ����Ӧ�ó����Ƿ��ܽ����ɴ˶�������������ʧ.
	bool                    _analysisScene;
	/*�Ƿ��Ͷ������пɼ��Բ���*/
	bool                    _visibilityTest;
	int                       _shadowSize;
	/*�����ƫ��*/
	float                   _polygonOffset[2];
	//����ɫ��������
	Vec3                  _lightBleedingVec3;
	//����ÿ���سߴ�
	Vec3                  _lightPixelStepVec3;
	//��һ�ΰ󶨵�֡����ȥ����
	int                      _lastFramebufferId;
	//��Ӱ��ͼ��shader�е�����,Ĭ��ΪCC_Texture3
	std::string        _shadowMapName;
	std::string        _lightViewMatrixName;
	std::string        _lightProjMatrixName;
	std::string        _lightDirectionName;
	std::string        _lightBleedingName;
	std::string        _lightColorName;
	std::string        _lightAmbientColorName;
	std::string        _useLightModeName;
	std::string        _lightViewProjMatrixName;
	std::string        _lightPixelStepName;
	LightShadowParam  _lightAttribUniformParam;
	//��λarray buffer
	unsigned          _identityArrayBuffer;
	GLProgram     *_debugProgram;
	//֡����ȥ��������
	GLProgram    *_clearProgram;
	int                      _clearPositionLoc;
	int                      _clearColorLoc;
	Vec4                  _clearColor;
	//��׿App������������
	EventListenerCustom *_backgroundListener;
public:
	explicit Shadows();
	~Shadows();
	bool   initWithShadowParam(const ShadowParam &param);
	static Shadows *create(const ShadowParam &param);
	/*
	  *��̬����,��⼰���Ƿ�֧��Shadows
	 */
	static bool   checkMachineSupport(ShadowType  shdowType);
	/*
	  *Ӧ�ó���İ汾
	 */
	static  int     getVertion();
	/*
	  *���ò�����Ӱ������
	 */
	bool   setShadowType(ShadowType  shadowType);
	 ShadowType getShadowType()const;
	/*
	  *���ò�����Ӱ�������,���������׶������Ų�����Ӱ�ķ�Χ
	 */
	void   setCamera(Camera *camera);
	Camera  *getCamera()const;
	/*
	  *����������ͼ����,lightDirectionһ��Ҫ��λ��
	 */
	void   setLightViewMatrix(const Vec3 &lightDirection,const Vec3 &lightPosition);
	/*
	  *���û�����ǿ��
	 */
	void  setAmbientColor(const Vec3  &ambientColor);
	const Vec3 &getAmbientColor()const;
	/*
	  *���ù��ǿ��
	 */
	void  setLightColor(const Vec3 &lightColor);
	const Vec3 &getLightColor()const;
	/*
	  *������Ӱռ����ɫ�ı���,����ֵӦ�뻷����� 1- a��ϵ,aΪ������
	 */
	void   setShadowWeight(float f);
	float   getShadowWeight()const;
	/*
	  *���ù���ɫ����,�˺�������ֻ��VSM������
	 */
	void  setLightBleeding(float light_bleeding);
	float  getLightBleeding()const;
	/*
	*����VSM��������Ŀ
	*/
	void   setVSMSampleCount(int  sample_count);
	int      getVSMSampleCount()const;
	/*
	  *�Ƿ��Թ���ӽǲ鿴����
	 */
	void   setLightMode(bool  lightMode);
	bool      isLightMode()const;
	/*
	  *�Ƿ���г�������
	 */
	void   setAnalysisScene(bool analysisScene);
	bool   isAnalysisScene()const;
	/*
	  *�Ƿ�Թ�ռ�ͶӰ��������пɼ��Բ���
	 */
	void   setCasterVisibilityTest(bool visibilityTest);
	bool   isCasterVisibilityTest()const;
	/*
	  *������Ӱ���������
	 */
	void   setShadowMapName(const std::string &name);
	const std::string &getShadowMapName()const;
	/*
	  *���ö����ƫ��ϵ��
	 */
	void   setPolygonOffset(float polygonOffset[2]);
	/*
	  *���������ƫ��
	 */
	void   setPolygonEnabled(bool b);
	/*
	  *���������Ӱ�Ķ�������Ӽ���ģ��
	 */
	void   pushShadowCaster(Mesh **meshes,int meshCount,const AABB &aabb,bool useShadow);
	void   addMeshes(Mesh **meshes, int meshCount);
	void   addMesh(Mesh *mesh);
	/*
	  *��Ⱦ��Ӱ
	 */
	void   renderShadow();
	/*
	  *��ȾVSM��Ӱ
	 */
	void   renderShadowVSM();
	/*
	  *��ȾLiSM��Ӱ
	 */
	void   renderShadowLiSM();
	/*
	  *��Ⱦ����ǰ��׼������
	 */
	void   beforeRender();
	/*
	  *�����꼸�����ĺ�������
	 */
	void   afterVisitGeometry();
	/*
	  *��Ⱦ������ĺ�������
	 */
	void   afterRender();
	/*
	  *��������
	 */
	void  debugTexture(int texture);
	/*
	  *��׿,Ӧ�ó������´���
	 */
	void  recreate(EventCustom *recreateEvent);
};

NS_CC_END
#endif
