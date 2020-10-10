/*
  *实时阴影
  *@author:xiaoxiong
  *@date:2018年8月22日
 */
//关于实时阴影的一些介绍
//1:光照与阴影占据了整个图形学的一半,并且都是集成在一起。
//1:良好的光影交互更能增强场景的真实感
//2:目前较为流行阴影技术有SM/LiSM/SSV/VSM/PSSM/PSVSM,以下我们做一个简单的介绍,以便读者快速了解
//2:各种阴影技术的特点与不足.
//2:阴影映射一直有着采样不足/采样过度的困扰,各种阴影设计方案都是为了解决这一问题而设定的.
//2:只不过出发的思路不同而已.
//3:SM:常规阴影映射,适用于静态小规模的场景,SM集中了所有基于后处理技术的所有缺点.
//3:通常这种方案只用来展示阴影的实现步骤,不会大规模使用.
//4:LiSM(Light Space Shadow Map)光空间透视阴影,关于LiSM的介绍,网上给出来的详细介绍不多,
//4:最详细的当属Nvidia出版的GPU-GEMS3地第10章给出的学术文档.其原理这里不再介绍.
//5:SSV(Stencil Shadow Map)模板阴影体,基于几何体的阴影映射,因为没有使用深度比较,因为避免了阴影走样问题.
//5:但是实现复杂,需要借助于Geometry Shader,并且复杂度与几何体的规模成正比.
//6:VSM(Variance Shadow Map)方差阴影贴图,其实现与SM相似,但是使用了 概率分析估计,因此能产生更高质量的阴影.
//7:PSSM(Parallelism Split Shadow Map)并行分割阴影,对视锥体进行分割以产生分层的阴影.计算复杂度非常大,
//与分割的数目成正比.并且需要借助于实例化渲染+Geometry Shader + 层渲染,此种方案不适合移动平台,因为移动平台的
//图形驱动版本普遍较低,只有极个别机器支持GS,而iOS平台根本不支持GS.
//8:PSVSM(Parallelism Split Variance Shadow Map):PSSM + VSM的综合体.
//9:在以上方案中,我们将会实现两种LiSM/VSM,因为他们的计算量小,并且产生的阴影质量是可信服的.
//9:另外作为一种可选方案,我们将将场景分析选项加入到方案中,实验表明,该选项能够显著增强生成的阴影的质量.
//关于实时阴影的详细论述,可以参考Nvidia出版的<GPU-GEMS3>第三部分:光照与阴影
//以及Elmar Eisemann / Michael Schwarz / Ulf Assarsson共同编著的<实时阴影技术>
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
  *在使用ShadowParam数据结构的时候
  *有关各个参数是否要使用的掩码描述
 */
enum  LightParamMask
{
	LightParamMask_None = 0x0,
	LightParamMask_Position = 0x1,//是否使用a_position属性描述符
	LightParamMask_BlendWeight =0x2,//是否使用a_blendWeight属性描述符
	LightParamMask_BlendIndex =0x4,//是否使用a_blendIndex属性描述符
	LightParamMask_LightViewMatrix = 0x8,//是否使用u_LightViewMatrix uniform变量描述符
	LightParamMask_LightProjMatrix = 0x10,//是否使用u_LightProjMatrix uniform变量描述符
	LightParamMask_LightViewProjMatrix = 0x20,//是否使用u_LightViewProjMatrix变量描述符
};

/*常规的阴影纹理*/
class CC_DLL ShadowMap
{
protected:
	ShadowType    _shadowType;
	unsigned _framebufferId;//帧缓冲去对象
	unsigned _depthbufferId;//深度缓冲区对象
	Size          _framebufferSize;
public:
	ShadowMap(const Size &framebufferSize,ShadowType type);
	~ShadowMap();
	virtual bool         initFramebuffer();
	/*返回帧缓冲区对象*/
	virtual unsigned   getFramebufferId()const;
	/*返回深度缓冲区对象*/
	virtual unsigned   getDepthbufferId()const;
	/*返回具体实现的类型*/
	ShadowType  getShadowType()const;
	/*渲染完毕之后的动作,函数体可以为空*/
	virtual   void   process() {};
	/*
	  *安卓上App重新创建
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
  *VSM的实现要复杂一些,因为VSM并非像LiSM那样基于二元的深度比较
  *而是基于切比雪夫-Cantelli大数定律而衍生的概率分析估算
  *因此其计算量要更大一些
 */
class CC_DLL ShadowMapVSM : public ShadowMap
{
	unsigned    _framebufferIds[3];//需要三个帧缓冲去对象
	unsigned    _textureIds[2];
	unsigned  _identityVertexBuffer;//
	//后置处理Shader
	GLProgram       *_fuzzyProgram;
	int              _fuzzyAttribPositionLoc;
	int              _fuzzyAttribFragCoordLoc;
	int              _fuzzyBaseMapLoc;
	int              _fuzzyPixelStepLoc;
	int              _fuzzyFuzzyCountLoc;
	//后置模糊处理的采样数目,默认数目位4,可以在运行时自行设置
	int              _fuzzyCount;
public:
	ShadowMapVSM(const Size &framebufferSize);
	~ShadowMapVSM();
	void     initShadowProgram();
	virtual bool initFramebuffer();
	virtual unsigned getFramebufferId()const;
	/*获得深度纹理之后,对其进行高斯模糊处理*/
	virtual void process();
	/*采样的数目*/
	void    setSampleCount(int fuzzyCount);
	int       getSampleCount()const;
	/*获取深度纹理*/
	unsigned getDepthbufferId()const;
	/*
	  *设置单位缓冲区对象
	  *对象被删除的时候不会对它做任何的操作
	 */
	void    setVertexBufferIdentity(unsigned  vertex_buffer);
	/*
	  *安卓App重新创建
	 */
	virtual  void   recreate();
};
/*
  *传递参数的数据结构
 */
struct CC_DLL ShadowParam
{
	Camera  *camera;//摄像机
	ShadowType  shadowType;//阴影的类型
	int                     shadowSize;/*阴影贴图的大小*/
	Vec3                 lightPosition;/*光墙的位置*/
	Vec3                 lightDirection;/*光的方向*/
};
/*
  *此数据结构用来传递给MeshCommand
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
  *在实时阴影的实现过程中,我们力求算法简洁,并且在实现过程中将会作出详细的注释
  *但是实时阴影作为一个涵盖范围非常广的概念,我们不会实现从平行光到点光源再到聚光灯等所有不同光源
  *产生的阴影,而只实现平行光产生的阴影.
  *如果读者有兴趣的话,也可以自行将没有实现的部分补充上去
  *产生阴影的必要条件为:Camera/ShadowMap不为空
 */
class CC_DLL Shadows : public Ref
{
	//阴影纹理实现类型
	ShadowMap    *_shadowMap;
	GLProgram     *_skinProgram;
	GLProgram     *_lismProgram;
	//与场景相关的摄像机
	Camera             *_sceneCamera;
	//视锥体的包围盒
	AABB                  _frustumAABB;
	//几何体模型的包围盒
	AABB                    _casterAABB;
	//光的颜色
	Vec3                   _lightColor;
	//环境光的颜色
	Vec3                   _lightAmbientColor;
	//光线的方向
	Vec3                    _lightDirection;
	//光墙的位置
	Vec3                    _lightPosition;
	//光源的视图矩阵
	Mat4                   _lightViewMatrix;
	//光源的投影矩阵
	Mat4                   _lightProjMatrix;
	//光源的视图投影矩阵
	Mat4                   _lightViewProjMatrix;
	//调试shader,是否以光的视角查看场景
	int                        _useLightMode;
	//作为场景的组成部分的几何模型,此几何模型用来产生阴影
	std::vector<Mesh*>   _shadowCasters;
	//是否分析场景,当这个标志设置位true之后,生成的阴影的质量将会大幅度提升,
	//当然计算量也会增加,这个取决于应用程序是否能接受由此而带来的性能损失.
	bool                    _analysisScene;
	/*是否对投光物进行可见性测试*/
	bool                    _visibilityTest;
	int                       _shadowSize;
	/*多边形偏移*/
	float                   _polygonOffset[2];
	//光渗色修正参数
	Vec3                  _lightBleedingVec3;
	//纹理每像素尺寸
	Vec3                  _lightPixelStepVec3;
	//上一次绑定的帧缓冲去对象
	int                      _lastFramebufferId;
	//阴影贴图在shader中的名字,默认为CC_Texture3
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
	//单位array buffer
	unsigned          _identityArrayBuffer;
	GLProgram     *_debugProgram;
	//帧缓冲去对象清理
	GLProgram    *_clearProgram;
	int                      _clearPositionLoc;
	int                      _clearColorLoc;
	Vec4                  _clearColor;
	//安卓App重新生成销毁
	EventListenerCustom *_backgroundListener;
public:
	explicit Shadows();
	~Shadows();
	bool   initWithShadowParam(const ShadowParam &param);
	static Shadows *create(const ShadowParam &param);
	/*
	  *静态函数,检测及其是否支持Shadows
	 */
	static bool   checkMachineSupport(ShadowType  shdowType);
	/*
	  *应用程序的版本
	 */
	static  int     getVertion();
	/*
	  *设置产生阴影的类型
	 */
	bool   setShadowType(ShadowType  shadowType);
	 ShadowType getShadowType()const;
	/*
	  *设置产生阴影的摄像机,摄像机的视锥体决定着产生阴影的范围
	 */
	void   setCamera(Camera *camera);
	Camera  *getCamera()const;
	/*
	  *重新设置视图矩阵,lightDirection一定要单位化
	 */
	void   setLightViewMatrix(const Vec3 &lightDirection,const Vec3 &lightPosition);
	/*
	  *设置环境光强度
	 */
	void  setAmbientColor(const Vec3  &ambientColor);
	const Vec3 &getAmbientColor()const;
	/*
	  *设置光的强度
	 */
	void  setLightColor(const Vec3 &lightColor);
	const Vec3 &getLightColor()const;
	/*
	  *设置阴影占据颜色的比重,此数值应与环境光呈 1- a关系,a为环境光
	 */
	void   setShadowWeight(float f);
	float   getShadowWeight()const;
	/*
	  *设置光渗色参数,此函数调用只对VSM起作用
	 */
	void  setLightBleeding(float light_bleeding);
	float  getLightBleeding()const;
	/*
	*设置VSM采样的数目
	*/
	void   setVSMSampleCount(int  sample_count);
	int      getVSMSampleCount()const;
	/*
	  *是否以光的视角查看场景
	 */
	void   setLightMode(bool  lightMode);
	bool      isLightMode()const;
	/*
	  *是否进行场景分析
	 */
	void   setAnalysisScene(bool analysisScene);
	bool   isAnalysisScene()const;
	/*
	  *是否对光空间投影几何体进行可见性测试
	 */
	void   setCasterVisibilityTest(bool visibilityTest);
	bool   isCasterVisibilityTest()const;
	/*
	  *设置阴影纹理的名字
	 */
	void   setShadowMapName(const std::string &name);
	const std::string &getShadowMapName()const;
	/*
	  *设置多边形偏移系数
	 */
	void   setPolygonOffset(float polygonOffset[2]);
	/*
	  *开启多边形偏移
	 */
	void   setPolygonEnabled(bool b);
	/*
	  *向待产生阴影的队列中添加几何模型
	 */
	void   pushShadowCaster(Mesh **meshes,int meshCount,const AABB &aabb,bool useShadow);
	void   addMeshes(Mesh **meshes, int meshCount);
	void   addMesh(Mesh *mesh);
	/*
	  *渲染阴影
	 */
	void   renderShadow();
	/*
	  *渲染VSM阴影
	 */
	void   renderShadowVSM();
	/*
	  *渲染LiSM阴影
	 */
	void   renderShadowLiSM();
	/*
	  *渲染场景前的准备工作
	 */
	void   beforeRender();
	/*
	  *遍历完几何体后的函数调用
	 */
	void   afterVisitGeometry();
	/*
	  *渲染场景后的后续动作
	 */
	void   afterRender();
	/*
	  *调试纹理
	 */
	void  debugTexture(int texture);
	/*
	  *安卓,应用程序重新创建
	 */
	void  recreate(EventCustom *recreateEvent);
};

NS_CC_END
#endif
